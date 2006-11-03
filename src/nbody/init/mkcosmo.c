/*
 * MKCOSMO: set up a cube from a density grid, for cosmology
 *	
 *	 1-nov-06  V0.1  Created     - Peter Teuben / Ed Shaya
 *
 * todo:
 *  - first point is always 0,0,0
 *  - do rescale_image()
 *  - implement (1+z) scaling in rescale_image
 */

#include <stdinc.h>
#include <getparam.h>
#include <vectmath.h>
#include <filestruct.h>

#include <snapshot/snapshot.h>
#include <snapshot/body.h>
#include <snapshot/put_snap.c>

#include <image.h>

string defv[] = {	/* DEFAULT INPUT PARAMETERS */
  "in=???\n       Input density (fluctuation) cube",
  "out=???\n      Output file name",
  "z=\n           Redshift, for growth function 1/(1+x)",
  "D=\n           Growth function value, given explicitly",
  "absrho=t\n     Is map absolute density or relative d(rho)/rho?",
  "sigma=0\n      Also perturb distances by gaussian sigma",
  "seed=0\n       Initial seed",
  "headline=\n    Random verbiage",
  "VERSION=0.3\n  2-nov-06 PJT",
  NULL,
};

string usage = "create a cosmology cube of equal massive stars";

string cvsid="$Id$";

local Body *btab;
local int nbody;
local real sigma;

local imageptr iptr=NULL;

extern double xrandom(double,double), grandom(double,double);

void rescale_image(bool Qabs);
void rescale_image_d(real d);
void writegalaxy(string name, string headline);
void mkcube(void);
void fiddle_x(void),  fiddle_y(void),  fiddle_z(void);

void nemo_main()
{
  int seed;
  stream instr;
  real z = getdparam("z");
  bool Qabs = getbparam("absrho");

  instr = stropen (getparam("in"),"r");
  read_image(instr,&iptr); 
  strclose(instr);      
  dprintf(0,"MinMax = %g %g \n",MapMin(iptr),MapMax(iptr));

  if (hasvalue("D"))
    rescale_image_d(getdparam("D"));

#if 0
  rescale_image(Qabs);
#endif
  
  if (Nx(iptr) != Ny(iptr) || Nx(iptr) != Nz(iptr))
    warning("Input data is not a cube: %d x %d x %d",Nx(iptr),Ny(iptr),Nz(iptr));
  nbody = Nx(iptr)*Ny(iptr)*Nz(iptr);

  sigma = getdparam("sigma");
  seed = init_xrandom(getparam("seed"));
  
  mkcube();
#if 0
  fiddle_x();
  fiddle_y();
  fiddle_z();
#else
  fiddle_z();
#endif
  writegalaxy(getparam("out"), getparam("headline"));
  free(btab);
}


void rescale_image(bool Qabs)
{  
  int ix,iy,iz;
  int nx=Nx(iptr), ny=Ny(iptr), nz=Nz(iptr);
  real sum;

  /* first get the total number in densities (mass) */

  sum = 0.0;
  for (iz=0; iz<nz; iz++)
    for (iy=0; iy<ny; iy++)
      for (ix=0; ix<nx; ix++)
	sum += CubeValue(iptr,ix,iy,iz);

  if (Qabs) {
    /* if Absolute densities: */
    for (iz=0; iz<nz; iz++)
      for (iy=0; iy<ny; iy++)
	for (ix=0; ix<nx; ix++)
	  CubeValue(iptr,ix,iy,iz) /= sum;
  } else {
    /* if Relative densities: */
    if (sum != 0.0) warning("Relative densities, but sum=%g should be 0.0",sum);
    for (iz=0; iz<nz; iz++)
      for (iy=0; iy<ny; iy++)
	for (ix=0; ix<nx; ix++)
	  CubeValue(iptr,ix,iy,iz) += 1.0;
  }
}

void rescale_image_d(real d)
{  
  int ix,iy,iz;
  int nx=Nx(iptr), ny=Ny(iptr), nz=Nz(iptr);
  real sum;

  /* first get the total number in densities (mass) */

  sum = 0.0;
  for (iz=0; iz<nz; iz++)
    for (iy=0; iy<ny; iy++)
      for (ix=0; ix<nx; ix++)
	CubeValue(iptr,ix,iy,iz) = d*CubeValue(iptr,ix,iy,iz);

}


/*
 * WRITEGALAXY: write galaxy model to output.
 */

void writegalaxy(string name, string headline)
{
  stream outstr;
  real tsnap = 0.0;
  int bits = MassBit | PhaseSpaceBit;
  
  if (! streq(headline, ""))
    set_headline(headline);
  outstr = stropen(name, "w");
  put_history(outstr);
  put_snap(outstr, &btab, &nbody, &tsnap, &bits);
  strclose(outstr);
}

/*
 * MKCUBE: initial homogenous cube based on the input image
 */

void mkcube(void)
{
  Body *bp;
  real x,y,z,mass_i;
  int ix,iy,iz;

  bp = btab = (Body *) allocate(nbody * sizeof(Body));
  mass_i = 1.0/nbody;
  for (iz=0, z=Zmin(iptr); iz<Nz(iptr); iz++, z+=Dz(iptr))
    for (iy=0, y=Ymin(iptr); iy<Ny(iptr); iy++, y+=Dy(iptr))
      for (ix=0, x=Xmin(iptr); ix<Nx(iptr); ix++, x+=Dx(iptr)) {
	Mass(bp) = mass_i;
	Phase(bp)[0][0] = x;
	Phase(bp)[0][1] = y;
	Phase(bp)[0][2] = z;
	Phase(bp)[1][0] = 0.0;
	Phase(bp)[1][1] = 0.0;
	Phase(bp)[1][2] = 0.0;
	if (sigma > 0) {
	  Phase(bp)[0][0] += grandom(0.0,sigma);
	  Phase(bp)[0][1] += grandom(0.0,sigma);
	  Phase(bp)[0][2] += grandom(0.0,sigma);
	}
	bp++;
      }
}


/* 
 * fiddle_x: sweep in X and accumulate mass as such, re-adjust
 *           X position from center of cell as to agree with
 *           the thus obtained cumulative M(<x)
 *
 * fiddle_y: --same, but then in y--
 * fiddle_z: --same, but then in z--
 */


void fiddle_x(void)
{
  int ix,iy,iz,i,nerr=0;
  int nx=Nx(iptr),ny=Ny(iptr),nz=Nz(iptr);
  real dx, xold, xnew, xhit, mold, mnew, mhit, err;
  Body *bp;

  mold = mhit = 0.0;
  xold = 0.0;
  dx = Dx(iptr);
  for (iz=0; iz<nz; iz++) {
    for (iy=0; iy<ny; iy++) {
      xold = Xmin(iptr);
      for (ix=0; ix<nx; ix++) {
	mnew = mold + CubeValue(iptr,ix,iy,iz);
	xnew = xold + dx;
	xhit = xold + dx*(mhit-mold)/(mnew-mold);
	dprintf(1,"X: %d %d %d -> xold=%g xhit=%g\n",ix,iy,iz,xold,xhit);
	i = ix + ny*(iy+nz*iz);
	bp = btab + i;
	err = (Phase(bp)[0][0]-xhit)/dx;
	if (-0.5<err && 0.5<err) nerr++;
	Phase(bp)[0][0] = xhit;
	mhit += 1.0;
	mold = mnew;
	xold = xnew;
      }
    }
  }
  if (nerr) warning("Found %d non-linear deviations in X",nerr);
}

void fiddle_y(void)
{
  int ix,iy,iz,i,nerr=0;
  int nx=Nx(iptr),ny=Ny(iptr),nz=Nz(iptr);
  real dy, yold, ynew, yhit, mold, mnew, mhit, err;
  Body *bp;
  
  mold = mhit = 0.0;
  yold = 0.0;
  dy = Dy(iptr);
  for (ix=0; ix<nx; ix++) {
    for (iz=0; iz<nz; iz++) {
      yold = Ymin(iptr);
      for (iy=0; iy<ny; iy++) {
	mnew = mold + CubeValue(iptr,ix,iy,iz);
	ynew = yold + dy;
	yhit = yold + dy*(mhit-mold)/(mnew-mold);
	dprintf(1,"Y: %d %d %d -> yold=%g yhit=%g\n",ix,iy,iz,yold,yhit);
	i = ix + ny*(iy+nz*iz);
	bp = btab + i;
	err = (Phase(bp)[0][1]-yhit)/dy;
	if (-0.5<err && 0.5<err) nerr++;
	Phase(bp)[0][1] = yhit;
	mhit += 1.0;
	mold = mnew;
	yold = ynew;
      }
    }
  }
  if (nerr) warning("Found %d non-linear deviations in Y",nerr);
}

void fiddle_z(void)
{
  int ix,iy,iz,i,nerr=0;
  int nx=Nx(iptr),ny=Ny(iptr),nz=Nz(iptr);
  real dz, zold, znew, zhit, mold, mnew, mhit, err;
  Body *bp;

  
  mold = mhit = 0.0;
  zold = 0.0;
  dz = Dz(iptr);
  for (iy=0; iy<ny; iy++) {
    for (ix=0; ix<nx; ix++) {
      zold = Zmin(iptr);
      for (iz=0; iz<nz; iz++) {
	mnew = mold + CubeValue(iptr,ix,iy,iz);
	znew = zold + dz;
	zhit = zold + dz*(mhit-mold)/(mnew-mold);
	dprintf(1,"Z: %d %d %d -> zold=%g zhit=%g\n",ix,iy,iz,zold,zhit);
	i = ix + ny*(iy+nz*iz);
	bp = btab + i;
	err = (Phase(bp)[0][2]-zhit)/dz;
	if (-0.5<err && 0.5<err) nerr++;
	Phase(bp)[0][2] = zhit;
	mhit += 1.0;
	mold = mnew;
	zold = znew;
      }
    }
  }
  if (nerr) warning("Found %d non-linear deviations in Z",nerr);
}
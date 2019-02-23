# A birds eye view of NEMO

## Installing

To use it (*this assumes somebody has installed it for you*) in csh shell:

       source /somewhere/nemo/nemo_start.csh

To install it in a bash shell on MacOSX with pgplot installed via Homebrew:

       curl -O https://teuben.github.io/nemo/install_nemo
       chmod +x install_nemo
       ./install_nemo brew=1
       source nemo/nemo_start.sh

The source command can then be added to your **.cshrc** (csh shell) or
**.bashrc** (linux bash) or **.bash_profile** (mac bash)

## Using

Your unix shell will now have been modified and a large number
of new commands are available. Much like other unix commands
these NEMO commands will:
    

      * live in $NEMOBIN, e.g.                    ls $NEMOBIN
      * have a unix man page, e.g.                man mkplummer
      * have a --help option, e.g.                mkplummer --help
      * have a help= system keyword, e.g.         mkplummer help=\?
        and describe the program keywords, e.g.   mkplummer help=h
      * use unix pipes                            mkplummer - 10 | tsf -

A few other NEMO things useful to know

      * recompile/add a NEMO program              mknemo ccdsky
      * parameters can have math                  vscale='1/sqrt(2)'
      * lists can have implied do loops           ycol=2:12:2  color=2,3::9,4
      * parameters can refer to others            dtadj='$deltat/2'
      * parameters can be stored in a file        rad=@rad.in
      * nemoinp expression parser                 nemoinp 'c,G,h,pi,iflt(1,2,3,4)'
        1 AU in km                                nemoinp 'p*pi/(3600*180)' 

## Example 1: Gentle collapse of a plummer sphere


Initialize

       mkplummer p1k 1024
       snapscale p1k p1ks vscale="1/sqrt(2)"

Evolve (pick one)

       hackcode1 p1ks run1.out tstop=20 freqout=10
       gyrfalcON p1ks run2.out tstop=20 step=0.1    eps=0.05 kmax=7
       runbody6  p1ks run3     tcrit=10 deltat=0.1 

Plot

       snapplot run1.out nxy=3,3 times=0,1,2,5,10,15,20 yapp=1/xs
       snapmradii run1.out 0.01,0.1:0.9:0.1,0.99 log=t | tabplot - 1 2:12 line=1,1  color=2,3::9,2 yapp=2/xs

Convert to a "CCD"

       snapgrid run1.out t0.ccd nx=128 ny=128 times=0
       snapgrid run1.out t1.ccd nx=128 ny=128 times=20

Smooth the CCD to 0.1

       ccdsmooth t0.ccd t0s.ccd
       ccdsmooth t1.ccd t1s.ccd

Plot
       ccdplot t0s.ccd contour=0.1,0.3,0.7 yapp=3/xs
       ccdplot t1s.ccd contour=0.1,0.3,0.7 yapp=4/xs

Convert to FITS for comparison with observations

       ccdfits t0s.ccd t0s.fits
       ccdfits t1s.ccd t1s.fits
       ds9 t0s.fits

Look at outliers via total M/L=1 intensity map (look for Max and Sum*Dx*Dy)

       ccdstat t0s.ccd
       ccdstat t1s.ccd


## Other geeky things that NEMO does

   * Uses hierarchy of "Makefile" for installation
   * Uses hierarchy of "Testfile" to do regression/baseline tests
   * Has $NEMO/src (supported) and $NEMO/usr (unsupported)
   * Can use $NEMO/opt to override bad system libraries

## Useful software that is NEMO friendly

   * ds9
   * glnemo2
   * python  (astropy, APLpy, ....)
   * ImageMagick
   * Montage

## Some other related good stuff

   * ZENO
   * yt
   * AMUSE
   
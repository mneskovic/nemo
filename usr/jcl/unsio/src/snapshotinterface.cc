// ============================================================================
// Copyright Jean-Charles LAMBERT - 2008-2010                                       
// e-mail:   Jean-Charles.Lambert@oamp.fr                                      
// address:  Dynamique des galaxies                                            
//           Laboratoire d'Astrophysique de Marseille                          
//           Pole de l'Etoile, site de Chateau-Gombert                         
//           38, rue Frederic Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 6110                                                   
// ============================================================================

/**
   @author Jean-Charles Lambert <Jean-Charles.Lambert@oamp.fr>
*/

#include "snapshotinterface.h"
#include "ctools.h"
#include "uns.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <cmath>
#include <cstdlib>
namespace uns {
  // ============================================================================
  // parseSelectTime                                                             
  void CSnapshotInterfaceIn::parseSelectTime()
  {
    std::string current_s,next_s;
    next_s = select_time;
    while ((current_s=uns::UserSelection::parseString(next_s)) != "") {  // || 1 force parsing
      getRangeTime(current_s);
    }
  }
  // ============================================================================
  // nextFrame 
  int CSnapshotInterfaceIn::nextFrame(std::string bits)
  {
    int ret=0;
    computeBits(bits);
    if (isNewFrame()) {
      crvs = getSnapshotRange();
      if (crvs) {
        ret=nextFrameSelect(crvs);
      }
    }
    return ret;
  }
  // ============================================================================
  // nextFrame 
  int CSnapshotInterfaceIn::nextFrameSelect(ComponentRangeVector * crvs)
  {
    user_select.setSelection(getSelectPart(),crvs);
    setNsel(user_select.getNSel());
    return(nextFrame(user_select.getIndexesTab(),user_select.getNSel()));
  }
  // ============================================================================
  // computeBits 
  void CSnapshotInterfaceIn::computeBits(std::string bits)
  {
    req_bits = 0;
    if (verbose) std::cerr << "BITS ="<<bits<<"\n";
    for (unsigned int i=0;i <bits.length();i++) {
      switch (bits.at(i)) {
      case 'm': req_bits |= MASS_BIT  ; break;
      case 'x': req_bits |= POS_BIT   ; break;
      case 'v': req_bits |= VEL_BIT   ; break;
      case 'p': req_bits |= POT_BIT   ; break;
      case 'a': req_bits |= ACC_BIT   ; break;
      case 'e': req_bits |= EPS_BIT   ; break;
      case 'k': req_bits |= KEYS_BIT  ; break;
      case 'X': req_bits |= AUX_BIT   ; break;
      case 'R': req_bits |= RHO_BIT   ; break;
      case 'I': req_bits |= ID_BIT    ; break;
      case 'U': req_bits |= U_BIT     ; break;
      case 'M': req_bits |= METAL_BIT ; break;
      case 'A': req_bits |= AGE_BIT   ; break;        
      case 'H': req_bits |= HSML_BIT  ; break;        
      default: 
        std::cerr << "!!!!WARNING unknown requested bit : <"<<bits.at(i)<<">\n";
        break;
      }
      //std::cerr << "req_bits ["<<bits.at(i)<<"]=" <<req_bits <<"\n";
    }
  }

  // ============================================================================
  // getRangeTime                                                                
  // split a string a:b[:c] into inf, sup,delta                                  
  void CSnapshotInterfaceIn::getRangeTime(const std::string rtime)
  {
    int status;
    std::vector<float> store;
    int ppos=0;
    bool stop=false;
    int cpt=0;
    while (! stop) {
      size_t found = rtime.find(':',ppos);
      if (found!=std::string::npos) {
        if (found > (size_t) (ppos)) {
          cpt++;
          std::string str=rtime.substr(ppos,found-ppos);
          std::istringstream ss(str);
          float val;
          ss>>val;
          store.push_back(val);
        }
        ppos=found+1; //
      } else { // no more ":"
        std::string str=rtime.substr(ppos);
        if (str == "all") {
          store.push_back(-1.0);
        }
        else {
          std::istringstream ss(str);
          float val;
          ss>>val;
          store.push_back(val);
        }
        stop=true;
      }
    }
    for (std::vector<float>::iterator it=store.begin(); it!=store.end(); it++) {
      //std::cerr << " range i ="<< it-store.begin() << "\n";
      //std::cerr << " value   =" << *it << "\n";
    }
    float inf=store[0];
    float sup=inf;
    float offset=0.0;
    if (store.size()>1) {
      sup = store[1];
    }
    if (store.size()>2) {
      offset = store[2];
    }
    
    assert(sup>=inf);
    status=0;
    // store inf and supp time in vector array
    CSelectTime st(inf,sup,offset,-666.0);
    stv.push_back(st);
  }
  // ============================================================================
  // checkRangeTime                                                              
  // check if current time belong to the interval of the selected time           
  bool CSnapshotInterfaceIn::checkRangeTime(const float current_time)
  {
    assert(stv.size()>0);
    for (CSelectTimeVector::iterator it=stv.begin(); it!=stv.end(); it++) {
      //std::cerr << "t inf="<<it->inf<<" t sup="<<it->sup
      //      << " c t="<<current_time <<"\n";
      if (it->inf == -1. || it->sup == -1. ||   // "all" time
          ( current_time >= it->inf && current_time <= it->sup)) { // in the range
        bool status=true;
        if (it->offset > 0.) {     // there is an offset time 
          if (it->lastt != 666.) { // it's not the first stime
            //std::cerr << std::setprecision(9) << "ct = " << current_time << " l+o = " <<it->lastt+it->offset<<" fabsdiff = "<< fabs(current_time-it->lastt-it->offset)<<" offset = "<<it->offset<<"\n"; 
            if ( current_time >= (it->lastt+it->offset) || 
                 diffTime(current_time-it->lastt-it->offset)) { // time match!!
              it->lastt = current_time; // save current time
            } else { // time does not match
              status=false;
            }
            
          } else {                 // first time
            it->lastt = current_time; // save current time
          }
        }
        return status;;
      }
    }
    return false;
  }
  // ============================================================================
  // shift
  // shift Positions or Velocities
  bool CSnapshotInterfaceIn::shift(std::string name,const float x, const float y, const float z)
  {
    float * data;
    if (name!="pos" && name!="vel") {
      std::cerr << "Error CSnapshotInterfaceIn::shift, name=["<<name<<"] sould be \"pos\" or \"vel\" \n"
          << "aborting....\n";
      std::exit(1);
    }
    int nbody;
    bool ok=getData("all",name,&nbody,&data);
    if (ok) {
      for (int i=0; i<nbody; i++) {        
        data[i*3+0] += x;
        data[i*3+1] += y;
        data[i*3+2] += z;        
      }
    }
    return ok;
  }

  // ============================================================================
  // getRangeSelect  
  // ============================================================================
  // getRangeSelect                                                              
  // return #bodies first and last particles for the selected particles in the   
  // selected range                                                              
  bool CSnapshotInterfaceIn::getRangeSelect(const char *  _comp, int * nbody, int * first,
                                            int * last, bool fortran )
  {
    std::string current_s,next_s;
    int offset=0;
    bool stop=false,status=false;
    *nbody=*first=*last=0;
    if (valid) {
      std::string comp = tools::Ctools::fixFortran(_comp);
      next_s = select_part; // sel_comp = user component requested
      // get CRV from user's selection reordered
      ComponentRangeVector * crv = getCrvFromSelection();//user_select.getCrvFromSelection();
      //ComponentRange::list(crv);
      if (crv->size()>0) { // it exists particles from the selection
        ComponentRange cr_match;
        //ComponentRange::list(crv);
        //std::exit(1);
        // loop on all user's selected component
        while ((current_s=uns::UserSelection::parseString(next_s)) != "" && !stop ) {
          // return the component's index belonging to the users' crv (last param=true)          
          int index=uns::ComponentRange::getIndexMatchType(crv,current_s,offset,true);
          //std::cerr << "index="<<index<<" current_s="<<current_s<<" comp="<<comp<<"\n";
          if (index>=0) { 
            if (current_s != comp) {
              if (current_s == "all") { // user req selected "all"
                // special case if user request "all"    
                // we must find if component exist in CRV of the snapshot
                ComponentRangeVector * crvs = getSnapshotRange(); // 30 march, commented out!!!
                //ComponentRange::list(crvs);
                assert(crvs);
                // return the component's index belonging to the SNAPSHOT'S crv
                int index=uns::ComponentRange::getIndexMatchType(crvs,comp,offset);
                if (index>=0) { // the component exist
                  *nbody=(*crvs)[index].n;
                  //offset+=(*crv)[index].n;
                  stop=true;
                  cr_match =(*crvs)[index];
                }
              }
#if 0	  
              else {
                offset+=(*crv)[index].n;
              }
#endif
            }
            else {  // current_s == comp
              *nbody=(*crv)[index].n;
              stop=true;
              cr_match=(*crv)[index];
            }
          }
        }
        if (stop) {
          status=true;
          int plusone;
          if (fortran) plusone=1;
          else         plusone=0;
#if 1
          *first = offset+plusone; 
          *last  = *first+(*nbody)-1;
#else
          *first = cr_match.first+ plusone;
          *last  = cr_match.last + plusone;
#endif
          if (verbose) {
            std::cerr << "CSnapshotInterfaceIn::getRangeSelect Component ["<<comp<<"]:\n"
                << std::setw(10) << std::left<< "nbody" << "=" << *nbody << "\n"
                << std::setw(10) << std::left<< "first" << "=" << *first << "\n"
                << std::setw(10) << std::left<< "last"  << "=" << *last  << "\n";}
        }
      }
    }
    return status;
  }
    
} // end of namespace uns
// ============================================================================
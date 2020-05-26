#ifndef NTUPLEINTERFACESTAGE2_H
#define NTUPLEINTERFACESTAGE2_H

#include "UserCode/proc/interface/sumup_loop.h"
#include "TTree.h"

/* the objects here are the same as in the sumup_loop interface
   -- it is the stage2 implementation of the sumup_loop interface
 */

//T_known_procs_info    known_procs_info_stage2;
//T_known_defs_channels known_defs_channels_stage2;
//T_known_systematics   known_systematics_stage2;
//T_known_defs_distrs   known_defs_distrs_stage2;

//T_known_normalization_per_somename known_normalization_per_syst_stage2;
//T_known_normalization_per_somename known_normalization_per_proc_stage2;
//T_known_normalization_per_somename known_normalization_per_chan_stage2;

T_known_procs_info    create_known_procs_info_stage2(void);
T_known_defs_distrs   create_known__TH1D_histo_definitions_stage2(void);
T_known_defs_channels create_known_channel_definitions_stage2(void);
T_known_systematics   create_known_systematics_stage2(void);

T_known_normalization_per_somename create_known_normalization_per_syst_stage2(void);
T_known_normalization_per_somename create_known_normalization_per_proc_stage2(void);
T_known_normalization_per_somename create_known_normalization_per_chan_stage2(void);


//F_connect_ntuple_interface connect_ntuple_interface_stage2;
int connect_ntuple_interface_stage2(TTree*);

//extern Int_t NT_nup;

#endif /* NTUPLEINTERFACESTAGE2_H */

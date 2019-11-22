import argparse
import logging
from array import array


# notice! ROOT takes over argv and prints its own help message when called from command line!
# instead I want the help message for my script
# therefore, check first if you are running from the command line
# and setup the argparser before ROOT cuts in

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class = argparse.RawDescriptionHelpFormatter,
        description = "printout stage2 interface, for C or for Python",
        epilog = "Example:\n$ python stage2_interface.py c"
        )

    parser.add_argument("lang", type=str, help="c or python, case-insensitive")
    parser.add_argument("-d", "--debug",    action='store_true', help="DEBUG level of logging")

    args = parser.parse_args()

    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    logging.debug("parsed args: %s" % repr(args))

else:
    # the module is imported in Python
    # to connect to the interface
    # setup root
    import ROOT
    from ROOT.Math import LorentzVector

    # lor vector for selected objects (particles and met)
    LorentzVector_Class = LorentzVector('ROOT::Math::PxPyPzE4D<double>')
    # vector of these for particles
    ROOT.gROOT.ProcessLine("typedef std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >> LorentzVectorS;")
    ROOT.gROOT.ProcessLine("typedef std::vector<int> IntVector;")
    ROOT.gROOT.ProcessLine("typedef std::vector<double> DoubleVector;")

    all_vector_branches = [] # for resetting them on each event


# procedures to meta-program the definitions
# each type has a Python exec and a C Macro definition

stage2_ttree_out = None # assign this variable to your TTree
language_difinitions_vars = {
# check if capital letters matter [Ll]?
'L': ("{varname} = array('L', [0]) ; stage2_ttree_out.Branch('{varname}', {varname}, '{varname}/l')", "ULong64_t_in_NTuple (OUTNTUPLE, {varname})"),
'i': ("{varname} = array('i', [0]) ; stage2_ttree_out.Branch('{varname}', {varname}, '{varname}/I')", "Int_t_in_NTuple     (OUTNTUPLE, {varname})"),
'f': ("{varname} = array('f', [0]) ; stage2_ttree_out.Branch('{varname}', {varname}, '{varname}/f')", "Float_t_in_NTuple   (OUTNTUPLE, {varname})"),
'LorentzVector_Class':  ("{varname} = LorentzVector_Class(0., 0., 0., 0.) ; stage2_ttree_out.Branch('{varname}', {varname})", "OBJECT_in_NTuple(OUTNTUPLE, {varname} , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)"),
'BoolVector':     ("{varname} = ROOT.BoolVector()     ; stage2_ttree_out.Branch('{varname}', {varname}) ; all_vector_branches.append({varname})", "VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Bool_t,  {varname})"),
'IntVector':      ("{varname} = ROOT.IntVector()      ; stage2_ttree_out.Branch('{varname}', {varname}) ; all_vector_branches.append({varname})", "VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   {varname})"),
'DoubleVector':   ("{varname} = ROOT.DoubleVector()   ; stage2_ttree_out.Branch('{varname}', {varname}) ; all_vector_branches.append({varname})", "VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, {varname})"),
'LorentzVectorS': ("{varname} = ROOT.LorentzVectorS() ; stage2_ttree_out.Branch('{varname}', {varname}) ; all_vector_branches.append({varname})", "VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, {varname}, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)"),
}

language_difinitions_in_c_macro = """
// default name of the output
#ifndef OUTNTUPLE
	#define OUTNTUPLE NT_output_ttree
#endif

#ifndef COMMA
#define COMMA ,
#endif

// another way of passing a comma, since in CMSSW94 the COMMA does not work anymore
#define UNPACK( ... ) __VA_ARGS__

#define STRINGIFY(x) #x
#define UNPACKS( ... ) #__VA_ARGS__

#ifndef DEFAULT_PARAMETER
#define DEFAULT_PARAMETER -111
#endif

/* macro declaring the object and setting a branch with its' pointer --- all in current, __not_global__ space (in main space)
 *
 * Notice the protocol:
 *    1) the object name in current namespace is `NT_Name`
 *    2) the branch name in the ntuple is `Name`
 */

/*
 * example to open ntuple interface in a TTree

	#define NTUPLE_INTERFACE_OPEN
	#include "UserCode/ttbar-leptons-80X/interface/ntupleOutput.h"

 */

#if defined(NTUPLE_INTERFACE_CLASS_DECLARE) // ALSO: in EDM Classes NTuple is pointer to TFile Service!
	// to declare vector of types (int/float etc): declate just vector
	#define VECTOR_PARAMs_in_NTuple(NTuple, TYPE, Name)   std::vector<TYPE> NT_##Name;
	// to declare vector of objects: declate vector and a pointer to it
	#define VECTOR_OBJECTs_in_NTuple(NTuple, Name, ...)   __VA_ARGS__ NT_##Name; __VA_ARGS__* pt_NT_##Name;
	// objects and types (simple parameters)
	#define OBJECT_in_NTuple(NTuple, Name, ...)     __VA_ARGS__   NT_##Name;
	#define Float_t_in_NTuple(NTuple, Name)         Float_t NT_##Name;
	#define Int_t_in_NTuple(NTuple, Name)           Int_t   NT_##Name;
	#define ULong64_t_in_NTuple(NTuple, Name)       ULong64_t   NT_##Name;
	#define Bool_t_in_NTuple(NTuple, Name)          Bool_t  NT_##Name;

#elif defined(NTUPLE_INTERFACE_CLASS_INITIALIZE)
	// hook up branch
	#define VECTOR_PARAMs_in_NTuple(NTuple, TYPE, Name)   NTuple->Branch(#Name, &NT_##Name);
	// hook up pointer for vectors of objects
	#define VECTOR_OBJECTs_in_NTuple(NTuple, Name, ...)   pt_NT_##Name = & NT_##Name; NTuple->Branch(#Name, #__VA_ARGS__, &pt_NT_##Name);
	// handle possible commas
	//#define VECTOR_OBJECTs_in_NTuple(NTuple, VECTOR_CLASS_MACRO, Name)   pt_NT_##Name = & NT_##Name; NTuple->Branch(#Name, VECTOR_CLASS_MACRO, &pt_NT_##Name);
	// objects and types (simple parameters)
	#define OBJECT_in_NTuple(NTuple, Name, ...)     NTuple->Branch(#Name, #__VA_ARGS__, &NT_##Name);
	#define Float_t_in_NTuple(NTuple, Name)         NTuple->Branch(#Name, &NT_##Name, #Name "/F");
	#define Int_t_in_NTuple(NTuple, Name)           NTuple->Branch(#Name, &NT_##Name, #Name "/I");
	#define ULong64_t_in_NTuple(NTuple, Name)       NTuple->Branch(#Name, &NT_##Name, #Name "/l");
	#define Bool_t_in_NTuple(NTuple, Name)          NTuple->Branch(#Name, &NT_##Name, #Name "/O");
#elif defined(NTUPLE_INTERFACE_CLASS_RESET)
	#define VECTOR_PARAMs_in_NTuple(NTuple, TYPE, Name)            NT_##Name.clear();
	#define VECTOR_OBJECTs_in_NTuple(NTuple, Name, ...)   NT_##Name.clear();
	// objects and types (simple parameters)
	//#define OBJECT_in_NTuple(NTuple, CLASS, Name)   CLASS   NT_##Name; NTuple.Branch(#Name, #CLASS, &NT_##Name);
	#define OBJECT_in_NTuple(NTuple, Name, ...)
	// WARNING: you'll have to reset your object yourself!
	// and these are defaults:
	#define Float_t_in_NTuple(NTuple, Name)         NT_##Name = DEFAULT_PARAMETER ;
	#define Int_t_in_NTuple(NTuple, Name)           NT_##Name = DEFAULT_PARAMETER ;
	#define ULong64_t_in_NTuple(NTuple, Name)       NT_##Name = DEFAULT_PARAMETER ;
	#define Bool_t_in_NTuple(NTuple, Name)          NT_##Name = false;
// ok, the classes should work
// the following are outdated at the moment:
#elif defined(NTUPLE_INTERFACE_CREATE)
	#define VECTOR_PARAMs_in_NTuple(NTuple, TYPE, Name)   std::vector<TYPE> NT_##Name; NTuple.Branch(#Name, &NT_##Name);
	#define VECTOR_OBJECTs_in_NTuple(NTuple, Name, ...)   __VA_ARGS__ NT_##Name; __VA_ARGS__* pt_NT_##Name ; NTuple.Branch(#Name, #__VA_ARGS__, &pt_NT_##Name);
	#define OBJECT_in_NTuple(NTuple, Name, ...)     __VA_ARGS__   NT_##Name; NTuple.Branch(#Name, #__VA_ARGS__, &NT_##Name);
	#define Float_t_in_NTuple(NTuple, Name)         Float_t NT_##Name; NTuple.Branch(#Name, &NT_##Name, #Name "/F");
	#define Int_t_in_NTuple(NTuple, Name)           Int_t   NT_##Name; NTuple.Branch(#Name, &NT_##Name, #Name "/I");
	#define ULong64_t_in_NTuple(NTuple, Name)       ULong64_t   NT_##Name; NTuple.Branch(#Name, &NT_##Name, #Name "/l");
	#define Bool_t_in_NTuple(NTuple, Name)          Bool_t  NT_##Name; NTuple.Branch(#Name, &NT_##Name, #Name "/O");

#elif defined(NTUPLE_INTERFACE_OPEN)
	#define VECTOR_PARAMs_in_NTuple(NTuple, TYPE, Name)   std::vector<TYPE> NT_##Name; std::vector<TYPE>* pt_NT_##Name = &NT_##Name; NTuple->SetBranchAddress(#Name, &pt_NT_##Name);
	#define VECTOR_OBJECTs_in_NTuple(NTuple, Name, ...)   __VA_ARGS__ NT_##Name; __VA_ARGS__* pt_NT_##Name = &NT_##Name ; NTuple->SetBranchAddress(#Name, &pt_NT_##Name);
	#define OBJECT_in_NTuple(NTuple, Name, ...)     __VA_ARGS__ NT_##Name; __VA_ARGS__*  pt_NT_##Name = 0; NTuple->SetBranchAddress(#Name, &pt_NT_##Name);
	#define PARAMETER_in_NTuple(NTuple, TYPE, Name)  TYPE   NT_##Name; NTuple->SetBranchAddress(#Name, &NT_##Name);
	#define Float_t_in_NTuple(NTuple, Name)         PARAMETER_in_NTuple(NTuple, Float_t, Name);
	#define Int_t_in_NTuple(NTuple, Name)           PARAMETER_in_NTuple(NTuple, Int_t, Name);
	#define ULong64_t_in_NTuple(NTuple, Name)       PARAMETER_in_NTuple(NTuple, ULong64_t, Name);
	#define Bool_t_in_NTuple(NTuple, Name)          PARAMETER_in_NTuple(NTuple, Bool_t, Name);

#elif defined(NTUPLE_INTERFACE_CONNECT)
	#define VECTOR_PARAMs_in_NTuple(NTuple, TYPE, Name)   std::vector<TYPE>* pt_NT_##Name = &NT_##Name; NTuple->SetBranchAddress(#Name, &pt_NT_##Name);
	#define VECTOR_OBJECTs_in_NTuple(NTuple, Name, ...)   __VA_ARGS__* pt_NT_##Name = &NT_##Name ; NTuple->SetBranchAddress(#Name, &pt_NT_##Name);
	#define OBJECT_in_NTuple(NTuple, Name, ...)     __VA_ARGS__*  pt_NT_##Name = 0; NTuple->SetBranchAddress(#Name, &pt_NT_##Name);
	#define PARAMETER_in_NTuple(NTuple, TYPE, Name) NTuple->SetBranchAddress(#Name, &NT_##Name);
	#define Float_t_in_NTuple(NTuple, Name)         PARAMETER_in_NTuple(NTuple, Float_t, Name);
	#define Int_t_in_NTuple(NTuple, Name)           PARAMETER_in_NTuple(NTuple, Int_t, Name);
	#define ULong64_t_in_NTuple(NTuple, Name)       PARAMETER_in_NTuple(NTuple, ULong64_t, Name);
	#define Bool_t_in_NTuple(NTuple, Name)          PARAMETER_in_NTuple(NTuple, Bool_t, Name);

#else
	error: set ntuple interface mode
#endif


#ifndef NTUPLEOUTPUT_LORENTZVECTOR_H
#define NTUPLEOUTPUT_LORENTZVECTOR_H
// the exact LorentzVector declaration
typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > LorentzVector;
typedef ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double>, ROOT::Math::DefaultCoordinateSystemTag> Vector_3D;
// typedef ROOT::Math::SMatrix<double,3,3,ROOT::Math::MatRepSym<double,3>>  Error_3D; // not used in stage2 and crashes in new CMSSW for some reason?
#endif /* NTUPLEOUTPUT_LORENTZVECTOR_H */

{variable_definitions}

// undefine all macro
#undef NTUPLE_INTERFACE_OPEN
#undef NTUPLE_INTERFACE_CREATE
#undef NTUPLE_INTERFACE_CONNECT
#undef NTUPLE_INTERFACE_CLASS_DECLARE
#undef NTUPLE_INTERFACE_CLASS_INITIALIZE
#undef NTUPLE_INTERFACE_CLASS_RESET

#undef VECTOR_PARAMs_in_NTuple
#undef VECTOR_OBJECTs_in_NTuple
#undef OBJECT_in_NTuple
#undef Float_t_in_NTuple
#undef Int_t_in_NTuple
#undef ULong64_t_in_NTuple
#undef Bool_t_in_NTuple
"""


# definitions
# varname: type
stage2_vars = {
'indexevents': 'L',
'runNumber':   'i',
'lumi':        'i',

'gen_proc_id' :  'i',

'selection_stage' :  'i',
'selection_stage_presel' :  'i',

'selection_stage_TESUp' :  'i',
'selection_stage_TESDown' :  'i',

'selection_stage_JESUp' :  'i',
'selection_stage_JESDown' :  'i',

'selection_stage_JERUp' :  'i',
'selection_stage_JERDown' :  'i',

'selection_stage_tt_alliso' :  'i',
'selection_stage_wjets' :  'i',

'selection_stage_dy'         : 'i',
'selection_stage_dy_TESUp'   : 'i',
'selection_stage_dy_TESDown' : 'i',
'selection_stage_dy_JERUp'   : 'i',
'selection_stage_dy_JERDown' : 'i',
'selection_stage_dy_JESUp'   : 'i',
'selection_stage_dy_JESDown' : 'i',

'selection_stage_dy_elmu'         : 'i',
'selection_stage_dy_elmu_JERUp'   : 'i',
'selection_stage_dy_elmu_JERDown' : 'i',
'selection_stage_dy_elmu_JESUp'   : 'i',
'selection_stage_dy_elmu_JESDown' : 'i',

'selection_stage_dy_mumu'         : 'i',
'selection_stage_dy_mumu_JERUp'   : 'i',
'selection_stage_dy_mumu_JERDown' : 'i',
'selection_stage_dy_mumu_JESUp'   : 'i',
'selection_stage_dy_mumu_JESDown' : 'i',

'selection_stage_em'         : 'i',
'selection_stage_em_JERUp'   : 'i',
'selection_stage_em_JERDown' : 'i',
'selection_stage_em_JESUp'   : 'i',
'selection_stage_em_JESDown' : 'i',

'nup' :  'i',
'amcatnlo_w' :  'f',

'event_jets_lj_var' :  'f',
'event_jets_lj_w_mass' :  'f',
'event_jets_lj_t_mass' :  'f',
'event_jets_input_has' :  'f',
'event_jets_found_has' :  'f',
'event_jets_n_jets' :  'i',
'event_jets_n_bjets' :  'i',
'event_jets_n_jets_lepmatched' :  'i',
'event_jets_alliso_n_jets' :  'i',
'event_jets_alliso_n_bjets' :  'i',
'event_weight' :  'f',
'event_weight_init' :  'f',
'event_weight_PU' :  'f',
'event_weight_PU_el' :  'f',
'event_weight_PU_mu' :  'f',
'event_weight_PU_bcdef' :  'f',
'event_weight_PU_gh' :  'f',
'event_weight_PU_per_epoch' :  'f',
'event_weight_th' :  'f',
'event_weight_bSF' :  'f',
'event_weight_bSFUp' :  'f',
'event_weight_bSFDown' :  'f',
'event_weight_bSF_JERUp' :  'f',
'event_weight_bSF_JERDown' :  'f',
'event_weight_bSF_JESUp' :  'f',
'event_weight_bSF_JESDown' :  'f',
'event_weight_toppt' :  'f',
'event_weight_LEPall' :  'f',

    'event_weight_LEPmuID'      :  'f',
    'event_weight_LEPmuID_Up'   :  'f',
    'event_weight_LEPmuID_Down' :  'f',
    'event_weight_LEPmuTRG'      :  'f',
    'event_weight_LEPmuTRG_Up'   :  'f',
    'event_weight_LEPmuTRG_Down' :  'f',
    'event_weight_LEPelID'      :  'f',
    'event_weight_LEPelID_Up'   :  'f',
    'event_weight_LEPelID_Down' :  'f',
    'event_weight_LEPelTRG'      :  'f',
    'event_weight_LEPelTRG_Up'   :  'f',
    'event_weight_LEPelTRG_Down' :  'f',

'event_weight_PUUp' :  'f',
'event_weight_PUDown' :  'f',
'event_weight_PUUp_el' :  'f',
'event_weight_PUDown_el' :  'f',
'event_weight_PUUp_mu' :  'f',
'event_weight_PUDown_mu' :  'f',
'event_weight_PUUp_bcdef' :  'f',
'event_weight_PUDown_bcdef' :  'f',
'event_weight_PUUp_gh' :  'f',
'event_weight_PUDown_gh' :  'f',
'event_weight_PetersonUp' :  'f',
'event_weight_FragUp' :  'f',
'event_weight_FragDown' :  'f',
'event_weight_SemilepBRUp' :  'f',
'event_weight_SemilepBRDown' :  'f',
'event_weight_me_f_rUp' :  'f',
'event_weight_me_f_rDn' :  'f',
'event_weight_me_fUp_r' :  'f',
'event_weight_me_fDn_r' :  'f',
'event_weight_me_frUp' :  'f',
'event_weight_me_frDn' :  'f',
'event_weight_AlphaS_up' :  'f',
'event_weight_AlphaS_dn' :  'f',
'event_weight_pdf' : 'DoubleVector',

'event_met'         : 'LorentzVector_Class',
'event_met_JERUp'   : 'LorentzVector_Class',
'event_met_JERDown' : 'LorentzVector_Class',
'event_met_JESUp'   : 'LorentzVector_Class',
'event_met_JESDown' : 'LorentzVector_Class',
'event_met_TESUp'   : 'LorentzVector_Class',
'event_met_TESDown' : 'LorentzVector_Class',
'event_met_init'    : 'LorentzVector_Class',
'event_met_init2'   : 'LorentzVector_Class',

'event_met_lep_mt_init'  : 'f',
'event_met_lep_mt_init2' : 'f',
'event_met_lep_mt' : 'f',
'event_met_lep_mt_JERUp'   : 'f',
'event_met_lep_mt_JERDown' : 'f',
'event_met_lep_mt_JESUp'   : 'f',
'event_met_lep_mt_JESDown' : 'f',
'event_met_lep_mt_TESUp'   : 'f',
'event_met_lep_mt_TESDown' : 'f',
'event_dilep_mass' : 'f',

'event_top_masses_medium' : 'DoubleVector',
'event_top_masses_loose' : 'DoubleVector',
'event_top_masses_closest' :  'f',

'event_leptons' : 'LorentzVectorS',
'event_leptons_ids' : 'IntVector',
'event_leptons_dxy' : 'DoubleVector',
'event_leptons_genmatch' : 'IntVector',
'event_leptons_alliso_reliso' : 'DoubleVector',
'event_leptons_alliso_p4' : 'LorentzVectorS',
'event_leptons_alliso_pdgId' : 'IntVector',
'event_taus_alliso_p4' : 'LorentzVectorS',
'event_taus_alliso_pdgId' : 'IntVector',
'event_taus_alliso_IDlev' : 'IntVector',
'event_candidate_taus' : 'LorentzVectorS',
'event_candidate_taus_ids' : 'IntVector',
'event_taus' : 'LorentzVectorS',

'event_taus_SF_VLoose'      : 'DoubleVector',
'event_taus_SF_VLoose_Up'   : 'DoubleVector',
'event_taus_SF_VLoose_Down' : 'DoubleVector',

'event_taus_SF_Medium'      : 'DoubleVector',
'event_taus_SF_Medium_Up'   : 'DoubleVector',
'event_taus_SF_Medium_Down' : 'DoubleVector',

'event_taus_SF_Tight'       : 'DoubleVector',
'event_taus_SF_Tight_Up'    : 'DoubleVector',
'event_taus_SF_Tight_Down'  : 'DoubleVector',

'event_taus_ids' : 'IntVector',
'event_taus_dms' : 'IntVector',
'event_taus_IDlev' : 'IntVector',
'event_taus_TES_up' : 'DoubleVector',
'event_taus_TES_down' : 'DoubleVector',

'event_taus_genmatch' : 'IntVector',
'event_taus_pat_sv_sign' : 'DoubleVector',
'event_taus_pat_sv_leng' : 'DoubleVector',
'event_taus_sv_sign' : 'DoubleVector',
'event_taus_sv_leng' : 'DoubleVector',
'event_taus_sv_dalitz_m1' : 'DoubleVector',
'event_taus_sv_dalitz_m2' : 'DoubleVector',
'event_taus_track_energy' : 'DoubleVector',
'event_taus_jet_bdiscr' : 'DoubleVector',
'event_taus_l' : 'LorentzVectorS',
'event_jets_b' : 'LorentzVectorS',
'event_jets_b_bdiscr' : 'DoubleVector',
'event_jets_JERUp' : 'DoubleVector',
'event_jets_JERDown' : 'DoubleVector',
'event_jets_JESUp' : 'DoubleVector',
'event_jets_JESDown' : 'DoubleVector',
'event_jets_b_genmatch' : 'IntVector',
'event_jets_b_bSFweight' : 'DoubleVector',
'event_jets_r' : 'LorentzVectorS',
'event_jets_r_bdiscr' : 'DoubleVector',
'event_jets_r_genmatch' : 'IntVector',
'event_jets_r_bSFweight' : 'DoubleVector',
'event_jets_l' : 'LorentzVectorS',
'event_jets_l_bdiscr' : 'DoubleVector',
'event_jets_l_genmatch' : 'IntVector',
'event_jets_l_bSFweight' : 'DoubleVector',
'event_jets_t' : 'LorentzVectorS',
'event_jets_t_bdiscr' : 'DoubleVector',
'event_jets_t_genmatch' : 'IntVector',
'event_jets_t_bSFweight' : 'DoubleVector',
}

'''
use this in a loop with exec-s to define the interface in your namespace

for varname, vartype in stage2_vars.items():
    vardef_py = language_difinitions_vars[vartype][0].format(varname=varname)
    #exec(vardef_py)
    exec vardef_py in globals(), locals()

As an example, indexevents, runNumber, lumi, all 'i' type, lead to an execution of this:

#ttree_out = TTree( 'ttree_out', 'tree with stage2 selection' ) # INPUT NOW
indexevents  = array( 'L', [ 0 ] )
ttree_out.Branch( 'indexevents', indexevents, 'indexevents/l' )
runNumber  = array( 'i', [ 0 ] )
ttree_out.Branch( 'runNumber', runNumber, 'runNumber/I' )
lumi  = array( 'i', [ 0 ] )
ttree_out.Branch( 'lumi', lumi, 'lumi/I' )
...
'''

def stage2_interface_define_here_def():
    logging.info("stage2_interface:define_interface_here")

    for varname, vartype in stage2_vars.items():
        vardef_py = language_difinitions_vars[vartype][0].format(varname=varname)
        #exec(vardef_py)
        exec vardef_py in globals(), locals()

    logging.info("stage2_interface:define_interface_here DONE")

stage2_interface_define_here = '''
logging.info("stage2_interface:define_interface_here")
for varname, vartype in stage2_vars.items():
    vardef_py = language_difinitions_vars[vartype][0].format(varname=varname)
    #exec(vardef_py)
    exec vardef_py in globals(), locals()
logging.info("stage2_interface:define_interface_here DONE")
'''



if __name__ == '__main__':
    # commandline command to printout interface definitions

    logging.info("printout")

    if args.lang.lower() == 'c':
        #
        logging.info("printout:C")
        vardefs = []

        for varname, vartype in stage2_vars.items():
            vardef_c = language_difinitions_vars[vartype][1].format(varname=varname)
            #exec vardef_py in globals(), locals()
            vardefs.append(vardef_c)

        print language_difinitions_in_c_macro.format(variable_definitions='\n'.join(vardefs))

    elif args.lang.lower() == 'python':
        logging.info("printout:Python")
        for varname, vartype in stage2_vars.items():
            vardef_py = language_difinitions_vars[vartype][0].format(varname=varname)
            #exec vardef_py in globals(), locals()
            print vardef_py

    logging.info("printout DONE")


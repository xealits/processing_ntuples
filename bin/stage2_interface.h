
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

Float_t_in_NTuple   (OUTNTUPLE, amcatnlo_w)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_TESUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_leptons_genmatch)
Int_t_in_NTuple     (OUTNTUPLE, runNumber)
OBJECT_in_NTuple(OUTNTUPLE, event_met_JERDown , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_bSFDown)
Int_t_in_NTuple     (OUTNTUPLE, event_jets_n_bjets)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_taus_ids)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_TES_up)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_elmu_JESUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_toppt)
OBJECT_in_NTuple(OUTNTUPLE, event_met_init2 , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_init)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUUp_el)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_taus_IDlev)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_Tight_Down)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt_JERUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUUp)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_elmu_JERDown)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_TESUp)
Int_t_in_NTuple     (OUTNTUPLE, nup)
OBJECT_in_NTuple(OUTNTUPLE, event_met_JERUp , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPelTRG_Down)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_VLoose)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPmuID_Up)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_Medium)
Float_t_in_NTuple   (OUTNTUPLE, event_estim_w_mass)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPmuTRG_Down)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_sv_leng)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PU_gh)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_mm_JESDown)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPmuID)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_TESDown)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_taus_alliso_IDlev)
OBJECT_in_NTuple(OUTNTUPLE, event_met_init , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_mm)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_JERUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_FragDown)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_VLoose_Up)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt_JESDown)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUUp_gh)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_leptons_alliso_reliso)
Float_t_in_NTuple   (OUTNTUPLE, event_dilep_mass)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_jets_b, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_AlphaS_dn)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_leptons_dxy)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_jets_l, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PU_per_epoch)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt_JESUp)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_jets_t, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_jets_t_genmatch)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_elmu)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_JESDown)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_top_masses_loose)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PU_bcdef)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt_TESUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPmuTRG_Up)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_t_bdiscr)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PU_el)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUDown)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_em_JERUp)
OBJECT_in_NTuple(OUTNTUPLE, event_met_JESDown , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_JESUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_Tight_Up)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_jets_l_genmatch)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_em)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_JESUp)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_mumu_JESDown)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPelTRG_Up)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_mumu_JERDown)
Float_t_in_NTuple   (OUTNTUPLE, event_jets_input_has)
Int_t_in_NTuple     (OUTNTUPLE, event_jets_n_jets)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_JERDown)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_jets_r_genmatch)
Int_t_in_NTuple     (OUTNTUPLE, event_jets_alliso_n_bjets)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_mm_JESUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUUp_bcdef)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PU)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_JERUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_taus_dms)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_candidate_taus_ids)
Float_t_in_NTuple   (OUTNTUPLE, event_top_masses_closest)
Int_t_in_NTuple     (OUTNTUPLE, event_jets_n_jets_lepmatched)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_em_JERDown)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_jets_r, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
Int_t_in_NTuple     (OUTNTUPLE, event_jets_alliso_n_jets)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_SemilepBRDown)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_Medium_Up)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_t_bSFweight)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_b_bdiscr)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_me_f_rUp)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_em_JESDown)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_wjets)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_th)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_top_masses_medium)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_JESDown)
Float_t_in_NTuple   (OUTNTUPLE, event_jets_lj_t_mass)
Int_t_in_NTuple     (OUTNTUPLE, lumi)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_sv_sign)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUDown_mu)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPall)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_me_frDn)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_jet_bdiscr)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_taus_alliso_p4, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_mm_JERUp)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_elmu_JERUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_l_bSFweight)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_mumu_JERUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPelID_Down)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_weight_pdf)
OBJECT_in_NTuple(OUTNTUPLE, event_met_JESUp , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_JERDown)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_leptons, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
OBJECT_in_NTuple(OUTNTUPLE, event_met_TESUp , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_presel)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_taus_l, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_r_bdiscr)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_JERDown)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_JESUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUUp_mu)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_leptons_alliso_p4, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_b_bSFweight)
OBJECT_in_NTuple(OUTNTUPLE, event_met_TESDown , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt_init2)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_jets_b_genmatch)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_leptons_ids)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_taus_alliso_pdgId)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPmuID_Down)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_candidate_taus, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
ULong64_t_in_NTuple (OUTNTUPLE, indexevents)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_mumu_JESUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_pat_sv_sign)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUDown_bcdef)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_taus_genmatch)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_AlphaS_up)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_Tight)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_bSF_JERUp)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_TESDown)
VECTOR_OBJECTs_in_NTuple (OUTNTUPLE, event_taus, std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >>)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_r_bSFweight)
Float_t_in_NTuple   (OUTNTUPLE, event_jets_lj_var)
Int_t_in_NTuple     (OUTNTUPLE, gen_proc_id)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPelID)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PU_mu)
Float_t_in_NTuple   (OUTNTUPLE, event_jets_found_has)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_JESDown)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPelID_Up)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPmuTRG)
Float_t_in_NTuple   (OUTNTUPLE, event_weight)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_me_frUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_jets_l_bdiscr)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_VLoose_Down)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUDown_gh)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_bSFUp)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_mm_JERDown)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_me_fUp_r)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_bSF)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt_init)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt_TESDown)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_elmu_JESDown)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_FragUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_bSF_JESUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_TES_down)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_SF_Medium_Down)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_sv_dalitz_m2)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PetersonUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_sv_dalitz_m1)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Int_t,   event_leptons_alliso_pdgId)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_pat_sv_leng)
OBJECT_in_NTuple(OUTNTUPLE, event_met , ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_me_fDn_r)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_bSF_JESDown)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_em_JESUp)
VECTOR_PARAMs_in_NTuple  (OUTNTUPLE, Float_t, event_taus_track_energy)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_me_f_rDn)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_JERUp)
Float_t_in_NTuple   (OUTNTUPLE, event_jets_lj_w_mass)
Float_t_in_NTuple   (OUTNTUPLE, event_met_lep_mt_JERDown)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_PUDown_el)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_dy_mumu)
Int_t_in_NTuple     (OUTNTUPLE, selection_stage_tt_alliso)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_SemilepBRUp)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_LEPelTRG)
Float_t_in_NTuple   (OUTNTUPLE, event_weight_bSF_JERDown)

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


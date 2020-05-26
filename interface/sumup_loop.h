#ifndef SUMUPLOOP_H
#define SUMUPLOOP_H

/** the interface to the sumup_loop process
 */

//#include "TH1F.h"
//#include "TH2F.h"
//#include "TH3F.h"

#include "TROOT.h"
//#include "TString.h"
#include "TH1D.h"
#include "TTree.h"

#include <map>
#include <vector>

using namespace std;

/* --------------------------------------------------------------- */
// helper functions for gdb
// substitute TStrings with standard supported string everywhere

const char* TString_Data (TString& str) {return str.Data();}
const char* TH1D_Name    (TH1D& hist) {return hist.GetName();}

/* --------------------------------------------------------------- */



/**
\ingroup Enumerations
\brief The identifiers for all object-related systematics.

The systematic corrections to objects usually change the energy of the object (not the direction),
get propagated to MET if needed,
and recompute all corresponding `stage_selection` parameters.
The object systematics are:
\c JERUp, \c JERDown,
\c JESUp, \c JESDown,
\c TESUp, \c TESDown.

The event-weight systematics are:
\c PUUp,  \c PUDown,
\c bSFUp, \c bSFDown,
and many more, especially many PDF weights.

The twofold nature of the effect of systematics, change in objects-event-selection and event weight,
calls for special treatment.
The event weights are just produced from an event, the same way as distributions.
But corrections to objects and correspondinly to event selection
are a bit different.
Therefore, setting a systematic propagates as input to the functions
calculating distributions and the functions calculating event selection.
At the same time, there are functions calculating event weights.
They are implemented in usual manner.

At the end, each systematic defines 3 things: "a name", enum ObjSys, weight function.
The "name" is given to the recorded histograms,
enum ObjSys sets the calculations of distrs and channels,
weight function defines the corrected event weight.
 */


enum ObjSystematics {NOMINAL /**< NOMINAL sysytematic corrections, event weights and object corrections */,
/* object-related systematics */
 JERUp   /**< jet energy resolution Up corrections are applied to jets. */,
 JERDown /**< jet energy resolution Down corrections are applied to jets */,
 JESUp   /**< jet energy scale Up corrections are applied to jets */,
 JESDown /**< jet energy scale Down corrections are applied to jets */,
 TESUp   /**< tau energy scale Up corrections are applied to taus */,
 TESDown /**< tau energy scale Down corrections are applied to taus */
};

typedef double (*_F_sysweight)();


/* the objects that define event records:

  the final state reconstructed channel   -- the event selection requirements, selection_stage from before etc
  the gen-level (sub-)process definitions -- like dilepton or semileptonic ttbar
  systematic
  distribution to record
 */


// ----- reco channel

/** \brief The channel-defining `bool` function.

Each final state channel is defined by a `bool` function,
that is calculated in the name space of the input `TTree` branches.
It takes the `ObjSystematics` identifier as input,
and returns `bool` whether event passes this channel selection or not.

\return bool
 */

typedef bool (*_F_channel_sel)(ObjSystematics);

/** \brief The full channel definition: the bool function and the nominal event weight.

 */

typedef struct {
	_F_channel_sel chan_sel;        /**< \brief the event selection function */
	_F_sysweight   chan_sel_weight; /**< \brief the nominal event weight */
} _S_chan_def;

typedef map<TString, _S_chan_def> T_known_defs_channels; // used in sumup_loop main


// ----- gen-level process

typedef bool (*_F_genproc_def)(void);

/** \brief The definition of (sub-)processes for a given dtag

Contains gen ID ranges for all possible sub-processes and the handy groups.
 */

typedef struct {
	TString catchall_name;    /**< \brief name for the inclusive process, or catch-all case */
	map<TString, _F_genproc_def> all;    /**< \brief all possible sub-processes */
	map<TString, _F_genproc_def> groups; /**< \brief groups of sub-processes */
	map<TString, vector<TString>> channel_standard; /**< \brief standard sub-processes per channel */
} _S_proc_ID_defs;



// ----- systematic

typedef struct {ObjSystematics obj_sys_id; _F_sysweight weight_func;} _S_systematic_definition;

typedef map<TString, _S_systematic_definition> T_known_systematics; // used in sumup_loop main

// ----- distribution

/** \brief The definition of TH1D ranges, linear and custom.

The custom range is defined by setting `.linear = false` and the pointer `.custom_bins` to an array.
I use \c static memory to set the arrays and pointers, like this:

    static double Mt_lep_met_c_bins[] = {0,16,32,44,54,64};
    r = {(sizeof(Mt_lep_met_c_bins) / sizeof(Mt_lep_met_c_bins[0]))-1, false,-1,  -1, Mt_lep_met_c_bins};

 */

typedef struct {
	unsigned int nbins;
	bool linear = true;
	double linear_min;
	double linear_max;
	double* custom_bins;
} _TH1D_histo_range;

/*
these do not really work
_TH1D_histo_range range_linear(unsigned int nbins, double min, double max)
	{
	return (_TH1D_histo_range) {nbins, true, min, max};
	}

_TH1D_histo_range range_custom_base(const double custom_bins[], unsigned int nbins)
	{
	//unsigned int nbins = sizeof(custom_bins) / sizeof(custom_bins[0]);
	return (_TH1D_histo_range) {nbins, false, -1, -1, {custom_bins}};
	}

//#define range_custom(...) range_custom_base(__VA_ARGS__, sizeof(__VA_ARGS__) / sizeof(__VA_ARGS__[0]))

#define set_range_linear(range_var, nbins, min, max) range_var = {(nbins), true,  (min), (max)}
#define set_range_custom(range_var, ...) unsigned int nbins = sizeof(__VA_ARGS__)/sizeof(__VA_ARGS__[0]) ; range_var = {(nbins), false, -1, -1, __VA_ARGS__}
*/

/** \brief The definition of an output histogram

 The function calculating the parameter, and the range of the histogram to store it.
 */

typedef struct {
	double (*func)(ObjSystematics);
	_TH1D_histo_range range;
} _TH1D_histo_def;

typedef map<TString, _TH1D_histo_def> T_known_defs_distrs; // used in sumup_loop main


// 'tt'  : (['MC2016_Summer16_TTJets_powheg'],  ["nom,common", "obj", "tt_weights", "tt_hard", "tt_pdf1", "tt_pdf10", "tt_pdf20", "tt_pdf30", "tt_pdf40", "tt_pdf50,tt_alpha"]), #select_sparse_channels

// standard systematics

#define SYSTS_OBJECTS  "JERDown", "JERUp"  , "JESDown", "JESUp"  , "TESDown", "TESUp"
#define SYSTS_COMMON   "TAUIDUp", "TAUIDDown", "PUUp", "PUDown", "bSFUp", "bSFDown", "LEPelIDUp", "LEPelIDDown", "LEPelTRGUp", "LEPelTRGDown", "LEPmuIDUp", "LEPmuIDDown", "LEPmuTRGUp", "LEPmuTRGDown"
#define SYSTS_TT_OBJ   "TOPPTDown"     , "TOPPTUp"       , "FragUp"        , "FragDown"      , "SemilepBRUp"   , "SemilepBRDown" , "PetersonUp"    , "PetersonDown"
#define SYSTS_TT_HARD  "MrUp"     , "MrDown"  , "MfUp"    , "MfDown"  , "MfrUp"   , "MfrDown"
#define SYSTS_TT_ALPHA "AlphaSUp" , "AlphaSDown"

#define SYSTS_TT_PDF1  "PDFCT14n1Up" , "PDFCT14n2Up" , "PDFCT14n3Up" , "PDFCT14n4Up" , "PDFCT14n5Up" , "PDFCT14n6Up" , "PDFCT14n7Up" , "PDFCT14n8Up" , "PDFCT14n9Up" , "PDFCT14n10Up"
#define SYSTS_TT_PDF10 "PDFCT14n11Up", "PDFCT14n12Up", "PDFCT14n13Up", "PDFCT14n14Up", "PDFCT14n15Up", "PDFCT14n16Up", "PDFCT14n17Up", "PDFCT14n18Up", "PDFCT14n19Up", "PDFCT14n20Up"
#define SYSTS_TT_PDF20 "PDFCT14n21Up", "PDFCT14n22Up", "PDFCT14n23Up", "PDFCT14n24Up", "PDFCT14n25Up", "PDFCT14n26Up", "PDFCT14n27Up", "PDFCT14n28Up", "PDFCT14n29Up", "PDFCT14n30Up"
#define SYSTS_TT_PDF30 "PDFCT14n31Up", "PDFCT14n32Up", "PDFCT14n33Up", "PDFCT14n34Up", "PDFCT14n35Up", "PDFCT14n36Up", "PDFCT14n37Up", "PDFCT14n38Up", "PDFCT14n39Up", "PDFCT14n40Up"
#define SYSTS_TT_PDF40 "PDFCT14n41Up", "PDFCT14n42Up", "PDFCT14n43Up", "PDFCT14n44Up", "PDFCT14n45Up", "PDFCT14n46Up", "PDFCT14n47Up", "PDFCT14n48Up", "PDFCT14n49Up", "PDFCT14n50Up"
#define SYSTS_TT_PDF50 "PDFCT14n51Up", "PDFCT14n52Up", "PDFCT14n53Up", "PDFCT14n54Up", "PDFCT14n55Up", "PDFCT14n56Up"

#define SYSTS_TT_PDFDown1  "PDFCT14n1Down" , "PDFCT14n2Down" , "PDFCT14n3Down" , "PDFCT14n4Down" , "PDFCT14n5Down" , "PDFCT14n6Down" , "PDFCT14n7Down" , "PDFCT14n8Down" , "PDFCT14n9Down" , "PDFCT14n10Down"
#define SYSTS_TT_PDFDown10 "PDFCT14n11Down", "PDFCT14n12Down", "PDFCT14n13Down", "PDFCT14n14Down", "PDFCT14n15Down", "PDFCT14n16Down", "PDFCT14n17Down", "PDFCT14n18Down", "PDFCT14n19Down", "PDFCT14n20Down"
#define SYSTS_TT_PDFDown20 "PDFCT14n21Down", "PDFCT14n22Down", "PDFCT14n23Down", "PDFCT14n24Down", "PDFCT14n25Down", "PDFCT14n26Down", "PDFCT14n27Down", "PDFCT14n28Down", "PDFCT14n29Down", "PDFCT14n30Down"
#define SYSTS_TT_PDFDown30 "PDFCT14n31Down", "PDFCT14n32Down", "PDFCT14n33Down", "PDFCT14n34Down", "PDFCT14n35Down", "PDFCT14n36Down", "PDFCT14n37Down", "PDFCT14n38Down", "PDFCT14n39Down", "PDFCT14n40Down"
#define SYSTS_TT_PDFDown40 "PDFCT14n41Down", "PDFCT14n42Down", "PDFCT14n43Down", "PDFCT14n44Down", "PDFCT14n45Down", "PDFCT14n46Down", "PDFCT14n47Down", "PDFCT14n48Down", "PDFCT14n49Down", "PDFCT14n50Down"
#define SYSTS_TT_PDFDown50 "PDFCT14n51Down", "PDFCT14n52Down", "PDFCT14n53Down", "PDFCT14n54Down", "PDFCT14n55Down", "PDFCT14n56Down"


#define SYSTS_TT_PDF       SYSTS_TT_PDF1, SYSTS_TT_PDF10, SYSTS_TT_PDF20, SYSTS_TT_PDF30, SYSTS_TT_PDF40, SYSTS_TT_PDF50
#define SYSTS_TT_PDFDown   SYSTS_TT_PDFDown1, SYSTS_TT_PDFDown10, SYSTS_TT_PDFDown20, SYSTS_TT_PDFDown30, SYSTS_TT_PDFDown40, SYSTS_TT_PDFDown50

#define SYSTS_TT       "NOMINAL", SYSTS_OBJECTS, SYSTS_COMMON, SYSTS_TT_OBJ, SYSTS_TT_HARD, SYSTS_TT_ALPHA, SYSTS_TT_PDF, SYSTS_TT_PDFDown
#define SYSTS_OTHER_MC "NOMINAL", SYSTS_OBJECTS, SYSTS_COMMON
#define SYSTS_QCD_MC   "NOMINAL"


/** gen-level process per dtag

there can be multiple per-ntuple-type implementations of this:
the same gen-level process is defined differently in original ntuples and in stage2...
therefore TODO: the known dtags must be produced in main on if (type of ntuples)
for now just link them directly with stage2 ntuples interface
 */
typedef map<TString, _S_proc_ID_defs> T_known_procs_info;
//extern T_known_procs_info known_procs_info_stage2;
/* extern means it must be defined elsewhere -- in ntuple_stage2.cpp
   let's just move this definition in ntuple_stage2 interface
 */

/*
map<TString, _S_proc_ID_defs> known_procs_info = create_known_procs_info(); <-- basically, this must be run in main like:

if      (stage2)
	T_known_procs_info known_procs_info = create_known_procs_info(known_procs_info_stage2);
else if (ntupler)
	T_known_procs_info known_procs_info = create_known_procs_info(known_procs_info_ntupler);
*/



// normalizations for MC datasets -- corrections of the overall generated event yield to keep it unchanged after all the MC callibrations
typedef map<TString, double>        T_known_normalization_per_somename;

typedef int (*F_connect_ntuple_interface)(TTree*);

#endif /* SUMUPLOOP_H */
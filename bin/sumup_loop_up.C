/**
\file sumup_loop_up.C
\mainpage The sumup_loop plotting routine.
\brief A flexible plotting program.

The program defines a set of objects (functions, struct-s, map-s, vectors etc)
to be used in the construction of the event loop in the `main` function.
These objects basically define the information about the input datasets,
distributions to record, systematics to apply etc.
They correspond to the old `std_defs.py` module.

The `main` function operates on the defined objects and consists of 3 parts:

1. parsing user's input and filling up the data structures operating the event loop
2. the event loop
3. application of final systematic corrections and normalizations to the recorded histograms and writing the output.

# Naming style

The beginning of the `typedef` names has 1 capital letter mnemonic for the defined type: S for struct, F for function, T for any other type.
The names starting with underscore, such as `_TH1D_histo_def` are not intendet to be used in `main` directly.
The collections of definitions for `main` start with `known_`, like `known_dtags_info`.
The corresponding functions defining these collections start with `create_known_`, like `create_known_dtags_info`.

Now, the names of the functions operating in the namespace of the interface of the TTree, i.e. on variables like `NT_event_leptons`,
start with `NT_` like `NT_distr_leading_lep_pt`.
In future we may try to separate them in an altogether different namespace,
with a separate function to connect an input TTree to that namespace etc.
 */

/*! 
    \defgroup Enumerations
        Public enumeration types
*/

#include <iostream>

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "THStack.h"
#include "TH1F.h"                                                                                                                                       
#include "TH2F.h"
#include "TH3F.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TEventList.h"
#include "TROOT.h"
#include "TNtuple.h"
#include <Math/VectorUtil.h>

#include <map>
#include <string>
#include <vector>
#include <stdlib.h> // abort

using namespace std;

#define cout_expr(Name)   cout << #Name << " = " << Name << std::endl
#define cerr_expr(Name)   cerr << #Name << " = " << Name << std::endl

/**
The mode of `Stopif` behavior: on a stop execute `error_action` or abort the program
Set this to \c 's' to stop the program on error. Otherwise execure error actions. */
char error_mode = 'a';

/** where to write errors? if this is \c NULL, write to \c stderr. */
FILE * error_log;

/** \brief Macro to handle exceptions: if assertion true print an error, then execute a user action or abort the program.

\param assertion any expression
\param error_action any other expression
\param ... a printf-like coma-separated input: "foo %s bar", string

Set the error_mode to \c 's' to stop the program on error. Otherwise execure error actions.

Example: `Stopif(!TFile::Open(input_file), continue, "cannot Open the file %s", input_file);` in a loop over filenames, to continue if a file is not found.*/

#define Stopif(assertion, error_action, ...) {                           \
	if (assertion){                                                  \
		fprintf(error_log ? error_log : stderr, __VA_ARGS__);    \
		fprintf(error_log ? error_log : stderr, "\n");           \
	if (error_mode=='s') abort();                                    \
	else                {error_action;}                              \
	}                                                                \
}

/* Global interface to the ttree, the name space for all the event-processing fucntions.
 */
#define NTUPLE_INTERFACE_CLASS_DECLARE
#include "stage2_interface.h"

/* --------------------------------------------------------------- */
/* STD DEFS */
//#include "std_defs.h"

/*
 * The distributions:
 * processing functions in the name space of the stage2 interface,
 * definitions of histograms with various ranges.
 */


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

/** \brief systematic function calculates event weight in the given systematic correction

This should scale all right to any number of systematics, including all PDF systematics.
 */

double NT_sysweight_NOMINAL()
	{
	return NT_event_weight;
	}

typedef double (*_F_sysweight)();
typedef struct {ObjSystematics obj_sys_id; _F_sysweight weight_func;} _S_systematic_definition;

#define NT_sysweight(sysname, weight_expr)   \
double NT_sysweight_ ##sysname(void)          \
	{                                  \
	return weight_expr; \
	}

NT_sysweight(PUUp, NT_event_weight*NT_event_weight_PUUp    )
NT_sysweight(PUDown, NT_event_weight*NT_event_weight_PUDown  )
NT_sysweight(bSFUp, (NT_event_weight_bSF > 0.? NT_event_weight*NT_event_weight_bSFUp   / NT_event_weight_bSF : 0.))
NT_sysweight(bSFDown, (NT_event_weight_bSF > 0.? NT_event_weight*NT_event_weight_bSFDown / NT_event_weight_bSF : 0.))
NT_sysweight(LEPelIDUp, NT_event_weight*NT_event_weight_LEPelIDUp   )
NT_sysweight(LEPelIDDown, NT_event_weight*NT_event_weight_LEPelIDDown )
NT_sysweight(LEPelTRGUp, NT_event_weight*NT_event_weight_LEPelTRGUp  )
NT_sysweight(LEPelTRGDown, NT_event_weight*NT_event_weight_LEPelTRGDown)
NT_sysweight(LEPmuIDUp, NT_event_weight*NT_event_weight_LEPmuIDUp   )
NT_sysweight(LEPmuIDDown, NT_event_weight*NT_event_weight_LEPmuIDDown )
NT_sysweight(LEPmuTRGUp, NT_event_weight*NT_event_weight_LEPmuTRGUp  )
NT_sysweight(LEPmuTRGDown, NT_event_weight*NT_event_weight_LEPmuTRGDown)


#define _quick_set_objsys(sysname) m[#sysname] = {sysname, NT_sysweight_NOMINAL}
#define _quick_set_wgtsys(sysname) m[#sysname] = {NOMINAL, NT_sysweight_##sysname}

map<TString, _S_systematic_definition> create_known_systematics()
	{
	map<TString, _S_systematic_definition> m;
	m["NOMINAL"] = {NOMINAL, NT_sysweight_NOMINAL};

	_quick_set_objsys(JERUp);
	_quick_set_objsys(JERDown);
	_quick_set_objsys(JESUp);
	_quick_set_objsys(JESDown);
	_quick_set_objsys(TESUp);
	_quick_set_objsys(TESDown);

	_quick_set_wgtsys(PUUp);
	_quick_set_wgtsys(PUDown);
	_quick_set_wgtsys(bSFUp);
	_quick_set_wgtsys(bSFDown);
	_quick_set_wgtsys(LEPelIDUp);
	_quick_set_wgtsys(LEPelIDDown);
	_quick_set_wgtsys(LEPelTRGUp);
	_quick_set_wgtsys(LEPelTRGDown);
	_quick_set_wgtsys(LEPmuIDUp);
	_quick_set_wgtsys(LEPmuIDDown);
	_quick_set_wgtsys(LEPmuTRGUp);
	_quick_set_wgtsys(LEPmuTRGDown);

	return m;
	}

map<TString, _S_systematic_definition> known_systematics = create_known_systematics();

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

/** \brief An instance of an output histogram.

 The function calculating the parameter, the `TH1D*` to the histogram object, the current calculated value (placeholder for future memoization).
 */

typedef struct {
	TH1D* histo;
	double (*func)(ObjSystematics);
	double value;
} TH1D_histo;

double NT_distr_leading_lep_pt(ObjSystematics sys)
	{
	return NT_event_leptons[0].pt();
	}

double NT_distr_Mt_lep_met(ObjSystematics sys)
	{

	if      (sys == NOMINAL) return NT_event_met_lep_mt;
	else if (sys == JERUp)   return NT_event_met_lep_mt_JERUp;
	else if (sys == JERDown) return NT_event_met_lep_mt_JERDown;
	else if (sys == JESUp)   return NT_event_met_lep_mt_JESUp;
	else if (sys == JESDown) return NT_event_met_lep_mt_JESDown;
	else if (sys == TESUp)   return NT_event_met_lep_mt_TESUp;
	else if (sys == TESDown) return NT_event_met_lep_mt_TESDown;

	else return NT_event_met_lep_mt;
	}

double NT_distr_met(ObjSystematics sys)
	{

	if      (sys == NOMINAL) return NT_event_met.pt();
	else if (sys == JERUp)   return NT_event_met_JERUp.pt();
	else if (sys == JERDown) return NT_event_met_JERDown.pt();
	else if (sys == JESUp)   return NT_event_met_JESUp.pt();
	else if (sys == JESDown) return NT_event_met_JESDown.pt();
	else if (sys == TESUp)   return NT_event_met_TESUp.pt();
	else if (sys == TESDown) return NT_event_met_TESDown.pt();

	else return NT_event_met.pt();
	}


/* "name": (func for value, range or custom range)
 *         -- it needs a function producing TH1D_histo, like in this block:

{
	TH1D* ahist = (TH1D*) new TH1D("lep_pt", "lep_pt", 40, 0, 200);
	TH1D_histo a_distr = {ahist, leading_lep_pt, 0.};
	distrs.push_back(a_distr);
}

somehow it seems another macro would do it!

but the name of the histogram might be complex and set at runtime:
channel_proc_sys_distr

so it must be a function
 */ 


/**
\brief The initialization function for the definitions of the known distributions.

\return map<const char*, _TH1D_histo_def>
 */

map<TString, _TH1D_histo_def> create_known__TH1D_histo_definitions()
{
	map<TString, _TH1D_histo_def> m;
	_TH1D_histo_range r;
	//r = {.nbins=40, .linear_min=0, .linear_max=200}; m["leading_lep_pt"] = {leading_lep_pt, r};
	// despicable!
	// "sorry, unimplemented: non-trivial designated initializers not supported"
	r = {40, true,  0, 200};                                                     m["leading_lep_pt"] = {NT_distr_leading_lep_pt, r};

	//r = {14, false,-1,  -1, .custom_bins=(double[]){0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//r = {2, false,-1,  -1, .custom_bins=(double[]){{0},{16},{32}}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//r = {2, false,-1,  -1, {0.,16.,32.}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	// it seems gcc on lxplus is buggy
	//static double Mt_lep_met_c_bins[] = {0,16,32,44,54,64};
	//r = {5, false,-1,  -1}; r.custom_bins = Mt_lep_met_c_bins; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	// -- ok, this works
	//r = {2, false,-1,  -1, (static double*){0.,16.,32.}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//static double Mt_lep_met_c_bins[] = {0,16,32,44,54,64}; r = {5, false,-1,  -1, Mt_lep_met_c_bins}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	static double bins_Mt_lep_met_c[] = {0,16,32,44,54,64}; r = {(sizeof(bins_Mt_lep_met_c) / sizeof(bins_Mt_lep_met_c[0]))-1, false,-1,  -1, bins_Mt_lep_met_c}; m["Mt_lep_met_c"]   = {NT_distr_Mt_lep_met,     r};
	// ok! this needs a wrapper-macro
	cerr_expr(r.custom_bins[0]);
	cerr_expr(r.custom_bins[1]);

	r = {20, true,  0, 250};                                                     m["Mt_lep_met_f"]   = {NT_distr_Mt_lep_met,     r};

	//set_range_linear(r, 40, 0, 200);                                         m["leading_lep_pt"] = {leading_lep_pt, r};
	//set_range_custom(r, (double[]){0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}); m["Mt_lep_met_c"]   = {Mt_lep_met, r};

	// these also do not work:
	//m["leading_lep_pt"] = {leading_lep_pt, range_linear(40, 0, 200)};
	////m["Mt_lep_met_c"]   = {Mt_lep_met, range_custom({0,16,32,44,54,64,74,81,88,95,104,116,132,160,250})};
	//m["Mt_lep_met_c"]   = {Mt_lep_met, range_custom_base({0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}, 14)};
	//m["Mt_lep_met_f"]   = {Mt_lep_met, range_linear(20, 0, 250)};

	r = {25, true,  0, 200};   m["met_f2"] = {NT_distr_met, r};
	r = {30, true,  0, 300};   m["met_f"]  = {NT_distr_met, r};
	static double bins_met_c[] = {0,20,40,60,80,100,120,140,200,500}; r = {(sizeof(bins_met_c) / sizeof(bins_met_c[0]))-1, false,  -1, -1, bins_met_c};   m["met_c"]  = {NT_distr_met, r};

	return m;
}

/** \brief The known distributions.
 */

map<TString, _TH1D_histo_def> known_defs_distrs = create_known__TH1D_histo_definitions();


/*
 * 
 */


/** \brief A helper function creating the instances of TH1D_histos with a specific name from the given _TH1D_histo_def definition.

Creates a `new TH1D(name, ..., range)` according to the linear or custom range in the _TH1D_histo_def definition.

\param  _TH1D_histo_def& def
\param  TString   name
\return TH1D_histo
 */

TH1D_histo create_TH1D_histo(_TH1D_histo_def& def, TString name)
{
	TH1D* histo;
	if (def.range.linear)
		{
		cout << "making linear bins histogram " << endl;
		histo = (TH1D*) new TH1D(name, name, def.range.nbins, def.range.linear_min, def.range.linear_max);
		}
	else
		{
		cout << "making custom bins histogram " << def.range.nbins << " " << def.range.custom_bins << endl;
		histo = (TH1D*) new TH1D(name, name, def.range.nbins, def.range.custom_bins);
		cout << "making custom bins histogram DONW" << endl;
		}
	//TH1D_histo a_distr = {ahist, def.func, 0.};
	//distrs.push_back(a_distr);
	return {histo, def.func, 0.};
}

/*
 * The final state channels:
 * boolean functions whether the event passes a selection or no.

old std_defs.py:

'mu_sel':          (lambda sel_stage, ev: (sel_stage==  9 or sel_stage==  7), main_sel_stages),
'mu_sel_ss':       (lambda sel_stage, ev: (sel_stage==  8 or sel_stage==  6), main_sel_stages),
'el_sel':          (lambda sel_stage, ev: (sel_stage== 19 or sel_stage== 17), main_sel_stages),
'el_sel_ss':       (lambda sel_stage, ev: (sel_stage== 18 or sel_stage== 16), main_sel_stages),

'tt_elmu':        (lambda sel_stage, ev: (sel_stage> 200 and sel_stage < 210 and ev.event_leptons[0].pt() > 30. and ev.event_leptons[1].pt() > 30.), {'NOMINAL': lambda ev: ev.selection_stage_em}),
'tt_elmu_tight':  (lambda sel_stage, ev: (sel_stage == 205 and ev.event_leptons[0].pt() > 30. and ev.event_leptons[1].pt() > 30.), {'NOMINAL': lambda ev: ev.selection_stage_em}),
 */

bool NT_channel_mu_sel(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_TESDown;
	else relevant_selection_stage = NT_selection_stage;
	return relevant_selection_stage == 9 && relevant_selection_stage == 7;
	}

bool NT_channel_mu_sel_ss(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_TESDown;
	else relevant_selection_stage = NT_selection_stage;
	return relevant_selection_stage == 8 && relevant_selection_stage == 6;
	}

bool NT_channel_el_sel(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_TESDown;
	else relevant_selection_stage = NT_selection_stage;
	return relevant_selection_stage == 19 && relevant_selection_stage == 17;
	}

bool NT_channel_el_sel_ss(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_TESDown;
	else relevant_selection_stage = NT_selection_stage;
	return relevant_selection_stage == 18 && relevant_selection_stage == 16;
	}



bool NT_channel_tt_elmu(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_em;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_em_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_em_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_em_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_em_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_em_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_em_TESDown;
	else relevant_selection_stage = NT_selection_stage_em;
	return relevant_selection_stage > 200 && relevant_selection_stage < 210;
	}

bool NT_channel_tt_elmu_tight(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_em;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_em_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_em_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_em_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_em_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_em_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_em_TESDown;
	else relevant_selection_stage = NT_selection_stage_em;
	return relevant_selection_stage == 205;
	}



bool NT_channel_dy_mutau(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_dy_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_dy_TESDown;
	else relevant_selection_stage = NT_selection_stage_dy;
	return relevant_selection_stage == 102 || relevant_selection_stage == 103;
	}

bool NT_channel_dy_eltau(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_dy_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_dy_TESDown;
	else relevant_selection_stage = NT_selection_stage_dy;
	return relevant_selection_stage == 112 || relevant_selection_stage == 113;
	}

bool NT_channel_dy_mutau_ss(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_dy_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_dy_TESDown;
	else relevant_selection_stage = NT_selection_stage_dy;
	return relevant_selection_stage == 202 || relevant_selection_stage == 203;
	}

bool NT_channel_dy_eltau_ss(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_dy_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_dy_TESDown;
	else relevant_selection_stage = NT_selection_stage_dy;
	return relevant_selection_stage == 212 || relevant_selection_stage == 213;
	}



bool NT_channel_dy_mumu(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy_mumu;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_mumu_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_mumu_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_mumu_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_mumu_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_dy_mumu_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_dy_mumu_TESDown;
	else relevant_selection_stage = NT_selection_stage_dy_mumu;
	return relevant_selection_stage == 102 || relevant_selection_stage == 103 || relevant_selection_stage == 105;
	}

bool NT_channel_dy_elel(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy_mumu;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_mumu_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_mumu_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_mumu_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_mumu_JESDown;
	else if (sys == TESUp)     relevant_selection_stage = NT_selection_stage_dy_mumu_TESUp  ;
	else if (sys == TESDown)   relevant_selection_stage = NT_selection_stage_dy_mumu_TESDown;
	else relevant_selection_stage = NT_selection_stage_dy_mumu;
	return relevant_selection_stage == 112 || relevant_selection_stage == 113 || relevant_selection_stage == 115;
	}




/** \brief The channel-defining `bool` function.

Each final state channel is defined by a `bool` function,
that is calculated in the name space of the input `TTree` branches.
It takes the `ObjSystematics` identifier as input,
and returns `bool` whether event passes this channel selection or not.
 */
typedef bool (*_F_channel_def)(ObjSystematics);


#define _quick_set_chandef(m, chan_name) m[#chan_name] = NT_channel_ ## chan_name

/** \brief The initialization function for the `bool` functions of the known distributions.

\return map<const char*, _F_channel_def>
 */

map<TString, _F_channel_def> create_known_channel_definitions()
{
	map<TString, bool (*)(ObjSystematics)> m;
	//m["el_sel"] = channel_el_sel;
	_quick_set_chandef(m, el_sel);
	_quick_set_chandef(m, el_sel_ss);
	_quick_set_chandef(m, mu_sel);
	_quick_set_chandef(m, mu_sel_ss);

	_quick_set_chandef(m, tt_elmu);
	_quick_set_chandef(m, tt_elmu_tight);

	_quick_set_chandef(m, dy_mutau);
	_quick_set_chandef(m, dy_eltau);
	_quick_set_chandef(m, dy_mutau_ss);
	_quick_set_chandef(m, dy_eltau_ss);
	_quick_set_chandef(m, dy_mumu);
	_quick_set_chandef(m, dy_elel);

	return m;
}


/** \brief The known channels.
 */
map<TString, _F_channel_def> known_defs_channels = create_known_channel_definitions();


/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
// DTAGS, dtags, all dtag-related info, x-secs, systematics, gen processes

// (sub-)processes in a dtag are defined by whether gen_proc_id integer falls inside inclusive range of IDs [min_gen_id, max_gen_id]
// a number of standard definitions are given
// so a user can define them per final state channel

/*
In principle, since the definition of a gen-level processes
is done with a simple integer in stage2, I could implement
definitions of processes with some ranges of gen ID.

typedef struct {
	int min;
	int max;
} proc_ID_range;

But I want to use functions operating in the name space of the TTree interface,
the same way as for the channel definitions.
It would is a much more flexible solution,
it is easily extensible to processing raw ntuples.

However, it requires more typing, therefore I will make some short-hand macro.
*/

/** \brief passes all events, to use in inclusive cases
*/

bool NT_genproc_inclusive()
	{
	return true;
	}

bool NT_genproc_tt_eltau3ch()
	{
	return NT_gen_proc_id == 42;
	}

typedef bool (*_F_genproc_def)(void);

// a short-hand macro

#define NT_genproc(procname, procID)         \
bool NT_genproc_ ##procname(void)            \
	{                                  \
	return NT_gen_proc_id == (procID); \
	}

NT_genproc(tt_eltau1ch, 41)
NT_genproc(tt_mutau3ch, 32)
NT_genproc(tt_mutau1ch, 31)

NT_genproc(tt_ljb        , 24)
NT_genproc(tt_ljw        , 23)
NT_genproc(tt_ljo        , 22)
NT_genproc(tt_ljz        , 21)
NT_genproc(tt_taultauh   , 12)
NT_genproc(tt_taulj      , 11)
NT_genproc(tt_elmu       , 3)
NT_genproc(tt_ltaul      , 2)
NT_genproc(tt_taueltaumu , 1)
//NT_genproc(tt_other      , 0) // no "tt_other"! it is a catchall process, they are handled automatically

bool NT_genproc_tt_eltau()
	{
	return NT_gen_proc_id > 40 && NT_gen_proc_id < 43;
	}

#define NT_genproc_range(procname, procID_min, procID_max)         \
bool NT_genproc_ ##procname(void)          \
	{                                  \
	return NT_gen_proc_id > (procID_min) && NT_gen_proc_id < (procID_max); \
	}

NT_genproc_range(tt_mutau, 30, 33)

NT_genproc_range(tt_lj, 20, 30)


/** \brief The definition of (sub-)processes for a given dtag

Contains gen ID ranges for all possible sub-processes and the handy groups.
 */

typedef struct {
	TString catchall_name;    /**< \brief name for the inclusive process, or catch-all case */
	map<TString, _F_genproc_def> all;    /**< \brief all possible sub-processes */
	map<TString, _F_genproc_def> groups; /**< \brief groups of sub-processes */
	map<TString, vector<TString>> channel_standard; /**< \brief standard sub-processes per channel */
} _S_proc_ID_defs;

// standard per-channel processes
vector<TString> _tt_procs_eltau = {"tt_eltau", "tt_taulj", "tt_lj"};
vector<TString> _tt_procs_mutau = {"tt_mutau", "tt_taulj", "tt_lj"};
vector<TString> _tt_procs_elmu  = {"tt_elmu",  "tt_ltaul"};
//vector<TString> _tt_procs_mumu  = {"tt_inclusive"};
//vector<TString> _tt_procs_elel  = {"tt_inclusive"};
// actually empty vectors must work, the catchall process will handle this
vector<TString> _tt_procs_mumu  = {};
vector<TString> _tt_procs_elel  = {};

map<TString, _S_proc_ID_defs> create_known_procs_info()
	{
	map<TString, _S_proc_ID_defs> m;

	m["tt"] = {
		.catchall_name = "tt_other",
		.all={
			{"tt_eltau3ch" , NT_genproc_tt_eltau3ch},
			{"tt_eltau1ch" , NT_genproc_tt_eltau1ch},
			{"tt_mutau3ch" , NT_genproc_tt_mutau3ch},
			{"tt_mutau1ch" , NT_genproc_tt_mutau1ch},

			{"tt_ljb"        , NT_genproc_tt_ljb        },
			{"tt_ljw"        , NT_genproc_tt_ljw        },
			{"tt_ljo"        , NT_genproc_tt_ljo        },
			{"tt_ljz"        , NT_genproc_tt_ljz        },
			{"tt_taultauh"   , NT_genproc_tt_taultauh   },
			{"tt_taulj"      , NT_genproc_tt_taulj      },
			{"tt_elmu"       , NT_genproc_tt_elmu       },
			{"tt_ltaul"      , NT_genproc_tt_ltaul      },
			{"tt_taueltaumu" , NT_genproc_tt_taueltaumu },
			//{"tt_other"      , NT_genproc_tt_other      },
			},

		.groups={
			{"tt_eltau" , NT_genproc_tt_eltau},
			{"tt_mutau" , NT_genproc_tt_mutau},
			{"tt_lj"    , NT_genproc_tt_lj},
			{"tt_inclusive" , NT_genproc_inclusive},
			},

		.channel_standard = {
			{"el_sel",    _tt_procs_eltau},
			{"mu_sel",    _tt_procs_eltau},
			{"el_sel_ss", _tt_procs_mutau},
			{"mu_sel_ss", _tt_procs_mutau},

			{"tt_elmu",       _tt_procs_elmu},
			{"tt_elmu_tight", _tt_procs_elmu},
			{"dy_mutau",      _tt_procs_mutau},
			{"dy_mutau_ss",   _tt_procs_mutau},
			{"dy_eltau",      _tt_procs_eltau},
			{"dy_eltau_ss",   _tt_procs_eltau},
			{"dy_mumu",       _tt_procs_mumu},
			{"dy_elel",       _tt_procs_mumu},

			}
		};

//genproc_tt_ljb       = 24
//genproc_tt_ljw       = 23
//genproc_tt_ljo       = 22
//genproc_tt_lj        = 21
//genproc_tt_taultauh  = 12
//genproc_tt_taulj     = 11
//
//genproc_tt_elmu       = 3
//genproc_tt_ltaul      = 2
//genproc_tt_taueltaumu = 1
//
//genproc_tt_other = 0


	return m;
	}

map<TString, _S_proc_ID_defs> known_procs_info = create_known_procs_info();

double W_lep_br    = 0.108;
double W_alllep_br = 3*0.108;
double W_qar_br    = 0.676;

double W_lep_br2    = W_lep_br*W_lep_br;
double W_alllep_br2 = W_alllep_br*W_alllep_br;
double W_qar_br2    = W_qar_br*W_qar_br;

double br_tau_electron = 0.1785;
double br_tau_muon     = 0.1736;
double br_tau_lepton   = br_tau_electron + br_tau_muon;
double br_tau_hadronic = 1. - br_tau_lepton;

double ttbar_xsec = 831.76; // at 13 TeV

typedef struct {
	double       cross_section;
	double       usual_gen_lumi;
	_S_proc_ID_defs procs;
	// all available systematics
} S_dtag_info;

map<TString, S_dtag_info> create_known_dtags_info()
	{
	//map<const char*, S_dtag_info> m;
	map<TString, S_dtag_info> m;

	m["MC2017legacy_Fall17_WJets_madgraph_v2"              ] = {.cross_section= 52940.                            , .usual_gen_lumi= 23102470.188817, .procs=known_procs_info["tt"]};
	m["MC2017legacy_Fall17_DYJetsToLL_50toInf_madgraph_v1" ] = {.cross_section=  6225.42                          , .usual_gen_lumi= 18928303.971956, .procs=known_procs_info["tt"]};
	m["MC2017legacy_Fall17_SingleT_tW_5FS_powheg_v1"       ] = {.cross_section=    35.6                           , .usual_gen_lumi=  5099879.048270, .procs=known_procs_info["tt"]};
	m["MC2017legacy_Fall17_SingleTbar_tW_5FS_powheg_v1"    ] = {.cross_section=    35.6                           , .usual_gen_lumi=  2349775.859249, .procs=known_procs_info["tt"]};
	m["MC2017legacy_Fall17_TTToHadronic_13TeV"             ] = {.cross_section=   831.76 * W_qar_br2              , .usual_gen_lumi= 29213134.729453, .procs=known_procs_info["tt"]};
	m["MC2017legacy_Fall17_TTToSemiLeptonic_v2"            ] = {.cross_section=   831.76 * 2*W_alllep_br*W_qar_br , .usual_gen_lumi= 21966343.919990, .procs=known_procs_info["tt"]};
	m["MC2017legacy_Fall17_TTTo2L2Nu"                      ] = {.cross_section=   831.76 * W_alllep_br2           , .usual_gen_lumi=  2923730.883332, .procs=known_procs_info["tt"]};

	return m;
	}

map<TString, S_dtag_info> known_dtags_info = create_known_dtags_info();



/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
/* main structure of the event loop
// the struct-s of the structure of the event loop
// since systematics affect event selection (final state channel) and distributions
// they must be up the hierarchy
// process definition is purely gen-level, independent from reco
// it can be the top-most level for a dtag
// but in daily usage we usually hadd everything, open a final state and look at distributions in the same or different processes.
// Ok, purely for the sake of symmetry and easy memorization, let's just revert the nesting w.r.t the usual one:
SYSTEMATIC / PROCESS / CHANNEL / channel_process_systematic_distrname
*/


typedef struct{
	_F_genproc_def proc_def;  /**< \brief `bool` function defining whether an event passes this gen proc definition */
	vector<TH1D_histo> histos;         /**< \brief the distributions to record */
} T_proc_histos;

typedef struct{
	_F_channel_def chan_def;      /**< \brief `bool` function defining whether an event passes the channel selection */
	vector<T_proc_histos> procs;  /**< \brief the channels with distributions to record */
	vector<TH1D_histo> catchall_proc_histos;  /**< \brief the channels with distributions to record in the catchall process */
} T_chan_proc_histos;

typedef struct{
	_S_systematic_definition syst_def;  /**< \brief the definition of a given systematic */
	vector<T_chan_proc_histos> chans;  /**< \brief the per-proc channels with distributions to record */
} T_syst_chan_proc_histos;

/* --------------------------------------------------------------- */



/** \brief The main program executes user's request over the given list of files, in all found `TTree`s in the files.

It parses the requested channels, systematics and distributions;
prepares the structure of the loop;
loops over all given files and `TTree`s in them,
recording the distributions at the requested systematics;
applies final corrections to the distributions;
and if asked normalizes the distribution to `cross_section/gen_lumi`;
finally all histograms are written out in the standard format `channel/process/systematic/channel_process_systematic_distr`.

The input now: `input_filename [input_filename+]`.
 */

int main (int argc, char *argv[])
{
argc--;
const char* exec_name = argv[0];
argv++;

if (argc < 1)
	{
	std::cout << "Usage:" << " input_filename [input_filename+]" << std::endl;
	exit (0);
	}

gROOT->Reset();

//Int_t rebin_factor(atoi(argv[3]));
//TString dir(argv[4]);
//TString dtag1(argv[5]);

/* Here we will parse user's request for figures
 * to create the structure filled up in the event loop.
 */

// figure out the dtag of the input files from the first input file
TString first_input_filename(argv[0]);
TString main_dtag("");

// loop over known dtags and find whether any of the matches
map<TString, S_dtag_info>::iterator a_dtag_info = known_dtags_info.begin();
while (a_dtag_info != known_dtags_info.end())
	{
	// Accessing KEY from element pointed by it.
	TString dtag_name = a_dtag_info->first;
	if (first_input_filename.Contains(dtag_name))
		{
		main_dtag = dtag_name;
		break;
		}
	// Increment the Iterator to point to next entry
	a_dtag_info++;
	}

// test if no dtag was recognized
//Stopif(!main_dtag, ;, "could not recognize any known dtag in %s", first_input_filename.Data());
Stopif(main_dtag.EqualTo(""), ;, "could not recognize any known dtag in %s", first_input_filename.Data());
//if (main_dtag.EqualTo(""))
//	{
//	cerr_expr();
//	}

S_dtag_info main_dtag_info = known_dtags_info[main_dtag];
cerr_expr(main_dtag << " : " << main_dtag_info.cross_section);

//// define histograms for the distributions
//vector<TH1D_histo> distrs;

// define a nested list: list of channels, each containing a list of histograms to record
vector<T_syst_chan_proc_histos> distrs_to_record;


// final state channels
const char* requested_channel_names[] = {"el_sel", "mu_sel", "el_sel_ss", "mu_sel_ss", "tt_elmu", "tt_elmu_tight", "dy_mutau", "dy_mutau_ss", "dy_eltau", "dy_eltau_ss", "dy_mumu", "dy_elel", NULL};
//const char* requested_channel_names[] = {"tt_elmu", NULL};
//vector<TString> requested_procs   = {"all"};
vector<TString> requested_procs   = {"std"};
const char* requested_systematics[]   = {"NOMINAL",
 "JERUp",
 "JERDown",
 "JESUp",
 "JESDown",
 "TESUp",
 "TESDown",
"PUUp",
"PUDown",
"bSFUp",
"bSFDown",
"LEPelIDUp",
"LEPelIDDown",
"LEPelTRGUp",
"LEPelTRGDown",
"LEPmuIDUp",
"LEPmuIDDown",
"LEPmuTRGUp",
"LEPmuTRGDown",
 NULL};

//requested_systematics[0] = "NOMINAL"; requested_systematics[1] = NULL;

const char* requested_distrs[]        = {"Mt_lep_met_c", "leading_lep_pt", NULL};

// set the known processes according to the request: whetehr groups of processes are requested or not
// check if all known processes are requested
map<TString, _F_genproc_def> known_procs;
if (TString(requested_procs[0]) == "all")
	{
	known_procs = main_dtag_info.procs.all;
	// and reset the requested processes to all known processes for this dtag
	requested_procs.clear();
	// loop over all known processes
	for (const auto& proc: main_dtag_info.procs.all)
		{
		cout_expr(proc.first);
		requested_procs.push_back(proc.first);
		}
	}

else
	{
	known_procs = main_dtag_info.procs.groups;
	//known_procs = main_dtag_info.procs.all;
	// populate definitions of groups with the fine-grain definitions of all processes			
	known_procs.insert(main_dtag_info.procs.all.begin(), main_dtag_info.procs.all.end());
	}

// name of the catchall process for the defined dtag
TString procname_catchall = main_dtag_info.procs.catchall_name;
map<TString, vector<TString>>& known_std_procs_per_channel = main_dtag_info.procs.channel_standard;

for (const char** requested_sys = requested_systematics; *requested_sys != NULL; requested_sys++)
	{
	// find the definition of this channel
	TString systname(*requested_sys);
	Stopif(known_systematics.find(systname) == known_systematics.end(), continue, "Do not know a systematic %s", systname.Data());

	T_syst_chan_proc_histos systematic = {.syst_def = known_systematics[systname]};

	// define channels
	for (const char** requested_channel = requested_channel_names; *requested_channel != NULL; requested_channel++)
		{
		// find the definition of this channel
		TString channame(*requested_channel);
		Stopif(known_defs_channels.find(channame) == known_defs_channels.end(), continue, "Do not know the channel %s", channame.Data());

		T_chan_proc_histos channel          = {.chan_def=known_defs_channels[channame]};

		// check if standard processes per channels are requested
		vector<TString>* process_definitions_to_use = &requested_procs;
		if ((*process_definitions_to_use)[0] == "std")
			{
			// TODO in case of an unknown process set an inclusive definition
			Stopif(known_std_procs_per_channel.find(channame) == known_std_procs_per_channel.end(), continue, "Do not know standard processes for the channel %s", channame.Data())
			// set the standard processes
			process_definitions_to_use = &known_std_procs_per_channel[channame];
			}

		// define processes
		bool is_catchall_proc_done = false; // TODO: it will be set with the first defined process, but make it work always -- push_back an additional process with the inclusive definition

		// loop over requested processes and find their definitions for recording in this channel
		//for (const char** requested_proc = requested_procs; *requested_proc != NULL; requested_proc++)
		for (const auto& procname: (*process_definitions_to_use))
			{
			// find the definition of this channel
			//TString procname(*requested_proc);

			Stopif(known_procs.find(procname) == known_procs.end(), continue, "Do not know the process %s", procname.Data());

			T_proc_histos process = {.proc_def=known_procs[procname]};

			// define distributions
			// create the histograms for all of these definitions
			for (const char** requested_distr = requested_distrs; *requested_distr != NULL; requested_distr++)
				{
				TString distrname(*requested_distr);
				Stopif(known_defs_distrs.find(distrname) == known_defs_distrs.end(), continue, "Do not know a distribution %s", distrname.Data());

				TH1D_histo a_distr = create_TH1D_histo(known_defs_distrs[distrname], channame + "_" + procname + "_" + systname + "_" + distrname);
				process.histos.push_back(a_distr);

				if (!is_catchall_proc_done)
					{
					TH1D_histo a_distr = create_TH1D_histo(known_defs_distrs[distrname], channame + "_" + procname_catchall + "_" + systname + "_" + distrname);
					channel.catchall_proc_histos.push_back(a_distr);
					}
				}

			channel.procs.push_back(process);
			// together with the first requested processes the catchall processes has been set up
			if (!is_catchall_proc_done) is_catchall_proc_done = true;
			}

		systematic.chans.push_back(channel);
		}

	distrs_to_record.push_back(systematic);
	}

// process input files
for (unsigned int cur_var = 0; cur_var<argc; cur_var++)
	{
	TString input_filename(argv[cur_var]);

	cerr_expr(cur_var << " " << input_filename);
	//cerr_expr(input_filename);

	//dtags.push_back(dtag);
	//files.push_back(TFile::Open(dir + "/" + dtag + ".root"));
	//dtags.push_back(5);

	// get input ttree
	TFile* input_file  = TFile::Open(input_filename);
	Stopif(!input_file,  continue, "cannot Open TFile in %s, skipping", input_filename.Data());

	TTree* NT_output_ttree = (TTree*) input_file->Get("ttree_out");
	Stopif(!NT_output_ttree, continue, "cannot Get TTree in file %s, skipping", input_filename.Data());

	// open the interface to stage2 TTree-s
	//#define NTUPLE_INTERFACE_OPEN
	#define NTUPLE_INTERFACE_CONNECT
	#include "stage2_interface.h"

	unsigned int n_entries = NT_output_ttree->GetEntries();
	//cerr_expr(n_entries);

	for (unsigned int ievt = 0; ievt < n_entries; ievt++)
		{
		NT_output_ttree->GetEntry(ievt);

		//// tests
		////cerr_expr(NT_runNumber);
		//cerr_expr(NT_event_leptons[0].pt());
		//cerr_expr(NT_event_leptons_genmatch[0]);

		//Stopif(ievt > 10, break, "reached 10 events, exiting");

		// set the object systematic
		ObjSystematics systematic = NOMINAL;

		for (int si=0; si<distrs_to_record.size(); si++)
			{
			ObjSystematics obj_systematic = distrs_to_record[si].syst_def.obj_sys_id;
			double event_weight       = distrs_to_record[si].syst_def.weight_func();

			// record distributions in all final states where the event passes
			vector<T_chan_proc_histos>& channels = distrs_to_record[si].chans;
			for (int ci=0; ci<channels.size(); ci++)
				{
				T_chan_proc_histos& chan = channels[ci];

				// check if event passes the channel selection
				if (!chan.chan_def(obj_systematic)) continue;

				// assign the gen process
				// loop over procs check if this event passes
				// if not get the catchall proc
				vector<TH1D_histo>* histos = NULL;
				for (int pi=0; pi<chan.procs.size(); pi++)
					{
					if (chan.procs[pi].proc_def())
						{
						histos = &chan.procs[pi].histos;
						break;
						}
					}

				if (histos == NULL)
					histos = & chan.catchall_proc_histos;

				// record all distributions with the given event weight
				for (int di=0; di<histos->size(); di++)
					{
					TH1D_histo& histo_torecord = (*histos)[di];
					// TODO memoize if possible
					double value = histo_torecord.func(obj_systematic);
					histo_torecord.histo->Fill(value, event_weight);
					//histo_torecord.histo->Fill(value);
					}
				// <-- I keep the loops with explicit indexes, since the indexes can be used to implement memoization

				//for(const auto& histo_torecord: chan.histos)
				//	{
				//	double value = histo_torecord.func(obj_systematic);
				//	histo_torecord.histo->Fill(value, event_weight);
				//	}
				}
			}

		// end of event loop
		}
	}

/*
for(std::map<TString, double>::iterator it = xsecs.begin(); it != xsecs.end(); ++it)
	{
	TString dtag = it->first;
	double xsec  = it->second;
	cout << "For dtag " << dtag << " xsec " << xsec << "\n";
	}
*/


//std::vector < TString > dtags;
//std::vector < TFile * > files;
//std::vector < TH1D * > histos;
//std::vector < TH1D * > weightflows;
//// nick->summed histo
//std::map<TString, TH1D *> nicknamed_mc_histos;
////vector<int> dtags;
////dtags.reserve();

// make stack of MC, scaling according to ratio = lumi * xsec / weightflow4 (bin5?)
// also nickname the MC....
// per-dtag for now..

TFile* output_file  = (TFile*) new TFile("outtest.root", "RECREATE");
output_file->Write();

/*
for (int distr_i = 0; distr_i<distrs.size(); distr_i++)
	{
	distrs[distr_i].histo->Print(); // for the tests
	distrs[distr_i].histo->Write();
	}
*/

/*
for (int chan_i = 0; chan_i<requested_channels.size(); chan_i++)
	{
	//TODO create nested TDirectories for easier manual management, or do they slow down hadd?

	// output the histograms
	vector<TH1D_histo>& distrs = requested_channels[chan_i].distrs;
	for (int distr_i = 0; distr_i<distrs.size(); distr_i++)
		{
		// TODO memoize, optimize
		distrs[distr_i].histo->Print();
		distrs[distr_i].histo->Write();
		}
	}
*/

for (int si=0; si<distrs_to_record.size(); si++)
	{
	// merge defined processes and the catchall
	vector<T_chan_proc_histos>& all_chans = distrs_to_record[si].chans;

	for(const auto& chan: all_chans)
		{
		for(const auto& proc: chan.procs)
			{
			for(const auto& recorded_histo: proc.histos)
				{
				recorded_histo.histo->Print();
				recorded_histo.histo->Write();
				}
			}
		// same for catchall
		for(const auto& recorded_histo: chan.catchall_proc_histos)
			{
			recorded_histo.histo->Write();
			}
		}
	}

output_file->Close();
}


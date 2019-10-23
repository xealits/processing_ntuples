/**
\file sumup_loop_up.C
\mainpage The sumup_loop plotting routine.
\brief A flexible plotting program.
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

#define cout_expr(Name)   cout << #Name << " = " << (Name) << std::endl

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


/** \ingroup Enumerations
 *  \brief The identifiers for all systematics.

The systematic corrections to objects usually change the energy of the object (not the direction),
get propagated to MET if needed,
and recompute all corresponding `stage_selection` parameters.
The object systematics are:
\c JERUp, \c JERDown,
\c JESUp, \c JESDown,
\c TESUp, \c TESDown.

 */
enum Systematics {NOMINAL /**< NOMINAL sysytematic corrections, event weights and object corrections */,
 JERUp   /**< jet energy resolution Up corrections are applied to jets. */,
 JERDown /**< jet energy resolution Down corrections are applied to jets */,
 JESUp   /**< jet energy scale Up corrections are applied to jets */,
 JESDown /**< jet energy scale Down corrections are applied to jets */,
 TESUp   /**< tau energy scale Up corrections are applied to jets */,
 TESDown /**< tau energy scale Down corrections are applied to jets */
};


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
} TH1D_range;

/*
these do not really work
TH1D_range range_linear(unsigned int nbins, double min, double max)
	{
	return (TH1D_range) {nbins, true, min, max};
	}

TH1D_range range_custom_base(const double custom_bins[], unsigned int nbins)
	{
	//unsigned int nbins = sizeof(custom_bins) / sizeof(custom_bins[0]);
	return (TH1D_range) {nbins, false, -1, -1, {custom_bins}};
	}

//#define range_custom(...) range_custom_base(__VA_ARGS__, sizeof(__VA_ARGS__) / sizeof(__VA_ARGS__[0]))

#define set_range_linear(range_var, nbins, min, max) range_var = {(nbins), true,  (min), (max)}
#define set_range_custom(range_var, ...) unsigned int nbins = sizeof(__VA_ARGS__)/sizeof(__VA_ARGS__[0]) ; range_var = {(nbins), false, -1, -1, __VA_ARGS__}
*/

/** \brief The definition of an output histogram

 The function calculating the parameter, and the range of the histogram to store it.
 */

typedef struct {
	double (*func)(Systematics);
	TH1D_range range;
} TH1D_def;

/** \brief An instance of an output histogram.

 The function calculating the parameter, the `TH1D*` to the histogram object, the current calculated value (placeholder for future memoization).
 */

typedef struct {
	TH1D* histo;
	double (*func)(Systematics);
	double value;
} TH1D_distr;

double distr_leading_lep_pt(Systematics sys)
	{
	return NT_event_leptons[0].pt();
	}

double distr_Mt_lep_met(Systematics sys)
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

double distr_met(Systematics sys)
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
 *         -- it needs a function producing TH1D_distr, like in this block:

{
	TH1D* ahist = (TH1D*) new TH1D("lep_pt", "lep_pt", 40, 0, 200);
	TH1D_distr a_distr = {ahist, leading_lep_pt, 0.};
	distrs.push_back(a_distr);
}

somehow it seems another macro would do it!

but the name of the histogram might be complex and set at runtime:
channel_proc_sys_distr

so it must be a function
 */ 


/** \brief The initialization function for the definitions of the known distributions.

\return map<const char*, TH1D_def>
 */

map<const char*, TH1D_def> create_known_TH1D_distr_definitions()
{
	map<const char*, TH1D_def> m;
	TH1D_range r;
	//r = {.nbins=40, .linear_min=0, .linear_max=200}; m["leading_lep_pt"] = {leading_lep_pt, r};
	// despicable!
	// "sorry, unimplemented: non-trivial designated initializers not supported"
	r = {40, true,  0, 200};                                                     m["leading_lep_pt"] = {distr_leading_lep_pt, r};

	//r = {14, false,-1,  -1, .custom_bins=(double[]){0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//r = {2, false,-1,  -1, .custom_bins=(double[]){{0},{16},{32}}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//r = {2, false,-1,  -1, {0.,16.,32.}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	// it seems gcc on lxplus is buggy
	//static double Mt_lep_met_c_bins[] = {0,16,32,44,54,64};
	//r = {5, false,-1,  -1}; r.custom_bins = Mt_lep_met_c_bins; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	// -- ok, this works
	//r = {2, false,-1,  -1, (static double*){0.,16.,32.}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//static double Mt_lep_met_c_bins[] = {0,16,32,44,54,64}; r = {5, false,-1,  -1, Mt_lep_met_c_bins}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	static double bins_Mt_lep_met_c[] = {0,16,32,44,54,64}; r = {(sizeof(bins_Mt_lep_met_c) / sizeof(bins_Mt_lep_met_c[0]))-1, false,-1,  -1, bins_Mt_lep_met_c}; m["Mt_lep_met_c"]   = {distr_Mt_lep_met,     r};
	// ok! this needs a wrapper-macro
	cout_expr(r.custom_bins[0]);
	cout_expr(r.custom_bins[1]);

	r = {20, true,  0, 250};                                                     m["Mt_lep_met_f"]   = {distr_Mt_lep_met,     r};

	//set_range_linear(r, 40, 0, 200);                                         m["leading_lep_pt"] = {leading_lep_pt, r};
	//set_range_custom(r, (double[]){0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}); m["Mt_lep_met_c"]   = {Mt_lep_met, r};

	// these also do not work:
	//m["leading_lep_pt"] = {leading_lep_pt, range_linear(40, 0, 200)};
	////m["Mt_lep_met_c"]   = {Mt_lep_met, range_custom({0,16,32,44,54,64,74,81,88,95,104,116,132,160,250})};
	//m["Mt_lep_met_c"]   = {Mt_lep_met, range_custom_base({0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}, 14)};
	//m["Mt_lep_met_f"]   = {Mt_lep_met, range_linear(20, 0, 250)};

	r = {25, true,  0, 200};   m["met_f2"] = {distr_met, r};
	r = {30, true,  0, 300};   m["met_f"]  = {distr_met, r};
	static double bins_met_c[] = {0,20,40,60,80,100,120,140,200,500}; r = {(sizeof(bins_met_c) / sizeof(bins_met_c[0]))-1, false,  -1, -1, bins_met_c};   m["met_c"]  = {distr_met, r};

	return m;
}

/** \brief The known distributions.
 */

map<const char*, TH1D_def> known_defs_distrs = create_known_TH1D_distr_definitions();


/*
 * 
 */


/** \brief A helper function creating the instances of TH1D_distrs with a specific name from the given TH1D_def definition.

Creates a `new TH1D(name, ..., range)` according to the linear or custom range in the TH1D_def definition.

\param  TH1D_def& def
\param  TString   name
\return TH1D_distr
 */

TH1D_distr create_TH1D_distr(TH1D_def& def, TString name)
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
	//TH1D_distr a_distr = {ahist, def.func, 0.};
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

bool channel_mu_sel(Systematics sys)
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

bool channel_mu_sel_ss(Systematics sys)
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

bool channel_el_sel(Systematics sys)
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

bool channel_el_sel_ss(Systematics sys)
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


bool channel_tt_elmu(Systematics sys)
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

bool channel_tt_elmu_tight(Systematics sys)
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


/** \brief The channel-defining `bool` function.

Each final state channel is defined by a `bool` function,
that is calculated in the name space of the input `TTree` branches.
It takes the `Systematics` identifier as input,
and returns `bool` whether event passes this channel selection or not.
 */
typedef bool (*channel_def_func)(Systematics);


/** \brief The initialization function for the `bool` functions of the known distributions.

\return map<const char*, channel_def_func>
 */

#define quick_set_chandef(m, chan_name) m[#chan_name] = channel_ ## chan_name
map<const char*, channel_def_func> create_known_channel_definitions()
{
	map<const char*, bool (*)(Systematics)> m;
	//m["el_sel"] = channel_el_sel;
	quick_set_chandef(m, el_sel);
	quick_set_chandef(m, el_sel_ss);
	quick_set_chandef(m, mu_sel);
	quick_set_chandef(m, mu_sel_ss);

	quick_set_chandef(m, tt_elmu);
	quick_set_chandef(m, tt_elmu_tight);

	return m;
}


/** \brief The known channels.
 */
map<const char*, channel_def_func> known_defs_channels = create_known_channel_definitions();


/** \brief A channel definition with corresponding output histograms.

Each final state channel is defined by a `bool` function and holds a list of histograms to record.
 */

typedef struct{
	channel_def_func chan_def; /**< \brief `bool` function defining whether an event passes the channel selection */
	vector<TH1D_distr> distrs; /**< \brief distributions to record in this final state channel */
} channel_histos;

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

//// define histograms for the distributions
//vector<TH1D_distr> distrs;

// define a nested list: list of channels, each containing a list of histograms to record
vector<channel_histos> requested_channels;


// final state channels
const char* requested_channel_names[] = {"el_sel", "mu_sel", "tt_elmu", "tt_elmu_tight", NULL};
for (const char** requested_channel = requested_channel_names; *requested_channel != NULL; requested_channel++)
	{

	// distributions
	vector<TH1D_distr> distrs;
	//TH1D* ahist = (TH1D*) new TH1D("lep_pt", "lep_pt", 40, 0, 200);
	//TH1D_distr a_distr = {ahist, leading_lep_pt, 0.};

	// test
	//cout_expr(TString(*requested_channel));
	//cout_expr(TString(*requested_channel) + "_anyProc_NOMINAL_" + "leading_lep_pt");

	TH1D_distr a_distr;
	//a_distr = create_TH1D_distr(known_defs_distrs["leading_lep_pt"], TString(*requested_channel) + "_anyProc_NOMINAL_" + "leading_lep_pt");
	//distrs.push_back(a_distr);
	a_distr = create_TH1D_distr(known_defs_distrs["Mt_lep_met_c"], TString(*requested_channel) + "_anyProc_NOMINAL_" + "Mt_lep_met_c");
	distrs.push_back(a_distr);

	requested_channels.push_back({known_defs_channels[*requested_channel], distrs});
	}

// process input files
unsigned int cur_var = 0;
for (; cur_var<argc; cur_var++)
	{
	TString input_filename(argv[cur_var]);

	cout_expr(cur_var);
	cout_expr(input_filename);

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

	//// just a test
	//if (NT_output_ttree) cout_expr(NT_output_ttree->GetEntries());
	//else cout << "NO TTREE" << std::endl;

	unsigned int n_entries = NT_output_ttree->GetEntries();
	cout_expr(n_entries);

	for (unsigned int ievt = 0; ievt < n_entries; ievt++)
		{
		NT_output_ttree->GetEntry(ievt);

		//// tests
		////cout_expr(NT_runNumber);
		cout_expr(NT_event_leptons[0].pt());
		//cout_expr(NT_event_leptons_genmatch[0]);

		Stopif(ievt > 10, break, "reached 10 events, exiting");

/*
in Python:

            # calculate parameters for the requested distrs
            for dname, distr_func, _ in chan_distrs:
                if dname in event_distrs:
                    param = event_distrs[dname]
                else:
                    param = distr_func.get(sname, distr_func['NOMINAL'])(ev)

                # record along the way
                #histos[(chan_name, event_chan_proc, sname, dname)].Fill(param, weight)
                record(histos, (chan_name, event_chan_proc, sname, dname), param, weight)
                # 3 loops deep calculation


# separated the procedure for profilimng performance
def record(histos, def_tuple, param, weight):
    histos[def_tuple].Fill(param, weight)

what we need is...
-- that and + 2D distributions
*/

		// set the systematic
		Systematics systematic = NOMINAL;

		/*
		for (int distr_i = 0; distr_i<distrs.size(); distr_i++)
			{
			// TODO memoize, optimize
			double val = distrs[distr_i].func(systematic);
			distrs[distr_i].histo->Fill(val);
			}
		*/

		for (int chan_i = 0; chan_i<requested_channels.size(); chan_i++)
			{
			// test the event passed the final state channel event selection
			if (!requested_channels[chan_i].chan_def(systematic)) continue;

			// fill in the histograms
			vector<TH1D_distr>& distrs = requested_channels[chan_i].distrs;
			for (int distr_i = 0; distr_i<distrs.size(); distr_i++)
				{
				// TODO memoize, optimize
				double val = distrs[distr_i].func(systematic);
				distrs[distr_i].histo->Fill(val);
				}
			}

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

for (int chan_i = 0; chan_i<requested_channels.size(); chan_i++)
	{
	//TODO create nested TDirectories for easier manual management, or do they slow down hadd?

	// output the histograms
	vector<TH1D_distr>& distrs = requested_channels[chan_i].distrs;
	for (int distr_i = 0; distr_i<distrs.size(); distr_i++)
		{
		// TODO memoize, optimize
		distrs[distr_i].histo->Print();
		distrs[distr_i].histo->Write();
		}
	}

output_file->Close();
}


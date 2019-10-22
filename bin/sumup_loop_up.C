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

/** Set this to \c 's' to stop the program on error. Otherwise execure error actions. */
char error_mode = 'a';

/** where to write errors? if this is \c NULL, write to \c stderr. */
FILE * error_log;
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

/* the processing functions in the name space of the stage2 interface
 */
//#include "std_defs.h"

enum Systematics {NOMINAL, JERUp, JERDown, JESUp, JESDown, TESUp, TESDown};

// one struct for TH1D ranges, linear and custom bins
typedef struct {
	unsigned int nbins;
	bool linear = true;
	double linear_min;
	double linear_max;
	double custom_bins[];
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


typedef struct {
	double (*func)(Systematics);
	TH1D_range range;
} TH1D_def;

typedef struct {
	TH1D* histo;
	double (*func)(Systematics);
	double value;
} TH1D_distr;

double leading_lep_pt(Systematics sys)
	{
	return NT_event_leptons[0].pt();
	}

double Mt_lep_met(Systematics sys)
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

double met(Systematics sys)
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


// name, def
map<const char*, TH1D_def> create_TH1D_distr_definitions()
{
	map<const char*, TH1D_def> m;
	TH1D_range r;
	//r = {.nbins=40, .linear_min=0, .linear_max=200}; m["leading_lep_pt"] = {leading_lep_pt, r};
	// despicable!
	// "sorry, unimplemented: non-trivial designated initializers not supported"
	r = {40, true,  0, 200};                                                     m["leading_lep_pt"] = {leading_lep_pt, r};
	r = {14, false, 0,   0, {0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}}; m["Mt_lep_met_c"]   = {Mt_lep_met, r};
	r = {20, false, 0, 250};                                                     m["Mt_lep_met_f"]   = {Mt_lep_met, r};

	//set_range_linear(r, 40, 0, 200);                                         m["leading_lep_pt"] = {leading_lep_pt, r};
	//set_range_custom(r, (double[]){0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}); m["Mt_lep_met_c"]   = {Mt_lep_met, r};

	// these also do not work:
	//m["leading_lep_pt"] = {leading_lep_pt, range_linear(40, 0, 200)};
	////m["Mt_lep_met_c"]   = {Mt_lep_met, range_custom({0,16,32,44,54,64,74,81,88,95,104,116,132,160,250})};
	//m["Mt_lep_met_c"]   = {Mt_lep_met, range_custom_base({0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}, 14)};
	//m["Mt_lep_met_f"]   = {Mt_lep_met, range_linear(20, 0, 250)};

	return m;
}

map<const char*, TH1D_def> std_defs_distrs = create_TH1D_distr_definitions();

TH1D_distr create_TH1D_distr(TH1D_def& def, TString name)
{
	TH1D* histo;
	if (def.range.linear)
		histo = (TH1D*) new TH1D(name, name, def.range.nbins, def.range.linear_min, def.range.linear_max);
	else
		histo = (TH1D*) new TH1D(name, name, def.range.nbins, def.range.custom_bins);
	//TH1D_distr a_distr = {ahist, def.func, 0.};
	//distrs.push_back(a_distr);
	return {histo, def.func, 0.};
}

/* --------------------------------------------------------------- */


//#define DTAG_ARGS_START 5

//int stacked_histo_distr (int argc, char *argv[])
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

unsigned int cur_var = 0;

//Int_t rebin_factor(atoi(argv[3]));
//TString dir(argv[4]);
//TString dtag1(argv[5]);

// define histograms for the distributions
vector<TH1D_distr> distrs;

/* Here we will parse user's request for figures
 * to create the structure filled up in the event loop.
 */

{
	//TH1D* ahist = (TH1D*) new TH1D("lep_pt", "lep_pt", 40, 0, 200);
	//TH1D_distr a_distr = {ahist, leading_lep_pt, 0.};
	TH1D_distr a_distr = create_TH1D_distr(std_defs_distrs["leading_lep_pt"], "inclChan_anyProc_NOMINAL_" "leading_lep_pt");
	distrs.push_back(a_distr);
	a_distr = create_TH1D_distr(std_defs_distrs["Mt_lep_met_c"], "inclChan_anyProc_NOMINAL_" "Mt_lep_met_c");
	distrs.push_back(a_distr);
}

// process input files
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

		for (int distr_i = 0; distr_i<distrs.size(); distr_i++)
			{
			// TODO memoize, optimize
			double val = distrs[distr_i].func(systematic);
			distrs[distr_i].histo->Fill(val);
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

for (int distr_i = 0; distr_i<distrs.size(); distr_i++)
	{
	distrs[distr_i].histo->Print(); // for the tests
	distrs[distr_i].histo->Write();
	}

output_file->Close();
}


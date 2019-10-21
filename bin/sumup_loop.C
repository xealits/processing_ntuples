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


double leading_lep_pt()
	{
	return NT_event_leptons[0].pt();
	}

typedef struct {
	TH1D* histo;
	double (*func)(void);
	double value;
} TH1D_distr;


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

TH1D* ahist = (TH1D*) new TH1D("lep_pt", "lep_pt", 40, 0, 200);
TH1D_distr a_distr = {ahist, leading_lep_pt, 0.};
distrs.push_back(a_distr);

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

		for (int distr_i = 0; distr_i<distrs.size(); distr_i++)
			{
			// TODO memoize, optimize
			double val = distrs[distr_i].func();
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
	distrs[distr_i].histo->Write();
	}

output_file->Close();
}


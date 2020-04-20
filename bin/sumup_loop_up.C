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

#include "TMath.h" // Cos

#include <map>
#include <string>
#include <vector>
#include <stdlib.h> // abort

using namespace std;

// TODO: turn these into arguments for main
bool do_not_overwrite     = true;
bool normalise_per_weight = true;

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


double NT_sysweight_NOMINAL_HLT_EL()
	{
	return NT_event_weight*NT_event_weight_PU*NT_event_weight_LEPmuID*NT_event_weight_LEPelID* NT_event_weight_LEPelTRG;
	}

double NT_sysweight_NOMINAL_HLT_MU()
	{
	return NT_event_weight*NT_event_weight_PU*NT_event_weight_LEPmuID*NT_event_weight_LEPelID* NT_event_weight_LEPmuTRG;
	}

double NT_sysweight_NOMINAL_HLT_LEP()
	{
	double common = NT_event_weight*NT_event_weight_PU*NT_event_weight_LEPmuID*NT_event_weight_LEPelID;
	if (abs(NT_event_leptons_ids[0]) == 13)
		return  NT_event_weight_LEPmuTRG * common;
	else
		return  NT_event_weight_LEPelTRG * common;
	}

double NT_sysweight_NOMINAL_HLT_EL_MedTau()
	{
	return NT_event_weight*NT_event_weight_PU*NT_event_weight_LEPmuID*NT_event_weight_LEPelID* NT_event_weight_LEPelTRG * NT_event_taus_SF_Medium[0];
	}

double NT_sysweight_NOMINAL_HLT_MU_MedTau()
	{
	return NT_event_weight*NT_event_weight_PU*NT_event_weight_LEPmuID*NT_event_weight_LEPelID* NT_event_weight_LEPmuTRG * NT_event_taus_SF_Medium[0];
	}

double NT_sysweight_NOMINAL_HLT_LEP_MedTau()
	{
	double common = NT_event_weight*NT_event_weight_PU*NT_event_weight_LEPmuID*NT_event_weight_LEPelID * NT_event_taus_SF_Medium[0];
	if (abs(NT_event_leptons_ids[0]) == 13)
		return  NT_event_weight_LEPmuTRG * common;
	else
		return  NT_event_weight_LEPelTRG * common;
	}

// systematic uncertainties from the nominal weight

double NT_sysweight_NOMINAL()
	{
	return 1.;
	}

typedef double (*_F_sysweight)();
typedef struct {ObjSystematics obj_sys_id; _F_sysweight weight_func;} _S_systematic_definition;

#define NT_sysweight(sysname, weight_expr)   \
double NT_sysweight_ ##sysname(void)          \
	{                                  \
	return weight_expr; \
	}

// COMMON systematics
NT_sysweight(PUUp,   NT_event_weight_PUUp   / NT_event_weight_PU )
NT_sysweight(PUDown, NT_event_weight_PUDown / NT_event_weight_PU )

NT_sysweight(bSFUp,    (NT_event_weight_bSF > 0.? NT_event_weight_bSFUp   / NT_event_weight_bSF : 0.))
NT_sysweight(bSFDown,  (NT_event_weight_bSF > 0.? NT_event_weight_bSFDown / NT_event_weight_bSF : 0.))

NT_sysweight(LEPelIDUp,    NT_event_weight_LEPelID_Up    / NT_event_weight_LEPelID)
NT_sysweight(LEPelIDDown,  NT_event_weight_LEPelID_Down  / NT_event_weight_LEPelID)
NT_sysweight(LEPelTRGUp,   NT_event_weight_LEPelTRG_Up   / NT_event_weight_LEPelTRG)
NT_sysweight(LEPelTRGDown, NT_event_weight_LEPelTRG_Down / NT_event_weight_LEPelTRG)
NT_sysweight(LEPmuIDUp,    NT_event_weight_LEPmuID_Up    / NT_event_weight_LEPmuID)
NT_sysweight(LEPmuIDDown,  NT_event_weight_LEPmuID_Down  / NT_event_weight_LEPmuID)
NT_sysweight(LEPmuTRGUp,   NT_event_weight_LEPmuTRG_Up   / NT_event_weight_LEPmuTRG)
NT_sysweight(LEPmuTRGDown, NT_event_weight_LEPmuTRG_Down / NT_event_weight_LEPmuTRG)

// Tau systematics

NT_sysweight(TAUIDUp,   (NT_event_taus_SF_Medium_Up   .size()>0 && NT_event_taus_SF_Medium[0] > 0.001) ? NT_event_taus_SF_Medium_Up[0]    / NT_event_taus_SF_Medium[0] : 1.)
NT_sysweight(TAUIDDown, (NT_event_taus_SF_Medium_Down .size()>0 && NT_event_taus_SF_Medium[0] > 0.001) ? NT_event_taus_SF_Medium_Down[0]  / NT_event_taus_SF_Medium[0] : 1.)

// TT_OBJ
NT_sysweight(TOPPTUp   , NT_event_weight_toppt    )
NT_sysweight(TOPPTDown , 1.    )

NT_sysweight(FragUp        , NT_event_weight_FragUp        )
NT_sysweight(FragDown      , NT_event_weight_FragDown      )
NT_sysweight(SemilepBRUp   , NT_event_weight_SemilepBRUp   )
NT_sysweight(SemilepBRDown , NT_event_weight_SemilepBRDown )
NT_sysweight(PetersonUp    , NT_event_weight_PetersonUp    )
NT_sysweight(PetersonDown  , 1.                               )

// TT_HARD
NT_sysweight(MrUp     , NT_event_weight_me_f_rUp )
NT_sysweight(MrDown   , NT_event_weight_me_f_rDn )
NT_sysweight(MfUp     , NT_event_weight_me_fUp_r )
NT_sysweight(MfDown   , NT_event_weight_me_fDn_r )
NT_sysweight(MfrUp    , NT_event_weight_me_frUp  ) 
NT_sysweight(MfrDown  , NT_event_weight_me_frDn  ) 

// TT_ALPHA
NT_sysweight(AlphaSUp     , NT_event_weight_AlphaS_up )
NT_sysweight(AlphaSDown   , NT_event_weight_AlphaS_dn )

// TT_PDF all of them
// maybe a better approach would be to add an index into systematics

NT_sysweight(PDFCT14n1Up     , NT_event_weight_pdf[0] )
NT_sysweight(PDFCT14n2Up     , NT_event_weight_pdf[1] )
NT_sysweight(PDFCT14n3Up     , NT_event_weight_pdf[2] )
NT_sysweight(PDFCT14n4Up     , NT_event_weight_pdf[3] )
NT_sysweight(PDFCT14n5Up     , NT_event_weight_pdf[4] )
NT_sysweight(PDFCT14n6Up     , NT_event_weight_pdf[5] )
NT_sysweight(PDFCT14n7Up     , NT_event_weight_pdf[6] )
NT_sysweight(PDFCT14n8Up     , NT_event_weight_pdf[7] )
NT_sysweight(PDFCT14n9Up     , NT_event_weight_pdf[8] )
NT_sysweight(PDFCT14n10Up    , NT_event_weight_pdf[9] )
NT_sysweight(PDFCT14n11Up    , NT_event_weight_pdf[10])
NT_sysweight(PDFCT14n12Up    , NT_event_weight_pdf[11])
NT_sysweight(PDFCT14n13Up    , NT_event_weight_pdf[12])
NT_sysweight(PDFCT14n14Up    , NT_event_weight_pdf[13])
NT_sysweight(PDFCT14n15Up    , NT_event_weight_pdf[14])
NT_sysweight(PDFCT14n16Up    , NT_event_weight_pdf[15])
NT_sysweight(PDFCT14n17Up    , NT_event_weight_pdf[16])
NT_sysweight(PDFCT14n18Up    , NT_event_weight_pdf[17])
NT_sysweight(PDFCT14n19Up    , NT_event_weight_pdf[18])
NT_sysweight(PDFCT14n20Up    , NT_event_weight_pdf[19])
NT_sysweight(PDFCT14n21Up    , NT_event_weight_pdf[20])
NT_sysweight(PDFCT14n22Up    , NT_event_weight_pdf[21])
NT_sysweight(PDFCT14n23Up    , NT_event_weight_pdf[22])
NT_sysweight(PDFCT14n24Up    , NT_event_weight_pdf[23])
NT_sysweight(PDFCT14n25Up    , NT_event_weight_pdf[24])
NT_sysweight(PDFCT14n26Up    , NT_event_weight_pdf[25])
NT_sysweight(PDFCT14n27Up    , NT_event_weight_pdf[26])
NT_sysweight(PDFCT14n28Up    , NT_event_weight_pdf[27])
NT_sysweight(PDFCT14n29Up    , NT_event_weight_pdf[28])
NT_sysweight(PDFCT14n30Up    , NT_event_weight_pdf[29])
NT_sysweight(PDFCT14n31Up    , NT_event_weight_pdf[30])
NT_sysweight(PDFCT14n32Up    , NT_event_weight_pdf[31])
NT_sysweight(PDFCT14n33Up    , NT_event_weight_pdf[32])
NT_sysweight(PDFCT14n34Up    , NT_event_weight_pdf[33])
NT_sysweight(PDFCT14n35Up    , NT_event_weight_pdf[34])
NT_sysweight(PDFCT14n36Up    , NT_event_weight_pdf[35])
NT_sysweight(PDFCT14n37Up    , NT_event_weight_pdf[36])
NT_sysweight(PDFCT14n38Up    , NT_event_weight_pdf[37])
NT_sysweight(PDFCT14n39Up    , NT_event_weight_pdf[38])
NT_sysweight(PDFCT14n40Up    , NT_event_weight_pdf[39])
NT_sysweight(PDFCT14n41Up    , NT_event_weight_pdf[40])
NT_sysweight(PDFCT14n42Up    , NT_event_weight_pdf[41])
NT_sysweight(PDFCT14n43Up    , NT_event_weight_pdf[42])
NT_sysweight(PDFCT14n44Up    , NT_event_weight_pdf[43])
NT_sysweight(PDFCT14n45Up    , NT_event_weight_pdf[44])
NT_sysweight(PDFCT14n46Up    , NT_event_weight_pdf[45])
NT_sysweight(PDFCT14n47Up    , NT_event_weight_pdf[46])
NT_sysweight(PDFCT14n48Up    , NT_event_weight_pdf[47])
NT_sysweight(PDFCT14n49Up    , NT_event_weight_pdf[48])
NT_sysweight(PDFCT14n50Up    , NT_event_weight_pdf[49])
NT_sysweight(PDFCT14n51Up    , NT_event_weight_pdf[50])
NT_sysweight(PDFCT14n52Up    , NT_event_weight_pdf[51])
NT_sysweight(PDFCT14n53Up    , NT_event_weight_pdf[52])
NT_sysweight(PDFCT14n54Up    , NT_event_weight_pdf[53])
NT_sysweight(PDFCT14n55Up    , NT_event_weight_pdf[54])
NT_sysweight(PDFCT14n56Up    , NT_event_weight_pdf[55])

NT_sysweight(PDFCT14n1Down     , 1.)
NT_sysweight(PDFCT14n2Down     , 1.)
NT_sysweight(PDFCT14n3Down     , 1.)
NT_sysweight(PDFCT14n4Down     , 1.)
NT_sysweight(PDFCT14n5Down     , 1.)
NT_sysweight(PDFCT14n6Down     , 1.)
NT_sysweight(PDFCT14n7Down     , 1.)
NT_sysweight(PDFCT14n8Down     , 1.)
NT_sysweight(PDFCT14n9Down     , 1.)
NT_sysweight(PDFCT14n10Down    , 1.)
NT_sysweight(PDFCT14n11Down    , 1.)
NT_sysweight(PDFCT14n12Down    , 1.)
NT_sysweight(PDFCT14n13Down    , 1.)
NT_sysweight(PDFCT14n14Down    , 1.)
NT_sysweight(PDFCT14n15Down    , 1.)
NT_sysweight(PDFCT14n16Down    , 1.)
NT_sysweight(PDFCT14n17Down    , 1.)
NT_sysweight(PDFCT14n18Down    , 1.)
NT_sysweight(PDFCT14n19Down    , 1.)
NT_sysweight(PDFCT14n20Down    , 1.)
NT_sysweight(PDFCT14n21Down    , 1.)
NT_sysweight(PDFCT14n22Down    , 1.)
NT_sysweight(PDFCT14n23Down    , 1.)
NT_sysweight(PDFCT14n24Down    , 1.)
NT_sysweight(PDFCT14n25Down    , 1.)
NT_sysweight(PDFCT14n26Down    , 1.)
NT_sysweight(PDFCT14n27Down    , 1.)
NT_sysweight(PDFCT14n28Down    , 1.)
NT_sysweight(PDFCT14n29Down    , 1.)
NT_sysweight(PDFCT14n30Down    , 1.)
NT_sysweight(PDFCT14n31Down    , 1.)
NT_sysweight(PDFCT14n32Down    , 1.)
NT_sysweight(PDFCT14n33Down    , 1.)
NT_sysweight(PDFCT14n34Down    , 1.)
NT_sysweight(PDFCT14n35Down    , 1.)
NT_sysweight(PDFCT14n36Down    , 1.)
NT_sysweight(PDFCT14n37Down    , 1.)
NT_sysweight(PDFCT14n38Down    , 1.)
NT_sysweight(PDFCT14n39Down    , 1.)
NT_sysweight(PDFCT14n40Down    , 1.)
NT_sysweight(PDFCT14n41Down    , 1.)
NT_sysweight(PDFCT14n42Down    , 1.)
NT_sysweight(PDFCT14n43Down    , 1.)
NT_sysweight(PDFCT14n44Down    , 1.)
NT_sysweight(PDFCT14n45Down    , 1.)
NT_sysweight(PDFCT14n46Down    , 1.)
NT_sysweight(PDFCT14n47Down    , 1.)
NT_sysweight(PDFCT14n48Down    , 1.)
NT_sysweight(PDFCT14n49Down    , 1.)
NT_sysweight(PDFCT14n50Down    , 1.)
NT_sysweight(PDFCT14n51Down    , 1.)
NT_sysweight(PDFCT14n52Down    , 1.)
NT_sysweight(PDFCT14n53Down    , 1.)
NT_sysweight(PDFCT14n54Down    , 1.)
NT_sysweight(PDFCT14n55Down    , 1.)
NT_sysweight(PDFCT14n56Down    , 1.)



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

	_quick_set_wgtsys(TAUIDUp);
	_quick_set_wgtsys(TAUIDDown);

	_quick_set_wgtsys(PUUp);
	_quick_set_wgtsys(PUDown);

	_quick_set_wgtsys(TOPPTUp);
	_quick_set_wgtsys(TOPPTDown);
	_quick_set_wgtsys(FragUp);
	_quick_set_wgtsys(FragDown);
	_quick_set_wgtsys(SemilepBRUp);
	_quick_set_wgtsys(SemilepBRDown);
	_quick_set_wgtsys(PetersonUp);
	_quick_set_wgtsys(PetersonDown);

	_quick_set_wgtsys(MrUp     );
	_quick_set_wgtsys(MrDown   );
	_quick_set_wgtsys(MfUp     );
	_quick_set_wgtsys(MfDown   );
	_quick_set_wgtsys(MfrUp    );
	_quick_set_wgtsys(MfrDown  );

	_quick_set_wgtsys(AlphaSUp     );
	_quick_set_wgtsys(AlphaSDown   );

	_quick_set_wgtsys(PDFCT14n1Up     );
	_quick_set_wgtsys(PDFCT14n2Up     );
	_quick_set_wgtsys(PDFCT14n3Up     );
	_quick_set_wgtsys(PDFCT14n4Up     );
	_quick_set_wgtsys(PDFCT14n5Up     );
	_quick_set_wgtsys(PDFCT14n6Up     );
	_quick_set_wgtsys(PDFCT14n7Up     );
	_quick_set_wgtsys(PDFCT14n8Up     );
	_quick_set_wgtsys(PDFCT14n9Up     );
	_quick_set_wgtsys(PDFCT14n10Up    );
	_quick_set_wgtsys(PDFCT14n11Up    );
	_quick_set_wgtsys(PDFCT14n12Up    );
	_quick_set_wgtsys(PDFCT14n13Up    );
	_quick_set_wgtsys(PDFCT14n14Up    );
	_quick_set_wgtsys(PDFCT14n15Up    );
	_quick_set_wgtsys(PDFCT14n16Up    );
	_quick_set_wgtsys(PDFCT14n17Up    );
	_quick_set_wgtsys(PDFCT14n18Up    );
	_quick_set_wgtsys(PDFCT14n19Up    );
	_quick_set_wgtsys(PDFCT14n20Up    );
	_quick_set_wgtsys(PDFCT14n21Up    );
	_quick_set_wgtsys(PDFCT14n22Up    );
	_quick_set_wgtsys(PDFCT14n23Up    );
	_quick_set_wgtsys(PDFCT14n24Up    );
	_quick_set_wgtsys(PDFCT14n25Up    );
	_quick_set_wgtsys(PDFCT14n26Up    );
	_quick_set_wgtsys(PDFCT14n27Up    );
	_quick_set_wgtsys(PDFCT14n28Up    );
	_quick_set_wgtsys(PDFCT14n29Up    );
	_quick_set_wgtsys(PDFCT14n30Up    );
	_quick_set_wgtsys(PDFCT14n31Up    );
	_quick_set_wgtsys(PDFCT14n32Up    );
	_quick_set_wgtsys(PDFCT14n33Up    );
	_quick_set_wgtsys(PDFCT14n34Up    );
	_quick_set_wgtsys(PDFCT14n35Up    );
	_quick_set_wgtsys(PDFCT14n36Up    );
	_quick_set_wgtsys(PDFCT14n37Up    );
	_quick_set_wgtsys(PDFCT14n38Up    );
	_quick_set_wgtsys(PDFCT14n39Up    );
	_quick_set_wgtsys(PDFCT14n40Up    );
	_quick_set_wgtsys(PDFCT14n41Up    );
	_quick_set_wgtsys(PDFCT14n42Up    );
	_quick_set_wgtsys(PDFCT14n43Up    );
	_quick_set_wgtsys(PDFCT14n44Up    );
	_quick_set_wgtsys(PDFCT14n45Up    );
	_quick_set_wgtsys(PDFCT14n46Up    );
	_quick_set_wgtsys(PDFCT14n47Up    );
	_quick_set_wgtsys(PDFCT14n48Up    );
	_quick_set_wgtsys(PDFCT14n49Up    );
	_quick_set_wgtsys(PDFCT14n50Up    );
	_quick_set_wgtsys(PDFCT14n51Up    );
	_quick_set_wgtsys(PDFCT14n52Up    );
	_quick_set_wgtsys(PDFCT14n53Up    );
	_quick_set_wgtsys(PDFCT14n54Up    );
	_quick_set_wgtsys(PDFCT14n55Up    );
	_quick_set_wgtsys(PDFCT14n56Up    );

	_quick_set_wgtsys(PDFCT14n1Down     );
	_quick_set_wgtsys(PDFCT14n2Down     );
	_quick_set_wgtsys(PDFCT14n3Down     );
	_quick_set_wgtsys(PDFCT14n4Down     );
	_quick_set_wgtsys(PDFCT14n5Down     );
	_quick_set_wgtsys(PDFCT14n6Down     );
	_quick_set_wgtsys(PDFCT14n7Down     );
	_quick_set_wgtsys(PDFCT14n8Down     );
	_quick_set_wgtsys(PDFCT14n9Down     );
	_quick_set_wgtsys(PDFCT14n10Down    );
	_quick_set_wgtsys(PDFCT14n11Down    );
	_quick_set_wgtsys(PDFCT14n12Down    );
	_quick_set_wgtsys(PDFCT14n13Down    );
	_quick_set_wgtsys(PDFCT14n14Down    );
	_quick_set_wgtsys(PDFCT14n15Down    );
	_quick_set_wgtsys(PDFCT14n16Down    );
	_quick_set_wgtsys(PDFCT14n17Down    );
	_quick_set_wgtsys(PDFCT14n18Down    );
	_quick_set_wgtsys(PDFCT14n19Down    );
	_quick_set_wgtsys(PDFCT14n20Down    );
	_quick_set_wgtsys(PDFCT14n21Down    );
	_quick_set_wgtsys(PDFCT14n22Down    );
	_quick_set_wgtsys(PDFCT14n23Down    );
	_quick_set_wgtsys(PDFCT14n24Down    );
	_quick_set_wgtsys(PDFCT14n25Down    );
	_quick_set_wgtsys(PDFCT14n26Down    );
	_quick_set_wgtsys(PDFCT14n27Down    );
	_quick_set_wgtsys(PDFCT14n28Down    );
	_quick_set_wgtsys(PDFCT14n29Down    );
	_quick_set_wgtsys(PDFCT14n30Down    );
	_quick_set_wgtsys(PDFCT14n31Down    );
	_quick_set_wgtsys(PDFCT14n32Down    );
	_quick_set_wgtsys(PDFCT14n33Down    );
	_quick_set_wgtsys(PDFCT14n34Down    );
	_quick_set_wgtsys(PDFCT14n35Down    );
	_quick_set_wgtsys(PDFCT14n36Down    );
	_quick_set_wgtsys(PDFCT14n37Down    );
	_quick_set_wgtsys(PDFCT14n38Down    );
	_quick_set_wgtsys(PDFCT14n39Down    );
	_quick_set_wgtsys(PDFCT14n40Down    );
	_quick_set_wgtsys(PDFCT14n41Down    );
	_quick_set_wgtsys(PDFCT14n42Down    );
	_quick_set_wgtsys(PDFCT14n43Down    );
	_quick_set_wgtsys(PDFCT14n44Down    );
	_quick_set_wgtsys(PDFCT14n45Down    );
	_quick_set_wgtsys(PDFCT14n46Down    );
	_quick_set_wgtsys(PDFCT14n47Down    );
	_quick_set_wgtsys(PDFCT14n48Down    );
	_quick_set_wgtsys(PDFCT14n49Down    );
	_quick_set_wgtsys(PDFCT14n50Down    );
	_quick_set_wgtsys(PDFCT14n51Down    );
	_quick_set_wgtsys(PDFCT14n52Down    );
	_quick_set_wgtsys(PDFCT14n53Down    );
	_quick_set_wgtsys(PDFCT14n54Down    );
	_quick_set_wgtsys(PDFCT14n55Down    );
	_quick_set_wgtsys(PDFCT14n56Down    );

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
	TString main_name;
	TH1D* histo;
	double (*func)(ObjSystematics);
	double value;
} TH1D_histo;

double NT_distr_nvtx(ObjSystematics sys)
	{
	return NT_nvtx;
	}

double NT_distr_leading_lep_pt(ObjSystematics sys)
	{
	return NT_event_leptons[0].pt();
	}

double NT_distr_dilep_mass(ObjSystematics sys)
	{
	if (NT_event_leptons.size()>1)
		return (NT_event_leptons[0] + NT_event_leptons[1]).mass();
	else if (NT_event_taus.size()>0)
		return (NT_event_leptons[0] + NT_event_taus[0]).mass();
	else
		return -111.;
	}

double NT_distr_tau_pt(ObjSystematics sys)
	{

	// from stage2.py and std_defs.py:
	//'tau_pt':        ({'NOMINAL': lambda ev: ev.event_taus[0].pt()    if len(ev.event_taus) > 0   else -111.},    ('histo-range',  [40,0,200])),
	//'event_taus_TES_up' : 'DoubleVector',
	//'event_taus_TES_down' : 'DoubleVector',
	//p4 = tau[0] * TES_nom
	//event_taus          .push_back(p4)
	//event_taus_TES_up   .push_back(TES_up / TES_nom)
	//event_taus_TES_down .push_back(TES_down / TES_nom)

	if (NT_event_taus.size()==0)
		return -111.;

	double pt = NT_event_taus[0].pt();
	if      (sys == TESUp)   return pt * NT_event_taus_TES_up[0];
	else if (sys == TESDown) return pt * NT_event_taus_TES_down[0];
	else                     return pt;
	}

double NT_distr_tau_sv_sign(ObjSystematics sys)
	{
	if (NT_event_taus_sv_sign.size()>0)
		return NT_event_taus_sv_sign[0];
	else
		return -111.;
	}

double NT_distr_sum_cos(ObjSystematics sys)
	{
	if (NT_event_leptons.size() == 0 || NT_event_taus.size() == 0) return -11.;
	double cos_lep_met = TMath::Cos(NT_event_leptons[0].Phi() - NT_event_met.Phi());
	double cos_tau_met = TMath::Cos(NT_event_taus[0]   .Phi() - NT_event_met.Phi());
	return cos_lep_met + cos_tau_met;
	}


double NT_distr_lj_var(ObjSystematics sys)
	{
	return NT_event_jets_lj_var;
	}

double NT_distr_lj_var_w_mass(ObjSystematics sys)
	{
	return NT_event_jets_lj_w_mass;
	}

double NT_distr_lj_var_t_mass(ObjSystematics sys)
	{
	return NT_event_jets_lj_t_mass;
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

	r = {50,  true,   0, 50};                                                      m["nvtx"] = {NT_distr_nvtx, r};

	r = {40,  true,   0, 200};                                                     m["leading_lep_pt"] = {NT_distr_leading_lep_pt, r};
	r = {40,  true,  30,  40};                                                     m["leading_lep_pt_el_edge35"] = {NT_distr_leading_lep_pt, r};
	r = {40,  true,   0, 200};                                                     m["tau_pt"]         = {NT_distr_tau_pt, r};
	// taus have smaller energy in ttbar, therefore we might want to look at a smaller range
	r = {40,  true,   0, 150};                                                     m["tau_pt_range2"]  = {NT_distr_tau_pt, r};
	r = {21,  true,  -1,  20};                                                     m["tau_sv_sign"]    = {NT_distr_tau_sv_sign, r};

	r = {100, true,   0, 400};                                                     m["dilep_mass"]     = {NT_distr_dilep_mass, r};
	r = {40,  true,  80, 100};                                                     m["dilep_mass_dy"]  = {NT_distr_dilep_mass, r};
	r = {40,  true,  20, 120};                                                     m["dilep_mass_dy_tautau"]  = {NT_distr_dilep_mass, r};

	r = {40,  true, -2., 2.};                                                      m["sum_cos"]        = {NT_distr_sum_cos, r};

	static double bins_lj_var[] = {0,15,30,45,60,90,120,170,220,270,400};   r = {(sizeof(bins_lj_var) / sizeof(bins_lj_var[0]))-1, false,-1,  -1, bins_lj_var};   m["lj_var"] = {NT_distr_lj_var, r};
	static double bins_lj_var_w_mass[] = {10,40,65,80,95,120,150,200};      r = {(sizeof(bins_lj_var_w_mass) / sizeof(bins_lj_var_w_mass[0]))-1, false,-1,  -1, bins_lj_var_w_mass};   m["lj_var_w_mass"] = {NT_distr_lj_var_w_mass, r};
	static double bins_lj_var_t_mass[] = {20,100,130,160,180,200,300,400};  r = {(sizeof(bins_lj_var_t_mass) / sizeof(bins_lj_var_t_mass[0]))-1, false,-1,  -1, bins_lj_var_t_mass};   m["lj_var_t_mass"] = {NT_distr_lj_var_t_mass, r};

	//r = {14, false,-1,  -1, .custom_bins=(double[]){0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//r = {2, false,-1,  -1, .custom_bins=(double[]){{0},{16},{32}}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//r = {2, false,-1,  -1, {0.,16.,32.}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	// it seems gcc on lxplus is buggy
	//static double Mt_lep_met_c_bins[] = {0,16,32,44,54,64};
	//r = {5, false,-1,  -1}; r.custom_bins = Mt_lep_met_c_bins; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	// -- ok, this works
	//r = {2, false,-1,  -1, (static double*){0.,16.,32.}}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	//static double Mt_lep_met_c_bins[] = {0,16,32,44,54,64}; r = {5, false,-1,  -1, Mt_lep_met_c_bins}; m["Mt_lep_met_c"]   = {Mt_lep_met,     r};
	static double bins_Mt_lep_met_c[] = {0,16,32,44,54,64,74,81,88,95,104,116,132,160,250}; r = {(sizeof(bins_Mt_lep_met_c) / sizeof(bins_Mt_lep_met_c[0]))-1, false,-1,  -1, bins_Mt_lep_met_c}; m["Mt_lep_met_c"]   = {NT_distr_Mt_lep_met,     r};
	// ok! this needs a wrapper-macro
	//cerr_expr(r.custom_bins[0]);
	//cerr_expr(r.custom_bins[1]);

	r = {20, true,  0, 250};   m["Mt_lep_met_f"]   = {NT_distr_Mt_lep_met,     r};
	r = {25, true,  0, 200};   m["met_f2"]         = {NT_distr_met, r};
	r = {30, true,  0, 300};   m["met_f"]          = {NT_distr_met, r};
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

TH1D_histo create_TH1D_histo(_TH1D_histo_def& def, TString name, TString main_name)
{
	TH1D* histo;
	if (def.range.linear)
		{
		//cout << "making linear bins histogram " << endl;
		histo = (TH1D*) new TH1D(name, name, def.range.nbins, def.range.linear_min, def.range.linear_max);
		}
	else
		{
		//cout << "making custom bins histogram " << def.range.nbins << " " << def.range.custom_bins << endl;
		histo = (TH1D*) new TH1D(name, name, def.range.nbins, def.range.custom_bins);
		//cout << "making custom bins histogram DONW" << endl;
		}
	//TH1D_histo a_distr = {ahist, def.func, 0.};
	//distrs.push_back(a_distr);
	return {main_name, histo, def.func, 0.};
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
	return relevant_selection_stage == 9 || relevant_selection_stage == 7;
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
	return relevant_selection_stage == 8 || relevant_selection_stage == 6;
	}

bool NT_channel_mu_sel_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_mu_sel(sys);
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}

bool NT_channel_mu_sel_ss_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_mu_sel_ss(sys);
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
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
	return relevant_selection_stage == 19 || relevant_selection_stage == 17;
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
	return relevant_selection_stage == 18 || relevant_selection_stage == 16;
	}

bool NT_channel_el_sel_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_el_sel(sys);
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}

bool NT_channel_el_sel_ss_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_el_sel_ss(sys);
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}

// union os mu_sel and el_sel
bool NT_channel_lep_sel(ObjSystematics sys)
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
	return relevant_selection_stage == 9 || relevant_selection_stage == 7 || relevant_selection_stage == 19 || relevant_selection_stage == 17;
	}

bool NT_channel_lep_sel_ss(ObjSystematics sys)
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
	return relevant_selection_stage == 8 || relevant_selection_stage == 6 || relevant_selection_stage == 18 || relevant_selection_stage == 16;
	}

bool NT_channel_lep_sel_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_lep_sel(sys);
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}

bool NT_channel_lep_sel_ss_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_lep_sel_ss(sys);
	// I use the if expression to be sure the vector NT_event_taus_sv_sign is not empty!
	//return sel && NT_event_taus_sv_sign[0] > 3.;
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}


bool NT_channel_tt_elmu(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_em;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_em_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_em_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_em_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_em_JESDown;
	else relevant_selection_stage = NT_selection_stage_em;
	return relevant_selection_stage > 210 && relevant_selection_stage < 220;
	}

bool NT_channel_tt_elmu_tight(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_em;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_em_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_em_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_em_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_em_JESDown;
	else relevant_selection_stage = NT_selection_stage_em;
	return relevant_selection_stage == 215;
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

	double mT = NT_distr_Mt_lep_met(sys);

	return mT < 40 && (relevant_selection_stage == 135 || relevant_selection_stage == 134 || relevant_selection_stage == 125 || relevant_selection_stage == 124);
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

	double mT = NT_distr_Mt_lep_met(sys);

	return mT < 40 && (relevant_selection_stage == 235 || relevant_selection_stage == 234 || relevant_selection_stage == 225 || relevant_selection_stage == 224);
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

	double mT = NT_distr_Mt_lep_met(sys);

	return mT < 40 && (relevant_selection_stage == 133 || relevant_selection_stage == 132 || relevant_selection_stage == 123 || relevant_selection_stage == 122);
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

	double mT = NT_distr_Mt_lep_met(sys);

	return mT < 40 && (relevant_selection_stage == 233 || relevant_selection_stage == 232 || relevant_selection_stage == 223 || relevant_selection_stage == 222);
	}

bool NT_channel_dy_mutau_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_dy_mutau(sys);
	// I use the if expression to be sure the vector NT_event_taus_sv_sign is not empty!
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}
bool NT_channel_dy_mutau_ss_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_dy_mutau_ss(sys);
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}

bool NT_channel_dy_eltau_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_dy_eltau(sys);
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}
bool NT_channel_dy_eltau_ss_tauSV3(ObjSystematics sys)
	{
	bool sel = NT_channel_dy_eltau_ss(sys);
	return sel ?  NT_event_taus_sv_sign[0] > 3. : false;
	}



bool NT_channel_dy_elmu(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy_elmu;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_elmu_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_elmu_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_elmu_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_elmu_JESDown;
	else relevant_selection_stage = NT_selection_stage_dy_elmu;
	return relevant_selection_stage == 105;
	}

bool NT_channel_dy_elmu_ss(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy_elmu;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_elmu_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_elmu_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_elmu_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_elmu_JESDown;
	else relevant_selection_stage = NT_selection_stage_dy_elmu;
	return relevant_selection_stage == 103;
	}


bool NT_channel_dy_mumu(ObjSystematics sys)
	{
	int relevant_selection_stage = 0;
	if      (sys == NOMINAL)   relevant_selection_stage = NT_selection_stage_dy_mumu;
	else if (sys == JERUp)     relevant_selection_stage = NT_selection_stage_dy_mumu_JERUp  ;
	else if (sys == JERDown)   relevant_selection_stage = NT_selection_stage_dy_mumu_JERDown;
	else if (sys == JESUp)     relevant_selection_stage = NT_selection_stage_dy_mumu_JESUp  ;
	else if (sys == JESDown)   relevant_selection_stage = NT_selection_stage_dy_mumu_JESDown;
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
	else relevant_selection_stage = NT_selection_stage_dy_mumu;
	return relevant_selection_stage == 112 || relevant_selection_stage == 113 || relevant_selection_stage == 115;
	}




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

#define _quick_set_chandef(m, chan_name, sel_weight_func) m[#chan_name] = {NT_channel_ ## chan_name, sel_weight_func}

/** \brief The initialization function for the `bool` functions of the known distributions.

\return map<const char*, _F_channel_sel>
 */

map<TString, _S_chan_def> create_known_channel_definitions()
{
	//map<TString, _F_channel_sel> m;
	map<TString, _S_chan_def> m;

	//m["el_sel"] = channel_el_sel;
	_quick_set_chandef(m, el_sel   , NT_sysweight_NOMINAL_HLT_EL_MedTau);
	_quick_set_chandef(m, el_sel_ss, NT_sysweight_NOMINAL_HLT_EL_MedTau);
	_quick_set_chandef(m, mu_sel   , NT_sysweight_NOMINAL_HLT_MU_MedTau);
	_quick_set_chandef(m, mu_sel_ss, NT_sysweight_NOMINAL_HLT_MU_MedTau);
	_quick_set_chandef(m, lep_sel      , NT_sysweight_NOMINAL_HLT_LEP_MedTau);
	_quick_set_chandef(m, lep_sel_ss   , NT_sysweight_NOMINAL_HLT_LEP_MedTau);

	_quick_set_chandef(m, el_sel_tauSV3   , NT_sysweight_NOMINAL_HLT_EL_MedTau);
	_quick_set_chandef(m, el_sel_ss_tauSV3, NT_sysweight_NOMINAL_HLT_EL_MedTau);
	_quick_set_chandef(m, mu_sel_tauSV3   , NT_sysweight_NOMINAL_HLT_MU_MedTau);
	_quick_set_chandef(m, mu_sel_ss_tauSV3, NT_sysweight_NOMINAL_HLT_MU_MedTau);
	_quick_set_chandef(m, lep_sel_tauSV3      , NT_sysweight_NOMINAL_HLT_LEP_MedTau);
	_quick_set_chandef(m, lep_sel_ss_tauSV3   , NT_sysweight_NOMINAL_HLT_LEP_MedTau);

	_quick_set_chandef(m, tt_elmu      , NT_sysweight_NOMINAL_HLT_EL);
	_quick_set_chandef(m, tt_elmu_tight, NT_sysweight_NOMINAL_HLT_EL);

	_quick_set_chandef(m, dy_elmu    , NT_sysweight_NOMINAL_HLT_MU);
	_quick_set_chandef(m, dy_elmu_ss , NT_sysweight_NOMINAL_HLT_MU);

	_quick_set_chandef(m, dy_mutau   , NT_sysweight_NOMINAL_HLT_MU_MedTau);
	_quick_set_chandef(m, dy_mutau_ss, NT_sysweight_NOMINAL_HLT_MU_MedTau);
	_quick_set_chandef(m, dy_eltau   , NT_sysweight_NOMINAL_HLT_EL_MedTau);
	_quick_set_chandef(m, dy_eltau_ss, NT_sysweight_NOMINAL_HLT_EL_MedTau);

	_quick_set_chandef(m, dy_mutau_tauSV3   , NT_sysweight_NOMINAL_HLT_MU_MedTau);
	_quick_set_chandef(m, dy_mutau_ss_tauSV3, NT_sysweight_NOMINAL_HLT_MU_MedTau);
	_quick_set_chandef(m, dy_eltau_tauSV3   , NT_sysweight_NOMINAL_HLT_EL_MedTau);
	_quick_set_chandef(m, dy_eltau_ss_tauSV3, NT_sysweight_NOMINAL_HLT_EL_MedTau);

	_quick_set_chandef(m, dy_mumu    , NT_sysweight_NOMINAL_HLT_MU);
	_quick_set_chandef(m, dy_elel    , NT_sysweight_NOMINAL_HLT_EL);

	return m;
}


/** \brief The known channels.
 */
map<TString, _S_chan_def> known_defs_channels = create_known_channel_definitions();


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

NT_genproc(dy_tautau , 1)

NT_genproc(wjets_tauh , 2)
NT_genproc(wjets_taul , 1)

NT_genproc(stop_eltau    , 20)
NT_genproc(stop_mutau    , 10)
NT_genproc(stop_lj    ,  2)
NT_genproc(stop_elmu  ,  1)

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
NT_genproc_range(tt_leptau, 30, 43)

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
vector<TString> _eltau_tt_procs  = {"tt_eltau", "tt_taulj", "tt_lj"};
vector<TString> _mutau_tt_procs  = {"tt_mutau", "tt_taulj", "tt_lj"};
vector<TString> _leptau_tt_procs = {"tt_leptau", "tt_taulj", "tt_lj"};
vector<TString>  _elmu_tt_procs = {"tt_elmu",  "tt_ltaul"};
//vector<TString> _mumu_tt_procs  = {"tt_inclusive"};
//vector<TString> _elel_tt_procs  = {"tt_inclusive"};
// actually empty vectors must work, the catchall process will handle this
vector<TString> _mumu_tt_procs  = {};
vector<TString> _elel_tt_procs  = {};

vector<TString> _leptau_dy_procs = {"dy_tautau"};
vector<TString>  _incl_dy_procs = {};

vector<TString> _eltau_stop_procs = {"stop_eltau", "stop_lj"};
vector<TString> _mutau_stop_procs = {"stop_mutau", "stop_lj"};
vector<TString>  _elmu_stop_procs = {"stop_elmu"};
vector<TString>  _mumu_stop_procs = {};
vector<TString>  _elel_stop_procs = {};

vector<TString> _eltau_wjets_procs = {"wjets_tauh", "wjets_taul"};
vector<TString> _mutau_wjets_procs = {"wjets_tauh", "wjets_taul"};
vector<TString>  _elmu_wjets_procs = {};
vector<TString>  _mumu_wjets_procs = {};
vector<TString>  _elel_wjets_procs = {};

// ^--- I am not sure this is the best approach to define these standard processes
//      initially I wanted to have a bunch of general sets, not the concrete per-channel ones


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
			{"tt_leptau" , NT_genproc_tt_leptau},
			{"tt_lj"    , NT_genproc_tt_lj},
			{"tt_inclusive" , NT_genproc_inclusive},
			},

		.channel_standard = {
			{"el_sel",    _eltau_tt_procs},
			{"el_sel_ss", _eltau_tt_procs},
			{"mu_sel",    _mutau_tt_procs},
			{"mu_sel_ss", _mutau_tt_procs},

			{"el_sel_tauSV3",    _eltau_tt_procs},
			{"el_sel_ss_tauSV3", _eltau_tt_procs},
			{"mu_sel_tauSV3",    _mutau_tt_procs},
			{"mu_sel_ss_tauSV3", _mutau_tt_procs},

			{"lep_sel",    _leptau_tt_procs},
			{"lep_sel_ss", _leptau_tt_procs},

			{"tt_elmu",       _elmu_tt_procs},
			{"tt_elmu_tight", _elmu_tt_procs},
			{"dy_elmu",       _elmu_tt_procs},
			{"dy_elmu_ss",    _elmu_tt_procs},

			{"dy_mutau",      _mutau_tt_procs},
			{"dy_mutau_ss",   _mutau_tt_procs},
			{"dy_eltau",      _eltau_tt_procs},
			{"dy_eltau_ss",   _eltau_tt_procs},

			{"dy_mutau_tauSV3",      _mutau_tt_procs},
			{"dy_mutau_ss_tauSV3",   _mutau_tt_procs},
			{"dy_eltau_tauSV3",      _eltau_tt_procs},
			{"dy_eltau_ss_tauSV3",   _eltau_tt_procs},

			{"dy_mumu",       _mumu_tt_procs},
			{"dy_elel",       _mumu_tt_procs},

			}
		};

	m["dy"] = {
		.catchall_name = "dy_other", // usually we run only on DYTo2L, so this is the elel and mumu
		.all={
			{"dy_tautau"   , NT_genproc_dy_tautau},
			},

		.groups={},

		.channel_standard = {
			{"el_sel",    _leptau_dy_procs},
			{"el_sel_ss", _leptau_dy_procs},
			{"mu_sel",    _leptau_dy_procs},
			{"mu_sel_ss", _leptau_dy_procs},

			{"el_sel_tauSV3",    _leptau_dy_procs},
			{"el_sel_ss_tauSV3", _leptau_dy_procs},
			{"mu_sel_tauSV3",    _leptau_dy_procs},
			{"mu_sel_ss_tauSV3", _leptau_dy_procs},

			{"tt_elmu",       _incl_dy_procs},
			{"tt_elmu_tight", _incl_dy_procs},

			{"dy_mutau",      _leptau_dy_procs},
			{"dy_mutau_ss",   _leptau_dy_procs},
			{"dy_eltau",      _leptau_dy_procs},
			{"dy_eltau_ss",   _leptau_dy_procs},

			{"dy_mutau_tauSV3",      _leptau_dy_procs},
			{"dy_mutau_ss_tauSV3",   _leptau_dy_procs},
			{"dy_eltau_tauSV3",      _leptau_dy_procs},
			{"dy_eltau_ss_tauSV3",   _leptau_dy_procs},

			{"dy_mumu",       _incl_dy_procs},
			{"dy_elel",       _incl_dy_procs},
			{"dy_elmu",       _leptau_dy_procs},
			{"dy_elmu_ss",    _leptau_dy_procs},
			}
		};

	m["stop"] = {
		.catchall_name = "stop_other",
		.all={
			{"stop_eltau"   , NT_genproc_stop_eltau},
			{"stop_mutau"   , NT_genproc_stop_mutau},
			{"stop_lj"   , NT_genproc_stop_lj},
			{"stop_elmu" , NT_genproc_stop_elmu},
			},

		.groups={},

		.channel_standard = {
			{"el_sel",    _eltau_stop_procs},
			{"el_sel_ss", _eltau_stop_procs},
			{"mu_sel",    _mutau_stop_procs},
			{"mu_sel_ss", _mutau_stop_procs},

			{"el_sel_tauSV3",    _eltau_stop_procs},
			{"el_sel_ss_tauSV3", _eltau_stop_procs},
			{"mu_sel_tauSV3",    _mutau_stop_procs},
			{"mu_sel_ss_tauSV3", _mutau_stop_procs},

			{"tt_elmu",        _elmu_stop_procs},
			{"tt_elmu_tight",  _elmu_stop_procs},

			{"dy_mutau",      _mutau_stop_procs},
			{"dy_mutau_ss",   _mutau_stop_procs},
			{"dy_eltau",      _eltau_stop_procs},
			{"dy_eltau_ss",   _eltau_stop_procs},

			{"dy_mutau_tauSV3",      _mutau_stop_procs},
			{"dy_mutau_ss_tauSV3",   _mutau_stop_procs},
			{"dy_eltau_tauSV3",      _eltau_stop_procs},
			{"dy_eltau_ss_tauSV3",   _eltau_stop_procs},

			{"dy_mumu",        _mumu_stop_procs},
			{"dy_elel",        _mumu_stop_procs},
			{"dy_elmu",       _elmu_stop_procs},
			{"dy_elmu_ss",    _elmu_stop_procs},
			}
		};

	m["wjets"] = {
		.catchall_name = "wjets_other",
		.all={
			{"wjets_tauh"   , NT_genproc_wjets_tauh},
			{"wjets_taul"   , NT_genproc_wjets_taul},
			},

		.groups={},

		.channel_standard = {
			{"el_sel",    _eltau_wjets_procs},
			{"el_sel_ss", _eltau_wjets_procs},
			{"mu_sel",    _mutau_wjets_procs},
			{"mu_sel_ss", _mutau_wjets_procs},

			{"el_sel_tauSV3",    _eltau_wjets_procs},
			{"el_sel_ss_tauSV3", _eltau_wjets_procs},
			{"mu_sel_tauSV3",    _mutau_wjets_procs},
			{"mu_sel_ss_tauSV3", _mutau_wjets_procs},

			{"tt_elmu",        _elmu_wjets_procs},
			{"tt_elmu_tight",  _elmu_wjets_procs},

			{"dy_mutau",      _mutau_wjets_procs},
			{"dy_mutau_ss",   _mutau_wjets_procs},
			{"dy_eltau",      _eltau_wjets_procs},
			{"dy_eltau_ss",   _eltau_wjets_procs},

			{"dy_mutau_tauSV3",      _mutau_wjets_procs},
			{"dy_mutau_ss_tauSV3",   _mutau_wjets_procs},
			{"dy_eltau_tauSV3",      _eltau_wjets_procs},
			{"dy_eltau_ss_tauSV3",   _eltau_wjets_procs},

			{"dy_mumu",        _mumu_wjets_procs},
			{"dy_elel",        _mumu_wjets_procs},
			{"dy_elmu",       _elmu_wjets_procs},
			{"dy_elmu_ss",    _elmu_wjets_procs},
			}
		};

	m["data"] = {
		.catchall_name = "data", // usually we run only on DYTo2L, so this is the elel and mumu
		.all={},
		.groups={},
		.channel_standard = {
			{"el_sel",    {}},
			{"el_sel_ss", {}},
			{"mu_sel",    {}},
			{"mu_sel_ss", {}},

			{"el_sel_tauSV3",    {}},
			{"el_sel_ss_tauSV3", {}},
			{"mu_sel_tauSV3",    {}},
			{"mu_sel_ss_tauSV3", {}},

			{"tt_elmu",       {}},
			{"tt_elmu_tight", {}},

			{"dy_mutau",      {}},
			{"dy_mutau_ss",   {}},
			{"dy_eltau",      {}},
			{"dy_eltau_ss",   {}},

			{"dy_mutau_tauSV3",      {}},
			{"dy_mutau_ss_tauSV3",   {}},
			{"dy_eltau_tauSV3",      {}},
			{"dy_eltau_ss_tauSV3",   {}},

			{"dy_mumu",       {}},
			{"dy_elel",       {}},
			{"dy_elmu",       {}},
			{"dy_elmu_ss",    {}},
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
	_S_proc_ID_defs std_procs;
	vector<TString> std_systs;
	// all available systematics
} S_dtag_info;

map<TString, S_dtag_info> create_known_dtags_info()
	{
	//map<const char*, S_dtag_info> m;
	map<TString, S_dtag_info> m;

	m["MC2017legacy_Fall17_WJets_madgraph"              ] = {.cross_section= 52940.                            ,
		.usual_gen_lumi= 23102470.188817,
		.std_procs = known_procs_info["wjets"],
		.std_systs = {SYSTS_OTHER_MC}};
	m["MC2017legacy_Fall17_DYJetsToLL_50toInf_madgraph" ] = {.cross_section=  6225.42                          ,
		.usual_gen_lumi= 18928303.971956,
		.std_procs = known_procs_info["dy"],
		.std_systs = {SYSTS_OTHER_MC}};
	m["MC2017legacy_Fall17_SingleT_tW_5FS_powheg"       ] = {.cross_section=    35.6                           ,
		.usual_gen_lumi=  5099879.048270,
		.std_procs=known_procs_info["stop"],
		.std_systs = {SYSTS_OTHER_MC}};
	m["MC2017legacy_Fall17_SingleTbar_tW_5FS_powheg"    ] = {.cross_section=    35.6                           ,
		.usual_gen_lumi=  2349775.859249,
		.std_procs=known_procs_info["stop"],
		.std_systs = {SYSTS_OTHER_MC}};
	m["MC2017legacy_Fall17_TTToHadronic_13TeV"             ] = {.cross_section=   831.76 * W_qar_br2              ,
		.usual_gen_lumi= 29213134.729453,
		.std_procs=known_procs_info["tt"],
		.std_systs = {SYSTS_TT}};
	m["MC2017legacy_Fall17_TTToSemiLeptonic"            ] = {.cross_section=   831.76 * 2*W_alllep_br*W_qar_br ,
		.usual_gen_lumi= 21966343.919990,
		.std_procs=known_procs_info["tt"],
		.std_systs = {SYSTS_TT}};
	m["MC2017legacy_Fall17_TTTo2L2Nu"                      ] = {.cross_section=   831.76 * W_alllep_br2           ,
		.usual_gen_lumi=  2923730.883332,
		.std_procs=known_procs_info["tt"],
		.std_systs = {SYSTS_TT}};

	m["MC2016_Summer16_TTJets_powheg"                      ] = {.cross_section=   831.76           ,
		.usual_gen_lumi=  1.,
		.std_procs = known_procs_info["tt"],
		.std_systs = {SYSTS_TT}};

	// I probably need some defaults for not found dtags

	// TODO: for now enter Data as any MC
	m["Data2017legacy"                      ] = {.cross_section=   1.           ,
		.usual_gen_lumi=  1.,
		.std_procs = known_procs_info["data"],
		.std_systs = {"NOMINAL"}};

	return m;
	}

map<TString, S_dtag_info> known_dtags_info = create_known_dtags_info();


/* --------------------------------------------------------------- */
// helper functions

const char* TString_Data (TString& str) {return str.Data();}
const char* TH1D_Name    (TH1D& hist) {return hist.GetName();}

/* --------------------------------------------------------------- */




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
	TString name;               /**< \brief keep the name of this process to assign final per-proc normalization */
	_F_genproc_def proc_def;    /**< \brief `bool` function defining whether an event passes this gen proc definition */
	vector<TH1D_histo> histos;  /**< \brief the distributions to record */
} T_proc_histos;

typedef struct{
	TString name;                 /**< \brief keep the name of this process to assign final per-channel normalization */
	_S_chan_def chan_def;         /**< \brief channel definition: `bool` function selecting events, and the nominal event weight funciton */
	vector<T_proc_histos> procs;  /**< \brief the channels with distributions to record */
	TString name_catchall_proc;   /**< \brief the name of the catchall processes */
	vector<TH1D_histo> catchall_proc_histos;  /**< \brief the channels with distributions to record in the catchall process */
} T_chan_proc_histos;

typedef struct{
	TString name;                      /**< \brief keep the name of this process to assign final per-systematic normalization */
	_S_systematic_definition syst_def; /**< \brief the definition of a given systematic */
	vector<T_chan_proc_histos> chans;  /**< \brief the per-proc channels with distributions to record */
} T_syst_chan_proc_histos;

/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
// final normalizations

// normalization per gen lumi event weight
TH1D* weight_counter = NULL;

// per dtag cross section
bool normalise_per_cross_section = true;

//
map<TString, double> create_known_normalization_per_syst()
	{
	map<TString, double> m;

	// TODO: review these, must the NOMINAL normalization be applied to all the others?

	// 2016 corrections
	m["NOMINAL" ] = 1. / 1.01388; // NOMINAL PU factor
	m["PUUp"    ] = 1. / 0.994846;
	m["PUDown"  ] = 1. / 1.03657;

	// tt only!
	m["FragUp"	] = 0.958852; //1.001123,
	m["FragDown"	] = 1.02936;  //0.999304,
	m["SemilepBRUp"	] = 1.002650;
	m["SemilepBRDown"	] = 1.035812;
	m["PetersonUp"	] = 0.990578;

	m["MrUp"	] = 0.896601;
	m["MrDown"	] = 1.114154;
	m["MfUp"	] = 0.980610;
	m["MfDown"	] = 1.025511;
	m["MfrUp"	] = 0.874602;
	m["MfrDown"	] = 1.135832;
	m["AlphaSDown"	] = 1.015573;
	m["AlphaSUp"	] = 0.984924;
	m["PDFCT14n1Up"	] = 0.999214;
	m["PDFCT14n10Up"	] = 0.992985;
	m["PDFCT14n11Up"	] = 1.011667;
	m["PDFCT14n12Up"	] = 0.991012;
	m["PDFCT14n13Up"	] = 1.011832;
	m["PDFCT14n14Up"	] = 0.994285;
	m["PDFCT14n15Up"	] = 1.020625;
	m["PDFCT14n16Up"	] = 0.985324;
	m["PDFCT14n17Up"	] = 0.986681;
	m["PDFCT14n18Up"	] = 1.015958;
	m["PDFCT14n19Up"	] = 0.999024;
	m["PDFCT14n2Up"	] = 0.999610;
	m["PDFCT14n20Up"	] = 1.001383;
	m["PDFCT14n21Up"	] = 1.000811;
	m["PDFCT14n22Up"	] = 0.998574;
	m["PDFCT14n23Up"	] = 1.004126;
	m["PDFCT14n24Up"	] = 0.996106;
	m["PDFCT14n25Up"	] = 1.012630;
	m["PDFCT14n26Up"	] = 0.987756;
	m["PDFCT14n27Up"	] = 1.003978;
	m["PDFCT14n28Up"	] = 0.994568;
	m["PDFCT14n29Up"	] = 1.002681;
	m["PDFCT14n3Up"	] = 0.991550;
	m["PDFCT14n30Up"	] = 0.999111;
	m["PDFCT14n31Up"	] = 1.001791;
	m["PDFCT14n32Up"	] = 0.997694;
	m["PDFCT14n33Up"	] = 0.996328;
	m["PDFCT14n34Up"	] = 1.008113;
	m["PDFCT14n35Up"	] = 0.985943;
	m["PDFCT14n36Up"	] = 1.013638;
	m["PDFCT14n37Up"	] = 0.993136;
	m["PDFCT14n38Up"	] = 1.010604;
	m["PDFCT14n39Up"	] = 0.995664;
	m["PDFCT14n4Up"	] = 0.995349;
	m["PDFCT14n40Up"	] = 1.004863;
	m["PDFCT14n41Up"	] = 0.999538;
	m["PDFCT14n42Up"	] = 1.000247;
	m["PDFCT14n43Up"	] = 0.997833;
	m["PDFCT14n44Up"	] = 1.005385;
	m["PDFCT14n45Up"	] = 1.004722;
	m["PDFCT14n46Up"	] = 1.001524;
	m["PDFCT14n47Up"	] = 0.999380;
	m["PDFCT14n48Up"	] = 0.993950;
	m["PDFCT14n49Up"	] = 0.993164;
	m["PDFCT14n5Up"	] = 0.994886;
	m["PDFCT14n50Up"	] = 1.003200;
	m["PDFCT14n51Up"	] = 1.001966;
	m["PDFCT14n52Up"	] = 1.000614;
	m["PDFCT14n53Up"	] = 0.968178;
	m["PDFCT14n54Up"	] = 1.010604;
	m["PDFCT14n55Up"	] = 0.992717;
	m["PDFCT14n56Up"	] = 1.012645;
	m["PDFCT14n6Up"	] = 1.004233;
	m["PDFCT14n7Up"	] = 1.001432;
	m["PDFCT14n8Up"	] = 0.992782;
	m["PDFCT14n9Up"	] = 1.008439;

	// 2017 from TTTo2L
	m["MU_PU"]     =         0.692955;
	m["MU_PUUp"]   =         0.757326;
	m["MU_PUDown"] =         0.637007;
	m["El_PU"]     =         0.579763;
	m["El_PUUp"]   =         0.627530;
	m["El_PUDown"] =         0.542434;
	// TODO: the same issue with PU, I need to normalize to nominal PU everywhere?
	// for now let's just normalize systematics
	// I use only muon channels now, so:
	m["NOMINAL" ] = 1./0.692955; // NOMINAL PU factor
	m["PUUp"]     = 1./0.757326;
	m["PUDown"]   = 1./0.637007;

	m["TOPPT"] = 1. /             1.000352;
	m["FragUp"] = 1. /             0.912333;
	m["FragDown"] = 1. /             1.067350;
	m["SemilepBRUp"] = 1. /             1.014130;
	m["SemilepBRDown"] = 1. /             1.047939;
	m["PetersonUp"] = 1. /             0.963072;

	m["MrUp"] = 1. /             0.897033;
	m["MrDown"] = 1. /             1.116338;
	m["MfUp"] = 1. /             0.979744;
	m["MfDown"] = 1. /             1.026878;
	m["MfrUp"] = 1. /             0.874539;
	m["MfrDown"] = 1. /             1.140431;

	m["AlphaSDown"] = 1. /            73.388846;
	m["AlphaSUp"] = 1. /            73.434533;
	m["PDFCT14n1"] = 1. /             0.842441;
	m["PDFCT14n10"] = 1. /             1.019382;
	m["PDFCT14n11"] = 1. /             1.017693;
	m["PDFCT14n12"] = 1. /             1.018400;
	m["PDFCT14n13"] = 1. /             1.021125;
	m["PDFCT14n14"] = 1. /             1.019251;
	m["PDFCT14n15"] = 1. /             1.017953;
	m["PDFCT14n16"] = 1. /             1.019955;
	m["PDFCT14n17"] = 1. /             1.017888;
	m["PDFCT14n18"] = 1. /             1.020231;
	m["PDFCT14n19"] = 1. /             1.016634;
	m["PDFCT14n2"] = 1. /             0.867848;
	m["PDFCT14n20"] = 1. /             1.017093;
	m["PDFCT14n21"] = 1. /             1.019368;
	m["PDFCT14n22"] = 1. /             1.019468;
	m["PDFCT14n23"] = 1. /             1.019067;
	m["PDFCT14n24"] = 1. /             1.016827;
	m["PDFCT14n25"] = 1. /             1.019057;
	m["PDFCT14n26"] = 1. /             1.018442;
	m["PDFCT14n27"] = 1. /             1.019834;
	m["PDFCT14n28"] = 1. /             1.018420;
	m["PDFCT14n29"] = 1. /             1.018189;
	m["PDFCT14n3"] = 1. /             0.894395;
	m["PDFCT14n30"] = 1. /             1.018549;
	m["PDFCT14n31"] = 1. /             1.018317;
	m["PDFCT14n32"] = 1. /             1.017693;
	m["PDFCT14n33"] = 1. /             1.017466;
	m["PDFCT14n34"] = 1. /             1.018964;
	m["PDFCT14n35"] = 1. /             1.018410;
	m["PDFCT14n36"] = 1. /             1.019069;
	m["PDFCT14n37"] = 1. /             1.016709;
	m["PDFCT14n38"] = 1. /             1.016373;
	m["PDFCT14n39"] = 1. /             1.022541;
	m["PDFCT14n4"] = 1. /             0.918370;
	m["PDFCT14n40"] = 1. /             1.020639;
	m["PDFCT14n41"] = 1. /             1.018076;
	m["PDFCT14n42"] = 1. /             1.019376;
	m["PDFCT14n43"] = 1. /             1.018341;
	m["PDFCT14n44"] = 1. /             1.019453;
	m["PDFCT14n45"] = 1. /             1.018983;
	m["PDFCT14n46"] = 1. /             1.019039;
	m["PDFCT14n47"] = 1. /             1.018464;
	m["PDFCT14n48"] = 1. /             1.018923;
	m["PDFCT14n49"] = 1. /             1.018704;
	m["PDFCT14n5"] = 1. /             0.958628;
	m["PDFCT14n50"] = 1. /             1.018930;
	m["PDFCT14n51"] = 1. /             1.017098;
	m["PDFCT14n52"] = 1. /             1.017097;
	m["PDFCT14n53"] = 1. /             1.018855;
	m["PDFCT14n54"] = 1. /             1.019012;
	m["PDFCT14n55"] = 1. /             1.019831;
	m["PDFCT14n56"] = 1. /             1.018409;
	m["PDFCT14n6"] = 1. /             0.986781;
	m["PDFCT14n7"] = 1. /             1.024974;
	m["PDFCT14n8"] = 1. /             1.053645;
	m["PDFCT14n9"] = 1. /             1.018451;

	return m;
	}

map<TString, double> known_normalization_per_syst = create_known_normalization_per_syst();

map<TString, double> create_known_normalization_per_proc()
	{
	map<TString, double> m;

	/* NO! tau ID is only applied in channels where a tau was actually required
	double tau_ID_correction = 0.95;
	m["tt_eltau"]    = tau_ID_correction;
	m["tt_mutau"]    = tau_ID_correction;
	m["tt_eltau3ch"] = tau_ID_correction;
	m["tt_mutau3ch"] = tau_ID_correction;
	m["tt_eltau1ch"] = tau_ID_correction;
	m["tt_mutau1ch"] = tau_ID_correction;

	m["tt_taultauh"] = tau_ID_correction;

	m["dy_tautau"] = tau_ID_correction;
	m["s_top_eltau"] = tau_ID_correction;
	m["s_top_mutau"] = tau_ID_correction;
	*/

	return m;
	}

map<TString, double> known_normalization_per_proc = create_known_normalization_per_proc();


map<TString, double> create_known_normalization_per_chan()
	{
	map<TString, double> m;

	return m;
	}

map<TString, double> known_normalization_per_chan = create_known_normalization_per_chan();


/* it was used for tau ID uncertainty, which is implemented per-channel now
TODO: remove if this system is absolutely not needed
double rogue_mixed_corrections(const TString& name_syst, const TString& name_chan, const TString& name_proc)
	{
	double correction = 1.;

	// tau ID correction
	// apply it in channels with a selected tau
	// to processes with genuine taus
	// TODO: it probably must be done via event weight in stage2
	double tau_ID_correction = 0.95;
	set<TString> tau_channels  = {"mu_sel", "el_sel", "mu_sel_ss", "el_sel_ss", "mu_selSV", "el_selSV", "mu_selSV_ss", "el_selSV_ss", "dy_mutau", "ctr_old_mu_sel", "ctr_old_mu_sel", "ctr_old_el_sel", "optel_tight_2L1M", "optmu_tight_2L1M"};
	set<TString> tau_processes = {"tt_mutau3ch", "tt_eltau3ch", "tt_mutau", "tt_eltau", "tt_taultauh", "dy_tautau", "s_top_eltau", "s_top_mutau"};
	if ((tau_channels.find(name_chan) != tau_channels.end()) && (tau_processes.find(name_proc) != tau_processes.end()))
		correction *= tau_ID_correction;

	return correction;
	}
*/


void normalise_final(TH1D* histo, double cross_section, double scale, const TString& name_syst, const TString& name_chan, const TString& name_proc)
	{
	if (normalise_per_weight)
		histo->Scale(1./weight_counter->GetBinContent(2));

	if (normalise_per_cross_section)
		histo->Scale(cross_section);

	/* per-syst/proc/chan systematic corrections
	*/

	double per_syst_factor = (name_syst == "NOMINAL" || known_normalization_per_syst.find(name_syst) == known_normalization_per_syst.end()) ?  known_normalization_per_syst["NOMINAL"] :
		(name_syst == "PUUp" || name_syst == "PUDown" ? known_normalization_per_syst[name_syst] : known_normalization_per_syst["NOMINAL"] * known_normalization_per_syst[name_syst]);

	double per_proc_factor = known_normalization_per_proc.find(name_proc) == known_normalization_per_proc.end() ?
		1. :
		known_normalization_per_proc[name_proc];

	double per_chan_factor = known_normalization_per_chan.find(name_chan) == known_normalization_per_chan.end() ?
		1. :
		known_normalization_per_chan[name_chan];

	//double rogue_mixed_correction = rogue_mixed_corrections(name_syst, name_chan, name_proc);

	histo->Scale(scale * per_syst_factor * per_proc_factor * per_chan_factor /* * rogue_mixed_correction*/);
	}
/* --------------------------------------------------------------- */

/** \brief parse char* coma-separated string into vector<TString>
 */

vector<TString> parse_coma_list(char* coma_list)
	{
	vector<TString> list;

	// the input is so simple, we can just substitute , for \0 and set up char* array
	// ok, lets parse with strtok and push_back into a vector
	char delimeter[] = ",";
	char* txt;
	char* scratch;

	txt = strtok_r(coma_list, delimeter, &scratch);
	list.push_back(TString(txt));
	while ((txt = strtok_r(NULL, delimeter, &scratch)))
		list.push_back(TString(txt));

	return list;
	}


/** \brief generate a tree with the record histograms from requested systematics, channels, processes, and histograms

\return vector<T_syst_chan_proc_histos>
 */

vector<T_syst_chan_proc_histos> setup_record_histos(
	S_dtag_info&     main_dtag_info,
	vector<TString>& requested_systematics,
	vector<TString>& requested_channels   ,
	vector<TString>& requested_procs      ,
	vector<TString>& requested_distrs     )
{
// the output tree
vector<T_syst_chan_proc_histos> distrs_to_record;

// some info for profiling
int n_systs_made = 0, n_chans_made = 0, n_procs_made = 0, n_distrs_made = 0;

// final state channels
vector<TString> requested_channels_all = {
	"el_sel",
	"mu_sel",
	"el_sel_ss",
	"mu_sel_ss",

	"el_sel_tauSV3",
	"mu_sel_tauSV3",
	"el_sel_ss_tauSV3",
	"mu_sel_ss_tauSV3",

	"tt_elmu",
	"tt_elmu_tight",

	"dy_mutau",
	"dy_mutau_ss",
	"dy_eltau",
	"dy_eltau_ss",

	"dy_mutau_tauSV3",
	"dy_mutau_ss_tauSV3",
	"dy_eltau_tauSV3",
	"dy_eltau_ss_tauSV3",

	"dy_elmu",
	"dy_elmu_ss",
	"dy_mumu",
	"dy_elel"};

if (requested_channels[0] == "all")
	requested_channels = requested_channels_all;

//const char* requested_channel_names[] = {"tt_elmu", NULL};
//vector<TString> requested_procs   = {"all"};
//vector<TString> requested_procs   = {"std"};
vector<TString> requested_systematics_test = {"NOMINAL",
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
	};

if (requested_systematics[0] == "test")
	requested_systematics = requested_systematics_test;
else if (requested_systematics[0] == "std")
	requested_systematics = main_dtag_info.std_systs;

//requested_systematics[0] = "NOMINAL"; requested_systematics[1] = NULL;

//const char* requested_distrs[]        = {"Mt_lep_met_c", "leading_lep_pt", NULL};

// set the known processes according to the request: whetehr groups of processes are requested or not
// check if all known processes are requested
map<TString, _F_genproc_def> known_procs;
if (requested_procs[0] == "all")
	{
	known_procs = main_dtag_info.std_procs.all;
	// and reset the requested processes to all known processes for this dtag
	requested_procs.clear();
	// loop over all known processes
	for (const auto& proc: main_dtag_info.std_procs.all)
		{
		//cout_expr(proc.first);
		requested_procs.push_back(proc.first);
		}
	}

else
	{
	known_procs = main_dtag_info.std_procs.groups;
	//known_procs = main_dtag_info.std_procs.all;
	// populate definitions of groups with the fine-grain definitions of all processes			
	known_procs.insert(main_dtag_info.std_procs.all.begin(), main_dtag_info.std_procs.all.end());
	}


// name of the catchall process for the defined dtag
TString procname_catchall = main_dtag_info.std_procs.catchall_name;
map<TString, vector<TString>>& known_std_procs_per_channel = main_dtag_info.std_procs.channel_standard;

// --------------------------------- SETUP RECORD HISTOS for output
for (const auto& systname: requested_systematics)
	{
	// find the definition of this channel
	Stopif(known_systematics.find(systname) == known_systematics.end(), continue, "Do not know a systematic %s", systname.Data());

	T_syst_chan_proc_histos systematic = {.name=systname, .syst_def = known_systematics[systname]};

	// define channels
	for (const auto& channame: requested_channels)
		{
		// find the definition of this channel
		Stopif(known_defs_channels.find(channame) == known_defs_channels.end(), continue, "Do not know the channel %s", channame.Data());

		T_chan_proc_histos channel          = {
			.name = channame,
			.chan_def = known_defs_channels[channame],
			.procs = {},
			.name_catchall_proc = procname_catchall};

		// check if standard processes per channels are requested
		vector<TString>* process_definitions_to_use = &requested_procs;
		if ((*process_definitions_to_use)[0] == "std")
			{
			// TODO in case of an unknown process set an inclusive definition
			Stopif(known_std_procs_per_channel.find(channame) == known_std_procs_per_channel.end(), continue, "Do not know standard processes for the channel %s", channame.Data())
			// set the standard processes
			process_definitions_to_use = &known_std_procs_per_channel[channame];
			}

		// loop over requested processes and find their definitions for recording in this channel
		//for (const char** requested_proc = requested_procs; *requested_proc != NULL; requested_proc++)
		for (const auto& procname: (*process_definitions_to_use))
			{
			// find the definition of this channel
			//TString procname(*requested_proc);

			Stopif(known_procs.find(procname) == known_procs.end(), continue, "Do not know the process %s", procname.Data());

			T_proc_histos process = {.name=procname, .proc_def=known_procs[procname]};

			// define distributions
			// create the histograms for all of these definitions
			for (const auto& distrname: requested_distrs)
				{
				Stopif(known_defs_distrs.find(distrname) == known_defs_distrs.end(), continue, "Do not know a distribution %s", distrname.Data());

				TH1D_histo a_distr = create_TH1D_histo(known_defs_distrs[distrname], channame + "_" + procname + "_" + systname + "_" + distrname, distrname);
				process.histos.push_back(a_distr);

				n_distrs_made +=1;
				}

			channel.procs.push_back(process);
			n_procs_made +=1;
			}

		// setup catchall process

		// define distributions
		// create the histograms for all of these definitions
		for (const auto& distrname: requested_distrs)
			{
			Stopif(known_defs_distrs.find(distrname) == known_defs_distrs.end(), continue, "Do not know a distribution %s", distrname.Data());

			TH1D_histo a_distr = create_TH1D_histo(known_defs_distrs[distrname], channame + "_" + procname_catchall + "_" + systname + "_" + distrname, distrname);
			channel.catchall_proc_histos.push_back(a_distr);
			n_distrs_made +=1;
			n_procs_made +=1;
			}

		systematic.chans.push_back(channel);
		n_chans_made +=1;
		}

	distrs_to_record.push_back(systematic);
	n_systs_made +=1;
	}

cerr_expr(n_systs_made << " " << n_chans_made << " " << n_procs_made << " " << n_distrs_made);
return distrs_to_record;
}

void event_loop(TTree* NT_output_ttree, vector<T_syst_chan_proc_histos>& distrs_to_record,
	bool skip_nup5_events, bool isMC)
{
// open the interface to stage2 TTree-s
//#define NTUPLE_INTERFACE_OPEN
#define NTUPLE_INTERFACE_CONNECT
#include "stage2_interface.h"

unsigned int n_entries = NT_output_ttree->GetEntries();
//cerr_expr(n_entries);

for (unsigned int ievt = 0; ievt < n_entries; ievt++)
	{
	NT_output_ttree->GetEntry(ievt);

	if (skip_nup5_events && NT_nup > 5) continue;

	//// tests
	////cerr_expr(NT_runNumber);
	//cerr_expr(NT_event_leptons[0].pt());
	//cerr_expr(NT_event_leptons_genmatch[0]);

	//Stopif(ievt > 10, break, "reached 10 events, exiting");

	// loop over systematics
	for (int si=0; si<distrs_to_record.size(); si++)
		{
		ObjSystematics obj_systematic = distrs_to_record[si].syst_def.obj_sys_id;

		// the factor to the NOMINAL_base weight
		double event_weight_factor    = isMC ? distrs_to_record[si].syst_def.weight_func() : 1.;

		// record distributions in all final states where the event passes
		vector<T_chan_proc_histos>& channels = distrs_to_record[si].chans;
		for (int ci=0; ci<channels.size(); ci++)
			{
			T_chan_proc_histos& chan = channels[ci];

			// check if event passes the channel selection
			if (!chan.chan_def.chan_sel(obj_systematic)) continue;

			// calculate the NOMINAL_base event weight for the channel
			double event_weight = isMC ? chan.chan_def.chan_sel_weight() : 1.;
			// and multiply by the systematic factor
			event_weight *= event_weight_factor;

			// assign the gen process
			// loop over procs check if this event passes
			// if not get the catchall proc

			// set catchall
			vector<TH1D_histo>* histos = &(chan.catchall_proc_histos);

			// check if any specific channel passes
			for (int pi=0; pi<chan.procs.size(); pi++)
				{
				if (chan.procs[pi].proc_def())
					{
					histos = &chan.procs[pi].histos;
					break;
					}
				}

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

void write_output(const char* output_filename, vector<T_syst_chan_proc_histos>& distrs_to_record,
	S_dtag_info& main_dtag_info,
	Float_t lumi,
	bool isMC, bool save_in_old_order, bool simulate_data)
{
TFile* output_file  = (TFile*) new TFile(output_filename, "RECREATE");
output_file->Write();

if (save_in_old_order)
  {
  for (int si=0; si<distrs_to_record.size(); si++)
	{
	// merge defined processes and the catchall
	TString& syst_name = distrs_to_record[si].name;
	vector<T_chan_proc_histos>& all_chans = distrs_to_record[si].chans;

	output_file->cd();
	//TDirectory* dir_syst = (TDirectory*) output_file->mkdir(syst_name);
	//dir_syst->SetDirectory(output_file);

	for(const auto& chan: all_chans)
		{
		//dir_syst->cd();
		//TDirectory* dir_chan = (TDirectory*) dir_syst->mkdir(chan.name);
		//dir_chan->SetDirectory(dir_syst);
		for(const auto& proc: chan.procs)
			{
			//dir_chan->cd();
			//TDirectory* dir_proc = (TDirectory*) dir_chan->mkdir(proc.name);
			//dir_proc->SetDirectory(dir_chan);
			//dir_proc->cd();
			for(const auto& recorded_histo: proc.histos)
				{
				// the old order of the path
				//TString path = chan.name + "/" + proc.name + "/" + syst_name + "/";

				// get or create this path
				output_file->cd();
				TDirectory* chanpath = output_file->Get(chan.name) ? (TDirectory*) output_file->Get(chan.name) :  (TDirectory*) output_file->mkdir(chan.name);
				chanpath->cd();
				TDirectory* procpath = chanpath->Get(proc.name) ? (TDirectory*) chanpath->Get(proc.name) :  (TDirectory*) chanpath->mkdir(proc.name);
				procpath->cd();
				TDirectory* systpatch = procpath->Get(syst_name) ? (TDirectory*) procpath->Get(syst_name) :  (TDirectory*) procpath->mkdir(syst_name);
				systpatch->cd();

				TString histoname = chan.name + "_" + proc.name + "_" + syst_name + "_" + recorded_histo.main_name;
				recorded_histo.histo->SetName(histoname);

				// all final normalizations of the histogram
				if (isMC)
					normalise_final(recorded_histo.histo, main_dtag_info.cross_section, lumi, syst_name, chan.name, proc.name);

				recorded_histo.histo->Write();
				}
			}

		// same for catchall
		// the old order of the path
		//TString path = chan.name + "/" + chan.name_catchall_proc + "/" + syst_name + "/";

		//for(const auto& recorded_histo: chan.catchall_proc_histos)
		for (unsigned int rec_histo_i = 0; rec_histo_i<chan.catchall_proc_histos.size(); rec_histo_i++)
			{
			const auto& recorded_histo = chan.catchall_proc_histos[rec_histo_i];

			// get or create this path
			output_file->cd();
			TDirectory* chanpath = output_file->Get(chan.name) ? (TDirectory*) output_file->Get(chan.name) :  (TDirectory*) output_file->mkdir(chan.name);
			chanpath->cd();
			TDirectory* procpath = chanpath->Get(chan.name_catchall_proc) ? (TDirectory*) chanpath->Get(chan.name_catchall_proc) :  (TDirectory*) chanpath->mkdir(chan.name_catchall_proc);
			procpath->cd();
			TDirectory* systpatch = procpath->Get(syst_name) ? (TDirectory*) procpath->Get(syst_name) :  (TDirectory*) procpath->mkdir(syst_name);
			systpatch->cd();

			TString histoname = chan.name + "_" + chan.name_catchall_proc + "_" + syst_name + "_" + recorded_histo.main_name;
			//cerr_expr(histoname);
			recorded_histo.histo->SetName(histoname);

			if (isMC)
				normalise_final(recorded_histo.histo, main_dtag_info.cross_section, lumi, syst_name, chan.name, chan.name_catchall_proc);
			recorded_histo.histo->Write();

			// data simulation for each recorded distr if requested
			if (simulate_data && syst_name == "NOMINAL")
				{
				output_file->cd();
				TDirectory* chanpath = output_file->Get(chan.name) ? (TDirectory*) output_file->Get(chan.name) :  (TDirectory*) output_file->mkdir(chan.name);
				chanpath->cd();
				TDirectory* procpath = chanpath->Get("data") ? (TDirectory*) chanpath->Get("data") :  (TDirectory*) chanpath->mkdir("data");
				procpath->cd();
				TDirectory* systpatch = procpath->Get("NOMINAL") ? (TDirectory*) procpath->Get("NOMINAL") :  (TDirectory*) procpath->mkdir("NOMINAL");
				systpatch->cd();

				TString data_name = chan.name + "_data_NOMINAL_" + recorded_histo.main_name;
				if (output_file->Get(chan.name + "/data/NOMINAL/" + data_name)) continue;
				TH1D* data_histo = (TH1D*) recorded_histo.histo->Clone();
				data_histo->SetDirectory(systpatch);
				data_histo->SetName(data_name);

				//data_histo->Reset();
				//data_histo->Fill(1);
				// instead, add histograms from all processes
				for(const auto& proc: chan.procs)
					{
					data_histo->Add(proc.histos[rec_histo_i].histo);
					}

				data_histo->Write();
				}

			}
		}
	}
  }

else
  {
  for (int si=0; si<distrs_to_record.size(); si++)
	{
	// merge defined processes and the catchall
	TString& syst_name = distrs_to_record[si].name;
	vector<T_chan_proc_histos>& all_chans = distrs_to_record[si].chans;

	output_file->cd();
	TDirectory* dir_syst = (TDirectory*) output_file->mkdir(syst_name);
	//dir_syst->SetDirectory(output_file);

	for(const auto& chan: all_chans)
		{
		dir_syst->cd();
		TDirectory* dir_chan = (TDirectory*) dir_syst->mkdir(chan.name);
		//dir_chan->SetDirectory(dir_syst);
		for(const auto& proc: chan.procs)
			{
			dir_chan->cd();
			TDirectory* dir_proc = (TDirectory*) dir_chan->mkdir(proc.name);
			//dir_proc->SetDirectory(dir_chan);
			dir_proc->cd();

			for(const auto& recorded_histo: proc.histos)
				{
				//recorded_histo.histo->Print();
				// all final normalizations of the histogram
				if (isMC)
					normalise_final(recorded_histo.histo, main_dtag_info.cross_section, lumi, syst_name, chan.name, proc.name);
				recorded_histo.histo->Write();
				}
			}

		//for(const auto& recorded_histo: chan.catchall_proc_histos)
		for (unsigned int rec_histo_i = 0; rec_histo_i<chan.catchall_proc_histos.size(); rec_histo_i++)
			{
			const auto& recorded_histo = chan.catchall_proc_histos[rec_histo_i];

			// same for catchall
			dir_chan->cd();
			TDirectory* dir_proc_catchall = dir_chan->Get(chan.name_catchall_proc) ? (TDirectory*) dir_chan->Get(chan.name_catchall_proc) : (TDirectory*) dir_chan->mkdir(chan.name_catchall_proc);
			//dir_proc_catchall->SetDirectory(dir_chan);
			dir_proc_catchall->cd();

			if (isMC)
				normalise_final(recorded_histo.histo, main_dtag_info.cross_section, lumi, syst_name, chan.name, chan.name_catchall_proc);
			recorded_histo.histo->Write();

			// data simulation if requested
			if (simulate_data && syst_name == "NOMINAL")
				{
				dir_syst->cd();

				TDirectory* chanpath = dir_syst->Get(chan.name) ? (TDirectory*) dir_syst->Get(chan.name) :  (TDirectory*) dir_syst->mkdir(chan.name);
				chanpath->cd();
				TDirectory* procpath = chanpath->Get("data") ? (TDirectory*) chanpath->Get("data") :  (TDirectory*) chanpath->mkdir("data");
				procpath->cd();

				TString data_name = TString("NOMINAL_") + chan.name + "_data_" + recorded_histo.main_name;
				if (procpath->Get(data_name)) continue;
				TH1D* data_histo = (TH1D*) recorded_histo.histo->Clone();
				data_histo->SetDirectory(procpath);
				data_histo->SetName(data_name);

				//data_histo->Reset();
				//data_histo->Fill(1);
				// instead, add histograms from all processes
				for(const auto& proc: chan.procs)
					{
					data_histo->Add(proc.histos[rec_histo_i].histo);
					}

				data_histo->Write();
				}

			}
		}
	}
  }

output_file->cd();

if (normalise_per_weight)
	weight_counter->Write();

output_file->Close();
}

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

if (argc < 7)
	{
	std::cout << "Usage:" << " 0|1<simulate_data> 0|1<save_in_old_order> 0|1<do_WNJets_stitching> <lumi> <systs coma-separated> <chans> <procs> <distrs> output_filename input_filename [input_filename+]" << std::endl;
	exit(0);
	}

gROOT->Reset();

/* --- input options --- */

// set to normalize per gen lumi number of events
//bool normalise_per_weight = <user input>;

// a special weird feature:
// produce an output as if it were data
// with NOMINAL systematic
// and  data    proc name
// histograms are filled with 1
bool simulate_data       = Int_t(atoi(*argv++)) == 1; argc--;

bool save_in_old_order   = Int_t(atoi(*argv++)) == 1; argc--;
bool do_WNJets_stitching = Int_t(atoi(*argv++)) == 1; argc--;
Float_t lumi(atof(*argv++)); argc--;

vector<TString> requested_systematics = parse_coma_list(*argv++); argc--;
vector<TString> requested_channels    = parse_coma_list(*argv++); argc--;
vector<TString> requested_procs       = parse_coma_list(*argv++); argc--;
vector<TString> requested_distrs      = parse_coma_list(*argv++); argc--;

const char* output_filename = *argv++; argc--;

if  (do_not_overwrite)
	Stopif(access(output_filename, F_OK) != -1, exit(0);, "the output file exists %s", output_filename);


cerr_expr(do_WNJets_stitching << " " << output_filename);
/* --------------------- */


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

bool isMC = main_dtag.Contains("MC");
//if (!isMC) normalise_per_weight = false;
normalise_per_weight &= isMC;

// for WNJets stiching
bool skip_nup5_events = do_WNJets_stitching && isMC && main_dtag.Contains("WJets_madgraph");

//// define histograms for the distributions
//vector<TH1D_histo> distrs;



// define a nested list: list of channels, each containing a list of histograms to record
vector<T_syst_chan_proc_histos> distrs_to_record = setup_record_histos(
	main_dtag_info,
	requested_systematics ,
	requested_channels    ,
	requested_procs       ,
	requested_distrs      );

// --------------------------------- EVENT LOOP
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

	if (normalise_per_weight)
		{
		// get weight distribution for the file
		TH1D* weight_counter_in_file = (TH1D*) input_file->Get("weight_counter");
		// if the common weight counter is still not set -- clone
		if (!weight_counter)
			{
			weight_counter = (TH1D*) weight_counter_in_file->Clone();
			weight_counter->SetDirectory(0);
			}
		else
			{
			weight_counter->Add(weight_counter_in_file);
			}
		}

	// loop over events in the ttree and record the requested histograms
	event_loop(NT_output_ttree, distrs_to_record, skip_nup5_events, isMC);

	// close the input file
	input_file->Close();
	}

// if there is still no weight counter when it was requested
// then no files were processed (probably all were skipped)
Stopif(normalise_per_weight && !weight_counter, exit(0), "no weight counter even though it was requested, probably no files were processed, exiting")

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

// --------------------------------- OUTPUT
write_output(output_filename, distrs_to_record, main_dtag_info, lumi, isMC, save_in_old_order, simulate_data);
}

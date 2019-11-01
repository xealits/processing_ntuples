import argparse
import logging
from os.path import isfile




parser = argparse.ArgumentParser(
    formatter_class = argparse.RawDescriptionHelpFormatter,
    description = "get the syst rates from the hadded ntuple file",
    #epilog = "Example:\n$ python job_submit.py ttbarleps80_eventSelection jobing/my_runAnalysis_cfg_NEWSUBMIT.templ.py bin/ttbar-leptons-80X/analysis/dsets_testing_noHLT_TTbar.json test/tests/outdir_test_v11_ttbar_v8.40/"
    )

parser.add_argument("--debug",  action='store_true', help="DEBUG level of logging")

parser.add_argument('input_files', nargs='+', help="""the files to sum up, passed by shell,
as:

/gstore/t3cms/store/user/otoldaie/v19/TT_TuneCUETP8M2T4_13TeV-powheg-pythia8/Ntupler_v19_MC2016_Summer16_TTJets_powheg/180226_022336/0000/MC2016_Summer16_TTJets_powheg_*.root""")


args = parser.parse_args()

if args.debug:
    logging.basicConfig(level=logging.DEBUG)
else:
    logging.basicConfig(level=logging.INFO)

logging.info("import ROOT")

import ROOT
from ROOT import TH1D, TFile, TTree, gROOT
from ROOT import kGreen, kYellow, kOrange, kViolet, kAzure, kWhite, kGray, kRed, kCyan, TColor
from ROOT import TLegend
#from ROOT import THStack

logging.info("done")

# syst names
syst_names = {
"NOM_EVENTS" : 1,
"NOMINAL"    : 2,

"MU_PU"      : 3,
"MU_PUUp"    : 4,
"MU_PUDown"  : 5,
"El_PU"      : 6,
"El_PUUp"    : 7,
"El_PUDown"  : 8,
"TOPPT"      : 9,

# renorm refact scales
"M_NOM"    : 10,
"MrUp"     : 11,
"MrDown"   : 12,
"MfUp"     : 13,
"MfDown"   : 14,
"MfrUp"    : 15,
"MfrDown"  : 16,

# fragmentatopn
"Central"       : 21,
"FragUp"        : 22,
"FragDown"      : 23,
"SemilepBRUp"   : 24,
"SemilepBRDown" : 25,
"PetersonUp"    : 26,
# PDF and alphaS
"PDF_NOM"       : 27,
"AlphaSUp"      : 28,
"AlphaSDown"    : 29,

# PDFs
"PDFCT14n1"     :  31,
"PDFCT14n2"     :  32,
"PDFCT14n3"     :  33,
"PDFCT14n4"     :  34,
"PDFCT14n5"     :  35,
"PDFCT14n6"     :  36,
"PDFCT14n7"     :  37,
"PDFCT14n8"     :  38,
"PDFCT14n9"     :  39,
"PDFCT14n10"    :  40,
"PDFCT14n11"    :  41,
"PDFCT14n12"    :  42,
"PDFCT14n13"    :  43,
"PDFCT14n14"    :  44,
"PDFCT14n15"    :  45,
"PDFCT14n16"    :  46,
"PDFCT14n17"    :  47,
"PDFCT14n18"    :  48,
"PDFCT14n19"    :  49,
"PDFCT14n20"    :  50,
"PDFCT14n21"    :  51,
"PDFCT14n22"    :  52,
"PDFCT14n23"    :  53,
"PDFCT14n24"    :  54,
"PDFCT14n25"    :  55,
"PDFCT14n26"    :  56,
"PDFCT14n27"    :  57,
"PDFCT14n28"    :  58,
"PDFCT14n29"    :  59,
"PDFCT14n30"    :  60,
"PDFCT14n31"    :  61,
"PDFCT14n32"    :  62,
"PDFCT14n33"    :  63,
"PDFCT14n34"    :  64,
"PDFCT14n35"    :  65,
"PDFCT14n36"    :  66,
"PDFCT14n37"    :  67,
"PDFCT14n38"    :  68,
"PDFCT14n39"    :  69,
"PDFCT14n40"    :  70,
"PDFCT14n41"    :  71,
"PDFCT14n42"    :  72,
"PDFCT14n43"    :  73,
"PDFCT14n44"    :  74,
"PDFCT14n45"    :  75,
"PDFCT14n46"    :  76,
"PDFCT14n47"    :  77,
"PDFCT14n48"    :  78,
"PDFCT14n49"    :  79,
"PDFCT14n50"    :  80,
"PDFCT14n51"    :  81,
"PDFCT14n52"    :  82,
"PDFCT14n53"    :  83,
"PDFCT14n54"    :  84,
"PDFCT14n55"    :  85,
"PDFCT14n56"    :  86,
}

syst_histo = None

for filename in args.input_files:
    infile = TFile(filename)
    if syst_histo:
        syst_histo.Add(infile.Get("ntupler/systematic_weights"))
    else:
        _temp_h = infile.Get("ntupler/systematic_weights")
        syst_histo = _temp_h.Clone()
        syst_histo.SetDirectory(0)

def sys(name):
    return syst_histo.GetBinContent(1 + syst_names[name])

print "init", sys("NOM_EVENTS")
nom = sys("NOMINAL")
print "nom ", nom

for sname in ["MU_PU"      , "MU_PUUp"    , "MU_PUDown"  , "El_PU"      , "El_PUUp"    , "El_PUDown"  , "TOPPT"]:
    #
    print "%20s %20f" % (sname, sys(sname) / nom)

nom_frag = sys("Central")
for sname in [ "FragUp"        , "FragDown"      , "SemilepBRUp"   , "SemilepBRDown" , "PetersonUp"]:
    print "%20s %20f" % (sname, sys(sname) / nom_frag)

nom_scale = sys("M_NOM")
for sname in ["MrUp"     , "MrDown"   , "MfUp"     , "MfDown"   , "MfrUp"    , "MfrDown"]:
    print "%20s %20f" % (sname, sys(sname) / nom_scale)

nom_pdf = sys("PDF_NOM")
for sname in sorted(['AlphaSUp', 'AlphaSDown'] + [name for name in syst_names.keys() if "PDFCT14" in name]):
    print "%20s %20f" % (sname, sys(sname) / nom_pdf)




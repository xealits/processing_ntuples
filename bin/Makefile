
test: compile
	sumup_loop 1 outfile_test.root MC2017legacy_Fall17_TTTo2L2Nu_1.root foobar ../gstore_outdirs/94v3/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v3_MC2017legacy_Fall17_TTTo2L2Nu/191008_124833/0000/MC2017legacy_Fall17_TTTo2L2Nu_1.root

time: compile
	time sumup_loop 1 outfile_time.root ../lstore_outdirs/94v3/processing1/MC2017legacy_Fall17_TTTo2L2Nu/*root

compile: sumup_loop_up.C
	scram b
	touch compile


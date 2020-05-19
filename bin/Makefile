
test: simulate_data_output=0
test: order=0
test: chans=all
test: compile
	sumup_loop ${simulate_data_output} ${order} 1 41300 test ${chans} std Mt_lep_met_c,leading_lep_pt outfile_test.root ../lstore_outdirs/94v10/processing2/Data2017legacy_13TeV_SingleElectron_2017D/Data2017legacy_13TeV_SingleElectron_2017D_25.root

acceptance_test: simulate_data_output=0
acceptance_test: order=0
acceptance_test: compile
	sumup_loop ${simulate_data_output} ${order} 1 41300 std all std Mt_lep_met_c,leading_lep_pt outfile_acceptance_test.root ./acceptance_test_MC2017legacy_Fall17_TTTo2L2Nu_1.root

#../lstore_outdirs//94v5/processing1/MC2017legacy_Fall17_TTTo2L2Nu/MC2017legacy_Fall17_TTTo2L2Nu_1.root foobar ../gstore_outdirs/94v4/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v4_MC2017legacy_Fall17_TTTo2L2Nu/191118_204546/0000/MC2017legacy_Fall17_TTTo2L2Nu_1.root

time: simulate_data_output=0
time: order=0
time: compile
	time sumup_loop ${simulate_data_output} ${order} 1 41300 std all std Mt_lep_met_c,leading_lep_pt outfile_time.root ../lstore_outdirs/94v4/processing3/MC2017legacy_Fall17_TTTo2L2Nu/*root

compile: sumup_loop.C
	time scram b
	touch compile


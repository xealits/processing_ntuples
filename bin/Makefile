
test: simulate_data_output=0
test: order=0
test: compile
	sumup_loop ${simulate_data_output} ${order} 1 41300 test all std Mt_lep_met_c,leading_lep_pt outfile_test.root ../lstore_outdirs//94v4/processing3/MC2017legacy_Fall17_TTTo2L2Nu/MC2017legacy_Fall17_TTTo2L2Nu_1.root foobar ../gstore_outdirs/94v4/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v4_MC2017legacy_Fall17_TTTo2L2Nu/191118_204546/0000/MC2017legacy_Fall17_TTTo2L2Nu_1.root

time: simulate_data_output=0
time: order=0
time: compile
	time sumup_loop ${simulate_data_output} ${order} 1 41300 std all std Mt_lep_met_c,leading_lep_pt outfile_time.root ../lstore_outdirs/94v4/processing3/MC2017legacy_Fall17_TTTo2L2Nu/*root

compile: sumup_loop_up.C
	time scram b
	touch compile



test: interface_type=0
test: simulate_data_output=0
test: order=0
test: chans=all
test: compile
	sumup_loop ${interface_type} ${simulate_data_output} ${order} 1 41300 test ${chans} std Mt_lep_met_c,leading_lep_pt outfile_test.root ../lstore_outdirs/94v10/processing2/Data2017legacy_13TeV_SingleElectron_2017D/Data2017legacy_13TeV_SingleElectron_2017D_25.root

test_ntupler: interface_type=1
test_ntupler: simulate_data_output=0
test_ntupler: order=0
test_ntupler: chans=all
test_ntupler: compile
#	sumup_loop ${interface_type} ${simulate_data_output} ${order} 1 41300 test ${chans} std Mt_lep_met_c,leading_lep_pt outfile_test_ntupler2.root ../gstore_outdirs/94v15/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v15_MC2017legacy_Fall17_TTTo2L2Nu/200225_121541/0000/MC2017legacy_Fall17_TTTo2L2Nu_1.root
	sumup_loop ${interface_type} ${simulate_data_output} ${order} 1 41300 test ${chans} std Mt_lep_met_c,leading_lep_pt outfile_test_ntupler4.root ../gstore_outdirs/94v22/MC2017_Fall17_TTTo2L2Nu_1.root

acceptance_test: interface_type=0
acceptance_test: simulate_data_output=0
acceptance_test: order=0
acceptance_test: compile
	sumup_loop ${interface_type} ${simulate_data_output} ${order} 1 41300 std all std Mt_lep_met_c,leading_lep_pt outfile_acceptance_test.root ./acceptance_test_MC2017legacy_Fall17_TTTo2L2Nu_1.root
	# for comparison: plot the figures
	# or compare the files with vbindiff

#../lstore_outdirs//94v5/processing1/MC2017legacy_Fall17_TTTo2L2Nu/MC2017legacy_Fall17_TTTo2L2Nu_1.root foobar ../gstore_outdirs/94v4/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v4_MC2017legacy_Fall17_TTTo2L2Nu/191118_204546/0000/MC2017legacy_Fall17_TTTo2L2Nu_1.root

time: interface_type=0
time: simulate_data_output=0
time: order=0
time: compile
	time sumup_loop ${interface_type} ${simulate_data_output} ${order} 1 41300 std all std Mt_lep_met_c,leading_lep_pt outfile_time.root ../lstore_outdirs/94v4/processing3/MC2017legacy_Fall17_TTTo2L2Nu/*root

compile: sumup_loop.C
	time scram b
	touch compile


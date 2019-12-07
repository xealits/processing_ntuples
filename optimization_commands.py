import logging

command = '''python sumup_ttree_draw.py '{draw_com}' --histo-range {histo_range}   --cond-com '({selection_cond}) && ({proc_cond})' --output opt_{selection_name}_{proc_name}.root   --histo-name {selection_name}_{proc_name} {input_paths}'''

# name: condition command

# common processes
tt_mutau  = '(abs(gen_t_w_decay_id) > 15*15 && abs(gen_tb_w_decay_id) == 13) || (abs(gen_tb_w_decay_id) > 15*15 && abs(gen_t_w_decay_id) == 13)'
tt_eltau  = '(abs(gen_t_w_decay_id) > 15*15 && abs(gen_tb_w_decay_id) == 11) || (abs(gen_tb_w_decay_id) > 15*15 && abs(gen_t_w_decay_id) == 11)'
tt_tau3ch = 'abs(gen_t_w_decay_id) >= 15*30 || abs(gen_tb_w_decay_id) >= 15*30'
tt_tau1ch = 'abs(gen_t_w_decay_id) <  15*30 || abs(gen_tb_w_decay_id) <  15*30'

tt_lj = 'abs(gen_t_w_decay_id * gen_tb_w_decay_id) == 13 || abs(gen_t_w_decay_id * gen_tb_w_decay_id) == 11'
first_tau_b =  'tau_matching_gen_dR[0] <  0.3 && abs(tau_matching_gen[0]) == 6'
first_tau_w =  'tau_matching_gen_dR[0] <  0.3 && abs(tau_matching_gen[0]) == 5'
first_tau_o = '(tau_matching_gen_dR[0] >= 0.3) || (abs(tau_matching_gen[0]) != 5 && abs(tau_matching_gen[0]) != 6)'

processes = {
'tt_mutau':    tt_mutau,
'tt_mutau3ch': '(%s) && (%s)' % (tt_mutau, tt_tau3ch),
'tt_mutau1ch': '(%s) && (%s)' % (tt_mutau, tt_tau1ch),
'tt_eltau':    tt_eltau,
'tt_eltau3ch': '(%s) && (%s)' % (tt_eltau, tt_tau3ch),
'tt_eltau1ch': '(%s) && (%s)' % (tt_eltau, tt_tau1ch),

'tt_lj':   tt_lj,
'tt_lj_b': '(%s) && (%s)' % (tt_lj, first_tau_b),
'tt_lj_w': '(%s) && (%s)' % (tt_lj, first_tau_w),
'tt_lj_o': '(%s) && (%s)' % (tt_lj, first_tau_o),
}

selection_channels = {
'HLT_el': 'HLT_el',
'HLT_mu': 'HLT_mu',
'lep_el': 'HLT_el && abs(leps_ID) == 11',
'lep_mu': 'HLT_mu && abs(leps_ID) == 13',

'lep_el_1b':      'HLT_el && abs(leps_ID) == 11 && nbjets > 0',
'lep_mu_1b':      'HLT_mu && abs(leps_ID) == 13 && nbjets > 0',
'lep_el_1bNoTau': 'HLT_el && abs(leps_ID) == 11 && nbjets_noMediumTau > 0',
'lep_mu_1bNoTau': 'HLT_mu && abs(leps_ID) == 13 && nbjets_noMediumTau > 0',

'lep_el_2b':      'HLT_el && abs(leps_ID) == 11 && nbjets > 1',
'lep_mu_2b':      'HLT_mu && abs(leps_ID) == 13 && nbjets > 1',
'lep_el_2bNoTau': 'HLT_el && abs(leps_ID) == 11 && nbjets_noMediumTau > 1',
'lep_mu_2bNoTau': 'HLT_mu && abs(leps_ID) == 13 && nbjets_noMediumTau > 1',

'lep_el_tau':      'HLT_el && abs(leps_ID) == 11 && tau_IDlev[0] > 0',
'lep_mu_tau':      'HLT_mu && abs(leps_ID) == 13 && tau_IDlev[0] > 0',

'lep_el_tau_1b': 'HLT_el && abs(leps_ID) == 11 && tau_IDlev[0] > 0 && nbjets_noMediumTau > 0',
'lep_mu_tau_1b': 'HLT_mu && abs(leps_ID) == 13 && tau_IDlev[0] > 0 && nbjets_noMediumTau > 0',

'lep_el_tau_2b': 'HLT_el && abs(leps_ID) == 11 && tau_IDlev[0] > 0 && nbjets_noMediumTau > 1',
'lep_mu_tau_2b': 'HLT_mu && abs(leps_ID) == 13 && tau_IDlev[0] > 0 && nbjets_noMediumTau > 1',

'lep_el_tau_2bVLoose': 'HLT_el && abs(leps_ID) == 11 && tau_IDlev[0] > 0 && nbjets_noVLooseTau > 1',
'lep_mu_tau_2bVLoose': 'HLT_mu && abs(leps_ID) == 13 && tau_IDlev[0] > 0 && nbjets_noVLooseTau > 1',
}


# write down the files
input_paths = 'gstore_outdirs/94v5/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTTo2L2Nu/191201_173420/0000/*root'
draw_com    = '5'
histo_range = '10,0,10'

for pname, pcom in processes.items():
    for cname, ccom in selection_channels.items():
        the_command = command.format(draw_com=draw_com, histo_range=histo_range, selection_cond=ccom, proc_cond=pcom, proc_name=pname, selection_name=cname, input_paths=input_paths)
        #print the_command
        job_filename = "opt_{selection_name}_{proc_name}.sh".format(selection_name=cname, proc_name=pname)
        with open(job_filename, 'w') as f:
            f.write(the_command + '\n')


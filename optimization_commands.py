import logging
from os.path import isfile

write_jobs = False
submit_to_condor = True
command = '''python sumup_ttree_draw.py '{draw_com}' --histo-range {histo_range}   --cond-com '({selection_cond}) && ({proc_cond})' --output optimization-studies/temp/opt_{selection_name}_{proc_name}.root   --histo-name {selection_name}_{proc_name} {input_paths}'''

input_paths = 'gstore_outdirs/94v5/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTTo2L2Nu/191201_173420/0000/*root'
input_paths = 'gstore_outdirs/94v5/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTToSemiLeptonic_v2/191201_174541/0000/*root'
draw_com    = '5'
histo_range = '10,0,10'


jobs_sh_template = '''#!/bin/sh
pwd
export X509_USER_PROXY=
export SCRAM_ARCH=slc6_amd64_gcc700
export BUILD_ARCH=slc6_amd64_gcc700
export VO_CMS_SW_DIR=/cvmfs/cms.cern.ch
source $VO_CMS_SW_DIR/cmsset_default.sh
export CMS_PATH=$VO_CMS_SW_DIR
cd /afs/cern.ch/work/o/otoldaie/public/CMSSW_10_3_3/src/
cmsenv
cd UserCode/proc/
{command}
'''

jobs_dir = 'optimization_jobs/'

condor_sub_template = '''executable            = /afs/cern.ch/work/o/otoldaie/public/CMSSW_10_3_3/src/UserCode/proc/{jobs_dir}/{job_id}
arguments             = 
output                = output/{job_id}.$(ClusterId).$(ProcId).out
error                 = error/{job_id}.$(ClusterId).$(ProcId).err
log                   = log/{job_id}.$(ClusterId).log
queue
'''

# name: condition command

# common processes
tt_mutau  = '(abs(gen_t_w_decay_id) > 15*15 && abs(gen_tb_w_decay_id) == 13) || (abs(gen_tb_w_decay_id) > 15*15 && abs(gen_t_w_decay_id) == 13)'
tt_eltau  = '(abs(gen_t_w_decay_id) > 15*15 && abs(gen_tb_w_decay_id) == 11) || (abs(gen_tb_w_decay_id) > 15*15 && abs(gen_t_w_decay_id) == 11)'
tt_tau3ch = 'abs(gen_t_w_decay_id) >= 15*30 || abs(gen_tb_w_decay_id) >= 15*30' # one of leptons is a hadronic tau
tt_tau1ch = 'abs(gen_t_w_decay_id) <  15*30 && abs(gen_tb_w_decay_id) <  15*30' # no hadronic taus

tt_lj = 'abs(gen_t_w_decay_id * gen_tb_w_decay_id) == 13 || abs(gen_t_w_decay_id * gen_tb_w_decay_id) == 11'
first_tau_b =  'tau_matching_gen_dR[0] <  0.3 && abs(tau_matching_gen[0]) == 6'
first_tau_w =  'tau_matching_gen_dR[0] <  0.3 && abs(tau_matching_gen[0]) == 5'
first_tau_o = '(tau_matching_gen_dR[0] >= 0.3) || (abs(tau_matching_gen[0]) != 5 && abs(tau_matching_gen[0]) != 6)'

# measure selection efficiencies for each process
processes_ttbar_dilep = {
'tt_mutau':    tt_mutau,
'tt_mutau3ch': '(%s) && (%s)' % (tt_mutau, tt_tau3ch),
'tt_mutau1ch': '(%s) && (%s)' % (tt_mutau, tt_tau1ch),
'tt_eltau':    tt_eltau,
'tt_eltau3ch': '(%s) && (%s)' % (tt_eltau, tt_tau3ch),
'tt_eltau1ch': '(%s) && (%s)' % (tt_eltau, tt_tau1ch),
}

processes_ttbar_semilep = {
'tt_lj':   tt_lj,
'tt_lj_b': '(%s) && (%s)' % (tt_lj, first_tau_b),
'tt_lj_w': '(%s) && (%s)' % (tt_lj, first_tau_w),
'tt_lj_o': '(%s) && (%s)' % (tt_lj, first_tau_o),
}

# selections
# measure separately nbjets_noVLooseTau VS nbjets_noMediumTau
selections_basic   = {
'HLT_el_ttOS':     'HLT_el                     && abs(leps_ID) == 11 && tau_IDlev[0] > 0 && lep_id[0]*tau_id[0] < 0 && nbjets_noVLooseTau > 0',
'HLT_mu_ttOS':     'HLT_mu                     && abs(leps_ID) == 13 && tau_IDlev[0] > 0 && lep_id[0]*tau_id[0] < 0 && nbjets_noVLooseTau > 0',
'HLT_elj_ttOS':    'HLT_el_low_pt30_35PFJet    && abs(leps_ID) == 11 && tau_IDlev[0] > 0 && lep_id[0]*tau_id[0] < 0 && nbjets_noVLooseTau > 0',
'HLT_eltau_ttOS':  'HLT_eltau                  && abs(leps_ID) == 11 && tau_IDlev[0] > 0 && lep_id[0]*tau_id[0] < 0 && nbjets_noVLooseTau > 0',
'HLT_mutau_ttOS':  '(HLT_mutau1 || HLT_mutau2) && abs(leps_ID) == 13 && tau_IDlev[0] > 0 && lep_id[0]*tau_id[0] < 0 && nbjets_noVLooseTau > 0',
}

#'lep': [('2GeV', )] how to ask for 2GeV above trigger?
bjets_2L = 'nbjets_noVLooseTau > 1'
bjets_1M = '((abs(jet_matching_gen[0]) > 4 && jet_b_discr[0] > 0.4941) || (abs(jet_matching_gen[1]) > 4 && jet_b_discr[0] > 0.4941))'

selections_objects = {
'lep':   [('HLTmatched', 'lep_matched_HLT[0]')],
'tauID': [('Medium', 'tau_IDlev[0] > 2'), ('Tight', 'tau_IDlev[0] > 3')],
'tauSV': [('3ch',    'tau_SV_geom_flightLenSign[0] > 0.'), ('3sign', 'tau_SV_geom_flightLenSign[0] > 3.')],
'bjets': [('2L', bjets_2L), ('1M', bjets_1M)], # kind-of the medium jet
'correl': [('tauID2Lb',  'tau_IDlev[0] > 2 && %s' % bjets_2L),
           ('tauID1Mb',  'tau_IDlev[0] > 2 && %s' % bjets_1M),
           ('tau3ch2Lb', 'tau_SV_geom_flightLenSign[0] > 0. && %s' % bjets_2L),
           ('tau3ch1Mb', 'tau_SV_geom_flightLenSign[0] > 0. && %s' % bjets_1M),
           ('tau3SV2Lb', 'tau_SV_geom_flightLenSign[0] > 3. && %s' % bjets_2L),
           ('tau3SV1Mb', 'tau_SV_geom_flightLenSign[0] > 3. && %s' % bjets_1M),
           ('tauMID3ch', 'tau_IDlev[0] > 2 && tau_SV_geom_flightLenSign[0] > 0.'),
           ('tauTID3ch', 'tau_IDlev[0] > 3 && tau_SV_geom_flightLenSign[0] > 0.'),
           ('tauMID3SV', 'tau_IDlev[0] > 2 && tau_SV_geom_flightLenSign[0] > 3.'),
           ('tauTID3SV', 'tau_IDlev[0] > 3 && tau_SV_geom_flightLenSign[0] > 3.'),
],
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


processes = processes_ttbar_semilep
processes = processes_ttbar_dilep

# write down the files
if write_jobs:
  for pname, pcom in processes.items():
    #for cname, ccom in selection_channels.items():
    for basic_name, basic_com in selections_basic.items():
      selection_com  = '(%s)' % basic_com
      selection_name = basic_name
      the_command = command.format(draw_com=draw_com, histo_range=histo_range, selection_cond=selection_com, proc_cond=pcom, proc_name=pname, selection_name=selection_name, input_paths=input_paths)

      #print the_command
      job_id = 'opt_{selection_name}_{proc_name}.sh'.format(selection_name=selection_name, proc_name=pname)
      job_filename = "{jobs_dir}/{job_id}".format(job_id=job_id, jobs_dir=jobs_dir)

      if submit_to_condor:
        # write the job file
        with open(job_filename, 'w') as f:
            f.write(jobs_sh_template.format(command=the_command) + '\n')
        # and the condor sub file
        with open(job_filename + '.sub', 'w') as f:
            f.write(condor_sub_template.format(jobs_dir=jobs_dir, job_id=job_id) + '\n')

      else:
        with open(job_filename, 'w') as f:
          f.write(the_command + '\n')

      #with open(job_filename, 'w') as f:
      #    f.write(the_command + '\n')

      for obj_name, obj_stages in selections_objects.items():
        for obj_stage_name, obj_com in obj_stages:
          selection_com  = '(%s) && (%s)' % (basic_com, obj_com)
          selection_name = basic_name + '_' + obj_name + '_' + obj_stage_name
          #the_command = command.format(draw_com=draw_com, histo_range=histo_range, selection_cond=ccom, proc_cond=pcom, proc_name=pname, selection_name=cname, input_paths=input_paths)
          the_command = command.format(draw_com=draw_com, histo_range=histo_range, selection_cond=selection_com, proc_cond=pcom, proc_name=pname, selection_name=selection_name, input_paths=input_paths)

          #print the_command
          job_id = 'opt_{selection_name}_{proc_name}.sh'.format(selection_name=selection_name, proc_name=pname)
          job_filename = "{jobs_dir}/{job_id}".format(job_id=job_id, jobs_dir=jobs_dir)

          if submit_to_condor:
            # write the job file
            with open(job_filename, 'w') as f:
                f.write(jobs_sh_template.format(command=the_command) + '\n')
            # and the condor sub file
            with open(job_filename + '.sub', 'w') as f:
                f.write(condor_sub_template.format(jobs_dir=jobs_dir, job_id=job_id) + '\n')

          else:
            with open(job_filename, 'w') as f:
              f.write(the_command + '\n')

else:
    # make the study
    from ROOT import TFile

    f_opt_name = 'optimization-studies/opt_last.root'
    f_opt = TFile(f_opt_name)

    for basic_name, basic_com in selections_basic.items():
        print('-' * 25)
        print('%20s' % basic_name)

        for obj_name, obj_stages in selections_objects.items():
          print('%20s' % obj_name)
          for pname, pcom in processes.items():
            histo_basic_name = '%s_%s' % (basic_name, pname)
            #print histo_basic_name
            histo_basic = f_opt.Get(histo_basic_name)
            basic_integral = histo_basic.Integral()

            #print('%20s %g' % (histo_basic_name, basic_integral))
            if basic_integral == 0: continue

            obj_integrals = []
            obj_names     = []
            for obj_stage_name, obj_com in obj_stages:
                histo = f_opt.Get('%s_%s_%s_%s' % (basic_name, obj_name, obj_stage_name, pname))
                obj_names.append('%s_%s' % (obj_name, obj_stage_name))
                obj_integrals.append(histo.Integral())

            # names of object stages
            print('%20s  ' % (basic_name + '_' + obj_name) + ' '.join(['%12s' % name for name in obj_names]))
            # efficiencies of each object stage
            print '%15s  ' % pname + '%12g  ' % basic_integral + ' '.join(['%12.3g' % (hi / basic_integral) for hi in obj_integrals])


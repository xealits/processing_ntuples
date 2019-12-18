import logging
from ROOT import TFile, TH1D, TH2D, TLorentzVector
from ROOT.Math import LorentzVector

def lor_to_tlor(p4):
    return TLorentzVector(p4.X(), p4.Y(), p4.Z(), p4.T())

debug = False

if debug:
    logging.basicConfig(level=logging.DEBUG)
else:
    logging.basicConfig(level=logging.INFO)

infile_name = 'gstore_outdirs/94v5/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTTo2L2Nu/191201_173420/0000/MC2017legacy_Fall17_TTTo2L2Nu_1.root'
output_filename = 'check_isocands.root'

infile_names = [
  'gstore_outdirs/94v5/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTToSemiLeptonic_v2/191201_174541/0000/MC2017legacy_Fall17_TTToSemiLeptonic_v2_1.root',
  'gstore_outdirs/94v5/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTToSemiLeptonic_v2/191201_174541/0000/MC2017legacy_Fall17_TTToSemiLeptonic_v2_2.root',
]
output_filename = 'check_isocands_semilep.root'

suffix = '_semilep'

histos = {}

def add_histo(name, ranges, histo_type=TH1D):
    histos[name] = histo_type(name + suffix, '', *ranges)

"""
th_mass = TH1D('iso_mass' + suffix, '', 100, 0, 10)

th_tau_dm   = TH1D('tau_dm' + suffix, '', 20, 0, 20)
th_tau_gen  = TH1D('tau_gen' + suffix, '', 30, -15, 15)
th_tau_mass = TH1D('tau_mass' + suffix, '', 20, 0, 2)
th_tau_mass_iso_mass = TH1D('tau_mass_iso_mass' + suffix, '', 20, 0, 2)

th_ch_mass = TH1D('iso_ch_mass' + suffix, '', 100, 0, 10)
th_ch_char = TH2D('iso_ch_char' + suffix, '', 10, -5,  5, 4, -2,  2)
th_ch_pdgs = TH1D('iso_ch_pdgs' + suffix, '', 1000, -500, 500)
"""

add_histo('iso_mass', (100, 0, 10))
add_histo('tau_dm', (20, 0, 20))
add_histo('tau_gen', (30, -15, 15))
add_histo('tau_mass', (20, 0, 2))

add_histo('tau_n_all_isocands', (20, 0, 20))
add_histo('tau_n_chr_isocands', (20, 0, 20))
add_histo('tau_n_ntr_isocands', (20, 0, 20))
add_histo('tau_n_gam_isocands', (20, 0, 20))

add_histo('tau_dR_all_isocands', (20, 0, 0.4))
add_histo('tau_dR_chr_isocands', (20, 0, 0.4))
add_histo('tau_dR_ntr_isocands', (20, 0, 0.4))

add_histo('tau_dR_chr_isocands_pt', (20, 0, 0.4, 20, 0, 50), histo_type=TH2D)
add_histo('tau_dR_chr_isocands_charge', (20, 0, 0.4, 4, -2, 2), histo_type=TH2D)

add_histo('tau_mass_iso_ch_mass', (20, 0, 2, 20, 0, 5), histo_type=TH2D)

add_histo('tau_mass_iso_ch_mass_os', (20, 0, 2, 20, 0, 5), histo_type=TH2D)
add_histo('tau_mass_iso_ch_mass_ss', (20, 0, 2, 20, 0, 5), histo_type=TH2D)

add_histo('iso_ch_mass', (100, 0, 10))
add_histo('iso_ch_char', (10, -5,  5, 4, -2,  2), histo_type=TH2D)
add_histo('iso_ch_pdgs', (1000, -500, 500))


def iter_tau_index(what_to_iterate, tau_indices, tau_index):
    for i, index in enumerate(tau_indices):
        if index != tau_index:
            continue
        else:
            yield what_to_iterate[i]

def sum_tau_index(what_to_sum, tau_indices, tau_index, item_func=lambda x: x):
    #return sum(item_func(item) for i, item in enumerate(what_to_sum) if ev.tau_all_isolationCands_tau_index[i] == tau_index).mass()
    return sum(item_func(item) for item in iter_tau_index(what_to_sum, tau_indices, tau_index))

for infile_name in infile_names:
  tfile = TFile(infile_name)
  ttree = tfile.Get('ntupler/reduced_ttree')

  for iev, ev in enumerate(ttree):
    # only check taus with IDs
    if len(ev.tau_IDlev) == 0: continue
    if not (ev.tau_IDlev[0] > 0 and ev.tau_decayMode[0] > 9): continue
    histos['tau_n_all_isocands'].Fill(len(ev.tau_all_isolationCands_tau_index))
    histos['tau_n_chr_isocands'].Fill(len(ev.tau_all_isolationCharged_tau_index))
    histos['tau_n_ntr_isocands'].Fill(len(ev.tau_all_isolationNeutr_tau_index))
    histos['tau_n_gam_isocands'].Fill(len(ev.tau_all_isolationGamma_tau_index))

    if len(ev.tau_all_isolationCands_tau_index) == 0: continue

    '''
    gen_dR = ev.tau_matching_gen_dR[tau_index]
    gen_id = ev.tau_matching_gen[tau_index]

    matched_w = gen_dR < 0.3 and abs(gen_id) == 5
    matched_b = gen_dR < 0.3 and abs(gen_id) == 6
    '''

    gen_id = -11
    if ev.tau_matching_gen_dR[0] < 0.3:
        gen_id = ev.tau_matching_gen[0]

    if abs(gen_id) != 5: continue

    histos['tau_gen'].Fill(gen_id)

    tau_charge = 1 if ev.tau_id[0] > 0 else -1
    tau_momentum = ev.tau_p4[0]
    #tau_momentum_t = TLorentzVector(tau_momentum)
    tau_momentum_t = lor_to_tlor(tau_momentum)

    #root [6] ((TTree*) ntupler->Get("reduced_ttree"))->Scan("tau_all_isolationCands_tau_index[0]:tau_all_isolationCands_tau_index[1]:tau_all_isolationCands_tau_index[2]", "tau_IDlev[0]>0")
    # work with the 0 tau only, sumup all cands
    #mass   = sum_tau_index(ev.tau_all_isolationCharged, ev.tau_all_isolationCands_tau_index, 0)
    #charge = sum_tau_index(ev.tau_all_isolationCharged_pdgId, ev.tau_all_isolationCands_tau_index, 0, item_func=lambda x: int(x>0))
    #charge = sum_tau_index(ev.tau_all_isolationCharged_pdgId, ev.tau_all_isolationCands_tau_index, 0, item_func=lambda x: int(x>0))

    #mass   = sum(mom for mom in iter_tau_index(ev.tau_all_isolationCharged, ev.tau_all_isolationCands_tau_index, 0))
    # root crashes
    #for mom in iter_tau_index(ev.tau_all_isolationCharged, ev.tau_all_isolationCands_tau_index, 0):
    #    iso_momentum += mom

    iso_momentum = LorentzVector('ROOT::Math::PxPyPzE4D<double>')(0., 0., 0., 0.)
    for i, index in enumerate(ev.tau_all_isolationCands_tau_index):
        if index > 0: break
        iso_momentum += ev.tau_all_isolationCands[i]

    histos['iso_mass'].Fill(iso_momentum.mass())

    iso_ch_charge = 0
    iso_ch_momentum = LorentzVector('ROOT::Math::PxPyPzE4D<double>')(0., 0., 0., 0.)
    for i, index in enumerate(ev.tau_all_isolationCharged_tau_index):
        if index > 0: break
        iso_momentum = ev.tau_all_isolationCharged[i]
        charge   = int(1 if ev.tau_all_isolationCharged_pdgId[i] > 0 else -1)

        iso_ch_momentum += iso_momentum
        iso_ch_charge   += charge

        histos['iso_ch_pdgs'].Fill(ev.tau_all_isolationCharged_pdgId[i])
        histos['tau_mass_iso_ch_mass'].Fill(tau_momentum.mass(), (tau_momentum + iso_momentum).mass())

        iso_ch_mom_t = lor_to_tlor(iso_momentum)
        histos['tau_dR_chr_isocands'].Fill(tau_momentum_t.DeltaR(iso_ch_mom_t))
        histos['tau_dR_chr_isocands_pt'].Fill(tau_momentum_t.DeltaR(iso_ch_mom_t), iso_momentum.pt())

        opposite_sign = charge*tau_charge < 0
        histos['tau_dR_chr_isocands_charge'].Fill(tau_momentum_t.DeltaR(iso_ch_mom_t), opposite_sign)

        if opposite_sign:
            histos['tau_mass_iso_ch_mass_os'].Fill(tau_momentum.mass(), (tau_momentum + iso_momentum).mass())
        else:
            histos['tau_mass_iso_ch_mass_ss'].Fill(tau_momentum.mass(), (tau_momentum + iso_momentum).mass())

    histos['iso_ch_mass'].Fill((iso_ch_momentum + ev.tau_p4[0]).mass())
    histos['iso_ch_char'].Fill(iso_ch_charge, tau_charge)

    histos['tau_dm']   .Fill(ev.tau_decayMode[0])
    histos['tau_mass'] .Fill(tau_momentum.mass())

    #tj_p4 = TLorentzVector(p4.X(), p4.Y(), p4.Z(), p4.T())

  tfile.Close()

fout = TFile(output_filename, 'RECREATE')
fout.cd()

##th_mass.SetDirectory(fout)
#th_mass.Write()
#th_ch_mass.Write()
#th_ch_char.Write()
#th_ch_pdgs.Write()
#
#th_tau_dm.Write()
#th_tau_gen.Write()

for histo in histos.values():
    histo.Write()

fout.Write()
fout.Close()


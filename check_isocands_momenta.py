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

infile_names = ['gstore_outdirs/94v5/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTTo2L2Nu/191201_173420/0000/MC2017legacy_Fall17_TTTo2L2Nu_1.root']
suffix = 'dilep'
output_filename = 'check_isocands_%s_%s.root' % (suffix, '1')

infile_names = [
  'gstore_outdirs/94v5/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTToSemiLeptonic_v2/191201_174541/0000/MC2017legacy_Fall17_TTToSemiLeptonic_v2_1.root',
  'gstore_outdirs/94v5/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/Ntupler_94v5_MC2017legacy_Fall17_TTToSemiLeptonic_v2/191201_174541/0000/MC2017legacy_Fall17_TTToSemiLeptonic_v2_2.root',
]
suffix = 'semilep'
output_filename = 'check_isocands_%s_%s.root' % (suffix, '1')


histos = {}

def add_histo(name, ranges, histo_type=TH1D, title=''):
    histos[name] = histo_type(name + '_' + suffix, title, *ranges)

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

add_histo('leps_ID', (100, 0, 100))

add_histo('iso_mass', (100, 0, 10))
add_histo('tau_dm', (20, 0, 20))
add_histo('tau_gen', (30, -15, 15))
add_histo('tau_mass', (50, 0, 4))

add_histo('tau_charge_vs_sumsig', (4, -2, 2, 8, -4, 4), histo_type=TH2D, title='charges;tau charge;sum sig cands')

add_histo('tau_n_all_isocands', (20, 0, 20))
add_histo('tau_n_chr_isocands', (20, 0, 20))
add_histo('tau_n_ntr_isocands', (20, 0, 20))
add_histo('tau_n_gam_isocands', (20, 0, 20))

add_histo('tau_dR_all_isocands', (20, 0, 0.5))
add_histo('tau_dR_chr_isocands', (20, 0, 0.5))
add_histo('tau_dR_ntr_isocands', (20, 0, 0.5))

add_histo('tau_dR_chr_isocands_pt',     (20, 0, 0.5, 20, 0, 50), histo_type=TH2D)
add_histo('tau_dR_chr_isocands_energy', (20, 0, 0.5, 20, 0, 50), histo_type=TH2D)
add_histo('tau_dR_chr_isocands_charge', (20, 0, 0.5, 4, -2,  2), histo_type=TH2D)

add_histo('tau_mass_iso_ch_mass', (20, 0, 2, 20, 0, 5), histo_type=TH2D)

add_histo('tau_mass_iso_ch_mass_os', (20, 0, 2, 20, 0, 5), histo_type=TH2D)
add_histo('tau_mass_iso_ch_mass_ss', (20, 0, 2, 20, 0, 5), histo_type=TH2D)

add_histo('iso_ch_mass', (100, 0, 10))
add_histo('iso_ch_energies', (100, 0, 50))
add_histo('iso_ch_plus_tau_mass', (100, 0, 10))
add_histo('iso_ch_char', (10, -5,  5, 4, -2,  2), histo_type=TH2D)
add_histo('iso_ch_pdgs', (1000, -500, 500))

add_histo('iso_gm_energies',   (100, 0, 50))
add_histo('iso_gm_all_energy', (100, 0, 50))

add_histo('tau_dR_gm_isocands', (20, 0, 0.5))
add_histo('tau_dR_gm_isocands_energy', (20, 0, 0.5, 20, 0, 50), histo_type=TH2D)

add_histo('dressed_tau_dR_ch',    (20, 0, 0.5))
add_histo('dressed_tau_dR_sc',    (20, 0, 0.5))
add_histo('dressed_tau_dR_gamma', (20, 0, 0.5))
add_histo('dressed_tau_n_gamma',   (20, 0, 10))
add_histo('dressed_tau_n_charged', (20, 0, 10))
add_histo('dressed_tau_n_charged_sc', (20, 0, 10))
add_histo('dressed_tau_n_charged_oc_sc', (20, 0, 10, 20, 0, 10), histo_type=TH2D)

add_histo('dressed_tau_w_gamma_mass',    (50, 0, 6))

add_histo('dressed_tau_sc_n_masses',     (50, 0, 50))
add_histo('dressed_tau_sc_mass',         (50, 0, 6))
add_histo('dressed_tau_sc_mass_w_gamma', (50, 0, 6))

add_histo('dressed_tau_neutral_n_masses',     (50, 0, 50))
add_histo('dressed_tau_neutral_mass',         (50, 0, 6))
add_histo('dressed_tau_neutral_mass_w_gamma', (50, 0, 6))

add_histo('dressed_tau_neutral_pass_D0_test', (4, -1, 3))
add_histo('dressed_tau_neutral_pass_D0_test_genPDGids', (500, 0, 500))
add_histo('all_w_genPDGids', (500, 0, 500))

add_histo('dressed_tau_flipped_n_masses',     (50, 0, 50))
add_histo('dressed_tau_flipped_mass',         (50, 0, 6))
add_histo('dressed_tau_flipped_mass_w_gamma', (50, 0, 6))

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

    if abs(gen_id) not in (5, 3): continue # only W products! and true taus
    # ID 1 = leptons
    #    2 = tau1h
    #    3 = tau3h
    #    4 = taul
    #    5 = W
    #    6 = b

    histos['leps_ID'].Fill(abs(ev.leps_ID))

    histos['tau_gen'].Fill(gen_id)

    ## saving all gen level PDG IDs of W products
    #for pdgs in [ev.gen_t_w1_final_pdgIds, ev.gen_t_w2_final_pdgIds, ev.gen_tb_w1_final_pdgIds, ev.gen_tb_w2_final_pdgIds]:
    #    for genPDGid in pdgs:
    #         if abs(genPDGid) in (11,12,13,14):
    #             continue
    #         histos['all_w_genPDGids']   .Fill(abs(genPDGid))

    #tau_charge = 1 if ev.tau_id[0] > 0 else -1
    # actuallly tau charge sign is opposite to the sign of PDG ID
    # PDG ID of tau(-) is +15, but pi(+) is +211
    tau_charge = -1 if ev.tau_id[0] > 0 else +1
    tau_momentum = ev.tau_p4[0]
    #tau_momentum_t = TLorentzVector(tau_momentum)
    tau_momentum_t = lor_to_tlor(tau_momentum)

    histos['tau_dm']   .Fill(ev.tau_decayMode[0])
    histos['tau_mass'] .Fill(tau_momentum.mass())

    # save the charge of sig candidates -- to compare with the tauh cand charge
    sig_charge = 0
    for i, index in enumerate(ev.tau_all_signalCharged_tau_index):
        if index > 0: break
        sig_charge += 1 if ev.tau_all_signalCharged_pdgId[i] > 0 else -1

    histos['tau_charge_vs_sumsig']   .Fill(tau_charge, sig_charge)

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

    # isolation charged particles
    iso_ch_charge = 0
    iso_ch_momentum = LorentzVector('ROOT::Math::PxPyPzE4D<double>')(0., 0., 0., 0.)
    dressed_iso_opcharge_momenta = []
    dressed_iso_sc_momenta = []
    for i, index in enumerate(ev.tau_all_isolationCharged_tau_index):
        if index > 0: break
        iso_momentum = ev.tau_all_isolationCharged[i]
        charge   = int(1 if ev.tau_all_isolationCharged_pdgId[i] > 0 else -1)

        iso_ch_mom_t = lor_to_tlor(iso_momentum)
        iso_dR = tau_momentum_t.DeltaR(iso_ch_mom_t)

        iso_ch_momentum += iso_momentum
        iso_ch_charge   += charge

        opposite_sign = charge*tau_charge < 0
        histos['tau_dR_chr_isocands_charge'].Fill(iso_dR, opposite_sign)

        # save iso candidates with opposite charge to tau
        if opposite_sign:
            dressed_iso_opcharge_momenta.append(iso_ch_mom_t)
            # and record dR
            histos['dressed_tau_dR_ch'].Fill(iso_dR)
	else:
            dressed_iso_sc_momenta.append(iso_ch_mom_t)
            histos['dressed_tau_dR_sc'].Fill(iso_dR)

        histos['iso_ch_energies'].Fill(iso_momentum.energy())
        histos['iso_ch_pdgs'].Fill(ev.tau_all_isolationCharged_pdgId[i])
        histos['tau_mass_iso_ch_mass'].Fill(tau_momentum.mass(), (tau_momentum + iso_momentum).mass())

        histos['tau_dR_chr_isocands'].Fill(iso_dR)
        histos['tau_dR_chr_isocands_pt'].Fill(iso_dR, iso_momentum.pt())
        histos['tau_dR_chr_isocands_energy'].Fill(iso_dR, iso_momentum.energy())


        if opposite_sign:
            histos['tau_mass_iso_ch_mass_os'].Fill(tau_momentum.mass(), (tau_momentum + iso_momentum).mass())
        else:
            histos['tau_mass_iso_ch_mass_ss'].Fill(tau_momentum.mass(), (tau_momentum + iso_momentum).mass())

    histos['iso_ch_mass'].Fill(iso_ch_momentum.mass())
    histos['iso_ch_plus_tau_mass'].Fill((iso_ch_momentum + ev.tau_p4[0]).mass())
    histos['iso_ch_char'].Fill(iso_ch_charge, tau_charge)

    # isolation gammas
    dressed_iso_close_gamma = [] # photons in the peak close to the candidate
    dressed_iso_close_gamma_all = TLorentzVector(0,0,0,0)
    dressed_iso_close_gamma_dR = 0.2
    for i, index in enumerate(ev.tau_all_isolationGamma_tau_index):
        if index > 0: break
        iso_momentum = ev.tau_all_isolationGamma[i]
        histos['iso_gm_energies'].Fill(iso_momentum.energy())

        iso_mom_t = lor_to_tlor(iso_momentum)
        iso_dR = tau_momentum_t.DeltaR(iso_mom_t)
        histos['tau_dR_gm_isocands'].Fill(iso_dR)
        histos['tau_dR_gm_isocands_energy'].Fill(iso_dR, iso_momentum.energy())

        if iso_dR < dressed_iso_close_gamma_dR:
            dressed_iso_close_gamma.append(iso_mom_t)
            dressed_iso_close_gamma_all += iso_mom_t
            # and record dR
            histos['dressed_tau_dR_gamma'].Fill(iso_dR)

    histos['iso_gm_all_energy']   .Fill(dressed_iso_close_gamma_all.M())

    #tj_p4 = TLorentzVector(p4.X(), p4.Y(), p4.Z(), p4.T())

    histos['dressed_tau_n_gamma']   .Fill(len(dressed_iso_close_gamma))
    histos['dressed_tau_n_charged'] .Fill(len(dressed_iso_opcharge_momenta))
    histos['dressed_tau_n_charged_sc'] .Fill(len(dressed_iso_sc_momenta))

    histos['dressed_tau_n_charged_oc_sc'] .Fill(len(dressed_iso_opcharge_momenta), len(dressed_iso_sc_momenta))

    # calculate all tau masses
    all_dressed_tau_masses_neutral   = []
    all_dressed_tau_masses_neutral_g = []
    all_dressed_tau_masses_sc   = [] # [tau_momentum_t.M()]
    all_dressed_tau_masses_sc_g = [] # [(tau_momentum_t + dressed_iso_close_gamma_all).M()]

    histos['dressed_tau_w_gamma_mass']   .Fill((tau_momentum_t + dressed_iso_close_gamma_all).M())

    # adding 1 of possible charge iso candidates
    for tmom in dressed_iso_opcharge_momenta:
        tmom_sum = tau_momentum_t + tmom
        all_dressed_tau_masses_neutral.append(tmom_sum.M())
        all_dressed_tau_masses_neutral_g.append((tmom_sum + dressed_iso_close_gamma_all).M())
        # adding 1 same charge neutralizes the pair
        for tmom_sc in dressed_iso_sc_momenta:
            tmom_sum = tau_momentum_t + tmom + tmom_sc
            all_dressed_tau_masses_sc.append(tmom_sum.M())
            all_dressed_tau_masses_sc_g.append((tmom_sum + dressed_iso_close_gamma_all).M())

    # pairs of oc iso cands flip the charge of the dressed tau
    all_dressed_tau_masses_flipped   = []
    all_dressed_tau_masses_flipped_g = []
    for i, tmom1 in enumerate(dressed_iso_opcharge_momenta):
        for tmom2 in dressed_iso_opcharge_momenta[i+1:]:
            tmom_sum = tau_momentum_t + tmom1 + tmom2
            all_dressed_tau_masses_flipped.append(tmom_sum.M())
            all_dressed_tau_masses_flipped_g.append((tmom_sum + dressed_iso_close_gamma_all).M())

    histos['dressed_tau_neutral_n_masses']   .Fill(len(all_dressed_tau_masses_neutral))
    test_D0_mass = False
    for m in all_dressed_tau_masses_neutral:
        histos['dressed_tau_neutral_mass']   .Fill(m)
        if 1.7 < m < 2.: test_D0_mass = True
    for m in all_dressed_tau_masses_neutral_g:
        histos['dressed_tau_neutral_mass_w_gamma']   .Fill(m)

    histos['dressed_tau_neutral_pass_D0_test']   .Fill(test_D0_mass)
    ## if the D0 mass test passed, then get the gen level final states of W and save them for these events
    ## gen_t_w1_final_pdgIds
    #if test_D0_mass:
    #  for pdgs in [ev.gen_t_w1_final_pdgIds, ev.gen_t_w2_final_pdgIds, ev.gen_tb_w1_final_pdgIds, ev.gen_tb_w2_final_pdgIds]:
    #    for genPDGid in pdgs:
    #         if abs(genPDGid) in (11,12,13,14):
    #             continue
    #         histos['dressed_tau_neutral_pass_D0_test_genPDGids']   .Fill(abs(genPDGid))

    histos['dressed_tau_sc_n_masses']   .Fill(len(all_dressed_tau_masses_sc))
    for m in all_dressed_tau_masses_sc:
        histos['dressed_tau_sc_mass']   .Fill(m)
    for m in all_dressed_tau_masses_sc_g:
        histos['dressed_tau_sc_mass_w_gamma']   .Fill(m)

    histos['dressed_tau_flipped_n_masses']   .Fill(len(all_dressed_tau_masses_flipped))
    for m in all_dressed_tau_masses_flipped:
        histos['dressed_tau_flipped_mass']   .Fill(m)
    for m in all_dressed_tau_masses_flipped_g:
        histos['dressed_tau_flipped_mass_w_gamma']   .Fill(m)

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


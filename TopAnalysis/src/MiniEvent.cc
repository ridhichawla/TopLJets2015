#include "TopLJets2015/TopAnalysis/interface/MiniEvent.h"

//
void createMiniEventTree(TTree *t,MiniEvent_t &ev)
{
  //event header
  t->Branch("isData",     &ev.isData,     "isData/O");
  t->Branch("run",       &ev.run,       "run/I");
  t->Branch("event",     &ev.event,     "event/I");
  t->Branch("lumi",      &ev.lumi,      "lumi/I");

  //generator level event
  t->Branch("pu",      &ev.pu,      "pu/I");
  t->Branch("putrue",      &ev.putrue,      "putrue/I");
  t->Branch("ttbar_nw",        &ev.ttbar_nw,        "ttbar_nw/I");
  t->Branch("ttbar_allmepartons",        &ev.ttbar_allmepartons,        "ttbar_allmepartons/I");
  t->Branch("ttbar_matchmepartons",        &ev.ttbar_matchmepartons,        "ttbar_matchmepartons/I");
  t->Branch("ttbar_w",        ev.ttbar_w,        "ttbar_w[ttbar_nw]/F");

  //gen event (jets and dressed leptons)
  t->Branch("ngjets",       &ev.ngjets,       "ngjets/I");
  t->Branch("ngbjets",       &ev.ngbjets,       "ngbjets/I");
  t->Branch("ngleptons",       &ev.ngleptons,       "ngleptons/I");
  t->Branch("ng",       &ev.ng,       "ng/I");
  t->Branch("g_id",      ev.g_id,     "g_id[ng]/I");
  t->Branch("g_pt",      ev.g_pt,     "g_pt[ng]/F");
  t->Branch("g_eta",     ev.g_eta,    "g_eta[ng]/F");
  t->Branch("g_phi",     ev.g_phi,    "g_phi[ng]/F");
  t->Branch("g_m",       ev.g_m,      "g_m[ng]/F");

  //gen level J/Psi
  t->Branch("ngjpsi",       &ev.ngjpsi, "ngjpsi/I");
  t->Branch("ngmeson",      &ev.ngmeson, "ngmeson/I");
  t->Branch("gmeson_daug_id",  ev.gmeson_daug_id, "gmeson_daug_id[ngmeson]/I");
  t->Branch("gmeson_daug_pt",  ev.gmeson_daug_pt, "gmeson_daug_pt[ngmeson]/F");
  t->Branch("gmeson_daug_eta", ev.gmeson_daug_eta, "gmeson_daug_eta[ngmeson]/F");
  t->Branch("gmeson_daug_phi", ev.gmeson_daug_phi, "gmeson_daug_phi[ngmeson]/F");
  t->Branch("gmeson_id",     ev.gmeson_id, "gmeson_id[ngmeson]/I");
  t->Branch("gmeson_pt",     ev.gmeson_pt, "gmeson_pt[ngmeson]/F");
  t->Branch("gmeson_eta",    ev.gmeson_eta, "gmeson_eta[ngmeson]/F");
  t->Branch("gmeson_phi",    ev.gmeson_phi, "gmeson_phi[ngmeson]/F");
  t->Branch("gmeson_m",      ev.gmeson_m, "gmeson_m[ngmeson]/F");
  t->Branch("gmeson_daug_dR",  ev.gmeson_daug_dR, "gmeson_daug_dR[ngmeson]/F");
  t->Branch("gmeson_index",  ev.gmeson_index, "gmeson_index[ngmeson]/F");
  t->Branch("gmeson_daug_meson_index",  ev.gmeson_daug_meson_index, "gmeson_daug_meson_index[ngmeson]/F");

  //top (lastCopy and pseudo-top)
  t->Branch("ngtop",     &ev.ngtop,      "ngtop/I");
  t->Branch("gtop_id",    ev.gtop_id,    "gtop_id[ngtop]/I");
  t->Branch("gtop_pt",    ev.gtop_pt,    "gtop_pt[ngtop]/F");
  t->Branch("gtop_eta",   ev.gtop_eta,   "gtop_eta[ngtop]/F");
  t->Branch("gtop_phi",   ev.gtop_phi,   "gtop_phi[ngtop]/F");
  t->Branch("gtop_m",     ev.gtop_m,     "gtop_m[ngtop]/F");

  //final state
  t->Branch("ngpf",       &ev.ngpf,       "ngpf/I");
  t->Branch("gpf_id",      ev.gpf_id,     "gpf_id[ngpf]/I");
  t->Branch("gpf_c",       ev.gpf_c,      "gpf_c[ngpf]/I");
  t->Branch("gpf_g",       ev.gpf_g,      "gpf_g[ngpf]/I");
  t->Branch("gpf_pt",      ev.gpf_pt,     "gpf_pt[ngpf]/F");
  t->Branch("gpf_eta",     ev.gpf_eta,    "gpf_eta[ngpf]/F");
  t->Branch("gpf_phi",     ev.gpf_phi,    "gpf_phi[ngpf]/F");
  t->Branch("gpf_m",       ev.gpf_m,      "gpf_m[ngpf]/F");

  //reco level event
  t->Branch("nvtx",      &ev.nvtx,      "nvtx/I");
  t->Branch("muTrigger",        &ev.muTrigger,        "muTrigger/I");
  t->Branch("elTrigger",        &ev.elTrigger,        "elTrigger/I");

  t->Branch("nleptons", &ev.nleptons, "nleptons/I");
  t->Branch("nl", &ev.nl, "nl/I");
  t->Branch("isPromptFinalState",                         ev.isPromptFinalState,        "isPromptFinalState[nl]/O");
  t->Branch("isDirectPromptTauDecayProductFinalState",    ev.isDirectPromptTauDecayProductFinalState,        "isDirectPromptTauDecayProductFinalState[nl]/O");
  t->Branch("l_id",       ev.l_id,      "l_id[nl]/I");
  t->Branch("l_pid",      ev.l_pid,     "l_pid[nl]/I");
  t->Branch("l_g",        ev.l_g,       "l_g[nl]/I");
  t->Branch("l_charge",   ev.l_charge,  "l_charge[nl]/I");
  t->Branch("l_pt",       ev.l_pt,      "l_pt[nl]/F");
  t->Branch("l_eta",      ev.l_eta,     "l_eta[nl]/F");
  t->Branch("l_phi",      ev.l_phi,     "l_phi[nl]/F");
  t->Branch("l_mass",     ev.l_mass,    "l_mass[nl]/F");
  t->Branch("l_chargedHadronIso", ev.l_chargedHadronIso, "l_chargedHadronIso[nl]/F");
  t->Branch("l_miniIso",          ev.l_miniIso,          "l_miniIso[nl]/F");
  t->Branch("l_relIso",           ev.l_relIso,           "l_relIso[nl]/F");
  t->Branch("l_ip3d",             ev.l_ip3d,             "l_ip3d[nl]/F");
  t->Branch("l_ip3dsig",          ev.l_ip3dsig,          "l_ip3dsig[nl]/F");
  t->Branch("l_chi2norm",         ev.l_chi2norm,         "l_chi2norm[nl]/F");
  t->Branch("l_dxy",              ev.l_dxy,              "dxy[nl]/F");
  t->Branch("l_dxyE",             ev.l_dxyE,             "dxyE[nl]/F");
  t->Branch("l_dz",               ev.l_dz,               "dz[nl]/F");
  t->Branch("l_global",           ev.l_global,           "global[nl]/O");
  t->Branch("l_pf",               ev.l_pf,               "pf[nl]/O");
  t->Branch("l_nValTrackerHits",  ev.l_nValTrackerHits,  "nValTrackerHits[nl]/F");
  t->Branch("l_globalTrackNumberOfValidHits",                 ev.l_globalTrackNumberOfValidHits,                 "globalTrackNumberOfValidHits[nl]/F");
  t->Branch("l_nValPixelHits",    ev.l_nValPixelHits,    "nValPixelHits[nl]/F");
  t->Branch("l_pixelLayerWithMeasurement",    ev.l_pixelLayerWithMeasurement,    "pixelLayerWithMeasurement[nl]/F");
  t->Branch("l_nMatchedStations",    ev.l_nMatchedStations,    "nMatchedStations[nl]/F");
  t->Branch("l_trackerLayersWithMeasurement",    ev.l_trackerLayersWithMeasurement,    "trackerLayersWithMeasurement[nl]/F");
  t->Branch("l_validFraction",               ev.l_validFraction,               "pf[nl]/F");
  t->Branch("l_chi2LocalPosition",               ev.l_chi2LocalPosition,               "pf[nl]/F");
  t->Branch("l_trkKink",               ev.l_trkKink,               "pf[nl]/F");


  //jet info
  t->Branch("nj",        &ev.nj,        "nj/I");
  t->Branch("j_g",        ev.j_g,   "j_g[nj]/I");
  t->Branch("j_area",     ev.j_area,      "j_area[nj]/F");
  t->Branch("j_rawsf",    ev.j_rawsf,      "j_rawsf[nj]/F");
  t->Branch("j_pt",       ev.j_pt,      "j_pt[nj]/F");
  t->Branch("j_eta",      ev.j_eta,     "j_eta[nj]/F");
  t->Branch("j_phi",      ev.j_phi,     "j_phi[nj]/F");
  t->Branch("j_mass",     ev.j_mass,     "j_mass[nj]/F");
  t->Branch("j_csv",      ev.j_csv,     "j_csv[nj]/F");
  t->Branch("j_csvl",     ev.j_cvsl,     "j_cvsl[nj]/F");
  t->Branch("j_cvsb",     ev.j_cvsb,     "j_cvsb[nj]/F");
  t->Branch("j_vtxpx",    ev.j_vtxpx, "j_vtxpx[nj]/F");
  t->Branch("j_vtxpy",    ev.j_vtxpy, "j_vtxpy[nj]/F");
  t->Branch("j_vtxpz",    ev.j_vtxpz, "j_vtxpz[nj]/F");
  t->Branch("j_vtxmass",  ev.j_vtxmass, "j_vtxmass[nj]/F");
  t->Branch("j_vtxNtracks",  ev.j_vtxNtracks, "j_vtxNtracks[nj]/I");
  t->Branch("j_vtx3DVal",    ev.j_vtx3DVal, "j_vtx3DVal[nj]/F");
  t->Branch("j_vtx3DSig",    ev.j_vtx3DSig, "j_vtx3DSig[nj]/F");
  t->Branch("j_puid",        ev.j_puid,    "j_puid[nj]/F");  
  t->Branch("j_flav",        ev.j_flav,    "j_flav[nj]/I");
  t->Branch("j_hadflav",     ev.j_hadflav,    "j_hadflav[nj]/I");
  t->Branch("j_pid",         ev.j_pid,     "j_pid[nj]/I");

  //pf candidates (only charged if outside jets)
  t->Branch("npf",        &ev.npf,         "npf/I");
  t->Branch("pf_j",        ev.pf_j,        "pf_j[npf]/I");
  t->Branch("pf_id",       ev.pf_id,       "pf_id[npf]/I");
  t->Branch("pf_fromPV",   ev.pf_fromPV,   "pf_fromPV[npf]/I");
  t->Branch("pf_c",        ev.pf_c,        "pf_c[npf]/I");
  t->Branch("pf_pt",       ev.pf_pt,       "pf_pt[npf]/F");
  t->Branch("pf_eta",      ev.pf_eta,      "pf_eta[npf]/F");
  t->Branch("pf_phi",      ev.pf_phi,      "pf_phi[npf]/F");
  t->Branch("pf_m",        ev.pf_m,        "pf_m[npf]/F");
  t->Branch("pf_puppiWgt", ev.pf_puppiWgt, "pf_puppiWgt[npf]/F");
  t->Branch("pf_dxy",      ev.pf_dxy,      "pf_dxy[npf]/F");
  t->Branch("pf_dxyE",     ev.pf_dxyE,     "pf_dxyE[npf]/F");
  t->Branch("pf_dz",       ev.pf_dz,       "pf_dz[npf]/F");
  t->Branch("pf_dzE",      ev.pf_dzE,      "pf_dzE[npf]/F");
  t->Branch("pf_highPurity",      ev.pf_highPurity,      "pf_highPurity[npf]/O");
  t->Branch("pf_quality",      ev.pf_quality,      "pf_quality[npf]/I");
  t->Branch("pf_muon",   ev.pf_muon,   "pf_muon[npf]/O");
  t->Branch("pf_standAloneMuon",   ev.pf_standAloneMuon,   "pf_standAloneMuon[npf]/O");
  t->Branch("pf_globalMuon",   ev.pf_globalMuon,   "pf_globalMuon[npf]/O");
  t->Branch("pf_trackerMuon",   ev.pf_trackerMuon,   "pf_trackerMuon[npf]/O");

  //MET
  t->Branch("nmet",      &ev.nmet,     "nmet/I");
  t->Branch("met_pt",     ev.met_pt,   "met_pt[nmet]/F");
  t->Branch("met_phi",    ev.met_phi,  "met_phi[nmet]/F");
}

//
void attachToMiniEventTree(TTree *t,MiniEvent_t &ev,bool full)
{
  //event header
  t->SetBranchAddress("isData",     &ev.isData);
  t->SetBranchAddress("run",       &ev.run);
  t->SetBranchAddress("event",     &ev.event);
  t->SetBranchAddress("lumi",      &ev.lumi);

  //generator level event
  t->SetBranchAddress("pu",      &ev.pu);
  t->SetBranchAddress("putrue",      &ev.putrue);
  t->SetBranchAddress("ttbar_nw",        &ev.ttbar_nw);
  t->SetBranchAddress("ttbar_allmepartons",        &ev.ttbar_allmepartons);
  t->SetBranchAddress("ttbar_matchmepartons",        &ev.ttbar_matchmepartons);
  t->SetBranchAddress("ttbar_w",        ev.ttbar_w);

  //gen event (jets and dressed leptons)
  t->SetBranchAddress("ngjets",       &ev.ngjets);
  t->SetBranchAddress("ngbjets",       &ev.ngbjets);
  t->SetBranchAddress("ngleptons",       &ev.ngleptons);
  t->SetBranchAddress("ng",       &ev.ng);
  t->SetBranchAddress("g_id",      ev.g_id);
  t->SetBranchAddress("g_pt",      ev.g_pt);
  t->SetBranchAddress("g_eta",     ev.g_eta);
  t->SetBranchAddress("g_phi",     ev.g_phi);
  t->SetBranchAddress("g_m",       ev.g_m);

  //top (lastCopy and pseudo-top)
  t->SetBranchAddress("ngtop",     &ev.ngtop);
  t->SetBranchAddress("gtop_id",    ev.gtop_id);
  t->SetBranchAddress("gtop_pt",    ev.gtop_pt);
  t->SetBranchAddress("gtop_eta",   ev.gtop_eta);
  t->SetBranchAddress("gtop_phi",   ev.gtop_phi);
  t->SetBranchAddress("gtop_m",     ev.gtop_m);

  //final state
  if(full)
    {
      t->SetBranchAddress("ngpf",       &ev.ngpf);
      t->SetBranchAddress("gpf_id",      ev.gpf_id);
      t->SetBranchAddress("gpf_c",       ev.gpf_c);
      t->SetBranchAddress("gpf_g",       ev.gpf_g); 
      t->SetBranchAddress("gpf_pt",      ev.gpf_pt);
      t->SetBranchAddress("gpf_eta",     ev.gpf_eta);
      t->SetBranchAddress("gpf_phi",     ev.gpf_phi);
      t->SetBranchAddress("gpf_m",       ev.gpf_m);
    }

  //gen level J/Psi
  t->SetBranchAddress("ngjpsi",       &ev.ngmeson);
  t->SetBranchAddress("ngmeson",      &ev.ngmeson);
  t->SetBranchAddress("gmeson_daug_id",  ev.gmeson_daug_id);
  t->SetBranchAddress("gmeson_daug_pt",  ev.gmeson_daug_pt);
  t->SetBranchAddress("gmeson_daug_eta", ev.gmeson_daug_eta);
  t->SetBranchAddress("gmeson_daug_phi", ev.gmeson_daug_phi);
  t->SetBranchAddress("gmeson_id",     ev.gmeson_id);
  t->SetBranchAddress("gmeson_pt",     ev.gmeson_pt);
  t->SetBranchAddress("gmeson_eta",    ev.gmeson_eta);
  t->SetBranchAddress("gmeson_phi",    ev.gmeson_phi);
  t->SetBranchAddress("gmeson_m",      ev.gmeson_m);
  t->SetBranchAddress("gmeson_daug_dR",  ev.gmeson_daug_dR);
  t->SetBranchAddress("gmeson_index",  ev.gmeson_index);
  t->SetBranchAddress("gmeson_daug_meson_index",  ev.gmeson_daug_meson_index);

  //reco level event
  t->SetBranchAddress("nvtx",      &ev.nvtx);
  t->SetBranchAddress("muTrigger",        &ev.muTrigger);
  t->SetBranchAddress("elTrigger",        &ev.elTrigger);

  t->SetBranchAddress("nleptons", &ev.nleptons);
  t->SetBranchAddress("nl", &ev.nl);
  t->SetBranchAddress("isPromptFinalState",                         ev.isPromptFinalState);
  t->SetBranchAddress("isDirectPromptTauDecayProductFinalState",    ev.isDirectPromptTauDecayProductFinalState);
  t->SetBranchAddress("l_id",       ev.l_id);
  t->SetBranchAddress("l_pid",      ev.l_pid);
  t->SetBranchAddress("l_g",        ev.l_g);
  t->SetBranchAddress("l_charge",   ev.l_charge);
  t->SetBranchAddress("l_pt",       ev.l_pt);
  t->SetBranchAddress("l_eta",      ev.l_eta);
  t->SetBranchAddress("l_phi",      ev.l_phi);
  t->SetBranchAddress("l_mass",     ev.l_mass);
  t->SetBranchAddress("l_chargedHadronIso", ev.l_chargedHadronIso);
  t->SetBranchAddress("l_miniIso",          ev.l_miniIso);
  t->SetBranchAddress("l_relIso",           ev.l_relIso);
  t->SetBranchAddress("l_ip3d",             ev.l_ip3d);
  t->SetBranchAddress("l_ip3dsig",          ev.l_ip3dsig);
  t->SetBranchAddress("l_chi2norm",         ev.l_chi2norm);
  t->SetBranchAddress("l_dxy",              ev.l_dxy);
  t->SetBranchAddress("l_dz",               ev.l_dz);
  t->SetBranchAddress("l_global",           ev.l_global);
  t->SetBranchAddress("l_pf",               ev.l_pf);
  t->SetBranchAddress("l_nValTrackerHits",  ev.l_nValTrackerHits);
  t->SetBranchAddress("l_globalTrackNumberOfValidHits",               ev.l_globalTrackNumberOfValidHits);
  t->SetBranchAddress("l_nValPixelHits",    ev.l_nValPixelHits);
  t->SetBranchAddress("l_pixelLayerWithMeasurement",    ev.l_pixelLayerWithMeasurement);
  t->SetBranchAddress("l_nMatchedStations",    ev.l_nMatchedStations);
  t->SetBranchAddress("l_trackerLayersWithMeasurement",    ev.l_trackerLayersWithMeasurement);
  t->SetBranchAddress("l_validFraction",               ev.l_validFraction);
  t->SetBranchAddress("l_chi2LocalPosition",               ev.l_chi2LocalPosition);
  t->SetBranchAddress("l_trkKink",               ev.l_trkKink);

  //jet info
  t->SetBranchAddress("nj",        &ev.nj);
  t->SetBranchAddress("j_g",        ev.j_g);
  t->SetBranchAddress("j_area",     ev.j_area);
  t->SetBranchAddress("j_rawsf",    ev.j_rawsf);
  t->SetBranchAddress("j_pt",       ev.j_pt);
  t->SetBranchAddress("j_eta",      ev.j_eta);
  t->SetBranchAddress("j_phi",      ev.j_phi);
  t->SetBranchAddress("j_mass",     ev.j_mass);
  t->SetBranchAddress("j_csv",      ev.j_csv);
  t->SetBranchAddress("j_csvl",     ev.j_cvsl);
  t->SetBranchAddress("j_cvsb",     ev.j_cvsb);
  t->SetBranchAddress("j_vtxpx",    ev.j_vtxpx);
  t->SetBranchAddress("j_vtxpy",    ev.j_vtxpy);
  t->SetBranchAddress("j_vtxpz",    ev.j_vtxpz);
  t->SetBranchAddress("j_vtxmass",  ev.j_vtxmass);
  t->SetBranchAddress("j_vtxNtracks",  ev.j_vtxNtracks);
  t->SetBranchAddress("j_vtx3DVal",    ev.j_vtx3DVal);
  t->SetBranchAddress("j_vtx3DSig",    ev.j_vtx3DSig);
  t->SetBranchAddress("j_puid",        ev.j_puid);
  t->SetBranchAddress("j_flav",        ev.j_flav);
  t->SetBranchAddress("j_hadflav",     ev.j_hadflav);
  t->SetBranchAddress("j_pid",         ev.j_pid);

  //pf candidates (only charged if outside jets)
  if(full)
    {
      t->SetBranchAddress("npf",        &ev.npf);
      t->SetBranchAddress("pf_j",        ev.pf_j);
      t->SetBranchAddress("pf_id",       ev.pf_id);
      t->SetBranchAddress("pf_fromPV",   ev.pf_fromPV);
      t->SetBranchAddress("pf_c",        ev.pf_c);
      t->SetBranchAddress("pf_pt",       ev.pf_pt);
      t->SetBranchAddress("pf_eta",      ev.pf_eta);
      t->SetBranchAddress("pf_phi",      ev.pf_phi);
      t->SetBranchAddress("pf_m",        ev.pf_m);
      t->SetBranchAddress("pf_puppiWgt", ev.pf_puppiWgt);

      t->SetBranchAddress("pf_dxy",      ev.pf_dxy);
      t->SetBranchAddress("pf_dxyE",     ev.pf_dxyE);
      t->SetBranchAddress("pf_dz",       ev.pf_dz);
      t->SetBranchAddress("pf_dzE",      ev.pf_dzE);
      t->SetBranchAddress("pf_highPurity",      ev.pf_highPurity);
      t->SetBranchAddress("pf_quality",      ev.pf_quality);
      t->SetBranchAddress("pf_muon",       ev.pf_muon);
      t->SetBranchAddress("pf_standAloneMuon",       ev.pf_standAloneMuon);
      t->SetBranchAddress("pf_globalMuon",       ev.pf_globalMuon);
      t->SetBranchAddress("pf_trackerMuon",       ev.pf_trackerMuon);
    }

  //MET
  t->SetBranchAddress("nmet",      &ev.nmet);
  t->SetBranchAddress("met_pt",    ev.met_pt);
  t->SetBranchAddress("met_phi",   ev.met_phi);
}

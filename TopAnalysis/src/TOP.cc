#include <TFile.h>
#include <TROOT.h>
#include <TH1.h>
#include <TH2.h>
#include <TSystem.h>
#include <TGraph.h>
#include <TLorentzVector.h>
#include <TGraphAsymmErrors.h>
#include <TMath.h>

#include "TopLJets2015/TopAnalysis/interface/MiniEvent.h"
#include "TopLJets2015/TopAnalysis/interface/CharmEvent.h"
#include "TopLJets2015/TopAnalysis/interface/TOPWidth.h"
#include "TopLJets2015/TopAnalysis/interface/LeptonEfficiencyWrapper.h"
#include "TopLJets2015/TopAnalysis/interface/BtagUncertaintyComputer.h"
#include "TopLJets2015/TopAnalysis/interface/CorrectionTools.h"

#include "TopLJets2015/TopAnalysis/interface/OtherFunctions.h"
#include "TopLJets2015/TopAnalysis/interface/Trigger.h"
#include "TopLJets2015/TopAnalysis/interface/Particle.h"
#include "TopLJets2015/TopAnalysis/interface/Leptons.h"
#include "TopLJets2015/TopAnalysis/interface/Jet.h"
#include "TopLJets2015/TopAnalysis/interface/StdPlots.h"
#include "TopLJets2015/TopAnalysis/interface/CharmTree.h"
#include "TopLJets2015/TopAnalysis/interface/KalmanEvent.h"

#include <vector>
#include <iostream>
#include <algorithm>

#include "TMath.h"
#include "TopQuarkAnalysis/TopTools/interface/MEzCalculator.h"

using namespace std;

void RunTop(TString filename,
		 TString outname,
		 Int_t channelSelection, 
		 Int_t chargeSelection, 
		 FlavourSplitting flavourSplitting,
		 TH1F *normH, 
		 Bool_t runSysts,
                 TString era,
                 TString runPeriod,
                 Bool_t debug=false)
{
  if(debug) cout << "in RunTop" << endl;

  bool isTTbar( filename.Contains("_TTJets") );
  //bool isData( filename.Contains("Data13TeV") );
  
  //CREATE CHARM TREE IN FILE
  TString selPrefix("");  
  if(flavourSplitting!=NOFLAVOURSPLITTING) selPrefix=Form("%d_",flavourSplitting);
  TString baseName=gSystem->BaseName(outname); 
  TString dirName=gSystem->DirName(outname);
  TFile *fOut=TFile::Open(dirName+"/"+selPrefix+baseName,"RECREATE");
  TTree *cht = new TTree("data","Charm tree");
  CharmEvent_t evch;
  createCharmEventTree(cht,evch);

  //READ TREE FROM FILE
  MiniEvent_t ev;
  TFile *f = TFile::Open(filename);
  TH1 *puTrue=(TH1 *)f->Get("analysis/putrue");
  puTrue->SetDirectory(0);
  puTrue->Scale(1./puTrue->Integral());
  TTree *t = (TTree*)f->Get("analysis/data");
  attachToMiniEventTree(t,ev,true);
  Int_t nentries(t->GetEntriesFast());
  t->GetEntry(0);

  //TTree *tK = (TTree*)f->Get("kalman/data");
  //KalmanEvent kalman(tK);

  cout << "...producing " << outname << " from " << nentries << " events" << (runSysts ? " syst variations will be considered" : "") << endl;
  
  //PILEUP WEIGHTING
  std::vector<TGraph *>puWgtGr;
  //std::vector<TGraph *>puWgt;
  std::vector<TH1D *>puWgt;
  TString tmpRun ("BCDEFGH");
  std::vector<TH1D*> puWgtsRun;
  if(!ev.isData)
    {
      if(debug) cout << "loading pileup weight" << endl;
      TString puWgtUrl(era+"/pileupWgts"+runPeriod+".root");
      gSystem->ExpandPathName(puWgtUrl);
      TFile *fIn=TFile::Open(puWgtUrl);
      for(size_t i=0; i<3; i++)
	{
	  TString grName("pu_nom");
          if(i==1) grName="pu_down";
          if(i==2) grName="pu_up";
          TH1D *puWgtData=(TH1D *)fIn->Get("puwgts_nom");
          puWgt.push_back(puWgtData);
	}

      //Load PU plots for all run periods
      /*
      for(int ic = 0; ic < tmpRun.Length(); ic++) {
        TString puWgtRunUrl(era+"/pileupWgts"+tmpRun[ic]+".root");
        gSystem->ExpandPathName(puWgtRunUrl);
        fIn=TFile::Open(puWgtRunUrl);
        TH1D *puWgtDataRun=(TH1D *)fIn->Get("puwgts_nom");
        puWgtDataRun->SetDirectory(0);
	puWgtsRun.push_back(puWgtDataRun);
        fIn->Close();
        
      }
      */
      TString puWgtRunUrl(era+"/pileupWgtsBCDEF.root");
      gSystem->ExpandPathName(puWgtRunUrl);
      fIn=TFile::Open(puWgtRunUrl);
      TH1D *puWgtDataRun=(TH1D *)fIn->Get("puwgts_nom");
      puWgtDataRun->SetDirectory(0);
      puWgtsRun.push_back(puWgtDataRun);
      fIn->Close();
      puWgtRunUrl = era+"/pileupWgtsGH.root";
      gSystem->ExpandPathName(puWgtRunUrl);
      fIn=TFile::Open(puWgtRunUrl);
      puWgtDataRun=(TH1D *)fIn->Get("puwgts_nom");
      puWgtDataRun->SetDirectory(0);
      puWgtsRun.push_back(puWgtDataRun);
      fIn->Close();
      if(debug) cout << "loading pileup weight DONE" << endl;
    }

  //LEPTON EFFICIENCIES
  LeptonEfficiencyWrapper lepEffH(filename.Contains("Data13TeV"),era,runPeriod,debug);
  LeptonEfficiencyWrapper lepEffH_BCDEF(filename.Contains("Data13TeV"),era,"BCDEF",debug);
  LeptonEfficiencyWrapper lepEffH_GH(filename.Contains("Data13TeV"),era,"GH",debug);


  //B-TAG CALIBRATION
  TString btagEffExpUrl(era+"/expTageff.root");
  gSystem->ExpandPathName(btagEffExpUrl);
  std::map<BTagEntry::JetFlavor, BTagCalibrationReader *> btvsfReaders  = getBTVcalibrationReaders(era,BTagEntry::OP_MEDIUM);
  std::map<TString, TGraphAsymmErrors *> expBtagEff, expBtagEffPy8;
  BTagSFUtil myBTagSFUtil;
  
  TFile *beffIn=TFile::Open(btagEffExpUrl);
  expBtagEffPy8["b"]=(TGraphAsymmErrors *)beffIn->Get("b");
  expBtagEffPy8["c"]=(TGraphAsymmErrors *)beffIn->Get("c");
  expBtagEffPy8["udsg"]=(TGraphAsymmErrors *)beffIn->Get("udsg");
  beffIn->Close();

  TString btagExpPostFix("");
  if(isTTbar)
	{
	  if(filename.Contains("_herwig")) btagExpPostFix="_herwig";
	  if(filename.Contains("_scaleup")) btagExpPostFix="_scaleup";
	  if(filename.Contains("_scaledown")) btagExpPostFix="_scaledown";
	}
  btagEffExpUrl.ReplaceAll(".root",btagExpPostFix+".root");
  beffIn=TFile::Open(btagEffExpUrl);
  expBtagEff["b"]=(TGraphAsymmErrors *)beffIn->Get("b");
  expBtagEff["c"]=(TGraphAsymmErrors *)beffIn->Get("c");
  expBtagEff["udsg"]=(TGraphAsymmErrors *)beffIn->Get("udsg");
  beffIn->Close();


  //jet energy uncertainties
  TString jecUncUrl(era+"/Summer16_23Sep2016V4_MC_UncertaintySources_AK4PFchs.txt");
  gSystem->ExpandPathName(jecUncUrl);
  JetCorrectorParameters *jecParam = new JetCorrectorParameters(jecUncUrl.Data(), "Total");
  JetCorrectionUncertainty *jecUnc = new JetCorrectionUncertainty( *jecParam );
  
  //LIST OF SYSTEMATICS
  
  //HISTOGRAMS BY RUNPERIOD
  /*
  StdPlots runB("B", outname, debug);
  StdPlots runC("C", outname, debug);
  StdPlots runD("D", outname, debug);
  StdPlots runE("E", outname, debug);
  StdPlots runF("F", outname, debug);
  StdPlots runG("G", outname, debug);
  StdPlots runH("H", outname, debug);
  */
  StdPlots runBCDEF("BCDEF", outname, debug);
  StdPlots runGH("GH", outname, debug);
  
  CharmTree treeBCDEF(t, "BCDEF", outname, debug);
  CharmTree treeGH(t, "GH", outname, debug);

  //BOOK HISTOGRAMS
  std::map<TString, TH1 *> allPlots;
  allPlots["puwgtctr"] = new TH1F("puwgtctr","Weight sums",4,0,4);
  allPlots["puwgtgr"] = new TH1F("puwgtgr","PU Weights (calc)",75,0,75);
  allPlots["puwgt"] = new TH1F("puwgt","PU Weights (data)",75,0,75);
  allPlots["topptwgt"] = new TH1F("topptwgt","Top p_{T} weights", 2, 0, 2);
  std::vector<TString> lfsVec = { "_all", "_e", "_ee", "_em", "_mm", "_m" }; 
  std::vector<TString> cutVec = { "", "_lep", "_lepjets", "_jpsi", "_csv", "_meson" };
  std::vector<TString> wgtVec = { "", "_no_weight" };

  for(int i = 0; i < (int)lfsVec.size(); i++) {
    TString tag(lfsVec[i]);
    allPlots["pid"+tag] = new TH1F("pid"+tag,";pid;Events triggered",3,0,3);
  for(int j = 0; j < (int)cutVec.size(); j++) {
  for(int k = 0; k < (int)wgtVec.size(); k++) {
    TString cut(cutVec[j]);
    TString weight(wgtVec[k]);
    allPlots["lp_pt_iso"+tag+cut+weight] = new TH1F("lp_pt_iso"+tag+cut+weight,";Lepton P_{T} [GeV] after cleaning;Events / 10 GeV", 20, 0,200);
    allPlots["lp_pt_veto"+tag+cut+weight] = new TH1F("lp_pt_veto"+tag+cut+weight,";Lepton P_{T} [GeV] after veto;Events / 10 GeV", 20, 0,200);
    allPlots["lp_pt_low"+tag+cut+weight] = new TH1F("lp_pt_low"+tag+cut+weight,";Leading Lepton P_{T} [GeV];Events / 1 GeV", 20, 20,40);
    allPlots["lp_pt"+tag+cut+weight] = new TH1F("lp_pt"+tag+cut+weight,";Leading lepton P_{T} [GeV];Events / 10 GeV", 20, 0,200);
    allPlots["l2p_pt"+tag+cut+weight] = new TH1F("l2p_pt"+tag+cut+weight,";Sub-leading lepton P_{T} [GeV];Events / 10 GeV", 20, 0,200);
    allPlots["dilp_pt"+tag+cut+weight] = new TH1F("dilp_pt"+tag+cut+weight,";Lepton P_{T} [GeV];Events / 10 GeV", 20, 0,200);
    allPlots["dilp_m"+tag+cut+weight] = new TH1F("dilp_m"+tag+cut+weight,";M_{ll} [GeV];Events / 10 GeV", 20, 0,200);
    allPlots["j_pt"+tag+cut+weight] = new TH1F("j_pt"+tag+cut+weight,";Leading light Jet P_{T} [GeV];Events / 20 GeV", 15, 0,300);
    allPlots["lj_pt"+tag+cut+weight] = new TH1F("lj_pt"+tag+cut+weight,";Leading light Jet P_{T} [GeV];Events / 20 GeV", 15, 0,300);
    allPlots["bj_pt"+tag+cut+weight] = new TH1F("bj_pt"+tag+cut+weight,";Leading b Jet P_{T} [GeV];Events / 20 GeV", 15, 0,300);
    allPlots["j_pt_low"+tag+cut+weight] = new TH1F("j_pt_low"+tag+cut+weight,";Leading light Jet P_{T} [GeV];Events / 1 GeV", 20, 30,50);
    allPlots["lj_pt_low"+tag+cut+weight] = new TH1F("lj_pt_low"+tag+cut+weight,";Leading light Jet P_{T} [GeV];Events / 1 GeV", 20, 30,50);
    allPlots["bj_pt_low"+tag+cut+weight] = new TH1F("bj_pt_low"+tag+cut+weight,";Leading b Jet P_{T} [GeV];Events / 1 GeV", 20, 30,50);
    allPlots["nlp"+tag+cut+weight]     = new TH1F("nlp"+tag+cut+weight,";N_{l};Events" ,3,0.,3.);
    allPlots["ndilp"+tag+cut+weight]     = new TH1F("ndilp"+tag+cut+weight,";N_{ll};Events" ,3,0.,3.);
    allPlots["nj"+tag+cut+weight]     = new TH1F("nj"+tag+cut+weight,";N_{jets} (P_{T} > 30 GeV);Events" ,10,0,10.);
    allPlots["nlj"+tag+cut+weight]     = new TH1F("nlj"+tag+cut+weight,";N_{jets} (P_{T} > 30 GeV);Events" ,10,0,10.);
    allPlots["nbj"+tag+cut+weight]     = new TH1F("nbj"+tag+cut+weight,";N_{b-jets} (CSV > 0.8);Events" ,4,1.,5.);
    allPlots["npf"+tag+cut+weight]     = new TH1F("npf"+tag+cut+weight,";N_{pf};Events / 10" ,5,0.,5.);
    allPlots["lp_eta"+tag+cut+weight]  = new TH1F("lp_eta"+tag+cut+weight,";Leading lepton #eta; Events / 0.1", 30, -2.5,2.5);
    allPlots["l2p_eta"+tag+cut+weight]  = new TH1F("l2p_eta"+tag+cut+weight,";Sub-Leading lepton #eta; Events / 0.1", 30, -2.5,2.5);
    allPlots["lp_phi"+tag+cut+weight]  = new TH1F("lp_phi"+tag+cut+weight,";Leading lepton #phi; Events", 50, -3.14,3.14);
    allPlots["l2p_phi"+tag+cut+weight]  = new TH1F("l2p_phi"+tag+cut+weight,";Sub-Leading lepton #phi; Events", 50, -3.14,3.14);
    allPlots["nstart"+tag+cut+weight]     = new TH1F("jetindex"+tag+cut+weight,";N_{jetindex};Events" ,5,0.,5.);
    allPlots["pfid"+tag+cut+weight]     = new TH1F("pfid"+tag+cut+weight,";PFID;Events" ,440,-220.,220.);
    allPlots["massJPsi"+tag+cut+weight]     = new TH1F("massJPsi"+tag+cut+weight,";M_{ll};Events / 18 MeV" ,50,2.5,3.4);
    allPlots["massJPsiK"+tag+cut+weight]     = new TH1F("massJPsiK"+tag+cut+weight,";M_{llk};Events / 15 MeV" ,100,4.5,6);
    allPlots["massD0"+tag+cut+weight]     = new TH1F("massD0"+tag+cut+weight,";M_{D^{0}};Events / 3 MeV" ,100,1.7,2.0);
    allPlots["massD0_lep"+tag+cut+weight]     = new TH1F("massD0_lep"+tag+cut+weight,";M_{K#pi};Events / 3 MeV" ,100,1.7,2.0);
    allPlots["massD0_mu"+tag+cut+weight]     = new TH1F("massD0_mu"+tag+cut+weight,";M_{K#pi};Events / 3 MeV" ,100,1.7,2.0);
    allPlots["massD0_e"+tag+cut+weight]     = new TH1F("massD0_ele"+tag+cut+weight,";M_{K#pi};Events / 3 MeV" ,100,1.7,2.0);
    allPlots["massDsmD0loose"+tag+cut+weight]     = new TH1F("massDsmD0loose"+tag+cut+weight,";M_{K#pi#pi} - M_{K#pi};Events / 0.5 MeV" ,20,0.14,0.16);
    allPlots["massDsmD0"+tag+cut+weight]     = new TH1F("massDsmD0"+tag+cut+weight,";M_{K#pi#pi} - M_{K#pi};Events / 0.5 MeV" ,20,0.14,0.16);
    allPlots["massDs"+tag+cut+weight]     = new TH1F("massDs"+tag+cut+weight,";M_{D^{*}};Events / 10 MeV" ,200,0.,2.0);
    allPlots["pi_pt"+tag+cut+weight] = new TH1F("pi_pt"+tag+cut+weight,";#pi^{#pm} P_{T} [GeV];Events / 5 GeV", 10, 0,50);
    allPlots["MET"+tag+cut+weight] = new TH1F("MET"+tag+cut+weight,";MET [GeV];Events / 20 GeV", 10,0,200);
    allPlots["HT"+tag+cut+weight] = new TH1F("HT"+tag+cut+weight,";HT [GeV];Events / 20 GeV", 10,0,200);
    allPlots["ST"+tag+cut+weight] = new TH1F("ST"+tag+cut+weight,";ST [GeV];Events / 20 GeV", 10,0,200);
    allPlots["MET2oST"+tag+cut+weight] = new TH1F("MET2oST"+tag+cut+weight,";MET2oST [GeV];Events / 20 GeV", 10,0,200);
    allPlots["charge"+tag+cut+weight] = new TH1F("charge"+tag+cut+weight,";Charge(l_{1}*l_{2});Events", 5,-2,2);
    allPlots["csv"+tag+cut+weight] = new TH1F("CSV"+tag+cut+weight,";Jet CSV;Events / 0.1", 10,0,1);
    allPlots["dR"+tag+cut+weight] = new TH1F("dR"+tag+cut+weight,";dR;Events / 0.05", 20,0.0,1.);
    allPlots["pflp_pt"+tag+cut+weight] = new TH1F("pflp_pt"+tag+cut+weight,";PF lepton P_{T} [GeV];Events / 0.2 GeV", 15, 0,3);
    allPlots["massZ"+tag+cut+weight]     = new TH1F("massZ_control"+tag+cut+weight,";M_{ll};Events / 1.0 GeV" ,30,81,111);
    allPlots["chargeZ"+tag+cut+weight]     = new TH1F("chargeZ_control"+tag+cut+weight,";M_{ll};Events / 1.0 GeV" ,5,-2,2);
    allPlots["nevt"+tag+cut+weight]     = new TH1F("nevt"+tag+cut+weight,";N_{events};Events" ,1,1.,2.);
    allPlots["weight"+tag+cut+weight]     = new TH1F("weight"+tag+cut+weight,";N_{events};Events/ 1.0" ,20,0.,2.);
    allPlots["norm"+tag+cut+weight]     = new TH1F("norm"+tag+cut+weight,";N_{events};Events / 1.0" ,2,0.,2.);
    allPlots["relIso"+tag+cut+weight] = new TH1F("relIso"+tag+cut+weight,";relIso;Events / 0.01", 25,0,0.25);
    allPlots["nvtx"+tag+cut+weight]     = new TH1F("nvtx"+tag+cut+weight,";N_{PV};Events / 1.0" ,50,0.,50.);
    allPlots["chi2"+tag+cut+weight] = new TH1F("normchi2"+tag+cut+weight,";#chi^2/n.d.o.f.;Events", 10,0.,10.);
    allPlots["lp_dxy"+tag+cut+weight] = new TH1F("lp_dxy"+tag+cut+weight,";d_{xy} [cm];Events / 0.01 #mum", 20, 0, 0.2);
    allPlots["lp_dz"+tag+cut+weight] = new TH1F("lp_dz"+tag+cut+weight,";d_{z} [cm];Events / 0.01 #mum", 50, 0, 0.5);
    allPlots["pf_dxy"+tag+cut+weight] = new TH1F("pf_dxy"+tag+cut+weight,";d_{xy} [cm];Events / 0.02 #mum", 20, 0, 0.1);
    allPlots["pf_dz"+tag+cut+weight] = new TH1F("pf_dz"+tag+cut+weight,";d_{z} [cm];Events / 0.02 #mum", 20, 0, 0.1);
    allPlots["pf_dxyE"+tag+cut+weight] = new TH1F("pf_dxyE"+tag+cut+weight,";#sigma(d_{xy}) [cm];Events / 0.02 #mum", 20, 0, 0.1);
    allPlots["pf_dzE"+tag+cut+weight] = new TH1F("pf_dzE"+tag+cut+weight,";#sigma(d_{z}) [cm];Events / 0.02 #mum", 20, 0, 0.1);
    allPlots["pf_dxy_sig"+tag+cut+weight] = new TH1F("pf_dxy_significance"+tag+cut+weight,";d_{xy};Events / 1", 30, 0, 30);
    allPlots["pf_dz_sig"+tag+cut+weight] = new TH1F("pf_dz_significance"+tag+cut+weight,";d_{z};Events / 1", 30, 0, 30);

  }
  }
  }
    allPlots["nevt_iso"] = new TH1F("nevt_iso",";After Isolation;Events", 1,1.,2.);
    allPlots["nevt_veto"] = new TH1F("nevt_veto",";After Veto;Events", 1,1.,2.);
    allPlots["nevt_trigger"] = new TH1F("nevt_trigger",";After Trigger;Events", 1,1.,2.);
    allPlots["norm_iso"] = new TH1F("norm_iso",";After Isolation;Events / 1.0", 20,0,2.);
    allPlots["norm_veto"] = new TH1F("norm_veto",";After Veto;Events / 1.0", 20,0.,2.);
    allPlots["nvtx_iso"]     = new TH1F("nvtx_iso",";N_{PV};Events / 1.0" ,50,0.,50.);
    allPlots["nvtx_veto"]     = new TH1F("nvtx_veto",";N_{PV};Events / 1.0" ,50,0.,50.);
    allPlots["chi2_iso"] = new TH1F("normchi2_iso",";#chi^2/n.d.o.f.;Events", 10,0.,10.);
    allPlots["chi2_veto"] = new TH1F("normchi2_veto",";#chi^2/n.d.o.f.;Events", 10,0.,10.);
    allPlots["lp_dxy_iso"] = new TH1F("lp_dxy_iso",";d_{xy} [cm];Events / 0.01 #mum", 20, 0, 0.2);
    allPlots["lp_dxy_veto"] = new TH1F("lp_dxy_veto",";d_{xy} [cm];Events / 0.01 #mum", 20, 0, 0.2);
    allPlots["lp_dz_iso"] = new TH1F("lp_dz_iso",";d_{z} [cm];Events / 0.01 #mum", 50, 0, 0.5);
    allPlots["lp_dz_veto"] = new TH1F("lp_dz_veto",";d_{z} [cm];Events / 0.01 #mum", 50, 0, 0.5);


  for (auto& it : allPlots)   { it.second->Sumw2(); it.second->SetDirectory(0); }
  //for (auto& it : all2dPlots) { it.second->Sumw2(); it.second->SetDirectory(0); }

  //LOOP OVER EVENTS
  for (Int_t iev=0;iev<nentries;iev++)
    {
      t->GetEntry(iev);
      if(iev%5000==0) printf ("\r [%3.0f/100] done",100.*(float)(iev)/(float)(nentries));
      //Normalize to XSec and lumi
      float norm(1.0);
      if(!ev.isData) {
        norm =  normH ? normH->GetBinContent(1) : 1.0;
	//update nominal event weight
	if(ev.ttbar_nw>0) norm*=ev.ttbar_w[0];
      }
      /*
      runB.SetNorm(norm);
      runC.SetNorm(norm);
      runD.SetNorm(norm);
      runE.SetNorm(norm);
      runF.SetNorm(norm);
      runG.SetNorm(norm);
      runH.SetNorm(norm);
      */
      runBCDEF.SetNorm(norm);
      runGH.SetNorm(norm);
      treeBCDEF.SetNorm(norm);
      treeGH.SetNorm(norm);

      //Apply top pT weight to ttbar events
      //https://twiki.cern.ch/twiki/bin/viewauth/CMS/TopPtReweighting#Run_2_strategy
      //Particle(float pt, float eta, float phi, float mass, int pdgId, float relIso, int pid);
      std::vector<Particle> tops;
      if(isTTbar) {
        float top_pt_wgt(1.0);
        vector<float> pt;
        for(int i = 0; i < ev.ngtop; i++) {
          //Aviod stops stored by ntupelizer
          if(abs(ev.gtop_id[i]) != 6) continue;
          float tpt = ev.gtop_pt[i];
          if(tpt > 400) tpt = 400;
          pt.push_back(tpt);
          if(debug) std::cout << "Top pT= " << tpt << std::endl;
          tops.push_back(Particle(ev.gtop_pt[i], ev.gtop_eta[i], ev.gtop_phi[i], ev.gtop_m[i], ev.gtop_id[i], 0, 0));
        }
        //Save onlt hardest two tops (ttbar)
        //Might not need after imopsing |PdgId|==6
        std::sort(pt.begin(), pt.end());
        std::reverse(pt.begin(), pt.end());
        //Calculate SFs based on expontial
        //https://twiki.cern.ch/twiki/bin/view/CMS/TopPtReweighting#Eventweight
        top_pt_wgt *= TMath::Exp(0.0615 - 0.0005*pt[0]);
        top_pt_wgt *= TMath::Exp(0.0615 - 0.0005*pt[1]);
        top_pt_wgt = TMath::Sqrt(top_pt_wgt);
        allPlots["topptwgt"]->Fill(0.,1.0);
        allPlots["topptwgt"]->Fill(1.,top_pt_wgt);
        /*
        runB.SetTopPtWgt(top_pt_wgt);
        runC.SetTopPtWgt(top_pt_wgt);
        runD.SetTopPtWgt(top_pt_wgt);
        runE.SetTopPtWgt(top_pt_wgt);
        runF.SetTopPtWgt(top_pt_wgt);
        runG.SetTopPtWgt(top_pt_wgt);
        runH.SetTopPtWgt(top_pt_wgt);
        */
        runBCDEF.SetTopPtWgt(top_pt_wgt);
        runGH.SetTopPtWgt(top_pt_wgt);
        treeBCDEF.SetTopPtWgt(top_pt_wgt);
        treeGH.SetTopPtWgt(top_pt_wgt);
      }

      allPlots["nevt_all"]->Fill(1,norm);
      allPlots["norm_all"]->Fill(norm,norm);
      allPlots["nvtx_all"]->Fill(ev.nvtx,norm);

      //Basic lepton kinematics
      std::vector<int> tightLeptons,vetoLeptons;
      Leptons Muons(Tight,debug);
      Leptons Electrons(TightNoIso,debug);
      Leptons VetoLeptons(Veto,Loose,debug); // Designate Veto, only veto on Loose

      Muons.setMinPt(20);
      Muons.setMaxEta(2.4);
      Muons.setMaxRelIso(0.15);

      //Electrons.setMinPt(12);
      Electrons.setMinPt(30);
      Electrons.setMaxEta(2.4);
      Electrons.setMaxRelIso(0.15);

      VetoLeptons.setMinPt(15);
      VetoLeptons.setMaxEta(2.4);
      VetoLeptons.setMaxRelIso(0.24);
      VetoLeptons.setMaxType(Loose);

      for(int il=0; il<ev.nl; il++)
	{
          //cout << "in lepton selection" << endl;
          Particle p(ev.l_pt[il], ev.l_eta[il], ev.l_phi[il], ev.l_mass[il], ev.l_id[il]*ev.l_charge[il], ev.l_relIso[il], ev.l_pid[il]);
          if(p.isMuon()) {
            Muons.addParticle(p); //only accepts tight
            VetoLeptons.addParticle(p); //only accepts loose FIXME think of more elegant way
          }
          else if(p.isElectron()) {
            Electrons.addParticle(p);
            VetoLeptons.addParticle(p);
            if(p.getType()==TightNoIso)
              allPlots["pid_all"]->Fill(1);
            if(p.getType()==Tight)
              allPlots["pid_all"]->Fill(2);
            else
              allPlots["pid_all"]->Fill(0);
          }
	}

      //Single Muon has tighter constraints
      if(Muons.size() == 1) {
        Muons.changeMinPt(26);
        Muons.changeMaxEta(2.1);
      }
      if(Muons.size() == 1) {
	allPlots["lp_pt_iso_m"]->Fill(Muons.getElement(0).Pt(),norm);
        if(VetoLeptons.size()==0)
	  allPlots["lp_pt_veto_m"]->Fill(Muons.getElement(0).Pt(),norm);
      }
      if(Electrons.size() == 1) {
        Electrons.changeMinPt(30);
        Electrons.changeParticleType(Tight); //TightNoIso -> Tight
      }
      if(Electrons.size() == 1) {
	allPlots["lp_pt_iso_e"]->Fill(Electrons.getElement(0).Pt(),norm);
        if(VetoLeptons.size()==0)
	  allPlots["lp_pt_veto_e"]->Fill(Electrons.getElement(0).Pt(),norm);
      }
      if(debug) cout << "lepton selection DONE" << endl;
      Leptons leptons(Tight,debug);
      leptons.combineLeptons(Muons);
      leptons.combineLeptons(Electrons);
      if(debug) cout << "sorting leptons" << endl;
      leptons.sortLeptonsByPt();

      allPlots["nevt_iso"]->Fill(1,norm);
      allPlots["norm_iso"]->Fill(norm,norm);
      allPlots["nvtx_iso"]->Fill(ev.nvtx,norm);
      
      //USE VETO HERE
      if(VetoLeptons.size()>0) continue; //veto only on lep+jets
      allPlots["nevt_veto"]->Fill(1,norm);
      allPlots["norm_veto"]->Fill(norm,norm);
      allPlots["nvtx_veto"]->Fill(ev.nvtx,norm);

      //check if triggers have fired
      //Trigger(muonTriggers, electronTriggers, debug=0)
      //Parse triggers
      Trigger trigger = Trigger(ev.muTrigger, ev.elTrigger, debug);
      //Check filetype (M/E/MM/EE/EM)
      trigger.setDataType(filename);

      //Dielectron
      //trigger.addRequiredDoubleElectronTrigger({"HLT_DoubleEle24_22_eta2p1_WPLoose_Gsf_v","HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v"});
      trigger.addRequiredDoubleElectronTrigger("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v");
      //non-DZ
      //trigger.addRequiredDoubleElectronTrigger("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v"); //FIXME not in Pedro's code, Carmen says prescaled
      //Prescaled

      //Dimuon
      trigger.addRequiredDoubleMuonTrigger({"HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v","HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v"});
      //non-DZ
      //trigger.addRequiredDoubleMuonTrigger({"HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v","HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v"}); //Not in Pedro's code
      //Prescaled


      //Electron Muon (ME as well)
      trigger.addRequiredEMTrigger({"HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_v","HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v","HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v"});
      //non-DZ
      trigger.addRequiredEMTrigger({"HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v","HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_v"});

      //Single muon
      //Check only these triggers
      trigger.addRequiredMuonTrigger("HLT_IsoMu24_v");
      trigger.addRequiredMuonTrigger("HLT_IsoTkMu24_v");
      //trigger.addRequiredMuonTrigger{""HLT_IsoMu24_v","HLT_IsoTkMu24_v"}) also works


      //Single electron
      //trigger.addRequiredElectronTrigger("HLT_Ele27_WPTight_Gsf_v");
      trigger.addRequiredElectronTrigger("HLT_Ele32_eta2p1_WPTight_Gsf_v");
      //trigger.addRequiredElectronTrigger("HLT_Ele25_eta2p1_WPTight_Gsf_v");

      //Only triggers in Pedro's code
      trigger.addRequiredElectronTrigger({"HLT_Ele32_eta2p1_WPTight_Gsf_v", "HLT_DoubleEle24_22_eta2p1_WPLoose_Gsf_v"});

      //decide the channel
      if(debug) cout << "decide channel" << endl;
      TString chTag("");

      if(debug) if(leptons.size()==0) cout << "NO LEPTONS!!" << endl;
      if(leptons.size()==0) continue;
      if(trigger.isSingleElectronEvent(leptons)) chTag="e";
      else if(trigger.isSingleMuonEvent(leptons)) chTag="m";
      else if(trigger.isDoubleElectronEvent(leptons)) chTag="ee";
      else if(trigger.isDoubleMuonEvent(leptons)) chTag="mm";
      else if(trigger.isEMEvent(leptons)) chTag="em";
      else continue;
      if(debug) cout << "check if Single Electron fired Single Muon trigger" << endl;
      if(leptons.size() == 2 && trigger.muonFired() && trigger.isElectronFile()) continue;
      chTag = "_"+chTag;
      if(debug) cout << "decide channel DONE" << endl;
      if(debug) cout << "Event: " << iev << endl;
      allPlots["nevt_trigger"]->Fill(1,norm);
      for(size_t i=0; i<leptons.size(); i++) {
        if(leptons[i].getType()==TightNoIso)
          allPlots["pid"+chTag]->Fill(1);
        else if(leptons[i].getType()==Tight)
          allPlots["pid"+chTag]->Fill(2);
        else
          allPlots["pid"+chTag]->Fill(0);
      }

      //one good lepton either isolated or in the non-isolated sideband or a Z candidate
      Bool_t isZ(false);//,isZPassingSIP3d(false);
      TLorentzVector l1p4,l2p4,dilp4;
      if(leptons.size()==2)
	{	  
          if(debug) cout << "di-lepton" << endl;
	  l1p4 = leptons[0].getVec();
	  l2p4 = leptons[1].getVec();
	  dilp4=l1p4+l2p4;
          if(leptons[0].getPdgId() == -leptons[1].getPdgId() &&
          //if(chTag=="mm" || chTag=="ee" &&
	     fabs(dilp4.M()-91)<15)
	    { 
	      isZ=true; 
	    }
          if(debug) cout << "di-lepton DONE" << endl;
	}

      //save lepton kinematics
      Float_t stsum(ev.met_pt[0]);
      for(size_t il=0; il<leptons.size(); il++)
	{
          stsum += leptons[il].Pt();
	}

      //select jets
      Float_t htsum(0);
      TLorentzVector jetDiff(0,0,0,0);
      int nbjets(0),ncjets(0),nljets(0);//,leadingJetIdx(-wgt);
      std::vector<Jet> bJetsVec, lightJetsVec, allJetsVec;
      for (int k=0; k<ev.nj;k++)
	{
	  //check kinematics
	  TLorentzVector jp4;
	  jp4.SetPtEtaPhiM(ev.j_pt[k],ev.j_eta[k],ev.j_phi[k],ev.j_mass[k]);
	  //jp4=updateJES(jp4,ev.j_rawsf[k],ev.j_area[k],ev.rho,ev.nvtx,jetCorr);

	  //cross clean with respect to leptons deltaR<0.4
          bool overlapsWithLepton(false);
          for(size_t il=0; il<leptons.size(); il++) {
            if(jp4.DeltaR(leptons[il].getVec())>0.4) continue;  //Jet is fine
	    overlapsWithLepton=true;                   //Jet ovelaps with an "isolated" lepton, event is bad
          }
          if(overlapsWithLepton) continue;
          if(debug) cout << "Overlap with lepton DONE" << endl;

	  //smear jet energy resolution for MC
	  //jetDiff -= jp4;
	  float genJet_pt(0);
	  if(ev.j_g[k]>-1) genJet_pt=ev.g_pt[ ev.j_g[k] ];
	  if(!ev.isData && genJet_pt>0) 
	    {
	      float jerSmear=getJetResolutionScales(jp4.Pt(),jp4.Pt(),genJet_pt)[0];
	      jp4 *= jerSmear;
	    }
	  //jetDiff += jp4;

	  // re-inforce kinematics cuts
	  if(jp4.Pt()<30) continue;
	  if(fabs(jp4.Eta()) > 2.4) continue;
	  
	  //if(leadingJetIdx<0) leadingJetIdx=k;
	  htsum += jp4.Pt();

	  //b-tag
	  if(debug) cout << "Starting b-tagging" << endl;
	  float csv = ev.j_csv[k];	  
	  bool isBTagged(csv>0.8484);//,isBTaggedUp(isBTagged),isBTaggedDown(isBTagged);
	  if(!ev.isData)
	    {
	      float jptForBtag(jp4.Pt()>1000. ? 999. : jp4.Pt()), jetaForBtag(fabs(jp4.Eta()));
	      float expEff(1.0), jetBtagSF(1.0);//, jetBtagSFUp(1.0), jetBtagSFDown(1.0);

	      BTagEntry::JetFlavor hadFlav=BTagEntry::FLAV_UDSG; 
	      if(abs(ev.j_hadflav[k])==4) hadFlav=BTagEntry::FLAV_C; 
	      if(abs(ev.j_hadflav[k])==5) hadFlav=BTagEntry::FLAV_B;

	      jetBtagSF = btvsfReaders[hadFlav]->eval_auto_bounds( "central", hadFlav, jetaForBtag, jptForBtag);
	      //jetBtagSFUp = btvsfReaders[hadFlav]->eval_auto_bounds( "up", hadFlav, jetaForBtag, jptForBtag);
	      //jetBtagSFUp = btvsfReaders[hadFlav]->eval_auto_bounds( "down", hadFlav, jetaForBtag, jptForBtag);
	      if(abs(ev.j_hadflav[k])==4) 
		{ 
		  ncjets++;
		  expEff    = expBtagEff["c"]->Eval(jptForBtag); 
		  jetBtagSF *= expEff>0 ? expBtagEffPy8["c"]->Eval(jptForBtag)/expBtagEff["c"]->Eval(jptForBtag) : 0.;
		}
	      else if(abs(ev.j_hadflav[k])==5) 
		{ 
		  nbjets++;
		  expEff    = expBtagEff["b"]->Eval(jptForBtag); 
		  jetBtagSF *= expEff>0 ? expBtagEffPy8["b"]->Eval(jptForBtag)/expBtagEff["b"]->Eval(jptForBtag) : 0.;
		}
	      else
		{
		  nljets++;
		  expEff    = expBtagEff["udsg"]->Eval(jptForBtag);
		  jetBtagSF *= expEff> 0 ? expBtagEffPy8["udsg"]->Eval(jptForBtag)/expBtagEff["udsg"]->Eval(jptForBtag) : 0.;
		}
	      
	      //updated b-tagging decision with the data/MC scale factor
	      myBTagSFUtil.modifyBTagsWithSF(isBTagged,    jetBtagSF,     expEff);
	      //myBTagSFUtil.modifyBTagsWithSF(isBTagged,      jetBtagSFUp,      expEff);
	      //myBTagSFUtil.modifyBTagsWithSF(isBTagged,  jetBtagSFDown,  expEff);
	    }
	  if(debug) cout << "b-tagging DONE" << endl;

	  //save jet
          //Jet tmpj(jp4, csv, k);
          Jet tmpj(jp4, csv, k, ev.j_pt_charged[k], ev.j_pt_pf[k], ev.j_g[k]); //Store pt of charged and total PF tracks and gen matched index
	  for(int ipf = 0; ipf < ev.npf; ipf++) {
	    if(ev.pf_j[ipf] != k) continue; //skip if PF track doesn't belong to current jet
	    if(ev.pf_c[ipf]==0) continue;   //skip if PF track is neutral
	    TLorentzVector tkP4(0,0,0,0);
	    tkP4.SetPtEtaPhiM(ev.pf_pt[ipf],ev.pf_eta[ipf],ev.pf_phi[ipf],0.);
            pfTrack pftk(tkP4, ev.pf_dxy[ipf], ev.pf_dxyE[ipf], ev.pf_dz[ipf], ev.pf_dzE[ipf], ev.pf_id[ipf],ev.pf_quality[ipf],ev.pf_highPurity[ipf]);
            if(abs(pftk.getPdgId())==13) {
              pftk.setGlobalMuon(ev.pf_globalMuon[ipf]);
              pftk.setTrackerMuon(ev.pf_trackerMuon[ipf]);
            }
	    tmpj.addTrack(pftk); //,ev.pf_id[ipf]);
            /*
            if(jecUnc) {
              jecUnc->setJetEta(jp4.Eta());
              jecUnc->setJetPt(jp4.Pt());
              float jes = jecUnc->getUncertainty(true);
              cout << "jes= " << jes << endl;
            }
            */
	  }
          tmpj.sortTracksByPt();

          if(isBTagged) bJetsVec.push_back(tmpj);
          else lightJetsVec.push_back(tmpj);
          allJetsVec.push_back(tmpj);
	}

      stsum += htsum;

      
      //event weight
      float wgt(1.0);

      std::vector<float> puWgts(3,1.0),topPtWgts(2,1.0);
      EffCorrection_t lepSelCorrWgt(1.0,0.0), triggerCorrWgt(1.0,0.0);
      if(debug) cout << "Lepton scale factors" << endl;
      if(!ev.isData)
	{
          //float *puWgtsRun = new float[tmpRun.Length()];
	  //account for pu weights and effect on normalization
	  allPlots["puwgtctr"]->Fill(0.,1.0);
	  allPlots["puwgtgr"]->Fill(0.,1.0);
	  allPlots["puwgt"]->Fill(0.,1.0);
	  if(debug) cout << "getting puWgts" << endl;
          for(int xbin=1; xbin<=puWgt[0]->GetXaxis()->GetNbins(); xbin++) {
	    Double_t yobs;
	    //Double_t xobs,yobs;
	    yobs = puWgt[0]->GetBinContent(xbin);
	    //puWgtGr[0]->GetPoint(xbin-1,xobs,yobs);
            //allPlots["puwgtgr"]->SetBinContent(xbin,yobs);
            allPlots["puwgt"]->Fill(xbin,yobs);
          }
          //Set PU weight for each run period
          /*
          runB.SetPuWgt(puWgtsRun[0]->GetBinContent(ev.putrue));
          runC.SetPuWgt(puWgtsRun[1]->GetBinContent(ev.putrue));
          runD.SetPuWgt(puWgtsRun[2]->GetBinContent(ev.putrue));
          runE.SetPuWgt(puWgtsRun[3]->GetBinContent(ev.putrue));
          runF.SetPuWgt(puWgtsRun[4]->GetBinContent(ev.putrue));
          runG.SetPuWgt(puWgtsRun[5]->GetBinContent(ev.putrue));
          runH.SetPuWgt(puWgtsRun[6]->GetBinContent(ev.putrue));
          */
          runBCDEF.SetPuWgt(puWgtsRun[0]->GetBinContent(ev.putrue));
          runGH.SetPuWgt(puWgtsRun[1]->GetBinContent(ev.putrue));
          treeBCDEF.SetPuWgt(puWgtsRun[0]->GetBinContent(ev.putrue));
          treeGH.SetPuWgt(puWgtsRun[1]->GetBinContent(ev.putrue));

	    for(size_t iwgt=0; iwgt<3; iwgt++)
	      {
	        puWgts[iwgt]=puWgt[iwgt]->GetBinContent(ev.putrue);  
	        allPlots["puwgtctr"]->Fill(iwgt+1,puWgts[iwgt]);
	      }
	  if(debug) cout << "getting puWgts DONE!" << endl;
	  //trigger/id+iso efficiency corrections
          if(debug) cout << "calling trigger function" << endl;
          std::vector<int> pdgIds; //vector of IDs for trigger correction function
	  triggerCorrWgt=lepEffH.getTriggerCorrection(leptons);
          if(debug) cout << "calling trigger function DONE!" << endl;
          // ** loop over all Particles in leptons **
	  for(size_t il=0; il<leptons.size(); il++) {
	    EffCorrection_t selSF=lepEffH.getOfflineCorrection(leptons[il], ev.nvtx);
	    lepSelCorrWgt.second = sqrt( pow(lepSelCorrWgt.first*selSF.second,2)+pow(lepSelCorrWgt.second*selSF.first,2));
            if(debug) cout << "lepSelCorrWgt=" << lepSelCorrWgt.first << endl;
            if(debug) cout << "selSF=" << selSF.first << endl;
	    lepSelCorrWgt.first *= selSF.first;
	  }
	  wgt=triggerCorrWgt.first*lepSelCorrWgt.first*puWgts[0]*norm;

          // ** SFs for BCDEF and GH separately
          EffCorrection_t lepSelCorrWgt_BCDEF(1.0,0.0), triggerCorrWgt_BCDEF(1.0,0.0);
          EffCorrection_t lepSelCorrWgt_GH(1.0,0.0), triggerCorrWgt_GH(1.0,0.0);
	  triggerCorrWgt_BCDEF=lepEffH_BCDEF.getTriggerCorrection(leptons);
	  triggerCorrWgt_GH=lepEffH_GH.getTriggerCorrection(leptons);
	  for(size_t il=0; il<leptons.size(); il++) {
	    EffCorrection_t selSF_BCDEF=lepEffH_BCDEF.getOfflineCorrection(leptons[il], ev.nvtx);
	    //EffCorrection_t selSF_BCDEF=lepEffH_BCDEF.getOfflineCorrection(leptons[il],"BCDEF");
	    EffCorrection_t selSF_GH=lepEffH_GH.getOfflineCorrection(leptons[il], ev.nvtx);
	    //EffCorrection_t selSF_GH=lepEffH_GH.getOfflineCorrection(leptons[il],"GH");
	    lepSelCorrWgt_BCDEF.second = sqrt( pow(lepSelCorrWgt_BCDEF.first*selSF_BCDEF.second,2)+pow(lepSelCorrWgt_BCDEF.second*selSF_BCDEF.first,2));
	    lepSelCorrWgt_GH.second = sqrt( pow(lepSelCorrWgt_GH.first*selSF_GH.second,2)+pow(lepSelCorrWgt_GH.second*selSF_GH.first,2));
            if(debug) cout << "lepSelCorrWgt_BCDEF=" << lepSelCorrWgt_BCDEF.first << endl;
            if(debug) cout << "selSF=" << selSF_BCDEF.first << endl;
            if(debug) cout << "lepSelCorrWgt_GH=" << lepSelCorrWgt_GH.first << endl;
            if(debug) cout << "selSF=" << selSF_GH.first << endl;
	    lepSelCorrWgt_BCDEF.first *= selSF_BCDEF.first;
	    lepSelCorrWgt_GH.first *= selSF_GH.first;
	  }
          /*
          runB.SetSFs(triggerCorrWgt_BCDEF.first*lepSelCorrWgt_BCDEF.first);
          runC.SetSFs(triggerCorrWgt_BCDEF.first*lepSelCorrWgt_BCDEF.first);
          runD.SetSFs(triggerCorrWgt_BCDEF.first*lepSelCorrWgt_BCDEF.first);
          runE.SetSFs(triggerCorrWgt_BCDEF.first*lepSelCorrWgt_BCDEF.first);
          runF.SetSFs(triggerCorrWgt_BCDEF.first*lepSelCorrWgt_BCDEF.first);
          runG.SetSFs(triggerCorrWgt_GH.first*lepSelCorrWgt_GH.first);
          runH.SetSFs(triggerCorrWgt_GH.first*lepSelCorrWgt_GH.first);
          */
          runBCDEF.SetSFs(triggerCorrWgt_BCDEF.first*lepSelCorrWgt_BCDEF.first);
          runGH.SetSFs(triggerCorrWgt_GH.first*lepSelCorrWgt_GH.first);
          treeBCDEF.SetSFs(triggerCorrWgt_BCDEF.first*lepSelCorrWgt_BCDEF.first);
          treeGH.SetSFs(triggerCorrWgt_GH.first*lepSelCorrWgt_GH.first);
          // **

          if(debug) cout << "weight=" << wgt << endl;
          if(debug) cout << "Trigger=" << triggerCorrWgt.first << endl << "Lepton=" << lepSelCorrWgt.first << endl << "PU=" << puWgts[0] << endl << "norm=" << norm  << endl;
          //wgt=1.0;
	}
      else
        if(debug) cout << "weight=" << wgt << " norm=" << norm << endl;
      if(debug) cout << "Lepton scale factors DONE!" << endl;

      //sort by Pt
      if(debug) cout << "sorting jets" << endl;
      sort(lightJetsVec.begin(),    lightJetsVec.end(),   sortJetsByPt);
      sort(bJetsVec.begin(),    bJetsVec.end(),   sortJetsByPt);
      sort(allJetsVec.begin(),  allJetsVec.end(), sortJetsByPt);

      for(size_t ij = 0; ij < allJetsVec.size(); ij++) {
        float csv = allJetsVec.at(ij).getCSV();
        allPlots["csv"+chTag]->Fill(csv,wgt);
        allPlots["csv_all"]->Fill(csv,wgt);
      }

      for(size_t il=0; il<leptons.size(); il++) {
        for(size_t ij=0; ij<allJetsVec.size(); ij++) {
	  TLorentzVector jp4=allJetsVec[ij].getVec();
          allPlots["dR"+chTag]->Fill(jp4.DeltaR(leptons[il].getVec()),wgt);
          allPlots["dR"+chTag+"_no_weight"]->Fill(jp4.DeltaR(leptons[il].getVec()),norm);
        }
      }


      //MET and transverse mass
      TLorentzVector met(0,0,0,0);
      met.SetPtEtaPhiM(ev.met_pt[0],0,ev.met_phi[0],0.);
      met+=jetDiff;
      met.SetPz(0.); met.SetE(met.Pt());
      //float mt( computeMT(isZ ? dilp4: lp4,met) );

      //simple fill
      bool singleLep(false);
      bool doubleLep(false);
      bool minJets(false);
      //kalman.loadEvent(ev.event);

      if(debug) cout << "starting simple plots" << endl;
      allPlots["nevt"+chTag]->Fill(1,norm);
      allPlots["weight"+chTag]->Fill(wgt,norm);
      allPlots["norm"+chTag]->Fill(norm,norm);
      allPlots["nvtx"+chTag]->Fill(ev.nvtx,wgt);
      /*
      runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);
      runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);
      runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);
      runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);
      runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);
      runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);
      runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);
      */
      runBCDEF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);
      runGH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag);

      /*
      runB.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);
      runC.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);
      runD.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);
      runE.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);
      runF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);
      runG.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);
      runH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);
      */
      runBCDEF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);
      runGH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag);

      /*
      runB.Fill(leptons, chTag);
      runC.Fill(leptons, chTag);
      runD.Fill(leptons, chTag);
      runE.Fill(leptons, chTag);
      runF.Fill(leptons, chTag);
      runG.Fill(leptons, chTag);
      runH.Fill(leptons, chTag);
      */
      runBCDEF.Fill(leptons, chTag);
      runGH.Fill(leptons, chTag);

      allPlots["nj"+chTag]->Fill(allJetsVec.size(),wgt);
      allPlots["nlj"+chTag]->Fill(lightJetsVec.size(),wgt);
      allPlots["nbj"+chTag]->Fill(bJetsVec.size(),wgt);
      allPlots["nlj"+chTag+"_no_weight"]->Fill(lightJetsVec.size(),norm);
      allPlots["nbj"+chTag+"_no_weight"]->Fill(bJetsVec.size(),norm);
      allPlots["nlj_all"]->Fill(lightJetsVec.size(),wgt);
      allPlots["nbj_all"]->Fill(bJetsVec.size(),wgt);
      allPlots["nlp"+chTag]->Fill(leptons.size(),wgt);
      allPlots["nlp"+chTag+"_no_weight"]->Fill(leptons.size(),norm);
      allPlots["HT"+chTag]->Fill(htsum,wgt);
      allPlots["ST"+chTag]->Fill(stsum,wgt);
      allPlots["MET2oST"+chTag]->Fill(pow(ev.met_pt[0],2)/stsum,wgt);

      if(leptons.size() > 0) {
        allPlots["lp_pt_low"+chTag]->Fill(leptons[0].Pt(),wgt);
        allPlots["lp_pt"+chTag]->Fill(leptons[0].Pt(),wgt);
        allPlots["lp_pt"+chTag+"_no_weight"]->Fill(leptons[0].Pt(),norm);
        allPlots["lp_pt_all"]->Fill(leptons[0].Pt(),wgt);
        allPlots["relIso"+chTag]->Fill(leptons[0].getRelIso(),wgt);
        allPlots["lp_eta"+chTag]->Fill(leptons[0].Eta(),wgt);
        allPlots["lp_eta"+chTag+"_no_weight"]->Fill(leptons[0].Eta(),norm);
        allPlots["lp_phi"+chTag]->Fill(leptons[0].Phi(),wgt);
        allPlots["lp_phi"+chTag+"_no_weight"]->Fill(leptons[0].Phi(),norm);
      }

      if(isZ) {
        allPlots["massZ"+chTag]->Fill(dilp4.M(),wgt);
        allPlots["massZ"+chTag+"_no_weight"]->Fill(dilp4.M(),norm);
      }

      if(allJetsVec.size() > 0) {
        allPlots["j_pt"+chTag]->Fill(allJetsVec[0].getVec().Pt(),wgt);
        allPlots["j_pt_low"+chTag]->Fill(allJetsVec[0].getVec().Pt(),wgt);
        allPlots["j_pt"+chTag+"_no_weight"]->Fill(allJetsVec[0].getVec().Pt(),norm);
        allPlots["j_pt_all"]->Fill(allJetsVec[0].getVec().Pt(),wgt);
      }
      if(lightJetsVec.size() > 0) {
        allPlots["lj_pt_low"+chTag]->Fill(lightJetsVec[0].getVec().Pt(),wgt);
        allPlots["lj_pt"+chTag]->Fill(lightJetsVec[0].getVec().Pt(),wgt);
        allPlots["lj_pt"+chTag+"_no_weight"]->Fill(lightJetsVec[0].getVec().Pt(),norm);
        allPlots["lj_pt_all"]->Fill(lightJetsVec[0].getVec().Pt(),wgt);
      }
      if(bJetsVec.size() > 0) {
        allPlots["bj_pt_low"+chTag]->Fill(bJetsVec[0].getVec().Pt(),wgt);
        allPlots["bj_pt"+chTag]->Fill(bJetsVec[0].getVec().Pt(),wgt);
        allPlots["bj_pt"+chTag+"_no_weight"]->Fill(bJetsVec[0].getVec().Pt(),norm);
        allPlots["bj_pt_all"]->Fill(bJetsVec[0].getVec().Pt(),wgt);
      }

      if(debug) cout << "starting lep/jets plots" << endl;
      //Require exactly 1 lepton
      if(leptons.size() == 1) {
        if(debug) cout << "single lepton" << endl;
        singleLep = true;
        /*
        runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
        runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
        runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
        runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
        runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
        runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
        runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");

        runB.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
        runC.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
        runD.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
        runE.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
        runF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
        runG.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
        runH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
        
        runB.Fill(leptons, chTag, "lep");
        runC.Fill(leptons, chTag, "lep");
        runD.Fill(leptons, chTag, "lep");
        runE.Fill(leptons, chTag, "lep");
        runF.Fill(leptons, chTag, "lep");
        runG.Fill(leptons, chTag, "lep");
        runH.Fill(leptons, chTag, "lep");
        */

        allPlots["lp_pt"+chTag+"_lep"]->Fill(leptons[0].Pt(),wgt);
        allPlots["lp_pt"+chTag+"_lep_no_weight"]->Fill(leptons[0].Pt(),norm);

        allPlots["lp_pt_all_lep"]->Fill(leptons[0].Pt(),wgt);
        allPlots["lp_eta"+chTag+"_lep"]->Fill(leptons[0].Eta(),wgt);
        allPlots["lp_eta"+chTag+"_lep"+"_no_weight"]->Fill(leptons[0].Eta(),norm);
        allPlots["lp_phi"+chTag+"_lep"]->Fill(leptons[0].Phi(),wgt);
        allPlots["lp_phi"+chTag+"_lep"+"_no_weight"]->Fill(leptons[0].Phi(),norm);
        allPlots["MET"+chTag+"_lep"]->Fill(ev.met_pt[0],wgt);
        allPlots["HT"+chTag+"_lep"]->Fill(htsum,wgt);
        allPlots["ST"+chTag+"_lep"]->Fill(stsum,wgt);
        allPlots["MET2oST"+chTag+"_lep"]->Fill(pow(ev.met_pt[0],2)/stsum,wgt);
        allPlots["MET"+chTag+"_lep_no_weight"]->Fill(ev.met_pt[0],norm);
        allPlots["relIso"+chTag+"_lep"]->Fill(leptons[0].getRelIso(),wgt);
        //Require at least 1 b-tagged and at least 2 light jets
        if(bJetsVec.size() >= 1 && lightJetsVec.size() >= 3) {
          if(debug) cout << "jet reqirements" << endl;
          minJets = true;
          /*
          runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          
          runB.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runC.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runD.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runE.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runG.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          
          runB.Fill(leptons, chTag, "lepjets");
          runC.Fill(leptons, chTag, "lepjets");
          runD.Fill(leptons, chTag, "lepjets");
          runE.Fill(leptons, chTag, "lepjets");
          runF.Fill(leptons, chTag, "lepjets");
          runG.Fill(leptons, chTag, "lepjets");
          runH.Fill(leptons, chTag, "lepjets");
          */

          allPlots["lp_pt"+chTag+"_lepjets"]->Fill(leptons[0].Pt(),wgt);
          //allPlots["lp_pt"+chTag+"_lepjets_no_weight"]->Fill(leptons[0].Pt(),norm);

          allPlots["lp_pt_all_lepjets"]->Fill(leptons[0].Pt(),wgt);
          allPlots["lp_eta"+chTag+"_lepjets"]->Fill(leptons[0].Eta(),wgt);
          allPlots["lp_eta"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[0].Eta(),norm);
          allPlots["lp_phi"+chTag+"_lepjets"]->Fill(leptons[0].Phi(),wgt);
          allPlots["lp_phi"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[0].Phi(),norm);
          allPlots["MET"+chTag+"_lepjets"]->Fill(ev.met_pt[0],wgt);
          allPlots["HT"+chTag+"_lepjets"]->Fill(htsum,wgt);
          allPlots["ST"+chTag+"_lepjets"]->Fill(stsum,wgt);
          allPlots["MET2oST"+chTag+"_lepjets"]->Fill(pow(ev.met_pt[0],2)/stsum,wgt);
          allPlots["relIso"+chTag+"_lepjets"]->Fill(leptons[0].getRelIso(),wgt);
          //allPlots["MET"+chTag+"_lepjets__no_weight"]->Fill(ev.met_pt[0],norm);
        }
      }
      //Require exactly 2 leptons
      else if(leptons.size() == 2) {
        if(debug) cout << "dilepton" << endl;
        doubleLep = true;
        //Z control plot
        if(isZ) {
          allPlots["massZ"+chTag+"_lep"]->Fill(dilp4.M(),wgt);
          allPlots["massZ"+chTag+"_lep_no_weight"]->Fill(dilp4.M(),norm);
        }
        if(abs(dilp4.M()-91)<15)
          allPlots["chargeZ"+chTag]->Fill(leptons[0].charge()*leptons[1].charge(),wgt); 
        //Exclude Z and low mass and require same falvor dilepton MET > 40 GeV
        if(!isZ && (dilp4.M() > 20 && leptons[0].getPdgId()==-leptons[1].getPdgId()) &&
          ((abs(leptons[0].getPdgId())!=abs(leptons[1].getPdgId())) || (leptons[0].getPdgId()==-leptons[1].getPdgId() && met.Pt() > 40))) {
          /*
          runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
          runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
          runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
          runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
          runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
          runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
          runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
          
          runB.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
          runC.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
          runD.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
          runE.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
          runF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
          runG.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
          runH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");

          
          runB.Fill(leptons, chTag, "lep");
          runC.Fill(leptons, chTag, "lep");
          runD.Fill(leptons, chTag, "lep");
          runE.Fill(leptons, chTag, "lep");
          runF.Fill(leptons, chTag, "lep");
          runG.Fill(leptons, chTag, "lep");
          runH.Fill(leptons, chTag, "lep");
          */

          allPlots["ndilp"+chTag+"_lep"]->Fill(leptons.size(),wgt);
          allPlots["dilp_pt"+chTag+"_lep"]->Fill(dilp4.Pt(),wgt);
          allPlots["dilp_m"+chTag+"_lep"]->Fill(dilp4.M(),wgt);
          allPlots["ndilp"+chTag+"_lep"+"_no_weight"]->Fill(leptons.size(),norm);
          allPlots["dilp_pt"+chTag+"_lep"+"_no_weight"]->Fill(dilp4.Pt(),norm);
          allPlots["dilp_m"+chTag+"_lep"+"_no_weight"]->Fill(dilp4.M(),norm);
          allPlots["lp_pt"+chTag+"_lep"]->Fill(leptons[0].Pt(),wgt);
          allPlots["l2p_pt"+chTag+"_lep"]->Fill(leptons[1].Pt(),wgt);
          allPlots["lp_pt"+chTag+"_lep"+"_no_weight"]->Fill(leptons[0].Pt(),norm);
          allPlots["l2p_pt"+chTag+"_lep"+"_no_weight"]->Fill(leptons[1].Pt(),norm);
          allPlots["MET"+chTag+"_lep"]->Fill(ev.met_pt[0],wgt);
          allPlots["charge"+chTag+"_lep"]->Fill(leptons[0].charge()*leptons[1].charge(),wgt);
          allPlots["MET"+chTag+"_lep"+"_no_weight"]->Fill(ev.met_pt[0],norm);
          allPlots["charge"+chTag+"_lep"+"_no_weight"]->Fill(leptons[0].charge()*leptons[1].charge(),norm);
          allPlots["relIso"+chTag+"_lep"]->Fill(leptons[0].getRelIso(),wgt);
          allPlots["relIso"+chTag+"_lep"]->Fill(leptons[1].getRelIso(),wgt);
        }
        //Require at least 1 b-tagged and at least 1 light jets
        if(bJetsVec.size() >= 1 && lightJetsVec.size() >= 1) {
          if(debug) cout << "jet reqirements" << endl;
          //Z control plot
          minJets = true;
          if(isZ) {
            allPlots["massZ"+chTag+"_lepjets"]->Fill(dilp4.M(),wgt);
            allPlots["massZ"+chTag+"_lepjets_no_weight"]->Fill(dilp4.M(),norm);
          }
          //Exclude Z mass
          if(isZ) minJets = false;
          //if(abs(dilp4.M()-91)<15 && abs(leptons[0].getPdgId())==abs(leptons[1].getPdgId()) && leptons[0].charge()!=leptons[1].charge()) minJets = false;
          //Exclude low mass (M < 20 GeV)
          if(dilp4.M() < 20 && leptons[0].getPdgId()==-leptons[1].getPdgId()) minJets = false;
          //Require same falvor dilepton MET > 40 GeV
          if(leptons[0].getPdgId()==-leptons[1].getPdgId() && met.Pt() < 40) minJets = false;
          /*
          runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
          
          runB.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runC.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runD.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runE.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runG.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          runH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
          
          runB.Fill(leptons, chTag, "lepjets");
          runC.Fill(leptons, chTag, "lepjets");
          runD.Fill(leptons, chTag, "lepjets");
          runE.Fill(leptons, chTag, "lepjets");
          runF.Fill(leptons, chTag, "lepjets");
          runG.Fill(leptons, chTag, "lepjets");
          runH.Fill(leptons, chTag, "lepjets");
          */

          if(minJets) {
            allPlots["ndilp"+chTag+"_lepjets"]->Fill(leptons.size(),wgt);
            allPlots["dilp_pt"+chTag+"_lepjets"]->Fill(dilp4.Pt(),wgt);
            allPlots["dilp_m"+chTag+"_lepjets"]->Fill(dilp4.M(),wgt);
            allPlots["ndilp"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons.size(),norm);
            allPlots["dilp_pt"+chTag+"_lepjets"+"_no_weight"]->Fill(dilp4.Pt(),norm);
            allPlots["dilp_m"+chTag+"_lepjets"+"_no_weight"]->Fill(dilp4.M(),norm);
            allPlots["lp_pt"+chTag+"_lepjets"]->Fill(leptons[0].Pt(),wgt);
            allPlots["l2p_pt"+chTag+"_lepjets"]->Fill(leptons[1].Pt(),wgt);
            allPlots["lp_pt"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[0].Pt(),norm);
            allPlots["l2p_pt"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[1].Pt(),norm);
            allPlots["lp_eta"+chTag+"_lepjets"]->Fill(leptons[0].Eta(),wgt);
            allPlots["l2p_eta"+chTag+"_lepjets"]->Fill(leptons[1].Eta(),wgt);
            allPlots["lp_eta"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[0].Eta(),norm);
            allPlots["l2p_eta"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[1].Eta(),norm);
            allPlots["lp_phi"+chTag+"_lepjets"]->Fill(leptons[0].Phi(),wgt);
            allPlots["l2p_phi"+chTag+"_lepjets"]->Fill(leptons[1].Phi(),wgt);
            allPlots["lp_phi"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[0].Phi(),norm);
            allPlots["l2p_phi"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[1].Phi(),norm);
            allPlots["MET"+chTag+"_lepjets"]->Fill(ev.met_pt[0],wgt);
            allPlots["charge"+chTag+"_lepjets"]->Fill(leptons[0].charge()*leptons[1].charge(),wgt);
            allPlots["MET"+chTag+"_lepjets"+"_no_weight"]->Fill(ev.met_pt[0],norm);
            allPlots["charge"+chTag+"_lepjets"+"_no_weight"]->Fill(leptons[0].charge()*leptons[1].charge(),norm);
            allPlots["relIso"+chTag+"_lepjets"]->Fill(leptons[0].getRelIso(),wgt);
            allPlots["relIso"+chTag+"_lepjets"]->Fill(leptons[1].getRelIso(),wgt);
          }
        }
      }
      if(debug) cout << "simple plots DONE" << endl;


      //Require lep+jets or dilepton
      if(!singleLep && !doubleLep) continue;
      if(debug) cout << "passed lep requirements" << endl;

      /*
      runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
      runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
      runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
      runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
      runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
      runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
      runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
      */
      runBCDEF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");
      runGH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lep");

      /*
      runB.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      runC.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      runD.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      runE.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      runF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      runG.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      runH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      */
      runBCDEF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      runGH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lep");
      
      /*
      runB.Fill(leptons, chTag, "lep");
      runC.Fill(leptons, chTag, "lep");
      runD.Fill(leptons, chTag, "lep");
      runE.Fill(leptons, chTag, "lep");
      runF.Fill(leptons, chTag, "lep");
      runG.Fill(leptons, chTag, "lep");
      runH.Fill(leptons, chTag, "lep");
      */
      runBCDEF.Fill(leptons, chTag, "lep");
      runGH.Fill(leptons, chTag, "lep");

      allPlots["npf"+chTag+"_lep"]->Fill(ev.npf,wgt);
      //allPlots["npf"+chTag+"_lep"+"_no_weight"]->Fill(ev.npf,norm);
      allPlots["nevt"+chTag+"_lep"]->Fill(1,norm);
      allPlots["weight"+chTag+"_lep"]->Fill(wgt,norm);
      allPlots["norm"+chTag+"_lep"]->Fill(norm,norm);
      allPlots["nvtx"+chTag+"_lep"]->Fill(ev.nvtx,wgt);
      allPlots["nevt_all_lep"]->Fill(1,norm);

      allPlots["nj"+chTag+"_lep"]->Fill(allJetsVec.size(),wgt);
      allPlots["nlj"+chTag+"_lep"]->Fill(lightJetsVec.size(),wgt);
      allPlots["nbj"+chTag+"_lep"]->Fill(bJetsVec.size(),wgt);
      //allPlots["nj"+chTag+"_lep"+"_no_weight"]->Fill(lightJetsVec.size(),norm);
      //allPlots["nbj"+chTag+"_lep"+"_no_weight"]->Fill(bJetsVec.size(),norm);
      allPlots["nlp"+chTag+"_lep"]->Fill(leptons.size(),wgt);

      if(lightJetsVec.size() > 0 and bJetsVec.size() > 0) {
        allPlots["j_pt"+chTag+"_lep"]->Fill(allJetsVec[0].getVec().Pt(),wgt);
        allPlots["lj_pt"+chTag+"_lep"]->Fill(lightJetsVec[0].getVec().Pt(),wgt);
        allPlots["bj_pt"+chTag+"_lep"]->Fill(bJetsVec[0].getVec().Pt(),wgt);
      }

      for(size_t ij = 0; ij < allJetsVec.size(); ij++) {
        float csv = allJetsVec.at(ij).getCSV();
        allPlots["csv"+chTag+"_lep"]->Fill(csv,wgt);
        allPlots["csv_all_lep"]->Fill(csv,wgt);
      //}
      }

      //Require b-tagged and light jets
      //if(!minJets || !kalman.isGoodEvent()) continue;
      if(!minJets) continue;
      if(debug) cout << "passed jet requirements" << endl;

      //Fill gen-level top plots
      if(isTTbar) {
        runBCDEF.FillGen(tops, chTag, "lepjets");
        runGH.FillGen(tops, chTag, "lepjets");
      }

      /*
      runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      */
      runBCDEF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      runGH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "lepjets");
      
      /*
      runB.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      runC.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      runD.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      runE.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      runF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      runG.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      runH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      */
      runBCDEF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      runGH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "lepjets");
      
      /*
      runB.Fill(leptons, chTag, "lepjets");
      runC.Fill(leptons, chTag, "lepjets");
      runD.Fill(leptons, chTag, "lepjets");
      runE.Fill(leptons, chTag, "lepjets");
      runF.Fill(leptons, chTag, "lepjets");
      runG.Fill(leptons, chTag, "lepjets");
      runH.Fill(leptons, chTag, "lepjets");
      */
      runBCDEF.Fill(leptons, chTag, "lepjets");
      runGH.Fill(leptons, chTag, "lepjets");

      allPlots["npf"+chTag+"_lepjets"]->Fill(ev.npf,wgt);
      allPlots["npf"+chTag+"_lepjets"+"_no_weight"]->Fill(ev.npf,norm);
      allPlots["nevt"+chTag+"_lepjets"]->Fill(1,norm);
      allPlots["weight"+chTag+"_lepjets"]->Fill(wgt,norm);
      allPlots["norm"+chTag+"_lepjets"]->Fill(norm,norm);
      allPlots["nvtx"+chTag+"_lepjets"]->Fill(ev.nvtx,wgt);
      allPlots["nevt_all_lepjets"]->Fill(1,norm);

      allPlots["nj"+chTag+"_lepjets"]->Fill(allJetsVec.size(),wgt);
      allPlots["nlj"+chTag+"_lepjets"]->Fill(lightJetsVec.size(),wgt);
      allPlots["nbj"+chTag+"_lepjets"]->Fill(bJetsVec.size(),wgt);
      allPlots["nlp"+chTag+"_lepjets"]->Fill(leptons.size(),wgt);

      allPlots["j_pt"+chTag+"_lepjets"]->Fill(allJetsVec[0].getVec().Pt(),wgt);
      allPlots["lj_pt"+chTag+"_lepjets"]->Fill(lightJetsVec[0].getVec().Pt(),wgt);
      allPlots["bj_pt"+chTag+"_lepjets"]->Fill(bJetsVec[0].getVec().Pt(),wgt);

      for(size_t ij = 0; ij < allJetsVec.size(); ij++) {
        float csv = allJetsVec.at(ij).getCSV();
        allPlots["csv"+chTag+"_lepjets"]->Fill(csv,wgt);
        allPlots["csv_all_lepjets"]->Fill(csv,wgt);
      }

      //charmed resonance analysis : use only jets with CSV>CSVL, up to two per event
      for(size_t ij = 0; ij < bJetsVec.size(); ij++) {

        if(ij > 1) continue;
        if(ij == 0) {
          /*
          runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");
          runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");
          runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");
          runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");
          runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");
          runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");
          runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");
          */
          runBCDEF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");
          runGH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "csv");

          /*
          runB.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          runC.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          runD.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          runE.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          runF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          runG.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          runH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          */
          runBCDEF.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          runGH.Fill(lightJetsVec,bJetsVec,allJetsVec, chTag, "csv");
          
          /*
          runB.Fill(leptons, chTag, "csv");
          runC.Fill(leptons, chTag, "csv");
          runD.Fill(leptons, chTag, "csv");
          runE.Fill(leptons, chTag, "csv");
          runF.Fill(leptons, chTag, "csv");
          runG.Fill(leptons, chTag, "csv");
          runH.Fill(leptons, chTag, "csv");
          */
          runBCDEF.Fill(leptons, chTag, "csv");
          runGH.Fill(leptons, chTag, "csv");

          //allPlots["nbj"+chTag+"_csv"]->Fill(1,wgt);
          allPlots["nevt"+chTag+"_csv"]->Fill(1,norm);
          allPlots["weight"+chTag+"_csv"]->Fill(wgt,norm);
          allPlots["norm"+chTag+"_csv"]->Fill(norm,norm);
          allPlots["nvtx"+chTag+"_csv"]->Fill(ev.nvtx,wgt);
          allPlots["j_pt"+chTag+"_csv"]->Fill(allJetsVec[0].getVec().Pt(),wgt);
          allPlots["lj_pt"+chTag+"_csv"]->Fill(lightJetsVec[0].getVec().Pt(),wgt);
          allPlots["bj_pt"+chTag+"_csv"]->Fill(bJetsVec[0].getVec().Pt(),wgt);
        }

        allPlots["csv"+chTag+"_csv"]->Fill(bJetsVec[ij].getCSV(),wgt);
        std::vector<pfTrack> &tracks = bJetsVec[ij].getTracks();

        //J/Psi

        //if(!kalman.isJPsiEvent()) continue;
        evch.njpsi=0;
        evch.nj=0;
        if(debug) cout << "starting J/Psi" << endl;
        const float gMassMu(0.1057),gMassK(0.4937),gMassPi(0.1396);
        std::vector<pfTrack> pfmuCands,kaonCands;
        for(size_t itk = 0; itk < tracks.size(); itk++) {
          if(!tracks[itk].highPurity()) continue; //only use high purity tracks
          if(abs(tracks[itk].getPdgId()) == 13) {
            /*
            TLorentzVector muP4;
            muP4.SetPtEtaPhiM(tracks[itk].Pt(), tracks[itk].Eta(), tracks[itk].Phi(), gMassMu);
            pfTrack pfmu(muP4, tracks[itk].getDxy(), tracks[itk].getDxyE(), tracks[itk].getDz(), tracks[itk].getDzE(), tracks[itk].getPdgId());
            pfmuCands.push_back(pfmu);
            */
            if(!tracks[itk].globalMuon() && !tracks[itk].trackerMuon()) continue;
            if(tracks[itk].Pt() < 3.0) continue;
            tracks[itk].setMass(gMassMu);
            pfmuCands.push_back(tracks[itk]);
          }
          if(abs(tracks[itk].getPdgId()) == 211) {
            /*
            TLorentzVector kP4;
            kP4.SetPtEtaPhiM(tracks[itk].Pt(), tracks[itk].Eta(), tracks[itk].Phi(), gMassK);
            pfTrack pfk(kP4, tracks[itk].getDxy(), tracks[itk].getDxyE(), tracks[itk].getDz(), tracks[itk].getDzE(), tracks[itk].getPdgId());
            kaonCands.push_back(pfk);
            */
            kaonCands.push_back(tracks[itk]);
          }
        }
    
        if(pfmuCands.size()>1 && (pfmuCands[0].getPfid() == -pfmuCands[1].getPfid())) {

          std::vector<pfTrack> pfmuMatched, pfmuReject;
          //Gen-matching
          if(!ev.isData) {
            //Don't use as it would skip non-gen events
            //if(!ev.ngmeson) continue; //event has no mesons
            bool isJPsiEvent(false);
            //if(ev.ngmeson_daug<2) continue; //require at least 2 daughters (mu^+ and mu^-)
            for(int ij = 0; ij < ev.ngmeson; ij++) {
              if(abs(ev.gmeson_id[ij])==443) { isJPsiEvent = true; continue; } //loop until JPsi found
            }
            //if(!isJPsiEvent) continue; //event doesn't have a JPsi

            std::vector<pfTrack> genTracks;
            std::vector<pfTrack> genMuTracks;
            for(int ig = 0; ig < ev.ngmeson_daug; ig++) {
              if(!isJPsiEvent) break;
              //if(abs(ev.gmeson_id[ig])!=443) continue; //JPsi only
              TLorentzVector gen;
              gen.SetPtEtaPhiM(ev.gmeson_daug_pt[ig], ev.gmeson_daug_eta[ig], ev.gmeson_daug_phi[ig], gMassMu);
              genTracks.push_back(pfTrack(gen,0,0,0,0,ev.gmeson_daug_id[ig],3,true));
              int mother = ev.gmeson_daug_meson_index[ig]; //daug -> mother id -> mother ttbar
              genTracks.back().setGenT(ev.gmeson_mother_id[mother]); //daug -> mother id -> mother ttbar
              //cout << genTracks.back().getGenT() << endl;
              if(abs(ev.gmeson_daug_id[ig])==13) genMuTracks.push_back(pfTrack(gen,0,0,0,0, ev.gmeson_daug_id[ig],3,true));
              if(abs(ev.gmeson_daug_id[ig])==13) genMuTracks.back().setGenT(ev.gmeson_mother_id[mother]); //daug -> mother id -> mother ttbar
              //cout << genMuTracks.back().getGenT() << endl;
            }

            for(auto & it : pfmuCands) { //FIXME reference might not work
              double dR = 0.3; //initial dR
              int best_idx = -1;
              for(auto & itg : genMuTracks) {
                if(it.getPdgId() != itg.getPdgId()) continue; //insure ID and charge
                if(it.getVec().DeltaR(itg.getVec())>dR) continue; //find dR
                if(((it.Pt()-itg.Pt())/it.Pt())>0.10) continue; //gen and reco less than 10% difference
                dR = it.getVec().DeltaR(itg.getVec());
                best_idx = &itg - &genMuTracks[0]; //get index on current closest gen particle
              }
              if(best_idx<0) { //no gen track matched
                pfmuReject.push_back(it);
              }
              else {
                //genMuTracks.erase(std::remove(genMuTracks.begin(), genMuTracks.end(), best_idx), genMuTracks.end()); //remove gen track so it cannot be matched again!
                genMuTracks.erase(genMuTracks.begin() + best_idx); //remove gen track so it cannot be matched again!
                it.setGenT(genMuTracks[best_idx].getGenT());
                pfmuMatched.push_back(it);
                //pfmuMatched.back().setGenT(genMuTracks[best_idx].getGenT());
              }
            }
            //pfmuCands = pfmuMatched; //overwrite original vector with matched vector so I don't have to change the rest of the code
          }

          float mass12((pfmuCands[0].getVec() + pfmuCands[1].getVec()).M());
          float mass123( kaonCands.size()>0 ? (pfmuCands[0].getVec()+pfmuCands[1].getVec()+kaonCands[0].getVec()).M() : -1);
          allPlots["nbj"+chTag+"_jpsi"]->Fill(1,wgt);
          allPlots["bj_pt"+chTag+"_jpsi"]->Fill(bJetsVec[ij].getVec().Pt(),wgt);
          allPlots["lp_pt"+chTag+"_jpsi"]->Fill(leptons[0].Pt(),wgt);
          allPlots["lp_pt_low"+chTag+"_jpsi"]->Fill(leptons[0].Pt(),wgt);
          allPlots["lp_eta"+chTag+"_jpsi"]->Fill(leptons[0].Eta(),wgt);
          allPlots["lp_eta"+chTag+"_jpsi"+"_no_weight"]->Fill(leptons[0].Eta(),norm);
          allPlots["lp_phi"+chTag+"_jpsi"]->Fill(leptons[0].Phi(),wgt);
          allPlots["lp_phi"+chTag+"_jpsi"+"_no_weight"]->Fill(leptons[0].Phi(),norm);
          allPlots["csv"+chTag+"_jpsi"]->Fill(bJetsVec[ij].getCSV(),wgt);
          allPlots["nevt"+chTag+"_jpsi"]->Fill(1,norm);
          allPlots["weight"+chTag+"_jpsi"]->Fill(wgt,norm);
          allPlots["norm"+chTag+"_jpsi"]->Fill(norm,norm);
          allPlots["nvtx"+chTag+"_jpsi"]->Fill(ev.nvtx,wgt);

          if(mass12>2.5 && mass12<3.4) {
            if(debug) cout << pfmuCands[0].Pt() << " " << pfmuCands[0].Eta() << " " << pfmuCands[0].Phi() << " " << gMassMu << endl;
            if(debug) cout << pfmuCands[1].Pt() << " " << pfmuCands[1].Eta() << " " << pfmuCands[1].Phi() << " " << gMassMu << endl;
            if(debug) cout << mass12 << endl << endl;
            allPlots["massJPsi"+chTag]->Fill(mass12,wgt);
  	    allPlots["massJPsi_all"]->Fill(mass12,wgt);

            /*
            runB.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            runC.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            runD.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            runE.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            runF.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            runG.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            runH.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            */
            runBCDEF.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            runGH.Fill(pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");

            //treeBCDEF.Fill(evch, pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi");
            //treeGH.Fill(evch, pfmuCands, leptons, bJetsVec[ij], chTag, "jpsi", ev.event);

            if(!ev.isData && pfmuMatched.size() > 1) { //save gen-matched J/Psi
              runBCDEF.Fill(pfmuMatched, leptons, bJetsVec[ij], chTag, "gjpsi");
              runGH.Fill(pfmuMatched, leptons, bJetsVec[ij], chTag, "gjpsi");
            }
            if(!ev.isData && pfmuReject.size() > 1) { //save gen-unmatched J/Psi
              runBCDEF.Fill(pfmuReject, leptons, bJetsVec[ij], chTag, "rgjpsi");
              runGH.Fill(pfmuReject, leptons, bJetsVec[ij], chTag, "rgjpsi");
            }

            /*
            runB.Fill(pfmuCands, leptons, chTag, "jpsi");
            runC.Fill(pfmuCands, leptons, chTag, "jpsi");
            runD.Fill(pfmuCands, leptons, chTag, "jpsi");
            runE.Fill(pfmuCands, leptons, chTag, "jpsi");
            runF.Fill(pfmuCands, leptons, chTag, "jpsi");
            runG.Fill(pfmuCands, leptons, chTag, "jpsi");
            runH.Fill(pfmuCands, leptons, chTag, "jpsi");

            runB.Fill(pfmuCands, bJetsVec[ij], chTag, "jpsi");
            runC.Fill(pfmuCands, bJetsVec[ij], chTag, "jpsi");
            runD.Fill(pfmuCands, bJetsVec[ij], chTag, "jpsi");
            runE.Fill(pfmuCands, bJetsVec[ij], chTag, "jpsi");
            runF.Fill(pfmuCands, bJetsVec[ij], chTag, "jpsi");
            runG.Fill(pfmuCands, bJetsVec[ij], chTag, "jpsi");
            runH.Fill(pfmuCands, bJetsVec[ij], chTag, "jpsi");
            */

            /*
            runB.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");
            runC.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");
            runD.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");
            runE.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");
            runF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");
            runG.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");
            runH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");
            */
            runBCDEF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");
            runGH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "jpsi");

            for(int itk = 0; itk < 2; itk++) {

              for(int i = 0; i < 2; i++) {
                allPlots["pf_dxy"+chTag+"_jpsi"]->Fill(abs(pfmuCands[i].getDxy()),wgt);
                allPlots["pf_dz"+chTag+"_jpsi"]->Fill(abs(pfmuCands[i].getDz()),wgt);
                allPlots["pf_dxyE"+chTag+"_jpsi"]->Fill(abs(pfmuCands[i].getDxyE()),wgt);
                allPlots["pf_dzE"+chTag+"_jpsi"]->Fill(abs(pfmuCands[i].getDzE()),wgt);
                allPlots["pf_dz_sig"+chTag+"_jpsi"]->Fill(abs(pfmuCands[i].getDz())/abs(pfmuCands[i].getDzE()),wgt);
                allPlots["pf_dxy_sig"+chTag+"_jpsi"]->Fill(abs(pfmuCands[i].getDxy())/abs(pfmuCands[i].getDxyE()),wgt);
                allPlots["pf_dz_sig"+chTag+"_jpsi"]->Fill(abs(pfmuCands[i].getDz())/abs(pfmuCands[i].getDzE()),wgt);
                allPlots["pf_dxy_all"]->Fill(abs(pfmuCands[i].getDxy()),wgt);
                allPlots["pf_dz_all"]->Fill(abs(pfmuCands[i].getDz()),wgt);
                allPlots["pf_dxyE_all"]->Fill(abs(pfmuCands[i].getDxyE()),wgt);
                allPlots["pf_dzE_all"]->Fill(abs(pfmuCands[i].getDzE()),wgt);
                allPlots["pf_dz_sig_all"]->Fill(abs(pfmuCands[i].getDz())/abs(pfmuCands[i].getDzE()),wgt);
                allPlots["pf_dxy_sig_all"]->Fill(abs(pfmuCands[i].getDxy())/abs(pfmuCands[i].getDxyE()),wgt);
                allPlots["pf_dz_sig_all"]->Fill(abs(pfmuCands[i].getDz())/abs(pfmuCands[i].getDzE()),wgt);
              }
            }

            if(mass123 > 0) {
              allPlots["massJPsiK"+chTag]->Fill(mass123,wgt);
              allPlots["massJPsiK_all"]->Fill(mass123,wgt);
            }
          }
        }
        //cht->Fill();
        if(debug) cout << "J/Psi DONE" << endl;
        //continue; //FIXME

        //D0 and D* 
        evch.njpsi=0;
        evch.nj=0;
        if(debug) cout << "Starting D0 and D*" << endl;
        int jetindex = allJetsVec[ij].getJetIndex();
        if(tracks.size() < 3) continue;
        size_t tmax = 4;
        tmax = tracks.size() >= tmax ? tmax : tracks.size();
        for(size_t i = 0; i < tmax; i++)
          for(size_t j = 0; j < tmax; j++) {
            if(i == j) continue;

            //opposite sign
            if(tracks[i].getPdgId()*tracks[j].getPdgId() != -211*211) continue;

            /*
            TLorentzVector p_track1, p_track2;
            p_track1.SetPtEtaPhiM(tracks[i].Pt(), tracks[i].Eta(), tracks[i].Phi(), gMassPi);
            p_track2.SetPtEtaPhiM(tracks[j].Pt(), tracks[j].Eta(), tracks[j].Phi(), gMassK);
            float mass12 = (p_track1+p_track2).M();
            */
            float mass12 = (tracks[i].getVec()+tracks[j].getVec()).M();

            //allPlots["dR"+chTag+"_meson"]->Fill(p_track1.DeltaR(p_track2), wgt);
            allPlots["dR"+chTag+"_meson"]->Fill(tracks[i].DeltaR(tracks[j]), wgt);
            //allPlots["dR"+chTag+"_meson_no_weight"]->Fill(p_track1.DeltaR(p_track2),norm);
            tracks[i].setMass(gMassPi);
            if(!tracks[i].highPurity()) continue;
            if(tracks[i].Pt() < 5.0) continue;
            tracks[j].setMass(gMassK);
            if(!tracks[j].highPurity()) continue;
            if(tracks[j].Pt() < 1.0) continue;
            std::vector<pfTrack> pfCands = {tracks[i], tracks[j]};
            /*
            pfCands.push_back(pfTrack(p_track1, tracks[i].getDxy(), tracks[i].getDxyE(), tracks[i].getDz(), tracks[i].getDzE(), tracks[i].getPdgId()));
            pfCands.push_back(pfTrack(p_track2, tracks[j].getDxy(), tracks[j].getDxyE(), tracks[j].getDz(), tracks[j].getDzE(), tracks[j].getPdgId()));
            */

            std::vector<pfTrack> pfMatched;
            //Gen-matching
            if(!ev.isData) {
              //Don't use as it would skip non-gen events
              //if(!ev.ngmeson) continue; //event has no mesons
              bool isDEvent(false);
              if(ev.ngmeson_daug<2) continue; //require at least 2 daughters (mu^+ and mu^-)
              for(int ij = 0; ij < ev.ngmeson; ij++) {
                if(abs(ev.gmeson_id[ij])==421 || abs(ev.gmeson_id[ij])==413) { isDEvent = true; continue; } //loop until D found
              }

              std::vector<pfTrack> genTracks;
              for(int ig = 0; ig < ev.ngmeson_daug; ig++) {
                if(!isDEvent) break;
                //if(abs(ev.gmeson_id[ig])!=443) continue; //JPsi only
                TLorentzVector gen;
                float gen_mass = 0.0;
                if(abs(ev.gmeson_daug_id[ig])==211) gen_mass = gMassPi;
                else if(abs(ev.gmeson_daug_id[ig])==321) gen_mass = gMassK;
                else continue; //no pi/K found
                gen.SetPtEtaPhiM(ev.gmeson_daug_pt[ig], ev.gmeson_daug_eta[ig], ev.gmeson_daug_phi[ig], gen_mass);
                genTracks.push_back(pfTrack(gen,0,0,0,0,ev.gmeson_daug_id[ig],3,true));
              }

              for(auto & it : pfCands) { //FIXME reference might not work
                double dR = 0.3; //initial dR
                int best_idx = -1;
                for(auto & itg : genTracks) {
                  if(it.getPdgId() != itg.getPdgId()) continue; //insure ID and charge
                  if(it.getVec().DeltaR(itg.getVec())>dR) continue; //find dR
                  if(((it.Pt()-itg.Pt())/it.Pt())>0.10) continue; //gen and reco less than 10% difference
                  dR = it.getVec().DeltaR(itg.getVec());
                  best_idx = &itg - &genTracks[0]; //get index on current closest gen particle
                }
                if(best_idx<0) continue; //no gen track matched
                genTracks.erase(genTracks.begin() + best_idx); //remove gen track so it cannot be matched again!
                pfMatched.push_back(it);
              }
            }
            runBCDEF.Fill(pfCands, leptons, bJetsVec[ij], chTag, "meson");
            runGH.Fill(pfCands, leptons, bJetsVec[ij], chTag, "meson");
            if(!ev.isData && pfMatched.size() > 1) { //save gen-matched J/Psi
              runBCDEF.Fill(pfMatched, leptons, bJetsVec[ij], chTag, "gmeson");
              runGH.Fill(pfMatched, leptons, bJetsVec[ij], chTag, "gmeson");
            }

            if (mass12>1.65 && mass12<2.0) {
              if(debug) cout << i << ": " << tracks[i].Pt() << " " << tracks[i].Eta() << " " << tracks[i].Phi() << " " << gMassPi << endl;
              if(debug) cout << j << ": " << tracks[j].Pt() << " " << tracks[j].Eta() << " " << tracks[j].Phi() << " " << gMassK << endl << endl;
              if(debug) cout << mass12 << endl;
              allPlots["massD0"+chTag]->Fill(mass12,wgt);
              allPlots["massD0_all"]->Fill(mass12,wgt);
              //allPlots["massD0"+chTag+"_no_weight"]->Fill(mass12,norm);
              allPlots["nbj"+chTag+"_meson"]->Fill(1,wgt);
              //cout << ev.event << " " << iev << " " << jetindex << endl;
              runBCDEF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "meson");
              runGH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "meson");
            }

            //looking for lepton
            if(debug) cout << "third lepton" << endl;
            //for(int tk3 = 0; tk3 < ev.npf; tk3++)
            for(size_t k = 0; k < tracks.size(); k++) {
              if(k == i) continue;
              if(k == j) continue;
              if(debug) cout << "third lepton possible" << endl;
            
              if(abs(tracks[k].getPdgId()) != 13 && abs(tracks[k].getPdgId()) != 11) continue;
              if(!tracks[k].highPurity()) continue;
              if(abs(tracks[k].getPdgId()) == 13 && !tracks[k].trackerMuon() && !tracks[k].globalMuon()) continue;
              if(tracks[k].Pt() < 3.0) continue;
              if(debug) cout << "third lepton found" << endl;

              //if(tracks[j].getPdgId()/abs(tracks[j].getPdgId()) == -tracks[k].getPdgId()/abs(tracks[k].getPdgId())) {
              if(tracks[j].charge() == -tracks[k].charge()) {
                //Kaon and lepton have same charge (e.g. b^- -> c^+ W^- -> c^+ l^+ nubar)
                //correct mass assumption
                if(debug) cout << "correct mass assumption" << endl;

                allPlots["massD0_lep"+chTag]->Fill(mass12,wgt);
                allPlots["massD0_lep"+chTag+"_no_weight"]->Fill(mass12,norm);

                if(abs(tracks[k].getPdgId()) == 13)
                  allPlots["massD0_mu"+chTag]->Fill(mass12,wgt);
                if(abs(tracks[k].getPdgId()) == 11)
                  allPlots["massD0_e"+chTag]->Fill(mass12,wgt);
                if(abs(tracks[k].getPdgId()) == 13)
                  allPlots["massD0_mu"+chTag+"_no_weight"]->Fill(mass12,norm);
                if(abs(tracks[k].getPdgId()) == 11)
                  allPlots["massD0_e"+chTag+"_no_weight"]->Fill(mass12,norm);
                
                std::vector<pfTrack> tmp_cands = pfCands;
                tmp_cands.push_back(tracks[k]);
                runBCDEF.Fill(tmp_cands, leptons, bJetsVec[ij], chTag, "meson");
                runGH.Fill(tmp_cands, leptons, bJetsVec[ij], chTag, "meson");
              }
            }
            //looking for pion
            if(debug) cout << "D*->pi+D0" << endl;
            for(size_t k = 0; k < tracks.size(); k++) {
              if(k == i) continue;
              if(k == j) continue;

              if(abs(tracks[k].getPdgId()) != 211) continue;
              if(debug) cout << "Pion found" << endl;

              TLorentzVector p_track3, p_cand;
              std::vector<pfTrack> tmp_cands = pfCands;
              p_track3.SetPtEtaPhiM(tracks[k].Pt(), tracks[k].Eta(), tracks[k].Phi(), gMassPi);
              tracks[k].setMass(gMassPi);
              //pfCands.push_back(pfTrack(p_track3, tracks[k].getDxy(), tracks[k].getDxyE(), tracks[k].getDz(), tracks[k].getDzE(), tracks[k].getPdgId()));
              if(debug) cout << k << ": " << tracks[k].Pt() << " " << tracks[k].Eta() << " " << tracks[k].Phi() << " " << gMassPi << endl;
              allPlots["pi_pt"+chTag]->Fill(p_track3.Pt(),wgt);
              allPlots["pi_pt"+chTag+"_no_weight"]->Fill(p_track3.Pt(),norm);
              //if( tracks[j].getPdgId()/abs(tracks[j].getPdgId()) == -tracks[k].getPdgId()/abs(tracks[k].getPdgId()) ) {
              if( tracks[j].charge() == -tracks[k].charge() ) {
                // Kaon and pion have opposite charges
                // I.e. correct mass assumption
                //tmp_cands.push_back(pfTrack(p_track3, tracks[k].getDxy(), tracks[k].getDxyE(), tracks[k].getDz(), tracks[k].getDzE(), tracks[k].getPdgId()));
                tmp_cands.push_back(tracks[k]);
                if(debug) cout << "correct mass assumption (D*)" << endl;

                //p_cand = p_track1+p_track2+p_track3;
                p_cand = tracks[i].getVec() + tracks[j].getVec() + tracks[k].getVec();
                allPlots["massDs"+chTag]->Fill(p_cand.M(), wgt);
                allPlots["massDs"+chTag+"_no_weight"]->Fill(p_cand.M(),norm);
                allPlots["massDs_all"]->Fill(p_cand.M(), wgt);

                runBCDEF.Fill(tmp_cands, leptons, bJetsVec[ij], chTag, "meson");
                runGH.Fill(tmp_cands, leptons, bJetsVec[ij], chTag, "meson");

                if(fabs(mass12-1.864) < 0.10) { // mass window cut
                  TLorentzVector p_jet;
                  p_jet.SetPtEtaPhiM(ev.j_pt[jetindex], ev.j_eta[jetindex], ev.j_phi[jetindex], 0.);

                  float deltam = p_cand.M() - mass12;

                  allPlots["massDsmD0loose"+chTag]->Fill(deltam, wgt);
                  allPlots["massDsmD0loose"+chTag+"_no_weight"]->Fill(deltam,norm);
                  allPlots["massDsmD0loose_all"]->Fill(deltam, wgt);
                  if(fabs(mass12-1.864) < 0.05) { // tighter mass window cut
                    if(debug) std::cout << "TOP.CC Masses: " << tmp_cands[0].getVec().M() << " ";
                    if(debug) std::cout << tmp_cands[1].getVec().M() << " ";
                    if(debug) std::cout << tmp_cands[2].getVec().M() << std::endl << std::endl;

                    allPlots["massDsmD0"+chTag]->Fill(deltam, wgt);
                    allPlots["massDsmD0"+chTag+"_no_weight"]->Fill(deltam, norm);
                    allPlots["massDsmD0_all"]->Fill(deltam, wgt);
                    allPlots["lp_pt"+chTag+"_meson"]->Fill(leptons[0].Pt(),wgt);
                    allPlots["lp_pt_low"+chTag+"_meson"]->Fill(leptons[0].Pt(),wgt);
                    allPlots["lp_eta"+chTag+"_meson"]->Fill(leptons[0].Eta(),wgt);
                    allPlots["lp_phi"+chTag+"_meson"]->Fill(leptons[0].Phi(),wgt);
                    if(deltam<0.14 || deltam>0.15) continue;
                    runBCDEF.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "meson");
                    runGH.Fill(1, ev.nvtx, htsum, stsum, ev.met_pt[0], chTag, "meson");
                    allPlots["nevt"+chTag+"_meson"]->Fill(1,norm);
                    allPlots["weight"+chTag+"_meson"]->Fill(wgt,norm);
                    allPlots["norm"+chTag+"_meson"]->Fill(norm,norm);
                    allPlots["nvtx"+chTag+"_meson"]->Fill(ev.nvtx,wgt);
                  }
                }
              }
            }
          }
        if(debug) cout << "D0 and D* DONE" << endl;
      }

    }

  //close input file
  f->Close();

  //save histos to file  
  //TString selPrefix("");  
  //if(flavourSplitting!=NOFLAVOURSPLITTING) selPrefix=Form("%d_",flavourSplitting);
  //TString baseName=gSystem->BaseName(outname); 
  //TString dirName=gSystem->DirName(outname);
  //TFile *fOut=TFile::Open(dirName+"/"+selPrefix+baseName,"RECREATE");
  fOut->cd();
  if(debug) cout << "writing histograms" << endl;

  for (auto& it : allPlots)  { 
    if(debug) cout << it.second->GetName() << endl;
    if(debug) cout << it.second->GetEntries() << endl;

    it.second->SetDirectory(fOut); it.second->Write(); 
    fOut->cd();
  }
  /*
  runB.Write();
  runC.Write();
  runD.Write();
  runE.Write();
  runF.Write();
  runG.Write();
  runH.Write();
  */
  runBCDEF.Write();
  runGH.Write();
  if(debug) cout << "writing histograms DONE" << endl;

  if(debug) cout << "writing tree" << endl;
  cht->Write();
  if(debug) cout << "writing tree DONE" << endl;

  if(debug) cout << "closing ROOT file" << endl;
  fOut->Close();
}


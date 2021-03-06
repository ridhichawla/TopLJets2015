//
// -*- C++ -*-
//
// Package:    TopLJets2015/TopAnalysis
// Class:      MiniAnalyzer
// 
/**\class MiniAnalyzer MiniAnalyzer.cc Test/MiniAnalyzer/plugins/MiniAnalyzer.cc

   Description: [one line class summary]

   Implementation:
   [Notes on implementation]
*/
//
// Original Author:  Qamar Ul Hassan
//         Created:  Sun, 13 Jul 2014 06:22:18 GMT
//
//
#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/VIDCutFlowResult.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "FWCore/Framework/interface/TriggerNamesService.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"

#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TopLJets2015/TopAnalysis/interface/MiniEvent.h"
#include "TopLJets2015/TopAnalysis/interface/MyIPTools.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/Ptr.h"

#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "TopLJets2015/TopAnalysis/interface/rochcor2016.h"

#include "TLorentzVector.h"
#include "TH1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TTree.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <iostream>
#include <string>

using namespace edm;
using namespace std;
using namespace reco;
using namespace pat; 

//
// class declaration
//

// For PF skimming
bool pfSort(std::pair<int,double> pf1, std::pair<int,double> pf2) { return pf1.second > pf2.second; }

class MiniAnalyzer : public edm::EDAnalyzer {
public:
  explicit MiniAnalyzer(const edm::ParameterSet&);
  ~MiniAnalyzer();  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  virtual void endRun(const edm::Run&,const edm::EventSetup&);  
private:
  virtual void beginJob() override;
  int genAnalysis(const edm::Event& iEvent, const edm::EventSetup& iSetup);
  int recAnalysis(const edm::Event& iEvent, const edm::EventSetup& iSetup);
  void KalmanAnalysis(const edm::Event& iEvent, const edm::EventSetup& iSetup, const pat::Jet &j, const reco::Vertex &primVtx);
  std::pair<float,float> ctau(const TLorentzVector &p4, const reco::Vertex &fittedVertex,
                              const reco::Vertex &primVtx, std::vector<std::pair<float,float>> &lsigma);
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  float getMiniIsolation(edm::Handle<pat::PackedCandidateCollection> pfcands,
			 const reco::Candidate* ptcl,  
                         float r_iso_min, float r_iso_max, float kt_scale,
                         bool charged_only);

  // member data 
  edm::EDGetTokenT<GenEventInfoProduct> generatorToken_;
  edm::EDGetTokenT<GenEventInfoProduct> generatorevtToken_;
  edm::EDGetTokenT<LHEEventProduct> generatorlheToken_;
  edm::EDGetTokenT<LHERunInfoProduct> generatorRunInfoToken_;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > puToken_;
  edm::EDGetTokenT<std::vector<reco::GenJet>  > genLeptonsToken_,   genJetsToken_;
  edm::EDGetTokenT<edm::ValueMap<float> > upFragToken_, centralFragToken_, downFragToken_;
  //edm::EDGetTokenT<edm::ValueMap<float> > petersonFragToken_, upFragToken_, centralFragToken_, downFragToken_;
  edm::EDGetTokenT<pat::PackedGenParticleCollection> genParticlesToken_;
  edm::EDGetTokenT<reco::GenParticleCollection> prunedGenParticlesToken_;
  edm::EDGetTokenT<reco::GenParticleCollection> pseudoTopToken_;

  edm::EDGetTokenT<edm::TriggerResults> triggerBits_;
  edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescales_;
  edm::EDGetTokenT<reco::VertexCollection> vtxToken_;
  edm::EDGetTokenT<double> rhoToken_;
  edm::EDGetTokenT<pat::MuonCollection> muonToken_;
  edm::EDGetTokenT<edm::View<pat::Electron>  >  electronToken_;
  edm::EDGetTokenT<edm::View<pat::Jet> > jetToken_;
  edm::EDGetTokenT<pat::METCollection> metToken_, puppiMetToken_;
  edm::EDGetTokenT<pat::PackedCandidateCollection> pfToken_;
  
  //Electron Decisions
  edm::EDGetTokenT<edm::ValueMap<bool> > eleVetoIdMapToken_;
  edm::EDGetTokenT<edm::ValueMap<bool> > eleTightIdMapToken_;
  edm::EDGetTokenT<edm::ValueMap<vid::CutFlowResult> > eleTightIdFullInfoMapToken_;

  std::unordered_map<std::string,TH1F*> histContainer_;

  //muon rochester corrections
  rochcor2016 *rochcor_;

  PFJetIDSelectionFunctor pfjetIDLoose_;

  std::vector<std::string> muTriggersToUse_, elTriggersToUse_;

  bool saveTree_,savePF_;
  TTree *tree_;
  MiniEvent_t ev_;

  //a counter for generator level scans (e.g. sms scans)
  std::map<TString, float>  genScanCounter_;

  edm::Service<TFileService> fs;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//


//
// constructors and destructor
//
MiniAnalyzer::MiniAnalyzer(const edm::ParameterSet& iConfig) :
  generatorToken_(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
  generatorevtToken_(consumes<GenEventInfoProduct>(edm::InputTag("generator",""))),
  generatorlheToken_(consumes<LHEEventProduct>(edm::InputTag("externalLHEProducer",""))),
  generatorRunInfoToken_(consumes<LHERunInfoProduct,edm::InRun>({"externalLHEProducer"})),
  puToken_(consumes<std::vector<PileupSummaryInfo>>(edm::InputTag("slimmedAddPileupInfo"))),  
  genLeptonsToken_(consumes<std::vector<reco::GenJet> >(edm::InputTag("pseudoTop:leptons"))),
  genJetsToken_(consumes<std::vector<reco::GenJet> >(edm::InputTag("pseudoTop:jets"))),
  /*
  xbToken_(consumes<edm::ValueMap<float> >(edm::InputTag("bfragWgtProducer:xb"))),
  petersonFragToken_(consumes<edm::ValueMap<float> >(edm::InputTag("bfragWgtProducer:PetersonFrag"))),
  */
  upFragToken_(consumes<edm::ValueMap<float> >(edm::InputTag("bfragWgtProducer:upFrag"))),
  centralFragToken_(consumes<edm::ValueMap<float> >(edm::InputTag("bfragWgtProducer:centralFrag"))),
  downFragToken_(consumes<edm::ValueMap<float> >(edm::InputTag("bfragWgtProducer:downFrag"))),
  genParticlesToken_(consumes<pat::PackedGenParticleCollection>(edm::InputTag("packedGenParticles"))),
  prunedGenParticlesToken_(consumes<reco::GenParticleCollection>(edm::InputTag("prunedGenParticles"))),
  pseudoTopToken_(consumes<reco::GenParticleCollection>(edm::InputTag("pseudoTop"))),
  triggerBits_(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerBits"))),
  triggerPrescales_(consumes<pat::PackedTriggerPrescales>(iConfig.getParameter<edm::InputTag>("prescales"))),
  vtxToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
  rhoToken_(consumes<double>(iConfig.getParameter<edm::InputTag>("rho"))),
  muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muons"))),
  jetToken_(consumes<edm::View<pat::Jet> >(iConfig.getParameter<edm::InputTag>("jets"))),  
  metToken_(consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("mets"))),
  puppiMetToken_(consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("puppimets"))),
  pfToken_(consumes<pat::PackedCandidateCollection>(iConfig.getParameter<edm::InputTag>("pfCands"))),
  eleVetoIdMapToken_(consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("eleVetoIdMap"))),
  eleTightIdMapToken_(consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("eleTightIdMap"))),
  eleTightIdFullInfoMapToken_(consumes<edm::ValueMap<vid::CutFlowResult> >(iConfig.getParameter<edm::InputTag>("eleTightIdFullInfoMap"))),
  pfjetIDLoose_( PFJetIDSelectionFunctor::FIRSTDATA, PFJetIDSelectionFunctor::LOOSE ),  
  saveTree_( iConfig.getParameter<bool>("saveTree") ),
  savePF_( iConfig.getParameter<bool>("savePF") )
{
  //now do what ever initialization is needed
  electronToken_ = mayConsume<edm::View<pat::Electron> >(iConfig.getParameter<edm::InputTag>("electrons"));
  elTriggersToUse_ = iConfig.getParameter<std::vector<std::string> >("elTriggersToUse");
  muTriggersToUse_ = iConfig.getParameter<std::vector<std::string> >("muTriggersToUse");

  //start the rochester correction tool
  rochcor_=new rochcor2016(2016);
 
  //  usesResource("TFileService");

  for(Int_t igenjet=0; igenjet<5; igenjet++)
    {
      TString tag("fidcounter"); tag+=igenjet;
      histContainer_[tag.Data()] = fs->make<TH1F>(tag,    ";Variation;Events", 1000, 0., 1000.); 
    }
  histContainer_["counter"] = fs->make<TH1F>("counter", ";Counter;Events",2,0,2);
  histContainer_["pu"]      = fs->make<TH1F>("pu",      ";Pileup observed;Events",100,0,100);
  histContainer_["putrue"]  = fs->make<TH1F>("putrue",  ";Pileup true;Events",100,0,100);
  for(std::unordered_map<std::string,TH1F*>::iterator it=histContainer_.begin();   it!=histContainer_.end();   it++) it->second->Sumw2();

  //create a tree for the selected events
  if(saveTree_)
    {
      tree_ = fs->make<TTree>("data","data");
      createMiniEventTree(tree_,ev_);
    }
}


MiniAnalyzer::~MiniAnalyzer()
{
 
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//
int MiniAnalyzer::genAnalysis(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  //
  // PILEUP
  //
  edm::Handle<std::vector <PileupSummaryInfo> > PupInfo;
  iEvent.getByToken(puToken_,PupInfo);
  std::vector<PileupSummaryInfo>::const_iterator ipu;
  for (ipu = PupInfo->begin(); ipu != PupInfo->end(); ++ipu) 
    {
      if ( ipu->getBunchCrossing() != 0 ) continue; // storing detailed PU info only for BX=0
      ev_.pu=ipu->getPU_NumInteractions();
      ev_.putrue=ipu->getTrueNumInteractions();
    }
  histContainer_["pu"]->Fill(ev_.pu);
  histContainer_["putrue"]->Fill(ev_.putrue);
  
  //
  // GENERATOR WEIGHTS
  //
  ev_.ttbar_nw=0;
  edm::Handle<GenEventInfoProduct> evt;
  iEvent.getByToken( generatorToken_,evt);
  if(evt.isValid())
    {
      ev_.ttbar_allmepartons   = evt->nMEPartons();
      ev_.ttbar_matchmepartons = evt->nMEPartonsFiltered();
      ev_.ttbar_w[0]           = evt->weight();
      ev_.ttbar_nw++;
    }
  histContainer_["counter"]->Fill(1,ev_.ttbar_w[0]);
  
  //alternative weights for systematics
  edm::Handle<LHEEventProduct> evet;
  iEvent.getByToken(generatorlheToken_, evet);
  if(evet.isValid())
    {
      double asdd=evet->originalXWGTUP();
      for(unsigned int i=0  ; i<evet->weights().size();i++){
	double asdde=evet->weights()[i].wgt;
	ev_.ttbar_w[ev_.ttbar_nw]=ev_.ttbar_w[0]*asdde/asdd;
	ev_.ttbar_nw++;
      }
    }
      
  //
  // GENERATOR LEVEL EVENT
  //
  ev_.ng=0; ev_.ngjets=0; ev_.ngbjets=0; ev_.ngj=0;
  edm::Handle<std::vector<reco::GenJet> > genJets;
  iEvent.getByToken(genJetsToken_,genJets);  
  /*
  edm::Handle<edm::ValueMap<float> > xb;
  iEvent.getByToken(xbToken_,xb);
  edm::Handle<edm::ValueMap<float> > petersonFrag;
  iEvent.getByToken(petersonFragToken_,petersonFrag);
  */
  edm::Handle<edm::ValueMap<float> > upFrag;
  iEvent.getByToken(upFragToken_,upFrag);
  edm::Handle<edm::ValueMap<float> > centralFrag;
  iEvent.getByToken(centralFragToken_,centralFrag);
  edm::Handle<edm::ValueMap<float> > downFrag;
  iEvent.getByToken(downFragToken_,downFrag);
  std::map<const reco::Candidate *,int> jetConstsMap;
  for(auto genJet = genJets->begin();  genJet != genJets->end(); ++genJet)
    {
      //map the gen particles which are clustered in this jet
      std::vector< const reco::Candidate * > jconst=genJet->getJetConstituentsQuick ();
      for(size_t ijc=0; ijc <jconst.size(); ijc++)
	jetConstsMap[ jconst[ijc] ] = ev_.ng;
      
      ev_.g_id[ev_.ng]   = genJet->pdgId();
      ev_.g_pt[ev_.ng]   = genJet->pt();
      ev_.g_eta[ev_.ng]  = genJet->eta();
      ev_.g_phi[ev_.ng]  = genJet->phi();
      ev_.g_m[ev_.ng]    = genJet->mass();       
      
      //gen level selection
      if(genJet->pt()>25 && fabs(genJet->eta())<2.5)
	{
	  ev_.ngjets++;
	  if(abs(genJet->pdgId())) ev_.ngbjets++;
	}
      edm::Ref<std::vector<reco::GenJet> > genJetRef(genJets,genJet-genJets->begin());
      /*
      ev_.xb[ev_.ngj] = (*xb)[genJetRef];
      ev_.peterson[ev_.ng] = (*petersonFrag)[genJetRef];
      */
      ev_.up[ev_.ng] = (*upFrag)[genJetRef];
      ev_.central[ev_.ng] = (*centralFrag)[genJetRef];
      ev_.down[ev_.ng] = (*downFrag)[genJetRef];
      ev_.ng++;
      ev_.ngj++;
    }

  //leptons
  ev_.ngleptons=-0;
  int ngleptons(0);
  edm::Handle<std::vector<reco::GenJet> > dressedLeptons;
  iEvent.getByToken(genLeptonsToken_,dressedLeptons);
  for(auto genLep = dressedLeptons->begin();  genLep != dressedLeptons->end(); ++genLep)
    {
      //map the gen particles which are clustered in this lepton
      std::vector< const reco::Candidate * > jconst=genLep->getJetConstituentsQuick ();
      for(size_t ijc=0; ijc <jconst.size(); ijc++)
	jetConstsMap[ jconst[ijc] ] = ev_.ng;
      
      ev_.g_pt[ev_.ng]   = genLep->pt();
      ev_.g_id[ev_.ng]   = genLep->pdgId();
      ev_.g_eta[ev_.ng]  = genLep->eta();
      ev_.g_phi[ev_.ng]  = genLep->phi();
      ev_.g_m[ev_.ng]    = genLep->mass();       
      ev_.ng++;

      //gen level selection
      if(genLep->pt()>20 && fabs(genLep->eta())<2.5) ev_.ngleptons++;
      if(genLep->pt()>20 && fabs(genLep->eta())<2.5) ngleptons++;
    }
  
  
  //final state particles 
  ev_.ngpf=0;
  edm::Handle<pat::PackedGenParticleCollection> genParticles;
  iEvent.getByToken(genParticlesToken_,genParticles);
  for (size_t i = 0; i < genParticles->size(); ++i)
    {
      const pat::PackedGenParticle & genIt = (*genParticles)[i];

      //this shouldn't be needed according to the workbook
      //if(genIt.status()!=1) continue;
      if(genIt.pt()<0.5 || fabs(genIt.eta())>2.5) continue;
      
      ev_.gpf_id[ev_.ngpf]     = genIt.pdgId();
      ev_.gpf_c[ev_.ngpf]      = genIt.charge();
      ev_.gpf_g[ev_.ngpf]=-1;
      for(std::map<const reco::Candidate *,int>::iterator it=jetConstsMap.begin();
	  it!=jetConstsMap.end();
	  it++)
	{
	  if(it->first->pdgId()!=genIt.pdgId()) continue;
	  if(deltaR( *(it->first), genIt)>0.01) continue; 
	  ev_.gpf_g[ev_.ngpf]=it->second;
	  break;
	}
      ev_.gpf_pt[ev_.ngpf]     = genIt.pt();
      ev_.gpf_eta[ev_.ngpf]    = genIt.eta();
      ev_.gpf_phi[ev_.ngpf]    = genIt.phi();
      ev_.gpf_m[ev_.ngpf]      = genIt.mass();
      ev_.gpf_mother[ev_.ngpf] = genIt.motherRef()->pdgId();
      ev_.ngpf++;    
    }

  //J/Psi decay
  //Meson decay (J/Psi or D0)
  ev_.ngjpsi=0;
  ev_.ngmeson=0;
  //prurned also used for top/stop
  edm::Handle<reco::GenParticleCollection> prunedGenParticles;
  iEvent.getByToken(prunedGenParticlesToken_,prunedGenParticles);
  for(size_t i = 0; i < prunedGenParticles->size(); i++) {
    const reco::GenParticle &genIt = (*prunedGenParticles)[i];
    int absid=abs(genIt.pdgId());
    if(absid!=443 && absid!=421 && absid!=413) continue;
    if(genIt.numberOfDaughters()<2) continue;
    /*
    const reco::GenParticle* motherTmp = &(*prunedGenParticles)[i];
    while(abs(motherTmp->pdgId()) != 6 && abs(motherTmp->pdgId()) != 22 && abs(motherTmp->pdgId()) != 2212) {
      if (motherTmp->mother() == 0) break;
      motherTmp = (reco::GenParticle*) motherTmp->mother();
    }
    */
    //cout << "mother0 id= " << motherTmp->pdgId() << endl;
    //if(genIt.daughter(0)->pdgId()*genIt.daughter(1)->pdgId()!=-13*13 &&
    //   genIt.daughter(0)->pdgId()*genIt.daughter(1)->pdgId()!=-211*321 &&
    //   genIt.daughter(0)->pdgId()*genIt.daughter(1)->pdgId()!=-211*321*-211) continue;
    //cout << "New meson found: ngmeson = " << ev_.ngmeson << endl; 
    //cout << "daughter found" << endl;
    bool JPsiDaughter = false;
    bool D0Daughter = false;
    bool DsDaughter = false;
    ev_.ngmeson_daug=0;
    for(size_t ipf=0; ipf<genIt.numberOfDaughters(); ipf++) {
      //cout << "loading daughter" << endl;
      const reco::Candidate *daug=genIt.daughter(ipf);
      if(abs(daug->pdgId())==13 && absid==443) JPsiDaughter = true;
      else if(abs(daug->pdgId())==211 && absid==421) D0Daughter = true;
      else if(abs(daug->pdgId())==321 && absid==421) D0Daughter = true;
      else if(abs(daug->pdgId())==211 && absid==413) DsDaughter = true;
      else if(abs(daug->pdgId())==321 && absid==413) DsDaughter = true;
      //else continue;
      if(abs(daug->pdgId())==14 || abs(daug->pdgId())==13) continue;
      if(abs(daug->pdgId())==12 || abs(daug->pdgId())==11) continue;
      //if(!JPsiDaughter) cout << "event = " << ev_.event << " ngmeson = " << ev_.ngmeson << " : pdgId() = " << genIt.pdgId() << " : daughter n = " << ipf <<  " : ngmeson_daughter = " << ev_.ngmeson_daug << " : daug->pdgId() = " << daug->pdgId() << endl;
      /*
      if(JPsiDaughter) cout << "J/Psi" << endl;
      else if(D0Daughter) cout << "D0" << endl;
      cout << "daugther is muon: pT, eta, phi" << endl;
      cout << daug->pdgId() << endl;
      cout << daug->pt() << ", ";
      cout << daug->eta() << ", ";
      cout << daug->phi() << endl;
      */
      //2*ev_.ngjpsi to account for 2 duaghters per J/Psi
      ev_.gmeson_daug_id[ev_.ngmeson_daug] = daug->pdgId();
      ev_.gmeson_daug_pt[ev_.ngmeson_daug] = daug->pt();
      ev_.gmeson_daug_eta[ev_.ngmeson_daug] = daug->eta();
      ev_.gmeson_daug_phi[ev_.ngmeson_daug] = daug->phi();
      ev_.gmeson_daug_meson_index[ev_.ngmeson_daug] = ev_.ngmeson;
      //cout << "i: " << i << " " << ev_.event << " " << ev_.ngmeson << " " << ev_.ngmeson_daug << " " << ev_.ngmeson << endl;

      /*
      ev_.gjpsi_mu_dxy[ev_.ngjpsi]  = daug->dxy(primVtx.position());
      ev_.gjpsi_mu_dxyE[ev_.ngjpsi]  = daug->dxyE();
      ev_.gjpsi_mu_dz[ev_.ngjpsi]  = daug->dz(primVtx.position());
      ev_.gjpsi_mu_dzE[ev_.ngjpsi]  = daug->dzE();
      */

      //Find t(tbar) mother
      while(abs(daug->pdgId()) != 6 && abs(daug->pdgId()) != 22 && abs(daug->pdgId()) != 2212) {
        if(daug->mother() == 0) break;
        //int charge = daug->charge();
        //if(!charge) charge = 1;
        //cout << "PdgId= " << daug->pdgId()*charge << endl;
        daug = daug->mother();
        //charge = daug->charge();
        //if(!charge) charge = 1;
        //cout << "Mother PdgId= " << daug->pdgId() << endl;
        //if(abs(daug->pdgId()) == 2212) break;
        //if(abs(daug->pdgId()) == 22) break;
      }
      //Find t(tbar) mother

      ev_.gmeson_mother_id[ev_.ngmeson_daug] = daug->pdgId(); //*daug->charge();
      ev_.ngmeson_daug++;
    }
    if(!JPsiDaughter && !D0Daughter && !DsDaughter) continue;
    /*
    cout << "J/Psi: pT, eta, phi, M" << endl;
    cout << genIt.pt() << ", ";
    cout << genIt.eta() << ", ";
    cout << genIt.phi() << ", ";
    cout << genIt.mass() << endl;
    */
    ev_.gmeson_id[ev_.ngmeson]     = genIt.pdgId();
    ev_.gmeson_pt[ev_.ngmeson]     = genIt.pt();
    ev_.gmeson_eta[ev_.ngmeson]    = genIt.eta();
    ev_.gmeson_phi[ev_.ngmeson]    = genIt.phi();
    ev_.gmeson_m[ev_.ngmeson]      = genIt.mass();
    ev_.gmeson_daug_dR[ev_.ngmeson]  = reco::deltaR(genIt.daughter(0)->eta(),genIt.daughter(0)->phi(),genIt.daughter(1)->eta(),genIt.daughter(1)->phi());
    ev_.gmeson_index[ev_.ngmeson] = ev_.ngmeson;
    if(JPsiDaughter) ev_.ngjpsi++;
    //if(JPsiDaughter || D0Daughter) ev_.ngmeson++;
    ev_.ngmeson++;
  }

  //top or stop quarks (lastCopy)
  //edm::Handle<reco::GenParticleCollection> prunedGenParticles;
  //iEvent.getByToken(prunedGenParticlesToken_,prunedGenParticles);
  ev_.ngtop=0; 
  float mStop(-1),mNeutralino(-1);
  for (size_t i = 0; i < prunedGenParticles->size(); ++i)
    {
      const reco::GenParticle & genIt = (*prunedGenParticles)[i];
      int absid=abs(genIt.pdgId());
      if(absid!=6 && absid!=1000006 && absid!=1000022) continue;
      if(!genIt.isLastCopy()) continue;      

      if(absid==1000006) mStop=genIt.mass();
      if(absid==1000022) mNeutralino=genIt.mass();

      ev_.gtop_id[ ev_.ngtop ]  = genIt.pdgId();
      ev_.gtop_pt[ ev_.ngtop ]  = genIt.pt();
      ev_.gtop_eta[ ev_.ngtop ] = genIt.eta();
      ev_.gtop_phi[ ev_.ngtop ] = genIt.phi();
      ev_.gtop_m[ ev_.ngtop ]   = genIt.mass();
      ev_.ngtop++;
    }

  //check if this is a SMS scan
  if(mStop>0 && mNeutralino>0)
    {
      TString key(Form("mstop_%d_mchi0_%d",int(10*mStop),int(10*mNeutralino)));
      if(genScanCounter_.find(key)==genScanCounter_.end()) genScanCounter_[key]=0.;
      genScanCounter_[key]+=ev_.ttbar_w[0];
    }
    

  //pseudo-tops 
  edm::Handle<reco::GenParticleCollection> pseudoTop;
  iEvent.getByToken(pseudoTopToken_,pseudoTop);
  for (size_t i = 0; i < pseudoTop->size(); ++i)
    {
      const GenParticle & genIt = (*pseudoTop)[i];
      ev_.gtop_id[ ev_.ngtop ]  = genIt.pdgId()*1000;
      ev_.gtop_pt[ ev_.ngtop ]  = genIt.pt();
      ev_.gtop_eta[ ev_.ngtop ] = genIt.eta();
      ev_.gtop_phi[ ev_.ngtop ] = genIt.phi();
      ev_.gtop_m[ ev_.ngtop ]   = genIt.mass();
      ev_.ngtop++;
    }
  
  //fiducial counters
  for(Int_t igenjet=0; igenjet<5; igenjet++)
    {
      TString tag("fidcounter"); tag+=igenjet;
      histContainer_[tag.Data()]->Fill(0.,ev_.ttbar_w[0]);
      if(igenjet<=ev_.ngjets && ev_.ngleptons>0)
	{
	  for(Int_t iw=1; iw<ev_.ttbar_nw; iw++)
	    histContainer_[tag.Data()]->Fill((float)iw,ev_.ttbar_w[iw]);
	}
    }
  return ngleptons;
}


//
int MiniAnalyzer::recAnalysis(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  //VERTICES
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vtxToken_, vertices);
  if (vertices->empty()) return 0; // skip the event if no PV found
  const reco::Vertex &primVtx = vertices->front();
  reco::VertexRef primVtxRef(vertices,0);
  ev_.nvtx=vertices->size();
  if(ev_.nvtx==0) return 0;

  //RHO
  edm::Handle< double > rhoH;
  iEvent.getByToken(rhoToken_,rhoH);
  float rho=*rhoH;
  ev_.rho=rho;


  //TRIGGER INFORMATION
  edm::Handle<edm::TriggerResults> h_trigRes;
  iEvent.getByToken(triggerBits_, h_trigRes);
  std::vector<string> triggerList;
  Service<service::TriggerNamesService> tns;
  tns->getTrigPaths(*h_trigRes,triggerList);
  ev_.muTrigger=0;
  ev_.elTrigger=0;
  for (unsigned int i=0; i< h_trigRes->size(); i++) 
    {	
      if( !(*h_trigRes)[i].accept() ) continue;
      for(size_t imu=0; imu<muTriggersToUse_.size(); imu++)
	{
	  if (triggerList[i].find(muTriggersToUse_[imu])==string::npos) continue;
	  ev_.muTrigger |= (1 << imu);
	}
      for(size_t iel=0; iel<elTriggersToUse_.size(); iel++) 
	{
	  if (triggerList[i].find(elTriggersToUse_[iel])==string::npos)continue;
	  ev_.elTrigger |= (1 << iel);
	}
    }
  bool passMuTrigger(ev_.isData ? ev_.muTrigger!=0 : true);
  bool passElTrigger(ev_.isData ? ev_.elTrigger!=0 : true);  
  if(!passMuTrigger && !passElTrigger) return 0;

  //PF candidates
  edm::Handle<pat::PackedCandidateCollection> pfcands;
  iEvent.getByToken(pfToken_,pfcands);

  //
  //LEPTON SELECTION
  //
  ev_.nl=0; ev_.nleptons=0;
  int nleptons(0);

  //MUON SELECTION: cf. https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideMuonIdRun2
  edm::Handle<pat::MuonCollection> muons;
  iEvent.getByToken(muonToken_, muons);
  for (const pat::Muon &mu : *muons) 
    { 
      //correct the 4-momentum
      TLorentzVector p4;
      p4.SetPtEtaPhiM(mu.pt(),mu.eta(),mu.phi(),mu.mass());
      float qter(1.0);
      try {
        if(iEvent.isRealData()) {
	  rochcor_->momcor_data(p4, mu.charge(), 0, qter);
        }
        else {
          int ntrk=mu.innerTrack()->hitPattern().trackerLayersWithMeasurement();
	  rochcor_->momcor_data(p4, mu.charge(), ntrk, qter);
        }
      }
      catch(...) {
        //probably no inner track...
      }

      //kinematics
      bool passPt( mu.pt() > 10 );
      bool passEta(fabs(mu.eta()) < 2.4 );
      if(!passPt || !passEta) continue;

      //ID
      bool isMedium(muon::isMediumMuon(mu));
      bool isTight(muon::isTightMuon(mu,primVtx));
      bool isLoose(muon::isLooseMuon(mu));
      bool isSoft(muon::isSoftMuon(mu,primVtx));
      if(!isMedium) continue;

      //save it
      const reco::GenParticle * gen=mu.genLepton(); 
      ev_.isPromptFinalState[ev_.nl] = gen ? gen->isPromptFinalState() : false;
      ev_.isDirectPromptTauDecayProductFinalState[ev_.nl] = gen ? gen->isDirectPromptTauDecayProductFinalState() : false;
      ev_.l_id[ev_.nl]=13;
      ev_.l_g[ev_.nl]=-1;
      for(int ig=0; ig<ev_.ng; ig++)
	{
	  if(abs(ev_.g_id[ig])!=ev_.l_id[ev_.nl]) continue;
	  if(deltaR( mu.eta(),mu.phi(), ev_.g_eta[ig],ev_.g_phi[ig])>0.4) continue;
	  ev_.l_g[ev_.nl]=ig;
	  break;
	}	 
      ev_.l_charge[ev_.nl]=mu.charge();
      ev_.l_pt[ev_.nl]=mu.pt();
      ev_.l_eta[ev_.nl]=mu.eta();
      ev_.l_phi[ev_.nl]=mu.phi();
      ev_.l_mass[ev_.nl]=mu.mass();
      ev_.l_pid[ev_.nl]=(isTight | (isMedium<<1) | (isLoose<<2) | (isSoft<<3));
      ev_.l_chargedHadronIso[ev_.nl]=mu.pfIsolationR04().sumChargedHadronPt;
      ev_.l_miniIso[ev_.nl]=getMiniIsolation(pfcands,&mu,0.05,0.2, 10., false);
      ev_.l_relIso[ev_.nl]=(mu.pfIsolationR04().sumChargedHadronPt + max(0., mu.pfIsolationR04().sumNeutralHadronEt + mu.pfIsolationR04().sumPhotonEt - 0.5*mu.pfIsolationR04().sumPUPt))/mu.pt();
      ev_.l_ip3d[ev_.nl] = -9999.;
      ev_.l_ip3dsig[ev_.nl] = -9999;
      if(mu.innerTrack().get())
	{
	  std::pair<bool,Measurement1D> ip3dRes = getImpactParameter<reco::TrackRef>(mu.innerTrack(), primVtxRef, iSetup, true);
	  ev_.l_ip3d[ev_.nl]    = ip3dRes.second.value();
	  ev_.l_ip3dsig[ev_.nl] = ip3dRes.second.significance();
	}   
      if(mu.outerTrack().isNonnull()) {
        ev_.l_dxy[ev_.nl]=mu.dB();
        ev_.l_dxyE[ev_.nl]=mu.edB();
      }
      if(mu.innerTrack().isNonnull()) {
        ev_.l_dxy[ev_.nl]=mu.dB();
        ev_.l_dxyE[ev_.nl]=mu.edB();
        ev_.l_dz[ev_.nl]=mu.innerTrack()->dz(primVtx.position());
        ev_.l_global[ev_.nl]=mu.isGlobalMuon();
        ev_.l_pf[ev_.nl]=mu.isPFMuon();
        ev_.l_nValTrackerHits[ev_.nl] = mu.innerTrack()->hitPattern().numberOfValidTrackerHits();
        ev_.l_nValPixelHits[ev_.nl] = mu.innerTrack()->hitPattern().numberOfValidPixelHits();
        ev_.l_nMatchedStations[ev_.nl] = mu.numberOfMatchedStations();
        ev_.l_pixelLayerWithMeasurement[ev_.nl]    = mu.innerTrack()->hitPattern().pixelLayersWithMeasurement();
        ev_.l_trackerLayersWithMeasurement[ev_.nl] = mu.innerTrack()->hitPattern().trackerLayersWithMeasurement();

        //Medium
        ev_.l_validFraction[ev_.nl] = mu.innerTrack()->validFraction();
        ev_.l_chi2LocalPosition[ev_.nl] = mu.combinedQuality().chi2LocalPosition;
        ev_.l_trkKink[ev_.nl] = mu.combinedQuality().trkKink;
      }

      if (mu.globalTrack().isNonnull()) {
        ev_.l_chi2norm[ev_.nl]=mu.normChi2();
        ev_.l_global[ev_.nl]=mu.isGlobalMuon();
        ev_.l_pf[ev_.nl]=mu.isPFMuon();
        ev_.l_globalTrackNumberOfValidHits[ev_.nl] = mu.globalTrack()->hitPattern().numberOfValidMuonHits();
        ev_.l_nMatchedStations[ev_.nl] = mu.numberOfMatchedStations();
      }
      ev_.nl++;    
      ev_.nleptons += ( isTight && mu.pt()>25); 
      nleptons += ( isTight && mu.pt()>25 && fabs(mu.eta())<2.5); 
    }
  
  // ELECTRON SELECTION: cf. https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2
  edm::Handle<edm::ValueMap<bool> > veto_id_decisions;
  iEvent.getByToken(eleVetoIdMapToken_ ,veto_id_decisions);
  edm::Handle<edm::ValueMap<bool> > tight_id_decisions;
  iEvent.getByToken(eleTightIdMapToken_ ,tight_id_decisions);
  edm::Handle<edm::ValueMap<bool> > medium_id_decisions;
  iEvent.getByToken(eleTightIdMapToken_ ,medium_id_decisions);
  edm::Handle<edm::ValueMap<vid::CutFlowResult> > tight_id_cutflow_data;
  iEvent.getByToken(eleTightIdFullInfoMapToken_,tight_id_cutflow_data);
  edm::Handle<edm::View<pat::Electron> >    electrons;
  iEvent.getByToken(electronToken_, electrons);    
  Int_t nele(0);
  for (const pat::Electron &el : *electrons) 
    {        
      const auto e = electrons->ptrAt(nele); 
      nele++;

      //kinematics cuts
      bool passPt(el.pt() > 15.0);
      bool passEta(fabs(el.eta()) < 2.5 && (fabs(el.superCluster()->eta()) < 1.4442 || fabs(el.superCluster()->eta()) > 1.5660));
      if(!passPt || !passEta) continue;
     
      //look up id decisions
      bool passVetoId = (*veto_id_decisions)[e];
      bool passTightId  = (*tight_id_decisions)[e];
      bool passMediumId  = (*medium_id_decisions)[e];
      vid::CutFlowResult fullCutFlowData = (*tight_id_cutflow_data)[e];
      bool passTightIdExceptIso(true);
      for(size_t icut = 0; icut<fullCutFlowData.cutFlowSize(); icut++)
 	{
	  if(icut!=9 && !fullCutFlowData.getCutResultByIndex(icut)) passTightIdExceptIso=false;
	}

      //save the electron
      const reco::GenParticle * gen=el.genLepton(); 
      ev_.isPromptFinalState[ev_.nl] = gen ? gen->isPromptFinalState() : false;
      ev_.isDirectPromptTauDecayProductFinalState[ev_.nl] = gen ? gen->isDirectPromptTauDecayProductFinalState() : false;
      ev_.l_id[ev_.nl]=11;
      ev_.l_g[ev_.nl]=-1;
      for(int ig=0; ig<ev_.ng; ig++)
	{
	  if(abs(ev_.g_id[ig])!=ev_.l_id[ev_.nl]) continue;
	  if(deltaR( el.eta(),el.phi(), ev_.g_eta[ig],ev_.g_phi[ig])>0.4) continue;
	  ev_.l_g[ev_.nl]=ig;
	  break;
	}	 
      ev_.l_pid[ev_.nl]=(passVetoId | (passTightId<<1) | (passTightIdExceptIso<<2) | (passMediumId<<3));
      ev_.l_charge[ev_.nl]=el.charge();
      ev_.l_pt[ev_.nl]=el.pt();
      ev_.l_eta[ev_.nl]=el.eta();
      ev_.l_phi[ev_.nl]=el.phi();
      ev_.l_mass[ev_.nl]=el.mass();
      ev_.l_miniIso[ev_.nl]=getMiniIsolation(pfcands,&el,0.05, 0.2, 10., false);
      //ev_.l_relIso[ev_.nl]=fullCutFlowData.getValueCutUpon(9);
      ev_.l_relIso[ev_.nl]=(el.chargedHadronIso()+ max(0., el.neutralHadronIso() + el.photonIso()  - 0.5*el.puChargedHadronIso()))/el.pt();     
      ev_.l_chargedHadronIso[ev_.nl]=el.chargedHadronIso();
      ev_.l_ip3d[ev_.nl] = -9999.;
      ev_.l_ip3dsig[ev_.nl] = -9999;
      if(el.gsfTrack().get())
	{
	  std::pair<bool,Measurement1D> ip3dRes = getImpactParameter<reco::GsfTrackRef>(el.gsfTrack(), primVtxRef, iSetup, true);
	  ev_.l_ip3d[ev_.nl]    = ip3dRes.second.value();
	  ev_.l_ip3dsig[ev_.nl] = ip3dRes.second.significance();
	}
      ev_.nl++;
      ev_.nleptons += (passTightIdExceptIso && el.pt()>25);
      nleptons += (passTightIdExceptIso && el.pt()>25 && fabs(el.eta())<2.5);
    }

  // JETS
  ev_.nj=0; 
  ev_.nkj=0;
  ev_.nkpf=0; 
  ev_.njpsi=0;
  ev_.nmeson=0;
  edm::Handle<edm::View<pat::Jet> > jets;
  iEvent.getByToken(jetToken_,jets);
  std::vector< std::pair<const reco::Candidate *,int> > clustCands;
  for(auto j = jets->begin();  j != jets->end(); ++j)
    {
      //kinematics
      if(j->pt()<20 || fabs(j->eta())>4.7) continue;
      
      // PF jet ID
      pat::strbitset retpf = pfjetIDLoose_.getBitTemplate();
      retpf.set(false);
      bool passLoose=pfjetIDLoose_( *j, retpf );
      if(!passLoose) continue;
     
      //save jet
      const reco::Candidate *genParton = j->genParton();
      ev_.j_area[ev_.nj]=j->jetArea();
      ev_.j_rawsf[ev_.nj]=j->correctedJet("Uncorrected").pt()/j->pt();
      ev_.j_pt[ev_.nj]=j->pt();
      ev_.j_p[ev_.nj]=j->p();
      ev_.j_pz[ev_.nj]=j->pz();
      ev_.j_mass[ev_.nj]=j->mass();
      ev_.j_eta[ev_.nj]=j->eta();
      ev_.j_phi[ev_.nj]=j->phi();
      ev_.j_g[ev_.nj]=-1;
      for(int ig=0; ig<ev_.ng; ig++)
	{
	  if(abs(ev_.g_id[ig])==11 || abs(ev_.g_id[ig])==13) continue;
	  if(deltaR( j->eta(),j->phi(), ev_.g_eta[ig],ev_.g_phi[ig])>0.4) continue;
	  ev_.j_g[ev_.nj]=ig;
	  break;
	}	 
      ev_.j_csv[ev_.nj]=j->bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags");
      ev_.j_cvsl[ev_.nj]=j->bDiscriminator("pfCombinedCvsLJetTags");
      ev_.j_cvsb[ev_.nj]=j->bDiscriminator("pfCombinedCvsBJetTags");
      ev_.j_vtxpx[ev_.nj]=j->userFloat("vtxPx");
      ev_.j_vtxpy[ev_.nj]=j->userFloat("vtxPy");
      ev_.j_vtxpz[ev_.nj]=j->userFloat("vtxPz");
      ev_.j_vtxmass[ev_.nj]=j->userFloat("vtxMass");
      ev_.j_vtxNtracks[ev_.nj]=j->userFloat("vtxNtracks");
      ev_.j_vtx3DVal[ev_.nj]=j->userFloat("vtx3DVal");
      ev_.j_vtx3DSig[ev_.nj]=j->userFloat("vtx3DSig");
      ev_.j_puid[ev_.nj]=j->userFloat("pileupJetId:fullDiscriminant");      
      ev_.j_flav[ev_.nj]=j->partonFlavour();
      ev_.j_hadflav[ev_.nj]=j->hadronFlavour();
      ev_.j_pid[ev_.nj]=genParton ? genParton->pdgId() : 0;

      ev_.j_pt_charged[ev_.nj]=0; //pT of charged tracks only
      ev_.j_p_charged[ev_.nj]=0; //p of charged tracks only
      ev_.j_pz_charged[ev_.nj]=0; //pz of charged tracks only
      //save all PF candidates central jet
      if(fabs(j->eta())>2.5) continue;
      ev_.j_pt_pf[ev_.nj]=0;
      ev_.j_p_pf[ev_.nj]=0;
      ev_.j_pz_pf[ev_.nj]=0;
      for(size_t ipf=0; ipf<j->numberOfDaughters(); ipf++)
	{
	  const reco::Candidate *pf=j->daughter(ipf);
	  clustCands.push_back(std::pair<const reco::Candidate *,int>(pf,ev_.nj));
          ev_.j_pt_charged[ev_.nj] += abs(pf->pt()*pf->charge()); //pT of charged tracks only (neutral particles weighted by 0)
          ev_.j_pt_pf[ev_.nj] += pf->pt(); //pT of charged tracks only (neutral particles weighted by 0)
          ev_.j_p_charged[ev_.nj] += abs(pf->p()*pf->charge()); //p of charged tracks only (neutral particles weighted by 0)
          ev_.j_p_pf[ev_.nj] += pf->p(); //p of charged tracks only (neutral particles weighted by 0)
          ev_.j_pz_charged[ev_.nj] += abs(pf->pz()*pf->charge()); //pz of charged tracks only (neutral particles weighted by 0)
          ev_.j_pz_pf[ev_.nj] += pf->pz(); //pz of charged tracks only (neutral particles weighted by 0)
	}

      KalmanAnalysis(iEvent,iSetup,*j,primVtx);
      ev_.nj++;
    }
      
  // MET
  ev_.nmet=2;
  for(int i=0; i<2; i++)
    {
      edm::Handle<pat::METCollection> mets;
      if(i==0) iEvent.getByToken(metToken_, mets);
      if(i==1) iEvent.getByToken(puppiMetToken_, mets);
      ev_.met_pt[i]=mets->at(0).pt();
      ev_.met_phi[i]=mets->at(0).phi();
    }

  //PF candidates
  //Skim
  //Save all muons an 6 hardest others
  //This block only counts the number of PF types
  ev_.npf=0;
  std::vector<pair<int,double>> pfCand;
  std::map<int,pair<int,int>> nPFJet; //pf_j, nMu, nKPi;
  std::pair<int,double> hard = std::pair<int,double>(-1,0); //save hadest PF
  for(auto pf = pfcands->begin();  pf != pfcands->end(); ++pf) {
    if(ev_.npf>=5000) continue;

    ev_.pf_j[ev_.npf] = -1;
    for(size_t i=0; i<clustCands.size(); i++) {
      if(pf->pdgId()!=clustCands[i].first->pdgId()) continue;
      if(deltaR(*pf,*(clustCands[i].first))>0.01) continue;
      ev_.pf_j[ev_.npf]=clustCands[i].second;
      break;
    }

    //extra requirements for unclustered PF candidates
    if(ev_.pf_j[ev_.npf]==-1) continue;

    //Only save hardest if not already saved
    //e.g. not a muon or not in the top 6 cands
    if(ev_.pf_pt[ev_.npf] > hard.second)
      hard = std::pair<int,double>(ev_.npf,ev_.pf_pt[ev_.npf]);

    std::pair<int,int> mupik = std::pair<int,int>(0,0);
    if(fabs(pf->pdgId())==13) mupik.first++;
    else if(fabs(pf->pdgId())==211) mupik.second++;
    else continue;

    //If this jet is already in the map, increment with the new paticles
    if(nPFJet.find(ev_.pf_j[ev_.npf]) != nPFJet.end()) {
      nPFJet[ev_.pf_j[ev_.npf]].first += mupik.first;
      nPFJet[ev_.pf_j[ev_.npf]].second += mupik.second;
      /*
      cout << "jet " << ev_.pf_j[ev_.npf] << endl;
      cout << "mu " << nPFJet[ev_.pf_j[ev_.npf]].first << endl;
      cout << "pi/K " << nPFJet[ev_.pf_j[ev_.npf]].second << endl;
      */
    }
    //Otherwise, add this jet to the map
    else nPFJet[ev_.pf_j[ev_.npf]] = mupik;
    pfCand.push_back(std::pair<int,double>(ev_.npf,pf->pt()));
    ev_.pf_jnpf[ev_.pf_j[ev_.npf]]++;
    if(pf->trackHighPurity()) ev_.pf_jnhppf[ev_.pf_j[ev_.npf]]++;
    ev_.npf++;

  }

  sort(pfCand.begin(),pfCand.end(), [](std::pair<int,double> i, std::pair<int,double> j) { return i.second > j.second ; } );
  pfCand.resize(6); //keep only 6 hardest non muon PF candidates

  //Now actually put the desired jets in the ntuple
  ev_.npf=0;
  for(auto pf = pfcands->begin();  pf != pfcands->end(); ++pf)
    {
      if(ev_.npf>=5000) continue;

      //int npf = ev_.npf;
      //if(!(fabs(pf->pdgId())==13 || std::any_of(pfCand.begin(), pfCand.end(),
      //                              [npf](std::pair<int,double>& elem) {return elem.first == npf;} ))) continue;

      ev_.pf_j[ev_.npf] = -1;
      for(size_t i=0; i<clustCands.size(); i++)
	{
	  if(pf->pdgId()!=clustCands[i].first->pdgId()) continue;
	  if(deltaR(*pf,*(clustCands[i].first))>0.01) continue;
	  ev_.pf_j[ev_.npf]=clustCands[i].second;
	  break;
	}
      if(ev_.pf_j[ev_.npf]==-1) continue;

      //only save jets with 2+ mu (+/- 13) or 2+ K/pi (+/- 211) or hardest pT track (if not a mu or K)
      if(ev_.pf_id[ev_.npf])
      if(ev_.pf_j[ev_.npf]>=0 &&
        (nPFJet[ev_.pf_j[ev_.npf]].first < 2 &&
         nPFJet[ev_.pf_j[ev_.npf]].second < 2) &&
         ev_.npf != hard.first) continue;
      
      //extra requirements for unclustered PF candidates
      //ONLY save mu/pi/K
      //if(fabs(ev_.pf_id[ev_.npf])!=13 && fabs(ev_.pf_id[ev_.npf])!=211) continue;
      if(ev_.pf_j[ev_.npf]==-1)
	{
	  if(pf->charge()==0) continue;
	  if(pf->fromPV()<2 && fabs(pf->pdgId())!=13) continue;
	  if(pf->pt()<0.5 || fabs(pf->eta())>2.5) continue;
	  if(pf->puppiWeight()<0.01) continue; // && fabs(pf->pdgId())!=13) continue;
	}
      if(ev_.pf_j[ev_.npf]==-1) continue; // skip unclustered PF candidates

      ev_.pf_fromPV[ev_.npf]   = pf->fromPV();
      ev_.pf_id[ev_.npf]       = pf->pdgId();
      ev_.pf_c[ev_.npf]        = pf->charge();
      ev_.pf_pt[ev_.npf]       = pf->pt();
      ev_.pf_eta[ev_.npf]      = pf->eta();
      ev_.pf_phi[ev_.npf]      = pf->phi();
      ev_.pf_m[ev_.npf]        = pf->mass();
      ev_.pf_puppiWgt[ev_.npf] = pf->puppiWeight();      

      ev_.pf_dxy[ev_.npf]      = pf->dxy(primVtx.position());
      ev_.pf_dxyE[ev_.npf]     = pf->dxyError();
      ev_.pf_dz[ev_.npf]       = pf->dz(primVtx.position());
      ev_.pf_dzE[ev_.npf]      = pf->dzError();

      ev_.pf_highPurity[ev_.npf] = pf->trackHighPurity();
      ev_.pf_quality[ev_.npf] = -1;
      ev_.pf_quality[ev_.npf] = pf->pseudoTrack().qualityMask();
      ev_.pf_muon[ev_.npf] = pf->isMuon();
      ev_.pf_standAloneMuon[ev_.npf] = pf->isStandAloneMuon();
      ev_.pf_globalMuon[ev_.npf] = pf->isGlobalMuon();
      ev_.pf_trackerMuon[ev_.npf] = pf->isTrackerMuon();

      ev_.pf_chi2ndof[ev_.npf] = pf->pseudoTrack().normalizedChi2();
      ev_.pf_vtxchi2ndof[ev_.npf] = pf->vertexNormalizedChi2();

      ev_.npf++;

    }
    return nleptons;
}

// ------------ method called for each event  ------------
void MiniAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  histContainer_["counter"]->Fill(0);

  //analyze the event
  int nleptons(0), ngleptons(0);
  if(!iEvent.isRealData()) ngleptons=genAnalysis(iEvent,iSetup);
  nleptons=recAnalysis(iEvent,iSetup);
  
  //save event if at least one lepton at gen or reco level
  //if((ev_.ngleptons==0 && ev_.nleptons==0) || !saveTree_) return;  
  if((ngleptons==0 && nleptons==0) || !saveTree_) return;  
  ev_.run     = iEvent.id().run();
  ev_.lumi    = iEvent.luminosityBlock();
  ev_.event   = iEvent.id().event(); 
  ev_.isData  = iEvent.isRealData();
  if(!savePF_) { ev_.ngpf=0; ev_.npf=0; }
  tree_->Fill();
  ev_ = {};
}


void MiniAnalyzer::KalmanAnalysis(const edm::Event& iEvent, const edm::EventSetup& iSetup, const pat::Jet &j, const reco::Vertex &primVtx)
{
  edm::ESHandle<TransientTrackBuilder> ttB;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder", ttB);
  TransientTrackBuilder thebuilder = *(ttB.product());

  // J/psi
  if (j.pt() < 30 || fabs(j.eta()) > 2.4) return;
  unsigned int ndau = j.numberOfDaughters();

  if(ndau < 2)return;
  for (unsigned int id1 = 0; id1 < ndau; ++id1) {// different way to combine w.r.t. D0
    for (unsigned int id2 = id1+1; id2 < ndau; ++id2) {
	
      if(id1 == id2) continue;
      
      const pat::PackedCandidate &pf1 = dynamic_cast<const pat::PackedCandidate &>(*j.daughter(id1));
      const pat::PackedCandidate &pf2 = dynamic_cast<const pat::PackedCandidate &>(*j.daughter(id2));
      
      if(pf1.pt()<1 || pf2.pt()<1) continue;
      // correct charge combination and not muons
      /*
      if(abs(pf1.pdgId()) != 13) continue;
      if(abs(pf2.pdgId()) != 13) continue;
      */
      if(pf1.pdgId()*pf2.pdgId() != -13*13) continue;
      
      if(pf1.pt() <3.0 || pf2.pt()<3.0) continue;
      if(fabs(pf1.eta()) > 2.4 || fabs(pf2.eta())> 2.4) continue;
	
      if(!pf1.trackHighPurity());
      if(!pf2.trackHighPurity());
      if(!pf1.isTrackerMuon() && !pf1.isGlobalMuon()) continue;
      if(!pf2.isTrackerMuon() && !pf2.isGlobalMuon()) continue;

      TLorentzVector p_track1, p_track2;
      const float gMassMu(0.1057);
      
      p_track1.SetPtEtaPhiM(pf1.pt(), pf1.eta(), pf1.phi(), gMassMu);
      p_track2.SetPtEtaPhiM(pf2.pt(), pf2.eta(), pf2.phi(), gMassMu);


      float mass12 = (p_track1+p_track2).M();
      
      if (mass12<2.5 || mass12>3.5) continue; 
      
      // vtx fitting
      const reco::Track* trk1 = &pf1.pseudoTrack();
      const reco::Track* trk2 = &pf2.pseudoTrack();

      reco::TransientTrack trTrack1 = thebuilder.build(*trk1);
      reco::TransientTrack trTrack2 = thebuilder.build(*trk2);

      vector<reco::TransientTrack> trTrackVec;
      trTrackVec.push_back(trTrack1);
      trTrackVec.push_back(trTrack2);

           
      KalmanVertexFitter kvf(true ); 
      TransientVertex tv;
      float vtxProb(0);
      try {
      //TransientVertex tv = kvf.vertex( trTrackVec );
      tv = kvf.vertex( trTrackVec );
      }
      catch (...) { std::cout << "Problem computing vertex" << std::endl; continue; }
      reco::Vertex fittedVertex = tv;
      float dxy = sqrt(pow(fittedVertex.x()-primVtx.x(),2) +
                       pow(fittedVertex.y()-primVtx.y(),2));
      float dxyE = sqrt(pow(fittedVertex.xError(),2) +
                        pow(primVtx.xError(),2) +
                        pow(fittedVertex.yError(),2) +
                        pow(primVtx.yError(),2));

      //*** L3D and sigmaL3D ****
      //http://www.phys.ufl.edu/~avery/fitting/lifetime.pdf eqn 10
      /*
      float sigmax = sqrt(pow(fittedVertex.xError(),2) + pow(primVtx.xError(),2));
      float sigmay = sqrt(pow(fittedVertex.yError(),2) + pow(primVtx.yError(),2));
      float sigmaz = sqrt(pow(fittedVertex.zError(),2) + pow(primVtx.zError(),2));

      float sigmaL3D = 1.0 / sqrt( pow( (jpsi.Px()/jpsi.M())/sigmax,2 ) +
                                   pow( (jpsi.Py()/jpsi.M())/sigmay,2 ) +
                                   pow( (jpsi.Pz()/jpsi.M())/sigmaz,2 ) );

      float L3D = (jpsi.Px()/jpsi.M()) * pow(sigmaL3D/sigmax,2) * (fittedVertex.x()-primVtx.x()) +
                  (jpsi.Py()/jpsi.M()) * pow(sigmaL3D/sigmay,2) * (fittedVertex.y()-primVtx.y()) +
                  (jpsi.Pz()/jpsi.M()) * pow(sigmaL3D/sigmaz,2) * (fittedVertex.z()-primVtx.z());
      */
      //*************************
      TLorentzVector jpsi = (p_track1+p_track2);
      std::vector<std::pair<float,float>> lsigma = {};
      std::pair<float,float> l3d = ctau(jpsi, fittedVertex, primVtx, lsigma);
      //BPH version
      /*
      TVector3 disp( fittedVertex.x() - primVtx.x(),
                     fittedVertex.y() - primVtx.y(),
                     fittedVertex.z() - primVtx.z() ); //changed z from 0
      TVector3 cmom( jpsi.Px(), jpsi.Py(), jpsi.Pz() ); //changed z from 0
      float cosAlpha = disp.Dot( cmom ) / ( disp.Perp() * cmom.Perp() );
      float mass = jpsi.M();
      AlgebraicVector3 vmom( jpsi.Px(), jpsi.Py(), jpsi.Pz() ); //changed z from 0
      VertexDistanceXY vdistXY;
      Measurement1D distXY = vdistXY.distance( fittedVertex, primVtx );
      double ctauPV = distXY.value() * cosAlpha * mass / cmom.Perp();
      GlobalError sve = fittedVertex.error();
      GlobalError pve = primVtx.error();
      AlgebraicSymMatrix33 vXYe = sve.matrix() + pve.matrix();
      double ctauErrPV = sqrt( ROOT::Math::Similarity( vmom, vXYe ) ) * mass /
                               cmom.Perp2();
      std::cout << L3D << " " << sigmaL3D << std::endl;
      std::cout << ctauPV << " " << ctauErrPV << std::endl;
      std::cout << L3D/ctauPV << " " << sigmaL3D/ctauErrPV << std::endl << std::endl;
      */

      vtxProb = TMath::Prob( tv.totalChiSquared(),tv.degreesOfFreedom() );

      ev_.k_j_pt[ev_.nj]=j.pt();
      ev_.k_j_eta[ev_.nj]=j.eta();
      ev_.k_j_phi[ev_.nj]=j.phi();
      ev_.k_j_mass[ev_.nj]=j.mass();
      //pf1
      ev_.k_j[ev_.nkpf]=ev_.nj;
      ev_.k_pf_id[ev_.nkpf]=pf1.pdgId();
      ev_.k_pf_pt[ev_.nkpf]=pf1.pt();
      ev_.k_pf_eta[ev_.nkpf]=pf1.eta();
      ev_.k_pf_phi[ev_.nkpf]=pf1.phi();
      ev_.k_pf_m[ev_.nkpf]=gMassMu;
      ev_.k_pf_tracker[ev_.nkpf]=pf1.isTrackerMuon();
      ev_.k_pf_global[ev_.nkpf]=pf1.isGlobalMuon();
      ev_.k_id[ev_.nkpf]=443;
      ev_.k_mass[ev_.nkpf]=mass12;
      ev_.k_chi2[ev_.nkpf]=tv.normalisedChiSquared();
      ev_.k_ndof[ev_.nkpf]=tv.degreesOfFreedom();
      ev_.k_vtxProb[ev_.nkpf]=vtxProb;
      ev_.k_dxy[ev_.nkpf]=dxy;
      ev_.k_dxyE[ev_.nkpf]=dxyE;
      ev_.k_l3d[ev_.nkpf]=l3d.first;
      ev_.k_sigmal3d[ev_.nkpf]=l3d.second;
      ev_.k_lx[ev_.nkpf]=lsigma[0].first;
      ev_.k_sigmax[ev_.nkpf]=lsigma[0].second;
      ev_.k_ly[ev_.nkpf]=lsigma[1].first;
      ev_.k_sigmay[ev_.nkpf]=lsigma[1].second;
      ev_.k_lz[ev_.nkpf]=lsigma[2].first;
      ev_.k_sigmaz[ev_.nkpf]=lsigma[2].second;
      ev_.nkpf++;
      //pf2
      ev_.k_j[ev_.nkpf]=ev_.nj;
      ev_.k_pf_id[ev_.nkpf]=pf2.pdgId();
      ev_.k_pf_pt[ev_.nkpf]=pf2.pt();
      ev_.k_pf_eta[ev_.nkpf]=pf2.eta();
      ev_.k_pf_phi[ev_.nkpf]=pf2.phi();
      ev_.k_pf_m[ev_.nkpf]=gMassMu;
      ev_.k_pf_tracker[ev_.nkpf]=pf1.isTrackerMuon();
      ev_.k_pf_global[ev_.nkpf]=pf1.isGlobalMuon();
      ev_.k_id[ev_.nkpf]=443;
      ev_.k_mass[ev_.nkpf]=mass12;
      ev_.k_chi2[ev_.nkpf]=tv.normalisedChiSquared();
      ev_.k_ndof[ev_.nkpf]=tv.degreesOfFreedom();
      ev_.k_vtxProb[ev_.nkpf]=vtxProb;
      ev_.k_dxy[ev_.nkpf]=dxy;
      ev_.k_dxyE[ev_.nkpf]=dxyE;
      ev_.k_l3d[ev_.nkpf]=l3d.first;
      ev_.k_sigmal3d[ev_.nkpf]=l3d.second;
      ev_.k_lx[ev_.nkpf]=lsigma[0].first;
      ev_.k_sigmax[ev_.nkpf]=lsigma[0].second;
      ev_.k_ly[ev_.nkpf]=lsigma[1].first;
      ev_.k_sigmay[ev_.nkpf]=lsigma[1].second;
      ev_.k_lz[ev_.nkpf]=lsigma[2].first;
      ev_.k_sigmaz[ev_.nkpf]=lsigma[2].second;
      ev_.nkpf++;

      ev_.njpsi++;
      ev_.nmeson++;

      //if (vtxProb<0.02) continue;
    }
  }// end of J/psi


  // D0  

  if(ndau < 4) return;
  unsigned int maxdau = min(ndau, (unsigned)4);
  /*
  for (unsigned int id1 = 0; id1 < ndau; ++id1) {
    for (unsigned int id2 = 0; id2 < ndau; ++id2) {
  */
  for (unsigned int id1 = 0; id1 < maxdau; ++id1) {
    for (unsigned int id2 = 0; id2 < maxdau; ++id2) {
	
      if(id1 == id2) continue;
	
      const pat::PackedCandidate &pf1 = dynamic_cast<const pat::PackedCandidate &>(*j.daughter(id1));
      const pat::PackedCandidate &pf2 = dynamic_cast<const pat::PackedCandidate &>(*j.daughter(id2));

      //if(pf1.pt()<1 || pf2.pt()<1) continue;
      // correct charge combination and not muons
      /*
      if(abs(pf1.pdgId()) != 211) continue;
      if(abs(pf2.pdgId()) != 211) continue;
      */
      if(pf1.pdgId()*pf2.pdgId() != -211*211) continue;
      
      if(pf1.pt()<1.0 || pf2.pt()<5.0) continue; //K pT > 1 GeV, pi pT > 5 GeV
      if(fabs(pf1.eta()) > 2.4 || fabs(pf2.eta())> 2.4) continue;
      
      TLorentzVector p_track1, p_track2;
      const float gMassK(0.4937);
      const float gMassPi(0.1396);
      
      p_track1.SetPtEtaPhiM(pf1.pt(), pf1.eta(), pf1.phi(), gMassK);
      p_track2.SetPtEtaPhiM(pf2.pt(), pf2.eta(), pf2.phi(), gMassPi);

      if(!pf1.trackHighPurity());
      if(!pf2.trackHighPurity());

      float mass12 = (p_track1+p_track2).M();
      
      if (mass12<1.7 || mass12>2.0) continue; 
      
      // vtx fitting
      const reco::Track* trk1 = &pf1.pseudoTrack();
      const reco::Track* trk2 = &pf2.pseudoTrack();
      
      reco::TransientTrack trTrack1 = thebuilder.build(*trk1);
      reco::TransientTrack trTrack2 = thebuilder.build(*trk2);
      
      vector<reco::TransientTrack> trTrackVec;
      trTrackVec.push_back(trTrack1);
      trTrackVec.push_back(trTrack2);

         
      KalmanVertexFitter kvf(true ); 
      TransientVertex tv;
      float vtxProb(0);
      try {
      //TransientVertex tv = kvf.vertex( trTrackVec );
      tv = kvf.vertex( trTrackVec );
      }
      catch (...) { std::cout << "Problem computing vertex" << std::endl; continue; }
      reco::Vertex fittedVertex = tv;
      float dxy = sqrt(pow(fittedVertex.x()-primVtx.x(),2) +
                       pow(fittedVertex.y()-primVtx.y(),2));
      float dxyE = sqrt(pow(fittedVertex.xError(),2) +
                        pow(primVtx.xError(),2) +
                        pow(fittedVertex.yError(),2) +
                        pow(primVtx.yError(),2));

      //*** L3D and sigmaL3D ****
      //http://www.phys.ufl.edu/~avery/fitting/lifetime.pdf eqn 10
      /*
      float sigmax = sqrt(pow(fittedVertex.xError(),2) + pow(primVtx.xError(),2));
      float sigmay = sqrt(pow(fittedVertex.yError(),2) + pow(primVtx.yError(),2));
      float sigmaz = sqrt(pow(fittedVertex.zError(),2) + pow(primVtx.zError(),2));

      float sigmaL3D = 1.0 / sqrt( pow( (jpsi.Px()/jpsi.M())/sigmax,2 ) +
                                   pow( (jpsi.Py()/jpsi.M())/sigmay,2 ) +
                                   pow( (jpsi.Pz()/jpsi.M())/sigmaz,2 ) );

      float L3D = (jpsi.Px()/jpsi.M()) * pow(sigmaL3D/sigmax,2) * (fittedVertex.x()-primVtx.x()) +
                  (jpsi.Py()/jpsi.M()) * pow(sigmaL3D/sigmay,2) * (fittedVertex.y()-primVtx.y()) +
                  (jpsi.Pz()/jpsi.M()) * pow(sigmaL3D/sigmaz,2) * (fittedVertex.z()-primVtx.z());
      */
      //*************************
      TLorentzVector d0 = (p_track1+p_track2);
      std::vector<std::pair<float,float>> lsigma = {};
      std::pair<float,float> l3d = ctau(d0, fittedVertex, primVtx, lsigma);
      //BPH version
      /*
      TVector3 disp( fittedVertex.x() - primVtx.x(),
                     fittedVertex.y() - primVtx.y(),
                     fittedVertex.z() - primVtx.z() ); //changed z from 0
      TVector3 cmom( jpsi.Px(), jpsi.Py(), jpsi.Pz() ); //changed z from 0
      float cosAlpha = disp.Dot( cmom ) / ( disp.Perp() * cmom.Perp() );
      float mass = jpsi.M();
      AlgebraicVector3 vmom( jpsi.Px(), jpsi.Py(), jpsi.Pz() ); //changed z from 0
      VertexDistanceXY vdistXY;
      Measurement1D distXY = vdistXY.distance( fittedVertex, primVtx );
      double ctauPV = distXY.value() * cosAlpha * mass / cmom.Perp();
      GlobalError sve = fittedVertex.error();
      GlobalError pve = primVtx.error();
      AlgebraicSymMatrix33 vXYe = sve.matrix() + pve.matrix();
      double ctauErrPV = sqrt( ROOT::Math::Similarity( vmom, vXYe ) ) * mass /
                               cmom.Perp2();
      std::cout << L3D << " " << sigmaL3D << std::endl;
      std::cout << ctauPV << " " << ctauErrPV << std::endl;
      std::cout << L3D/ctauPV << " " << sigmaL3D/ctauErrPV << std::endl << std::endl;
      */

      vtxProb = TMath::Prob( tv.totalChiSquared(),tv.degreesOfFreedom() );

      //Openening angle with respect to primary vertex
      GlobalPoint vtxPos(fittedVertex.x(), fittedVertex.y(), fittedVertex.z());
      std::auto_ptr<TrajectoryStateClosestToPoint> trajPlus;
      std::auto_ptr<TrajectoryStateClosestToPoint> trajMins;
      std::vector<reco::TransientTrack> theRefTracks;
      if (tv.hasRefittedTracks()) {
        theRefTracks = tv.refittedTracks();
      }
    
      reco::TransientTrack* thePositiveRefTrack = nullptr;
      reco::TransientTrack* theNegativeRefTrack = nullptr;
      for (std::vector<reco::TransientTrack>::iterator iTrack = theRefTracks.begin(); iTrack != theRefTracks.end(); ++iTrack) {
        if (iTrack->track().charge() > 0.) {
          thePositiveRefTrack = &*iTrack;
        } else if (iTrack->track().charge() < 0.) {
          theNegativeRefTrack = &*iTrack;
        }
      }
      if (thePositiveRefTrack == nullptr || theNegativeRefTrack == nullptr) continue;
      trajPlus.reset(new TrajectoryStateClosestToPoint(thePositiveRefTrack->trajectoryStateClosestToPoint(vtxPos)));
      trajMins.reset(new TrajectoryStateClosestToPoint(theNegativeRefTrack->trajectoryStateClosestToPoint(vtxPos)));
      
      if (trajPlus.get() == nullptr || trajMins.get() == nullptr || !trajPlus->isValid() || !trajMins->isValid()) continue;
      GlobalVector positiveP(trajPlus->momentum());
      GlobalVector negativeP(trajMins->momentum());
      GlobalVector totalP(positiveP + negativeP);
    
      double dx = fittedVertex.x()-(primVtx.position().x());
      double dy = fittedVertex.y()-(primVtx.position().y());
      double px = totalP.x();
      double py = totalP.y();
      double angleXY = (dx*px+dy*py)/(sqrt(dx*dx+dy*dy)*sqrt(px*px+py*py));
      //if(angleXY<0.99) continue; //Cut on opnening cos(angle) < 0.99

      ev_.k_j_pt[ev_.nj]=j.pt();
      ev_.k_j_eta[ev_.nj]=j.eta();
      ev_.k_j_phi[ev_.nj]=j.phi();
      ev_.k_j_mass[ev_.nj]=j.mass();
      //pf1
      ev_.k_j[ev_.nkpf]=ev_.nj;
      ev_.k_pf_ndau[ev_.nkpf]=id1;
      ev_.k_pf_id[ev_.nkpf]=pf1.pdgId();
      ev_.k_pf_pt[ev_.nkpf]=pf1.pt();
      ev_.k_pf_eta[ev_.nkpf]=pf1.eta();
      ev_.k_pf_phi[ev_.nkpf]=pf1.phi();
      ev_.k_pf_m[ev_.nkpf]=gMassK;
      ev_.k_pf_dxy[ev_.nkpf]=trk1->dxy(primVtx.position());
      ev_.k_pf_dxyE[ev_.nkpf]=trk1->dxyError();
      ev_.k_pf_dz[ev_.nkpf]=trk1->dz(primVtx.position());
      ev_.k_pf_dzE[ev_.nkpf]=trk1->dzError();
      ev_.k_id[ev_.nkpf]=421;
      ev_.k_mass[ev_.nkpf]=mass12;
      ev_.k_chi2[ev_.nkpf]=tv.normalisedChiSquared();
      ev_.k_ndof[ev_.nkpf]=tv.degreesOfFreedom();
      ev_.k_vtxProb[ev_.nkpf]=vtxProb;
      ev_.k_dxy[ev_.nkpf]=dxy;
      ev_.k_dxyE[ev_.nkpf]=dxyE;
      ev_.k_l3d[ev_.nkpf]=l3d.first;
      ev_.k_sigmal3d[ev_.nkpf]=l3d.second;
      ev_.k_lx[ev_.nkpf]=lsigma[0].first;
      ev_.k_sigmax[ev_.nkpf]=lsigma[0].second;
      ev_.k_ly[ev_.nkpf]=lsigma[1].first;
      ev_.k_sigmay[ev_.nkpf]=lsigma[1].second;
      ev_.k_lz[ev_.nkpf]=lsigma[2].first;
      ev_.k_sigmaz[ev_.nkpf]=lsigma[2].second;
      ev_.k_opang[ev_.nkpf]=angleXY;
      ev_.nkpf++;
      //pf2
      ev_.k_j[ev_.nkpf]=ev_.nj;
      ev_.k_pf_ndau[ev_.nkpf]=id2;
      ev_.k_pf_id[ev_.nkpf]=pf2.pdgId();
      ev_.k_pf_pt[ev_.nkpf]=pf2.pt();
      ev_.k_pf_eta[ev_.nkpf]=pf2.eta();
      ev_.k_pf_phi[ev_.nkpf]=pf2.phi();
      ev_.k_pf_m[ev_.nkpf]=gMassPi;
      ev_.k_pf_dxy[ev_.nkpf]=trk2->dxy(primVtx.position());
      ev_.k_pf_dxyE[ev_.nkpf]=trk2->dxyError();
      ev_.k_pf_dz[ev_.nkpf]=trk2->dz(primVtx.position());
      ev_.k_pf_dzE[ev_.nkpf]=trk2->dzError();
      ev_.k_id[ev_.nkpf]=421;
      ev_.k_mass[ev_.nkpf]=mass12;
      ev_.k_chi2[ev_.nkpf]=tv.normalisedChiSquared();
      ev_.k_ndof[ev_.nkpf]=tv.degreesOfFreedom();
      ev_.k_vtxProb[ev_.nkpf]=vtxProb;
      ev_.k_dxy[ev_.nkpf]=dxy;
      ev_.k_dxyE[ev_.nkpf]=dxyE;
      ev_.k_l3d[ev_.nkpf]=l3d.first;
      ev_.k_sigmal3d[ev_.nkpf]=l3d.second;
      ev_.k_lx[ev_.nkpf]=lsigma[0].first;
      ev_.k_sigmax[ev_.nkpf]=lsigma[0].second;
      ev_.k_ly[ev_.nkpf]=lsigma[1].first;
      ev_.k_sigmay[ev_.nkpf]=lsigma[1].second;
      ev_.k_lz[ev_.nkpf]=lsigma[2].first;
      ev_.k_sigmaz[ev_.nkpf]=lsigma[2].second;
      ev_.k_opang[ev_.nkpf]=angleXY;
      ev_.nkpf++;

      ev_.njpsi++;
      ev_.nmeson++;

      for (unsigned int id3 = 0; id3 < ndau; ++id3) {
	if(id1 == id3) continue;
	if(id2 == id3) continue;

	const pat::PackedCandidate &pf3 = dynamic_cast<const pat::PackedCandidate &>(*j.daughter(id3));
        if(abs(pf3.pdgId())!=13) continue;
        if(pf3.pt()<3.0) continue; //lep pT > 3 GeV
        if(pf1.pdgId()*pf3.pdgId() > 0) continue; //K and lep must have same sign
        //mu^- is POSITVE 13, PDGID and charge are flipped for leptons

        if(!pf3.trackHighPurity());
        if(!pf3.isTrackerMuon() && !pf3.isGlobalMuon()) continue;

        const float gMassMu(0.1057);

	if(fabs(pf3.eta()) > 2.4) continue;

        //pf3
        ev_.k_j[ev_.nkpf]=ev_.nj;
        ev_.k_pf_id[ev_.nkpf]=pf3.pdgId();
        ev_.k_pf_pt[ev_.nkpf]=pf3.pt();
        ev_.k_pf_eta[ev_.nkpf]=pf3.eta();
        ev_.k_pf_phi[ev_.nkpf]=pf3.phi();
        ev_.k_pf_m[ev_.nkpf]=gMassMu;
        ev_.k_id[ev_.nkpf]=42113;
        ev_.k_mass[ev_.nkpf]=mass12;
        ev_.k_chi2[ev_.nkpf]=tv.normalisedChiSquared();
        ev_.k_ndof[ev_.nkpf]=tv.degreesOfFreedom();
        ev_.k_vtxProb[ev_.nkpf]=vtxProb;
        ev_.k_dxy[ev_.nkpf]=dxy;
        ev_.k_dxyE[ev_.nkpf]=dxyE;
        ev_.k_l3d[ev_.nkpf]=l3d.first;
        ev_.k_sigmal3d[ev_.nkpf]=l3d.second;
        ev_.k_lx[ev_.nkpf]=lsigma[0].first;
        ev_.k_sigmax[ev_.nkpf]=lsigma[0].second;
        ev_.k_ly[ev_.nkpf]=lsigma[1].first;
        ev_.k_sigmay[ev_.nkpf]=lsigma[1].second;
        ev_.k_lz[ev_.nkpf]=lsigma[2].first;
        ev_.k_sigmaz[ev_.nkpf]=lsigma[2].second;
        ev_.nkpf++;

        ev_.njpsi++;
        ev_.nmeson++;

      }

      // D*
      for (unsigned int id3 = 0; id3 < ndau; ++id3) {
	if(id1 == id3) continue;
	if(id2 == id3) continue;

	const pat::PackedCandidate &pf3 = dynamic_cast<const pat::PackedCandidate &>(*j.daughter(id3));
        //if(abs(pf3.pdgId()) != 211) continue;
        if(!pf3.trackHighPurity());
        //if(pf1.pdgId()*pf3.pdgId() > 0) continue; //K and pi must have opposite sign
        if(pf1.pdgId()*pf3.pdgId() != -211*211) continue;

	if(fabs(pf3.eta()) > 2.4) continue;

	TLorentzVector p_track3;
	    
	p_track3.SetPtEtaPhiM(pf3.pt(), pf3.eta(), pf3.phi(), gMassPi);

        //if(!pf3.trackHighPurity());

	float mass123 = (p_track1+p_track2+p_track3).M();
	    
	if (mass123<1.9 || mass123>2.2) continue; 
	    
        //pf3
        ev_.k_j[ev_.nkpf]=ev_.nj;
        ev_.k_pf_id[ev_.nkpf]=pf3.pdgId();
        ev_.k_pf_pt[ev_.nkpf]=pf3.pt();
        ev_.k_pf_eta[ev_.nkpf]=pf3.eta();
        ev_.k_pf_phi[ev_.nkpf]=pf3.phi();
        ev_.k_pf_m[ev_.nkpf]=gMassPi;
        ev_.k_id[ev_.nkpf]=413;
        ev_.k_mass[ev_.nkpf]=mass123;
        ev_.k_chi2[ev_.nkpf]=tv.normalisedChiSquared();
        ev_.k_ndof[ev_.nkpf]=tv.degreesOfFreedom();
        ev_.k_vtxProb[ev_.nkpf]=vtxProb;
        ev_.k_dxy[ev_.nkpf]=dxy;
        ev_.k_dxyE[ev_.nkpf]=dxyE;
        ev_.k_l3d[ev_.nkpf]=l3d.first;
        ev_.k_sigmal3d[ev_.nkpf]=l3d.second;
        ev_.k_lx[ev_.nkpf]=lsigma[0].first;
        ev_.k_sigmax[ev_.nkpf]=lsigma[0].second;
        ev_.k_ly[ev_.nkpf]=lsigma[1].first;
        ev_.k_sigmay[ev_.nkpf]=lsigma[1].second;
        ev_.k_lz[ev_.nkpf]=lsigma[2].first;
        ev_.k_sigmaz[ev_.nkpf]=lsigma[2].second;
        ev_.nkpf++;

        ev_.njpsi++;
        ev_.nmeson++;

      } // end of D*
    }
  }// end of D0
  ev_.nkj++;

}

std::pair<float,float> MiniAnalyzer::ctau(const TLorentzVector &p4, const reco::Vertex &fittedVertex,
                                          const reco::Vertex &primVtx, std::vector<std::pair<float,float>> &lsigma) {
    //*** L3D and sigmaL3D ****
    //http://www.phys.ufl.edu/~avery/fitting/lifetime.pdf eqn 10
    float sigmax = sqrt(pow(fittedVertex.xError(),2) + pow(primVtx.xError(),2));
    float sigmay = sqrt(pow(fittedVertex.yError(),2) + pow(primVtx.yError(),2));
    float sigmaz = sqrt(pow(fittedVertex.zError(),2) + pow(primVtx.zError(),2));

    float sigmaL3D = 1.0 / sqrt( pow( (p4.Px()/p4.M())/sigmax,2 ) +
                                 pow( (p4.Py()/p4.M())/sigmay,2 ) +
                                 pow( (p4.Pz()/p4.M())/sigmaz,2 ) );

    float L3D = (p4.Px()/p4.M()) * pow(sigmaL3D/sigmax,2) * (fittedVertex.x()-primVtx.x()) +
                (p4.Py()/p4.M()) * pow(sigmaL3D/sigmay,2) * (fittedVertex.y()-primVtx.y()) +
                (p4.Pz()/p4.M()) * pow(sigmaL3D/sigmaz,2) * (fittedVertex.z()-primVtx.z());
    //*************************
    lsigma.push_back(std::pair<float,float>((p4.Px()/p4.M()) * pow(sigmaL3D/sigmax,2) * (fittedVertex.x()-primVtx.x()), sigmax));
    lsigma.push_back(std::pair<float,float>((p4.Py()/p4.M()) * pow(sigmaL3D/sigmay,2) * (fittedVertex.y()-primVtx.y()), sigmay));
    lsigma.push_back(std::pair<float,float>((p4.Pz()/p4.M()) * pow(sigmaL3D/sigmaz,2) * (fittedVertex.z()-primVtx.z()), sigmaz));
    
    return std::pair<float,float>(L3D,sigmaL3D);
  }

// ------------ method called once each job just before starting event loop  ------------
void 
MiniAnalyzer::beginJob(){
}

//
void 
MiniAnalyzer::endRun(const edm::Run& iRun,
		     const EventSetup& iSetup) 
{
  try{

    cout << "[MiniAnalyzer::endRun]" << endl;

    //save histograms with the counts per point in the gen scan (if available)
    for(auto it=genScanCounter_.begin(); it!=genScanCounter_.end(); it++)
      {
	TString key(it->first);
	float counts(it->second);
	histContainer_[key.Data()]=fs->make<TH1F>(key,key,1,0,1);
	histContainer_[key.Data()]->SetBinContent(1,counts);
      }

    edm::Handle<LHERunInfoProduct> lheruninfo;
    typedef std::vector<LHERunInfoProduct::Header>::const_iterator headers_const_iterator;
    iRun.getByToken(generatorRunInfoToken_, lheruninfo );
    //iRun.getByLabel( "externalLHEProducer", lheruninfo );

    LHERunInfoProduct myLHERunInfoProduct = *(lheruninfo.product());
    for (headers_const_iterator iter=myLHERunInfoProduct.headers_begin(); 
	 iter!=myLHERunInfoProduct.headers_end(); 
	 iter++)
      {
	std::string tag("generator");
	if(iter->tag()!="") tag+="_"+iter->tag();
	
	std::vector<std::string> lines = iter->lines();
	std::vector<std::string> prunedLines;
	for (unsigned int iLine = 0; iLine<lines.size(); iLine++) 
	  {
            //std::cout << lines.at(iLine); //uncomment to print all weight names
	    if(lines.at(iLine)=="") continue;
	    if(lines.at(iLine).find("weightgroup")!=std::string::npos) continue;
	    prunedLines.push_back( lines.at(iLine) );
	  }
	
	if(histContainer_.find(tag)==histContainer_.end()) 
	  {
	    std::cout << "Starting histo for " << tag << std::endl;
	    histContainer_[tag]=fs->make<TH1F>(tag.c_str(),tag.c_str(),prunedLines.size(),0,prunedLines.size());
	  }
	for (unsigned int iLine = 0; iLine<prunedLines.size(); iLine++) 
	  histContainer_[tag]->GetXaxis()->SetBinLabel(iLine+1,prunedLines.at(iLine).c_str());  
      }
  }
  catch(std::exception &e){
    std::cout << e.what() << endl
	      << "Failed to retrieve LHERunInfoProduct" << std::endl;
  }
}

//-------------
//cf. https://twiki.cern.ch/twiki/bin/view/CMS/MiniIsolationSUSY
float MiniAnalyzer::getMiniIsolation(edm::Handle<pat::PackedCandidateCollection> pfcands,
				     const reco::Candidate* ptcl,  
				     float r_iso_min, float r_iso_max, float kt_scale,
				     bool charged_only) 
{

    if (ptcl->pt()<5.) return 99999.;

    float deadcone_nh(0.), deadcone_ch(0.), deadcone_ph(0.), deadcone_pu(0.);
    if(ptcl->isElectron()) {
      if (fabs(ptcl->eta())>1.479) {deadcone_ch = 0.015; deadcone_pu = 0.015; deadcone_ph = 0.08;}
    } else if(ptcl->isMuon()) {
      deadcone_ch = 0.0001; deadcone_pu = 0.01; deadcone_ph = 0.01;deadcone_nh = 0.01;  
    } else {
      //deadcone_ch = 0.0001; deadcone_pu = 0.01; deadcone_ph = 0.01;deadcone_nh = 0.01; // maybe use muon cones??
    }

    float iso_nh(0.), iso_ch(0.), iso_ph(0.), iso_pu(0.);
    float ptThresh(0.5);
    if(ptcl->isElectron()) ptThresh = 0;
    float r_iso = (float)TMath::Max((float)r_iso_min,
				    (float)TMath::Min((float)r_iso_max, (float)(kt_scale/ptcl->pt())));
    for (const pat::PackedCandidate &pfc : *pfcands) {
      if (abs(pfc.pdgId())<7) continue;
      
      float dr = deltaR(pfc, *ptcl);
      if (dr > r_iso) continue;
      
      //////////////////  NEUTRALS  /////////////////////////
      if (pfc.charge()==0){
        if (pfc.pt()>ptThresh) {
          /////////// PHOTONS ////////////
          if (abs(pfc.pdgId())==22) {
            if(dr < deadcone_ph) continue;
            iso_ph += pfc.pt();
	    /////////// NEUTRAL HADRONS ////////////
          } else if (abs(pfc.pdgId())==130) {
            if(dr < deadcone_nh) continue;
            iso_nh += pfc.pt();
          }
        }
        //////////////////  CHARGED from PV  /////////////////////////
      } else if (pfc.fromPV()>1){
        if (abs(pfc.pdgId())==211) {
          if(dr < deadcone_ch) continue;
          iso_ch += pfc.pt();
        }
        //////////////////  CHARGED from PU  /////////////////////////
      } else {
        if (pfc.pt()>ptThresh){
          if(dr < deadcone_pu) continue;
          iso_pu += pfc.pt();
        }
      }
    }
    float iso(0.);
    if (charged_only){
      iso = iso_ch;
    } else {
      iso = iso_ph + iso_nh;
      iso -= 0.5*iso_pu;
      if (iso>0) iso += iso_ch;
      else iso = iso_ch;
    }
    iso = iso/ptcl->pt();

    return iso;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
MiniAnalyzer::endJob() 
{
  std::cout << "[MiniAnalyzer::endJob]" << endl;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
MiniAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(MiniAnalyzer);

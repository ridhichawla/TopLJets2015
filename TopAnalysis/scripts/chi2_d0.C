#include <RooFit.h>
#include "RooGlobalFunc.h"
#include "RooWorkspace.h"
#include "RooHistPdf.h"
#include "RooGenericPdf.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooAddPdf.h"
#include "RooIntegralMorph.h"
#include "RooChi2Var.h"
#include "RooMinuit.h"
#include "RooFitResult.h"
#include "RooAddition.h"
#include "RooArgSet.h"
#include "/afs/cern.ch/user/b/byates/TopAnalysis/LJets2015/2016/mtop/convert.h"
#include "/afs/cern.ch/user/b/byates/TopAnalysis/LJets2015/2016/mtop/tdr.C"

using namespace RooFit;
//TString name("");
float low(50.), high(50.),nom(0.8103),nerr(0.05);
bool TDR(1);
int epoch(0);
bool fullpt(0);
TString epoch_name[4] = {"_xb", "_BCDEFGH", "_BCDEF", "_GH"};

float N(1.);
TCanvas *c1 = setupCanvas();
TString report("");
TString json("\"d0\" :      [");
float chi2_d0_(TString tune="", TString name="", float num=0.855);
void run_chi2_d0(TString);
RooRealVar ptfrac;

void getHist(TString name, TString tune, TH1F *&data, TH1F *&mc, int epoch, bool norm=true) {
TString fname = TString::Format("sPlot/sPlot//TopMass_Data_sPlot_d0.root");
if(epoch>0) fname.ReplaceAll(".root",TString::Format("%d.root",epoch));
else if(epoch<0) fname.ReplaceAll(".root","_xb.root");
if(name.Contains("noHT")) fname.ReplaceAll("Data", "Data_noHT");
if(name.Contains("d0kk")) fname.ReplaceAll("Data", "Data_d0kk");
if(name.Contains("FSR")) fname.ReplaceAll("Data","FSR_toyData");
if(fullpt) fname.ReplaceAll(".root","_jpT.root");
std::cout << fname << std::endl;
TFile *fdata = TFile::Open(fname);
if(name.Length()==0)
fname = TString::Format("sPlot/sPlot//TopMass_172v5%s_sPlot_d0.root",tune.Data());
//fname = TString::Format("sPlot/sPlot//morph/TopMass_172v5%s_sPlot_d0.root",tune.Data());
else
fname = TString::Format("sPlot/sPlot//TopMass_%s%s_sPlot_d0.root",name.Data(),tune.Data());
if(epoch>0) fname.ReplaceAll(".root",TString::Format("%d.root",epoch));
else if(epoch<0) fname.ReplaceAll(".root","_xb.root");
if(fullpt) fname.ReplaceAll(".root","_jpT.root");
std::cout << fname << std::endl;
TFile *fmc = TFile::Open(fname);

RooPlot *tmp = nullptr;
RooBinning bins(0,1.1);
/*
std::vector<float> bin;
bin = {-0.025, 0.05, 0.125, 0.2, 0.275, 0.35, 0.425, 0.5, 0.575, 0.65, 0.725, 0.8, 0.875, 0.95, 1.0};
bin = {0, 0.075, 0.15, 0.225, 0.3, 0.375, 0.45, 0.525, 0.6, 0.675, 0.75, 0.825, 0.9, 0.975, 1.0};
*/
//bin = {0.025, 0.1, 0.175, 0.25, 0.325, 0.4, 0.475, 0.55, 0.625, 0.7, 0.775, 0.85, 0.925, 0.975, 1.0};
//bin = {0, 0.075, 0.15, 0.225, 0.3, 0.375, 0.45, 0.525, 0.6, 0.675, 0.75, 0.825, 0.9, 0.975, 1.0};
//bin = {-0.025, 0.025, 0.075, 0.125,  0.175, 0.225, 0.275, 0.325, 0.375, 0.425, 0.475, 0.525, 0.575, 0.625, 0.675, 0.725, 0.775, 0.825, 0.875, 0.925, 0.975, 1.0};
//bin = {0, 0.05, 0.1, 0.15,  0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.975, 1.0};
#include "/afs/cern.ch/user/b/byates/TopAnalysis/LJets2015/2016/mtop/param.h"
if(fullpt)
bin = {0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.7, 0.8, 0.9, 0.95, 1.0};
for(int i = 0; i < bin.size(); i++) {
 bins.addBoundary(bin[i]);
}
if(epoch<0) mc = (TH1F*)fmc->Get("ptfrac_signal_hist")->Clone();
else { tmp = (RooPlot*)fmc->Get("ptfrac_signal")->Clone(TString::Format("ptfrac_signal_mc%s%s",name.Data(),tune.Data()));
//tmp = ((RooWorkspace*)fmc->Get("w"))->var("ptfrac")->frame();
//((RooDataSet*)((RooWorkspace*)fmc->Get("w"))->data("sigData"))->plotOn(tmp, RooFit::Binning(bins), DataError(RooAbsData::SumW2));
if(tmp==nullptr) {std::cout << fname << std::endl; return;}
//mc = (TH1F*)convert(tmp, norm, 0, 1.1);
mc = (TH1F*)convert(tmp, norm, bin);
}
mc->SetDirectory(0);
mc->SetTitle(mc->GetName());
//mc->Rebin();
delete tmp;
if(epoch<0) data = (TH1F*)fdata->Get("ptfrac_signal_hist")->Clone();
else { tmp = (RooPlot*)fdata->Get("ptfrac_signal")->Clone(TString::Format("ptfrac_signal_data%s%s",name.Data(),tune.Data()));
//tmp = ((RooWorkspace*)fdata->Get("w"))->var("ptfrac")->frame();
//((RooDataSet*)((RooWorkspace*)fdata->Get("w"))->data("sigData"))->plotOn(tmp, RooFit::Binning(bins), DataError(RooAbsData::SumW2));
//data = (TH1F*)convert(tmp, norm, 0, 1.1);
data = (TH1F*)convert(tmp, norm, bin);
}
data->SetDirectory(0);
data->SetTitle(data->GetName());
//data->Rebin();
std::cout << data->GetTitle() << std::endl;
delete tmp;

fdata->Close();
fmc->Close();
delete fdata;
delete fmc;
}

void chi2_d0(int ep=epoch, TString samp="", bool jpT=false) {
  epoch = ep;
  fullpt = jpT;
  if(samp != "")
  run_chi2_d0(samp);
  else {
  run_chi2_d0("");
  run_chi2_d0("isr-down");
  run_chi2_d0("isr-up");
  run_chi2_d0("fsr-down");
  run_chi2_d0("fsr-up");
  run_chi2_d0("uedown");
  run_chi2_d0("ueup");
  //run_chi2_d0("erdON");
  run_chi2_d0("GluonMove_erdON");
  //run_chi2_d0("QCD_erdON");
  std::vector<TString> syst = {"LEP", "PU", "PI", "TRIGGER", "JER"};//, "JSF" }; //no lepton tracker efficiencies are used!
  //std::vector<TString> syst = {"TRK", "LEP", "PU", "PI", "TRIGGER", "JER" };
  for(auto & it : syst) {
    run_chi2_d0("down_"+it);
    run_chi2_d0("up_"+it);
  }
  run_chi2_d0("hdampdown");
  run_chi2_d0("hdampup");
  }
/*
  run_chi2_d0("tpt");
  run_chi2_d0("bkg");
  run_chi2_d0("bkg");
  run_chi2_d0("as117");
  run_chi2_d0("as119");
  */
  /*
  run_chi2_d0("m166v5");
  run_chi2_d0("m169v5");
  run_chi2_d0("m171v5");
  run_chi2_d0("m173v5");
  run_chi2_d0("m175v5");
  run_chi2_d0("m178v5");
  */
  /*
  */

  json += ("],");
  std::cout << json << std::endl;

}

void run_chi2_d0(TString name="") {
gROOT->Reset();
low=999.;
high=0;
//name=lname;
//gROOT->ProcessLine(".L convert.C");
//std::vector<TString> tune = {"", "_up", "_central", "_down"};
//std::vector<float> param = {0.855, 1.079, 0.8949, 0.6981};
/*
std::vector<TString> tune = {"_down", "", "_cccentral", "_central", "_up" };
std::vector<float> param = {0.755, 0.855, 0.875, 0.955, 1.055};
std::vector<TString> tune = {"_sdown", "_down", "_scentral", "", "_cccentral", "_central", "_up" };
std::vector<float> param = {0.655, 0.755, 0.825, 0.855, 0.875, 0.955, 1.055};
std::vector<TString> tune = {"_sdown", "_down", "_scentral", "", "_cccentral", "_925", "_central", "_up" };
std::vector<float> param = {0.655, 0.755, 0.825, 0.855, 0.875, 0.925, 0.955, 1.055};
*/
std::vector<TString> tune = {"_sdown", "_700", "_725", "_down", "_dddown", "_ddown", "_scentral", "", "_cccentral", "_ccentral", "_925", "_central", "_uuup", "_up", "_1125" };
std::vector<float> param = {0.655, 0.700, 0.725, 0.755, 0.775, 0.800, 0.825, 0.855, 0.875, 0.900, 0.925, 0.955, 0.975, 1.055, 1.125, 0.802};
/*
std::vector<TString> tune = {"_sdown", "_700", "_725", "_down", "_ddown", "_dddown", "_scentral", "", "_cccentral", "_ccentral", "_925", "_central", "_uuup", "_uup", "_up" };
std::vector<float> param = {0.655, 0.700, 0.725, 0.755, 0.775, 0.800, 0.825, 0.855, 0.875, 0.900, 0.925, 0.955, 0.975, 1.000, 1.055};
std::vector<TString> tune = {"_down", "_ddown", "_dddown", "", "_cccentral", "_ccentral", "_central", "_uuup", "_uup", "_up" };
std::vector<float> param = {0.755, 0.775, 0.800, 0.855, 0.875, 0.900, 0.955, 0.975, 1.000, 1.055};
*/
//TCanvas *c1 = new TCanvas("c1","c1");
//TCanvas *c1 = setupCanvas();
TH1F *chiTest = new TH1F("chiTest_"+name,TString::Format("chiTest_%s",name.Data()),400,0,2);
//TH1F *chiTest = new TH1F("chiTest_"+name,TString::Format("chiTest_%s",name.Data()),400,0,2);
chiTest->Sumw2();
//chiTest->SetDirectory(0);
for(auto & it : tune) {
  int pos = &it - &tune[0];
  if(param[pos]>1) continue;
  //if(param[pos]>1 && !name.Contains("fsr-down")) continue;
  //if(name == "up_PI" && param[pos]>0.975 && fullpt) continue; //remove up_PI 0.975 with large chi^2
  if(name == "GluonMove_erdON" && param[pos]==0.900 && epoch==0) continue; //remove up_PI 0.975 with large chi^2
  if(name == "GluonMove_erdON" && param[pos]<0.700 && epoch==1) continue; //remove up_PI 0.975 with large chi^2
  //if(name == "down_PU" && it=="_down" && epoch==2) continue; //remove down_UP 0.755 FIXME
  std::cout << "Running on tune: " << it << std::endl;
  float chi = chi2_d0_(it, name, param[pos]);
  if(chi<low) low = chi;
  if(chi>high) high = chi;
  chiTest->GetYaxis()->SetRangeUser(int(low)-1,int(high)+2);
  chiTest->SetBinContent(chiTest->FindBin(param[pos]),chi);
  //chiTest->SetBinError(chiTest->FindBin(param[pos]),sqrt(1./N));
}

chiTest->GetXaxis()->SetRangeUser(0.65,1.255);
chiTest->GetXaxis()->SetRangeUser(0.65,0.976);//1.055);
//chiTest->GetYaxis()->SetRangeUser(55,90);
chiTest->GetYaxis()->SetRangeUser(int(low)-1,int(high)+2);
//chiTest->GetYaxis()->SetRangeUser(200,220);
chiTest->SetMarkerStyle(20);
chiTest->Draw("p9");
std::cout << chiTest->GetName() << std::endl;
std::cout << chiTest->GetTitle() << std::endl;
tdr(chiTest,epoch);
/*
TLatex txt;
txt.SetNDC(true);
txt.SetTextFont(43);
txt.SetTextSize(16);
txt.SetTextAlign(12);
float iniy=0.95;// if self.wideCanvas else 0.95
float inix=0.12;// if noStack else 0.12
float lumi(35859.038);
if(lumi<100)
    txt.DrawLatex(inix,iniy,TString::Format("#bf{CMS} #it{Preliminary} %3.1f pb^{-1} (13 TeV)", (lumi) ));
else
    txt.DrawLatex(inix,iniy,TString::Format("#bf{CMS} #it{Preliminary} %3.1f fb^{-1} (13 TeV)", (lumi/1000.) ));
*/
((TF1*)(gROOT->GetFunction("pol3")))->SetParameters(1., 1., 1., 1.);
((TF1*)(gROOT->GetFunction("pol3")))->SetParameters(-88.3245, 1045.87, -2049.8, 1137.63);
 //TFitResultPtr fit = chiTest->Fit("pol3","FSEMQ","",0.6,1.055);
chiTest->Fit("pol3","FSMEQRW","",0.6,0.976);
//TFitResultPtr fit = chiTest->Fit("pol3","FSEMQ","",0.6,0.975);
//TFitResultPtr fit = chiTest->Fit("pol2","FSMEQ");
//TFitResultPtr fit = chiTest->Fit("pol2","FSMEQ","",0.8,1.0);
/*
float min = (-1)*fit->Parameter(1)/(2*fit->Parameter(2));
float chimin = fit->Parameter(0) + fit->Parameter(1)*min + fit->Parameter(2) * pow(min,2);
float err = (-1)*fit->Parameter(1) / (2 * fit->Parameter(2)) - sqrt(pow(fit->Parameter(1),2)
            - 4 * fit->Parameter(2) * (fit->Parameter(0) - chimin - 1)) / (2 * fit->Parameter(2));
*/
float min = chiTest->GetFunction("pol3")->GetMinimumX(0.6,1.075);
//float chimin = fit->Parameter(0) + fit->Parameter(1)*min + fit->Parameter(2) * pow(min,2) + fit->Parameter(3) * pow(min,3);
std::cout << std::endl << chiTest->GetFunction("pol3")->GetParameter(0) << std::endl << chiTest->GetFunction("pol3")->GetParameter(1) << std::endl << chiTest->GetFunction("pol3")->GetParameter(2) << std::endl << chiTest->GetFunction("pol3")->GetParameter(3) << std::endl << std::endl; 
float chimin = chiTest->GetFunction("pol3")->Eval(min);
float err = chiTest->GetFunction("pol3")->GetX(chimin+1,0.6,1.075);
if(name=="") { nom=min; nerr=err; }
report = Form("Minimum at x= %g +/- %0.6g",min, abs(min-err));
json += Form("%.4f, %.4f, ",min,abs(min-err));
//std::cout << "Minimum at x= " << min << " +/- " << abs(min - err) << std::endl;
std::cout << report << std::endl;
std::cout << "chi^2 at min= " << chimin << " " << TMath::Prob(chimin,10) << std::endl;
std::cout << "chi^2_min + 1 at x= " << err << std::endl;

TPaveText *pt = new TPaveText(0.12,0.85,0.3,0.65,"NDC"); //NB blNDC
pt->SetFillStyle(0);
pt->SetTextAlign(11);
pt->SetBorderSize(0);
pt->SetTextFont(42);
pt->SetTextSize(0.046);
TString text = TString::Format("r_{B}= %.4f +/- %.4f (stat)",min,abs(min-err));
if(name.Length() > 0)
  text += TString::Format(" %c %.4f (syst) +/- %.4f",(min<nom ? '-' : '+'), abs(nom-min), sqrt(abs(pow(nerr,2)-pow(abs(min-err),2))));
  //text += TString::Format(" %c %.4f (syst) +/- %.4f",(min<nom ? '-' : '+'), abs(nom-min), sqrt(abs(pow(0.0507584,2)-pow(abs(min-err),2))));
  //text += TString::Format(" %c %.4f (syst) +/- %.4f",(min<0.818905 ? '-' : '+'), abs(0.818905-min), sqrt(abs(pow(0.0507584,2)-pow(abs(min-err),2))));
pt->AddText(text);
if(!TDR) pt->Draw();
gStyle->SetOptStat(0);

if(name.Length()>0) name = "_" + name;
name += epoch_name[epoch+1];
if(fullpt) name += "_jpT";
c1->SaveAs("chi2_d0"+name+".pdf");
c1->SaveAs("chi2_d0"+name+".png");

delete pt;
//chiTest->Delete();
//delete chiTest;
//delete c1;
}

float chi2_d0_(TString tune="", TString name="", float num=0.855) {
TH1F *data, *data2, *mc, *mc2;
if(epoch!=0) {
getHist(name, tune, data, mc, epoch);
}
else {
getHist(name, tune, data, mc, 1, false);
getHist(name, tune, data2, mc2, 2, false);
data->Add(data2);
mc->Add(mc2);
delete data2;
delete mc2;
}
mc->GetXaxis()->SetTitle("D^{0} #it{p}_{T} / #Sigma #it{p}_{T}^{ch}");
data->GetXaxis()->SetTitle("D^{0} #it{p}_{T} / #Sigma #it{p}_{T}^{ch}");
setupPad()->cd();
tdr(mc, epoch);
if(fullpt) mc->GetXaxis()->SetTitle("D^{0} #it{p}_{T}/ jet #it{p_}{T}");
mc->Draw();
tdr(mc, epoch);
//if(epoch>=0) {
gStyle->SetOptStat(0);
TString namet(name);
data->SetMarkerStyle(20);
data->SetMarkerColor(kBlack);
data->SetLineColor(kBlack);
data->SetLineWidth(2);
if(num==0) num=0.855;
if(namet == "") namet = "172v5";
//if(tunet == "") tunet = "855";
c1->SaveAs(TString::Format("www/meson/morph/ptfrac/ptfrac_signal_%s_%d%s_d0%s.pdf",namet.Data(),int(num*1000), epoch_name[epoch+1].Data(), (fullpt ? "_jpT" : "")));
c1->SaveAs(TString::Format("www/meson/morph/ptfrac/ptfrac_signal_%s_%d%s_d0%s.png",namet.Data(),int(num*1000), epoch_name[epoch+1].Data(), (fullpt ? "_jpT" : "")));

std::cout << "" << std::endl;
if(namet=="172v5" && num > 0.825 && num < 0.875) {
data->SetTitle("");
data->GetXaxis()->SetRangeUser(0.,1.);
//data->GetYaxis()->SetRangeUser(0,0.145);
mc->SetMarkerStyle(20);
data->SetMarkerStyle(20);
data->SetMarkerColor(kBlack);
data->SetLineColor(kBlack);
data->SetLineWidth(2);
tdr(data, epoch);
data->Draw();
tdr(data, epoch);
//c1->SaveAs("www/meson/morph/ptfrac/ptfrac_signal_Data_BCDEFGH_d0.pdf");
//c1->SaveAs("www/meson/morph/ptfrac/ptfrac_signal_Data_BCDEFGH_d0.png");
c1->SaveAs(TString::Format("www/meson/morph/ptfrac/ptfrac_signal_Data%s_d0%s.pdf", epoch_name[epoch+1].Data(), (fullpt ? "_jpT" : "")));
c1->SaveAs(TString::Format("www/meson/morph/ptfrac/ptfrac_signal_Data%s_d0%s.png", epoch_name[epoch+1].Data(), (fullpt ? "_jpT" : "")));
}
//}

/*
if(tune=="" && name=="") {
TCanvas *c1 = setupCanvas();
TPad *p1 = setupPad();
p1->cd();
data->Draw();
gStyle->SetOptStat(0);
tdr(data,0);
data->SetMarkerStyle(20);
data->SetMarkerColor(kBlack);
data->SetLineColor(kBlack);
data->SetLineWidth(2);
c1->SaveAs("ptfrac_signal_Data_"+name+"d0.pdf");
c1->SaveAs("ptfrac_signal_Data_"+name+"d0.png");

}
*/

N = mc->Integral();
mc->Scale(1./mc->Integral());
data->Scale(1./data->Integral());

data->GetXaxis()->SetRangeUser(0.125,0.975);
mc->GetXaxis()->SetRangeUser(0.125,0.975);
data->GetXaxis()->SetRangeUser(0.2,0.975);
mc->GetXaxis()->SetRangeUser(0.2,0.975);
if(epoch<0 && 0) {
data->GetXaxis()->SetRangeUser(0.425,0.95);
mc->GetXaxis()->SetRangeUser(0.425,0.95);
}
if(fullpt) {
data->GetXaxis()->SetRangeUser(0.0,0.7);
mc->GetXaxis()->SetRangeUser(0.0,0.7);
}
data->SetLineColor(kBlack);
data->SetMarkerColor(kBlack);
data->SetMarkerStyle(20);
data->SetLineWidth(2);
mc->SetLineColor(kRed);
mc->SetMarkerColor(kRed);
mc->SetMarkerStyle(1);
mc->SetLineWidth(1);
mc->GetYaxis()->SetRangeUser(0.,.14);
data->GetYaxis()->SetRangeUser(0.,.14);
if(fullpt) {
mc->GetYaxis()->SetRangeUser(0.,.17);
data->GetYaxis()->SetRangeUser(0.,.17);
}
/*
int bin(mc->FindBin(0.425));
mc->SetBinContent(bin, 0);
data->SetBinContent(bin, 0);
*/
mc->Draw("hist");
tdr(mc, epoch);
mc->Draw("same e");
data->Draw("same");
if(num==0) num=0.855;
if(name=="") name="172v5";
TString mcvname(TString::Format("mcVdata_%s_%d_d0",name.Data(),(int)(num*1000)) + epoch_name[epoch+1]);
if(fullpt) mcvname += "_jpT";
c1->SaveAs(mcvname + ".pdf");
c1->SaveAs(mcvname + ".png");
//data->SetBinContent(data->FindBin(0.4), 0);
//mc->SetBinContent(data->FindBin(0.4), 0);
float chi2 = data->Chi2Test(mc, "CHI2 P WW");
/*
chi2 = 0.;
float sum1(0.);
float sum2(0.);
for(int i = 0; i < data->GetNbinsX(); i++) {
  float exp = mc->GetBinContent(i);
  float obs = data->GetBinContent(i);
  sum1 += obs;
  sum2 += exp;
  
}
std::cout << sum1 << " " << sum2 << std::endl;
float ndf = data->GetXaxis()->GetLast() - data->GetXaxis()->GetFirst();
for(int i = 0; i < data->GetNbinsX(); i++) {
  float cnt1 = data->GetBinContent(i);
  float cnt2 = mc->GetBinContent(i);
  float e1sq = pow(data->GetBinError(i),2);
  float e2sq = pow(mc->GetBinError(i),2);
  if (cnt1 * cnt1 == 0 && cnt2 * cnt2 == 0){  continue; }
  //if (cnt1 * cnt1 == 0 && cnt2 * cnt2 == 0){ ndf--;  continue; }
  float delta = sum1 * cnt2 - sum2 * cnt1;
  float sigma = sum1 * sum1 * e2sq + sum2 * sum2 *e1sq;
  chi2 += delta * delta / sigma;
}
std::cout << ndf << std::endl;
*/
std::cout << tune << " Chi2= " << chi2 << std::endl;
if(chi2<low) low = chi2;
if(chi2>high) high = chi2;

delete data;
delete mc;

return chi2;

}

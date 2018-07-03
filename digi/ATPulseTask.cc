#include "ATPulseTask.hh"
#include "ATHit.hh"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "ATVertexPropagator.h"
#include "ATPad.hh"
#include "ATSimulatedPoint.hh"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>

#include "TRandom.h"
#include "TMath.h"
#include "TF1.h"
#include "TH1.h"

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"
#define cGREEN "\033[1;32m"


ATPulseTask::ATPulseTask():FairTask("ATPulseTask"),
fEventID(0)
{

}

ATPulseTask::~ATPulseTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of ATPulseTask");
}

void
ATPulseTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of ATAvalancheTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (ATDigiPar*) rtdb->getContainer("ATDigiPar");
}

InitStatus
ATPulseTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of ATPulseTask");

  FairRootManager* ioman = FairRootManager::Instance();

fDriftedElectronArray = (TClonesArray *) ioman -> GetObject("ATSimulatedPoint");
  if (fDriftedElectronArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find fDriftedElectronArray array!");
    return kERROR;
  }
  fRawEventArray  = new TClonesArray("ATRawEvent", 100);        //!< Raw Event array(only one)
  ioman -> Register("ATRawEvent", "cbmsim", fRawEventArray, fIsPersistent);

  fGain = fPar->GetGain();
  std::cout<<"Gain: "<<fGain<<std::endl;

  // ***************Create ATTPC Pad Plane***************************
  TString scriptfile = "Lookup20150611.xml";
  TString dir = getenv("VMCWORKDIR");
  TString scriptdir = dir + "/scripts/"+ scriptfile;

      fMap = new AtTpcMap();
      fMap->GenerateATTPC();
      Bool_t MapIn = fMap->ParseXMLMap(scriptdir);
      fPadPlane = fMap->GetATTPCPlane();

  fEventID = 0;
  fRawEvent = NULL;

  /*TF1 ePulse(Form("ePulse_%i",iEvents),PadResponse,0,100,3);
  ePulse.SetParameter(0,fGain);
  ePulse.SetParameter(1,eTime);
  ePulse.SetParameter(2,tau);*/
  
  return kSUCCESS;
}

struct vPad{
  Double_t RawADC[512];
  Int_t padnumb;
};

Double_t PadResponse(Double_t *x, Double_t *par){
    return par[0] * TMath::Exp(-3.0*(x[0]-par[1])/par[2] )  * TMath::Sin((x[0]-par[1])/par[2]) * TMath::Power((x[0]-par[1])/par[2],3);  
}

void ATPulseTask::Exec(Option_t* option) {
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of ATPulseTask");
  
  Double_t tau  = 1; //shaping time (us)
  
  Int_t nMCPoints = fDriftedElectronArray->GetEntries();
  std::cout<<" ATPulseTask: Number of Points "<<nMCPoints<<std::endl;
  if(nMCPoints<10){
    fLogger->Warning(MESSAGE_ORIGIN, "Not enough hits for digitization! (<10)");
    return;
  }
  
  fRawEventArray -> Delete();
  fRawEvent = NULL;
  fRawEvent = (ATRawEvent*)fRawEventArray->ConstructedAt(0);
  fPadPlane->Reset(0); 
  Int_t size = fRawEventArray -> GetEntriesFast();
  
  //TFile output("test_output.root","recreate");
  
  //Distributing electron pulses among the pads
  for(Int_t iEvents = 0; iEvents<nMCPoints; iEvents++){//for every electron
    
    auto dElectron   = (ATSimulatedPoint*) fDriftedElectronArray -> At(iEvents); 
    auto coord       = dElectron->GetPosition();
    auto xElectron   = coord (0); //mm
    auto yElectron   = coord (1); //mm
    auto eTime       = coord (2); //us
    auto padNumber   = (int)fPadPlane->Fill(xElectron,yElectron) - 1;
    
    if(padNumber<0 || padNumber>10240) continue;
    //std::cout<<pBin<<"  "<<coord(0)<<"  "<<coord(1)<<"  "<<coord(2)<<"\n";
    
    std::map<Int_t,TH1F*>::iterator ite = electronsMap.find(padNumber);
    if(ite == electronsMap.end()){
      char buff[100];
      sprintf(buff,"%d",padNumber);
      eleAccumulated = new TH1F(buff,buff,512,0,60); //CHECK PARAMETERS FOR MAXIMUM DRIFT TIME 
      eleAccumulated->Fill(eTime);
      electronsMap[padNumber]=eleAccumulated;
    }
    else{
      eleAccumulated = (ite->second);
      eleAccumulated->Fill(eTime);
      electronsMap[padNumber]=eleAccumulated;
    } 
  }
  std::cout << "...End of collection of electrons in this event."<< std::endl;
  
  //output.cd();
  
  std::vector<Float_t> PadCenterCoord;
  std::map<Int_t,TH1F*>::iterator ite2 = electronsMap.begin();
  Int_t signal[512];
  TF1 *gain = new TF1("gain", "4*(x/[0])*pow(2.718, -2*(x/[0]))", 80, 120);//Polya distribution of gain
  gain->SetParameter(0, fGain);
  
  Double_t *thePar = new Double_t[3];
  while(ite2!=electronsMap.end()){
    for(Int_t kk=0;kk<512;kk++) signal[kk]=0;
    Int_t thePadNumber = (ite2->first);
    eleAccumulated = (ite2->second);
    TAxis* axis = eleAccumulated->GetXaxis();
    Double_t binWidth = axis->GetBinWidth(10);
    // Set Pad and add to event
    ATPad *pad = new ATPad();
    for(Int_t kk=0;kk<512;kk++){
      if(eleAccumulated->GetBinContent(kk)>0){
	for(Int_t nn=kk;nn<512;nn++){
	  Double_t binCenter = axis->GetBinCenter(kk);
	  signal[nn] += 1000*eleAccumulated->GetBinContent(kk)*pow(2.718,-3*((((((Double_t)nn)+0.5)*binWidth)-binCenter)/tau))*sin((((((Double_t)nn)+0.5)*binWidth)-binCenter)/tau)*pow((((((Double_t)nn)+0.5)*binWidth)-binCenter)/tau,3);
	  
	  //std::cout << "1: " << eleAccumulated->GetBinContent(kk) << "  2: "<< pow(2.718,-3*((((((Double_t)nn)+0.5)*binWidth)-binCenter)/tau))*sin(((((Double_t)nn)+0.5)*binWidth)-binCenter)/tau)*pow(((((Double_t)nn)+0.5)*binWidth)-binCenter)/tau,3)<< std::endl;
	}
      }
    }
    //for(Int_t kk=0;kk<512;kk++) std::cout << "ele["<< kk << "]"<< eleAccumulated->GetBinContent(kk) << std::endl;
    //for(Int_t kk=0;kk<512;kk++) std::cout << "signal["<< kk << "]"<< signal[kk] << std::endl;
    
    pad->SetPad(thePadNumber);
    PadCenterCoord = fMap->CalcPadCenter(thePadNumber);
    pad->SetValidPad(kTRUE);
    pad->SetPadXCoord(PadCenterCoord[0]);
    pad->SetPadYCoord(PadCenterCoord[1]);
    pad->SetPedestalSubtracted(kTRUE);
    Double_t g = gain->GetRandom();
    for(Int_t bin = 0; bin<512; bin++){
      pad->SetADC(bin,signal[bin]*g);
    }
    fRawEvent->SetPad(pad);
    fRawEvent->SetEventID(fEventID);
    //std::cout <<" Event "<<aux<<" "<<electronsMap.size()<< "*"<< std::flush;
    ite2++;
  }
  
  return;
}

ClassImp(ATPulseTask);

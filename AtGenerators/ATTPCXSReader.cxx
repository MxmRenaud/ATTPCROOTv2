// -------------------------------------------------------------------------
// -----               ATTPCXSReader implementation file               -----
// -----                Created 03/07/18  by H. Alvarez                -----
// -------------------------------------------------------------------------
#include "ATTPCXSReader.h"
#include "ATVertexPropagator.h"

#include "FairPrimaryGenerator.h"
#include "FairRootManager.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"

#include "FairIon.h"
#include "FairParticle.h"
#include "FairRunSim.h"
#include "FairRunAna.h"

#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TObjArray.h"
                    
#include "TRandom.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TGenPhaseSpace.h"
#include "TVirtualMC.h"
#include "TParticle.h"
#include "TClonesArray.h"
  
#include "FairRunSim.h"
#include "FairIon.h"
#include <iostream>
#include <fstream>
#include "TParticle.h"

#include "AtStack.h"
#include "AtTpcPoint.h"
#include "ATVertexPropagator.h"
#include "ATEulerTransformation.h"

using std::cout;
using std::endl;

Int_t ATTPCXSReader::fgNIon = 0;

ATTPCXSReader::ATTPCXSReader()
  :  fMult(0),
     fPx(0.), fPy(0.), fPz(0.),
     fVx(0.), fVy(0.), fVz(0.),
     fIon(0), fQ(0) {
  //  cout << "-W- ATTPCXSReader: "
  //      << " Please do not use the default constructor! " << endl;
}

ATTPCXSReader::ATTPCXSReader(const char* name,std::vector<Int_t> *z,std::vector<Int_t> *a,std::vector<Int_t> *q,
			     Int_t mult,
			     std::vector<Double_t> *px,std::vector<Double_t>* py,std::vector<Double_t> *pz,
			     std::vector<Double_t> *mass)
  : fMult(0),
    fPx(0.), fPy(0.), fPz(0.),
    fVx(0.), fVy(0.), fVz(0.),
    fIon(0),fPType(0.),fQ(0) {
  
  fgNIon++;
  fMult = mult;
  fIon.reserve(fMult);
  fIsResonnance = 2; 			//for compatibility with ReadEvent().
  fWhichDecayChannel = 99;	//for compatibility with ReadEvent().

  SetXSFileName();
  
  TString dir = getenv("VMCWORKDIR");
  TString XSFileName = dir+"/AtGenerators/"+fXSFileName;
  std::cout << " ATTPCXSReader: Opening input file " << XSFileName << std::endl;
  std::ifstream*  fInputXSFile = new std::ifstream(XSFileName);
  //std::ifstream*  fInputXSFile = new std::ifstream("/home/ayyadlim/fair_install/ATTPCROOTv2_HAP/AtGenerators/xs_22Mgp_fusionEvaporation.txt");
  if ( ! fInputXSFile->is_open() )
    Fatal("ATTPCXSReader","Cannot open input file.");
  
  std::cout << "ATTPCXSReader: opening PACE cross sections..." << std::endl;

  Double_t ene[31];
  Double_t xs[31][18];
  
  //fixed format
  for(Int_t energies=0;energies<31;energies++){
    *fInputXSFile >> ene[energies];
    //std::cout << ene[energies] << " ";
    for(Int_t xsvalues=0;xsvalues<18;xsvalues++){
      *fInputXSFile >> xs[energies][xsvalues];
      //std::cout << xs[energies][xsvalues]<< " ";
    }
    //std::cout << std::endl;
  }

  fh_pdf = new TH2F("pdf","pdf",31,0,31,18,0,180);
  for(Int_t energies=0;energies<31;energies++){
    for(Int_t xsvalues=0;xsvalues<18;xsvalues++){
      fh_pdf->SetBinContent(energies+1,xsvalues+1,xs[energies][xsvalues]);
    }
  }
  //fh_pdf->Write();
  
  TDatabasePDG* pdgDB = TDatabasePDG::Instance();
  TParticlePDG* kProtonPDG = pdgDB->GetParticle(2212);
  TParticle* kProton = new TParticle();
  kProton->SetPdgCode(2212);
  
  TParticle* kNeutron = new TParticle();
  kNeutron->SetPdgCode(2112);
  
  char buffer[20];
  for(Int_t i=0;i<fMult;i++){
    fPx.push_back( Double_t(a->at(i)) * px->at(i) );
    fPy.push_back( Double_t(a->at(i)) * py->at(i) );
    fPz.push_back( Double_t(a->at(i)) * pz->at(i) );
    Masses.push_back(mass->at(i)*1000.0);
    fWm.push_back( mass->at(i)*1000.0);
    FairIon *IonBuff;
    FairParticle *ParticleBuff;
    sprintf(buffer, "Product_Ion%d", i);
    
    if( a->at(i)!=1  ){  
      IonBuff = new FairIon(buffer, z->at(i), a->at(i), q->at(i),0.0,mass->at(i));
      ParticleBuff = new FairParticle("dummyPart",1,1,1.0,0,0.0,0.0);
      fPType.push_back("Ion");
      std::cout<<" Adding : "<<buffer<<std::endl;
    }
    else if( a->at(i)==1 && z->at(i)==1  ){
      IonBuff = new FairIon("dummyIon",50,50,0,0.0,100); // We fill the std::vector with a dummy ion
      ParticleBuff = new FairParticle(2212,kProton);
      fPType.push_back("Proton");
    }
    else if( a->at(i)==1 && z->at(i)==0  ){
      IonBuff = new FairIon("dummyIon",50,50,0,0.0,100); // We fill the std::vector with a dummy ion
      ParticleBuff = new FairParticle(2112,kNeutron);
      fPType.push_back("Neutron");
    }
    
    std::cout<<" Z "<<z->at(i)<<" A "<<a->at(i)<<std::endl;
    //std::cout<<buffer<<std::endl;
    fIon.push_back(IonBuff);
    fParticle.push_back(ParticleBuff);
  }

  FairRunSim* run = FairRunSim::Instance();
  if ( ! run ) {
    std::cout << "-E- FairIonGenerator: No FairRun instantised!" << std::endl;
    Fatal("FairIonGenerator", "No FairRun instantised!");
  }
  
  
  for(Int_t i=0;i<fMult;i++){  
    if(fPType.at(i)=="Ion"){
      std::cout<<" In position "<<i<<" adding an : "<<fPType.at(i)<<std::endl;
      run->AddNewIon(fIon.at(i));
      std::cout<<" fIon name :"<<fIon.at(i)->GetName()<<std::endl;
      std::cout<<" fParticle name :"<<fParticle.at(i)->GetName()<<std::endl;
    }
    else if(fPType.at(i)=="Proton"){
      std::cout<<" In position "<<i<<" adding an : "<<fPType.at(i)<<std::endl;
      //run->AddNewParticle(fParticle.at(i));
      std::cout<<" fIon name :"<<fIon.at(i)->GetName()<<std::endl;
      std::cout<<" fParticle name :"<<fParticle.at(i)->GetName()<<std::endl;
      std::cout<<fParticle.at(i)->GetName()<<std::endl;
    }
    else if(fPType.at(i)=="Neutron"){  
      std::cout<<" In position "<<i<<" adding an : "<<fPType.at(i)<<std::endl;
      //run->AddNewParticle(fParticle.at(i));
      std::cout<<" fIon name :"<<fIon.at(i)->GetName()<<std::endl;
      std::cout<<" fParticle name :"<<fParticle.at(i)->GetName()<<std::endl;
      std::cout<<fParticle.at(i)->GetName()<<std::endl;
    } 
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//fusion reaction version - EDIT: Maxime Renaud
ATTPCXSReader::ATTPCXSReader(const char* name, const char* nameProjTargReac, std::vector<Int_t> *z,std::vector<Int_t> *a,std::vector<Int_t> *q,
			     Int_t mult, std::vector<Double_t> *px,std::vector<Double_t>* py,std::vector<Double_t> *pz,
			     std::vector<Double_t> *mass)
  : fMult(0),
    fPx(0.), fPy(0.), fPz(0.),
    fVx(0.), fVy(0.), fVz(0.),
    fIon(0),fPType(0.),fQ(0) {
  
  fgNIon++;
  fMult = mult;    
  
  SetFusExFctName(nameProjTargReac);	//give the file name except for energy
  SetDecayChanFileName();
  SetPNEnergyRange(28.);				//give the "energy range (MeV)" of the proton & neutron evaporation distribution, aka number of lines in the file TODO move this to runfile !
  
  
  
  TString dir = getenv("VMCWORKDIR");
  TString FusExFctName = dir+"/AtGenerators/"+fFusExFctName+"-3hOmega.txt";//ex: 8B_40Ar-fusion-3hOmega.txt (Beam Energy, recoil energy, CrossSec, Excitation)
  std::cout << " ATTPCXSReader: Opening input file " << FusExFctName << std::endl;
  std::ifstream*  fInputFusExFctFile = new std::ifstream(FusExFctName);
  if ( ! fInputFusExFctFile->is_open() ) Fatal("ATTPCXSReader","Cannot open input file: fusion excitation.");
  
  TString DecayChannel = dir+"/AtGenerators/"+fDecayChanFileName;
  std::cout<< " ATTPCXReader: Opening input file " << DecayChannel <<std::endl;// energy, (probability, number of nucleons emitted, number of protons emitted)*4
  std::ifstream*  fInputDCFile = new std::ifstream(DecayChannel);
  if ( ! fInputDCFile->is_open() ) Fatal("ATTPCXSReader","Cannot open input file: decay channel.");
  
  TString ProtonDistribution; 				//for later; need beam energy first.
  std:: ifstream* fInputDistribXSFile; 	//for later; need beam energy first. //energy, for(deg=0;deg<180;deg+=10){numberOfProtonsEvaporated;}
  
  std::cout << "ATTPCXSReader: opening PACE cross sections..." << std::endl;
  
  

  Double_t ene[122];
  Double_t fusExFct[122][2];
  Double_t angularXS[fPNEnergyRange][18];
  Double_t decayChan[10][13];
  Double_t ExecEnerg;
  
  //!format is fixed! (Beam Energy, recoil energy, CrossSec, Excitation)
  for(Int_t energies=0;energies<122;energies++){//TODO change those values to accomodate other reactions
    *fInputFusExFctFile  >> ene[energies];
    for(Int_t recoilNfusExFct=0;recoilNcrossSec<2;recoilNcrossSec++){
      *fInputFusExFctFile >> fusExFct[energies][recoilNcrossSec];
    }
    *fInputFusExFctFile >> ExecEnerg;
  }
  
  for (Int_t energies=0;energies<20;energies++){//energy, (probability, number of nucleons emitted, number of protons emitted)*4
	  for (Int_t entries=0;entries<13;entries++){
		  *fInputDCFile >> decayChan[energies][entries];
	  }
  }

  //NOTE for later; first determine energy of reaction, use cross-sec graph to determine if you go through with the reaction, if you propagate, etc. 
  cs_fef = new TH1F("fusion ex fct","fusion ex fct",122,8,20);//TODO change those values to accomodate other reactions?
  for(Int_t energies=0;energies<122;energies++){
      cs_fef->SetBinContent(energies,fusExFct[energies][0]);
  }
 
 
  //Check if event happens: gas density * cross sec is probability for 1 particle
  Double_t temporaryDensVal = (1000/(39.948*0.9+16*0.1))*6.022*TMath::Power(10.,23.)*0.003*31330.8/(287.05*283.15); //1000/M*Na*depth(for 1 energy)*gasDensity [m^-2]
  Double_t whatCS = cs_fef->GetBinContent(cs_fef->GetBin(energ))*TMath::Power(10.,-21.); //[m^2]
  //selecting decay channel
  Double_t decayChanSelection = ((Double_t)rand())/RAND_MAX;
  Int_t decayEnergSelection = (Int_t) ((a->front())*((px->front())*(px->front())+(py->front())*(py->front())+(pz->front())*(pz->front()))/2);
  
  //Reaction or not ? Immediate decay (fIsResonnance) or not ?
  if (temporaryDensVal*whatCS > 1){//check validity of nmbers
	  std::cout<<"Somehow reaction probability was >= 1. Please check input."<<std::endl;
	  Fatal("ARRPCXSReader","Reaction probability greater than one");
  }
  else{//determine if compound stable or not, and if reaction happens at all
	  if(((double)rand())/RAND_MAX < temporaryDensVal*whatCS){
		  std::cout<<"Generating fusion event... Current energy "<< gATVP->GetEnergy()/*WARNING legit call ?*/ <<std::endl;
		  if (ExecEnerg > 0){//need to immediately decay
			  fIsResonnance = 1; 
		  }
		  else {//generate compound (then decay ? TODO maybe later)
			  fIsResonnance = 0; //will generate neutron and proton, but no energy or momentum
		  }
	  }
	  else if (decayEnergSelection < 8){//no reaction, fusion excitation fct not computed for lower energies(xs too low).
		  fWhichDecayChannel = 0;
	  }
	  else {//no reaction
		  fWhichDecayChannel = 0;
	  }
  }
  
  //select decay channel
  Double_t av = 0.01568; //In MeV, cst of mass frml
  Double_t as = 0.1856; //In MeV, cst of mass frml
  Double_t cc = 0.741; 	//In MeV, cst of mass frml
  Doule_t kappa = 1.79; //cst of mass frml
  Double_t tempA,tempZ,tempParity;			//for mass frml
  if (decayEnergSelection < 10){//WARNING TODO implement REAL condition; currently because fInputDCFile does not go lower than 10 MeV
	  fMult = decayChan[0][2]+2;
	  fProtonMult = decayChan[0][3];
	  fWhichDecayChannel = 1;
  }
  for (int i = 4;i > 1;i--){//select decay channel
	  if (decayChan[decayEnergSelection-10][i*3-2]>decayChanSelection){//from least likely channel to most. If likelyness higher than random number between [0;1], select that channel
		  fMult = decayChan[decayEnergSelection-10][i*3-1]+2;
		  fProtonMult = decayChan[decayEnergSelection-10][i*3];
		  fWhichDecayChannel = i;
	  }
	  else{//if not, select most likely channel
		  fMult = decayChan[decayEnergSelection-10][2]+2;
		  fProtonMult = decayChan[decayEnergSelection-10][3];
		  fWhichDecayChannel = 1;
	  }
	  
	  tempA = a->at(0)-fMult;
	  tempZ = z->at(0)-fProtonMult;
	  if(tempA%2 == 0 && tempZ%2 == 0){tempParity = -12./TMath::Sqrt(tempA);}
	  else if(tempA%2 != 0 && tempZ%2 != 0){tempParity = 12./TMath::Sqrt(tempA)-20./tempA;}
	  else{tempParity = 0;}
	  mass->at(2) = tempA-(-av*(1.-kappa*TMath::Power(1.-2.*tempZ/tempA,2))*tempA+as*(1.-kappa*TMath::Power(1-2*tempZ/tempA-,2))*TMath:Power(tempA,2./3.)+cc*tempZ*tempZ*TMath::Power(tempA,-1./3.)+tempParity)/1000.;
	  for (int i=3;i<fMult;i++){
		  a->push_back(1);
		  if(i-3>fProtonMult){z->push_back(1);}
		  else{z->push_back(0);}
	  }
	  
	  //Set file containing the relevant proton evaporation angular distribution WARNING TODO this is not right; (1) Is energy selecton at this stage ok ? (2) Proton distributions merged for all channels by PACE4, so what now ?
	  ProtonDistribution = dir+"/AtGenerators/FusionDistribution/"+std::Form("xs_48V-%i_fusionEvaporation.txt",decayEnergSelection);
	  std::cout<< " ATTPCXReader: Opening input file " << ProtonDistribution <<std::endl;
	  *fInputDistribXSFile = new std::ifstream(ProtonDistribution);
	  if ( ! fInputDistribXSFile->is_open() ) Fatal("ATTPCXSReader","Cannot open input file: fusionEvaporation.");
	  
	  //Now extract proton distribution and energy
	  for(Int_t energies=0;energies<fPNEnergyRange;energies++){
		  *fInputDistributionXSFile >> tempParity; //to be trashed, not useful afterwards
		  for(Int_t xsvalues=0;xsvalues<18;xsvalues++){
			  *fInputDistributionXSFile >> angularXS[energies][xsvalues];
		  }
	  }
	  
  }
  std::cout<<" Selected Decay Channel number "<<fWhichDecayChannel<<", shedding "<<fMult<<" nucleons, amongst which "<<fProtonMult<<" protons."<<std::endl<<std::endl;
  
  fh_pdf = new TH2F("pdf proton","pdf proton",fPNEnergyRange,0,fPNEnergyRange,18,0,180); //
  for(Int_t energies=0;energies<fPNEnergyRange;energies++){
    for(Int_t xsvalues=0;xsvalues<18;xsvalues++){
      fh_pdf->SetBinContent(energies,xsvalues+1,angularXS[energies][xsvalues]);
    }
  }
  
  
  TDatabasePDG* pdgDB = TDatabasePDG::Instance();
  TParticlePDG* kProtonPDG = pdgDB->GetParticle(2212);
  TParticle* kProton = new TParticle();
  kProton->SetPdgCode(2212);
  
  TParticle* kNeutron = new TParticle();
  kNeutron->SetPdgCode(2112);
  
  fIon.reserve(fMult);
  char buffer[20];
  for(Int_t i=0;i<fMult;i++){
    fPx.push_back( Double_t(a->at(i)) * px->at(i) );
    fPy.push_back( Double_t(a->at(i)) * py->at(i) );
    fPz.push_back( Double_t(a->at(i)) * pz->at(i) );
	 Masses.push_back(mass->at(i)*1000.0);
	 fWm.push_back( mass->at(i)*1000.0);
    FairIon *IonBuff;
    FairParticle *ParticleBuff;
    sprintf(buffer, "Product_Ion%d", i);
    
    if( a->at(i)!=1  ){  
      IonBuff = new FairIon(buffer, z->at(i), a->at(i), q->at(i),0.0,mass->at(i));
      ParticleBuff = new FairParticle("dummyPart",1,1,1.0,0,0.0,0.0);
      fPType.push_back("Ion");
      std::cout<<" Adding : "<<buffer<<std::endl;
    }
    else if( a->at(i)==1 && z->at(i)==1  ){
      IonBuff = new FairIon("dummyIon",50,50,0,0.0,100); // We fill the std::vector with a dummy ion
      ParticleBuff = new FairParticle(2212,kProton);
      fPType.push_back("Proton");
    }
    else if( a->at(i)==1 && z->at(i)==0  ){
      IonBuff = new FairIon("dummyIon",50,50,0,0.0,100); // We fill the std::vector with a dummy ion
      ParticleBuff = new FairParticle(2112,kNeutron);
      fPType.push_back("Neutron");
    }
    
    std::cout<<" Z "<<z->at(i)<<" A "<<a->at(i)<<std::endl;
    //std::cout<<buffer<<std::endl;
    fIon.push_back(IonBuff);
    fParticle.push_back(ParticleBuff);
  }

  FairRunSim* run = FairRunSim::Instance();
  if ( ! run ) {
    std::cout << "-E- FairIonGenerator: No FairRun instantised!" << std::endl;
    Fatal("FairIonGenerator", "No FairRun instantised!");
  }
  
  
  for(Int_t i=0;i<fMult;i++){  
    if(fPType.at(i)=="Ion"){
      std::cout<<" In position "<<i<<" adding an : "<<fPType.at(i)<<std::endl;
      run->AddNewIon(fIon.at(i));
      std::cout<<" fIon name :"<<fIon.at(i)->GetName()<<std::endl;
      std::cout<<" fParticle name :"<<fParticle.at(i)->GetName()<<std::endl;
    }
    else if(fPType.at(i)=="Proton"){
      std::cout<<" In position "<<i<<" adding an : "<<fPType.at(i)<<std::endl;
      //run->AddNewParticle(fParticle.at(i));
      std::cout<<" fIon name :"<<fIon.at(i)->GetName()<<std::endl;
      std::cout<<" fParticle name :"<<fParticle.at(i)->GetName()<<std::endl;
      std::cout<<fParticle.at(i)->GetName()<<std::endl;
    }
    else if(fPType.at(i)=="Neutron"){  
      std::cout<<" In position "<<i<<" adding an : "<<fPType.at(i)<<std::endl;
      //run->AddNewParticle(fParticle.at(i));
      std::cout<<" fIon name :"<<fIon.at(i)->GetName()<<std::endl;
      std::cout<<" fParticle name :"<<fParticle.at(i)->GetName()<<std::endl;
      std::cout<<fParticle.at(i)->GetName()<<std::endl;
    } 
  }
}


ATTPCXSReader::~ATTPCXSReader() {
  //
}


Bool_t ATTPCXSReader::ReadEvent(FairPrimaryGenerator* primGen) {
  const Double_t rad2deg = 0.0174532925;

  std::vector<Double_t> Ang; // Lab Angle of the products
  std::vector<Double_t> Ene; // Lab Energy of the products
  Ang.reserve(fMult-2);
  Ene.reserve(fMult-2);
  fPx.clear();
  fPy.clear();
  fPx.clear();
  fPx.resize(fMult); 
  fPy.resize(fMult);
  fPx.resize(fMult);

  AtStack* stack = (AtStack*) gMC->GetStack();
  
  fBeamEnergy = gATVP->GetEnergy();
  
  //Requires a non zero vertex energy and pre-generated Beam event (not punch through)
  if(gATVP->GetEnergy()>0 && gATVP->GetDecayEvtCnt()%2!=0 /*&& fWhichDecayChannel != 0*/){
    //proton parameters come from the XS PDF
    Double_t energyFromPDF, thetaFromPDF;
	 
	 fPxBeam = gATVP->GetPx();
    fPyBeam = gATVP->GetPy();
    fPzBeam = gATVP->GetPz();
    
	 Double_t eb;			//total (beam) projectile energy = projectile kinetic e + mass
	 Double_t pb2;			//(beam) projectile momentum squared  
	 Double_t pb;			//(beam)projectile momentum
	 //Double_t beta = pb/(eb+fWm.at(1));         // ??check beta of the projectile+target compound check??
    //Double_t gamma = 1.0/sqrt(1.0-beta*beta);
    Double_t e;			//total energy (beam+target)
    Double_t e_cm2;		//cm energy (beam+target) squared
    Double_t e_cm;		//cm energy (beam+target)
    Double_t t_cm;		//kinetic energy available for final products
    
    
    if (fIsResonnance == 2){ //original ATTPCXSReader() function
		 fh_pdf->GetRandom2(energyFromPDF,thetaFromPDF);
		 Ang.push_back(thetaFromPDF*TMath::Pi()/180); //set angle PROTON (in rad)
		 Ene.push_back(energyFromPDF); //set energy PROTON
		 
		 eb = fBeamEnergy+fWm.at(0);
		 pb2 = fBeamEnergy*fBeamEnergy+2.0*fBeamEnergy*fWm.at(0);
		 pb = TMath::Sqrt(pb2);            
		 e = fBeamEnergy+fWm.at(0)+fWm.at(1);   
		 e_cm2 = e*e-pb2;                       
		 e_cm = TMath::Sqrt(e_cm2);   
		 t_cm = e_cm-fWm.at(2)-fWm.at(3);
	 }
	 else if (fIsResonnance == 1){//TODO finish-able ?
		 fh_pdf->GetRandom2(energyFromPDF,thetaFromPDF);
		 Ang.push_back(thetaFromPDF*TMath::Pi()/180); //set angle PROTON (in rad) 
		 Ene.push_back(energyFromPDF); //set energy PROTON 
		 
		 eb = fBeamEnergy+fWm.at(0);
		 pb2 = fBeamEnergy*fBeamEnergy+2.0*fBeamEnergy*fWm.at(0);
		 pb = TMath::Sqrt(pb2);            
		 e = fBeamEnergy+fWm.at(0)+fWm.at(1);   
		 e_cm2 = e*e-pb2;                       
		 e_cm = TMath::Sqrt(e_cm2); 
		 t_cm = e_cm;
		 for (int i=2;i<fMult;i++){
			 t_cm -= fWm.at(i);
		 }
	 }
	 else if (fIsResonnance == 0){//complete fusion, no decay TODO add proper decay
		 energyFromPDF = 0.;
		 Ang.push_back(0.);//set proton angle (want no proton)
		 Ene.push_back(0.);//set proton energy (want no proton)
		 
		 eb = fBeamEnergy+fWm.at(0);
		 pb2 = fBeamEnergy*fBeamEnergy+2.0*fBeamEnergy*fWm.at(0);
		 pb = TMath::Sqrt(pb2);            
		 e = fBeamEnergy+fWm.at(0)+fWm.at(1);   
		 e_cm2 = e*e-pb2;                       
		 e_cm = TMath::Sqrt(e_cm2);
		 t_cm = e_cm-fWm.at(2)//(minus mass of the compound)
	 }
	 


    //HERE REMAINS THE CALCULATION OF THE ANGLE OF THE SCATTER AS A FUNCTION OF THE
    //ANGLE AND KINETIC ENERGY OF THE RECOIL (proton)
    //Double_t p_c
    //      tan theta_scatter = p_1*cos(theta1)*sin(theta1)/(p_A - p_1(cos(theta1))*cos(theta1));
    // with p_1(cos(theta1))=mA*E_1
      
    Double_t t_scatter = 0;
    if(t_cm - energyFromPDF>0)t_scatter = t_cm - energyFromPDF;
    else std::cout << "Kinetic Energy of scatter particle negative!" << std::endl;

    Double_t theta_scatter = 0.05; //TMath::Atan(*TMath::Sin(angleFromPDF)/)   (in rad)
    
    Ang.push_back(theta_scatter); //set angle ION   DUMMY FOR THE MOMENT!!!!!!!!!!!!!!! CHECK AND SOLVE
    Ene.push_back(t_scatter); //set energy ION

    gATVP->SetRecoilE(Ene.at(0));
    gATVP->SetRecoilA(Ang.at(0)*180.0/TMath::Pi()); //in degrees

    gATVP->SetScatterE(Ene.at(1));
    gATVP->SetScatterA(Ang.at(1)*180.0/TMath::Pi());
    
    fPx.at(0) = 0.0; fPy.at(0) = 0.0; fPz.at(0) = 0.0;
    fPx.at(1) = 0.0; fPy.at(1) = 0.0; fPz.at(1) = 0.0;

    Double_t phi1=0., phi2=0.;
    phi1 = 2*TMath::Pi() * gRandom->Uniform();         //flat probability in phi
    phi2 = phi1 + TMath::Pi();
    
    //To MeV for Euler Transformation
    TVector3 BeamPos(gATVP->GetPx()*1000,gATVP->GetPy()*1000,gATVP->GetPz()*1000); 

    TVector3 direction1 = TVector3(sin(Ang.at(0))*cos(phi1),
				   sin(Ang.at(0))*sin(phi1),
				   cos(Ang.at(0)));  //recoil
    
    TVector3 direction2 = TVector3(sin(Ang.at(1))*cos(phi2),
				   sin(Ang.at(1))*sin(phi2),
				   cos(Ang.at(1)));  //scatter (ion)

    Double_t p2_recoil = Ene.at(0)*Ene.at(0)+2.0*Ene.at(0)*fWm.at(3);
    Double_t p2_scatter= Ene.at(1)*Ene.at(1)+2.0*Ene.at(1)*fWm.at(2);
    if(p2_recoil<0 || p2_scatter<0)
      std::cout << "Particle momentum negative!" << std::endl;

    Double_t p_recoil =  TMath::Sqrt(p2_recoil); 
    Double_t p_scatter=  TMath::Sqrt(p2_scatter); 

    
    fPx.at(2) = p_scatter*direction2.X()/1000.0; // To GeV for FairRoot
    fPy.at(2) = p_scatter*direction2.Y()/1000.0;
    fPz.at(2) = p_scatter*direction2.Z()/1000.0;
    
    fPx.at(3) = p_recoil*direction1.X()/1000.0;
    fPy.at(3) = p_recoil*direction1.Y()/1000.0;
    fPz.at(3) = p_recoil*direction1.Z()/1000.0;
    
    // Particle transport begins here
    for(Int_t i=0; i<fMult; i++){
      TParticlePDG* thisPart;
      if(fPType.at(i)=="Ion") thisPart = TDatabasePDG::Instance()->GetParticle(fIon.at(i)->GetName());
      else if(fPType.at(i)=="Proton")  thisPart = TDatabasePDG::Instance()->GetParticle(fParticle.at(i)->GetName());
      else if(fPType.at(i)=="Neutron") thisPart = TDatabasePDG::Instance()->GetParticle(fParticle.at(i)->GetName());
      
      if ( ! thisPart ) {	
	if(fPType.at(i)=="Ion") std::cout << "-W- FairIonGenerator: Ion " << fIon.at(i)->GetName()<< " not found in database!" << std::endl;
	else if(fPType.at(i)=="Proton")  std::cout << "-W- FairIonGenerator: Particle " << fParticle.at(i)->GetName()<< " not found in database!" << std::endl;
	else if(fPType.at(i)=="Neutron") std::cout << "-W- FairIonGenerator: Particle " << fParticle.at(i)->GetName()<< " not found in database!" << std::endl;	
	return kFALSE;
      }  
      
      int pdgType = thisPart->PdgCode();
      
      // Propagate the vertex of the previous event
      fVx = gATVP->GetVx();
      fVy = gATVP->GetVy();
      fVz = gATVP->GetVz();
      
      // TODO: Dirty way to propagate only the products (0 and 1 are beam and target respectively)
      if(i>1 && gATVP->GetDecayEvtCnt() && pdgType!=1000500500 && fPType.at(i)=="Ion" ){
	std::cout << "-I- FairIonGenerator: Generating ions of type "
		  << fIon.at(i)->GetName() << " (PDG code " << pdgType << ")" << std::endl;
	std::cout << "    Momentum (" << fPx.at(i) << ", " << fPy.at(i) << ", " << fPz.at(i)
		  << ") Gev from vertex (" << fVx << ", " << fVy
		  << ", " << fVz << ") cm" << std::endl;
	primGen->AddTrack(pdgType, fPx.at(i), fPy.at(i), fPz.at(i), fVx, fVy, fVz);
      }
      else if(i>1 && gATVP->GetDecayEvtCnt() && pdgType==2212 && fPType.at(i)=="Proton" && fIsResonnance != 0){
	std::cout << "-I- FairIonGenerator: Generating ions of type "
		  << fParticle.at(i)->GetName() << " (PDG code " << pdgType << ")" << std::endl;
	std::cout << "    Momentum (" << fPx.at(i) << ", " << fPy.at(i) << ", " << fPz.at(i)
		  << ") Gev from vertex (" << fVx << ", " << fVy
		  << ", " << fVz << ") cm" << std::endl;
	primGen->AddTrack(pdgType, fPx.at(i), fPy.at(i), fPz.at(i), fVx, fVy, fVz);	
      }
      else if(i>1 && gATVP->GetDecayEvtCnt() && pdgType==2112 && fPType.at(i)=="Neutron" && fIsResonnance != 0){
	std::cout << "-I- FairIonGenerator: Generating ions of type "
		  << fParticle.at(i)->GetName() << " (PDG code " << pdgType << ")" << std::endl;
	std::cout << "    Momentum (" << fPx.at(i) << ", " << fPy.at(i) << ", " << fPz.at(i)
		  << ") Gev from vertex (" << fVx << ", " << fVy
		  << ", " << fVz << ") cm" << std::endl;
	primGen->AddTrack(pdgType, fPx.at(i), fPy.at(i), fPz.at(i), fVx, fVy, fVz);
      }
    }
  }//if residual energy > 0
 
  gATVP->IncDecayEvtCnt();  //TODO: Okay someone should put a more suitable name but we are on a hurry...
 
  return kTRUE;
}

ClassImp(ATTPCXSReader)

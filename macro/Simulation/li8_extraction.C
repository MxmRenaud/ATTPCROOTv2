#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include <iostream>
#include <fstream>


void li8_extraction(Int_t num_ev=20000)
{
    TString mcFileNameHead = "data/attpcsim_proto_li8-alph-n-ExE-oldgas";
    TString mcFileNameTail = ".root";
    TString mcFileName     = mcFileNameHead + mcFileNameTail;
    std::cout << " Analysis of simulation file  " << mcFileName << endl;
	
	TString outFileNameHead = "trialExtract-li8-alpha-n-ExE-oldgas";
	TString outFileNameTail = ".dat";
	TString outFileName 	= outFileNameHead + outFileNameTail;
	ofstream doutput(outFileName,ios::out);
	std::cout << " Outut file : " << outFileName << endl;
	

    AtTpcPoint* point = new AtTpcPoint();
    AtTpcPoint* point_forw = new AtTpcPoint();
    AtTpcPoint* point_back = new AtTpcPoint();
    TClonesArray *pointArray=0;
    TFile* file = new TFile(mcFileName.Data(),"READ");
    TTree* tree = (TTree*) file -> Get("cbmsim");


    tree = (TTree*) file -> Get("cbmsim");
    //TBranch *branch = tree->GetBranch("AtTpcPoint");
    tree -> SetBranchAddress("AtTpcPoint", &pointArray);
    Int_t nEvents = tree -> GetEntriesFast();

    Double_t vertex =0.0;

    if(nEvents>num_ev) nEvents=num_ev;

    for(Int_t iEvent=0; iEvent<nEvents; iEvent++)
    {
        TString VolName;
        tree->GetEvent(iEvent);
        // tree -> GetEntry(iEvent);
        Int_t n = pointArray -> GetEntries();
        std::cout << "n points: " << n << std::endl;
        std::cout<<" Event Number : "<<iEvent<<std::endl;
		//rad->Reset();

		// loop over points
        for(Int_t i=0; i<n; i++) {

            point = (AtTpcPoint*) pointArray -> At(i);

			doutput << iEvent << "\t" << point->GetTrackID()<<"\t"<<point->GetEIni()<<"\t"<<point->GetXIn()<<"\t"<<point->GetYIn()<<"\t"<<point->GetZIn()<<"\t"<<point->GetEnergyLoss()<<"\t"<<point->GetPxOut()<<"\t"<<point->GetPyOut()<<"\t"<<point->GetPzOut()<<endl;

			// BEAM: TrackID == 0

			// RECOIL: TrackID == 2
            
            // SCATTER: TrackID == 1
			
			//Energy Loss in GeV, Initial energy (== energy at track origin, if I understood things right) likely same.



        } // end of n points loop

    } // end of event loop


}

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include <iostream>
#include <fstream>


void b8_ana(Int_t num_ev=1000)
{

    TH2D *Eloss_vs_Range_Sca = new TH2D("EnergySca_vs_AngleSca","EnergySca_vs_AngleSca",40,0,40,1000,0,40);
    Eloss_vs_Range_Sca->SetMarkerStyle(20);
    Eloss_vs_Range_Sca->SetMarkerSize(0.5);

    TH2D *Eloss_vs_Range_Rec = new TH2D("EnergyRec_vs_AngleRec","EnergyRec_vs_AngleRec",180,0,180,1000,0,40);
    Eloss_vs_Range_Rec->SetMarkerStyle(20);
    Eloss_vs_Range_Rec->SetMarkerSize(0.5);
    Eloss_vs_Range_Rec->SetMarkerColor(2);

    TH2D *tracks = new TH2D("tracks","tracks",1000,-100,1000,1000,-300,300);
	TH2D *ElossRings = new TH2D("ElossRings","ElossRings",125,0,250,100,0,0.0004);

    TH2D *ESca_vs_ERec = new TH2D("ESca_vs_ERec","ESca_vs_ERec",100,0,40,100,0,40);
	TH2D *RangeSca_vs_RangeRec = new TH2D("RangeSca_vs_RangeRec","RangeSca_vs_RangeRec",100,0,40,100,0,40);
	
    TH1D *ICELoss = new TH1D("ICELoss","ICELoss",1000,0,100);

    TH1D *rad = new TH1D("rad","rad",2000,0,2000);

    TH2D *HKineRecoil =  new TH2D("ElossTime","ElossTime",700,0,700,100,0,0.0002);
    
    TH2D *Range_vs_Energy_Sca = new TH2D("Range_vs_Energy_Sca","Range_vs_Energy_Sca",100,0,700,100,0,50);
    TH2D *Range_vs_Energy_Rec = new TH2D("Range_vs_Energy_Rec","Range_vs_Energy_Rec",100,0,700,100,0,50);
    TH2D *Range_vs_Energy_Beam = new TH2D("Range_vs_Energy_Beam","Range_vs_Energy_Beam",100,0,700,100,0,50);

    TH3D *Vertex_vs_Angle_Rec = new TH3D("Vertex_vs_Angle_Rec","Vertex_vs_Angle_Rec",100,0,500,100,0,500,100,0,180);

    TH2D *Angle_sca_vs_Angle_rec = new TH2D("Angle_sca_vs_Angle_rec","Angle_sca_vs_Angle_rec",90,0,90,90,0,90);

	// Canvases
    TCanvas *c1 = new TCanvas();
    c1->Divide(2,2);
    c1->Draw();

    TCanvas *c2 = new TCanvas();
    c2->Divide(2,2);
    c2->Draw();

    TCanvas *c6 = new TCanvas();
    c6->Divide(1,3);
    c6-> Draw();

    TString mcFileNameHead = "data/attpcsim_proto_b8-fusbe";
    TString mcFileNameTail = ".root";
    TString mcFileName     = mcFileNameHead + mcFileNameTail;
    std:cout << " Analysis of simulation file  " << mcFileName << endl;

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
        Double_t energyLoss_sca=0.0;
        Double_t range_sca=0.0;
        Double_t energyLoss_rec=0.0;
        Double_t range_rec=0.0;
	    Double_t range_beam=0.0;
        Double_t BeamEnergyLoss_IC=0.0;
        Double_t EnergyRecoil = 0.0;
	    Double_t EnergyBeam=0.0;
        Double_t EnergySca=0.0;
        Double_t AngleRecoil = 0.0;
        Double_t AngleSca = 0.0;
		Double_t zpos = 0.0;
		Double_t xpos = 0.0;
        Double_t radius=0.0;
		Double_t radmean=0.0;
		Double_t thetamean=0.0;
		Double_t theta=0.0;
        Int_t n2=0;
		Int_t nrad=0;

        TString VolName;
        tree->GetEvent(iEvent);
        // tree -> GetEntry(iEvent);
        Int_t n = pointArray -> GetEntries();
        std:cout << "n points: " << n << std::endl;
        std::cout<<" Event Number : "<<iEvent<<std::endl;
		rad->Reset();

		// loop over points
        for(Int_t i=0; i<n; i++) {

            point = (AtTpcPoint*) pointArray -> At(i);
            VolName=point->GetVolName();
            //std::cout<<" Volume Name : "<<VolName<<std::endl;
            Int_t trackID = point -> GetTrackID();

			// ionization chamber energy loss
            if(trackID==0 && VolName.Contains("IC_")){

                BeamEnergyLoss_IC+=( point -> GetEnergyLoss() )*1000;//MeV


            }

		// beam
	      if(trackID==0 && VolName=="drift_volume"){
                vertex=point->GetZ()*10;
				range_beam=point -> GetLength()*10;//mm
				EnergyBeam=point -> GetEIni();
				std::cout << EnergyBeam*1000 << std::endl;
               // std::cout<<" Vertex : "<<vertex<<std::endl;
				tracks->Fill(vertex,point->GetXIn()*10); //TODO //TODO GetXIn()
				//std::cout << point->GetEnergyLoss() << std::endl;
				HKineRecoil->Fill(vertex,point->GetEnergyLoss());
				ElossRings->Fill(TMath::Sqrt(TMath::Power(point->GetXIn()*10.0,2.0) + TMath::Power(point->GetYIn()*10.0,2.0)), 
					point->GetEnergyLoss());
			//std::cout << EnergyBeam*1000 << std::endl;

            	}
			//std::cout << EnergyBeam*1000 << std::endl;

			// RECOIL: TrackID == 2
            if(trackID==2 && VolName=="drift_volume"){ 
		        n2++;
                range_rec = point -> GetLength()*10; //mm
                energyLoss_rec+=( point -> GetEnergyLoss() )*1000;//MeV
                EnergyRecoil= point->GetEIni();
                AngleRecoil= point->GetAIni();
		        zpos=point->GetZ()*10;
		        xpos=point->GetXIn()*10; //TODO GetX(z)
				tracks->Fill(zpos,xpos); //TODO
		
				// energy loss of alpha
				HKineRecoil->Fill(zpos,point->GetEnergyLoss());

				ElossRings->Fill(TMath::Sqrt(TMath::Power(point->GetXIn()*10.0,2.0) + TMath::Power(point->GetYIn()*10.0,2.0)), 
					point->GetEnergyLoss());				
            }	
            
            // SCATTER: TrackID == 1
            if(trackID==1 && VolName=="drift_volume"){ 
                range_sca = point -> GetLength()*10; //mm
                EnergySca = point ->GetEIni();
                energyLoss_sca+=( point -> GetEnergyLoss() )*1000; //MeV
                AngleSca= point->GetAIni();
                
                zpos=point->GetZ()*10;
		        xpos=point->GetXIn()*10; //TODO GetXIn()
				tracks->Fill(zpos,xpos);
				
				HKineRecoil->Fill(zpos,point->GetEnergyLoss());

				ElossRings->Fill(TMath::Sqrt(TMath::Power(point->GetXIn()*10.0,2.0) + TMath::Power(point->GetYIn()*10.0,2.0)), 
					point->GetEnergyLoss());

                // std::cout<<" Track ID : "<<trackID<<std::endl;
                // std::cout<<" Range_sca : "<<range_sca<<std::endl;
                //std::cout << " energyLoss_sca : " << energyLoss_sca << "\t" << EnergySca << std::endl;
            }


        } // end of n points loop


        //if(iEvent%2!=0){
            //std::cout<<" Range_rec : "<<range_rec<<std::endl;
            //std::cout<<" energyLoss_rec : "<<energyLoss_rec<<std::endl;
            Eloss_vs_Range_Rec->Fill(AngleRecoil,EnergyRecoil);

           // std::cout<<" Range_sca : "<<range_sca<<std::endl;
           // std::cout<<" energyLoss_sca : "<<energyLoss_sca<<std::endl;
            Eloss_vs_Range_Sca->Fill(AngleSca,EnergySca);
            
            //ELossRatio->Fill(energyLoss_sca/energyLoss_rec);

            //HKineRecoil->Fill(AngleRecoil,EnergyRecoil);
            Angle_sca_vs_Angle_rec->Fill(AngleRecoil,AngleSca);
            Vertex_vs_Angle_Rec->Fill(vertex,range_rec,AngleRecoil);
        //}
        
        ESca_vs_ERec->Fill(EnergySca,EnergyRecoil);
        
	Range_vs_Energy_Sca->Fill(range_sca,EnergySca);
    Range_vs_Energy_Rec->Fill(range_rec,EnergyRecoil);
	Range_vs_Energy_Beam->Fill(range_beam,EnergyBeam);

    } // end of event loop

    c1->cd(1);
    Eloss_vs_Range_Sca->Draw("scat");
    c1->cd(2);
    Eloss_vs_Range_Rec->Draw("scat");
    c1->cd(3);
    ESca_vs_ERec->Draw();
    c1->cd(4);
    Angle_sca_vs_Angle_rec->Draw("zcol");

    c2->cd(1);
    HKineRecoil->Draw("scat");
    c2->cd(2);
    tracks->Draw("col");
    c2->cd(3);
	ElossRings->Draw("scat");

      c6->cd(1);
      Range_vs_Energy_Sca->Draw();
      c6->cd(2);
      Range_vs_Energy_Rec->Draw();
      c6->cd(3);
      Range_vs_Energy_Beam->Draw();


}

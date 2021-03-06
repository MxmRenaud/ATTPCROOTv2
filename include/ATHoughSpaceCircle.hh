/*******************************************************************
* Daughter class for Circular Hough Space transformation           *
* Log: Class started 26-10-2015                                    *
* Author: Y. Ayyad (NSCL ayyadlim@nscl.msu.edu)                    *
********************************************************************/

#ifndef ATHOUGHSPACECIRCLE_H
#define ATHOUGHSPACECIRCLE_H

#include "ATHoughSpace.hh"
#include "TH2F.h"
// FairRoot classes
#include "FairRootManager.h"
#include "FairLogger.h"

#include "ATDigiPar.hh"
#include "ATRansac.hh"
#include "ATTrack.hh"


class ATHoughSpaceCircle : public ATHoughSpace{

      public:
	       ATHoughSpaceCircle();
        ~ATHoughSpaceCircle();

      	TH2F* GetHoughSpace(TString ProjPlane);
        void CalcHoughSpace(ATEvent* event,Bool_t YZplane,Bool_t XYplane, Bool_t XZplane);
        void CalcHoughSpace(ATEvent* event,TH2Poly* hPadPlane);
        void CalcHoughSpace(ATEvent* event,TH2Poly* hPadPlane,const multiarray& PadCoord); // Circular Hough+RANSAC+MCQ
        void CalcHoughSpace(ATProtoEvent* protoevent,Bool_t q1,Bool_t q2, Bool_t q3, Bool_t q4);
        void CalcMultiHoughSpace(ATEvent* event);
        void CalcHoughSpace(ATEvent* event);

        void SetELossFuncArray(std::vector<std::function<Double_t(Double_t,std::vector<Double_t>&)>>& func_array);

        Double_t GetXCenter() {return fXCenter;}
        Double_t GetYCenter() {return fYCenter;}
        std::vector<Double_t>* GetRadiusDist() {return fRadius;}
        std::vector<Int_t>* GetTimeStamp()  {return fTimeStamp;}
        std::vector<Double_t>* GetPhi()  {return fPhi;}
        std::vector<Double_t>* GetTheta()  {return fTheta;}
        std::vector<Double_t>* GetDl()  {return fDl;}

        Double_t  GetIniPhi()                {return fIniPhi;}
        Double_t  GetIniTheta()              {return fIniTheta;}
        Double_t  GetIniRadius()             {return fIniRadius;}
        Double_t  GetIniPhiRansac()          {return fIniPhiRansac;}
        Double_t  GetIniThetaRansac()        {return fIniThetaRansac;}
        Double_t  GetIniRadiusRansac()       {return fIniRadiusRansac;}

        ATHit*    GetIniHit()                {return fIniHit;}
        ATHit*    GetIniHitRansac()          {return fIniHitRansac;}

        Double_t* GetInitialParameters()     {return fParameter;}

        std::vector<Double_t> GetPosXMin()   {return fPosXmin;}
        std::vector<Double_t> GetPosYMin()   {return fPosYmin;}
        std::vector<Double_t> GetPosZMin()   {return fPosZmin;}
        std::vector<Double_t> GetPosXExp()   {return fPosXexp;}
        std::vector<Double_t> GetPosYExp()   {return fPosYexp;}
        std::vector<Double_t> GetPosZExp()   {return fPosZexp;}
        std::vector<Double_t> GetPosXInt()   {return fPosXinter;}
        std::vector<Double_t> GetPosYInt()   {return fPosYinter;}
        std::vector<Double_t> GetPosZInt()   {return fPosZinter;}
        std::vector<Double_t> GetPosXBack()  {return fPosXBack;}
        std::vector<Double_t> GetPosYBack()  {return fPosYBack;}
        std::vector<Double_t> GetPosZBack()  {return fPosZBack;}

        void FillHoughMap(Double_t ang, Double_t dist);
        std::pair<Double_t,Double_t> GetHoughPar();
        std::vector<Double_t>  GetRansacPar();
        // Test function
        static Double_t GetEloss(Double_t c0,std::vector<Double_t>& par);
        void SetElossParameters(const std::vector<Double_t>& par);



        //void SetThreshold(Double_t value);

        struct FitPar
        {
          Double_t sThetaMin;
          Double_t sEnerMin;
          TVector3 sPosMin;
          Double_t sBrhoMin;
          Double_t sBMin;
          Double_t sPhiMin;
          Double_t sChi2Min;
          TVector3 sVertexPos;
    			Double_t sVertexEner;
          Double_t sMinDistAppr;
          Int_t    sNumMCPoint;
          Double_t sNormChi2;

        };

        FitPar FitParameters;
        TH2F *HistHoughXY;

        std::vector<ATHit>* fClusteredHits;
        std::vector<Double_t> fElossPar;


      protected:

        std::pair<Double_t,Double_t> CalHoughParameters(TH2F* hist); //TODO: implement it as virtual member in the base class
        std::pair<Double_t,Double_t> CalHoughParameters(); //Overloaded version for std::map
        Int_t GetTBMult(Int_t TB,std::vector<ATHit> *harray,Int_t index);
        ATTrack& FindCandidateTrack(const std::vector<ATTrack*>& tracks);
        void GetDeviation(std::vector<ATHit>* hits,Double_t& _x_dev,Double_t& _y_dev);
        void GetTBDeviation(std::vector<ATHit>* hits, Float_t& maxdev_ratio,Double_t& mean_dev_x,Double_t& mean_dev_y);
        Int_t GetDensityOfHits(std::vector<ATHit>* hits,Int_t index, Int_t tb_range); //Gets number of hits around a time bucket
        std::vector<ATHit> GetTBHitArray(Int_t TB,std::vector<ATHit> *harray);

        //Double_t fThreshold;
        std::map<std::vector<Float_t>,Int_t> HoughMap_XY;
        std::vector<Float_t> HoughPar;
        std::vector<Double_t> *fRadius;
        std::vector<Int_t> *fTimeStamp;
        std::vector<Double_t> *fPhi;
        std::vector<Double_t> *fTheta;
        std::vector<Double_t> *fDl;

        TH2F *HistHoughAux;
        Double_t fXCenter;
        Double_t fYCenter;

        ATHit   *fIniHit;
        ATHit   *fIniHitRansac;
        Double_t fIniRadius;
        Double_t fIniRadiusRansac;
        Double_t fIniPhi;
        Double_t fIniPhiRansac;
        Double_t fIniTheta;
        Double_t fIniThetaRansac;
        Int_t fIniTS;
        Int_t fIniHitID;
        Double_t fDriftVelocity;
        Int_t fTBTime;

        Bool_t kDebug;
        Bool_t kHough;

        std::vector<Double_t> fPosXmin;
        std::vector<Double_t> fPosYmin;
        std::vector<Double_t> fPosZmin;
        std::vector<Double_t> fPosXexp;
        std::vector<Double_t> fPosYexp;
        std::vector<Double_t> fPosZexp;
        std::vector<Double_t> fPosXinter;
        std::vector<Double_t> fPosYinter;
        std::vector<Double_t> fPosZinter;
        std::vector<Double_t> fPosXBack;
        std::vector<Double_t> fPosYBack;
        std::vector<Double_t> fPosZBack;



        std::map<ULong64_t,Int_t> HoughMap; //8 byte for the key, unsigned, no negative distance in Linear Hough space is expected for each quadrant (Radius and Z are the vairbales)
        std::vector<ULong64_t> HoughMapKey;
        std::pair<Double_t,Double_t> fHoughLinePar;
        std::vector<Double_t> fRansacLinePar;

        Double_t fStdDeviationLimit;

        Double_t fParameter[8];
        std::vector<std::function<Double_t(Double_t,std::vector<Double_t>&)>> fEloss_func_array;//!

        struct maxpersecond
        {
            template <typename Lhs, typename Rhs>
              bool operator()(const Lhs& lhs, const Rhs& rhs) const
                {
                  return lhs.second < rhs.second;
                }
        };


        ClassDef(ATHoughSpaceCircle, 3);

};

#endif

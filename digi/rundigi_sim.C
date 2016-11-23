void rundigi_sim
(TString mcFile = "~/fair_install_ROOT6/ATTPCROOTv2_October2016/digi/data/attpcsim_2.root",
 TString digiParFile = "/home/attpc/fair_install_ROOT6/ATTPCROOTv2/parameters/AT.digi.par",
 TString gasParFile = "/home/attpc/fair_install_ROOT6/ATTPCROOTv2/parameters/Gas.digi.par")
{


  // -----   Timer   --------------------------------------------------------
 TStopwatch timer;
 timer.Start();
 // ------------------------------------------------------------------------
  // __ Run ____________________________________________
  FairRunAna* fRun = new FairRunAna();
              fRun -> SetInputFile(mcFile);
              fRun -> SetOutputFile("~/fair_install_ROOT6/ATTPCROOTv2_October2016/macro/Unpack_GETDecoder2/output.root");


  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
              FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
              parIo1 -> open(digiParFile.Data(), "in");
              rtdb -> setSecondInput(parIo1);
              //FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
              //parIo2 -> open(gasParFile.Data(), "in");
              //rtdb -> setFirstInput(parIo2);

  // __ AT digi tasks___________________________________

  ATClusterizeTask* clusterizer = new ATClusterizeTask();
                clusterizer -> SetPersistence(kFALSE);

  ATPulseTask* pulse = new ATPulseTask();
      pulse -> SetPersistence(kTRUE);

      ATPSATask *psaTask = new ATPSATask();
      psaTask -> SetPersistence(kTRUE);
      psaTask -> SetThreshold(20);
      psaTask -> SetPSAMode(1); //NB: 1 is ATTPC - 2 is pATTPC
      //psaTask -> SetPeakFinder(); //NB: Use either peak finder of maximum finder but not both at the same time
      psaTask -> SetMaxFinder();
      psaTask -> SetBaseCorrection(kTRUE); //Directly apply the base line correction to the pulse amplitude to correct for the mesh induction. If false the correction is just saved
      psaTask -> SetTimeCorrection(kFALSE); //Interpolation around the maximum of the signal peak

      ATRansacTask *RansacTask = new ATRansacTask();
    	RansacTask->SetPersistence(kTRUE);
      RansacTask->SetDistanceThreshold(6.0);


  fRun -> AddTask(clusterizer);
  fRun -> AddTask(pulse);
  fRun -> AddTask(psaTask);
  fRun -> AddTask(RansacTask);

  // __ Init and run ___________________________________

  fRun -> Init();
  fRun -> Run(1,2);

  std::cout << std::endl << std::endl;
  std::cout << "Macro finished succesfully."  << std::endl << std::endl;
  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------
}
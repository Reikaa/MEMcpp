#include <string>
#include <vector>
#include <iostream>

#include "SubProcesses/P0_Sigma_sm_gg_epvebmumvmxbx/CPPProcess.h"

#include "LHAPDF/LHAPDF.h"
#include "LHAPDF/PDFSet.h"

#include "cuba.h"

#include "TLorentzVector.h"

#include "MEWeight.h"
#include "MEEvent.h"
#include "jacobianD.h"
#include "transferFunction.h"
#include "utils.h"

#define SQRT_S 13000

using namespace std;

MEWeight::MEWeight(const std::string paramCardPath, const std::string pdfName, const std::string fileTF){

  cout << "Initializing Matrix Element computation with:" << endl;
  cout << "Parameter card " << paramCardPath << endl;
  cout << "PDF " << pdfName << endl;
  cout << "TF file " << fileTF << endl;

  process.initProc(paramCardPath);
  pdf = LHAPDF::mkPDF(pdfName, 0);
  myEvent = new MEEvent();
  myTF = new TransferFunction(fileTF);

  //hst_TTbar = new TH1D("DMEM_TTbar", "DMEM  M_{tt}", BINNING, START, STOP);
}

double MEWeight::ComputePdf(const int pid, const double x, const double q2){
  // return f(pid,x,q2)
  if(x <= 0 || x >= 1 || q2 <= 0){
    cout << "WARNING: PDF x or Q^2 value out of bounds!" << endl;
    return 0.;
  }else{
    return pdf->xfxQ2(pid, x, q2)/x;
  }
}

MEEvent* MEWeight::GetEvent(){
  return myEvent;
}

void MEWeight::SetEvent(const TLorentzVector ep, const TLorentzVector mum, const TLorentzVector b, const TLorentzVector bbar, const TLorentzVector met){
  myEvent->SetVectors(ep, mum, b, bbar, met);
}

void MEWeight::AddTF(const std::string particleName, const std::string histName){
  myTF->DefineComponent(particleName, histName);
}

/*void MEWeight::WriteHist(){

  hst_TTbar->Scale(1./hst_TTbar->Integral());

  TCanvas *c = new TCanvas("DMEM_TTbar", "Canvas for plotting", 600, 600);
  c->cd();
  hst_TTbar->Draw();
  //c->Write();
  hst_TTbar->Write();
  //c->Print("plots/DMEM_ttbar.png", "png");
  //delete c; c = 0;

}*/

double MEWeight::ComputeWeight(double &error){
  
  cout << "Initializing integration..." << endl;

  int neval, nfail;
  double mcResult=0, prob=0;
  
  char verbosity = 0; // 0-3
  bool subregion = false; // true = only last set of samples is used for final evaluation of integral
  bool smoothing = false;
  bool retainStateFile = false; // false => delete state file when integration ends
  bool takeOnlyGridFromFile = true; // false => full state taken from file (if present), true => only grid is taken (e.g. to use it for another integrand)
  unsigned int level = 0; 

  unsigned int flags = setFlags(verbosity, subregion, retainStateFile, level, smoothing, takeOnlyGridFromFile);

  cout << "Starting integration..." << endl << endl;

  cubacores(0,0);           // This is mandatory if the integrand wants to *modify* something in the MEWeight object passed as argument
  Vegas(
    8,                      // (int) dimensions of the integrated volume
    1,                      // (int) dimensions of the integrand
    (integrand_t) CUBAIntegrand,  // (integrand_t) integrand (cast to integrand_t)
    (void*) this,           // (void*) pointer to additional arguments passed to integrand
    1,                      // (int) maximum number of points given the integrand in each invocation (=> SIMD) ==> PS points = vector of sets of points (x[ndim][nvec]), integrand returns vector of vector values (f[ncomp][nvec])
    0.005,                  // (double) requested relative accuracy |-> error < max(rel*value,abs)
    0.,                     // (double) requested absolute accuracy |
    flags,                  // (int) various control flags in binary format, see setFlags function
    0,                      // (int) seed (seed==0 => SOBOL; seed!=0 && control flag "level"==0 => Mersenne Twister)
    0,                  // (int) minimum number of integrand evaluations
    50000,                  // (int) maximum number of integrand evaluations (approx.!)
    10000,                  // (int) number of integrand evaluations per interations (to start)
    30000,                      // (int) increase in number of integrand evaluations per interations
    1000,                   // (int) batch size for sampling
    0,                      // (int) grid number, 1-10 => up to 10 grids can be stored, and re-used for other integrands (provided they are not too different)
    "",                     // (char*) name of state file => state can be stored and retrieved for further refinement
    NULL,                   // (int*) "spinning cores": -1 || NULL <=> integrator takes care of starting & stopping child processes (other value => keep or retrieve child processes, probably not useful here)
    &neval,                 // (int*) actual number of evaluations done
    &nfail,                 // 0=desired accuracy was reached; -1=dimensions out of range; >0=accuracy was not reached
    &mcResult,              // (double*) integration result ([ncomp])
    &error,                 // (double*) integration error ([ncomp])
    &prob                   // (double*) Chi-square p-value that error is not reliable (ie should be <0.95) ([ncomp])
  );
  
  cout << "Integration done." << endl;

  //cout << "Status: " << nfail << ", nr. fails: " << mycount << endl;
  //mycount = 0;

  cout << " mcResult= " << mcResult << " +- " << error << " in " << neval << " evaluations. Chi-square prob. = " << prob << endl << endl;

  //myEvent->writeHists();

  /*if(myEvent->GetTTbar()->Integral()){
    myEvent->GetTTbar()->Scale(1./myEvent->GetTTbar()->Integral());
    myEvent->GetTTbar()->SetEntries(1);
    hst_TTbar->Add(myEvent->GetTTbar());
  }*/
  
  if(std::isnan(error))
  error = 0.;
  if(std::isnan(mcResult))
  mcResult = 0.;
  return mcResult;
}

MEWeight::~MEWeight(){
  cout << "Deleting PDF" << endl;
  delete pdf; pdf = NULL;
  cout << "Deleting myEvent" << endl;
  delete myEvent; myEvent = NULL;
  cout << "Deleting myTF" << endl;
  delete myTF; myTF = NULL;
  //cout << "Deleting hst_TTbar" << endl;
  //delete hst_TTbar; hst_TTbar = NULL;
}

int CUBAIntegrand(const int *nDim, const double* Xarg, const int *nComp, double *Value, void *inputs, const int *nVec, const int *core, const double *weight){
  //cout << endl << endl << endl << "########## Starting phase-space point ############" << endl << endl;

  //cout << "Inputs = [" << Xarg[0] << "," << Xarg[1] << "," << Xarg[2] << "," << Xarg[3] << endl;
  
  MEWeight* myWeight = (MEWeight*) inputs;
  *Value = myWeight->Integrand(Xarg, weight);

  return 0;
}

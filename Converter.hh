#ifndef __CONVERTER_HH
#define __CONVERTER_HH

#include <vector>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"
#include "TVector3.h"

#include "TGRSIRunInfo.h"
#include "TChannel.h"
#include "TFragment.h"
#include "TGriffin.h"
#include "TBgo.h"
#include "TSceptar.h"
#include "TPaces.h"
#include "TLaBr.h"
#include "TDescant.h"

#include "Settings.hh"

class Converter {
public:
	Converter(std::vector<std::string>& inputFileNames, const int& runNumber, const int& subRunNumber, const TGRSIRunInfo* runInfo, Settings* settings, bool writeFragmentTree);
	~Converter();

	bool Run();

private:
	int  Cfd(TMnemonic::EDigitizer);
	bool AboveThreshold(double, int);
	bool InsideTimeWindow();
	bool DescantNeutronDiscrimination();
	void FillDetectors();

	void PrintStatistics();

	Settings* fSettings;
	TChain fChain;
	TFile* fFragmentFile;
	TFile* fAnalysisFile;
	TTree fEventTree;
	TFragment* fFragment;
	std::map<uint32_t, TFragment> fFragments;
	bool fWriteFragmentTree;
	TTree fFragmentTree;
	int fFragmentTreeEntries;
	int fRunNumber;
	int fSubRunNumber;
	const TGRSIRunInfo* fRunInfo;
	int fKValue;
	TRandom3 fRandom;

	//branches of input tree/chain
	Int_t fEventNumber;
	Int_t fTrackID;
	Int_t fParentID;
	Int_t fStepNumber;
	Int_t fParticleType;
	Int_t fProcessType;
	Int_t fSystemID;
	Int_t fCryNumber;
	Int_t fDetNumber;
	Double_t fDepEnergy;
	Double_t fPosx;
	Double_t fPosy;
	Double_t fPosz;
	Double_t fTime;

	//branches of output tree
	// GRIFFIN
	TGriffin* fGriffin;

	// BGO
	TBgo* fBgo;

	// LaBr
	TLaBr* fLaBr;

	// Sceptar
	TSceptar* fSceptar;

	// Descant
	TDescant* fDescant;

	// Paces
	TPaces* fPaces;
};
#endif

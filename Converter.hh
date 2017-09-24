#ifndef __CONVERTER_HH
#define __CONVERTER_HH

#include <vector>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"
#include "TVector3.h"

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
    Converter(std::vector<std::string>&, const std::string&, Settings*, bool);
    ~Converter();

    bool Run();

private:
    bool AboveThreshold(double, int);
    bool InsideTimeWindow();
    bool DescantNeutronDiscrimination();

    void PrintStatistics();

    TVector3 GriffinCrystalCenterPosition(int cry, int det);
    bool AreGriffinCrystalCenterPositionsWithinVectorLength(int cry1, int det1, int cry2, int det2);

    double transX(double x, double y, double z, double theta, double phi);
    double transY(double x, double y, double z, double theta, double phi);
    double transZ(double x, double y, double z, double theta, double phi);

    Settings* fSettings;
    TChain fChain;
    TFile* fOutput;
    TTree fEventTree;
	 TFragment* fFragment;
	 bool fWriteFragmentTree;
	 TTree fFragmentTree;
	 int fFragmentTreeEntries;
    TRandom3 fRandom;

    Int_t LaBrGriffinNeighbours_det[8][3];
    Int_t LaBrGriffinNeighbours_cry[8][3];

    Int_t GriffinAncillaryBgoNeighbours_det[16][2];
    Int_t GriffinAncillaryBgoNeighbours_cry[16][2];

    Int_t GriffinSceptarSuppressors_det[16][4];

    Int_t GriffinNeighbours_counted[16];
    Int_t GriffinNeighbours_det[16][4];

    double GriffinDetCoords[16][5];

    double GriffinCryMap[64][64];
    double GriffinCryMapCombos[52][2];

    double GriffinDetMap[16][16];
    double GriffinDetMapCombos[7][2];

    TVector3 GriffinCrystalCenterVectors[64];

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

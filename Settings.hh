#ifndef __SETTINGS_HH
#define __SETTINGS_HH

#include <string>
#include <map>
#include <vector>

#include "TF1.h"

class Settings {
public:
    Settings(std::string, int);
    ~Settings(){};

    std::string NtupleName() { return fNtupleName; }

    int VerbosityLevel() { return fVerbosityLevel; }

    int BufferSize() { return fBufferSize; }

    int SortNumberOfEvents() { return fSortNumberOfEvents; }

    bool WriteTree() { return fWriteTree; }

	 int KValue() { return fKValue; }

    bool WriteGriffinAddbackVector() { return fWriteGriffinAddbackVector; }

	 bool DontSmearEnergy() { return fDontSmearEnergy; }

    double GriffinAddbackVectorLengthmm() { return fGriffinAddbackVectorLengthmm; }

    double GriffinAddbackVectorDepthmm() { return fGriffinAddbackVectorDepthmm; }

    double GriffinAddbackVectorCrystalFaceDistancemm() { return fGriffinAddbackVectorCrystalFaceDistancemm; }

    double Resolution(int systemID, int detectorID, int crystalID, double en) {
        if(fResolution.find(systemID) != fResolution.end()) {
            return fResolution[systemID].at(detectorID).at(crystalID).Eval(en);
        }
        return 0.;
    }
    double Threshold(int systemID, int detectorID, int crystalID) {
        if(fThreshold.find(systemID) != fThreshold.end()) {
            return fThreshold[systemID].at(detectorID).at(crystalID);
        }
        return 0.001;
    }
    double ThresholdWidth(int systemID, int detectorID, int crystalID) {
        if(fThresholdWidth.find(systemID) != fThresholdWidth.end()) {
            return fThresholdWidth[systemID].at(detectorID).at(crystalID);
        }
        return 0.;
    }
    double TimeWindow(int systemID, int detectorID, int crystalID) {
        if(fTimeWindow.find(systemID) != fTimeWindow.end()) {
            return fTimeWindow[systemID].at(detectorID).at(crystalID);
        }
        return 0.;
    }

private:
    std::string fNtupleName;

    int fVerbosityLevel;
    int fBufferSize;
    int fSortNumberOfEvents;

    bool fWriteTree;
	 int fKValue;
    bool fWriteGriffinAddbackVector;
	 bool fDontSmearEnergy;

    double fGriffinAddbackVectorLengthmm;
    double fGriffinAddbackVectorDepthmm;
    double fGriffinAddbackVectorCrystalFaceDistancemm;

    std::map<int,std::vector<std::vector<TF1> > > fResolution;
    std::map<int,std::vector<std::vector<double> > > fThreshold;
    std::map<int,std::vector<std::vector<double> > > fThresholdWidth;
    std::map<int,std::vector<std::vector<double> > > fTimeWindow;
};

#endif

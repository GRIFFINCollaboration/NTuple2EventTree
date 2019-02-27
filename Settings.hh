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
			  if(detectorID < fResolution[systemID].size() && crystalID < fResolution[systemID][detectorID].size()) {
				  return fResolution[systemID][detectorID][crystalID].Eval(en);
			  }
        }
        return 0.;
    }
    double Threshold(int systemID, int detectorID, int crystalID) {
        if(fThreshold.find(systemID) != fThreshold.end()) {
			  if(detectorID < fThreshold[systemID].size() && crystalID < fThreshold[systemID][detectorID].size()) {
				  return fThreshold[systemID][detectorID][crystalID];
			  }
        }
        return 0.001;
    }
    double ThresholdWidth(int systemID, int detectorID, int crystalID) {
        if(fThresholdWidth.find(systemID) != fThresholdWidth.end()) {
			  if(detectorID < fThresholdWidth[systemID].size() && crystalID < fThresholdWidth[systemID][detectorID].size()) {
				  return fThresholdWidth[systemID][detectorID][crystalID];
			  }
        }
        return 0.;
    }
    double TimeWindow(int systemID, int detectorID, int crystalID) {
        if(fTimeWindow.find(systemID) != fTimeWindow.end()) {
			  if(detectorID < fTimeWindow[systemID].size() && crystalID < fTimeWindow[systemID][detectorID].size()) {
				  return fTimeWindow[systemID][detectorID][crystalID];
			  }
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

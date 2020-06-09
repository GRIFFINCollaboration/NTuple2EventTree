#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"

#include "CommandLineInterface.hh"
#include "Utilities.hh"

#include "Settings.hh"
#include "Converter.hh"

int main(int argc, char** argv) {
    //parse all command line options
    CommandLineInterface interface;
    std::string settingsFileName;
    interface.Add("-sf","settings file (required)", &settingsFileName);
    std::vector<std::string> inputFileNames;
    interface.Add("-if","input file(s) (required)", &inputFileNames);
    int runNumber = 0;
    interface.Add("-rn","run number (default = 0)", &runNumber);
    int subRunNumber = 0;
    interface.Add("-sn","sub-run number (default = 0)", &subRunNumber);
	 std::string runInfoFile;
	 interface.Add("-ri","run info file (default = '')", &runInfoFile);
    int verbosityLevel = 0;
    interface.Add("-vl","verbosity level (default = 0)", &verbosityLevel);
	 bool writeFragmentTree = false;
	 interface.Add("-wf","write FragmentTree to separate file", &writeFragmentTree);

    //-------------------- check flags and arguments --------------------
    interface.CheckFlags(argc, argv);

    if(inputFileNames.size() == 0) {
        std::cerr<<"Missing input file name(s)!"<<std::endl;
        return 1;
    }

    //read settings
    Settings settings(settingsFileName, verbosityLevel);

	 //read run info
	 TRunInfo* runInfo = new TRunInfo;
	 if(!runInfoFile.empty()) {
		 runInfo->ReadInfoFile(runInfoFile.c_str());
	 }

    //create converter and run
    Converter converter(inputFileNames, runNumber, subRunNumber, runInfo, &settings, writeFragmentTree);
    if(!converter.Run()) {
        std::cerr<<"processing ended abnormally!"<<std::endl;
        return 1;
    }

    return 0;
}

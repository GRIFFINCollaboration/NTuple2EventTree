#include "Converter.hh"

#include <iostream>
#include <iomanip>

#include "TMath.h"

#include "TChannel.h"

#include "Utilities.hh"

Converter::Converter(std::vector<std::string>& inputFileNames, const int& runNumber, const int& subRunNumber, const TGRSIRunInfo* runInfo, Settings* settings, bool writeFragmentTree)
	: fSettings(settings), fWriteFragmentTree(writeFragmentTree), fFragmentTreeEntries(0), fRunNumber(runNumber), fSubRunNumber(subRunNumber), fRunInfo(runInfo), fKValue(settings->KValue())
{
	//create TChain to read in all input files
	for(auto fileName = inputFileNames.begin(); fileName != inputFileNames.end(); ++fileName) {
		if(!FileExists(*fileName)) {
			std::cerr<<"Failed to find file '"<<*fileName<<"', skipping it!"<<std::endl;
			continue;
		}
		//add sub-directory and tree name to file name
		fileName->append(fSettings->NtupleName());
		fChain.Add(fileName->c_str());
	}

	//add branches to input chain
	fChain.SetBranchAddress("eventNumber", &fEventNumber);
	fChain.SetBranchAddress("trackID", &fTrackID);
	fChain.SetBranchAddress("parentID", &fParentID);
	fChain.SetBranchAddress("stepNumber", &fStepNumber);
	fChain.SetBranchAddress("particleType", &fParticleType);
	fChain.SetBranchAddress("processType", &fProcessType);
	fChain.SetBranchAddress("systemID", &fSystemID);
	fChain.SetBranchAddress("detNumber", &fDetNumber);
	fChain.SetBranchAddress("cryNumber", &fCryNumber);
	fChain.SetBranchAddress("depEnergy", &fDepEnergy);
	fChain.SetBranchAddress("posx", &fPosx);
	fChain.SetBranchAddress("posy", &fPosy);
	fChain.SetBranchAddress("posz", &fPosz);
	fChain.SetBranchAddress("time", &fTime);

	//create output file
	fAnalysisFile = new TFile(Form("analysis%05d_%03d.root", fRunNumber, fSubRunNumber), "recreate");
	if(!fAnalysisFile->IsOpen()) {
		std::cerr<<"Failed to open file '"<<Form("analysis%05d_%03d.root", fRunNumber, fSubRunNumber)<<"', check permissions on directory and disk space!"<<std::endl;
		throw;
	}

	//set tree to belong to output file
	fEventTree.SetDirectory(fAnalysisFile);
	if(fWriteFragmentTree) {
		fFragmentFile = new TFile(Form("fragment%05d_%03d.root", fRunNumber, fSubRunNumber), "recreate");
		if(!fFragmentFile->IsOpen()) {
			std::cerr<<"Failed to open file '"<<Form("fragment%05d_%03d.root", fRunNumber, fSubRunNumber)<<"', check permissions on directory and disk space!"<<std::endl;
			throw;
		}
		fFragmentTree.SetDirectory(fFragmentFile);
	}

	//create branches for output tree
	// GRIFFIN
	fGriffin = new TGriffin;
	fEventTree.Branch("TGriffin", &fGriffin, fSettings->BufferSize());

	// BGO
	fBgo = new TBgo;
	fEventTree.Branch("TBgo", &fBgo, fSettings->BufferSize());

	// LaBr
	fLaBr = new TLaBr;
	fEventTree.Branch("TLaBr", &fLaBr, fSettings->BufferSize());

	// SCEPTAR
	fSceptar = new TSceptar;
	fEventTree.Branch("TSceptar", &fSceptar, fSettings->BufferSize());

	// DESCANT
	fDescant = new TDescant;
	fEventTree.Branch("TDescant", &fDescant, fSettings->BufferSize());

	// PACES
	fPaces = new TPaces;
	fEventTree.Branch("TPaces", &fPaces, fSettings->BufferSize());

	// Fragments
	fFragment = new TFragment;
	if(fSettings->VerbosityLevel() > 0) {
		std::cout<<"created new fragment "<<fFragment<<std::endl;
	}

	if(fWriteFragmentTree) {
		fFragmentTree.Branch("Fragment", &fFragment, fSettings->BufferSize());
	}
}

Converter::~Converter() {
	if(fAnalysisFile->IsOpen()) {
		fAnalysisFile->cd();
		fEventTree.Write("EventTree");
		fRunInfo->Write("TGRSIRunInfo");
		TChannel::WriteToRoot();
		fAnalysisFile->Close();
	}
	if(fWriteFragmentTree) {
		if(fFragmentFile->IsOpen()) {
			fFragmentFile->cd();
			fFragmentTree.Write("FragmentTree");
			fRunInfo->Write("TGRSIRunInfo");
			TChannel::WriteToRoot();
			fFragmentFile->Close();
		}
	}
}

bool Converter::Run() {
	int status;
	int eventNumber = 0;

	float smearedEnergy;
	std::map<int,int> belowThreshold;
	std::map<int,int> outsideTimeWindow;

	long int nEntries = fChain.GetEntries();

	TChannel* channel;
	uint32_t address;
	std::string mnemonic;
	std::string crystalColor = "BGRW";
	for(int i = 0; i < nEntries; ++i) {
		status = fChain.GetEntry(i);
		if(status == -1) {
			std::cerr<<"Error occured, couldn't read entry "<<i<<" from tree "<<fChain.GetName()<<" in file "<<fChain.GetFile()->GetName()<<std::endl;
			continue;
		} else if(status == 0) {
			std::cerr<<"Error occured, entry "<<i<<" in tree "<<fChain.GetName()<<" in file "<<fChain.GetFile()->GetName()<<" doesn't exist"<<std::endl;
			return false;
		}

		//if this entry is from the next event, we fill the tree with everything we've collected so far and reset the vector(s)
		if((fEventNumber != eventNumber) && ((fSettings->SortNumberOfEvents()==0)||(fSettings->SortNumberOfEvents()>=eventNumber))) {
			//for(int j = 0; j < 16; j++) {
			//	GriffinNeighbours_counted[j] = 0;
			//}

			// this takes the fragments we have collected and adds them to the detector classes
			// it also automatically fills the fragment tree
			FillDetectors();

			fEventTree.Fill(); // Tree contains suppressed data

			fGriffin->Clear();
			fBgo->Clear();
			fLaBr->Clear();
			fSceptar->Clear();
			fDescant->Clear();
			fPaces->Clear();

			eventNumber = fEventNumber;
			belowThreshold.clear();
			outsideTimeWindow.clear();

			fFragments.clear();
		}


		// if fSystemID is NOT GRIFFIN, then set fCryNumber to zero
		// This is a quick fix to solve resolution and threshold values from Settings.cc
		if(fSystemID >= 2000) {
			fCryNumber = 0;
		}
		//create energy-resolution smeared energy
		smearedEnergy = fRandom.Gaus(fDepEnergy,fSettings->Resolution(fSystemID,fDetNumber,fCryNumber,fDepEnergy));

		if((fSettings->SortNumberOfEvents()==0)||(fSettings->SortNumberOfEvents()>=fEventNumber) ) {
			//if the hit is above the threshold, we add it to the vector
			if(AboveThreshold(smearedEnergy, fSystemID)) {
				if(InsideTimeWindow() ) {
					switch(fSystemID) {
						//mapping systems to address ranges: 0 - GRIFFIN, 1 - BGO, 2 - LaBr, 3 - ancilliary BGO, 4 - NaI, 5 - SCEPTAR, 6 - SPICE, 7 - PACES, 8 - DESCANT
						case 1000://griffin
							address = 4*fDetNumber + fCryNumber;
							break;
						case 1010://left extension suppressor
						case 1020://right extension suppressor
						case 1030://left casing suppressor
						case 1040://right casing suppressor
						case 1050://back suppressor
							address = 1000 + 10*fDetNumber + fCryNumber;
							break;
						case 10://SPICE
							address = 6000 + fDetNumber;
							break;
						case 50://PACES
							address = 7000 + fDetNumber;
							break;
						case 6000://8pi
						case 6010://8pi inner BGO
						case 6020://8pi outer BGO
							std::cerr<<"Sorry, 8pi is not implemented in GRSISort!"<<std::endl;
							throw;
						case 7000:
							std::cerr<<"Sorry, gridcell is not implemented in GRSISort!"<<std::endl;
							throw;
						case 8010://blue
						case 8020://green
						case 8030://red
						case 8040://white
						case 8050://yellow
							address = 8000 + fDetNumber;
							break;
						case 8500://testcan
							std::cerr<<"Sorry, testcan is not implemented in GRSISort!"<<std::endl;
							throw;
						default: //2000 - LaBr, 3000 - ancillary BGO, 4000 - NaI, 5000 - Sceptar
							address = fSystemID + fDetNumber;
							break;
					}
					if(fFragments.count(address) == 1) {
						// add charge
						fFragments[address].SetCharge(fFragments[address].GetCharge()+smearedEnergy*fKValue);
						// update timestamp
						fFragments[address].SetTimeStamp(fTime*1e8);
					} else {
						fFragments[address].SetAddress(address);
						//fFragments[address].SetCcLong();
						//fFragments[address].SetCcShort();
						fFragments[address].SetCfd(0);
						fFragments[address].SetCharge(smearedEnergy*700);
						fFragments[address].SetKValue(700);
						fFragments[address].SetMidasId(fFragmentTreeEntries);
						// fTime is the time from the beginning of the event in seconds
						fFragments[address].SetMidasTimeStamp(fTime); 
						fFragments[address].SetTimeStamp(fTime*1e8);
						//fFragments[address].SetZc();
						++fFragmentTreeEntries;
						//check if the channel for this address exists, and if not create one and add it to the map
						channel = TChannel::GetChannel(address);
						if(channel == nullptr) {
							switch(fSystemID) {
								case 1000://griffin
									mnemonic = Form("GRG%02d%cN00A", fDetNumber, crystalColor[fCryNumber]);
									break;
								case 1010://left extension suppressor
								case 1020://right extension suppressor
								case 1030://left casing suppressor
								case 1040://right casing suppressor
								case 1050://back suppressor
									mnemonic = Form("GRS%02d%cN00A", fDetNumber, crystalColor[fCryNumber]);
									break;
								case 2000://LABr
									mnemonic = Form("DAL%02dXN00X", fDetNumber);
									break;
								case 3000://ancilliary BGO
									mnemonic = Form("DAS%02dXN00X", fDetNumber);
									break;
								case 5000://SCEPTAR
									mnemonic = Form("SEP%02dXN00X", fDetNumber);
									break;
								case 10://SPICE
									mnemonic = Form("SPI%02dXN%0dX", fDetNumber, fCryNumber);//TODO: fix SPICE mnemonic
									break;
								case 50://PACES
									mnemonic = Form("PAC%02dXN00A", fDetNumber);
									break;
								case 8010://blue
								case 8020://green
								case 8030://red
								case 8040://white
								case 8050://yellow
									mnemonic = Form("DSC%02dXN00X", fDetNumber);
									break;
								default: 
									std::cerr<<"Sorry, unknown system ID "<<fSystemID<<std::endl;
									throw;
							}
							channel = new TChannel;
							channel->SetAddress(address);
							channel->SetName(mnemonic.c_str());
							channel->SetDetectorNumber(fDetNumber);
							channel->SetCrystalNumber(fCryNumber);
							TChannel::AddChannel(channel);
						}
						if(fSettings->VerbosityLevel() > 1) {
							std::cout<<"Initialized values of fragment at address "<<address<<" = 0x"<<std::hex<<address<<std::dec<<std::endl;
							fFragments[address].Print();
						}
					}
				} else {
					++outsideTimeWindow[fSystemID];
				}
			} else {
				++belowThreshold[fSystemID];
			}
		}

		if(i%1000 == 0 && fSettings->VerbosityLevel() > 0) {
			std::cout<<std::setw(3)<<100*i/nEntries<<"% done\r"<<std::flush;
		}
	}

	if(fSettings->VerbosityLevel() > 0) {
		std::cout<<"100% done"<<std::endl;
		if(fSettings->VerbosityLevel() > 1) {
			PrintStatistics();
		}
	}

	return true;
}

void Converter::FillDetectors() {
	for(auto frag : fFragments) {
		if(fWriteFragmentTree) {
			*fFragment = frag.second;
			fFragmentTree.Fill();
		}
		TChannel* channel = TChannel::GetChannel(frag.second.GetAddress());
		switch(frag.second.GetAddress()/1000) {
			//mapping systems to address ranges: 0 - GRIFFIN, 1 - BGO, 2 - LaBr, 3 - ancilliary BGO, 4 - NaI, 5 - SCEPTAR, 6 - SPICE, 7 - PACES, 8 - DESCANT
			case 0:
				fGriffin->AddFragment(std::make_shared<TFragment>(*fFragment), channel);
				if(fSettings->VerbosityLevel() > 2) {
					std::cout<<"Added fragment "<<fFragment<<" to griffin:"<<std::endl;
					fFragment->Print();
				}
				break;
			case 1:
			case 3:
				fBgo->AddFragment(std::make_shared<TFragment>(*fFragment), channel);
				if(fSettings->VerbosityLevel() > 2) {
					std::cout<<"Added fragment "<<fFragment<<" to bgo:"<<std::endl;
					fFragment->Print();
				}
				break;
			case 2:
				fLaBr->AddFragment(std::make_shared<TFragment>(*fFragment), channel);
				if(fSettings->VerbosityLevel() > 2) {
					std::cout<<"Added fragment "<<fFragment<<" to labr:"<<std::endl;
					fFragment->Print();
				}
				break;
			case 5:
				fSceptar->AddFragment(std::make_shared<TFragment>(*fFragment), channel);
				if(fSettings->VerbosityLevel() > 2) {
					std::cout<<"Added fragment "<<fFragment<<" to sceptar:"<<std::endl;
					fFragment->Print();
				}
				break;
			case 7:
				fPaces->AddFragment(std::make_shared<TFragment>(*fFragment), channel);
				if(fSettings->VerbosityLevel() > 2) {
					std::cout<<"Added fragment "<<fFragment<<" to paces:"<<std::endl;
					fFragment->Print();
				}
				break;
			case 8:
				fDescant->AddFragment(std::make_shared<TFragment>(*fFragment), channel);
				if(fSettings->VerbosityLevel() > 2) {
					std::cout<<"Added fragment "<<fFragment<<" to descant:"<<std::endl;
					fFragment->Print();
				}
				break;

			default:
				if(fSettings->VerbosityLevel() > 1) {
					std::cerr<<"Unknown address "<<frag.second.GetAddress()<<" = 0x"<<std::hex<<frag.second.GetAddress()<<std::dec<<std::endl;
					frag.second.Print();
				}
				break;
		}
	}
}

bool Converter::AboveThreshold(double energy, int systemID) {
	if(systemID == 5000) {
		// apply hard threshold of 50 keV on Sceptar
		// SCEPTAR in reality saturates at an efficiency of about 80%. In simulation we get an efficiency of 90%
		// 0.9 * 1.11111111 = 100%, 0.8*1.1111111 = 0.888888888
		if(energy > 50.0 && (fRandom.Uniform(0.,1.) < 0.88888888 )) {
			return true;
		}
		else {
			return false;
		}
	}
	else if(energy > fSettings->Threshold(fSystemID,fDetNumber,fCryNumber)+10*fSettings->ThresholdWidth(fSystemID,fDetNumber,fCryNumber)) {
		return true;
	}

	if(fRandom.Uniform(0.,1.) < 0.5*(TMath::Erf((energy-fSettings->Threshold(fSystemID,fDetNumber,fCryNumber))/fSettings->ThresholdWidth(fSystemID,fDetNumber,fCryNumber))+1)) {
		return true;
	}

	return false;
}

bool Converter::InsideTimeWindow() {
	if(fSettings->TimeWindow(fSystemID,fDetNumber,fCryNumber) == 0) {
		return true;
	}
	if(fTime < fSettings->TimeWindow(fSystemID,fDetNumber,fCryNumber)) {
		return true;
	}
	return false;
}

bool Converter::DescantNeutronDiscrimination() { // Assuming perfect gamma-neutron discrimination
	if(fParticleType == 5) { // neutron
		return true;
	}
	return false;
}

void Converter::PrintStatistics() {
}


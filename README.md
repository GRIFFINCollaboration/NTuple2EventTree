# NTuple2EventTree

A program that sorts the Geant4 NTuple output into an EventTree (and an FragmentTree if wanted).
This program accepts a settings file that includes thresholds, resolutions, etc, for each detection system.

This program was written by Vinzenz Bildstein.

-----------------------------------------
 Installation
-----------------------------------------

Since NTuple2EventTree writes out event trees in the GRSISort format, you need to have GRSISort installed and properly setup (see https://github.com/GRIFFINCollaboration/GRSISort).
The NTuple2EventTree code requires the CommandLineInterface library (https://github.com/GRIFFINCollaboration/CommandLineInterface.git), please clone and compile this library first.
You will also need to have GRSISort installed and set up (https://github.com/GRIFFINCollaboration/GRSISort).
The Makefile assumes that the code of the CommandLineInterface library is in ~/CommandLineInterface, if this is not the case, edit the Makefile to point COMM_DIR to the directory where the code is.
The Makefile also assumes that you have a ~/lib directory where the shared-object libraries from the CommandLineInterface are installed.
Again, if this is not the case, modify the Makefile so that LIB_DIR points to where these shared-object libraries are.
Once the Makefile has been adjusted, the code can be compiled with a simple make.

-----------------------------------------
 Usage
-----------------------------------------

use NTuple2EventTree with following flags:
        [-sf <string        >: settings file (required)]
        [-if <vector<string>>: input file(s) (required)]
        [-rn <int           >: run number (default = 0)]
        [-sn <int           >: sub-run number (default = 0)]
        [-ri <string        >: run info file (default = '')]
        [-vl <int           >: verbosity level (default = 0)]
        [-wf                 : write FragmentTree to separate file]

The settings file allows you to change multiple settings of the program, from the name of the ntuple input tree to the resolutions applied to the different detectors. To see what settings are possible please have a look at the Setting.cc file.

The run number R and sub-run number S determine the name of the output file which will have the format analysisRRRRR_SSS.root.

The provided run info file will be read via the TGRSIRunInfo::ReadInfoFile function.
The resulting TGRSIRunInfo object will be written to the output file.

If you choose to also create a fragment tree, a separate file will be produce (the name will be formatted to fragmentRRRRR_SSS.root) which contains the fragment tree.

The verbosity level can be used to turn on debug messages (the higher the level the more verbose these messages become).

-----------------------------------------
 How the program works
-----------------------------------------

The program uses the system ID, detector number dd, and crystal color c (B,  G, R, or W base on the crystal number) of the simulation to determine what address and mnemonic the hit belongs to.
The addresses of a detection system fall in a group of 1000 addresses:

- GRIFFIN has system ID 1000 and its addresses are 4 * detector number + crystal number (group 0), its mnemonics are GRGddcN00A
- GRIFFIN BGOs have system IDs 1010, 1020, 1030, 1040, and 1050 and get addresses as 1000 + 10 * detector number + crystal number (this will likely have to be corrected!) (group 1), its mnemonics are GRSddsN00A
- LaBrs have the system ID 2000 and get the addresse 2000 + detector number (group 2), its mnemomics are DALddXN00X
- Ancilliary BGOs have the system ID 3000 and get the addresses 3000 + detector number (group 3), its mnemonics are DASddXN00X
- NaI detectors have the system ID 4000, but aren't implemented in GRSISort and thus ignored
- SCEPTAR has system ID 5000 and the addresses 5000 + detector number (group 5), its mnemonics are SEPddXN00X
- SPICE has the system ID 10 and gets addresses 6000 + detector number (might need to be corrected) (group 6), its mnemonics are SPIddXNssX, with ss being the crystal number (this will have to be corrected)
- PACES has the system ID 50 and gets addresses 7000 + detector number (group 7), its mnemonics are PACddXN00A
- DESCANT has the system IDs 8010, 8020, 8030, 8040, and 8050 and gets addresses 8000 + detector number (group 8), its mnemonics are DSCddXN00X

For each hit we check if the event number of the hit matches the event number of the last hit.
If so, we check if fragment map has a fragment with the same address. If it does, we just add the smeared energy multiplied by the k-value to the charge and update the time stamp to the simulaton time.
If it does not we set the address, charge, k-value, midas ID (fragment tree entry #), midas timestamp (simulation time), timestamp (also simulation time), and create a new TChannel with the correct mnemonic.
If the event number of the hit does not match the event number of the last hit, we have read all hits of the previous event, so we loop over all fragments we got in our map, write to the fragment tree if that option was chosen, fill them in their corresponding detector, and then clear the map of fragments.

This means that the timestamp of a detector is determined by the simulation time of the last hit.
It also doesn't yet get converted into the proper 10 ns timestamps, nor does the CFD value get set.


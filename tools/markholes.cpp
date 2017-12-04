//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Nov 26 08:10:23 PST 2017
// Last Modified: Fri Dec  1 16:46:51 PST 2017
// Filename:      markholes.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Mark holes in green and paper margins in blue.
//

#include "RollImage.h"

#include <vector>

using namespace std;
using namespace tiff;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 3) {
		cerr << "Usage: frameduplicates file.tiff\n";
		exit(1);
	}

	RollImage roll;
	if (!roll.open(argv[1])) {
		cerr << "Input filename " << argv[1] << " cannot be opened" << endl;
		exit(1);
	}

	fstream output;
	output.open(argv[2], ios::binary | ios::in | ios::out);
	if (!output.is_open()) {
		cerr << "Output filename " << argv[2] << " cannot be opened" << endl;
		exit(1);
	}

	roll.loadGreenChannel();
	roll.analyze();
	roll.printRollImageProperties();
	roll.markHoleBBs();
	roll.markTrackerPositions();
	roll.mergePixelOverlay(output);

	output.close();
	return 0;
}


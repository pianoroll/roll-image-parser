//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Feb  8 22:17:15 PST 2018
// Last Modified: Thu Feb  8 22:17:18 PST 2018
// Filename:      checkquality.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Report to standard error if there are any problems.
//

#include "RollImage.h"

#include <vector>

using namespace std;
using namespace prp;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 2) {
		cerr << "Usage: frameduplicates file.tiff\n";
		exit(1);
	}

	RollImage roll;
	if (!roll.open(argv[1])) {
		cerr << "Input filename " << argv[1] << " cannot be opened" << endl;
		exit(1);
	}

	roll.loadGreenChannel(255);
	roll.analyze();
	roll.printQualityReport();

	return 0;
}


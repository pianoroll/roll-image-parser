//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Feb 17 20:04:29 PST 2018
// Last Modified: Sat Feb 17 20:04:31 PST 2018
// Filename:      getGreenPgm.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Extract green channel to a PGM format image.
//

#include "RollImage.h"

#include <vector>

using namespace std;
using namespace rip;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 2) {
		cerr << "Usage: getGreenPgm file.tiff\n";
		exit(1);
	}

	RollImage roll;
	if (!roll.open(argv[1])) {
		cerr << "Input filename " << argv[1] << " cannot be opened" << endl;
		exit(1);
	}

	int threshold = 249;

	roll.setDebugOn();
	roll.setWarningOn();
	roll.loadGreenChannel(threshold);

	int rows = roll.getRows();
	int cols = roll.getCols();

	cout << "P2" << endl;
	cout << cols << " " << rows << endl;
	cout << 255 << endl;

	for (int r=0; r<(int)rows; r++) {
		for (int c=0; c<(int)cols; c++) {
			cout << (int)roll.monochrome.at(r).at(c);
			if (c < cols - 1) {
				cout << ' ';
			}
		}
		cout << endl;
	}

	return 0;
}




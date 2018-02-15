//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Nov 26 08:10:23 PST 2017
// Last Modified: Sun Nov 26 08:10:26 PST 2017
// Filename:      channelhistograms.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Create histograms for intensity values for each color.
//

#include "TiffFile.h"

#include <vector>

using namespace std;
using namespace prp;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 2) {
		cerr << "Usage: frameduplicates file.tiff\n";
	}

	TiffFile tfile;
	if (!tfile.open(argv[1])) {
		cerr << "Input filename " << argv[1] << " cannot be opened" << endl;
		exit(1);
	}

	vector<vector<ulongint>> histograms(3);
	for (ulongint i=0; i<histograms.size(); i++) {
		histograms[i].resize(256);
		std::fill(histograms[i].begin(), histograms[i].end(), 0);
	}
	tfile.goToPixelIndex(0);
	ulongint count = tfile.getPixelCount();
	for (ulongint i=0; i<count; i++) {
		ucharint red   = tfile.read1UByte();
		ucharint green = tfile.read1UByte();
		ucharint blue  = tfile.read1UByte();
		histograms[0].at(red)++;
		histograms[1].at(green)++;
		histograms[2].at(blue)++;
	}

	cout << "**value\t**red\t**green\t**blue\n";
	for (ulongint j=0; j<256; j++) {
		cout << j;
		cout << "\t" << histograms[0].at(j);
		cout << "\t" << histograms[1].at(j);
		cout << "\t" << histograms[2].at(j);
		cout << "\n";
	}
	cout << "*-\t*-\t*-\t*-\n";

	return 0;
}




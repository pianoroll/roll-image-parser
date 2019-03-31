//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Nov 26 08:10:23 PST 2017
// Last Modified: Sun Nov 26 08:10:26 PST 2017
// Filename:      markbright.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Mark bright regions (looking at green channel only).
//

#include "TiffFile.h"

#include <vector>

using namespace std;
using namespace rip;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 3) {
		cerr << "Usage: frameduplicates file.tiff\n";
		exit(1);
	}

	TiffFile tfile;
	if (!tfile.open(argv[1])) {
		cerr << "Input filename " << argv[1] << " cannot be opened" << endl;
		exit(1);
	}

	fstream output;
	output.open(argv[2], ios::binary | ios::in | ios::out);
	if (!output.is_open()) {
		cerr << "Output filename " << argv[2] << " cannot be opened" << endl;
		exit(1);
	}

	vector<ucharint> pixel(3, 0);
	tfile.goToPixelIndex(0);
	ulongint count = tfile.getPixelCount();
	ulongint offset;
	for (ulongint i=0; i<count; i++) {
		pixel[0] = tfile.read1UByte();  // red
		pixel[1] = tfile.read1UByte();  // green
		pixel[2] = tfile.read1UByte();  // blue
		if (pixel[1] == 255) {
			// holes set to green
			pixel[0] = 0;
			pixel[1] = 255;
			pixel[2] = 0;
			offset = tfile.getPixelOffset(i);
			output.seekp(offset);
			output.write((char*)pixel.data(), 3);
		} else if (pixel[1] > 200) {
			// border regions set to red.
			pixel[0] = 255;
			pixel[1] = 0;
			pixel[2] = 0;
			offset = tfile.getPixelOffset(i);
			output.seekp(offset);
			output.write((char*)pixel.data(), 3);
		}
	}

	return 0;
}



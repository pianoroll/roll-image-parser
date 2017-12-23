//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Dec 21 14:01:50 PST 2017
// Last Modified: Thu Dec 21 14:01:53 PST 2017
// Filename:      markholes.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Reverse the order of pixels in rows of a TIFF image.
//

#include "RollImage.h"

#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace prp;

void flipRow(fstream& output, TiffFile& image);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 3) {
		cerr << "Usage: leftrightswap input.tiff output.tiff\n";
		exit(1);
	}

	TiffFile image;
	if (!image.open(argv[1])) {
		cerr << "Input filename " << argv[1] << " cannot be opened" << endl;
		exit(1);
	}

	fstream output;
	output.open(argv[2], ios::binary | ios::out);
	if (!output.is_open()) {
		cerr << "Output filename " << argv[2] << " cannot be opened" << endl;
		exit(1);
	}

	ulong dataoffset = image.getDataOffset();

	image.goToByteIndex(0);
	string header = image.readString(dataoffset);

	output.write(header.data(), header.size());

	image.goToPixelIndex(0);  // should already be there, but being careful.
	ulong rows = image.getRows();
	
	// assuming 24-bit color for now.
	for (ulong r=0; r<rows; r++) {
		flipRow(output, image);
	}

	ulong position = image.tellg();
	image.seekg(0, image.end);
	ulong endpos = image.tellg();
	image.seekg(position, image.beg);
	ulong trailersize = endpos - position;
	string trailer = image.readString(trailersize);
	output.write(trailer.data(), trailer.size());
	cerr << "TRAILER SIZE = " << trailersize << endl;

	output.close();
	return 0;
}



//////////////////////////////
//
// flipRow -- flipping the current row, presuming read pointer is set to the
//    correct position already.  Also presuming 24-bit color pixels.
//

void flipRow(fstream& output, TiffFile& image) {
	string indata;
	string outdata;
	int cols = (int)image.getCols();
	outdata.resize(cols*3);
	indata = image.readString(cols*3);

	if ((int)indata.size() != cols * 3) {
		cerr << "Strange problem happened when flipping row." << endl;
		exit(1);
	}
	for (int c=0; c<cols; c++) {
		outdata[(c*3)+0] = indata[(cols-c-1)*3+0];
		outdata[(c*3)+1] = indata[(cols-c-1)*3+1];
		outdata[(c*3)+2] = indata[(cols-c-1)*3+2];
	}
	output.write(outdata.data(), outdata.size());
}




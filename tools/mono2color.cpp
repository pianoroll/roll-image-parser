//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 14 18:03:40 PDT 2018
// Last Modified: Sun Oct 14 18:03:44 PDT 2018
// Filename:      mono2color.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Convert 8-bit b&w image to 24-bit RGB.
//

#include "RollImage.h"

#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace prp;

void duplicateSamples(fstream& output, TiffFile& image);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 3) {
		cerr << "Usage: mono2color input.tiff output.tiff\n";
		exit(1);
	}

	TiffFile image;
	image.allowMonochrome();

	if (!image.open(argv[1])) {
		cerr << "Input filename " << argv[1] << " cannot be opened" << endl;
		exit(1);
	}

	if (!image.isMonochrome()) {
		cerr << "Input file must be monochrome" << endl;
		exit(1);
	}

	fstream output;
	output.open(argv[2], ios::binary | ios::out);
	if (!output.is_open()) {
		cerr << "Output filename " << argv[2] << " cannot be opened" << endl;
		exit(1);
	}

	ulonglongint dataoffset = image.getDataOffset();
	ulonglongint monobytes = image.getDataBytes();

	image.goToByteIndex(0);
	string header = image.readString(dataoffset);

	// copy the bytes before the data
	output.write(header.data(), header.size());

	// copy the data bytes, duplicating the samples to create color image
	duplicateSamples(output, image);

	// copy the bytes after the data
	ulonglongint position = image.tellg();
	image.seekg(0, std::ios::end);
	ulonglongint endpos = image.tellg();
	prp::goToByteIndex(image, position);
	ulonglongint trailersize = endpos - position;
	string trailer = image.readString(trailersize);
	output.write(trailer.data(), trailer.size());

	// go back and update the directory offset (tiff header) location if
	// it is placed after the data (which has now expanded).
	ulonglongint diroffset = image.getDirectoryOffset();
	if (diroffset > monobytes) {
		// directory is after data, so update the directory offset.
		image.writeDirectoryOffset(diroffset + 2 * monobytes);
	}

	image.close();
	image.allowMonochrome();
	if (!image.open(argv[2])) {
		cerr << "Output filename " << argv[2] << " cannot be reopened" << endl;
		exit(1);
	}
	image.writeSamplesPerPixel(3);
	image.close();


	return 0;
}



//////////////////////////////
//
// duplicateSamples -- Convert one-sample pixels into three-sample pixels.
//

void duplicateSamples(fstream& output, TiffFile& image) {

	image.goToPixelIndex(0);  // should already be there, but being careful.

	ulong rows = image.getRows();
	ulong cols = image.getCols();

	string indata;
	string outdata;
	outdata.resize(cols*3);

	for (int r=0; r<(int)rows; r++) {
		indata = image.readString(cols);
		if ((int)indata.size() != (int)cols) {
			cerr << "Strange problem happened converting file." << endl;
			exit(1);
		}
		for (int i=0; i<(int)cols; i++) {
			outdata[i*3 + 0] = indata[i + 0];
			outdata[i*3 + 1] = indata[i + 1];
			outdata[i*3 + 2] = indata[i + 2];
		}
		output.write(outdata.data(), outdata.size());
	}
}



//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Apr  6 13:34:49 EDT 2019
// Last Modified: Sun Apr  7 13:58:38 EDT 2019
// Filename:      straighten.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Correct for left-right drifting along the length of a roll image.
//
// Options:
//

#include "TiffFile.h"
#include "Options.h"

#include <vector>

using namespace std;
using namespace rip;
using namespace smf;

bool getDriftAnalysis(vector<pair<int, double>>& driftAnalysis, const string& filename);
void fillDriftArray(vector<double>& drift, vector<pair<int, double>>& driftAnalysis, int rows);
void shiftImageRow(fstream& output, TiffFile& tfile, int row, int adjust);

int Brightness = 254;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("b|brightness=i:254", "Brightness level for margin edge");
	options.process(argc, argv);

	Brightness = options.getInteger("brightness");

	if (options.getArgCount() != 3) {
		cerr << "Usage: straighten analysis.txt original.tiff output.tiff" << endl;
		cerr << "original.tiff must be a 24-bit color image, uncompressed" << endl;
		cerr << "output.tiff must be a copy of original.tiff and it will be straightened." << endl;
		exit(1);
	}

	if (options.getArg(2) == options.getArg(3)) {
		cerr << "Error: Input and output files cannot be the same" << endl;
		exit(1);
	}

	TiffFile image;
	if (!image.open(options.getArg(2))) {
		cerr << "Input filename " << options.getArg(2) << " cannot be opened" << endl;
		exit(1);
	}

	fstream output;
	output.open(options.getArg(3).c_str(), ios::binary | ios::out);
	if (!output.is_open()) {
		cerr << "Output filename " << options.getArg(3) << " cannot be opened" << endl;
		exit(1);
	}

	ulonglongint dataoffset = image.getDataOffset();
	image.goToByteIndex(0);
	string header = image.readString(dataoffset);
	output.write(header.data(), header.size());
	image.goToPixelIndex(0);  // should already be there, but being careful.
	ulongint rows = image.getRows();

	vector<pair<int, double>> driftAnalysis;
	bool status = getDriftAnalysis(driftAnalysis, options.getArg(1));
	if (!status) {
		cerr << "Error: could not find drift analysis" << endl;
		exit(1);
	}

	vector<double> drift;
	fillDriftArray(drift, driftAnalysis, rows);

	// assuming 24-bit color for now.
	for (ulongint r=0; r<rows; r++) {
		shiftImageRow(output, image, r, drift[r]);
	}

	ulonglongint position = image.tellg();
	image.seekg(0, std::ios::end);
	ulonglongint endpos = image.tellg();
	rip::goToByteIndex(image, position);
	ulonglongint trailersize = endpos - position;
	string trailer = image.readString(trailersize);
	output.write(trailer.data(), trailer.size());
	output.close();
	return 0;
}



//////////////////////////////
//
// shiftImageRow -- 
//

void shiftImageRow(fstream& output, TiffFile& image, int row, int adjust) {

	int iadjust = 0;
	if (adjust > 0) {
		iadjust = int(adjust + 0.5);
	} else {
		iadjust = -int(-adjust + 0.5);
	}

	string indata;
	string outdata;
	int cols = (int)image.getCols();
	indata = image.readString(cols*3);

	if ((int)indata.size() != cols * 3) {
		cerr << "Strange problem happened when processing row." << endl;
		exit(1);
	}

	outdata.resize(cols*3);
	for (int c=0; c<cols*3; c++) {
		outdata[c] = Brightness;
	}

	for (int c=0; c<cols; c++) {
		int newc = c + iadjust;
		if (newc < 0) {
			continue;
		}
		if (newc >= cols) {
			continue;
		}
		outdata[(newc*3)+0] = indata[c*3+0];
		outdata[(newc*3)+1] = indata[c*3+1];
		outdata[(newc*3)+2] = indata[c*3+2];
	}
	output.write(outdata.data(), outdata.size());
}



//////////////////////////////
//
// fillDriftArray --
//

void fillDriftArray(vector<double>& drift, vector<pair<int, double>>& driftAnalysis, int rows) {
	drift.resize(rows);
	double offset = driftAnalysis[0].second;
	for (int i=0; i<(int)driftAnalysis.size(); i++) {
		int startj = 0;
		double value = offset;
		if (i>0) {
			startj = driftAnalysis[i-1].first;
			value = driftAnalysis[i-1].second;
		}
		for (int j=startj; j<driftAnalysis[i].first; j++) {
			drift[j] = value - offset;
		}
	}
	
	for (int i=driftAnalysis.back().first; i<(int)drift.size(); i++) {
		drift[i] = driftAnalysis.back().second - offset;
	}
}



//////////////////////////////
//
// getDriftAnalysis --
//

bool getDriftAnalysis(vector<pair<int, double>>& driftAnalysis, const string& filename) {
	driftAnalysis.resize(0);
	driftAnalysis.reserve(10000);
	ifstream datafile(filename.c_str());
	if (!datafile.is_open()) {
		return false;
	}
	string line;
	// bool founddriftsection = false;

	while (getline(datafile, line)) {
		if (line.find("@@BEGIN:") == string::npos) {
			continue;
		}
		if (line.find("DRIFT") == string::npos) {
			continue;
		}
		// founddriftsection = true;
		break;
	}

	while (getline(datafile, line)) {
		if (line.find("@DATA:") == string::npos) {
			continue;
		}
		break;
	}

	while (getline(datafile, line)) {
		if ((line.find("@@END:") != string::npos) && (line.find("DRIFT") != string::npos)) {
			break;
		}
		if (line.find("@") != string::npos) {
			break;
		}
		int pixel;
		double drift;
		int count = sscanf(line.c_str(), "\t%d\t%lf", &pixel, &drift);
		if (count == 2) {
			pair<int, double> values;
			values.first = pixel;
			values.second = drift;
			driftAnalysis.push_back(values);
		}
	}
	datafile.close();
	if (driftAnalysis.size() > 0) {
		return true;
	}
	return false;
}





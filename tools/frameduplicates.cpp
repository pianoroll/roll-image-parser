//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Sat Nov 25 20:57:58 PST 2017
// Filename:      frameduplicates.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Identify line duplications in a piano-roll image.
//
// References:
//      https://web.archive.org/web/20160306201233/http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf (page 13)
//      https://www.loc.gov/preservation/digital/formats/content/tiff_tags.shtml
//      https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
//

#include "TiffFile.h"
#include "Crc32.h"

#include <vector>
#include <map>
#include <iomanip>

using namespace std;
using namespace rip;

class DuplicateInfo {
	public:
		int count = 0;
		vector<ulongint> rows;
};

// function declarations:
void   getRowCheckSums            (vector<ulongint>& checksums, TiffFile& tfile);
void   identifyDuplicateFrames    (fstream& output, TiffFile& tfile, 
                                   vector<ulongint>& rowchecksums, ulongint framesize);
void   markImageDuplicateFrame    (fstream& output, TiffFile& tfile, int color,
                                   int firstrow, int otherrow, int framesize,
                                   int dupnum);
bool   verifyDuplicate            (TiffFile& tfile, int row1, int row2);


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 3) {
		cerr << "Usage: frameduplicates input.tiff output.tiff\n";
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

	ulonglongint expected = (ulonglongint)tfile.getRows() * (ulonglongint)tfile.getCols() * (ulonglongint)3;
	if (expected != tfile.getDataBytes()) {
		cerr << "ERROR: image size does not match header information." << endl;
		cerr << "STRIP BYTE COUNT " << tfile.getDataBytes() << endl;
		cerr << "EXPECTED BYTE COUNT " << expected<< endl;
		cerr << "DIFFERENCE " << tfile.getDataBytes() - expected << endl;
		exit(1);
	}

	vector<ulongint> rowchecksums;
	getRowCheckSums(rowchecksums, tfile);
	//for (int i=0; i<(int)rowchecksums.size(); i++) {
	//	cout << rowchecksums[i] << endl;
	//}

	identifyDuplicateFrames(output, tfile, rowchecksums, 30);

	output.close();

	return 0;
}

///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// identifyDuplicateFrames --
//

void identifyDuplicateFrames(fstream& output, TiffFile& tfile, 
		vector<ulongint>& rowchecksums, ulongint framesize) {

	map<ulongint, DuplicateInfo> duplicates;
	for (ulongint i=0; i<rowchecksums.size(); i++) {
		duplicates[rowchecksums[i]].count++;
		duplicates[rowchecksums[i]].rows.push_back(i);
	}

	int color = 2;
	vector<int> marked(rowchecksums.size(), -1);

	for (ulongint i=0; i<rowchecksums.size(); i++) {
		if (marked[i] >0) {
			continue;
		}
		ulongint checksum = rowchecksums[i];
		DuplicateInfo& di = duplicates[checksum];
		if (di.count < 2) {
			continue;
		}
		if (di.rows.size() < 2) {
			continue;
		}
		if (di.rows.at(0) != i) {
			continue;
		}

		if ((i == 0) || (marked[i-1] < 0)) {
			color = (color + 1) % 3;
		} else if ((i >= framesize) && (marked[i-1] >= 0)) {
			if (rowchecksums[i] != rowchecksums[i-framesize]) {
				if (rowchecksums[i] != rowchecksums[i-1]) {
					color = (color + 1) % 3;
				}
			}
		}

		for (ulongint j=1; j<di.rows.size(); j++) {
			int ii = di.rows[j];
			if (!verifyDuplicate(tfile, i, ii)) {
				continue;
			}
			// cerr << "\tDuplicate rows " << i << " and " << ii << endl;
			marked[i] = color;
			marked[ii] = color;
			markImageDuplicateFrame(output, tfile, color, i, ii, 1, j);
		}
	}

	//for (ulongint i=0; i<marked.size(); i++) {
	//	cerr << marked[i] << endl;
	//}

}



//////////////////////////////
//
// verifyDuplicate --
//

bool verifyDuplicate(TiffFile& tfile, int row1, int row2) {
	int rowbytecount = tfile.getCols() * 3;
	tfile.goToRowColumnIndex(row1, 0);
	string rowbytes1 = tfile.readString(rowbytecount);

	tfile.goToRowColumnIndex(row2, 0);
	string rowbytes2 = tfile.readString(rowbytecount);
	
	for (int i=0; i<(int)rowbytes1.size(); i++) {
		if (rowbytes1[i] != rowbytes2[i]) {
			return false;
		}
	}

	return true;
}



//////////////////////////////
//
// markImageDuplicateFrame -- Mark the duplicate with a half-row solid color.
//

void markImageDuplicateFrame(fstream& output, TiffFile& tfile, int color, int firstrow, 
		int otherrow, int framesize, int dupnum) {

	if (firstrow % 30 == 0) {
		cerr << "DUPLICATE FRAME PAIR AT " << firstrow << " and " << otherrow << endl;
	}
	vector<char> pixel;
	pixel.resize(3);
	switch (color % 3) {
		case 0: pixel[0] = 0xff; pixel[1] = 0x00; pixel[2] = 0x00; break;
		case 1: pixel[0] = 0xff; pixel[1] = 0x99; pixel[2] = 0x33; break;
		case 2: pixel[0] = 0xff; pixel[1] = 0x00; pixel[2] = 0xff; break;
	}

	vector<char> quarterrow;
	int qsize = tfile.getCols() / 4;
	quarterrow.resize(qsize * 3);
	for (int i=0; i<qsize; i++) {
		quarterrow[3*i+0] = pixel[0];
		quarterrow[3*i+1] = pixel[1];
		quarterrow[3*i+2] = pixel[2];
	}

	int side = dupnum % 2;
	ulonglongint offset;

	if (dupnum == 1) {
		for (int i = 0; i<framesize; i++) {
			offset = tfile.getDataOffset() + (firstrow + i) * tfile.getCols() * 3;
			output.seekp(offset, output.beg);
			output.write(quarterrow.data(), qsize * 3);
		}
	}

	for (int i = 0; i<framesize; i++) {
		offset = tfile.getDataOffset() + (otherrow + i) * tfile.getCols() * 3 + side * 3 * qsize * 3;
		output.seekp(offset, output.beg);
		output.write(quarterrow.data(), qsize * 3);
	}
}



//////////////////////////////
//
// getRowCheckSums -- Calculate checksums for each row of the image.
//

void getRowCheckSums(vector<ulongint>& checksums, TiffFile& tfile) {
	tfile.goToPixelIndex(0);
	int rowbytecount = tfile.getCols() * 3;
	string rowbytes;

	checksums.resize(tfile.getRows());
	ulongint crc;
	for (ulongint i=0; i<tfile.getRows(); i++) {
		rowbytes = tfile.readString(rowbytecount);
    	crc = crc32_fast(rowbytes.data(), rowbytecount, 0);
		checksums[i] = crc;
	}
}




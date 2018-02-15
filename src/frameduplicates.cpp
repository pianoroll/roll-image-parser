//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Nov 24 11:40:27 PST 2017
// Last Modified: Fri Nov 24 14:28:21 PST 2017
// Filename:      frameduplicates.cpp
// Web Address:
// Syntax:        C++;
// vim:           ts=3:nowrap:ft=text
//
// Description:   Mark duplicate rows of an image.
//

#include "Crc32.cpp"

class DuplicateInfo {
	public:
		int count = 0;
		vector<ulongint> rows;
};

// function declarations:
void     getTiffHeader            (TiffHeader& header, fstream& input, int diroffset);
ulongint    readEntryUInt            (fstream& input, int datatype, int count);
void     readDirectoryEntry       (TiffHeader& header, fstream& input);
double   readType5Value           (fstream& input, int datatype, int count);
void     getRowCheckSums          (vector<ulongint>& checksums, fstream& input,
                                   TiffHeader& header);
void     identifyDuplicateFrames  (fstream& output, TiffHeader& header,
                                   vector<ulongint>& rowchecksums, int framesize);
void     markImageDuplicateFrame  (fstream& output, TiffHeader& header, int color,
                                   int firstrow, int otherrow, int framesize,
                                   int dupnum);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {

	fstream output;
	output.open(argv[2], ios::binary | ios::in | ios::out);
	if (!output.is_open()) {
		cerr << "Output filename " << argv[2] << " cannot be opened" << endl;
		exit(1);
	}

	vector<ulongint> rowchecksums;
	getRowCheckSums(rowchecksums, input, header);
	// for (int i=0; i<(int)rowchecksums.size(); i++) {
	// 	cout << rowchecksums[i] << endl;
	// }

	identifyDuplicateFrames(output, header, rowchecksums, 30);

	output.close();
	input.close();

	return 0;
}

///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// identifyDuplicateFrames --
//

void identifyDuplicateFrames(fstream& output, TiffHeader& header,
		vector<ulongint>& rowchecksums, int framesize) {

	map<ulongint, DuplicateInfo> duplicates;
	for (int i=0; i<(int)rowchecksums.size(); i++) {
		duplicates[rowchecksums[i]].count++;
		duplicates[rowchecksums[i]].rows.push_back(i);
	}

	int color = 0;
	vector<int> marked(rowchecksums.size(), 0);

	for (int i=0; i<(int)rowchecksums.size(); i++) {
		if (marked[i]) {
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
		for (int j=1; j<(int)di.rows.size(); j++) {
			int ii = di.rows[j];
			bool duplicate = true;
			for (int k=0; k<framesize; k++) {
				if (ii+k >= rowchecksums.size()) {
					duplicate = false;
					break;
				}
				if (rowchecksums[i+k] != rowchecksums[ii+k]) {
					duplicate = false;
					break;
				}
			}
			if (!duplicate) {
				continue;
			}
			cerr << "\tDuplicate frames at rows " << i << " and " << ii << endl;
			for (int a=0; a<framesize; a++) {
				marked[i+a] = 1;
				marked[ii+a] = 2;
			}
			markImageDuplicateFrame(output, header, color, i, ii, framesize, j);
		}
		color++;
		color = color % 3;
	}


	/* print duplicates
	for (auto it = duplicates.begin(); it != duplicates.end(); it++) {
		if ((*it).second.count > 1) {
			int count = (*it).second.count;
			cout << (*it).first << "\t" << count << ":\t";
			for (int i=0; i<count; i++) {
				cout << (*it).second.rows[i];
				if (i < count - 1) {
					cout << ", ";
				}
			}
			cout << endl;
		}
	}
	*/


}



//////////////////////////////
//
// markImageDuplicateFrame -- Mark the duplicate with a half-row solid color.
//

void markImageDuplicateFrame(fstream& output, TiffHeader& header, int color,
		int firstrow, int otherrow, int framesize, int dupnum) {

	vector<char> pixel;
	pixel.resize(3);
	switch (color % 3) {
		case 0: pixel[0] = 0xff; pixel[1] = 0x00; pixel[2] = 0x00; break;
		case 1: pixel[0] = 0xff; pixel[1] = 0x99; pixel[2] = 0x33; break;
		case 2: pixel[0] = 0xff; pixel[1] = 0x00; pixel[2] = 0xff; break;
	}

	vector<char> quarterrow;
	int qsize = header.cols / 4;
	quarterrow.resize(qsize * 3);
	for (int i=0; i<qsize; i++) {
		quarterrow[3*i+0] = pixel[0];
		quarterrow[3*i+1] = pixel[1];
		quarterrow[3*i+2] = pixel[2];
	}

	int side = dupnum % 2;
	ulongint offset;

	if (dupnum == 1) {
		for (int i = 0; i<framesize; i++) {
			offset = header.dataoffset + (firstrow + i) * header.cols * 3;
			output.seekp(offset, output.beg);
			output.write(quarterrow.data(), qsize * 3);
		}
	}

	for (int i = 0; i<framesize; i++) {
		offset = header.dataoffset + (otherrow + i) * header.cols * 3 + side * 3 * qsize * 3;
		output.seekp(offset, output.beg);
		output.write(quarterrow.data(), qsize * 3);
	}
}



//////////////////////////////
//
// getRowCheckSums -- Calculate checksums for each row of the image.
//

void getRowCheckSums(vector<ulongint>& checksums, fstream& input, TiffHeader& header) {
	input.seekg(header.dataoffset);
	int rowbytecount = header.cols * 3;
	string rowbytes;

	checksums.resize(header.rows);
	ulongint crc;
	for (int i=0; i<header.rows; i++) {
		rowbytes = readString(input, rowbytecount);
    	crc = crc32_fast(rowbytes.data(), rowbytecount, 0);
		checksums[i] = crc;
	}
}




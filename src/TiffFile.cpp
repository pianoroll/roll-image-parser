//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Sat Nov 25 18:24:51 PST 2017
// Filename:      TiffFile.cpp
// Web Address:
// Syntax:        C++;
// vim:           ts=3:nowrap:ft=text
//
// Description:   TIFF file parser.
//
// References:
//      https://web.archive.org/web/20160306201233/http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf (page 13)
//      https://www.loc.gov/preservation/digital/formats/content/tiff_tags.shtml
//      https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
//


#include "TiffFile.h"


using namespace prp;
using namespace std;


//////////////////////////////
//
// TiffFile::TiffFile --
//

TiffFile::TiffFile(void) {

}



//////////////////////////////
//
// TiffFile::~TiffFile --
//

TiffFile::~TiffFile(void) {
	close();
}



//////////////////////////////
//
// TiffFile::close --
//

void TiffFile::close(void) {
	fstream::close();
	TiffHeader::clear();
}



//////////////////////////////
//
// TiffFile::open --
//

bool TiffFile::open(const string& filename) {
	if (is_open()) {
		close();
	}

	fstream::open(filename.c_str(), ios::binary | ios::in | ios::out);
	if (!is_open()) {
		cerr << "Input filename " << filename << " cannot be opened" << endl;
		return false;
	}

	m_filename = filename;
	return parseHeader(*this);
}



//////////////////////////////
//
// TiffFile::goToByteIndex --
//

bool TiffFile::goToByteIndex(ulonglongint offset) {
	if (offset <= (ulonglongint)0xffffffff) {
		seekg((ulongint)offset, ios::beg);
	} else {
		seekg((ulongint)0xffffffff, ios::beg);
		ulonglongint amount = offset - 0xffffffff;
		while (amount > (ulonglongint)0xffffffff) {
			seekg((ulongint)0xffffffff, ios::cur);
			amount -= (ulonglongint)0xffffffff;
		}
		if (amount > 0) {
			seekg((ulongint)amount, ios::cur);
		}
	}
	return true;
}



//////////////////////////////
//
// TiffFile::readLittleEndian2ByteUInt --
//

ushortint TiffFile::readLittleEndian2ByteUInt(void) {
	return prp::readLittleEndian2ByteUInt(*this);
}



//////////////////////////////
//
// TiffFile::read1UByte --
//

ucharint TiffFile::read1UByte(void) {
	return prp::read1UByte(*this);
}



//////////////////////////////
//
// TiffFile::readString --
//

string TiffFile::readString(ulongint count) {
	return prp::readString(*this, count);
}



//////////////////////////////
//
// TiffFile::readString -- Hard-coded to 24-bit pixels for now.
//

bool TiffFile::goToPixelIndex(ulonglongint pindex) {
	goToByteIndex(this->getDataOffset() + pindex * 3);
	return true;
}



//////////////////////////////
//
// TiffFile::goToRowColumnIndex -- Hard-coded to 24-bit pixels for now.
//

bool TiffFile::goToRowColumnIndex(ulongint rowindex, ulongint colindex) {
	ulonglongint offset = rowindex * 3 * this->getCols() + colindex * 3;
	goToByteIndex(this->getDataOffset() + offset);
	return true;
}



//////////////////////////////
//
// TiffFile::getImageGreenChannel --
//

void TiffFile::getImageGreenChannel(vector<vector<ucharint> >& image) {
	this->goToPixelIndex((int)0);
	ulongint rows = this->getRows();
	ulongint cols = this->getCols();
	vector<ucharint> pixel(3);
	image.resize(rows);
	for (ulongint r=0; r<rows; r++) {
		image.at(r).resize(cols);
		for (ulongint c=0; c<cols; c++) {
			pixel[0] = this->read1UByte();  // red
			pixel[1] = this->read1UByte();  // green
			pixel[2] = this->read1UByte();  // blue
			image.at(r).at(c) = pixel[1];
		}
	}
}



//////////////////////////////
//
// TiffFile::writeSamplesPerPixel -- 1 = monochrome, 3 = color.
//

bool TiffFile::writeSamplesPerPixel(int count) {
	return ((TiffHeader*)this)->writeSamplesPerPixel(*this, count);
}



//////////////////////////////
//
// TiffFile::writeDirectoryOffset --
//

void TiffFile::writeDirectoryOffset(ulonglongint offset) {
	((TiffHeader*)this)->writeDirectoryOffset(*this, offset);
}



//////////////////////////////
//
// TiffFile::getFilename --
//

std::string TiffFile::getFilename(void) {
	return m_filename;
}




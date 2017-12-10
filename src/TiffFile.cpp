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


namespace prp {


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
	m_input.close();
	clear();
}



//////////////////////////////
//
// TiffFile::openFile --
//

bool TiffFile::open(const std::string& filename) {
	if (m_input.is_open()) {
		close();
	}

	m_input.open(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out);
	if (!m_input.is_open()) {
		std::cerr << "Input filename " << filename << " cannot be opened" << std::endl;
		return false;
	}

	return parseHeader(m_input);
}



//////////////////////////////
//
// TiffFile::goToByteIndex --
//

bool TiffFile::goToByteIndex(ulong offset) {
	m_input.seekg(offset, m_input.beg);
	return true;
}



//////////////////////////////
//
// TiffFile::readLittleEndian2ByteUInt --
//

ushort TiffFile::readLittleEndian2ByteUInt(void) {
	return prp::readLittleEndian2ByteUInt(m_input);
}



//////////////////////////////
//
// TiffFile::read1UByte --
//

uchar TiffFile::read1UByte(void) {
	return prp::read1UByte(m_input);
}



//////////////////////////////
//
// TiffFile::readString --
//

std::string TiffFile::readString(ulong count) {
	return prp::readString(m_input, count);
}



//////////////////////////////
//
// TiffFile::readString -- Hard-coded to 24-bit pixels for now.
//

bool TiffFile::goToPixelIndex(ulong pindex) {
	m_input.seekg(this->getDataOffset() + pindex * 3);
	return true;
}



//////////////////////////////
//
// TiffFile::goToRowColumnIndex -- Hard-coded to 24-bit pixels for now.
//

bool TiffFile::goToRowColumnIndex(ulong rowindex, ulong colindex) {
	ulong offset = rowindex * 3 * this->getCols() + colindex * 3;
	m_input.seekg(this->getDataOffset() + offset);
	return true;
}



//////////////////////////////
//
// TiffFile::getImageGreenChannel -- 
//

void TiffFile::getImageGreenChannel(std::vector<std::vector<uchar>>& image) {
	this->goToPixelIndex(0);
	ulong rows = this->getRows();
	ulong cols = this->getCols();
	std::vector<uchar> pixel(3);
	image.resize(rows);
	for (ulong r=0; r<rows; r++) {
		image.at(r).resize(cols);
		for (ulong c=0; c<cols; c++) {
			pixel[0] = this->read1UByte();  // red
			pixel[1] = this->read1UByte();  // green
			pixel[2] = this->read1UByte();  // blue
			image.at(r).at(c) = pixel[2];
		}
	}
}




} // end of namespace prp




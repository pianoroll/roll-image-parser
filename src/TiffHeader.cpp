//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Sat Nov 25 12:29:32 PST 2017
// Filename:      TiffHeader.cpp
// Web Address:   
// Syntax:        C++;
// vim:           ts=3:nowrap:ft=text
//
// Description:   TIFF file header.
//
// References:
//      https://web.archive.org/web/20160306201233/http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf (page 13)
//      https://www.loc.gov/preservation/digital/formats/content/tiff_tags.shtml
//      https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
//


#include "TiffHeader.h"


namespace prp {


//////////////////////////////
//
// TiffHeader::TiffHeader --
//

TiffHeader::TiffHeader(void) {
	clear();
}



//////////////////////////////
//
// TiffHeader::~TiffHeader --
//

TiffHeader::~TiffHeader() {
	clear();
}



//////////////////////////////
//
// TiffHeader::clear --
//

void TiffHeader::clear(void) {
	m_rows        = 0;
	m_cols        = 0;
	m_orientation = -1;
	m_dataoffset  = 0;
	m_databytes   = 0;
	m_rowdpi      = 0.0;
	m_coldpi      = 0.0;
}



//////////////////////////////
//
// TiffHeader::getRows --
//

ulong TiffHeader::getRows(void) const {
	return m_rows;
}



//////////////////////////////
//
// TiffHeader::setRows --
//

void TiffHeader::setRows(ulong value) {
	m_rows = value;
}



//////////////////////////////
//
// TiffHeader::getCols --
//

ulong TiffHeader::getCols(void) const {
	return m_cols;
}



//////////////////////////////
//
// TiffHeader::setCols --
//

void TiffHeader::setCols(ulong value) {
	m_cols = value;
}



//////////////////////////////
//
// TiffHeader::setOrientation --
//

void TiffHeader::setOrientation(int value) {
	m_orientation = value;
}



//////////////////////////////
//
// TiffHeader::getOrientation --
//

int TiffHeader::getOrientation(void) const {
	return m_orientation;
}



//////////////////////////////
//
// TiffHeader::getDataOffset --
//

ulong TiffHeader::getDataOffset(void) const {
	return m_dataoffset;
}



//////////////////////////////
//
// TiffHeader::setDataOffset --
//

void TiffHeader::setDataOffset(ulong value) {
	m_dataoffset = value;
}



//////////////////////////////
//
// TiffHeader::getDataBytes --
//

ulong TiffHeader::getDataBytes(void) const {
	return m_databytes;
}



//////////////////////////////
//
// TiffHeader::setDataBytes --
//

void TiffHeader::setDataBytes(ulong value) {
	m_databytes = value;
}



//////////////////////////////
//
// TiffHeader::getRowDpi --
//

double TiffHeader::getRowDpi(void) const {
	return m_rowdpi;
}



//////////////////////////////
//
// TiffHeader::setRowDpi --
//

void TiffHeader::setRowDpi(double value) {
	m_rowdpi = value;
}



//////////////////////////////
//
// TiffHeader::getColDpi --
//

double TiffHeader::getColDpi(void) const {
	return m_coldpi;
}



//////////////////////////////
//
// TiffHeader::setColDpi --
//

void TiffHeader::setColDpi(double value) {
	m_coldpi = value;
}



//////////////////////////////
//
// TiffHeader::parseHeader -- Presumes that the input is at its beginning.
//

bool TiffHeader::parseHeader(std::fstream& input) {

	// Read 2-byte format code.  Required for now to be "II" for little-std::endian.
	std::string format = readString(input, 2);
	if (format != "II") {
		std::cerr << "File format must be little-std::endian" << std::endl;
		std::cerr << "Format should be 'II', but is instead '" << format << "'." << std::endl;
		return false;
	}

	// Read file-type magic number.  Required to be 42 for TIFF.
	ushort filetype = readLittleEndian2ByteUInt(input);
	if (filetype != 42) {
		std::cerr << "File type must be 42, but is instead " << filetype << std::endl;
		return false;
	}

	// byte offset of first directory
	ulong diroffset = readLittleEndian4ByteUInt(input);

	bool status = parseDirectory(input, diroffset);
	if (!status) {
		clear();
		return false;
	}

	long long expected = this->getRows() * this->getCols() * 3;
	if (expected != (long long)this->getDataBytes()) {
		std::cerr << "WARNING: image size does not match header information." << std::endl;
		std::cerr << "STRIP BYTE COUNT " << this->getDataBytes() << std::endl;
		std::cerr << "EXPECTED BYTE COUNT " << expected<< std::endl;
		std::cerr << "DIFFERENCE " << (long long)this->getDataBytes() - expected << std::endl;
	}

	return true;
}



//////////////////////////////
//
// parseDirectory -- Read data parameters for a TIFF directory structure.
//

bool TiffHeader::parseDirectory(std::fstream& input, ulong diroffset) {
	// jump to the header beginning:
	input.seekg(diroffset, input.beg);

	// Number of directory entries (parameters).  Each entry is 12 bytes long.
	int entrycount = readLittleEndian2ByteUInt(input);

	for (int i=0; i<entrycount; i++) {
		if (!readDirectoryEntry(input)) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// readDirectoryEntry -- Read header parameters from TIFF image.
//

bool TiffHeader::readDirectoryEntry(std::fstream& input) {
	// get the parameter type
	int id = readLittleEndian2ByteUInt(input);

	// get the data type
	int datatype = readLittleEndian2ByteUInt(input);

	// get number of values in parameter
	int count = readLittleEndian4ByteUInt(input);

	if (count > 3) {
		std::cerr << "LARGE COUNT IS " << count << " FOR ID " << id << std::endl;
	}

	// There are four bytes left in entry that need to be
	// processed by the following switch:

	ulong value;
	ulong position;
	switch (id) {

		case 256: // image width (columns)
			this->setCols(readEntryUInt(input, datatype, count));
			break;

		case 257: // image height (rows)
			this->setRows(readEntryUInt(input, datatype, count));
			break;

		case 258: // bits per sample (3 numbers)
			// Currently ignoring, assuming 8 bits/samples
			readLittleEndian4ByteUInt(input);
			break;

		case 259: // compression scheme
			value = readEntryUInt(input, datatype, count);
			// must be "1" for no compression
			if (value != 1) {
				std::cerr << "Error: Cannot deal with image compression" << std::endl;
				return false;
			}
			break;

		case 262: // photometric interpretation
			// Shouldn't be needed, require to be 2: 0,0,0=black 255,255,255=white
			value = readEntryUInt(input, datatype, count);
			if (value != 2) {
				std::cerr << "Cannot handle photometric interpretation " << value << "." << std::endl;
				return false;
			}
			break;

		case 273: // strip offset
			this->setDataOffset(readEntryUInt(input, datatype, count));
			break;

		case 274: // orientation
			this->setOrientation(readEntryUInt(input, datatype, count));
			break;

		case 277: // samples per pixel
			value = readEntryUInt(input, datatype, count);
			if (value != 3) {
				std::cerr << "Error image must be full color." << std::endl;
				return false;
			}
			break;

		case 279: // strip byte counts
			this->setDataBytes(readEntryUInt(input, datatype, count));
			break;

		case 282: // horizontal dpi
			position = input.tellg();
			this->setColDpi(readType5Value(input, datatype, count));
			input.seekg(position + 4, input.beg);
			break;

		case 283: // vertical dpi
			position = input.tellg();
			this->setRowDpi(readType5Value(input, datatype, count));
			input.seekg(position + 4, input.beg);
			break;

		case 284: // planar configuration
			value = readEntryUInt(input, datatype, count);
			if (value != 1) {
				std::cerr << "Can only handle contiguous pixel data." << std::endl;
				return false;
			}
			break;

		case 296: // resolution units
			value = readEntryUInt(input, datatype, count);
			if (value != 2) {
				std::cerr << "Expecting resolution units to be in inches." << std::endl;
				return false;
			}
			break;

		default:  // ignore unknown parameters
			value = readLittleEndian4ByteUInt(input);
			std::cerr << "UNKNOWN ID TYPE " << id 
			     << " datatype " << datatype << " count " << count << " value " << value << std::endl;
	}

	return true;
}



//////////////////////////////
//
// TiffHeader::getPixelOffset --
//

ulong TiffHeader::getPixelOffset(ulong pindex) const {
	return getDataOffset() + 3 * pindex;
}


ulong TiffHeader::getPixelOffset(ulong rindex, ulong cindex) const {
	return this->getDataOffset() + 3 * rindex * this->getCols() + 3 * cindex;
}


//////////////////////////////
//
// TiffHeader::getPixelCount --
//

ulong TiffHeader::getPixelCount(void) const {
	return getRows() * getCols();
}



} // std::end namespace prp



//////////////////////////////
//
// operator<< --
//

std::ostream& operator<<(std::ostream& out, const prp::TiffHeader& header) {
	out << "ROWS:\t"           << header.getRows()                     << std::endl;
	out << "COLS:\t"           << header.getCols()                     << std::endl;
	out << "ORIENTATION:\t"    << header.getOrientation()              << std::endl;
	out << "HORIZONTAL DPI:\t" << header.getColDpi() << " pixels/inch" << std::endl;
	out << "VERTICAL DPI:\t"   << header.getRowDpi() << " pixels/inch" << std::endl;
	out << "DATA OFFSET:\t"    << header.getDataOffset()               << std::endl;
	out << "DATA BYTES:\t"     << header.getDataBytes()                << std::endl;
	return out;
}





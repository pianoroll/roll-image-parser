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
	m_64bitQ      = false;
}



//////////////////////////////
//
// TiffHeader::getRows --
//

ulongint TiffHeader::getRows(void) const {
	return m_rows;
}



//////////////////////////////
//
// TiffHeader::setRows --
//

void TiffHeader::setRows(ulongint value) {
	m_rows = value;
}



//////////////////////////////
//
// TiffHeader::getCols --
//

ulongint TiffHeader::getCols(void) const {
	return m_cols;
}



//////////////////////////////
//
// TiffHeader::setCols --
//

void TiffHeader::setCols(ulongint value) {
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

ulonglongint TiffHeader::getDataOffset(void) const {
	return m_dataoffset;
}



//////////////////////////////
//
// TiffHeader::setDataOffset --
//

void TiffHeader::setDataOffset(ulonglongint value) {
	m_dataoffset = value;
}



//////////////////////////////
//
// TiffHeader::setBigTiff --
//

void TiffHeader::setBigTiff(void) {
	m_64bitQ = true;
}



//////////////////////////////
//
// TiffHeader::isBigTiff --
//

bool TiffHeader::isBigTiff(void) {
	return m_64bitQ;
}



//////////////////////////////
//
// TiffHeader::getDataBytes --
//

ulonglongint TiffHeader::getDataBytes(void) const {
	return m_databytes;
}



//////////////////////////////
//
// TiffHeader::setDataBytes --
//

void TiffHeader::setDataBytes(ulonglongint value) {
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
   // These are hex bytes "4D 4D".
	std::string format = readString(input, 2);
	if (format != "II") {
		std::cerr << "File format must be little-std::endian" << std::endl;
		std::cerr << "Format should be 'II', but is instead '" << format << "'." << std::endl;
		return false;
	}

	// Read file-type magic number.  Required to be 0x2A for 32-bit TIFF,
	// or 0x2B for 64-bit TIFF:
	ushortint filetype = readLittleEndian2ByteUInt(input);
	if (filetype == 0x2A) {
		// do nothing
	} else if (filetype == 0x2B) {
		this->setBigTiff();
	} else {
		std::cerr << "File type must be 42, but is instead " << filetype << std::endl;
		return false;
	}


	// bigTiff images have two extra parameters: 2-byte count of bytes in offsets
	// and 2-byte constant 0x0000:
	if (this->isBigTiff()) {
		short value = readLittleEndian2ByteUInt(input);
		if (value != 8) {
			std::cerr << "Strange offset size: " << value << " bytes" << std::endl;
			return false;
		}
		value = readLittleEndian2ByteUInt(input);
		// value should now be 0, but actual number is not important.
	}

	// byte offset of first directory
	ulonglongint diroffset;
	if (this->isBigTiff()) {
		diroffset = readLittleEndian8ByteUInt(input);
	} else {
		diroffset = readLittleEndian4ByteUInt(input);
	}

	bool status = parseDirectory(input, diroffset);
	if (!status) {
		clear();
		return false;
	}

	ulonglongint expected = (ulonglongint)this->getRows() * (ulonglongint)this->getCols() * 3;
	if (expected != (ulonglongint)this->getDataBytes()) {
		std::cerr << "WARNING: image size does not match header information." << std::endl;
		std::cerr << "STRIP BYTE COUNT " << this->getDataBytes() << std::endl;
		std::cerr << "EXPECTED BYTE COUNT " << expected << std::endl;
		std::cerr << "DIFFERENCE " << (longlongint)this->getDataBytes() - expected << std::endl;
	}

	return true;
}



//////////////////////////////
//
// parseDirectory -- Read data parameters for a TIFF directory structure.
//

bool TiffHeader::parseDirectory(std::fstream& input, ulonglongint diroffset) {
	// jump to the header beginning:
	goToByteIndex(input, diroffset);

	// Number of directory entries (parameters).
	// Each entry is 12 bytes long for 32-bit TIFFs and 20 bytes for 64-bt TIFFs
	ulonglongint entrycount;
	if (this->isBigTiff()) {
		entrycount = readLittleEndian8ByteUInt(input);
	} else {
		entrycount = readLittleEndian2ByteUInt(input);
	}

	for (ulonglongint i=0; i<entrycount; i++) {
		if (!readDirectoryEntry(input)) {
			return false;
		}
	}
	return true;
}



//////////////////////////////
//
// TiffHeader::goToByteIndex --
//

bool TiffHeader::goToByteIndex(std::fstream& input, ulongint offset) {
	input.seekg(offset, input.beg);
	return true;
}

bool TiffHeader::goToByteIndex(std::fstream& input, ulonglongint offset) {
	if (offset <= (ulonglongint)0xffffffff) {
		input.seekg((ulongint)offset, input.beg);
	} else {
		input.seekg((ulongint)0xffffffff, input.beg);
		ulonglongint amount = offset - 0xffffffff;
		while (amount > (ulonglongint)0xffffffff) {
			input.seekg((ulongint)0xffffffff, input.cur);
			amount -= (ulonglongint)0xffffffff;
		}
		if (amount > 0) {
			input.seekg((ulongint)amount, input.cur);
		}
	}
	return true;
}



//////////////////////////////
//
// readDirectoryEntry -- Read header parameters from TIFF image.
//

bool TiffHeader::readDirectoryEntry(std::fstream& input) {
	// get the parameter type (tag)
	int id = readLittleEndian2ByteUInt(input);

	// get the data type
	int datatype = readLittleEndian2ByteUInt(input);

	// get number of values in parameter
	ulonglongint count;
	if (this->isBigTiff()) {
		count = readLittleEndian8ByteUInt(input);
	} else {
		count = readLittleEndian4ByteUInt(input);
	}

	if (count > 3) {
		if (!((id == 273) || (id == 279))) {
			std::cerr << "LARGE COUNT IS " << count << " FOR ID " << id << std::endl;
		}
	}

	// There are four bytes (or eight bytes) left in entry that need to be
	// processed by the following switch:

	ulongint value;
	switch (id) {

		case 256: // image width (columns)
			this->setCols((ulongint)this->readEntryUInteger(input, datatype, count, id));
			break;

		case 257: // image height (rows)
			this->setRows((ulongint)this->readEntryUInteger(input, datatype, count, id));
			break;

		case 258: // bits per sample (3 numbers)
			// Currently ignoring, assuming 8 bits/samples, so read the
			// parameter data byte offset and throw it away:
			if (this->isBigTiff()) {
				readLittleEndian8ByteUInt(input);
			} else {
				readLittleEndian4ByteUInt(input);
			}
			break;

		case 259: // compression scheme
			value = (ulongint)this->readEntryUInteger(input, datatype, count, id);
			// must be "1" for no compression
			if (value != 1) {
				std::cerr << "Error: Cannot deal with image compression" << std::endl;
				return false;
			}
			break;

		case 262: // photometric interpretation
			// Shouldn't be needed, require to be 2: 0,0,0=black 255,255,255=white
			value = (ulongint)this->readEntryUInteger(input, datatype, count, id);
			if (value != 2) {
				std::cerr << "Cannot handle photometric interpretation " << value << "." << std::endl;
				return false;
			}
			break;

		case 266: // Tag fill order.  Should be 1 if parameter exists
			value = (ulongint)this->readEntryUInteger(input, datatype, count, id);
			if (value != 1) {
				std::cerr << "Unknown Tag fill order: " << value << std::endl;
				return false;
			}
			break;

		case 273: // strip offset
			this->setDataOffset(this->readEntryUInteger(input, datatype, count, id));
			break;

		case 274: // orientation
			this->setOrientation((ulongint)this->readEntryUInteger(input, datatype, count, id));
			break;

		case 277: // samples per pixel
			value = (ulongint)this->readEntryUInteger(input, datatype, count, id);
			if (value != 3) {
				std::cerr << "Error image must be full color." << std::endl;
				return false;
			}
			break;

		case 278: // rows per strip
			value = (ulongint)this->readEntryUInteger(input, datatype, count, id);
			// Currently ignoring this value and assuming the strip
			// count is 1 and the rows per strip is the rows of the image.
			break;

		case 279: // strip byte counts
			this->setDataBytes(this->readEntryUInteger(input, datatype, count, id));
			break;

		case 282: // horizontal dpi
			this->setColDpi(this->readType5Value(input, datatype, count));
			break;

		case 283: // vertical dpi
			this->setRowDpi(this->readType5Value(input, datatype, count));
			break;

		case 284: // planar configuration
			value = (ulongint)this->readEntryUInteger(input, datatype, count, id);
			if (value != 1) {
				std::cerr << "Can only handle contiguous pixel data." << std::endl;
				return false;
			}
			break;

		case 296: // resolution units
			value = (ulongint)this->readEntryUInteger(input, datatype, count, id);
			if (value != 2) {
				std::cerr << "Expecting resolution units to be in inches." << std::endl;
				return false;
			}
			break;

		// case 297: // The page number of the page from which this images was scanned.
		// case 319: // The chromaticities of the primaries of the image.

		default:  // ignore unknown parameters
			value = readLittleEndian4ByteUInt(input);
			if (this->isBigTiff()) {
				// read next four bytes as well (presume 4 all are zeros)
				readLittleEndian4ByteUInt(input);
			}
			std::cerr << "UNKNOWN ID TYPE " << id
			     << " datatype " << datatype << " count " << count << " value " << value << std::endl;
	}

	return true;
}



//////////////////////////////
//
// TiffHeader::readEntryUInteger -- Read a short or long or long long in 4-byte
//      or 8-byte location in file.  Throw away any padding bytes that come after
//      the width of the data.
//

ulonglongint TiffHeader::readEntryUInteger(std::fstream& input, int datatype,
		ulonglongint count, int tag) {
	if (count != 1) {
		if (!((tag == 273) || (tag == 279))) {
			std::cerr << "Problem2 reading value, bad parameter count: " << count << std::endl;
			std::cerr << "TAG IS " << tag << std::endl;
			exit(1);
		}
	}

	ulonglongint output = 0;

	if ((tag == 273) && (count > 1)) {
		// read an offset to the offset, and then read that offset
		// (assuming all "strips" are contiguous).  This case
		// is needed for libtiff where it lists an offset for each line
		// of the image in this area.  Only reading first offset from list.

		if (this->isBigTiff()) {
			ulonglongint valueoffset = readLittleEndian8ByteUInt(input);
			ulonglongint position = input.tellg();
			this->goToByteIndex(input, valueoffset);
			output = readLittleEndian8ByteUInt(input);
			this->goToByteIndex(input, position);
		} else {
			ulonglongint valueoffset = readLittleEndian4ByteUInt(input);
			ulonglongint position = input.tellg();
			this->goToByteIndex(input, valueoffset);
			output = readLittleEndian4ByteUInt(input);
			this->goToByteIndex(input, position);
		}

	} else if ((tag == 279) && (count > 1)) {
		// Need this case for multiple strips.  Assume each strip is the same size.

		if (this->isBigTiff()) {
			ulonglongint valueoffset = readLittleEndian8ByteUInt(input);
			ulonglongint position = input.tellg();
			this->goToByteIndex(input, valueoffset);
			output = count * readLittleEndian8ByteUInt(input);
			this->goToByteIndex(input, position);
		} else {
			ulonglongint valueoffset = readLittleEndian4ByteUInt(input);
			ulonglongint position = input.tellg();
			this->goToByteIndex(input, valueoffset);
			output = count * readLittleEndian4ByteUInt(input);
			this->goToByteIndex(input, position);
		}

	} else if (datatype == 3) {  // unsigned short
		output = readLittleEndian2ByteUInt(input);
		// skip over buffer bytes
		if (this->isBigTiff()) {
			readLittleEndian2ByteUInt(input);
			readLittleEndian4ByteUInt(input);
		} else {
			readLittleEndian2ByteUInt(input);
		}
	} else if (datatype == 4) { // unsigned long
		output = readLittleEndian4ByteUInt(input);
		// skip over buffer bytes
		if (this->isBigTiff()) {
			readLittleEndian4ByteUInt(input);
		}
	} else {
		std::cerr << "Unknown directory entry data type: " << datatype << std::endl;
		exit(1);
	}

	return output;
}



//////////////////////////////
//
// TiffHeader::readType5Value -- read a double expressed as two 4-byte unsigned longs.
//

double TiffHeader::readType5Value(std::fstream& input, int datatype,
		ulonglongint count, int tag) {
	if (count != 1) {
		std::cerr << "Problem1 reading value, bad parameter count: " << count << std::endl;
		exit(1);
	}
	if (datatype != 5) {
		std::cerr << "Wrong data type for reading a double value: " << datatype << "." << std::endl;
		exit(1);
	}


	double value = -1.0;
	if (this->isBigTiff()) {
		ulongint top = readLittleEndian4ByteUInt(input);
		ulongint bot = readLittleEndian4ByteUInt(input);
		value = (double)top / (double)bot;
	} else {
		ulonglongint offset;
		offset = readLittleEndian4ByteUInt(input);
 		ulonglongint position = input.tellg();
		goToByteIndex(input, offset);
		ulongint top = readLittleEndian4ByteUInt(input);
		ulongint bot = readLittleEndian4ByteUInt(input);
		// go back to after the entry offset value:
		goToByteIndex(input, position);
		value = (double)top / (double)bot;
	}

	return value;
}



//////////////////////////////
//
// TiffHeader::getPixelOffset --
//

ulongint TiffHeader::getPixelOffset(ulongint pindex) const {
	return getDataOffset() + 3 * pindex;
}


ulongint TiffHeader::getPixelOffset(ulongint rindex, ulongint cindex) const {
	return this->getDataOffset() + 3 * rindex * this->getCols() + 3 * cindex;
}


//////////////////////////////
//
// TiffHeader::getPixelCount --
//

ulongint TiffHeader::getPixelCount(void) const {
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





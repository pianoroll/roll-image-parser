//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Sat Nov 25 16:51:23 PST 2017
// Filename:      Utilities.cpp
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Utility functions for working with TIFF images.
//


#include "Utilities.h"

namespace prp {


//////////////////////////////
//
// readLittleEndian8ByteUInt -- Read eight-byte int which is in
//      little-endian order (smallest byte is first).
//

ulonglongint readLittleEndian8ByteUInt(std::istream& input) {
   ucharint buffer[8];
   input.read((char*)buffer, 8);
   if (input.eof()) {
      std::cerr << "Error: unexpected end of file." << std::endl;
      return 0;
   }
	ulonglongint output = buffer[7];
	output = (output << 8) | buffer[6];
	output = (output << 8) | buffer[5];
	output = (output << 8) | buffer[4];
	output = (output << 8) | buffer[3];
	output = (output << 8) | buffer[2];
	output = (output << 8) | buffer[1];
	output = (output << 8) | buffer[0];
   return output;
}



//////////////////////////////
//
// readLittleEndian4ByteUInt -- Read four-byte int which is in
//      little-endian order (smallest byte is first).
//

ulongint readLittleEndian4ByteUInt(std::istream& input) {
   ucharint buffer[4];
   input.read((char*)buffer, 4);
   if (input.eof()) {
      std::cerr << "Error: unexpected end of file." << std::endl;
      return 0;
   }
	ulongint output = buffer[3];
	output = (output << 8) | buffer[2];
	output = (output << 8) | buffer[1];
	output = (output << 8) | buffer[0];
   return output;
}



//////////////////////////////
//
// readLittleEndian2ByteUInt -- Read two-byte int which is in
//      little-endian order (smallest byte is first).
//

ushort readLittleEndian2ByteUInt(std::istream& input) {
   ucharint buffer[2];
   input.read((char*)buffer, 2);
   if (input.eof()) {
      std::cerr << "Error: unexpected end of file." << std::endl;
      return 0;
   }
	ushort output = buffer[1];
	output = (output << 8) | buffer[0];
   return output;
}



//////////////////////////////
//
// read1UByte -- Read a single byte from the current position in the file stream.
//

ucharint  read1UByte(std::istream& input) {
	ucharint buffer[1];
	input.read((char*)buffer, 1);
   if (input.eof()) {
      std::cerr << "Error: unexpected end of file." << std::endl;
      return 0;
   }
   return buffer[0];
}



//////////////////////////////
//
// readString -- Read string of characters with the given length.
//

std::string readString(std::istream& input, int count) {
	std::string output;
	output.resize(count);
   input.read((char*)(&output[0]), count);
   if (input.eof()) {
      std::cerr << "Error: unexpected end of file." << std::endl;
      return "";
   }
   return output;
}



//////////////////////////////
//
// aboveThreshold -- Returns true if the value is above (or equal) to
//    given threshold value.
//

bool aboveThreshold(ucharint value, ucharint threshold) {
	if (value >= threshold) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// maxValueIndex -- return the index of the largest value in the list.
//

ulongint maxValueIndex(std::vector<ulongint> array) {
	if (array.empty()) {
		return 0;
	}
	ulongint maxindex = 0;
	for (ulongint i=1; i<array.size(); i++) {
		if (array[i] > array[maxindex]) {
			maxindex = i;
		}
	}

	return maxindex;
}



//////////////////////////////
//
// getMaximum --
//

int getMaximum(std::vector<int>& array, ulongint startindex, ulongint length) {
	ulongint stopindex = array.size() - 1;
	if (length > 0) {
		stopindex = startindex + length;
	}
	int maxi = startindex;
	for (ulongint r=startindex+1; r<=stopindex; r++) {
		if (array[r] > array[maxi]) {
			array[maxi] = array[r];
		}
	}
	return array[maxi];
}



//////////////////////////////
//
// exponentialSmoothing -- Filter applied twice: in forward and
//    backwards direction.
//

void exponentialSmoothing(std::vector<double>& array, double gain) {
	double k = gain;
	double nk = 1.0 - k;
	for (ulongint i=1; i<array.size(); i++) {
		array[i] = k * array[i] + nk * array[i-1];
	}
	for (int i=array.size()-2; i>=0; i--) {
		array[i] = k * array[i] + nk * array[i+1];
	}
}



//////////////////////////////
//
// goToByteIndex -- Generalized to work with files larger than 4GB, especially
//    when seekg cannot handle 64-bit integers.
//

bool goToByteIndex(std::fstream& file, ulonglongint offset) {
	if (offset <= (ulonglongint)0xffffffff) {
		file.seekg((ulongint)offset, std::ios::beg);
	} else {
		file.seekg((ulongint)0xffffffff, std::ios::beg);
		ulonglongint amount = offset - 0xffffffff;
		while (amount > (ulonglongint)0xffffffff) {
			file.seekg((ulongint)0xffffffff, std::ios::cur);
			amount -= (ulonglongint)0xffffffff;
		}
		if (amount > 0) {
			file.seekg((ulongint)amount, std::ios::cur);
		}
	}
	return true;
}



} // end namespace prp



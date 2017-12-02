//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Sat Nov 25 16:51:23 PST 2017
// Filename:      TiffUtilities.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Utility functions for working with TIFF images.
//


#include "TiffUtilities.h"

namespace tiff {


//////////////////////////////
//
// readType5Value -- read a double expressed as two 4-byte unsigned longs.
//

double readType5Value(std::fstream& input, int datatype, int count) {
	if (count != 1) {
		std::cerr << "Problem1 reading value, bad parameter count: " << count << std::endl;
		exit(1);
	}
	if (datatype != 5) {
		std::cerr << "Wrong data type for reading a double value: " << datatype << "." << std::endl;
		exit(1);
	}
	ulong offset = readLittleEndian4ByteUInt(input);
	input.seekg(offset, input.beg);
	int top = readLittleEndian4ByteUInt(input);
	int bot = readLittleEndian4ByteUInt(input);
	return (double)top / (double)bot;
}



//////////////////////////////
//
// readEntryUInt -- Read a short or long in 4-byte location in file.
//      Throw away any padding bytes.
//

ulong readEntryUInt(std::fstream& input, int datatype, int count) {
	if (count != 1) {
		std::cerr << "Problem2 reading value, bad parameter count: " << count << std::endl;
		exit(1);
	}
	ulong output = 0;
	if (datatype == 3) {  // unsigned short
		output = readLittleEndian2ByteUInt(input);
		// skip over buffer bytes
		readLittleEndian2ByteUInt(input);
	} else if (datatype == 4) { // unsigned long
		output = readLittleEndian4ByteUInt(input);
	} else {
		std::cerr << "Unknown directory entry data type: " << datatype << std::endl;
		exit(1);
	}
	return output;
}



//////////////////////////////
//
// readLittleEndian4ByteUInt -- Read four-byte int which is in
//      little-endian order (smallest byte is first).
//

ulong readLittleEndian4ByteUInt(std::istream& input) {
   uchar buffer[4];
   input.read((char*)buffer, 4);
   if (input.eof()) {
      std::cerr << "Error: unexpected end of file." << std::endl;
      return 0;
   }
	ulong output = buffer[3];
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
   uchar buffer[2];
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

uchar  read1UByte(std::istream& input) {
	uchar buffer[1];
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

bool aboveThreshold(uchar value, uchar threshold) {
	if (value >= threshold) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// getAverage --
//

double getAverage(std::vector<int>& array, ulong startindex, ulong length) {
	ulong stopindex = array.size() - 1;
	if (length > 0) {
		stopindex = startindex + length;
	}
	int newlength =  stopindex - startindex;
	double average = 0.0;
	for (ulong r=startindex; r<=stopindex; r++) {
		average += (double)array.at(r) / (double)newlength;
	}
	return average;
}



//////////////////////////////
//
// maxValueIndex -- return the index of the largest value in the list.
//

ulong maxValueIndex(std::vector<ulong> array) {
	if (array.empty()) {
		return 0;
	}
	ulong maxindex = 0;
	for (ulong i=1; i<array.size(); i++) {
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

int getMaximum(std::vector<int>& array, ulong startindex, ulong length) {
	ulong stopindex = array.size() - 1;
	if (length > 0) {
		stopindex = startindex + length;
	}
	int maxi = startindex;
	for (ulong r=startindex+1; r<=stopindex; r++) {
		if (array[r] > array[maxi]) {
			array[maxi] = array[r];
		}
	}
	return array[maxi];
}




} // end namespace tiff



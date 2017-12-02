//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Thu Nov 23 11:47:50 PST 2017
// Filename:      TiffUtilities.h
// Web Address:   
// Syntax:        C++; 
// vim:            ts=3:nowrap
//
// Description:   Basic functions for processing TIFF images.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace tiff {

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

// File reading functions
ulong       readLittleEndian4ByteUInt  (std::istream& input);
ushort      readLittleEndian2ByteUInt  (std::istream& input);
uchar       read1UByte                 (std::istream& input);
std::string readString                 (std::istream& input, int count);
ulong       readEntryUInt              (std::fstream& input, int datatype, int count);
double      readType5Value             (std::fstream& input, int datatype, int count);

// misc. utility functions:
bool        aboveThreshold             (uchar value, uchar threshold);
double      getAverage                 (std::vector<int>& array, ulong startindex = 0,
                                        ulong length = 0);
int         getMaximum                 (std::vector<int>& array, ulong startindex = 0, 
                                        ulong length = 0);
ulong       maxValueIndex              (std::vector<ulong> array);


} // end namespace tiff



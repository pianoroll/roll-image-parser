//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Thu Nov 23 11:47:50 PST 2017
// Filename:      Utilities.h
// Web Address:   
// Syntax:        C++; 
// vim:            ts=3:nowrap:ft=text
//
// Description:   Basic functions for processing TIFF images.
//
//
#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>

namespace tiff {

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef std::pair<double, double> mycomplex;

// File reading functions
ulong       readLittleEndian4ByteUInt  (std::istream& input);
ushort      readLittleEndian2ByteUInt  (std::istream& input);
uchar       read1UByte                 (std::istream& input);
std::string readString                 (std::istream& input, int count);
ulong       readEntryUInt              (std::fstream& input, int datatype, int count);
double      readType5Value             (std::fstream& input, int datatype, int count);

// misc. utility functions:
bool        aboveThreshold             (uchar value, uchar threshold);

int         getMaximum                 (std::vector<int>& array, ulong startindex = 0, 
                                        ulong length = 0);

ulong       maxValueIndex              (std::vector<ulong> array);

template <class TYPE>
double      getAverage                 (std::vector<TYPE>& array, ulong startindex = 0,
                                        ulong length = 0);


// FFT functions

void FFT(std::vector<mycomplex>& output, std::vector<mycomplex>& input);
void shuffle(std::vector<mycomplex>& X);
void dftmerge(std::vector<mycomplex>& XF);
mycomplex cexp(mycomplex& Z);
void swap(mycomplex& a, mycomplex& b);
int bitrev(int n, int B);
void fft_destructive(std::vector<mycomplex>& X);
mycomplex operator*(mycomplex& a, mycomplex& b);
mycomplex operator+(mycomplex& a, mycomplex& b);
mycomplex operator-(mycomplex& a, mycomplex& b);
bool isPowerOfTwo(ulong value);
#define two(x) (1 << (x))	 /* 2**x by left-shifting */
double magnitude(mycomplex& input);




///////////////////////////////////////////////////////////////////////////
//
//  Templates --
//


//////////////////////////////
//
// getAverage --
//

template <class TYPE>
double getAverage(std::vector<TYPE>& array, ulong startindex, ulong length) {
	ulong stopindex = array.size() - 1;
	if (length > 0) {
		stopindex = startindex + length - 1;
	}
	int newlength =  stopindex - startindex + 1;
	double average = 0.0;
	for (ulong r=startindex; r<=stopindex; r++) {
		average += (double)array.at(r);
	}
	return average / (double)newlength;
}



} // end namespace tiff


#endif /* _UTILITIES_H */

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Thu Feb 15 06:08:56 PST 2018
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

#include "FFT.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>

namespace prp {

//typedef int64_t  longlongint;
//typedef uint64_t ulonglongint;
//typedef uint32_t ulongint;
//typedef uint16_t ushortint;
//typedef uint8_t  ucharint;

typedef long long longlongint;
typedef unsigned long long ulonglongint;
typedef unsigned long ulongint;
typedef unsigned short ushortint;
typedef unsigned char ucharint;

// File reading functions
ulonglongint   readLittleEndian8ByteUInt  (std::istream& input);
ulongint       readLittleEndian4ByteUInt  (std::istream& input);
ushortint      readLittleEndian2ByteUInt  (std::istream& input);
ucharint       read1UByte                 (std::istream& input);
std::string    readString                 (std::istream& input, int count);

// File writing functions
void           writeLittleEndian8ByteUInt (std::ostream& output, ulonglongint);
void           writeLittleEndian4ByteUInt (std::ostream& output, ulongint);
void           writeLittleEndian2ByteUInt (std::ostream& output, ushortint);
void           write1UByte                (std::ostream& output, ucharint);
void           writeString                (std::ostream& output, std::string data);

// misc. utility functions:
bool           aboveThreshold             (ucharint value, ucharint threshold);
int            getMaximum                 (std::vector<int>& array, ulongint startindex = 0, 
                                           ulongint length = 0);
ulongint       maxValueIndex              (std::vector<ulongint> array);
void           exponentialSmoothing       (std::vector<double>& array, double gain);
bool           goToByteIndex              (std::fstream& file, ulonglongint offset);

template <class TYPE>
double         getAverage                 (std::vector<TYPE>& array, ulongint startindex = 0,
                                           ulongint length = 0);


///////////////////////////////////////////////////////////////////////////
//
//  Templates --
//

//////////////////////////////
//
// getAverage --
//

template <class TYPE>
double getAverage(std::vector<TYPE>& array, ulongint startindex, ulongint length) {
	ulongint stopindex = array.size() - 1;
	if (length > 0) {
		stopindex = startindex + length - 1;
	}
	int newlength =  stopindex - startindex + 1;
	double average = 0.0;
	for (ulongint r=startindex; r<=stopindex; r++) {
		average += (double)array.at(r);
	}
	return average / (double)newlength;
}



} // end namespace prp


#endif /* _UTILITIES_H */

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Dec  7 22:14:40 PST 2017
// Last Modified: Thu Dec  7 22:14:44 PST 2017
// Filename:      Utilities.h
// Web Address:   
// Syntax:        C++; 
// vim:            ts=3:nowrap:ft=text
//
// Description:   Basic functions for processing TIFF images.
//
//
#ifndef _FFT_H
#define _FFT_H

#include <complex>
#include <vector>


typedef std::complex<double> mycomplex;

namespace prp {

#define two(x) (1 << (x))	 /* 2**x by left-shifting */

void       FFT              (std::vector<mycomplex>& output, 
                             std::vector<mycomplex>& input);
void       shuffle          (std::vector<mycomplex>& X);
void       dftmerge         (std::vector<mycomplex>& XF);
mycomplex  cexp             (mycomplex& Z);
void       swap             (mycomplex& a, mycomplex& b);
int        bitrev           (int n, int B);
void       fft_destructive  (std::vector<mycomplex>& X);
bool       isPowerOfTwo     (int value);


} // end namespace prp


#endif /* _FFT_H */

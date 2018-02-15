//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Dec  7 22:24:12 PST 2017
// Last Modified: Thu Dec  7 22:24:15 PST 2017
// Filename:      FFT.cpp
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   FFT function
//


#include "FFT.h"

#ifndef DONOTUSEFFT

#include <iostream>

using namespace std::complex_literals;


namespace prp {


//////////////////////////////
//
// isPowerOfTwo --
//

bool isPowerOfTwo(int value) {
    return (value != 0) && ((value & (value - 1)) == 0);
}



//////////////////////////////
//
// FFT -- Fast Fourier Transform O(N Log N)
//   Returns the complex spectrum of the given complex input signal.
//   Length of Block must be a power of 2.
//

void FFT(std::vector<mycomplex>& output, std::vector<mycomplex>& input) {
   int N = input.size();

   if (!isPowerOfTwo(N)) {
      std::cerr << "You can only take the FFT of a block with length being"
           << " a power of 2.\nRequested transform length: " << N << std::endl;
      exit(1);
   }

   output = input;
   fft_destructive(output);
}



//////////////////////////////
//
// fft_destructive -- Output is stored in same array as input.
//

void fft_destructive(std::vector<mycomplex>& X) {
   shuffle(X);   // bit reversal
   dftmerge(X);  // merging of DFTs
}



//////////////////////////////
//
// shuffle --
//

void shuffle(std::vector<mycomplex>& X) {
   int n, r;
   int N = X.size();
   int B = 1;                    // number of bits
   while ( (N >> B) > 0 ) {
      B++;
   }
   B--;                          // N = 2**B
   for (n=0; n<N; n++) {
      r = bitrev(n, B);          // bit-reversed version of n
      if (r<n) continue;         // swap only half of the n's
      swap(X[n], X[r]);          // swap values
   }
}



//////////////////////////////
//
// cexp -- complex exponential
//

mycomplex cexp(mycomplex& Z) {
   double R = exp(Z.real());
   mycomplex output(R * cos(Z.imag()), R * sin(Z.imag()));
   return output;
}



//////////////////////////////
//
// dftmerge -- put DFTs back together
//

void dftmerge(std::vector<mycomplex>& XF) {
   int N = XF.size();
   double pi = 4.0 * atan(1.0);
   int k, i, p, q, M;
   mycomplex A, B, V, W;
   M = 2;
	mycomplex value;
   while (M <= N) {                  // two (M/2)-DFT's into one M-DFT
		value = -2i * (pi / M);
      W = cexp(value);               // order-M twiddle factor
		V = 1;
      for (k=0; k<M/2; k++) {        // index for an (M/2)-DFT
         for (i=0; i<N; i+=M) {      // ith butterfly; increments by M
            p = k+i;                 // absoute indices for
            q = p + M / 2;           //    ith butterfly
            A = XF[p];
            B = XF[q] * V;           // V = W**k
            XF[p] = A + B;           // butterfly operations
            XF[q] = A - B;
         }
         V = V * W;                  // V = VW = W**(k+1)
      }
      M = 2 * M;                     // next stage
   }
}



//////////////////////////////
//
// swap -- Swap the location of two Complex numbers.
//

void swap(mycomplex& a, mycomplex& b) {
   mycomplex temp = a;
   a = b;
   b = temp;
}



//////////////////////////////
//
// bitrev -- reverse bits in a number n with B number of bits.
//

int bitrev(int n, int B) {
   int m, r;
   for (r=0, m=B-1; m>=0; m--) { // B is the number of digits
      if ((n>>m)==1) {           // if 2**m term is present
         r += two(B-1-m);        // add 2**(B-1-m) to r
         n -= two(m);            // subtract 2**m from n
      }
   }
   return r;
}



} // end namespace prp


#endif /* DONOTUSEFFT */

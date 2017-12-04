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



//////////////////////////////
//
// isPowerOfTwo --
//

bool isPowerOfTwo(ulong value) {
    return (value != 0) && ((value & (value - 1)) == 0);
}



//////////////////////////////
//
// FFT -- Fast Fourier Transform O(N Log N)
//   Returns the complex spectrum of the given complex input signal.
//   Length of Block must be a power of 2.
//

void FFT(std::vector<mycomplex>& output, std::vector<mycomplex>& input) {
   ulong N = input.size();
 
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
// fft_destructive --
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
   double R = exp(Z.first);
   mycomplex output(R * cos(Z.second), R * sin(Z.second));
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
   while (M <= N) {                            // two (M/2)-DFT's into one M-DFT
		value.first = 0.0;
		value.second = -2 * pi / M;
      W = cexp(value);                         // order-M twiddle factor
		V.first = 1;
		V.second = 0;
      for (k=0; k<M/2; k++) {                  // index for an (M/2)-DFT
         for (i=0; i<N; i+=M) {                // ith butterfly; increments by M
            p = k+i;                           // absoute indices for
            q = p + M / 2;                     //    ith butterfly
            A = XF[p];
            B = XF[q] * V;                     // V = W**k
            XF[p] = A + B;                     // butterfly operations
            XF[q] = A - B;
         }
         V = V * W;                            // V = VW = W**(k+1)
      }    
      M = 2 * M;                               // next stage
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
// bitrev -- reverse bits in a number n with B number of bits
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



//////////////////////////////
//
// operator+ -- complex addition
//

mycomplex operator+(mycomplex& a, mycomplex& b) {
	mycomplex output;
	output.first = a.first + b.first;
	output.second = a.second + b.second;
	return output;
}



//////////////////////////////
//
// operator- -- complex addition
//

mycomplex operator-(mycomplex& a, mycomplex& b) {
	mycomplex output;
	output.first = a.first - b.first;
	output.second = a.second - b.second;
	return output;
}



//////////////////////////////
//
// operator* -- complex multiplication
//

mycomplex operator*(mycomplex& a, mycomplex& b) {
	mycomplex output;
	output.first = a.first * b.first - a.second * b.second;
	output.second =  a.first * b.second + a.second * b.first;
	return output;
}



//////////////////////////////
//
// magnitude --
//

double magnitude(mycomplex& input) {
	return sqrt(pow(input.first, 2.0) + pow(input.second, 2.0));
}



} // end namespace tiff



//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Feb 17 14:47:22 PST 2018
// Last Modified: Sat Feb 17 14:47:25 PST 2018
// Filename:      tifforientation.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Returns the orientation of the TIFF image.  Works on 32-bit TIFFs as
//                well as 64-bit TIFFs (tiffinfo program only works on 32-bit TIFFs).
//

#include "TiffFile.h"

#include <vector>
#include <iostream>

using namespace std;
using namespace rip;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	if (argc != 2) {
		cerr << "Usage: tifflength file.tiff\n";
		exit(1);
	}

	TiffFile image;
	if (!image.open(argv[1])) {
		cerr << "Input filename " << argv[1] << " cannot be opened" << endl;
		exit(1);
	}

	cout << image.getOrientation() << endl;

	return 0;
}


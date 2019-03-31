//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Feb 16 21:39:57 PST 2018
// Last Modified: Fri Feb 16 21:40:00 PST 2018
// Filename:      tifflength.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Returns the number of rows in the TIFF image.  Works on 32-bit TIFFs as
//                well as 64-bit TIFFs (tiffinfo program only works on 32-bit TIFFs).
//

#include "TiffFile.h"

#include <vector>
#include <iostream>

#include <stdlib.h>

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

	cout << image.getRows() << endl;

	return 0;
}


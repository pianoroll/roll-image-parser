//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Nov 26 08:10:23 PST 2017
// Last Modified: Mon Mar 25 21:59:07 PDT 2019
// Filename:      tiff2holes.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Identify holes in TIFF images of piano rolls.  The same
//                program as markholes.cpp, but this one does not do analytic
//                markup of the copy of the input image.
// Options:
//     -r         Assume a Red Welte-Mignon piano roll (T-100). (currently hard-wired on until more roll types implemented)
//     -g         Assume a Green Welte-Mignon piano roll (T-98), but option not yet active.
//     -l         Assume a Welte-Mignon (Deluxe) Licensee piano roll, but option not yet active.
//     -a         Assume an Ampico [A] (older) piano roll, but option not yet active.
//     -b         Assume an Ampico B (newer) piano roll, but option not yet active.
//     -d         Assume a Duo-Art piano roll, but option not yet active.
//     -t         Set the paper/hole brightness boundary (from 0-255, with 249 being the default).
//

#include "RollImage.h"
#include "Options.h"

#include <vector>

using namespace std;
using namespace rip;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("r|red|red-welte|welte-red=b", "Assume Red-Welte (T-100) piano roll");
	options.define("g|green|green-welte|welte-green=b", "Assume Green-Welte (T-98) piano roll (option not active yet)");
	options.define("l|licensee|licensee-welte|welte-licensee=b", "Assume Licensee piano roll (option not active yet)");
	options.define("a|ampico=b", "Assume Ampico [A] piano roll (option not active yet)");
	options.define("b|ampico-b=b", "Assume Ampico B piano roll (option not active yet)");
	options.define("d|duo-art=b", "Assume Aeolean Duo-Art piano roll (option not active yet)");
	options.define("t|threshold=i:249", "Brightness threshold for hole/paper separation");
	options.process(argc, argv);

	if (options.getArgCount() != 1) {
		cerr << "Usage: tiff2holes [-rt] file.tiff > analysis.txt" << endl;
		cerr << "file.tiff must be a 24-bit color image, uncompressed" << endl;
		exit(1);
	}

	RollImage roll;
	if (!roll.open(options.getArg(1))) {
		cerr << "Input filename " << options.getArg(1) << " cannot be opened" << endl;
		exit(1);
	}

	int threshold = options.getInteger("threshold");

	roll.setDebugOn();
	roll.setWarningOn();
	roll.loadGreenChannel(threshold);
	// if (options.getBoolean("red-welte")) {
		roll.setRollTypeRedWelte();
	// } else if (options.getBoolean("green-welte")) {
	// 	roll.setRollTypeGreenWelte();
	// }
	
	roll.analyze();
	cerr << "DONE ANALYZING" << endl;
	roll.printRollImageProperties();

	return 0;
}


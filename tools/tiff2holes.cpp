//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Nov 26 08:10:23 PST 2017
// Last Modified: Wed Feb 13 11:58:36 PST 2019
// Filename:      tiff2holes.cpp
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Identify holes in TIFF images of piano rolls.  The same
//                program as markholes.cpp, but this one does not do analytic
//                markup of the input image.
//

#include "RollImage.h"
#include "Options.h"

#include <vector>

using namespace std;
using namespace prp;

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.define("r|red-welte=b", "Assume Red-Welte piano roll");
	options.define("t|threshold=i:249", "Brightness threshold for hole/paper separation");
	options.process(argc, argv);

	if (options.getArgCount() != 1) {
		cerr << "Usage: tiff2holes [-rt] file.tiff > analysis.txt\n";
		exit(1);
	}

	RollImage roll;
	if (!roll.open(options.getArg(1))) {
		cerr << "Input filename " << options.getArg(1) << " cannot be opened" << endl;
		exit(1);
	}


	/*
	fstream output;
	output.open(options.getArg(2), ios::binary | ios::in | ios::out);
	if (!output.is_open()) {
		cerr << "Output filename " << options.getArg(2) << " cannot be opened" << endl;
		exit(1);
	}
	*/

	int threshold = options.getInteger("threshold");

	roll.setDebugOn();
	roll.setWarningOn();
	roll.loadGreenChannel(threshold);
	// if (options.getBoolean("red-welte")) {
		roll.setRollTypeRedWelte();
	// }
	roll.analyze();
	cerr << "DONE ANALYZING" << endl;
	roll.printRollImageProperties();
	cerr << "DONE PRINTROLLIMAGEPROPERTIES" << endl;

/*
	roll.markHoleBBs();
	cerr << "DONE MARKHOLEBBS" << endl;

	roll.markHoleShifts();
	cerr << "DONE MARKHOLESHIFTS" << endl;

	roll.markHoleAttacks();
	cerr << "DONE MARKHOLEATTACKS" << endl;

	roll.markSnakeBites();
	cerr << "DONE MARKSNAKEBITES" << endl;

	roll.markTrackerPositions(false);
	cerr << "DONE MARKTRACKERPOSITIONS" << endl;

	roll.markShifts();
	cerr << "DONE MARKSHIFTS" << endl;
	// roll.drawMajorAxes();

	roll.mergePixelOverlay(output);
	cerr << "DONE MERGEPIXELOVERLAY" << endl;

	output.close();
	cerr << "DONE CLOSE" << endl;
*/
	return 0;
}


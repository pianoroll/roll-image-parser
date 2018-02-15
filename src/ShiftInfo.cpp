//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Dec 29 19:18:43 PST 2017
// Last Modified: Fri Dec 29 19:18:46 PST 2017
// Filename:      ShiftInfo.cpp
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Information about holes on piano roll
//

#include "ShiftInfo.h"

namespace prp  {


//////////////////////////////
//
// ShiftInfo::ShiftInfo --
//

ShiftInfo::ShiftInfo(void) {
	clear();
}



//////////////////////////////
//
// ShiftInfo::~ShiftInfo --
//

ShiftInfo::~ShiftInfo() {
	clear();
}



//////////////////////////////
//
// ShiftInfo::clear --
//

void ShiftInfo::clear(void) {
	row = 0;
	score = 0.0;
}



//////////////////////////////
//
// ShiftInfo::printAton -- print Hole information in ATON format.
//

std::ostream& ShiftInfo::printAton(std::ostream& out) {
	out << "@@BEGIN: SHIFT\n";
	if (!id.empty()) {
		out << "@ID:\t\t" << id << std::endl;
	}
	double value = int(score * 100 + 0.5)/100.0;
	out << "@ROW:\t\t"    << row   << "px" << std::endl;
	out << "@MOVEMENT:\t"   << value << "px" << std::endl;

	out << "@@END: SHIFT\n";
	return out;
}



} // end prp namespace




//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  2 03:01:50 PST 2017
// Last Modified: Sat Dec  2 03:01:55 PST 2017
// Filename:      HoleInfo.cpp
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Information about holes on piano roll
//

// Compactness: area/perimeter^2
// http://web.cs.wpi.edu/~emmanuel/courses/cs545/S14/slides/lecture08.pdf

#include "HoleInfo.h"

namespace prp  {


//////////////////////////////
//
// HoleInfo::HoleInfo --
//

HoleInfo::HoleInfo(void) { 
	clear(); 
}



//////////////////////////////
//
// HoleInfo::~HoleInfo --
//

HoleInfo::~HoleInfo() {

}



//////////////////////////////
//
// HoleInfo::clear --
//

void HoleInfo::clear(void) {
	area            = 0;
	origin.first    = 0;
	origin.second   = 0;
	width.first     = 0;
	width.second    = 0;
	centroid.first  = 0.0;
	centroid.second = 0.0;
}


//////////////////////////////
//
// operator<< --
//

std::ostream& operator<<(std::ostream& out, HoleInfo& hi) {
	out << "(" << hi.origin.second << ", " << hi.origin.first << ")";
	out << "{" << hi.width.second << ", " << hi.width.first << "}";
	out << "\tA:" << hi.area;
	out << "\tCc:" << int(100.0*hi.centroid.second+0.5)/100.0;
	return out;
}

std::ostream& operator<<(std::ostream& out, HoleInfo* hi) {
	out << (*hi);
	return out;
}



} // end prp namespace




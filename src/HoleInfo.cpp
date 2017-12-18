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
	entry.first     = 0;
	entry.second    = 0;
	m_type          = 1;
	track           = 0;
	perimeter       = 0.0;
	circularity     = 0.0;
	majoraxis       = 0.0;
	coldrift        = 0.0;
}



//////////////////////////////
//
// HoleInfo::printAton -- print Hole information in ATON format.
//

std::ostream& HoleInfo::printAton(std::ostream& out) {
	out << "@@BEGIN: HOLE\n";
	if (!id.empty()) {
		out << "@ID:\t\t" << id << std::endl;
	}
	out << "@ORIGIN_ROW:\t"   << origin.first     << "px"  << std::endl;
	out << "@ORIGIN_COL:\t"   << origin.second    << "px"  << std::endl;
	out << "@WIDTH_ROW:\t"    << width.first      << "px"  << std::endl;
	out << "@WIDTH_COL:\t"    << width.second     << "px"  << std::endl;
	out << "@CENTROID_ROW:\t" << centroid.first   << "px"  << std::endl;
	out << "@CENTROID_COL:\t" << centroid.second  << "px"  << std::endl;
	out << "@AREA:\t\t"       << area             << "px"  << std::endl;
	out << "@PERIMETER:\t"    << perimeter        << "px"  << std::endl;
	out << "@CIRCULARITY:\t"  << int(circularity*100.0+0.5)/100.0 << std::endl;
	// if (!isMusicHole) {
		out << "@MAJOR_AXIS:\t"   << int(majoraxis + 0.5) << "deg" << std::endl;
	// }
	out << "@@END: HOLE\n";
	return out;
}



//////////////////////////////
//
// operator<< --
//

std::ostream& operator<<(std::ostream& out, HoleInfo& hi) {
	out << "(" << hi.origin.second << ", " << hi.origin.first << ")";
	out << "{" << hi.width.second  << ", " << hi.width.first  << "}";
	out << "\tA:"   << hi.area;
	out << "\tCc:"  << int(hi.centroid.second*100.0+0.5)/100.0;
	out << "\tP:"   << int(hi.perimeter*100.0+0.5)/100.0;
	out << "\tCir:" << int(hi.circularity*100.0+0.5)/100.0;
	out << "\tMA:"  << int(hi.majoraxis*100.0+0.5)/100.0;
	return out;
}

std::ostream& operator<<(std::ostream& out, HoleInfo* hi) {
	out << (*hi);
	return out;
}



} // end prp namespace




//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  2 03:01:50 PST 2017
// Last Modified: Sat Dec  2 03:01:55 PST 2017
// Filename:      TearInfo.cpp
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Information about tears on piano roll edges.
//

#include "TearInfo.h"

namespace prp  {


//////////////////////////////
//
// TearInfo::TearInfo --
//

TearInfo::TearInfo(void) {
	clear();
}



//////////////////////////////
//
// TearInfo::~TearInfo --
//

TearInfo::~TearInfo() {

}



//////////////////////////////
//
// TearInfo::printAton -- print Tear information in ATON format.
//

std::ostream& TearInfo::printAton(std::ostream& out) {
	out << "@@BEGIN: TEAR\n";
	if (!id.empty()) {
		out << "@ID:\t\t" << id << std::endl;
	}
	out << "@ORIGIN_ROW:\t"      << origin.first     << "px"  << std::endl;
	out << "@ORIGIN_COL:\t"      << origin.second    << "px"  << std::endl;
	out << "@WIDTH_ROW:\t"       << width.first      << "px"  << std::endl;
	out << "@WIDTH_COL:\t"       << width.second     << "px"  << std::endl;
	// out << "@CENTROID_ROW:\t" << centroid.first   << "px"  << std::endl;
	// out << "@CENTROID_COL:\t" << centroid.second  << "px"  << std::endl;
	out << "@AREA:\t\t"          << area             << "px"  << std::endl;
	// out << "@PERIMETER:\t"    << perimeter        << "px"  << std::endl;
	// out << "@CIRCULARITY:\t"  << int(circularity*100.0+0.5)/100.0 << std::endl;
	// if (!isMusicHole) {
	//	out << "@MAJOR_AXIS:\t"   << int(majoraxis + 0.5) << "deg" << std::endl;
	// }
	out << "@@END: TEAR\n";
	return out;
}



//////////////////////////////
//
// operator<< --
//

std::ostream& operator<<(std::ostream& out, TearInfo& hi) {
	out << "@@BEGIN: TEARINFO" << std::endl;
	out << "@BBOX:\t" << hi.origin.second << " " << hi.origin.first << " "
	    << hi.width.second << " " << hi.width.first << std::endl;
	out << "@DEPTH:\t"<< hi.width.second << std::endl;
	out << "@LENGTH:\t" << hi.width.first << std::endl;
	out << "@AREA:\t"<<  hi.area << std::endl;
	out << "@@END: TEARINFO" << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream& out, TearInfo* hi) {
	out << (*hi);
	return out;
}



} // end prp namespace




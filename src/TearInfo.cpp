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




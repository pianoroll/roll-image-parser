//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Dec  7 21:46:42 PST 2017
// Last Modified: Thu Dec  7 21:46:45 PST 2017
// Filename:      RollOptions.cpp
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Options for parsing images of piano rolls.
//

#include "RollOptions.h"


namespace prp  {


//////////////////////////////
//
// RollOptions::RollOptions --
//

RollOptions::RollOptions(void) {
	reset();
}



//////////////////////////////
//
// RollOptions::~RollOptions --
//

RollOptions::~RollOptions() {
	// do nothing
}



//////////////////////////////
//
// RollOptions::reset --
//

void RollOptions::reset(void) {
	m_minTrackerSpacingToPaperEdge = 1.75;
};



//////////////////////////////
//
// RollOptions::getMinTrackerEdge --
//

double RollOptions::getMinTrackerEdge(void) {
	return m_minTrackerSpacingToPaperEdge;
}



//////////////////////////////
//
// RollOptions::setMinTrackerEdge --
//

void RollOptions::setMinTrackerEdge(double value) {
		m_minTrackerSpacingToPaperEdge = value;
}



} // end prp namespace



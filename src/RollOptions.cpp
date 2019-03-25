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

using namespace std;

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
// RollOptions::reset -- Set to default parameters.
//

void RollOptions::reset(void) {
	m_minTrackerSpacingToPaperEdge = 0.50;
	m_maxHoleWidth                 = 1.50; // wide for rewind hole of duoart salesman
	m_aspectRatioThreshold         = 1.25;
	m_majorAxisThreshold           = 13.0;
	m_circularityThreshold         =  0.4;
	m_maxHoleCount                 = 100000;
	m_maxTearFill                  = 100000;
	m_attackLineSpacing            = 10;
	m_holeShiftCutoff              = 3.0;
};



//////////////////////////////
//
// RollOptions::getAspectRatioThreshold --
//

double RollOptions::getAspectRatioThreshold(void) {
	return m_aspectRatioThreshold;
}



//////////////////////////////
//
// RollOptions::getAspectRatioThreshold --
//

void RollOptions::setAspectRatioThreshold(double value) {
	m_aspectRatioThreshold = value;
}



//////////////////////////////
//
// RollOptions::getMaxHoleTrackerWidth --
//

double RollOptions::getMaxHoleTrackerWidth(void) {
	return m_maxHoleWidth;
}



//////////////////////////////
//
// RollOptions::setMaxHoleTrackerWidth --
//

void RollOptions::setMaxHoleTrackerWidth(double value) {
		m_maxHoleWidth = value;
}



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



//////////////////////////////
//
// RollOptions::getMinTrackerEdge --
//

double RollOptions::getMajorAxisCutoff(void) {
	return m_majorAxisThreshold;
}



//////////////////////////////
//
// RollOptions::setMinTrackerEdge --
//


void RollOptions::setMajorAxisCutoff(double value) {
	m_majorAxisThreshold = value;
}



//////////////////////////////
//
// RollOptions::getCircularityThreshold --
//

double RollOptions::getCircularityThreshold(void) {
	return m_circularityThreshold;
}



//////////////////////////////
//
// RollOptions::setCircularityThreshold --
//

void RollOptions::setCircularityThreshold(double value) {
	m_circularityThreshold = value;
}



//////////////////////////////
//
// RollOptions::getMaxHoleCount --
//

int RollOptions::getMaxHoleCount(void) {
	return m_maxHoleCount;
}



//////////////////////////////
//
// RollOptions::setMaxHoleCount --
//

void RollOptions::setMaxHoleCount(int value) {
	m_maxHoleCount = value;
}



//////////////////////////////
//
// RollOptions::getMaxTearFill --
//

int RollOptions::getMaxTearFill(void) {
	return m_maxTearFill;
}



//////////////////////////////
//
// RollOptions::setMaxTearFill --
//

void RollOptions::setMaxTearFill(int value) {
	m_maxTearFill = value;
}



//////////////////////////////
//
// RollOptions::getAttackLineSpacing --
//

int RollOptions::getAttackLineSpacing(void) {
	return m_attackLineSpacing;
}



//////////////////////////////
//
// RollOptions::setAttackLineSpacing --
//

void RollOptions::setAttackLineSpacing(int value) {
	m_attackLineSpacing = value;
}



//////////////////////////////
//
// RollOptions::getHoleShiftCutoff --
//

double RollOptions::getHoleShiftCutoff(void) {
	return m_holeShiftCutoff;
}



//////////////////////////////
//
// RollOptions::setHoleShiftCutoff --
//

void RollOptions::setHoleShiftCutoff(double value) {
	m_holeShiftCutoff = value;
}



//////////////////////////////
//
// RollOptions::getRollType -- Return the roll type being processed:
//   "" = unknown
//   "welte-red"      = Welte Mignon T-100 red roll
//   "welte-green"    = Welte Mignon T-98 green roll
//   "welte-licensee" = Welte Mignon (Deluxe) Licensee
//   "ampico"         = AMPICO roll (variation not specified)
//   "ampico-a"       = AMPICO roll, earlier model
//   "ampico-b"       = AMPICO roll, later model
//   "duoart"         = Aeolean Duo-Art
//

std::string RollOptions::getRollType(void) {
	return m_rollType;
}



//////////////////////////////
//
// RollOptions::setRollTypeRedWelte -- Apply settings suitable for Red Welte piano rolls.
//
// red Welte tracker holes:
//
//   10 expression on left side:
//       1:  MF-Off                          MIDI Key 14
//       2:  MF-On                           MIDI Key 15
//       3:  Crescendo-Off                   MIDI Key 16
//       4:  Crescendo-On                    MIDI Key 17
//       5:  Forzando-Off                    MIDI Key 18
//       6:  Forzando-On                     MIDI Key 19
//       7:  Soft-Pedal-Off                  MIDI Key 20
//       8:  Soft-Pedal-On                   MIDI Key 21
//       9:  Motor-Off                       MIDI Key 22
//       10: Motor-On                        MIDI Key 23
//   Then 80 notes from C1 to G7 (MIDI note 24 to 103
//       11: C1                              MIDI Key 24
//       ...
//       50:  D#4                            MIDI Key 63
//    Treble register:
//       51:  E4                             MIDI Key 64
//       ...
//       90:  G7                             MIDI Key 103
//   Then 10 expression holes on the right side:
//       91:  -10: Rewind                    MIDI Key 104
//       92:  -9:  Electric-Cutoff           MIDI Key 105
//       93:  -8:  Sustain-Pedal-On          MIDI Key 106
//       94:  -7:  Sustain-Pedal-Off         MIDI Key 107
//       95:  -6:  Forzando-On               MIDI Key 108
//       96:  -5:  Forzando-Off              MIDI Key 109
//       97:  -4:  Crescendo-On              MIDI Key 110
//       98:  -3:  Crescendo-Off             MIDI Key 111
//       99:  -2:  Mezzo-Forte-On            MIDI Key 112
//       100: -1:  Mezzo-Forte-Off           MIDI Key 113
//
//

void RollOptions::setRollTypeRedWelte(void) {
		m_rollType = "welte-red";
		m_maxHoleCount = 100;
		m_minHoleCount = 100;
		m_minTrackerSpacingToPaperEdge = 1.6;
		m_rewindHole = 91;  // 91st hole from left (bass)
		m_rewindHoleMidi = 104;
cerr << ">>> SETTING REWIND HOLE TO " << m_rewindHoleMidi << endl;

		m_bassExpressionTrackStartNumberLeft = 1;
		m_bassExpressionTrackStartMidi = 14;
		m_bassNotesTrackStartNumberLeft = 11;
		m_bassNotesTrackStartMidi = 24;
		m_trebleNotesTrackStartNumberLeft = 51;
		m_trebleNotesTrackStartMidi = 64;
		m_trebleExpressionTrackStartNumberLeft = 91;
		m_trebleExpressionTrackStartMidi = 104;

}


/////////////////////////////////
//
// RollOptions::getRewindHoleNumber(void) -- returns 0 if unknown.
//

int RollOptions::getRewindHoleBassNumber(void) {
	return m_rewindHole;
}



/////////////////////////////////
//
// RollOptions::getRewindHoleLeftIndex(void) -- returns 0 if unknown.
//

int RollOptions::getRewindHoleBassIndex(void) {
	return m_rewindHole - 1;
}



/////////////////////////////////
//
// RollOptions::getRewindHoleMidi(void) -- returns 0 if unknown; otherwise,
//     returns the MIDI note number of the rewind hole.
//

int RollOptions::getRewindHoleMidi(void) {
	return m_rewindHoleMidi;
}



} // end prp namespace




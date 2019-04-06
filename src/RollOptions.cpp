//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Dec  7 21:46:42 PST 2017
// Last Modified: Mon Mar 25 22:38:36 PDT 2019
// Filename:      RollOptions.cpp
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Options for parsing images of piano rolls.
//

#include "RollOptions.h"

using namespace std;

namespace rip  {


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
//   "" = unknown/unspecified
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
// RollOptions::setRollTypeRedWelte -- Apply settings suitable for Red Welte (T-100) piano rolls.
//
// red Welte tracker holes:
//
//   10 expression on bass side:
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
//   Then 80 notes from C1 to G7 (MIDI notes 24 to 103)
//       11: C1                              MIDI Key 24
//       ...
//       53:  F#4                            MIDI Key 66
//    Treble register (note bass register has more notes):
//       54:  G4                             MIDI Key 67
//       ...
//       90:  G7                             MIDI Key 103
//   10 expression holes on the treble side:
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
	m_minTrackerSpacingToPaperEdge = 1.6;
	m_rewindHole = 91;  // 91st hole from left (bass)
	m_rewindHoleMidi = 104;
	m_trackerHoles = 100;

	m_bassExpressionTrackStartNumberLeft = 1;
	m_bassExpressionTrackStartMidi = 14;
	m_bassNotesTrackStartNumberLeft = 11;
	m_bassNotesTrackStartMidi = 24;
	m_trebleNotesTrackStartNumberLeft = 54;
	m_trebleNotesTrackStartMidi = 67;
	m_trebleExpressionTrackStartNumberLeft = 91;
	m_trebleExpressionTrackStartMidi = 104;
}



//////////////////////////////
//
// RollOptions::setRollTypeGreenWelte -- Apply settings suitable for Red Welte (T-98) piano rolls.
//
// Peter Phillips dissertation: https://ses.library.usyd.edu.au/bitstream/2123/16939/1/Piano%20Rolls.pdf
//
// green Welte tracker holes:
//
//   5 expression holes on bass side:
//       1:  Bass Sforzando piano (& rewind) MIDI Key 16
//       2:  Bass Mezzoforte                 MIDI Key 17
//       3:  Sustain pedal                   MIDI Key 18
//       4:  Bass Crescendo                  MIDI Key 19
//       5:  Bass Sforzando forte            MIDI Key 20
//   Then 88 notes from C0 to C8 (MIDI notes 21 to 108) (but actually only 80 used)
//       6: A0                               MIDI Key 21
//       ...
//       51:  F#4                            MIDI Key 66
//    Treble register:
//       52:  G4                             MIDI Key 67
//       ...
//       93:  G7                             MIDI Key 108
//   5 expression holes on the treble side:
//       94:  -5:  Treble Sforzando forte    MIDI Key 109
//       95:  -4:  Treble Crescendo          MIDI Key 110
//       96:  -3:  Soft pedal                MIDI Key 111
//       97:  -2:  Treble Mezzoforte         MIDI Key 112
//       98:  -1:  Treble Sforzando piano    MIDI Key 113
//

void RollOptions::setRollTypeGreenWelte(void) {
	cerr << "GREEN ROLL NOT IMPLEMENT YET" << endl;
	exit(1);

	m_rollType = "welte-green";
	m_minTrackerSpacingToPaperEdge = 1.6; // check
	m_rewindHole = 1;  // 1st hole from left (bass), but only if "long"
	m_rewindHoleMidi = 16;
	m_trackerHoles = 98;

	m_bassExpressionTrackStartNumberLeft = 1;
	m_bassExpressionTrackStartMidi = 16;
	m_bassNotesTrackStartNumberLeft = 6;
	m_bassNotesTrackStartMidi = 21;

	m_trebleNotesTrackStartNumberLeft = 54;
	m_trebleNotesTrackStartMidi = 67;

	m_trebleExpressionTrackStartNumberLeft = 94;
	m_trebleExpressionTrackStartMidi = 109;
}


// Duo-Art register break at D#4/E4

// Ampico register break at E4/F4


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



/////////////////////////////////
//
// RollOptions::getBridgeFactor(void) -- Returns the ratio of width to length
//     distance which will force the merging of adjacent holes.
//

double RollOptions::getBridgeFactor(void) {
	return m_bridgeFactor;
}



//////////////////////////////
//
// RollOptions::getExpectedTrackerHoleCount -- Returns 0 if roll type is undefined.
//

int RollOptions::getExpectedTrackerHoleCount(void) {
	return m_trackerHoles;
}



//////////////////////////////
//
// RollOptions::setThreshold -- 
//

void RollOptions::setThreshold(int value) {
	m_threshold = value;
}



//////////////////////////////
//
// RollOptions::getThreshold -- 
//

int RollOptions::getThreshold(void) {
	return (ucharint)m_threshold;
}

} // end rip namespace




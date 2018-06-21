//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Dec  1 16:40:52 PST 2017
// Last Modified: Fri Dec 15 03:48:43 PST 2017
// Filename:      RollImage.cpp
// Web Address:
// Syntax:        C++;
// vim:           ts=3:nowrap:ft=text
//
// Description:   Piano-roll description parameters.
//

///////////////////////////////////////////////////////////
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


#include "RollImage.h"
#include "HoleInfo.h"
#include "ShiftInfo.h"
#include "CheckSum.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace prp {


//////////////////////////////
//
// RollImage::RollImage --
//

RollImage::RollImage(void) {
	clear();
}

//////////////////////////////
//
// RollImage::my_to_string --
//

string RollImage::my_to_string(int value) {
	stringstream stream;
	stream << value;
	return stream.str();
}



//////////////////////////////
//
// RollImage::clear --
//

void RollImage::clear(void) {
	holeSeparation = 0.0;
	holeOffset = 0.0;
	averageRollWidth = 0.0;
	m_debug                     = false;
	m_warning                   = false;
	m_analyzedBasicMargins      = false;
	m_analyzedLeaders           = false;
	m_analyzedAdvancedMargins   = false;
	hardMarginLeftIndex         = 0;
	hardMarginRightIndex        = 0;
	m_threshold                 = 255;
	preleaderIndex              = 0;
	leaderIndex                 = 0;
	firstMusicRow               = 0;
	lastMusicRow                = 0;
	m_lastHolePosition          = 0.0;
	m_firstHolePosition         = 0.0;
	m_dustscore                 = -1.0;
	m_dustscorebass             = -1.0;
	m_dustscoretreble           = -1.0;
	m_averageHoleWidth          = -1.0;

}




//////////////////////////////
//
// RollImage::~RollImage --
//

RollImage::~RollImage(void) {
	for (ulongint i=0; i<holes.size(); i++) {
		delete holes[i];
	}
	holes.resize(0);
	for (ulongint i=0; i<antidust.size(); i++) {
		delete antidust[i];
	}
	antidust.resize(0);

	for (ulongint i=0; i<bassTears.size(); i++) {
		delete bassTears[i];
	}
	bassTears.resize(0);

	for (ulongint i=0; i<trebleTears.size(); i++) {
		delete trebleTears[i];
	}
	trebleTears.resize(0);

	for (ulongint i=0; i<shifts.size(); i++) {
		delete shifts[i];
	}
	shifts.resize(0);

	close();
}



//////////////////////////////
//
// RollImage::loadGreenChannel --
//

void RollImage::loadGreenChannel(int threshold) {
	setThreshold(threshold);
	ulongint rows = getRows();
	ulongint cols = getCols();
	this->getImageGreenChannel(monochrome);
	pixelType.resize(rows);
	for (ulongint r=0; r<rows; r++) {
		pixelType[r].resize(getCols());
		for (ulongint c=0; c<cols; c++) {
			if (aboveThreshold(monochrome[r][c], getThreshold())) {
				pixelType[r][c] = PIX_NONPAPER;
			} else {
				pixelType[r][c] = PIX_PAPER;
			}
		}
	}
}



//////////////////////////////
//
// RollImage::analyze --
//

void RollImage::analyze(void) {
#ifndef DONOTUSEFFT
	start_time = std::chrono::system_clock::now();
#endif

	if (m_debug) { cerr << "STEP 1: analyzeBasicMargins" << endl; }
	analyzeBasicMargins();
	if (m_debug) { cerr << "STEP 2: analyzeLeaders" << endl; }
	analyzeLeaders();
	if (m_debug) { cerr << "STEP 3: analyzeAdvancedMargins" << endl; }
	analyzeAdvancedMargins();
	if (m_debug) { cerr << "STEP 4: generateDriftCorrection" << endl; }
	generateDriftCorrection(0.01);
	if (m_debug) { cerr << "STEP 5: analyzeHoles" << endl; }
	analyzeHoles();
	if (m_debug) { cerr << "STEP 6: analyzeTears" << endl; }
	analyzeTears();
	if (m_debug) { cerr << "STEP 7: analyzeShifts" << endl; }
	analyzeShifts();
	if (m_debug) { cerr << "STEP 8: generateDriftCorrection" << endl; }
	generateDriftCorrection(0.01);
	if (m_debug) { cerr << "STEP 9: calculateHoleDescriptors" << endl; }
	calculateHoleDescriptors();
	if (m_debug) { cerr << "STEP 10: invalidateSkewedHoles" << endl; }
	invalidateSkewedHoles();
	if (m_debug) { cerr << "STEP 11: markPosteriorLeader" << endl; }
	markPosteriorLeader();
	if (m_debug) { cerr << "STEP 12: analyzeTrackerBarSpacing" << endl; }
	storeCorrectedCentroidHistogram();
	analyzeRawRowPositions();
	analyzeTrackerBarSpacing();
	if (m_debug) { cerr << "STEP 13: analyzeTrackerBarPositions" << endl; }
	// analyzeTrackerBarPositions();
	calculateTrackerSpacings2();
	if (m_debug) { cerr << "STEP 14: analyzeHorizontalHolePosition" << endl; }
	analyzeHorizontalHolePosition();
	if (m_debug) { cerr << "STEP 15: analyzeMidiKeyMapping" << endl; }
	analyzeMidiKeyMapping();
	if (m_debug) { cerr << "STEP 16: invalidateEdgeHoles" << endl; }
	invalidateEdgeHoles();
	if (m_debug) { cerr << "STEP 17: invalidateOffTrackerHoles" << endl; }
	invalidateOffTrackerHoles();
	if (m_debug) { cerr << "STEP 18: recalculateFirstMusicHole" << endl; }
	recalculateFirstMusicHole();
	if (m_debug) { cerr << "STEP 19: addDriftInfoToHoles" << endl; }
	addDriftInfoToHoles();
	if (m_debug) { cerr << "STEP 20: addAntidustToBadHoles" << endl; }
	addAntidustToBadHoles(50);
	if (m_debug) { cerr << "STEP 21: assignMusicHoleIds" << endl; }
	assignMusicHoleIds();
	if (m_debug) { cerr << "STEP 22: groupHoles" << endl; }
	groupHoles();
	if (m_debug) { cerr << "STEP 23: analyzeSnakeBites" << endl; }
	analyzeSnakeBites();
	if (m_debug) { cerr << "STEP 24: FINSHED WITH ANALYSIS!" << endl; }

#ifndef DONOTUSEFFT
	stop_time = std::chrono::system_clock::now();
#endif
}



//////////////////////////////
//
// RollImage::analyzeSnakeBites -- Needs to be improved since it is sensitive
//   to noise (search for up to two pairs of small holes next to each other
//   in the margins on both sides).  Currently looking for the smallest
//   tracker hole rows and giving up if there are not exactly four of them.
//

void RollImage::analyzeSnakeBites(void) {

#ifndef DONOTUSEFFT
	std::vector<double> avgwidth(trackerArray.size(), 0.0);
	ulongint count;

	for (ulongint i=0; i<trackerArray.size(); i++) {
		count = 0;
		for (ulongint j=0; j<trackerArray[i].size(); j++) {
			if (!trackerArray[i][j]->isMusicHole()) {
				continue;
			}
			avgwidth[i] += trackerArray[i][j]->width.second;
			count++;
		}
		if (count > 1) {
			avgwidth[i] /= (double)count;
		}
	}

	std::pair<double, int> a;
	std::vector<std::pair<double, int> > sortlist;
	for (ulongint i=0; i<avgwidth.size(); i++) {
		if (avgwidth[i] == 0) {
			continue;
		}
		a.first = avgwidth[i];
		a.second = i;
		sortlist.push_back(a);
	}

	std::sort(sortlist.begin(), sortlist.end(),
		[](std::pair<double, int>& a, std::pair<double, int>& b) -> bool
			{
				return a.first < b.first;
			}
		);

	int split = 0;
	for (ulongint i=0; i<sortlist.size()-1; i++) {
		double factor = sortlist[i].first / sortlist[i+1].first;
		if (factor <= 0.75) {
			split = i;
			break;
		}
	}

	if (split == 0) {
		// no melodic accents
		return;
	}

	if (split != 3) {
		// requiring four tracks (could be two, but exclude for now).
		return;
	}

	// makesure the holes are in pairs
	std::vector<int> pairing(4, 0);
	for (ulongint i=0; i<4; i++) {
		if (pairing[i]) {
			continue;
		}
		for (ulongint j=i+1; j<4; j++) {
			if (pairing[j]) {
				continue;
			}
			if (abs(sortlist[i].second - sortlist[j].second) == 1) {
				pairing[i] = j+1;
				pairing[j] = i+1;
				break;
			}
		}
	}

	int zerocount = 0;
	for (ulongint i=0; i<pairing.size(); i++) {
		if (pairing[i] == 0) {
			zerocount++;
		}
	}
	if (zerocount > 0) {
		return;
	}

	// All holes in first for slots of sortlist are melodic accent tracker
	// positions.
	for (ulongint i=0; i<4; i++) {
		ulongint index = sortlist[i].second;
		trackMeaning[index] = TRACK_SNAKEBITE;
		for (ulongint j=0; j<trackerArray[index].size(); j++) {
			trackerArray[index][j]->snakebite = true;
		}
	}
#endif
}



//////////////////////////////
//
// RollImage::invalidateOffTrackerHoles --
//

void RollImage::invalidateOffTrackerHoles(void) {
	for (ulongint i=0; i<trackerArray.size(); i++) {
		if (trackerArray[i].empty()) {
			continue;
		}
		invalidateHolesOffTracker(trackerArray[i], i);
	}
}



//////////////////////////////
//
// RollImage::invalidateHolesOffTracker --
// Check right edge at top of image:
//   https://ccrma.stanford.edu/~craig/piano-roll-project/pianorolls/acceptance_test-20171219/greenwelte-test_roll_11/druid-cx178xw6230-analysis/analysis-8.jpg
//

void RollImage::invalidateHolesOffTracker(std::vector<HoleInfo*>& hi, ulongint index) {
return; // disabling for now
	double maxoffset = 0.25;  // 25% of the way to the next track
	double trackpos = index * holeSeparation + holeOffset;
	for (ulongint i=0; i<hi.size(); i++) {
		double newtrackpos = trackpos - driftCorrection[hi[i]->origin.first];
		double offset = fabs(newtrackpos - hi[i]->centroid.second);
		if (offset > maxoffset) {
			cerr << "BAD HOLE " << offset << endl;
			hi[i]->reason = "uncentered";
			hi[i]->setNonHole();
		}
	}
}



//////////////////////////////
//
// RollImage::groupHoles -- Mark whether or not holes are the
//   start of a note attack or not.
//

void RollImage::groupHoles(void) {
	for (ulongint i=0; i<trackerArray.size(); i++) {
		groupHoles(i);
	}
}


void RollImage::groupHoles(ulongint index) {
	vector<HoleInfo*>& hi = trackerArray[index];
	double scalefactor = 1.37;
	double length = getAverageMusicalHoleWidth() * scalefactor;
	if (hi.empty()) {
		return;
	}

	HoleInfo* lastattack = NULL;
	hi[0]->attack = true;
	hi[0]->offtime = hi[0]->origin.first + hi[0]->width.first;
	lastattack = hi[0];
	for (ulongint i=1; i<hi.size(); i++) {
		hi[i]->prevOff = hi[i]->origin.first - (hi[i-1]->origin.first + hi[i-1]->width.first);
		if (hi[i]->prevOff <= length) {
			hi[i]->attack = false;
			if (lastattack) {
				// extend off time of previous attack
				lastattack->offtime = hi[i]->origin.first + hi[i]->width.first;
			}
		} else {
			hi[i]->attack = true;
			hi[i]->offtime = hi[i]->origin.first + hi[i]->width.first;
			lastattack = hi[i];
		}
	}

}



//////////////////////////////
//
// RollImage::addAntidustToBadHoles -- Add antidust holes to badHoles if they
//    are larger than the given threshold area.
//

void RollImage::addAntidustToBadHoles(ulongint areaThreshold) {
	for (ulongint i=0; i<antidust.size(); i++) {
		if (antidust[i]->origin.first < firstMusicRow - 100) {
			// don't keep track of bad holes before first music hole.
			continue;
		}
		if (antidust[i]->area >= areaThreshold) {
			badHoles.push_back(antidust[i]);
		}
	}
}



//////////////////////////////
//
// RollImage::calculateHoleDescriptors -- also circularity
//
void RollImage::calculateHoleDescriptors(void) {
	for (ulongint i=0; i<holes.size(); i++) {
		int status = calculateHolePerimeter(*holes[i]);
		if (!status) {
			// bad region so don't do any more calculations.
			continue;
		}
		holes[i]->circularity = 4 * M_PI * holes[i]->area /
			holes[i]->perimeter / holes[i]->perimeter;
		holes[i]->majoraxis = calculateMajorAxis(*holes[i]);
	}
}



//////////////////////////////
//
// RollImage::assignMidiKeyNumbersToHoles --
//

void RollImage::assignMidiKeyNumbersToHoles(void) {
	for (ulongint i=0; i<midiToTrackMapping.size(); i++) {
		int track = midiToTrackMapping[i];
		if (track <= 0) {
			continue;
		}
		if (track >= (int)trackerArray.size()) {
			continue;
		}
		for (ulongint j=0; j<trackerArray.at(track).size(); j++) {
			trackerArray[track][j]->midikey = i;
		}
	}
}



//////////////////////////////
//
// RollImage::calculateCentralMoment --
//

double RollImage::calculateCentralMoment(HoleInfo& hole, int p, int q) {
	std::pair<double, double> center = hole.centroid;
	int ro = hole.origin.first;
	int co = hole.origin.second;
	double moment = 0.0;
	ulongint r, c;
	for (r=0; r<hole.width.first; r++) {
		for (c=0; c<hole.width.second; c++) {
			if (pixelType[r+ro][c+co] != PIX_HOLE) {
				continue;
			}
			moment += pow(c+co - center.second, p) *
					pow(r+ro - center.first, q);
		}
	}
	return moment;
}



//////////////////////////////
//
// RollImage::calculateMajorAxis -- Major axis in degrees, with
//  0 degrees being vertical.
//

double RollImage::calculateMajorAxis(HoleInfo& hole) {
	double m11 = calculateCentralMoment(hole, 1, 1);
	double m20 = calculateCentralMoment(hole, 2, 0);
	double m02 = calculateCentralMoment(hole, 0, 2);

	double tan = 2 * m11 / (m20 - m02);
	double angle = 0.5 * atan(tan);
	angle *= 180 / M_PI;
	return angle;
}




//////////////////////////////
//
// RollImage::calculateNormalCentralMoment --
//

double RollImage::calculateNormalCentralMoment(HoleInfo& hole, int p,
		int q) {
	double m00 = hole.area;
	double norm = pow(m00, (double)(p+q+2)/2);
	return calculateCentralMoment(hole, p, q) / norm;
}



//////////////////////////////
//
// RollImage::calculateHolePerimeter --
//

bool RollImage::calculateHolePerimeter(HoleInfo& hole) {
	hole.perimeter = 0.0;
	ulongint r;
	long c;
	r = hole.entry.first;
	for (c=(int)hole.entry.second; c>=0; c--) {
		if (pixelType.at(r).at(c) == PIX_PAPER) {
			break;
		}
	}
	if (pixelType[r][c] != PIX_PAPER) {
		return 1;
	}
	pair<ulongint, ulongint> start(r, c);
	pair<ulongint, ulongint> successor; // next point after starting point

	pair<ulongint, ulongint> previous = start;
	pair<ulongint, ulongint> current = previous;
	int direction = 0;
	direction = findNextPerimeterPoint(current, direction);
	successor = current;
	bool done = start == successor;

	// pixelType[start.first][start.second] = PIX_DEBUG2;
	// pixelType[successor.first][successor.second] = PIX_DEBUG2;

	double sum = 0.0;

	int counter = 0;
	while (!done) {
		previous = current;
		direction = (direction + 6) % 8;
		direction = findNextPerimeterPoint(current, direction);
		if (direction < -100) {
			// bad perimeter (on image edge)
			return 0;
		}
		done = (current == successor) && (previous == start);
		if (!done) {
			//pixelType[current.first][current.second] = PIX_DEBUG5;
			if (direction % 2) {
				sum += 1.41421356237;
			} else {
				sum += 1;
			}
		}
		if (++counter >= 100000) {
			// failsafe
			std::cerr << "PERIMETER SEARCH TOO LARGE" << std::endl;
			break;
		}
	}

	hole.perimeter = 0.95 * sum;
	return 1;
}



//////////////////////////////
//
// RollImage::findNextPerimeterPoint --
//

int RollImage::findNextPerimeterPoint(pair<ulongint, ulongint>& point, int dir) {
	int delta[][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1},
		{-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
	int r, c;
	for (int i=0; i<7; i++) {
		c = point.second + delta[dir][0];
		r = point.first  + delta[dir][1];
		if (c >= (int)getCols()) {
			return -1000;
		}
		if (r >= (int)getRows()) {
			return -1000;
		}
		if (pixelType.at(r).at(c) == PIX_HOLE) {
			dir = (dir+1) % 8;
		} else {
			// pixelType[r][c] = PIX_DEBUG5;
			point.first  = r;
			point.second = c;
			break;
		}
	}
	return dir;
}



//////////////////////////////
//
// RollImage::analyzeMidiKeyMapping -- assign tracker bar positions to MIDI key numbers.
//

void RollImage::analyzeMidiKeyMapping(void) {

	std::vector<double>&  position = m_normalizedPosition;
	position.resize(trackerArray.size());
	std::fill(position.begin(), position.end(), 0.0);

	// assigned normalized pixel column positions to each hole column.
	for (ulongint i=0; i<position.size(); i++) {
		position[i] = i * holeSeparation + holeOffset;
	}

	// find the first position that has holes
	for (ulongint i=0; i<trackerArray.size(); i++) {
		if (trackerArray[i].empty()) {
			continue;
		}
		m_firstHolePosition = position[i];
		break;
	}

	// find the last position which has holes
	for (long i=(int)trackerArray.size()-1; i>=0; i--) {
		if (trackerArray[i].empty()) {
			continue;
		}
		m_lastHolePosition = position[i];
		break;
	}

	// check that the hole positions are within the bounds of the paper
	// within the tolerance of getMinTrackerEdge():

	ulongint r = getFirstMusicHoleStart();
	double leftmin = leftMarginIndex[r];
//	leftmin += driftCorrection[r]; // don't add drift correction?
	leftmin += getMinTrackerEdge() * holeSeparation;
	int leftmostIndex = 0;
	for (ulongint i=0; i<position.size(); i++) {
		if (position[i] > leftmin) {
			leftmostIndex = i;
			break;
		}
	}

	double rightmin = rightMarginIndex[r];
//	rightmin += driftCorrection[r];  // don't add drift correction?
	rightmin += -getMinTrackerEdge() * holeSeparation;
	int rightmostIndex = position.size()-1;
	for (ulongint i=position.size() - 1; i>0; i--) {
		if (position[i] < rightmin) {
			rightmostIndex = i;
			break;
		}
	}

	if ((rightmostIndex < (int)trackerArray.size() - 1) && 
			!trackerArray.at(rightmostIndex+1).empty()) {
		rightmostIndex++;
	}

	if (!trackerArray.at(leftmostIndex-1).empty()) {
		leftmostIndex--;
	}

	if (trackerArray.at(rightmostIndex).empty() && trackerArray.at(leftmostIndex).empty()) {
		// shrink if no holes in extreme positions of both sides
		leftmostIndex++;
		rightmostIndex--;
	}

	if (trackerArray[rightmostIndex].empty() && trackerArray[leftmostIndex].empty()) {
		// shrink if no holes in extreme positions of both sides
		leftmostIndex++;
		rightmostIndex--;
	}

	int holecount = rightmostIndex - leftmostIndex + 1;
	if (m_warning && (holecount > 100)) {
		std::cerr << "Warning hole count is quite large: " << holecount << std::endl;
	}
	if (holecount > 105) {
		std::cerr << "Error: way too many hole rows on paper (can't handle organ rolls yet)" << endl;
		exit(1);
	}

	// Rough guess for now on the mapping: placing the middle hole position on E4/F4 boundary
	int F4split = int((rightmostIndex - leftmostIndex) / 2 + leftmostIndex + 0.5); // 0.5 needed?

	midiToTrackMapping.resize(128);
	std::fill(midiToTrackMapping.begin(), midiToTrackMapping.end(), 0);

	int adjustment = 64 - F4split;
	adjustment -= 1;  // temporary fix for red Welte rolls.
	for (int i=leftmostIndex; i<=rightmostIndex; i++) {
		midiToTrackMapping.at(i + adjustment) = i;
	}

	int trackerholes = getTrackerHoleCount();

	if (trackerholes == 65) {
		// re-map up a major second
		for (ulongint i=127; i>2; i--) {
			midiToTrackMapping[i] = midiToTrackMapping[i-2];
		}
	}

}



//////////////////////////////
//
// RollImage::invalidateEdgeHoles -- Mark holes in regions where there could be
//    tracker lines, but analyzeMidiKeyMapping() removed these lines from consideration.
//
// Later move from holes to antidust as well as removing from trackerArray
// std::vector<HoleInfo*> antidust;
// std::vector<HoleInfo*> holes;
// std::vector<std::vector<HoleInfo*> > trackerArray;
//

void RollImage::invalidateEdgeHoles(void) {

	ulongint minmidi = 0;
	for (ulongint i=0; i<midiToTrackMapping.size(); i++) {
		if (midiToTrackMapping[i]) {
			minmidi = i;
			break;
		}
	}

	ulongint maxmidi = midiToTrackMapping.size() - 1;
	for (long i=maxmidi; i>=0; i--) {
		if (midiToTrackMapping[i]) {
			maxmidi = (ulongint)i;
			break;
		}
	}

	ulongint mintrack = midiToTrackMapping[minmidi];
	ulongint maxtrack = midiToTrackMapping[maxmidi];

	ulongint maxwidth = int(holeSeparation * getMaxHoleTrackerWidth() + 0.5);

	for (ulongint i=0; i<holes.size(); i++) {
		if (holes[i]->track == 0) {
			clearHole(*holes[i], PIX_ANTIDUST);
			badHoles.push_back(holes[i]);
			holes[i]->reason = "strange";
			continue;
		} else if (holes[i]->track < mintrack) {
			// out of range on bass side
			clearHole(*holes[i], PIX_ANTIDUST);
			holes[i]->track = 0;
			badHoles.push_back(holes[i]);
			holes[i]->reason = "bass margin";
			continue;
		} else if (holes[i]->track > maxtrack) {
			// out of range on treble side
			clearHole(*holes[i], PIX_ANTIDUST);
			holes[i]->track = 0;
			badHoles.push_back(holes[i]);
			holes[i]->reason = "treble margin";
			continue;
		}
		if (holes[i]->width.second >= maxwidth) {
			// Hole is too wide
			clearHole(*holes[i], PIX_BADHOLE);
			holes[i]->track = 0;
			holes[i]->setNonHole();
			badHoles.push_back(holes[i]);
			holes[i]->reason = "wide";
			continue;
		}
		double aspect = (double)holes[i]->width.second / (double)holes[i]->width.first;
		if (aspect > getAspectRatioThreshold()) {
			// Hole is wider than it is long, which should never happen for a music hole.
			clearHole(*holes[i], PIX_BADHOLE_ASPECT);
			holes[i]->reason = "aspect";
			holes[i]->track = 0;
			holes[i]->setNonHole();
			badHoles.push_back(holes[i]);
			continue;
		}
	}
}



//////////////////////////////
//
// RollImage::invalidateSkewedHoles --  If non-circular holes are not
//   vertically aligned, then they cannot be music holes, or the music
//   holes are defective in some way.  Remove those holes to the badHole list.
//

void RollImage::invalidateSkewedHoles(void) {
	for (ulongint i=0; i<holes.size(); i++) {
		if (holes[i]->circularity > getCircularityThreshold()) {
			// hole is too round to determine skew.
			continue;
		}
		if (fabs(holes[i]->majoraxis) < getMajorAxisCutoff()) {
			// hole is basically aligned in correct direction
			continue;
		}
		// hole has a problem, probably a rip/tear/etc.
		// std::cerr << "REMOVING " << holes[i]->origin.first
		//		<< "\t" << holes[i]->circularity
		//		<< "\t" << holes[i]->majoraxis
		//		<< std::endl;
		clearHole(*holes[i], PIX_BADHOLE_SKEWED);
		holes[i]->reason = "skewed";
		badHoles.push_back(holes[i]);
	}
}



//////////////////////////////
//
// RollImage::clearHole -- mark hole as something else (antidust);
//

void RollImage::clearHole(HoleInfo& hi, int type) {
	hi.setNonHole();
	ulongint r = hi.entry.first;
	ulongint c = hi.entry.second;
	int target = pixelType[r][c];
	int counter = 0;
	fillHoleSimple(r, c, target, type, counter);
	hi.setNonHole();
}



//////////////////////////////
//
// RollImage::analyzeHorizontalHolePosition -- assign holes to tracker bar
//      positions.
//

// ggg problem area

void RollImage::analyzeHorizontalHolePosition() {

	int tcount = (getCols()+holeOffset) / holeSeparation;
	trackerArray.resize(0);
	trackerArray.resize(tcount);
	for (ulongint i=0; i<holes.size(); i++) {
		double position = holes[i]->centroid.second;
		double correction = driftCorrection[int(holes[i]->centroid.first+0.5)];
		// double cpos = position + correction + holeOffset;
		double cpos = position + correction - holeOffset;
		int index = int(cpos / holeSeparation + 0.5);
		trackerArray.at(index).push_back(holes.at(i));
		holes[i]->track = index;
	}

	trackMeaning.resize(trackerArray.size());
	std::fill(trackMeaning.begin(), trackMeaning.end(), TRACK_UNKNOWN);

	/*
	for (ulongint i=0; i<trackerArray.size(); i++) {
		cerr << i << "\t" << trackerArray[i].size() << "\t";
		if (trackerArray[i].size() > 0) {
			cerr << trackerArray[i][0]->centroid.first;
		} else {
			cerr << 0;
		}
		cerr << "\n";
	}
	*/
}



//////////////////////////////
//
// RollImage::assignMusicHoleIds --
//

void RollImage::assignMusicHoleIds(void) {
	vector<vector<HoleInfo*> >& ta = trackerArray;

	ulongint counter;
	ulongint key;
	for (ulongint i=0; i<ta.size(); i++) {
		counter = 1;
		key = midiToTrackMapping[i];
		for (ulongint j=0; j<ta[i].size(); j++) {
			if (!ta[i][j]->isMusicHole()) {
				continue;
			}
			ta[i][j]->id = "K" + my_to_string(key) + "_N" + my_to_string(counter++);
		}
	}
}



//////////////////////////////
//
// RollImage::analyzeTrackerBarPositions -- Now replaced with another algorithm
//

void RollImage::analyzeTrackerBarPositions(void) {
	int count = (int)holeSeparation;
	std::vector<double>& score = m_trackerShiftScores;
	score.resize(count);

	for (int i=0; i<count; i++) {
		score[i] = getTrackerShiftScore(i);
		// cerr << i << "\t" << score[i] << "\n";
	}

	int minindex = 0;
	for (ulongint i=1; i<score.size(); i++) {
		if (score[i] < score[minindex]) {
			minindex = i;
		}
	}

	double y1 = minindex < 1 ? score[count-1] : score[minindex - 1];
	double y2 = score[minindex];
	double y3 = minindex == count ? score[0] : score[minindex + 1];

	double b = (y3 - y2)/2.0;
	double a = y1/2.0 - y2 + y3/2.0;
	double newi = -b / 2 / a ;

	// cerr << "BASIC SEPARATION: " << minindex << " pixels\n";
	// cerr << "REFINED SEPARATION: " << newi << " pixels\n";
	// cerr << "FINAL ANSWER: " << (minindex + newi) << " pixels\n";

	holeOffset = minindex + newi;
	if (holeOffset > holeSeparation / 2) {
		holeOffset = holeSeparation - holeOffset;
	}

	// cerr << endl;
}



//////////////////////////////
//
// RollImage::getTrackerShiftScore --
//

double RollImage::getTrackerShiftScore(double shift) {
	std::vector<int>& x = correctedCentroidHistogram;
	double score = 0.0;
	for (ulongint i=0; i<x.size(); i++) {
		if (x[i] == 0) {
			continue;
		}
		double position = (i + shift) / holeSeparation;
		position = position - (int)position;
		if (position < 0.5) {
			score += x[i] * position;
		} else {
			score += x[i] * (1.0 - position);
		}
	}
	return score;
}



//////////////////////////////
//
// RollImage::analyzeRawRowPositions --
//

void RollImage::analyzeRawRowPositions(void) {
	std::vector<pair<double, int> >& rrp = rawRowPositions;
	rrp.resize(0);

	std::vector<int>& cch = correctedCentroidHistogram;
	for (uint i=0; i<cch.size(); i++) {
		if (cch[i] == 0) {
			continue;
		}
		i = storeWeightedCentroidGroup(i);
	}
}



//////////////////////////////
//
// RollImage::
//

void RollImage::calculateTrackerSpacings2(void) {
	std::vector<pair<double, int> >& rrp = rawRowPositions;
	if (rrp.empty()) {
		return;
	}

	double initialGuess = holeSeparation;
	int maxi = 0;
	for (ulongint i=1; i<rrp.size(); i++) {
		if (rrp[i].second > rrp[maxi].second) {
			maxi = i;
		}
	}
	double value = (rrp[maxi].first / initialGuess);
	value = value - int(value);
	if (value > 0.5) {
		value -= 1.0;
	}
	value *= initialGuess;
	holeOffset = value;
}



//////////////////////////////
//
// RollImage::storeWeightedCentroidGroup --
//

ulongint RollImage::storeWeightedCentroidGroup(ulongint startindex) {
	std::vector<pair<double, int> >& rrp = rawRowPositions;
	std::vector<int>& cch = correctedCentroidHistogram;

	ulongint holesum = 0;
	double weightedsum = 0.0;

	ulongint afterindex = startindex+1;
	for (ulongint i=startindex; i<cch.size(); i++) {
		if (cch[i] == 0) {
			afterindex = i;
			break;
		}
		afterindex = i+1;
		weightedsum += i * cch[i];
		holesum += cch[i];
	}
	if (holesum == 0) {
		return afterindex;
	}

	std::pair<double, int> value;
	value.first = weightedsum / (double)holesum;
	value.second = holesum;
	rrp.push_back(value);

	return afterindex;
}



//////////////////////////////
//
// RollImage::storeCorrectedCentroidHistogram --
//

void RollImage::storeCorrectedCentroidHistogram(void) {
	uncorrectedCentroidHistogram.resize(getCols(), 0);
	correctedCentroidHistogram.resize(getCols(), 0);

	for (ulongint i=0; i<holes.size(); i++) {
		ulongint centroidr = holes[i]->centroid.first;
		double correction = driftCorrection[centroidr];
		int position = holes[i]->centroid.second + correction + 0.5;
		correctedCentroidHistogram[position]++;
		position = holes[i]->centroid.second + 0.5;
		uncorrectedCentroidHistogram[position]++;
	}
}



//////////////////////////////
//
// RollImage::analyzeTrackerBarSpacing --
//

void RollImage::analyzeTrackerBarSpacing(void) {

	ulongint leftside  = getHardMarginLeftIndex();
	ulongint rightside = getHardMarginRightIndex();
	ulongint width     = rightside - leftside;
	ulongint padding = width / 10;
	leftside += padding;
	rightside -= padding;

	std::vector<mycomplex> spectrum;
	int factor = 16;
	std::vector<mycomplex> input(4096 * factor);
	for (ulongint i=0; i<4096; i++) {
		input.at(i) = correctedCentroidHistogram.at(i);
	}
	for (ulongint i=4096; i<input.size(); i++) {
		input.at(i) = 0.0;
	}
	
#ifndef DONOTUSEFFT

	FFT(spectrum, input);

	vector<double> magnitudeSpectrum(spectrum.size());
	int maxmagi = factor*2;
	for (ulongint i=0 ;i<spectrum.size(); i++) {
		magnitudeSpectrum.at(i) = std::abs(spectrum.at(i));
		if (i <= 50) {
			continue;
		}
		if (i > spectrum.size()/4) {
			continue;
		}
		if (magnitudeSpectrum.at(i) > magnitudeSpectrum.at(maxmagi)) {
			maxmagi = i;
		}
	}

	double y1 = magnitudeSpectrum.at(maxmagi-1);
	double y2 = magnitudeSpectrum.at(maxmagi);
	double y3 = magnitudeSpectrum.at(maxmagi+1);

	double estimate = 4096.0 * factor / maxmagi;
	double b = (y3 - y2)/2.0;
	double a = y1/2.0 - y2 + y3/2.0;
	double newi = -b / 2 / a / factor ;

	// cerr << "PIXEL SEPARATION: " << estimate << " pixels\n";
	// cerr << "REFINED PIXEL SEPARATION: " << newi << " pixels\n";
	// cerr << "FINAL ANSWER: " << (estimate + newi) << " pixels\n";

	holeSeparation = estimate + newi;
#else
	holeSeparation = 37.5;    /* for 8 holes/inch */
#endif /* DONOTUSEFFT */

	// cerr << "\n\nspectrum:\n";
	// for (ulongint i=0; i<spectrum.size(); i++) {
	// 	cerr << spectrum[i].first << "\t" << spectrum[i].second << endl;
	// }

}



//////////////////////////////
//
// RollImage::describeTears --
//

void RollImage::describeTears(void) {
	ulongint rows = getRows();
	ulongint cols = getCols();

	for (ulongint r=0; r<rows; r++) {
		for (ulongint c=0; c<cols/2; c++) {
			if (pixelType[r][c] != PIX_TEAR) {
				continue;
			}
			r = processTearLeft(r, c);
		}
	}

	for (ulongint r=0; r<rows; r++) {
		for (ulongint c=cols/2; c<cols; c++) {
			if (pixelType[r][c] != PIX_TEAR) {
				continue;
			}
			r = processTearRight(r, c);
		}
	}

}



//////////////////////////////
//
// RollImage::processTearLeft --
//

ulongint RollImage::processTearLeft(ulongint startrow, ulongint startcol) {
	ulongint rows = getRows();
	ulongint cols = getCols();
	ulongint area = 1;
	ulongint minc = startcol;
	ulongint maxc = startcol;
	ulongint minr = startrow;
	ulongint maxr = startrow;
	bool hastear;
	ulongint widththreshold = 10; // put into RollOptions
	ulongint mintearwidth   = 30; // put into RollOptions

	for (ulongint r=startrow; r<rows; r++) {
		hastear = false;
		for (ulongint c=cols/2; c>1; c--) {
			if (pixelType[r][c] != PIX_TEAR) {
				continue;
			}
			hastear = true;
			area++;
			if (c < minc) { minc = c; }
			if (c > maxc) { maxc = c; }
		}
		if (!hastear) {
			break;
		}
		maxr = r;
	}

	ulongint minarea = 30;  // move to RollOptions

	if (maxc - minc + 1 <= widththreshold) {
		removeTearLeft(minr, maxr, minc, maxc);
	} else if (area < minarea) {
		// do nothing
	} else if (maxc - minc + 1 >= mintearwidth) {
		TearInfo* ti = new TearInfo;
		ti->origin.first = minr;
		ti->origin.second = minc;
		ti->width.first = maxr - minr + 1;
		ti->width.second = maxc - minc + 1;
		ti->area = area;
		bassTears.push_back(ti);
	}

	return maxr;
}



//////////////////////////////
//
// RollImage::processTearRight --
//

ulongint RollImage::processTearRight(ulongint startrow, ulongint startcol) {
	ulongint rows = getRows();
	ulongint cols = getCols();
	ulongint area = 1;
	ulongint minc = startcol;
	ulongint maxc = startcol;
	ulongint minr = startrow;
	ulongint maxr = startrow;
	bool hastear;
	ulongint widththreshold = 10; // put into RollOptions
	ulongint mintearwidth   = 30; // put into RollOptions

	for (ulongint r=startrow; r<rows; r++) {
		hastear = false;
		for (ulongint c=cols/2; c<cols; c++) {
			if (pixelType[r][c] != PIX_TEAR) {
				continue;
			}
			hastear = true;
			area++;
			if (c < minc) { minc = c; }
			if (c > maxc) { maxc = c; }
		}
		if (!hastear) {
			break;
		}
		maxr = r;
	}

	if (maxc - minc + 1 <= widththreshold) {
		removeTearRight(minr, maxr, minc, maxc);
	} else if (maxc - minc + 1 >= mintearwidth) {
		TearInfo* ti = new TearInfo;
		ti->origin.first = minr;
		ti->origin.second = minc;
		ti->width.first = maxr - minr + 1;
		ti->width.second = maxc - minc + 1;
		ti->area = area;
		trebleTears.push_back(ti);
	}

	return maxr;
}



//////////////////////////////
//
// RollImage::removeTearLeft --
//

void RollImage::removeTearLeft(ulongint minrow, ulongint maxrow, ulongint mincol, ulongint maxcol) {
	ulongint r;
	ulongint c;
	for (r=minrow; r<=maxrow; r++) {
		for (c=mincol; c<=maxcol; c++) {
			if (pixelType[r][c] != PIX_TEAR) {
				continue;
			}
			pixelType[r][c] = PIX_MARGIN;
			if (leftMarginIndex[r] < (int)c) {
				leftMarginIndex[r] = c;
			}
		}
	}
}



//////////////////////////////
//
// RollImage::removeTearRight --
//

void RollImage::removeTearRight(ulongint minrow, ulongint maxrow, ulongint mincol, ulongint maxcol) {
	ulongint r;
	ulongint c;
	for (r=minrow; r<=maxrow; r++) {
		for (c=mincol; c<=maxcol; c++) {
			if (pixelType[r][c] != PIX_TEAR) {
				continue;
			}
			pixelType[r][c] = PIX_MARGIN;
			if (leftMarginIndex[r] > (int)c) {
				leftMarginIndex[r] = c;
			}
		}
	}
}



//////////////////////////////
//
// RollImage::analyzeTears -- Find tears in the edges of the rolls.
//

void RollImage::analyzeTears(void) {

	ulongint rows = getRows();
	std::vector<double> fastLeft(rows);
	std::vector<double> fastRight(rows);
	std::vector<double> mediumLeft(rows);
	std::vector<double> mediumRight(rows);
	std::vector<double> slowLeft(rows);
	std::vector<double> slowRight(rows);

	for (ulongint r=0; r<rows; r++) {
		fastLeft[r]    = leftMarginIndex[r];
		mediumLeft[r]  = leftMarginIndex[r];
		slowLeft[r]    = leftMarginIndex[r];
		fastRight[r]   = rightMarginIndex[r];
		mediumRight[r] = rightMarginIndex[r];
		slowRight[r]   = rightMarginIndex[r];
	}
	exponentialSmoothing(fastLeft,    0.100);
	exponentialSmoothing(fastRight,   0.100);
	exponentialSmoothing(mediumLeft,  0.050);
	exponentialSmoothing(mediumRight, 0.050);
	exponentialSmoothing(slowLeft,    0.001);
	exponentialSmoothing(slowRight,   0.001);

	ulongint startr = getFirstMusicHoleStart();
	int rfactor = 300;  // expansion of tear search windows
	int wfactor = 5;    // trigger deviation between slow margin and raw margin
	int cols = getCols();
	int c;

	// Generate a boolean vector where true means the the margins
	// are stable, and false means they are unstable (probably due
	// to a tear on one or both sides).

	std::vector<bool> stableRegion(rows, true);
	std::vector<bool> stableLeft(rows, true);
	std::vector<bool> stableRight(rows, true);
	std::vector<double> fwidths(rows, 0.0);  // fast (now raw) width of paper
	std::vector<double> swidths(rows, 0.0);  // slow width of paper
	double sum = 0.0;
	int counter = 0;
	for (ulongint r=0; r<rows; r++) {
		// double fwidth = fastRight[r] - fastLeft[r];
		double fwidth = rightMarginIndex[r] - leftMarginIndex[r];
		double swidth = slowRight[r] - slowLeft[r];
		swidths[r] = swidth;
		fwidths[r] = fwidth;
		if (fabs(swidth - fwidth) > wfactor) {
			stableRegion[r] = false;
		} else {
			if (r > 10000) {
				// don't look at leader
				sum += swidth;
				counter++;
			}
		}
		if (fabs(fastLeft[r] - slowLeft[r]) > wfactor) {
			stableLeft[r] = false;
		}
		if (fabs(fastRight[r] - slowRight[r]) > wfactor) {
			stableRight[r] = false;
		}
	}

	double avgwidth = sum / counter;

	std::vector<bool> sr(stableRegion.size(), true);

	// expand unstable paper-width regions
	for (ulongint r=1; r<rows-rfactor-1; r++) {
		if (stableRegion[r] == false) {
			sr[r] = false;
		}
		if ((stableRegion[r] == false) && (stableRegion[r+1] == true)) {
			for (int s=0; s<rfactor; s++) {
				if (r+s >= rows) {
					break;
				} else {
					sr.at(r+s) = false;
				}
			}
			r = r+rfactor-1;
		}
	}

	// expand the other direction
	for (ulongint r=rows-1; r>(ulongint)rfactor; r--) {
		if (sr[r] == false) {
			stableRegion[r] = false;
		}
		if ((sr[r] == false) && (sr[r-1] == true)) {
			for (int s=0; s<rfactor; s++) {
				if (r-s <= 0) {
					break;
				} else {
					stableRegion.at(r-s) = false;
				}
			}
			r = r-rfactor+1;
		}
	}

	// adjust tear region to expected paper width if one side
	// was stable and the other was not.
	for (ulongint r=startr; r<rows; r++) {
		if (stableLeft[r] && !stableRight[r]) {
			int startindex = slowLeft[r] + avgwidth;
			rightMarginIndex[r] = startindex;
			for (c=startindex; (c > 0) && (pixelType[r][c] == PIX_MARGIN); c--) {
				pixelType[r][c] = PIX_TEAR;
				// back-fill due to dust:
				for (ulongint rr = r-1; rr >= startr; r--) {
					if (pixelType[rr][c] == PIX_MARGIN) {
						pixelType[rr][c] = PIX_TEAR;
					} else {
						break;
					}
				}
			}
		}

		if (stableRight[r] && !stableLeft[r]) {
			int startindex = slowRight[r] - avgwidth;
			leftMarginIndex[r] = startindex;
			for (c=cols/2; c>=startindex; c--) {
				if (pixelType[r][c] == PIX_MARGIN) {
					pixelType[r][c] = PIX_TEAR;
				}
			}
		}
	}


	// recalculate curves
	for (ulongint r=0; r<rows; r++) {
		fastLeft[r]    = leftMarginIndex[r];
		mediumLeft[r]  = leftMarginIndex[r];
		slowLeft[r]    = leftMarginIndex[r];
		fastRight[r]   = rightMarginIndex[r];
		mediumRight[r] = rightMarginIndex[r];
		slowRight[r]   = rightMarginIndex[r];
	}
	exponentialSmoothing(fastLeft,    0.100);
	exponentialSmoothing(fastRight,   0.100);
	exponentialSmoothing(mediumLeft,  0.050);
	exponentialSmoothing(mediumRight, 0.050);
	exponentialSmoothing(slowLeft,    0.001);
	exponentialSmoothing(slowRight,   0.001);

	int xvalue = 10;
	// Initial marking of tears:
	for (ulongint r=startr; r<rows; r++) {
		if (stableRegion[r]) {
			continue;
		}
		if (mediumLeft[r] < slowLeft[r] + xvalue) {
			continue;
		}
		if (leftMarginIndex[r] > slowLeft[r]) {
			leftMarginIndex[r] = slowLeft[r];
		}
		for (c=slowLeft[r]; c<cols/2; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_TEAR;
			}
		}
	}
	// do the same thing on the right side.
	for (ulongint r=startr; r<rows; r++) {
		if (stableRegion[r]) {
			continue;
		}
		if (mediumRight[r] > slowRight[r] + xvalue) {
			continue;
		}
		if (rightMarginIndex[r] < slowRight[r]) {
			rightMarginIndex[r] = slowRight[r];
		}
		for (c=cols/2; c < slowRight[r]; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_TEAR;
			}
		}
	}


	// recalculate curves
	for (ulongint r=0; r<rows; r++) {
		fastLeft[r]    = leftMarginIndex[r];
		mediumLeft[r]  = leftMarginIndex[r];
		slowLeft[r]    = leftMarginIndex[r];
		fastRight[r]   = rightMarginIndex[r];
		mediumRight[r] = rightMarginIndex[r];
		slowRight[r]   = rightMarginIndex[r];
	}
	exponentialSmoothing(fastLeft,    0.100);
	exponentialSmoothing(fastRight,   0.100);
	exponentialSmoothing(mediumLeft,  0.050);
	exponentialSmoothing(mediumRight, 0.050);
	exponentialSmoothing(slowLeft,    0.001);
	exponentialSmoothing(slowRight,   0.001);

	// do another fill to get closer to true edges without tears
	for (ulongint r=startr; r<rows; r++) {
		if (stableRegion[r]) {
			continue;
		}
		if (mediumLeft[r] < slowLeft[r] + xvalue) {
			continue;
		}
		if (leftMarginIndex[r] > slowLeft[r]) {
				leftMarginIndex[r] = slowLeft[r];
		}
		for (c=slowLeft[r]; c<cols/2; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_TEAR;
			}
		}
	}
	// do the same thing on the right side.
	for (ulongint r=startr; r<rows; r++) {
		if (stableRegion[r]) {
			continue;
		}
		if (mediumRight[r] > slowRight[r] + xvalue) {
			continue;
		}
		if (rightMarginIndex[r] < slowRight[r]) {
				rightMarginIndex[r] = slowRight[r];
		}
		for (c=cols/2; c < slowRight[r]; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_TEAR;
			}
		}
	}


	// recaulate curves again
	for (ulongint r=0; r<rows; r++) {
		fastLeft[r]    = leftMarginIndex[r];
		mediumLeft[r]  = leftMarginIndex[r];
		slowLeft[r]    = leftMarginIndex[r];
		fastRight[r]   = rightMarginIndex[r];
		mediumRight[r] = rightMarginIndex[r];
		slowRight[r]   = rightMarginIndex[r];
	}
	exponentialSmoothing(fastLeft,    0.100);
	exponentialSmoothing(fastRight,   0.100);
	exponentialSmoothing(mediumLeft,  0.050);
	exponentialSmoothing(mediumRight, 0.050);
	exponentialSmoothing(slowLeft,    0.001);
	exponentialSmoothing(slowRight,   0.001);
	// fill between the margin and the slow edges
	for (ulongint r=startr; r<rows; r++) {
		if (stableRegion[r]) {
			continue;
		}
		if (slowLeft[r] >= leftMarginIndex[r]) {
			continue;
		}
		int start = leftMarginIndex[r];
		for (c=start; c>=slowLeft[r]; c--) {
			if (pixelType[r][c] != PIX_PAPER) {
				pixelType[r][c] = PIX_TEAR;
				leftMarginIndex[r] = c;
			}
		}
	}
	// Same on other side
	for (ulongint r=startr; r<rows; r++) {
		if (stableRegion[r]) {
			continue;
		}
		if (slowRight[r] <= rightMarginIndex[r]) {
			continue;
		}
		int start = rightMarginIndex[r];
		for (c=start; c<=slowRight[r]; c++) {
			if (pixelType[r][c] != PIX_PAPER) {
				pixelType[r][c] = PIX_TEAR;
				rightMarginIndex[r] = c;
			}
		}
	}


	describeTears();

/*
	// push margins off of paper if they got on (due to tears)
	for (ulongint r=startr; r<rows; r++) {
		if (pixelType[r][leftMarginIndex[r]] == PIX_PAPER) {
			c = leftMarginIndex[r];
			while ((c > 1) && (pixelType[r][c] == PIX_PAPER)) {
				c--;
			}
			leftMarginIndex[r] = c;
		}
	}
	for (ulongint r=startr; r<rows; r++) {
		if (pixelType[r][rightMarginIndex[r]] == PIX_PAPER) {
			c = rightMarginIndex[r];
			while ((c < cols-1) && (pixelType[r][c] == PIX_PAPER)) {
				c++;
			}
			rightMarginIndex[r] = c;
		}
	}

	// Get rid of some noise and dust cases in raw margins
	for (ulongint r=startr+1; r<rows-1; r++) {
		if ((leftMarginIndex[r] < leftMarginIndex[r-1]) &&
				(leftMarginIndex[r] < leftMarginIndex[r+1])) {
			leftMarginIndex[r] = int(leftMarginIndex[r-1] + leftMarginIndex[r+1] + 0.5)/2.0;
		}
	}
	for (ulongint r=startr+1; r<rows-1; r++) {
		if ((rightMarginIndex[r] > rightMarginIndex[r-1]) &&
				(rightMarginIndex[r] > rightMarginIndex[r+1])) {
			rightMarginIndex[r] = int(rightMarginIndex[r-1] + rightMarginIndex[r+1] + 0.5)/2.0;
		}
	}
*/

	return;
}



//////////////////////////////
//
// RollImage::analyzeShifts --
//

void RollImage::analyzeShifts(void) {
	ulongint rows = getRows();
	std::vector<double> fastLeft(rows);
	std::vector<double> fastRight(rows);
	std::vector<double> mediumLeft(rows);
	std::vector<double> mediumRight(rows);
	std::vector<double> slowLeft(rows);
	std::vector<double> slowRight(rows);

	for (ulongint r=0; r<rows; r++) {
		fastLeft[r]    = leftMarginIndex[r];
		mediumLeft[r]  = leftMarginIndex[r];
		slowLeft[r]    = leftMarginIndex[r];
		fastRight[r]   = rightMarginIndex[r];
		mediumRight[r] = rightMarginIndex[r];
		slowRight[r]   = rightMarginIndex[r];
	}
	exponentialSmoothing(fastLeft,    0.100);
	exponentialSmoothing(fastRight,   0.100);
	exponentialSmoothing(mediumLeft,  0.050);
	exponentialSmoothing(mediumRight, 0.050);
	exponentialSmoothing(slowLeft,    0.001);
	exponentialSmoothing(slowRight,   0.001);

	int wfactor = 5;    // trigger deviation between slow margin and raw margin

	// Generate a boolean vector where true means the the margins
	// are stable, and false means they are unstable (probably due
	// to a tear on one or both sides).

	std::vector<bool> stableRegion(rows, true);
	std::vector<bool> stableRight(rows, true);
	std::vector<double> fwidths(rows, 0.0);  // fast (now raw) width of paper
	std::vector<double> swidths(rows, 0.0);  // slow width of paper
	double sum = 0.0;
	int counter = 0;
	for (ulongint r=0; r<rows; r++) {
		double fwidth = fastRight[r] - fastLeft[r];
		// double fwidth = rightMarginIndex[r] - leftMarginIndex[r];
		double swidth = slowRight[r] - slowLeft[r];
		swidths[r] = swidth;
		fwidths[r] = fwidth;
		if (fabs(swidth - fwidth) > wfactor) {
			stableRegion[r] = false;
		} else {
			if (r > 10000) {
				// don't look at leader
				sum += swidth;
				counter++;
			}
		}
	}

	ulongint firstrow = getFirstMusicHoleStart() - 100;
	int window = 50;
	std::vector<double> leftDiff(rows, 0.0);
	std::vector<double> rightDiff(rows, 0.0);

	for (ulongint r=firstrow+window; r<rows-window; r++) {
		// shift left is negative, shift right is positive
		leftDiff[r] = fastLeft[r+window] - fastLeft[r-window];
		rightDiff[r] = fastRight[r+window] - fastRight[r-window];
	}

//ggg debugging information:

	double sthresh = 3;
	double sthresh2 = 3;

	std::vector<double> score(rows, 0.0);
	for (ulongint r=0; r<rows; r++) {
		if (fabs(leftDiff[r]) < sthresh) {
			continue;
		}
		if (fabs(rightDiff[r]) < sthresh) {
			continue;
		}
		if (fabs(leftDiff[r] - rightDiff[r]) > sthresh2) {
			continue;
		}

		score[r] = (leftDiff[r] + rightDiff[r]) / 2.0;

		if (score[r] < sthresh2) {
			continue;
		}

		continue;

		// the following can be used to graph anlysis in margins:

		double value = 4 * leftDiff[r] + 80;
		double value2 = 4 * rightDiff[r] + 160;

		if (value >= 0) {
			pixelType[r][80] = PIX_DEBUG4;
			pixelType[r][value] = PIX_DEBUG1;
		}
		if (value2 >= 0) {
			pixelType[r][160] = PIX_DEBUG4;
			pixelType[r][value2] = PIX_DEBUG1;
		}
	}

	for (ulongint r=0; r<rows; r++) {
		if (score[r] == 0) {
			continue;
		}
		r = storeShift(score, r);
	}
}



//////////////////////////////
//
// RollImage::storeShift --
//

ulongint RollImage::storeShift(std::vector<double>& scores, ulongint startrow) {
	ulongint r = startrow;
	int count = 0;
	double maxvalue = 0.0;
	double minvalue = 0.0;
	while ((r < scores.size()) && (scores[r] != 0.0)) {
		if (scores[r] > maxvalue) {
			maxvalue = scores[r];
		}
		if (scores[r] < minvalue) {
			minvalue = scores[r];
		}
		count++;
		r++;
	}

	int mincount = 30;
	if (count < mincount) {
		// don't store tiny shifts
		return r;
	}

	if (count == 0) {
		// strange: shouldn't happen unless very end of image
		return r;
	}

	double value = maxvalue;
	if (fabs(minvalue) > maxvalue) {
		value = minvalue;
	}

	ShiftInfo* si = new ShiftInfo;
	si->row = startrow + count/2;
	si->score = value;
	shifts.push_back(si);

	return r;
}





//////////////////////////////
//
// RollImage::fillColumn --
//

void RollImage::fillColumn(ulongint col, ulongint toprow, ulongint botrow, ulongint target, ulongint threshold, ulongint replacement,
		std::vector<int>& margin) {

	if (toprow > botrow) {
		ulongint temp = toprow;
		toprow = botrow;
		botrow = temp;
	}

	ulongint maxup    = toprow - threshold;
	ulongint maxdown  = botrow + threshold;
	ulongint midpoint = abs((int)maxup-(int)maxdown)/2 + maxup;

	for (ulongint r=midpoint; r>=maxup; r--) {
		if (pixelType[r][col] == target) {
			pixelType[r][col] = (char)replacement;
			margin[r] = col;
		} else {
			break;
		}
	}

	for (ulongint r=midpoint; r<=maxdown; r++) {
		if (pixelType[r][col] == PIX_PAPER) {
			pixelType[r][col] = (char)replacement;
			margin[r] = col;
		} else {
			break;
		}
	}
}



//////////////////////////////
//
// RollImage::goodColumn -- returns true if all pixels within threshold pixels of the endpoints
//    are the given pixel type.
//

bool RollImage::goodColumn(ulongint col, ulongint toprow, ulongint botrow, ulongint ptype, ulongint threshold) {
	if (toprow > botrow) {
		ulongint temp = toprow;
		toprow = botrow;
		botrow = temp;
	}

	ulongint maxup    = toprow - threshold;
	ulongint maxdown  = botrow + threshold;
	ulongint midpoint = abs((int)maxup-(int)maxdown)/2 + maxup;

	ulongint toppaper = 0;
	ulongint botpaper = 0;

	for (ulongint r=midpoint; r>=maxup; r--) {
		pixelType[r][col] = PIX_DEBUG3;
		if (pixelType[r][col] == PIX_PAPER) {
			toppaper = r;
			break;
		}
	}
	if (toppaper == 0) {
		return false;
	}
	if (abs((int)toppaper - (int)toprow) > threshold) {
		return false;
	}

	for (ulongint r=midpoint; r<=maxdown; r++) {
		pixelType[r][col] = PIX_DEBUG2;
		if (pixelType[r][col] == PIX_PAPER) {
			botpaper = r;
			break;
		}
	}
	if (botpaper <= 0) {
		return false;
	}
	if (abs((int)botrow - (int)botpaper) > threshold) {
		return false;
	}

	return true;
}



//////////////////////////////
//
// RollImage::findPeak --
//

ulongint RollImage::findPeak(std::vector<double>& array, ulongint r, ulongint& peakindex,
		double& peakvalue) {
	peakindex = r;
	peakvalue = array[r];
	ulongint rows = getRows();
	for (ulongint i=r; i<rows; i++) {
		if (array[i] == 0.0) {
			return i;
		}
		if (array[i] > array[peakindex]) {
			peakindex = i;
			peakvalue = array[i];
		}
	}
	return rows;
}



//////////////////////////////
//
// RollImage::generateDriftCorrection -- gain = 0.01
//

void RollImage::generateDriftCorrection(double gain) {

	ulongint rows = getRows();
	std::vector<double> lmargin(rows);
	std::vector<double> rmargin(rows);

	for (ulongint r=0; r<rows; r++) {
		lmargin[r] = leftMarginIndex[r];
		rmargin[r] = rightMarginIndex[r];
	}

	exponentialSmoothing(lmargin, gain);
	exponentialSmoothing(rmargin, gain);

	ulongint startrow = getLeaderIndex() + 100;
	ulongint endrow   = getRows() - 100;
	ulongint length = endrow - startrow + 1;

	double lavg = getAverage(lmargin, startrow, length);
	double ravg = getAverage(rmargin, startrow, length);

	driftCorrection.resize(rows);
	std::fill(driftCorrection.begin(), driftCorrection.end(), 0.0);

	for (ulongint r=startrow; r<=endrow; r++) {
		driftCorrection[r] = -((lmargin[r] - lavg) + (rmargin[r] - ravg)) / 2.0;
	}

	// Store the drift correction for each hole (at the centroid point,
	// although perhaps the leading edge drift might be better).
	for (ulongint i=0; i<holes.size(); i++) {
		ulongint r = holes[i]->centroid.first;
		if (r > 0) {
			holes[i]->coldrift = driftCorrection.at(r);
		}
	}

}



//////////////////////////////
//
// RollImage::markPosteriorLeader --
//

void RollImage::markPosteriorLeader(void) {
	ulongint startrow = getLeaderIndex() + 1;
	ulongint endrow   = getFirstMusicHoleStart() - 1;

	ulongint cols = getCols();
	for (ulongint r=startrow; r<=endrow; r++) {
		for (ulongint c=0; c<cols; c++) {
			if (pixelType[r][c] != PIX_PAPER) {
				pixelType[r][c] = PIX_POSTLEADER;
			}
		}
	}

	ulongint rows = getRows();
	endrow = rows - 1;
	startrow = getLastMusicHoleEnd() + 1;
	for (ulongint r=startrow; r<=endrow; r++) {
		for (ulongint c=0; c<cols; c++) {
			if (pixelType[r][c] != PIX_PAPER) {
				pixelType[r][c] = PIX_POSTMUSIC;
			}
		}
	}

}



//////////////////////////////
//
// RollImage::analyzeHoles --
//

void RollImage::analyzeHoles(void) {
	int   startcol = getHardMarginLeftIndex()+1;
	int   endcol   = getHardMarginRightIndex();
	ulongint startrow = getLeaderIndex();
	ulongint endrow   = getRows();
	holes.clear();
	holes.reserve(getMaxHoleCount() + 1024);

	for (ulongint r=startrow; r<endrow; r++) {
		for (int c=startcol; c<endcol; c++) {
			if (pixelType.at(r).at(c) == PIX_NONPAPER) {
				extractHole(r, c);
				if ((int)holes.size() > getMaxHoleCount()) {
					cerr << "Too many holes, giving up after " << getMaxHoleCount() << " holes." << endl;
					return;
				}
			}
		}
	}
}



//////////////////////////////
//
// RollImage::extractHole --
//

void RollImage::extractHole(ulongint row, ulongint col) {
	HoleInfo* hi = new HoleInfo;

	hi->origin.first = row;
	hi->origin.second = col;
	hi->width.first = row;
	hi->width.second = col;

	int counter = 0;
	fillHoleInfo(*hi, row, col, counter);
	hi->entry.first  = row;
	hi->entry.second = col;
	hi->centroid.first  /= hi->area;
	hi->centroid.second /= hi->area;
	// hi->coldrift set in RollImage::generateDriftCorrection.

	ulongint testFirst = hi->origin.first;
	ulongint testLast  = hi->width.first;

	// Convert lower right corner corrdinate into a width:
	hi->width.first  = hi->width.first  - hi->origin.first;
	hi->width.second = hi->width.second - hi->origin.second;

	ulongint minarea = 100;
	if (hi->area > minarea) {
		holes.push_back(hi);
		if ((firstMusicRow == 0) || (testFirst < firstMusicRow)) {
			firstMusicRow = testFirst;
		}
		if (testLast > lastMusicRow) {
			lastMusicRow = testLast;
		}
	} else {
		// Too small to be considered a musical hole.
		clearHole(*hi, PIX_ANTIDUST);
		hi->reason = "small";
		hi->track = 0;
		antidust.push_back(hi);
	}
}



//////////////////////////////
//
// RollImage::fillHoleSimple --
//

void RollImage::fillHoleSimple(ulongint r, ulongint c, int target, int type, int& counter) {
	counter++;
	if (counter > getMaxHoleCount()) {
		cerr << "CLEARING TOO LARGE A HOLE!" << endl;
		return;
	}
	if (r >= getRows()) {
		return;
	}
	if (c >= getCols()) {
		return;
	}
	if (pixelType.at(r).at(c) != target) {
		return;
	} else {
		pixelType.at(r).at(c) = type;
	}
	if ((r > 0) && (c > 0)) {
		fillHoleSimple(r-1, c-1, target, type, counter);
	}
	if (r > 0) {
		fillHoleSimple(r-1, c  , target, type, counter);
	}
	if (r > 0) {
		fillHoleSimple(r-1, c+1, target, type, counter);
	}
	if (c > 0) {
		fillHoleSimple(r  , c-1, target, type, counter);
	}
	fillHoleSimple(r  , c+1, target, type, counter);
	if (c > 0) {
		fillHoleSimple(r+1, c-1, target, type, counter);
	}
	fillHoleSimple(r+1, c  , target, type, counter);
	fillHoleSimple(r+1, c+1, target, type, counter);

}



//////////////////////////////
//
// RollImage::fillHoleInfo --
//

void RollImage::fillHoleInfo(HoleInfo& hi, ulongint r, ulongint c, int& counter) {
	counter++;
	if (counter > 300000) {
		cerr << "HOLE TOO BIG " << endl;
		return;
	}

	if (r >= getRows()) {
		return;
	}
	if (c >= getCols()) {
		return;
	}

	if (pixelType.at(r).at(c) != PIX_NONPAPER) {
		return;
	} else {
		pixelType.at(r).at(c) = PIX_HOLE;
	}
	if (r < hi.origin.first) {
		hi.origin.first = r;
	}
	if (c < hi.origin.second) {
		hi.origin.second = c;
	}
	if (r > hi.width.first) {
		hi.width.first = r;
	}
	if (c > hi.width.second) {
		hi.width.second = c;
	}
	hi.area++;
	hi.centroid.first  += r;
	// hi.centroid.second += c + driftCorrection[r];
	hi.centroid.second += c;

	if ((r > 0) && (c > 0)) {
		fillHoleInfo(hi, r-1, c-1, counter);
	}
	if (r > 0) {
		fillHoleInfo(hi, r-1, c  , counter);
		fillHoleInfo(hi, r-1, c+1, counter);
	}
	if (c > 0) {
		fillHoleInfo(hi, r  , c-1, counter);
	}
	fillHoleInfo(hi, r  , c+1, counter);
	if (c > 0) {
		fillHoleInfo(hi, r+1, c-1, counter);
	}
	fillHoleInfo(hi, r+1, c  , counter);
	fillHoleInfo(hi, r+1, c+1, counter);
}



//////////////////////////////
//
// RollImage::fillTearInfo --
//

void RollImage::fillTearInfo(TearInfo& ti, ulongint r, ulongint c, int& counter) {
	counter++;
	if (counter > getMaxTearFill()) {
		cerr << "ERROR TEAR TOO LARGE" << endl;
		return;
	}

	if (r >= getRows()) {
		return;
	}
	if (c >= getCols()) {
		return;
	}

	if (pixelType.at(r).at(c)== PIX_NONPAPER) {
		pixelType.at(r).at(c) = PIX_TEAR;
	} else if (pixelType.at(r).at(c) == PIX_MARGIN) {
		pixelType.at(r).at(c) = PIX_TEAR;
	} else {
		return;
	}

	if (ti.origin.first == 0) {
		ti.origin.first = r;
	} else if (r < ti.origin.first) {
		ti.origin.first = r;
	}

	if (ti.origin.second == 0) {
		ti.origin.second = c;
	} else if (c < ti.origin.second) {
		ti.origin.second = c;
	}

	if (ti.width.first == 0) {
		ti.width.first = r;
	} else if (r > ti.width.first) {
		ti.width.first = r;
	}

	if (ti.width.second == 0) {
		ti.width.second = c;
	} else if (c > ti.width.second) {
		ti.width.second = c;
	}

	ti.area++;
	ti.centroid.first  += r;
	// ti.centroid.second += c + driftCorrection[r];
	ti.centroid.second += c ;

	if ((c > 0) && (r > 0)) {
		fillTearInfo(ti, r-1, c-1, counter);
	}
	if (r > 0) {
		fillTearInfo(ti, r-1, c  , counter);
		fillTearInfo(ti, r-1, c+1, counter);
	}
	if (c > 0) {
		fillTearInfo(ti, r  , c-1, counter);
	}
	fillTearInfo(ti, r  , c+1, counter);
	if (c > 0) {
		fillTearInfo(ti, r+1, c-1, counter);
	}
	fillTearInfo(ti, r+1, c  , counter);
	fillTearInfo(ti, r+1, c+1, counter);
}



//////////////////////////////
//
// RollImage::analyzeAdvancedMargins -- Separate the hard and soft margins.
//

void RollImage::analyzeAdvancedMargins(void) {
	analyzeHardMargins(getLeaderIndex());
	m_analyzedAdvancedMargins = true;
}



//////////////////////////////
//
// RollImage::analyzeBasicMargins --
//

void RollImage::analyzeBasicMargins(void) {
	getRawMargins();
	waterfallDownMargins();
	waterfallUpMargins();
	waterfallLeftMargins();
	waterfallRightMargins();

	m_analyzedBasicMargins = true;
}


//////////////////////////////
//
// RollImage::getRawMargins -- Identifies the pixel position of the
//   left and right margins. The index value stored in the margins
//   is the closest pixel in the margin to the paper, searched
//   by moving horizontally from the edge of the image.  Dust will
//   cause problems that are fixed later in waterfallDownMargins().
//

void RollImage::getRawMargins(void) {
	ulongint rows = getRows();
	ulongint cols = getCols();

	leftMarginIndex.resize(rows);
	rightMarginIndex.resize(rows);

	int startcol = 5; // starting a little off of the margin due to digital noise
	                  // the second and third columns.
	for (ulongint r=0; r<rows; r++) {
		std::vector<ucharint>& rowdata = pixelType.at(r);
		leftMarginIndex[r] = 0;
		for (ulongint c=startcol; c<cols; c++) {
			if (rowdata.at(c) == PIX_PAPER) {
				leftMarginIndex[r] = c - 1;
				break;
			} else {
				rowdata.at(c) = PIX_MARGIN;
				leftMarginIndex[r] = c;
			}
		}
	}


	for (ulongint r=0; r<rows; r++) {
		std::vector<ucharint>& rowdata = pixelType.at(r);
		rightMarginIndex[r] = 0;
		for (int c=cols-1-startcol; c>=0; c--) {
			if (rowdata.at(c) == PIX_PAPER) {
				rightMarginIndex[r] = c + 1;
				break;
			} else {
				rowdata.at(c) = PIX_MARGIN;
				rightMarginIndex[r] = c;
			}
		}
	}
}



//////////////////////////////
//
// RollImage::waterfallDownMargins -- Fill in margin areas that are blocked
//     from the left/right by dust.
//

void RollImage::waterfallDownMargins(void) {
	ulongint rows = getRows();
	int cols = (int)getCols();

	for (ulongint r=0; r<rows-1; r++) {
		std::vector<ucharint>& row1 = pixelType[r];
		std::vector<ucharint>& row2 = pixelType[r+1];
		for (int c=0; c<cols; c++) {
			if (row1[c] != PIX_MARGIN) {
				continue;
			}
			if (row2[c] != PIX_PAPER) {
				row2[c] = PIX_MARGIN;
				if (c < cols/2) {
					if (c > leftMarginIndex[r+1]) {
						leftMarginIndex[r+1] = c;
					}
				} else {
					if (c < rightMarginIndex[r+1]) {
						rightMarginIndex[r+1] = c;
					}
				}
			}
		}
	}
}



//////////////////////////////
//
// RollImage::waterfallUpMargins -- Fill in margin areas that are blocked
//     from the left/right by dust by going up from bottom of image.
//

void RollImage::waterfallUpMargins(void) {
	ulongint rows = getRows();
	ulongint cols = getCols();

	for (ulongint r=rows-1; r>0; r--) {
		std::vector<ucharint>& row1 = pixelType.at(r);
		std::vector<ucharint>& row2 = pixelType.at(r-1);
		for (ulongint c=0; c<cols; c++) {
			if (row1.at(c) != PIX_MARGIN) {
				continue;
			}
			if (row2.at(c) != PIX_PAPER) {
				row2.at(c) = PIX_MARGIN;
				if (c < cols/2) {
					if (c > (ulongint)leftMarginIndex.at(r-1)) {
						leftMarginIndex.at(r-1) = c;
					}
				} else {
					if (c < (ulongint)rightMarginIndex.at(r-1)) {
						rightMarginIndex.at(r-1) = c;
					}
				}
			}
		}
	}
}



//////////////////////////////
//
// RollImage::waterfallRightMargins -- Fill in margin areas that are blocked
//     from up/down by dust by going right from the left side of the image.
//     This function is needed to go around fingerprints to avoid spurious
//     hole detection on the edges of the roll.
//

void RollImage::waterfallRightMargins(void) {
	ulongint rows = getRows();
	ulongint cols = getCols();

	for (ulongint c=0; c<cols-1; c++) {
		for (ulongint r=0; r<rows; r++) {
			if (pixelType.at(r).at(c) != PIX_MARGIN) {
				continue;
			}
			if (pixelType.at(r).at(c+1) != PIX_PAPER) {
				pixelType.at(r).at(c+1) = PIX_MARGIN;
				if (c < cols/2) {
					if (c+1 > (ulongint)leftMarginIndex.at(r)) {
						leftMarginIndex.at(r) = c+1;
					}
				} else {
					if (c+1 < (ulongint)rightMarginIndex.at(r)) {
						rightMarginIndex.at(r-1) = c+1;
					}
				}
			}
		}
	}
}



//////////////////////////////
//
// RollImage::waterfallLeftMargins -- Fill in margin areas that are blocked
//     from up/down by dust by going left from the right side of the image.
//     This function is needed to go around fingerprints to avoid spurious
//     hole detection on the edges of the roll.
//

void RollImage::waterfallLeftMargins(void) {
	ulongint rows = getRows();
	ulongint cols = getCols();

	for (ulongint c=cols-1; c>0; c--) {
		for (ulongint r=0; r<rows; r++) {
			if (pixelType.at(r).at(c) != PIX_MARGIN) {
				continue;
			}
			if (pixelType.at(r).at(c-1) != PIX_PAPER) {
				pixelType.at(r).at(c-1) = PIX_MARGIN;
				if (c < cols/2) {
					if (c-1 > (ulongint)leftMarginIndex.at(r)) {
						leftMarginIndex.at(r) = c-1;
					}
				} else {
					if (c-1 < (ulongint)rightMarginIndex.at(r)) {
						rightMarginIndex.at(r) = c-1;
					}
				}
			}
		}
	}
}



//////////////////////////////
//
// RollImage::analyzeLeaders --
//

void RollImage::analyzeLeaders(void) {
	if (!m_analyzedBasicMargins) {
		analyzeBasicMargins();
	}
	ulongint cols = getCols();
	ulongint rows = getRows();

//cerr << "LEFT MARGIN INDEX " << endl;
//for (int zz=0; zz<(int)leftMarginIndex.size(); zz++) {
//cerr << "\t" << leftMarginIndex[zz] << endl;
//}
//cerr << endl;

	// Analyze the average margin width in a square region 4096x4096 at
	// each corner of the image:
	double topLeftAvg  = getAverage(leftMarginIndex, 0, cols);
	double topRightAvg = getAverage(rightMarginIndex, 0, cols);
	double botLeftAvg  = getAverage(leftMarginIndex, rows-1-cols, cols);
	double botRightAvg = getAverage(rightMarginIndex, rows-1-cols, cols);

	// std::cerr << "topLeftAvg = "  << topLeftAvg << std::endl;
	// std::cerr << "topRightAvg = " << topRightAvg << std::endl;
	// std::cerr << "botLeftAvg = "  << botLeftAvg << std::endl;
	// std::cerr << "botRightAvg = " << botRightAvg << std::endl;

	if ((topLeftAvg > botLeftAvg) && (topRightAvg < botRightAvg)) {
		// do nothing, everything is as expected
	} else if ((topLeftAvg < botLeftAvg) && (topRightAvg > botRightAvg)) {
		// leader is on the bottom of the image, so don't continue processing
		// eventually, perhaps reverse processing.
		std::cerr << "Cannot deal with bottom leader" << std::endl;
		exit(1);
	} else {
		std::cerr << "Cannot find leader (deal with partial rolls later)." << std::endl;
		std::cerr << "TOP LEFT SHOULD BE GREATER THAN BOTTOM LEFT:" << std::endl;
		std::cerr << "   TOP    LEFT  AVERAGE " << topLeftAvg << std::endl;
		std::cerr << "   BOTTOM LEFT  AVERAGE " << botLeftAvg << std::endl;
		std::cerr << "TOP RIGHT SHOULD BE GREATER THAN BOTTOM RIGHT:" << std::endl;
		std::cerr << "TOP    RIGHT AVERAGE " << topRightAvg << std::endl;
		std::cerr << "BOTTOM RIGHT AVERAGE " << botRightAvg << std::endl;
		exit(1);
	}

	ulongint leftLeaderBoundary = 0;
	leftLeaderBoundary = findLeftLeaderBoundary(leftMarginIndex, botLeftAvg, cols, 4096*4);

	ulongint rightLeaderBoundary = 0;
	rightLeaderBoundary = findRightLeaderBoundary(rightMarginIndex, botRightAvg, cols, 4096*4);

	ulongint leaderBoundary = (leftLeaderBoundary + rightLeaderBoundary) / 2;
	setLeaderIndex(leaderBoundary);
	markLeaderRegion();

	// find pre-leader region
	setPreleaderIndex(extractPreleaderIndex(leaderBoundary));
	markPreleaderRegion();

	m_analyzedLeaders = true;
}



//////////////////////////////
//
// findLeftLeaderBoundary --
//

ulongint RollImage::findLeftLeaderBoundary(std::vector<int>& margin, double avg, ulongint cols,
		ulongint searchlength) {
	std::vector<int> status(searchlength, 0);
	int cutoff = int(avg * 1.05 + 0.5);
	for (ulongint i=0; (i<status.size()) && (i<margin.size()); i++) {
		if (margin[i] > cutoff) {
			status[i] = 1;
		}
	}
	ulongint boundary = getBoundary(status);
	return boundary;
}



//////////////////////////////
//
// findRightLeaderBoundary -- Boundary between the leader and the rest of the
//   roll as a row index number.
//

ulongint RollImage::findRightLeaderBoundary(std::vector<int>& margin, double avg,
		ulongint cols, ulongint searchlength) {
	std::vector<int> status(searchlength, 0);
	int cutoff = int(avg / 1.05 + 0.5);
	for (ulongint i=0; (i<status.size()) && (i<margin.size()); i++) {
		if (margin.at(i) < cutoff) {
			status[i] = 1;
		}
	}
	ulongint boundary = getBoundary(status);
	return boundary;
}



/////////////////////////////
//
// getBoundary -- Used by findRightLeaderBoundary and findLeftLeaderBondary.
//

ulongint RollImage::getBoundary(std::vector<int>& status) {
	ulongint windowsize = 100;
	ulongint above = 0;
	ulongint below = 0;
	for (ulongint i=0; (i<windowsize) && (i<status.size()); i++) {
		above += status.at(i);
	}
	for (ulongint i=windowsize; (i<windowsize*2) && (i<status.size()); i++) {
		below += status.at(i);
	}

	for (ulongint i=windowsize+1; i<status.size()-1-windowsize; i++) {
		above += status[i];
		above -= status[i-windowsize-1];
		below += status[i+windowsize];
		below -= status[i-1];
		if ((above > 90) && (below < 10)) {
			return i;
		}
		if ((above < 10) && (below > 90)) {
			return i;
		}
	}

	// make a guess at six inches if not found
	return 6 * 300;

	std::cerr << "COULD NOT FIND LEADER BOUNDARY" << std::endl;
	std::cerr << "SEARCH LENGTH:\t" << status.size() << std::endl;
	std::cerr << "WINDOW SIZE:\t" << windowsize*2+1 << endl;
	exit(1);
	return 0;
}



//////////////////////////////
//
// RollImage::mergePixelOverlay --
//

void RollImage::mergePixelOverlay(std::fstream& output) {

	std::vector<ucharint> pixel(3);
	ulonglongint offset;
	ulongint rows = getRows();
	ulongint cols = getCols();

	for (ulongint r=0; r<rows; r++) {
		for (ulongint c=0; c<cols; c++) {
			int value = pixelType[r][c];
			if (!value) {
				continue;
			}
			switch (value) {
				case PIX_NONPAPER:         // undifferentiated non-paper (green)
					pixel[0] = 0;
					pixel[1] = 255;
					pixel[2] = 0;
					break;

				case PIX_MARGIN:           // paper margins (blue)
					pixel[0] = 0;
					pixel[1] = 0;
					pixel[2] = 255;
					break;

				case PIX_HARDMARGIN:       // paper margins with not paper in rect.
					pixel[0] = 0;
					pixel[1] = 64;
					pixel[2] = 255;
					break;

				case PIX_LEADER:           // leader region (cyan)
					pixel[0] = 0;
					pixel[1] = 255;
					pixel[2] = 255;
					break;

				case PIX_PRELEADER:        // pre-leader region (light blue)
					pixel[0] = 0;
					pixel[1] = 128;
					pixel[2] = 255;
					break;

				case PIX_POSTLEADER:       // post-leader region (lighter blue)
					pixel[0] = 128;
					pixel[1] = 128;
					pixel[2] = 255;
					break;

				case PIX_POSTMUSIC:       // post-music region (lighter blue)
					pixel[0] = 128;
					pixel[1] = 128;
					pixel[2] = 255;
					break;

				case PIX_TEAR:             // tears at edge of roll (magenta)
					pixel[0] = 255;
					pixel[1] = 0;
					pixel[2] = 255;
					break;

				case PIX_ANTIDUST:         // non-musical holes in roll (light-magenta)
					pixel[0] = 255;
					pixel[1] = 128;
					pixel[2] = 255;
					break;

				case PIX_HOLE:             // musical holes in roll (cornflowerblue)
					pixel[0] = 100;
					pixel[1] = 149;
					pixel[2] = 237;
					break;

				case PIX_HOLE_SNAKEBITE:   // snake bites (red)
					pixel[0] = 255;
					pixel[1] =   0;
					pixel[2] =   0;
					break;

				case PIX_HOLE_SHIFT:       // musical holes in roll (lightblue)
					pixel[0] = 173;
					pixel[1] = 216;
					pixel[2] = 230;
					break;

				case PIX_BADHOLE:          // non-musical hole but significant (magenta)
					pixel[0] = 255;
					pixel[1] = 0;
					pixel[2] = 255;
					break;

				case PIX_BADHOLE_SKEWED:   // non-musical hole which is skewed (deep pink)
					pixel[0] = 255;
					pixel[1] =  20;
					pixel[2] = 147;
					break;

				case PIX_BADHOLE_ASPECT:   // non-musical hole which has a bad aspect ratio (springgreen)
					pixel[0] = 0;
					pixel[1] = 255;
					pixel[2] = 127;
					break;

				case PIX_HOLEBB:           // musical hole bounding box (red)
					pixel[0] = 255;
					pixel[1] =   0;
					pixel[2] =   0;
					break;

				case PIX_HOLEBB_LEADING_A:   // musical hole attack edge (yellow)
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] =   0;
					break;

				case PIX_HOLEBB_LEADING_S:   // musical hole leading edge, sustain  (orange)
					pixel[0] = 255;
					pixel[1] = 165;
					pixel[2] =   0;
					break;

				case PIX_HOLEBB_TRAILING:  // musical hole bounding box (red)
					pixel[0] = 255;
					pixel[1] =   0;
					pixel[2] =   0;
					break;

				case PIX_HOLEBB_BASS:      // musical hole bounding box (orange)
					pixel[0] = 255;
					pixel[1] = 165;
					pixel[2] =   0;
					break;

				case PIX_HOLEBB_TREBLE:    // musical hole bounding box (red)
					pixel[0] = 255;
					pixel[1] =   0;
					pixel[2] =   0;
					break;

				case PIX_TRACKER:           // hole for tracker position (green)
					pixel[0] =   0;
					pixel[1] = 255;
					pixel[2] =   0;
					break;

				case PIX_TRACKER_BASS:      // hole for bass tracker position (green)
					pixel[0] =   0;
					pixel[1] = 255;
					pixel[2] =   0;
					break;

				case PIX_TRACKER_TREBLE:    // hole for treble tracker position (cyan)
					pixel[0] =   0;
					pixel[1] = 255;
					pixel[2] = 255;
					break;

				case PIX_DEBUG:				 // white
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] = 255;
					break;

				case PIX_DEBUG1:				// red
					pixel[0] = 255;
					pixel[1] = 0;
					pixel[2] = 0;
					break;

				case PIX_DEBUG2:				// orange
					pixel[0] = 255;
					pixel[1] = 153;
					pixel[2] = 127;
					break;

				case PIX_DEBUG3:				// yellow
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] = 0;
					break;

				case PIX_DEBUG4:				// green
					pixel[0] = 50;
					pixel[1] = 255;
					pixel[2] = 50;
					break;

				case PIX_DEBUG5:				// light blue
					pixel[0] = 0;
					pixel[1] = 255;
					pixel[2] = 255;
					break;

				case PIX_DEBUG6:				// dark blue
					pixel[0] = 0;
					pixel[1] = 0;
					pixel[2] = 255;
					break;

				case PIX_DEBUG7:				// purple
					pixel[0] = 150;
					pixel[1] = 50;
					pixel[2] = 255;
					break;

				default:
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] = 255;

			}
			offset = this->getPixelOffset(r, c);
			prp::goToByteIndex(output, offset);
			output.write((char*)pixel.data(), 3);
		}
	}
}



//////////////////////////////
//
// RollImage::extractPreleaderIndex -- Return the row in the image where the preleader
//    ends and the leader starts.
//

ulongint RollImage::extractPreleaderIndex(ulongint leaderBoundary) {

	std::vector<ulongint> marginsum;
	ulongint tolerance = 20;       // How much the positions adjacent to the min
	                            // can grow, yet still be called the preleader.
	ulongint startboundary = 10;   // Avoid the very start of the image
	                            // in case there are scanning artifacts.
	ulongint cols = getCols();

	marginsum.resize(leaderBoundary);
	std::fill(marginsum.begin(), marginsum.end(), 0);
	for (ulongint i=startboundary; i<marginsum.size(); i++) {
		marginsum[i] = leftMarginIndex[i] + cols - rightMarginIndex[i];
	}
	ulongint position = maxValueIndex(marginsum);

	ulongint posadj = position;
	while (posadj < leaderBoundary) {
		if (marginsum[posadj] > marginsum[position] - tolerance) {
			posadj++;
			continue;
		} else {
			break;
		}
	}
	return posadj;
}




//////////////////////////////
//
// RollImage:analyzeHardMargins -- Mark the rectangular area in the margin where no paper is
//     found.  Dust is expected to be already filtered out of the margin data.
//

void RollImage::analyzeHardMargins(ulongint leaderBoundary) {

	ulongint endboundary = 1000;

	ulongint minpos = leftMarginIndex[leaderBoundary];
	ulongint rows = pixelType.size();
	for (ulongint r=leaderBoundary+1; r<rows-endboundary; r++) {
		if ((ulongint)leftMarginIndex[r] < minpos) {
			minpos = leftMarginIndex[r];
			//minrow = r;
		}
	}
	setHardMarginLeftIndex(minpos);

	for (ulongint r=leaderBoundary; r<rows; r++) {
		for (ulongint c=0; c<=minpos; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_HARDMARGIN;
			}
		}
	}

	ulongint maxpos = rightMarginIndex[leaderBoundary];
	for (ulongint r=leaderBoundary+1; r<rows-endboundary; r++) {
		if ((ulongint)rightMarginIndex[r] > maxpos) {
			maxpos = rightMarginIndex[r];
		}
	}
	setHardMarginRightIndex(maxpos);

	for (ulongint r=leaderBoundary; r<rows; r++) {
		ulongint cols = pixelType[r].size();
		for (ulongint c=maxpos; c<cols; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_HARDMARGIN;
			}
		}
	}

}



//////////////////////////////
//
// RollImage::setHardMarginLeftIndex --
//

void RollImage::setHardMarginLeftIndex(ulongint index) {
	hardMarginLeftIndex = index;
}



//////////////////////////////
//
// RollImage::setHardMarginRightIndex --
//

void RollImage::setHardMarginRightIndex(ulongint index) {
	hardMarginRightIndex = index;
}



//////////////////////////////
//
// RollImage::markPreleaderRegion --
//

void RollImage::markPreleaderRegion(void) {
	ulongint cols = getCols();

	// mark holes in leader region as leader holes.

	for (ulongint r=0; r<=preleaderIndex; r++) {
		for (ulongint c=0; c<cols; c++) {
			if (pixelType[r][c]) {
				pixelType[r][c] = PIX_PRELEADER;
			}
		}
	}

}



//////////////////////////////
//
// RollImage::markLeaderRegion --
//

void RollImage::markLeaderRegion(void) {
	ulongint cols = getCols();

	// mark holes in leader region as leader holes.

	for (ulongint r=0; r<leaderIndex; r++) {
		for (ulongint c=0; c<cols; c++) {
			if (pixelType[r][c]) {
				pixelType[r][c] = PIX_LEADER;
			}
		}
	}
}



//////////////////////////////
//
// RollImage::getHardMarginLeft -- return the left hard margin (the region in the
//    image on the left side where there is no roll content.  Dust is ignored and
//    can be included in hard margin.
//

int RollImage::getHardMarginLeftWidth(void) {
	if (!m_analyzedAdvancedMargins) {
		analyzeAdvancedMargins();
	}
	return hardMarginLeftIndex;
}



//////////////////////////////
//
// RollImage::getHardMarginRight -- return the right hard margin (the region in the
//    image on the right side where there is no roll content.  Dust is ignored and
//    can be included in hard margin.
//

int RollImage::getHardMarginRightWidth(void) {
	if (!m_analyzedAdvancedMargins) {
		analyzeAdvancedMargins();
	}
	return getCols() - hardMarginRightIndex;
}



//////////////////////////////
//
// RollImage::setPreleaderIndex -- Set the row index that is the boundary
//    between the preleader and the leader.
//

void RollImage::setPreleaderIndex(ulongint value) {
	preleaderIndex = value;
}



//////////////////////////////
//
// RollImage::setLeaderIndex -- Set the row index that is the boundary
//    between the preleader and the leader.
//

void RollImage::setLeaderIndex(ulongint value) {
	leaderIndex = value;
}



//////////////////////////////
//
// RollImage::getPreleaderIndex -- return the row index that is the boundary
//    between the preleader and the leader.
//

ulongint RollImage::getPreleaderIndex(void) {
	if (!m_analyzedLeaders) {
		analyzeLeaders();
	}
	return preleaderIndex;
}



//////////////////////////////
//
// RollImage::getLeaderIndex -- return the row index that is the boundary
//    between the leader and the main content of the roll.
//

ulongint RollImage::getLeaderIndex(void) {
	if (!m_analyzedLeaders) {
		analyzeLeaders();
	}
	return leaderIndex;
}



//////////////////////////////
//
// RollImage::getHardMarginLeftIndex --
//

int RollImage::getHardMarginLeftIndex(void) {
	return hardMarginLeftIndex;
}



//////////////////////////////
//
// RollImage::getHardMarginRightIndex --
//

int RollImage::getHardMarginRightIndex(void) {
	return hardMarginRightIndex;
}




//////////////////////////////
//
// RollImage::getFirstMusicHoleStart --
//

ulongint RollImage::getFirstMusicHoleStart(void) {
	return firstMusicRow;
}



//////////////////////////////
//
// RollImage::recalculateFirstMusicHole --
//

void RollImage::recalculateFirstMusicHole(void) {
	ulongint minrow = getRows() - 1;
	std::vector<std::vector<HoleInfo*> >& ta = trackerArray;
	ulongint row;
	for (ulongint i=0; i<ta.size(); i++) {
		for (ulongint j=0; j<ta[i].size(); j++) {
			if (!ta[i][j]->isMusicHole()) {
				continue;
			}
			row = ta[i][j]->origin.first;
			if (row < minrow) {
				minrow = row;
			}
		}
	}
	if (minrow > firstMusicRow) {
		firstMusicRow = minrow;
		markPosteriorLeader();
		removeBadLeaderHoles();
	}
}



//////////////////////////////
//
// RollImage::removeBadLeaderHoles -- After moving the first music hole position
//    further along in the roll, review the bad holes and remove any holes which are
//    before the new first music hole position.
//

void RollImage::removeBadLeaderHoles(void) {
	ulongint limit = firstMusicRow;
	std::vector<ulongint> listing;
	for (ulongint i=0; i<badHoles.size(); i++) {
		if (badHoles[i]->origin.first < limit) {
			listing.push_back(i);
		}
	}

	if (listing.empty()) {
		// nothing to do
		return;
	}

	ulongint start = listing.size() - 1;
	for (int i=(int)start; i >= 0; i--) {
		badHoles.erase(badHoles.begin() + listing.back());
		listing.pop_back();
	}
}



//////////////////////////////
//
// RollImage::getLastMusicHoleStartEnd --
//

ulongint RollImage::getLastMusicHoleEnd(void) {
	return lastMusicRow;
}



//////////////////////////////
//
// RollImage::markHoleShifts --
//

void RollImage::markHoleShifts(void) {
	int counter;
	for (ulongint i=0; i<holes.size(); i++) {
		if (!holes[i]->isMusicHole()) {
			continue;
		}

		if (!holes[i]->isShifting()) {
			continue;
		}

		//if (fabs(holes[i]->leadinghcor - holes[i]->trailinghcor) < getHoleShiftCutoff()) {
		//	continue;
		//}
		// Hole shifts too much to mark it with a different color from
		// regular holes.
		counter = 0;
		ulongint r = holes[i]->entry.first;
		ulongint c = holes[i]->entry.second;
		int target = pixelType[r][c];
		fillHoleSimple(r, c, target, PIX_HOLE_SHIFT, counter);
	}
}



//////////////////////////////
//
// RollImage::markSnakeBites --
//

void RollImage::markSnakeBites(void) {
	int counter;
	for (ulongint i=0; i<holes.size(); i++) {
		if (!holes[i]->isMusicHole()) {
			continue;
		}

		if (!holes[i]->snakebite) {
			continue;
		}

		counter = 0;
		ulongint r = holes[i]->entry.first;
		ulongint c = holes[i]->entry.second;
		int target = pixelType[r][c];
		fillHoleSimple(r, c, target, PIX_HOLE_SNAKEBITE, counter);
	}
}



//////////////////////////////
//
// RollImage::markHoleBBs --
//

void RollImage::markHoleBBs(void) {
	for (ulongint i=0; i<holes.size(); i++) {
		if (holes[i]->isMusicHole()) {
			markHoleBB(*holes[i]);
		}
	}
}



//////////////////////////////
//
// RollImage::markHoleBB --
//

void RollImage::markHoleBB(HoleInfo& hi) {
// cerr << "MARKING HOLE " << hi << endl;
	long r;
	long c;

	// Mark upper side of box (leading edge)
	r = hi.origin.first - 1;
	if (hi.attack) {
		for (c=-1; c<(long)hi.width.second+1; c++) {
			pixelType.at(r).at(c + (long)hi.origin.second) = PIX_HOLEBB_LEADING_A;
		}
	} else {
		for (c=-1; c<(long)hi.width.second+1; c++) {
			pixelType.at(r).at(c + (long)hi.origin.second) = PIX_HOLEBB_LEADING_S;
		}
	}

	// Mark lower side of box (trailing edge):
	r = hi.origin.first + hi.width.first + 1;
	if (r < (long)getRows()) {
		for (c=-1; c<(long)hi.width.second+1; c++) {
			pixelType.at(r).at(c + (long)hi.origin.second) = PIX_HOLEBB_TRAILING;
		}
	}

	// Mark left side of box:
	c = hi.origin.second - 1;
	if (c >= 0) {
		for (r=-1; r<(long)hi.width.first+1; r++) {
			pixelType.at(r + (long)hi.origin.first).at(c) = PIX_HOLEBB_BASS;
		}
	}

	// Mark right side of box:
	c = hi.origin.second + hi.width.second + 1;
	if (c < (long)getCols()) {
		for (r=-1; r<=(long)hi.width.first+1; r++) {
			pixelType.at(r + (long)hi.origin.first).at(c) = PIX_HOLEBB_TREBLE;
		}
	}
}



//////////////////////////////
//
// RollImage::markHoleAttacks --
//

void RollImage::markHoleAttacks(void) {
	for (ulongint i=0; i<holes.size(); i++) {
		if (holes[i]->isMusicHole()) {
			if (holes[i]->attack) {
				markHoleAttack(*holes[i]);
			}
		}
	}
}



//////////////////////////////
//
// RollImage::markHoleAttack --
//

void RollImage::markHoleAttack(HoleInfo& hi) {
	long r, c;

	// Mark upper side of box (leading edge)
	r = hi.origin.first - 1;
	long cols = (int)getCols();
	// long offset = (long)hi.origin.second;

	for (c=0; c<cols; c++) {
		if (pixelType[r][c] != PIX_PAPER) {
			continue;
		}
		if (c % getAttackLineSpacing() == 0) {
			pixelType[r][c] = PIX_HOLEBB_LEADING_A;
		}
	}
}



//////////////////////////////
//
// RollImage::getSoftMarginLeftWidth -- return the distance from the
//   hard margin to the left roll edge.
//

int RollImage::getSoftMarginLeftWidth(ulongint rowindex) {
	if (leftMarginIndex[rowindex] > getHardMarginLeftIndex()) {
		return (ulongint)leftMarginIndex[rowindex] - getHardMarginLeftIndex();
	} else {
		return 0;
	}
}



//////////////////////////////
//
// RollImage::getSoftMarginRightWidth -- return the distance from the
//   hard margin to the left roll edge.
//

int RollImage::getSoftMarginRightWidth(ulongint rowindex) {
	if (rightMarginIndex[rowindex] < getHardMarginRightIndex()) {
		return getHardMarginRightIndex() - (ulongint)rightMarginIndex[rowindex];
	} else {
		return 0;
	}
}



//////////////////////////////
//
// RollImage::getSoftMarginLeftWidthMax --
//

int RollImage::getSoftMarginLeftWidthMax(void) {
	int max = 0;
	ulongint startrow = getFirstMusicHoleStart();
	ulongint endrow = getLastMusicHoleEnd();
	for (ulongint r=startrow; r<=endrow; r++) {
		int value = leftMarginIndex[r] - getHardMarginLeftIndex();
		if (value > max) {
			max = value;
		}
	}
	return max;
}



//////////////////////////////
//
// RollImage::getSoftMarginRightWidthMax --
//

int RollImage::getSoftMarginRightWidthMax(void) {
	ulongint max = 0;
	ulongint startrow = getFirstMusicHoleStart();
	ulongint endrow = getLastMusicHoleEnd();
	for (ulongint r=startrow; r<=endrow; r++) {
		ulongint value = getHardMarginRightIndex() - (ulongint)rightMarginIndex[r];
		if (value > max) {
			max = value;
		}
	}
	return max;
}



//////////////////////////////
//
// RollImage::getAverageRollWidth --  Only looks in the music region.
//

double RollImage::getAverageRollWidth(void) {
	if (averageRollWidth > 0.0) {
		return averageRollWidth;
	}

	// otherwise, calculate the width:
	ulongint startrow = getFirstMusicHoleStart();
	ulongint endrow = getLastMusicHoleEnd();
	ulongint count = endrow - startrow + 1;

	// int cols = getCols();
	double sum = 0.0;
	for (ulongint r=startrow; r<=endrow; r++) {
		double value = rightMarginIndex.at(r) - leftMarginIndex.at(r);
		sum += value;
	}
	averageRollWidth = sum / count;
	return averageRollWidth;
}



//////////////////////////////
//
// RollImage::getLeftMarginWidth --
//

ulongint RollImage::getLeftMarginWidth(ulongint rowindex) {
	return leftMarginIndex[rowindex];
}



//////////////////////////////
//
// RollImage::getRightMarginWidth --
//

ulongint RollImage::getRightMarginWidth(ulongint rowindex) {
	return getCols() - rightMarginIndex[rowindex];
}



//////////////////////////////
//
// RollImage::getAverageMusicalHoleWidth --
//

double RollImage::getAverageMusicalHoleWidth(void) {
	if (m_averageHoleWidth != -1.0) {
		return m_averageHoleWidth;
	}
	if (holes.size() == 0) {
		return 0.0;
	}
	double sum = 0.0;
	for (ulongint i=0; i<holes.size(); i++) {
		sum += holes[i]->width.second;
	}
	m_averageHoleWidth = sum / holes.size();
	return m_averageHoleWidth;
}



//////////////////////////////
//
// RollImage::getAverageSoftMarginTotal -- Only checking within musical region.
//

double RollImage::getAverageSoftMarginTotal(void) {
	ulongint startrow = getFirstMusicHoleStart();
	ulongint endrow = getLastMusicHoleEnd();
	ulongint count = endrow - startrow + 1;

	double sum = 0.0;
	for (ulongint r=startrow; r<=endrow; r++) {
		if ((int)getHardMarginLeftWidth() < (int)getLeftMarginWidth(r)) {
			sum += getLeftMarginWidth(r) - getHardMarginLeftWidth();
		}
		if ((int)getHardMarginRightWidth() < (int)getRightMarginWidth(r)) {
			sum += getRightMarginWidth(r) - getHardMarginRightWidth();
		}
	}
	return sum / count;
}



//////////////////////////////
//
// RollImage::markTrackerPositions --
//

void RollImage::markTrackerPositions(bool showAll) {

	int midiStart = midiToTrackMapping.size() - 1;
	for (int i=0; i<(int)midiToTrackMapping.size(); i++) {
		if (midiToTrackMapping.at(i)) {
			midiStart = i;
			break;
		}
	}

	int midiEnd = 0;
	for (int i=midiToTrackMapping.size()-1; i>=0; i--) {
		if (midiToTrackMapping.at(i)) {
			midiEnd = (ulongint)i;
			break;
		}
	}

	int realcolstart = midiToTrackMapping.at(midiStart);
	int realcolend   = midiToTrackMapping.at(midiEnd);

	int colstart = 0;
	int colend = midiToTrackMapping.size() - 1;

	if (!showAll) {
		colstart = realcolstart;
		colend   = realcolend;
	}

	if (colend >= (int)trackerArray.size()) {
		colend = (int)trackerArray.size() - 1;
	}

	ulongint startrow = getFirstMusicHoleStart() - 100;
	ulongint endrow   = getLastMusicHoleEnd()    + 100;
	if (endrow >= getRows()) {
		endrow = getRows() - 1;
	}

	// int cutoff = midiToTrackMapping.at(65);
	// temporary fix for red Welte: Lowest pitch of treble register (E4)
	int cutoff = midiToTrackMapping.at(64);

	int c;
	int color;
	int cols = getCols();
	for (ulongint r=startrow; r<=endrow; r++) {
		for (int i=colstart; i<=colend; i++) {
			// Reverse drift correction to find horizontal position in image:
			c = int(m_normalizedPosition[i] - driftCorrection[r] + 0.5);
			if (c <= holeOffset) {
				continue;
			}
			if (c >= cols) {
				continue;
			}
			if (i < cutoff) {
				color = PIX_TRACKER_BASS;
			} else {
				color = PIX_TRACKER_TREBLE;
			}
			if (i < realcolstart + 10) {
				// expression tracks on the left (for red rolls only: need to generalize)
				color = PIX_DEBUG7;
			}
			if (i > realcolend-10) {
				// expression tracks on the right (for red rolls only: need to generalize)
				color = PIX_DEBUG7;
			}
			if ((i < realcolstart) || (i > realcolend)) {
				color = PIX_DEBUG2;
				if (i == realcolstart) {
					color = PIX_DEBUG7;
				}
				if (i == realcolend) {
					color = PIX_DEBUG7;
				}
				if (r % 20 == 0) {
					// dotted line to indiate off-paper track position
					pixelType[r][c] = color;
				}
			} else if (!trackerArray.at(i).empty()) {
				if (trackMeaning.at(i) == TRACK_SNAKEBITE) {
					pixelType.at(r).at(c) = PIX_HOLE_SNAKEBITE;
				} else {
					pixelType.at(r).at(c) = color;
				}
			} else if (r % 20 < 10) {
				// dashed line to indiate no activity in track
				pixelType.at(r).at(c) = color;
			}
		}
	}
}



//////////////////////////////
//
// RollImage::markShifts --
//

void RollImage::markShifts(void) {
	for (ulongint i=0; i<shifts.size(); i++) {
		markShift(i);
	}
}



//////////////////////////////
//
// RollImage::markShift -- Orange = shift left, Red = shift right
//

void RollImage::markShift(int index) {
	ShiftInfo* si = shifts[index];
	ulongint row = si->row;
	double score = si->score;
	int cols = getCols();
	for (int c=0; c<cols; c++) {
		if (score > 0) {
			if (pixelType[row][c] == PIX_PAPER) {
				pixelType[row][c] = PIX_DEBUG1;
			}
		} else {
			if (pixelType[row][c] == PIX_PAPER) {
				pixelType[row][c] = PIX_DEBUG2;
			}
		}
	}
}



//////////////////////////////
//
// RollImage::drawMajorAxes --
//

void RollImage::drawMajorAxes(void) {
	for (ulongint i=0; i<holes.size(); i++) {
		drawMajorAxis(*holes[i]);
	}
}



//////////////////////////////
//
// RollImage::drawMajorAxis --
//

void RollImage::drawMajorAxis(HoleInfo& hi) {
	ulongint startrow = hi.origin.first;
	ulongint endrow   = hi.origin.first + hi.width.first;
	double centerr = hi.centroid.first;
	double centerc = hi.centroid.second;
	double angle = hi.majoraxis * M_PI / 180.0;
	double cosangle = cos(angle);
	double side1, side2;
	double hypot;
	ulongint c;
	for (ulongint r=startrow; r<=endrow; r++) {
		side1 = r - centerr;
		hypot = side1 / cosangle;
		side2 = sqrt(hypot*hypot - side1*side1);
		if (r > centerr) {
			c = centerc + side2;
		} else {
			c = centerc - side2;
		}
		pixelType[r][c] = PIX_DEBUG7;
	}
	pixelType[centerr][centerc] = PIX_DEBUG2;
}



//////////////////////////////
//
// RollImage::getDustScore --
//

double RollImage::getDustScore(void) {
	if (m_dustscore >= 0.0) {
		return m_dustscore;
	}

	double bass = getDustScoreBass();
	double treble = getDustScoreTreble();
	m_dustscore = (bass + treble) / 2.0;
	return int(m_dustscore+ 0.5);
}



//////////////////////////////
//
// RollImage::getDustScoreBass -- Probably do not need to count all of the
//   dust the entire roll, since the dust particles will cycle through the
//   scanning region every 6.47 inches. Reported value is in parts per million
//   (ppm).
//

double RollImage::getDustScoreBass(void) {
	if (m_dustscorebass >= 0.0) {
		return m_dustscorebass;
	}

	ulongint counter = 0;
	ulongint startcol = 0;
	ulongint endcol = hardMarginLeftIndex;
	ulongint startrow = getFirstMusicHoleStart();
	ulongint endrow   = getLastMusicHoleEnd();

	for (ulongint r=startrow; r<=endrow; r++) {
		for (ulongint c=startcol; c<=endcol; c++) {
			if (pixelType[r][c] == PIX_PAPER) {
				counter++;
			} else if (pixelType[r][c] == PIX_NONPAPER) {
				// this is technically not paper, but there should
				// be no PIX_NONPAPER in hard margin region.  When
				// there is, that means there is a lot of dust around,
				// so treat the dust-shadowed region as dust itself.
				counter++;
			}
		}
	}

	double marginarea = (endcol - startcol + 1) * (endrow - startrow + 1);
	m_dustscorebass = (double)counter / marginarea * 1000000.0;
	return int(m_dustscorebass + 0.5);
}



//////////////////////////////
//
// RollImage::getDustScoreTreble --
//

double RollImage::getDustScoreTreble(void) {
	if (m_dustscoretreble >= 0.0) {
		return m_dustscoretreble;
	}

	ulongint counter = 0;
	ulongint startcol = hardMarginRightIndex;
	ulongint endcol = getCols() - 1;
	ulongint startrow = getFirstMusicHoleStart();
	ulongint endrow   = getLastMusicHoleEnd();

	for (ulongint r=startrow; r<=endrow; r++) {
		for (ulongint c=startcol; c<=endcol; c++) {
			if (pixelType[r][c] == PIX_PAPER) {
				counter++;
			} else if (pixelType[r][c] == PIX_NONPAPER) {
				// this is technically not paper, but there should
				// be no PIX_NONPAPER in hard margin region.  When
				// there is, that means there is a lot of dust around,
				// so treat the dust-shadowed region as dust itself.
				counter++;
			}
		}
	}

	double marginarea = (endcol - startcol + 1) * (endrow - startrow + 1);
	m_dustscoretreble = (double)counter / marginarea * 1000000.0;
	return int(m_dustscoretreble + 0.5);
}



//////////////////////////////
//
// RollImage::sortBadHolesByArea -- Sort holes by area from largest to smallest.
//

void RollImage::sortBadHolesByArea(void) {

#ifndef DONOTUSEFFT
	if (badHoles.size() <= 1) {
		// nothing to do.
		return;
	}

	std::sort(badHoles.begin(), badHoles.end(),
		[](HoleInfo* a, HoleInfo* b) -> bool
			{
				return a->area > b->area;
			}
		);
#endif
}





/////////////////////////////
//
// RollImage::sortShiftsByAmount --
//

void RollImage::sortShiftsByAmount(void) {
#ifndef DONOTUSEFFT
	if (shifts.size() <= 1) {
		// nothing to do
		return;
	}

	std::sort(shifts.begin(), shifts.end(),
		[](ShiftInfo* a, ShiftInfo* b) -> bool
			{
				return fabs(a->score) > fabs(b->score);
			}
		);
#endif
}




//////////////////////////////
//
// RollImage::sortTearsByArea -- Sort tears by area from largest to smallest.
//

void RollImage::sortTearsByArea(void) {
#ifndef DONOTUSEFFT
	if (bassTears.size() >= 1) {
		std::sort(bassTears.begin(), bassTears.end(),
			[](TearInfo* a, TearInfo* b) -> bool
				{
					return a->area > b->area;
				}
			);
	}
	if (trebleTears.size() >= 1) {
		std::sort(trebleTears.begin(), trebleTears.end(),
			[](TearInfo* a, TearInfo* b) -> bool
				{
					return a->area > b->area;
				}
			);
	}
#endif
}



//////////////////////////////
//
// RollImage::getTrackerHoleCount -- Simple algorithm for now
//

int RollImage::getTrackerHoleCount(void) {
	int counter = 0;
	for (ulongint i=0; i<midiToTrackMapping.size(); i++) {
		if (midiToTrackMapping[i]) {
			counter++;
		}
	}
	return counter;
}



//////////////////////////////
//
// RollImage::addDriftInfoToHoles --
//

void RollImage::addDriftInfoToHoles(void) {
	for (ulongint i=0; i<holes.size(); i++) {
		holes[i]->leadinghcor = driftCorrection[holes[i]->origin.first];
		holes[i]->trailinghcor = driftCorrection[holes[i]->origin.first+holes[i]->width.first];
	}
}



//////////////////////////////
//
// RollImage::getDataMD5Sum --
//

std::string RollImage::getDataMD5Sum(void) {
	CheckSum checksum;
	return checksum.getMD5Sum(monochrome);
}



//////////////////////////////
//
// RollImage::generateNoteMidiFileHex -- Generate MIDI file where holes are grouped into notes.
//

void RollImage::generateNoteMidiFileHex(ostream& output) {
#ifndef DONOTUSEFFT
	MidiFile midifile;
	generateMidifile(midifile);
	midifile.writeHex(output, 25);
#endif
}



//////////////////////////////
//
// RollImage::generateNoteMidiFileBinasc -- Generate MIDI file where holes are grouped into notes.
//

void RollImage::generateNoteMidiFileBinasc(ostream& output) {
#ifndef DONOTUSEFFT
	MidiFile midifile;
	generateMidifile(midifile);
	midifile.writeBinasc(output);
#endif
}


//////////////////////////////
//
// RollImage::generateMidifile --
//


#ifndef DONOTUSEFFT
void RollImage::generateMidifile(MidiFile& midifile) {

	if (holes.empty()) {
		return;
	}

	assignMidiKeyNumbersToHoles();

	midifile.setTPQ(510);

	// tempo 30 = 180
	// tempo 35 = 210
	// tempo 40 = 240
	// tempo 45 = 270
	// tempo 50 = 300
	// tempo 55 = 330
	// tempo 60 = 360
	// tempo 65 = 390
	// tempo 70 = 420
	// tempo 75 = 450
	// tempo 80 = 480
	// tempo 85 = 510
	// tempo 90 = 540
	// tempo 95 = 570
	// tempo 100 = 600
	// tempo 105 = 630
	// tempo 110 = 660
	// tempo 115 = 690
	// tempo 120 = 720
	// tempo 125 = 750
	// tempo 130 = 780
	// tempo 135 = 810
	// tempo 140 = 840

	midifile.addTracks(4);
	// track 1 = bass notes
	// track 2 = treble notes
	// track 3 = bass expression
	// track 4 = treble expression

	midifile.addController(3, 0, 3, 7, 0); // mute channel 3 notes (they are bass expression holes)
	midifile.addController(4, 0, 4, 7, 0); // mute channel 4 notes (they are treble expression holes)

	midifile.addController(1, 0, 1, 10, 32); // bass notes pan leftish
	midifile.addController(2, 0, 2, 10, 96); // treble notes pan rightish

	ulongint mintime = holes[0]->origin.first;
	ulongint maxtime = 0;

	int track;
	int key;
	int channel;
	int velocity;
	for (ulongint i=0; i<holes.size(); i++) {
		if (!holes[i]->attack) {
			continue;
		}
		HoleInfo* hi = holes[i];
		key = hi->midikey;

		// estimate the location of the expression tracks and the
		// bass/treble register split (refine later):
		if (key < 24) {          // Bass expression: for red Welte specifically
			track    = 3;
			channel  = 0;
			velocity = 1;
		} else if (key < 64) {   // Bass register: for red Welte specifically
			track    = 1;
			channel  = 1;
			velocity = 64;
		} else if (key < 104) {  // Treble register: for red Welte specifically
			track    = 2;
			channel  = 2;
			velocity = 64;
		} else {                  // Treble expression: for red Welte specifically
			track    = 4;
			channel  = 3;
			velocity = 1;
		}
		midifile.addNoteOn(track, hi->origin.first - mintime, channel, hi->midikey, velocity);
		midifile.addNoteOff(track, hi->offtime - mintime, channel, hi->midikey);
		if (hi->offtime <= 0) {
			cerr << "ERROR OFFTIME IS ZERO: " << hi->offtime << endl;
		}
		if (hi->offtime < hi->origin.first) {
			cerr << "ERROR OFF TIME IS BEFORE ON TIME " << hi->origin.first << " VERSUS " << hi->offtime << " FOR KEY " << hi->midikey << endl;
		}
		if (hi->offtime > maxtime) {
			maxtime = hi->offtime;
		}
	}

	// add tempo correction
	double timevalue = 1.0;
	double tempo;
	ulongint curtime = 0;
	while (curtime < maxtime - mintime) {
		tempo = 60 / timevalue;
      midifile.addTempo(0, curtime, tempo);
		curtime += 3600;
		timevalue /= 1.0004;
	}

	midifile.sortTracks();
}
#endif



//////////////////////////////
//
// RollImage::printQualityReport --
//    default value: out = cerr
//

std::ostream& RollImage::printQualityReport(std::ostream& out) {
	if (!m_analyzedLeaders) {
		analyzeLeaders();
	}

	if (shifts.size() >= 20) {
		out << "Error: Too many shifts (" << shifts.size() << ")."
		    << " Maximum allowed is 19." << endl;
	}
	sortShiftsByAmount();
	if (!shifts.empty()) {
		double maxshift = std::fabs(shifts[0]->score);
		if (maxshift > 15.0) {
			out << "Error: Too large of a shift detected (" << maxshift << ")."
			    << " Maximum allowed is 15 pixels." << endl;
		}
	}
	int dustscore = int(getDustScore() + 0.5);
	if (dustscore > 1000) {
		out << "Error: margins are too dusty (" << dustscore << ")"
		    << " Maximum allowed is 1000 ppm." << endl;
	}

	return out;
}



//////////////////////////////
//
// RollImage::printRollImageProperties --
//    default value: out = cout
//

std::ostream& RollImage::printRollImageProperties(std::ostream& out) {
	if (!m_analyzedLeaders) {
		analyzeLeaders();
	}

	double averageRollWidth = getAverageRollWidth();
	averageRollWidth = int(averageRollWidth*100.0+0.5)/100.0;
	double averageSoftMarginWidth = getAverageSoftMarginTotal();
	averageSoftMarginWidth = int(averageSoftMarginWidth*100.0+0.5)/100.0;

	int musiclength = getLastMusicHoleEnd() - getFirstMusicHoleStart();

	double avgholewidth = int(getAverageMusicalHoleWidth()*100.0+0.5)/100.0;

	double leftCol = m_firstHolePosition - driftCorrection.at(getFirstMusicHoleStart());
	leftCol = leftCol - leftMarginIndex.at(getFirstMusicHoleStart());

	double rightCol = m_lastHolePosition - driftCorrection.at(getFirstMusicHoleStart());
	rightCol = rightMarginIndex.at(getFirstMusicHoleStart()) - rightCol;

	int trackerholes = getTrackerHoleCount();

	ulongint musicnotecount = 0;
	for (ulongint i=0; i<holes.size(); i++) {
		if (holes.at(i)->attack) {
			musicnotecount++;
		}
	}

	double driftmax = driftCorrection.at(getFirstMusicHoleStart());;
	double driftmin = driftCorrection.at(getFirstMusicHoleStart());;

	for (ulongint i=getFirstMusicHoleStart()+1; i<getLastMusicHoleEnd(); i++) {
		double drift = driftCorrection.at(i);
		if (driftmax < drift) {
			driftmax = drift;
		}
		if (driftmin > drift) {
			driftmin = drift;
		}
	}
	double driftrange = driftmax - driftmin;

#ifndef DONOTUSEFFT
	std::chrono::duration<double> processing_time = stop_time - start_time;
	std::chrono::system_clock::time_point nowtime = std::chrono::system_clock::now();
	std::time_t current_time = std::chrono::system_clock::to_time_t(nowtime);
#endif

	out << "@@ This file describes features extracted from a scan of a piano roll.\n";
	out << "@@ The contents of this file can be converted to JSON format with the\n";
	out << "@@ ATON.js library from http://aton.sapp.org\n";
	out << "\n";

	out << "@@BEGIN: ROLLINFO\n";

	out << "\n";

	out << "@@ Description of extracted parameters:\n";
	out << "@@\n";
	out << "@@ THRESHOLD:\t\t"         << "Threshold byte value for non-paper boundary" << std::endl;
	out << "@@ IMAGE_WIDTH:\t\t"       << "Width of the input image in pixels." << std::endl;
	out << "@@ IMAGE_LENGTH:\t"        << "Length of the input image in pixels." << std::endl;
	out << "@@ ROLL_WIDTH:\t\t"        << "Measured average width of the piano-roll in pixels." << std::endl;
	out << "@@ HARD_MARGIN_BASS:\t"    << "Pixel width of the margin on the bass side of the roll" << endl;
	out << "@@ \t\t\twhere the roll paper never enters." << std::endl;
	out << "@@ HARD_MARGIN_TREBLE:\t"  << "Pixel width of the margin on the treble side of the roll" << std::endl;
	out << "@@ \t\t\twhere the roll paper never enters." << std::endl;
	out << "@@ MAX_BASS_DRIFT:\t"      << "Maximum range of the \"soft\" margin on the bass side: the" << std::endl;
	out << "@@ \t\t\tmargin area where the roll edge will temporarily enter." << std::endl;
	out << "@@ MAX_TREBLE_DRIFT:\t"    << "Maximum range of the \"soft\" margin on the treble side: the margin" << std::endl;
	out << "@@ \t\t\tarea where the roll edge will temporarily enter." << std::endl;
	out << "@@ AVG_SOFT_MARGIN_SUM:\t" << "Average sum of the bass and treble soft margins." << std::endl;
	out << "@@ DRIFT_RANGE:\t"         << "Total drift range in pixels." << std::endl;
	out << "@@ DRIFT_MIN:\t\t"         << "Leftmost drift from average position in pixels." << std::endl;
	out << "@@ DRIFT_MAX:\t\t"         << "Rightmost drift from average position in pixels." << std::endl;
	out << "@@ PRELEADER_ROW:\t"     << "Last pixel row of the portion of the image which contains" << std::endl;
	out << "@@ \t\t\tthe velcro strap that initially pulls the roll." << std::endl;
	out << "@@ LEADER_ROW:\t\t"        << "Last pixel row of the leader (although text on" << std::endl;
	out << "@@ \t\t\tthe roll may continue)." << std::endl;
	out << "@@ FIRST_HOLE:\t\t"        << "Pixel row of the first musical hole." << std::endl;
	out << "@@ LAST_HOLE:\t\t"         << "Pixel row of the end of the last musical hole. Currently includes" << std::endl;
	out << "@@ \t\t\trewind holes and any punches after the rewind." << std::endl;
	out << "@@ END_MARGIN:\t\t"        << "IMAGE_LENGTH - LAST_HOLE." << std::endl;
	out << "@@ MUSICAL_LENGTH:\t"      << "Pixel row count from the first music hole to the end of" << std::endl;
	out << "@@ \t\t\tthe last music hole." << std::endl;
	out << "@@ MUSICAL_HOLES:\t"       << "Estimated number of intentional holes to be read" << std::endl;
	out << "@@ \t\t\tby the tracker bar." << std::endl;
	out << "@@ MUSICAL_NOTES:\t"       << "Number of notes after grouping chained holes (expression tracks" << std::endl;
	out << "@@ \t\t\tcount as notes in this census)." << std::endl;
	out << "@@ AVG_HOLE_WIDTH:\t"      << "Average width of a musical hole in pixels (based on" << std::endl;
	out << "@@ \t\t\tholes' bounding boxes)." << std::endl;
	out << "@@ ANTIDUST_COUNT:\t"      << "Number of holes in the paper with an area less than" << std::endl;
	out << "@@ \t\t\t50 pixels (the smallest music holes typically have 300 pixels)" << std::endl;
	out << "@@ BAD_HOLE_COUNT:\t"      << "Number of suspcious holes pulled out for further observation." << std::endl;
	out << "@@ EDGE_TEAR_COUNT:\t"     << "Number of edge tears which are deeper than 1/10 of an inch." << std::endl;
	out << "@@ BASS_TEAR_COUNT:\t"     << "Number of tears on the bass register side of the roll." << std::endl;
	out << "@@ TREBLE_TEAR_COUNT:\t"   << "Number of tears on the treble register side of the roll." << std::endl;
	out << "@@ DUST_SCORE:\t\t"        << "Count of dust particles in hard margin regions in units" << std::endl;
	out << "@@ \t\t\tof parts per million." << std::endl;
	out << "@@ DUST_SCORE_BASS:\t"     << "Dust particle count in bass register margin." << std::endl;
	out << "@@ DUST_SCORE_TREBLE:\t"   << "Dust particle count in bass register margin." << std::endl;
	out << "@@ SHIFTS:\t\t"            << "Number of automatically detected operator shifts greater" << std::endl;
	out << "@@ \t\t\tthan 1/100th of an inch over 1/3 of an inch." << std::endl;
	out << "@@ HOLE_SEPARATION:\t"     << "Distance between muiscal hole centers (i.e., the tracker" << std::endl;
	out << "@@ \t\t\tbar hole spacings)." << std::endl;
	out << "@@ HOLE_OFFSET:\t\t"       << "The offset of the tracker bar spacing pattern with respect to" << std::endl;
	out << "@@ \t\t\tthe first column of the image." << std::endl;
	out << "@@ TRACKER_HOLES:\t"       << "The esitmated number of tracker bar holes that reads this roll." << std::endl;
	out << "@@ SOFTWARE_DATE:\t"       << "The compiling date for the software that generates this file." << std::endl;
	out << "@@ ANALYSIS_DATE:\t"       << "The date that the analysis was done." << std::endl;
	out << "@@ ANALYSIS_TIME:\t"       << "The duration of the analysis phase of the software (excluding" << std::endl;
	out << "@@ \t\t\tloading of the image data or writing the analysis overlay" << std::endl;
	out << "@@ \t\t\tonto the image)." << std::endl;
	out << "@@ COLOR_CHANNEL:\t"       << "The color channel used to generate this analysis data." << std::endl;
	out << "@@ CHANNEL_MD5:\t\t"       << "The MD5 checksum of the color channel pixels used" << std::endl;
	out << "@@ \t\t\tin this analysis." << std::endl;
	out << "@@ MANUAL_EDITS:\t\t"      << "Set this field to \"yes\" if any manual edits are made to this file." << std::endl;
	out << "\n";

	out << "@THRESHOLD:\t\t"         << getThreshold()                << "\n";
	out << "@IMAGE_WIDTH:\t\t"       << getCols()                     << "px\n";
	out << "@IMAGE_LENGTH:\t\t"      << getRows()                     << "px\n";
	out << "@ROLL_WIDTH:\t\t"        << averageRollWidth              << "px\n";
	out << "@HARD_MARGIN_BASS:\t"    << getHardMarginLeftWidth()      << "px\n";
	out << "@HARD_MARGIN_TREBLE:\t"  << getHardMarginRightWidth()     << "px\n";
	out << "@MAX_BASS_DRIFT:\t"      << getSoftMarginLeftWidthMax()   << "px\n";
	out << "@MAX_TREBLE_DRIFT:\t"    << getSoftMarginRightWidthMax()  << "px\n";
	out << "@AVG_SOFT_MARGIN_SUM:\t" << averageSoftMarginWidth        << "px\n";
	out << "@DRIFT_RANGE:\t\t"       << int(driftrange*100+0.5)/100.0 << "px\n";
	out << "@DRIFT_MIN:\t\t"         << int(driftmax*100+0.5)/100.0   << "px\n";
	out << "@DRIFT_MAX:\t\t"         << int(driftmin*100+0.5)/100.0   << "px\n";
	out << "@PRELEADER_ROW:\t\t"     << getPreleaderIndex()           << "px\n";
	out << "@LEADER_ROW:\t\t"        << getLeaderIndex()              << "px\n";
	out << "@FIRST_HOLE:\t\t"        << getFirstMusicHoleStart()      << "px\n";
	out << "@LAST_HOLE:\t\t"         << getLastMusicHoleEnd()         << "px\n";
	out << "@END_MARGIN:\t\t"        << getRows() - getLastMusicHoleEnd() << "px\n";
	out << "@MUSICAL_LENGTH:\t"      << musiclength                   << "px\n";
	out << "@MUSICAL_HOLES:\t\t"     << holes.size()                  << "\n";
	out << "@MUSICAL_NOTES:\t\t"     << musicnotecount                << "\n";
	out << "@AVG_HOLE_WIDTH:\t"      << avgholewidth                  << "px\n";
	out << "@ANTIDUST_COUNT:\t"      << antidust.size()               << "\n";
	out << "@BAD_HOLE_COUNT:\t"      << badHoles.size()               << "\n";
	out << "@EDGE_TEAR_COUNT:\t"     << trebleTears.size() + bassTears.size() << "\n";
	out << "@BASS_TEAR_COUNT:\t"     << bassTears.size()              << "\n";
	out << "@TREBLE_TEAR_COUNT:\t"   << trebleTears.size()            << "\n";
	out << "@DUST_SCORE:\t\t"        << int(getDustScore()+0.5)       << "ppm\n";
	out << "@DUST_SCORE_BASS:\t"     << int(getDustScoreBass()+0.5)   << "ppm\n";
	out << "@DUST_SCORE_TREBLE:\t"   << int(getDustScoreTreble()+0.5) << "ppm\n";
	out << "@SHIFTS:\t\t"            << shifts.size()                 << "\n";
	out << "@HOLE_SEPARATION:\t"     << holeSeparation                << "px\n";
	out << "@HOLE_OFFSET:\t\t"       << holeOffset                    << "px\n";
	out << "@TRACKER_HOLES:\t\t"     << trackerholes                  << " (estimate)\n";
	out << "@SOFTWARE_DATE:\t\t"     << __DATE__ << " " << __TIME__ << endl;
#ifndef DONOTUSEFFT
	out << "@ANALYSIS_DATE:\t\t"     << std::ctime(&current_time);
	out << "@ANALYSIS_TIME:\t\t"     << int(processing_time.count()*100.0+0.5)/100.0 << "sec" << endl;
#endif
	out << "@COLOR_CHANNEL:\t\t"     << "green"                       << endl;
	out << "@CHANNEL_MD5:\t\t"       << getDataMD5Sum()               << endl;
	out << "@MANUAL_EDITS:\t\t"      << "no"                          << endl;


	/// MUSICAL HOLES /////////////////////////////////////////////////////////
	out << "\n\n";
	out << "@@ The HOLES array contains a list of all musical holes (note and expression holes).\n";
	out << "@@ They are sorted in time (or image row) from the start of the roll to the end.\n";
	out << "@@\n";
	out << "@@ Meaning of HOLE parameters:\n";
	out << "@@\n";
	out << "@@ ORIGIN_ROW:\t\tThe pixel row of the leading edge of the bounding box" << std::endl;
	out << "@@ \t\t\taround the hole.\n";
	out << "@@ ORIGIN_COL:\t\tThe pixel column of the leading edge of the bounding box around" << std::endl;
	out << "@@ \t\t\tthe hole, bass side.\n";
	out << "@@ WIDTH_ROW:\t\tThe pixel length of the bounding box around the hole.\n";
	out << "@@ WIDTH_COL:\t\tThe pixel column of the leading edge of the hole, bass side.\n";
	out << "@@ CENTROID_ROW:\tThe center of mass row of the hole.\n";
	out << "@@ CENTROID_COL:\tThe center of mass column of the hole.\n";
	out << "@@ AREA:\t\tThe area of the hole (in pixels).\n";
	out << "@@ PERIMETER:\t\tThe perimeter of the hole (in pixels).\n";
	out << "@@ CIRCULARITY:\t\tThe circularity of the hole (1 = circular, 0 = very uncircular).\n";
	out << "@@ MAJOR_AXIS:\t\tThe major axis of the hole in degrees (0 = vertically aligned" << std::endl;
	out << "@@ \t\t\twith roll).\n";
	out << "@@ HPIXCOR:\t\tHorizontal (column) pixel correction: shift to straighten hole columns" << std::endl;
	out << "@@ \t\t\ton paper.  If the leading and trailing edges of musical holes do not have" << std::endl;
	out << "@@ \t\t\tthe same correction value, then the following two parameters are given instead:\n";
	out << "@@ \t\t\t   HPIXCOR_LEAD:\tHorizontal pixel correction of the hole's leading edge.\n";
	out << "@@ \t\t\t   HPIXCOR_TRAIL:\tHorizontal pixel correction of the hole's trailing edge.\n";
	out << "@@\n";
	out << "\n";

	out << "@@BEGIN: HOLES\n\n";
	for (ulongint i=0; i<holes.size(); i++) {
		if (holes.at(i)->isMusicHole()) {
			holes.at(i)->printAton(out);
			out << std::endl;
		}
	}
	out << "@@END: HOLES\n\n";


	/// BAD HOLES //////////////////////////////////////////////////////////
	if (!badHoles.empty()) {
		sortBadHolesByArea();
		for (ulongint i=0; i<badHoles.size(); i++) {
			string id = "bad";
			if (i+1 < 100) { id += "0"; }
			if (i+1 < 10 ) { id += "0"; }
			id += my_to_string(i+1);
			badHoles.at(i)->id = id;
		}
		out << "\n\n";
		out << "@@BEGIN: BADHOLES\n\n";
		for (ulongint i=0; i<badHoles.size(); i++) {
			badHoles.at(i)->printAton(out);
			out << "\n";
		}
		out << "@@END: BADHOLES\n\n";
	}


	/// EDGE TEARS /////////////////////////////////////////////////////////
	if (bassTears.size() + trebleTears.size() > 0) {
		sortTearsByArea();
		out << "\n@@BEGIN: TEARS\n";
		if (trebleTears.size() > 0) {
			for (ulongint i=0; i<trebleTears.size(); i++) {
				string id = "trebletear";
				if (i+1 < 100) { id += "0"; }
				if (i+1 < 10 ) { id += "0"; }
				id += my_to_string(i+1);
				trebleTears.at(i)->id = id;
			}
			out << "@@BEGIN: TREBLE_TEARS\n";
			for (ulongint i=0; i<trebleTears.size(); i++) {
				trebleTears.at(i)->printAton(out);
			}
			out << "@@END: TREBLE_TEARS\n";
		}
		if (bassTears.size() > 0) {
			for (ulongint i=0; i<bassTears.size(); i++) {
				string id = "basstear";
				if (i+1 < 100) { id += "0"; }
				if (i+1 < 10 ) { id += "0"; }
				id += my_to_string(i+1);
				bassTears.at(i)->id = id;
			}
			out << "\n@@BEGIN: BASS_TEARS\n";
			for (ulongint i=0; i<bassTears.size(); i++) {
				bassTears.at(i)->printAton(out);
			}
			out << "@@END: BASS_TEARS\n";
		}
		out << "@@END: TEARS\n";
	}


	/// DRIFT /////////////////////////////////////////////////////////////
	out << "\n\n";
	out << "@@\n";
	out << "@@ The Drift data describes the left/right shifting of the roll along the\n";
	out << "@@ length of the image.  Each data point is a list values:\n";
	out << "@@    (1) The row number in the image\n";
	//out << "@@    (2) The distance in feet from the start of the roll (not the image)\n";
	//out << "@@        to the given row.\n";
	out << "@@    (2) The correction to apply to the row to straighten the roll.\n";
	out << "@@ A new data point is given whenever the drift changes by more than 0.1 pixels.\n";
	out << "@@ Drift analysis starts at the first music hole on the roll.\n";
	out << "@@\n";
	out << "\n";
	out << "@@BEGIN: DRIFT\n";
	out << "@RESOLUTION:\t0.1px\n";
	out << "@DATA:\n";
	// ulongint fff = getPreleaderIndex();
	// ulongint fff = getFirstMusicHoleStart();
	double lastdrift = -1.0;
	double drift;
	for (ulongint i=getFirstMusicHoleStart(); i<getLastMusicHoleEnd(); i++) {
		drift = int(driftCorrection.at(i)*10.0+0.5)/10.0;
		if (drift == lastdrift) {
			continue;
		}
		lastdrift = drift;
		out << "\t" << i;
		// out << "\t" << int((i - fff)/300.25/12*10000.0+0.5)/10000.0;
		out << "\t" << drift << "\n";
	}
	out << "@@END: DRIFT\n";


	/// SHIFTS //////////////////////////////////////////////////////////////
	if (!shifts.empty()) {
		sortShiftsByAmount();
		for (ulongint i=0; i<shifts.size(); i++) {
			string id = "shift";
			if (i+1 < 100) { id += "0"; }
			if (i+1 < 10 ) { id += "0"; }
			id += my_to_string(i+1);
			shifts.at(i)->id = id;
		}
		out << "\n\n";
		out << "@@\n";
		out << "@@ Shifts are left/right movements of the roll that are most likely\n";
		out << "@@ caused by the operator of the scanner as they adjust the paper to\n";
		out << "@@ recenter it.  The following list of shifts are sorted from largest\n";
		out << "@@ to smallest, with the smallest recorded shift being 1/100 of an inch\n";
		out << "@@ left or right, during 1/3 of an inch lengthwise on the roll.\n";
		out << "@@\n";
		out << "@@ Shift parameters are:\n";
		out << "@@    ROW: the pixel row in the image from the start of the scan to about the middle of the shift.\n";
		out << "@@    MOVEMENT: this amount of shift in pixels, with negative values being shifts to the bass (left).\n";
		out << "@@\n";
		out << "\n@@BEGIN: SHIFTS\n";
		out << "\n";
		for (ulongint i=0; i<shifts.size(); i++) {
			shifts.at(i)->printAton(out);
			out << "\n";
		}
		out << "@@END: SHIFTS\n";
	}

	out << "\n@@BEGIN: MIDIFILES\n\n";
	out << "@MIDIFILE:\n";
	stringstream ss;
	generateNoteMidiFileBinasc(ss);
	out << ss.str();
	out << "\n@@END: MIDIFILE\n";
	out << "\n@@END: MIDIFILES\n\n";

	// The following section is for displaying intermediate analysis data, mostly about
	// the tracker bar position.
	out << "\n@@BEGIN: DEBUGGING\n";
	out << "\n";
	out << "@@ HOLE_HISTOGRAM: a histogram of the centers of holes, both with and without" << endl;
	out << "@@ drift correction.  The meaning of the columns:\n";
	out << "@@ (1) the uncorrected positions of the hole centers\n";
	out << "@@ (2) the drift-corrected positions of the hole centers\n";
	out << "@@ (3) the weighted-average positions of the hole centers from (2) for each tracker bar position\n";
	out << "@@ (4) the modeled position of the tracker bar positions\n";
	out << "\n@@HOLE_HISTOGRAM:" << endl;
	std::vector<int> three(getCols(), 0);
	for (ulongint i=0; i< rawRowPositions.size(); i++) {
		three.at(rawRowPositions.at(i).first + 0.5) += rawRowPositions.at(i).second;
	}
	std::vector<double>& position = m_normalizedPosition;
	std::vector<int> four(getCols(), 0);
	for (ulongint i=0; i<position.size(); i++) {
		if (position.at(i) < 0) {
			continue;
		}
		four.at(position.at(i) + 0.5) += -100;
	}
	for (ulongint i=0; i<correctedCentroidHistogram.size(); i++) {
		out << "\t" << uncorrectedCentroidHistogram.at(i);
		out << "\t" << correctedCentroidHistogram.at(i);
		out << "\t" << three.at(i);
		out << "\t" << four.at(i) << endl;
	}
	out << "\n@@END: DEBUGGING\n";


	out << "\n@@END: ROLLINFO\n";
	return out;
}


//////////////////////////////
//
// RollImage::setDebugOn --
//

void RollImage::setDebugOn(void) {
	m_debug = true;
}



//////////////////////////////
//
// RollImage::setDebugOff --
//

void RollImage::setDebugOff(void) {
	m_debug = false;
}



//////////////////////////////
//
// RollImage::setWarningOn --
//

void RollImage::setWarningOn(void) {
	m_warning = true;
}



//////////////////////////////
//
// RollImage::setWarningOff --
//

void RollImage::setWarningOff(void) {
	m_warning = false;
}



//////////////////////////////
//
// RollImage::setThreshold --
//

void RollImage::setThreshold(int value) {
	m_threshold = value;
}



//////////////////////////////
//
// RollImage::getThreshold --
//

ucharint RollImage::getThreshold(void) {
	return (ucharint)m_threshold;
}


} // end of namespace prp




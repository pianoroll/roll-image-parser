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


#include "RollImage.h"
#include "HoleInfo.h"
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
	// do nothing
}



//////////////////////////////
//
// RollImage::~RollImage --
//

RollImage::~RollImage(void) {
	for (ulong i=0; i<holes.size(); i++) {
		delete holes[i];
	}
	holes.resize(0);
	for (ulong i=0; i<antidust.size(); i++) {
		delete antidust[i];
	}
	antidust.resize(0);

	for (ulong i=0; i<bassTears.size(); i++) {
		delete bassTears[i];
	}
	bassTears.resize(0);

	for (ulong i=0; i<trebleTears.size(); i++) {
		delete trebleTears[i];
	}
	trebleTears.resize(0);

	close();
}



//////////////////////////////
//
// RollImage::loadGreenChannel --
//

void RollImage::loadGreenChannel(void) {
	ulong rows = getRows();
	ulong cols = getCols();
	this->getImageGreenChannel(monochrome);
	pixelType.resize(rows);
	for (ulong r=0; r<rows; r++) {
		pixelType[r].resize(getCols());
		for (ulong c=0; c<cols; c++) {
			if (aboveThreshold(monochrome[r][c], 255)) {
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
	bool debug = 1;
	if (debug) { cerr << "STEP 1: analyzeBasicMargins" << endl; }
	analyzeBasicMargins();
	if (debug) { cerr << "STEP 2: analyzeLeaders" << endl; }
	analyzeLeaders();
	if (debug) { cerr << "STEP 3: analyzeAdvancedMargins" << endl; }
	analyzeAdvancedMargins();
	if (debug) { cerr << "STEP 4: generateDriftCorrection" << endl; }
	generateDriftCorrection(0.01);
	if (debug) { cerr << "STEP 5: analyzeHoles" << endl; }
	analyzeHoles();
	if (debug) { cerr << "STEP 6: analyzeTears" << endl; }
	analyzeTears();
	if (debug) { cerr << "STEP 7: generateDriftCorrection" << endl; }
	generateDriftCorrection(0.01);
	if (debug) { cerr << "STEP 8: calculateHoleDescriptors" << endl; }
	calculateHoleDescriptors();
	if (debug) { cerr << "STEP 9: invalidateSkewedHoles" << endl; }
	invalidateSkewedHoles();
	if (debug) { cerr << "STEP 10: markPosteriorLeader" << endl; }
	markPosteriorLeader();
	if (debug) { cerr << "STEP 11: analyzeTrackerBarSpacing" << endl; }
	analyzeTrackerBarSpacing();
	if (debug) { cerr << "STEP 12: analyzeTrackerBarPositions" << endl; }
	analyzeTrackerBarPositions();
	if (debug) { cerr << "STEP 13: analyzeHorizontalHolePosition" << endl; }
	analyzeHorizontalHolePosition();
	if (debug) { cerr << "STEP 14: analyzeMidiKeyMapping" << endl; }
	analyzeMidiKeyMapping();
	if (debug) { cerr << "STEP 15: invalidateEdgeHoles" << endl; }
	invalidateEdgeHoles();
	if (debug) { cerr << "STEP 16: recalculateFirstMusicHole" << endl; }
	recalculateFirstMusicHole();
	if (debug) { cerr << "STEP 17: addDriftInfoToHoles" << endl; }
	addDriftInfoToHoles();
	if (debug) { cerr << "STEP 18: addAntidustToBadHoles" << endl; }
	addAntidustToBadHoles(50);
	if (debug) { cerr << "STEP 19: assignMusicHoleIds" << endl; }
	assignMusicHoleIds();
	if (debug) { cerr << "STEP 20: FINSHED WITH ANALYSIS!" << endl; }
}



//////////////////////////////
//
// RollImage::addAntidustToBadHoles -- Add antidust holes to badHoles if they
//    are larger than the given threshold area.
//

void RollImage::addAntidustToBadHoles(ulong areaThreshold) {
	for (ulong i=0; i<antidust.size(); i++) {
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
	for (ulong i=0; i<holes.size(); i++) {
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
// RollImage::calculateCentralMoment --
//

double RollImage::calculateCentralMoment(HoleInfo& hole, int p, int q) {
	std::pair<double, double> center = hole.centroid;
	int ro = hole.origin.first;
	int co = hole.origin.second;
	double moment = 0.0;
	ulong r, c;
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
	ulong r;
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
	pair<ulong, ulong> start(r, c);
	pair<ulong, ulong> successor; // next point after starting point

	pair<ulong, ulong> previous = start;
	pair<ulong, ulong> current = previous;
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

int RollImage::findNextPerimeterPoint(pair<ulong, ulong>& point, int dir) {
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


	//double newoffset = holeOffset;
	//if (holeOffset > holeSeparation / 2) {
	//	newoffset = holeSeparation - holeOffset;
	//}
cerr << "HOLE SEPARATION: " << holeSeparation << endl;
cerr << "HOLE OFFSET:     " << holeOffset << endl;

	// assigned normalized pixel column positions to each hole column.
	for (ulong i=0; i<position.size(); i++) {
		position[i] = i * holeSeparation + holeOffset;
	}

	// find the frist position which has holes
	// ulong colstart = 0;
	for (ulong i=0; i<trackerArray.size(); i++) {
		if (trackerArray[i].empty()) {
			continue;
		}
		// colstart = i;
		m_firstHolePosition = position[i];
		break;
	}

	// find the last position which has holes
	// ulong colend = 0;
//ggg
	for (long i=(int)trackerArray.size()-1; i>=0; i--) {
		if (trackerArray[i].empty()) {
			continue;
		}
		// colend = i;
		m_lastHolePosition = position[i];
		break;
	}

	// check that the hole positions are within the bounds of the paper
	// within the tolerance of getMinTrackerEdge():

	std::vector<double> pixelpos((int)(getCols() / holeSeparation + 10), 0);
	for (ulong i=0; i<pixelpos.size(); i++) {
		pixelpos[i] = i * holeSeparation + holeOffset;
	}

	ulong r = getFirstMusicHoleStart();
cerr << "DRIFT CORRECTION " << driftCorrection[r] << endl;
	double leftmin = leftMarginIndex[r];
cerr << "LEFTMARGININDEX " << leftMarginIndex[r] << endl;
	leftmin += driftCorrection[r];
	leftmin += getMinTrackerEdge() * holeSeparation;
//	leftmin += holeOffset;
	int leftmostIndex = 0;
	for (ulong i=0; i<pixelpos.size(); i++) {
		if (pixelpos[i] > leftmin) {
			leftmostIndex = i;
			break;
		}
	}

// cerr << "LEFT TRACK INDEX " << leftmostIndex
// << "\tLEFTMIN = " << leftmin
// << "\tPIXELPOS = " << pixelpos[leftmostIndex]
// << endl;

	double rightmin = rightMarginIndex[r];
cerr << "RIGHTMARGININDEX " << rightMarginIndex[r] << endl;
	// rightmin -= driftCorrection[r]
	rightmin += - getMinTrackerEdge() * holeSeparation;
//	rightmin += holeOffset;
	int rightmostIndex = pixelpos.size()-1;
	for (ulong i=pixelpos.size() - 1; i>0; i--) {
		if (pixelpos[i] < rightmin) {
			rightmostIndex = i;
			break;
		}
	}

// cerr << "RIGHT TRACK INDEX " << rightmostIndex
// << "\tRIGHTMIN = " << rightmin
// << "\tPIXELPOS = " << pixelpos[rightmostIndex]
// << endl;

	int holecount = rightmostIndex - leftmostIndex + 1;
	if (holecount > 100) {
		std::cerr << "Warning hole count is quite large: " << holecount << std::endl;
	}
	if (holecount > 105) {
		std::cerr << "Error: way too many holes on paper (can't handle organ rolls yet)" << endl;
		exit(1);
	}

	// Rough guess for now on the mapping: placing the middle hole position on E4/F4 boundary
	int F4split = int((rightmostIndex - leftmostIndex) / 2 + leftmostIndex + 0.5); // 0.5 needed?

	// cerr << "RIGHTMOST INDEX " << rightmostIndex << endl;
	// cerr << "LEFTMOST INDEX " << leftmostIndex << endl;
	// cerr << "F4split INDEX " << F4split << endl;

	midiToHoleMapping.resize(256);
	std::fill(midiToHoleMapping.begin(), midiToHoleMapping.end(), 0);

	int adjustment = 64 - F4split;
	for (int i=leftmostIndex; i<=rightmostIndex; i++) {
		midiToHoleMapping.at(i + adjustment) = i;
	}

	int trackerholes = getTrackerHoleCount();

	if (trackerholes == 65) {
		// re-map up a major second
		for (ulong i=127; i>2; i--) {
			midiToHoleMapping[i] = midiToHoleMapping[i-2];

if (midiToHoleMapping[i]) {
cerr << "MIDI NOTE " << i << " MAPS TO TRACKER " << midiToHoleMapping[i] << endl;
}

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
// std::vector<std::vector<HoleInfo*>> trackerArray;
//

void RollImage::invalidateEdgeHoles(void) {

	ulong minmidi = 0;
	for (ulong i=0; i<255; i++) {
		if (midiToHoleMapping[i]) {
			minmidi = i;
			break;
		}
	}

	ulong maxmidi = 255;
	for (long i=255; i>=0; i--) {
		if (midiToHoleMapping[i]) {
			maxmidi = (ulong)i;
			break;
		}
	}

	ulong mintrack = midiToHoleMapping[minmidi];
	ulong maxtrack = midiToHoleMapping[maxmidi];

	ulong maxwidth = int(holeSeparation * getMaxHoleTrackerWidth() + 0.5);

	for (ulong i=0; i<holes.size(); i++) {
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
			holes[i]->reason = "too wide";
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
	for (ulong i=0; i<holes.size(); i++) {
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
	ulong r = hi.entry.first;
	ulong c = hi.entry.second;
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

void RollImage::analyzeHorizontalHolePosition() {
	int tcount = (getCols()+holeOffset) / holeSeparation;
	trackerArray.resize(0);
	trackerArray.resize(tcount);
	for (ulong i=0; i<holes.size(); i++) {
		double position = holes[i]->centroid.second;
		double correction = driftCorrection[int(holes[i]->centroid.first+0.5)];
		double cpos = position + correction;
		int index = int((cpos + holeOffset) / holeSeparation + 0.5);
		trackerArray.at(index).push_back(holes.at(i));
		holes[i]->track = index;
	}

	/*
	for (ulong i=0; i<trackerArray.size(); i++) {
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
	vector<vector<HoleInfo*>>& ta = trackerArray;

	ulong counter;
	ulong key;
	for (ulong i=0; i<ta.size(); i++) {
		counter = 1;
		key = midiToHoleMapping[i];
		for (ulong j=0; j<ta[i].size(); j++) {
			if (!ta[i][j]->isMusicHole()) {
				continue;
			}
			ta[i][j]->id = "K" + to_string(key) + "_N" + to_string(counter++);
		}
	}
}



//////////////////////////////
//
// RollImage::analyzeTrackerBarPositions --
//

void RollImage::analyzeTrackerBarPositions(void) {
	// cerr << "\n\nCALCULATING BAR POSITIONS: " << endl;
	int count = (int)holeSeparation;
	std::vector<double> score(count);

	for (int i=0; i<count; i++) {
		score[i] = getTrackerShiftScore(i);
		// cerr << i << "\t" << score[i] << "\n";
	}

	int minindex = 0;
	for (ulong i=1; i<score.size(); i++) {
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
	for (ulong i=0; i<x.size(); i++) {
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
// RollImage::analyzeTrackerBarSpacing --
//

void RollImage::analyzeTrackerBarSpacing(void) {
	correctedCentroidHistogram.resize(getCols(), 0);

	for (ulong i=0; i<holes.size(); i++) {
		ulong centroidr = holes[i]->centroid.first;
		double correction = driftCorrection[centroidr];
		int position = holes[i]->centroid.second + correction + 0.5;
		correctedCentroidHistogram[position]++;
	}
	//cerr << "Histogram:\n";
	//for (ulong i=0; i<correctedCentroidHistogram.size(); i++) {
	//	cerr << i << "\t" << correctedCentroidHistogram[i] << endl;
	//}

	ulong leftside  = getHardMarginLeftIndex();
	ulong rightside = getHardMarginRightIndex();
	ulong width     = rightside - leftside;
	ulong padding = width / 10;
	leftside += padding;
	rightside -= padding;

	std::vector<mycomplex> spectrum;
	int factor = 16;
	std::vector<mycomplex> input(4096 * factor);
	for (ulong i=0; i<4096; i++) {
		input.at(i) = correctedCentroidHistogram.at(i);
	}
	for (ulong i=4096; i<input.size(); i++) {
		input.at(i) = 0.0;
	}
	FFT(spectrum, input);

	vector<double> magnitudeSpectrum(spectrum.size());
	int maxmagi = factor*2;
	for (ulong i=0 ;i<spectrum.size(); i++) {
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

	// cerr << "\n\nspectrum:\n";
	// for (ulong i=0; i<spectrum.size(); i++) {
	// 	cerr << spectrum[i].first << "\t" << spectrum[i].second << endl;
	// }

}



//////////////////////////////
//
// RollImage::analyzeTears -- Find tears in the edges of the rolls.
//

void RollImage::analyzeTears(void) {

	ulong rows = getRows();
	std::vector<double> fastLeft(rows);
	std::vector<double> fastRight(rows);
	std::vector<double> slowLeft(rows);
	std::vector<double> slowRight(rows);

	for (ulong r=0; r<rows; r++) {
		fastLeft[r]  = leftMarginIndex[r];
		slowLeft[r]  = leftMarginIndex[r];
		fastRight[r] = rightMarginIndex[r];
		slowRight[r] = rightMarginIndex[r];
	}

	exponentialSmoothing(fastLeft,  0.100);
	exponentialSmoothing(fastRight, 0.100);
	exponentialSmoothing(slowLeft,  0.001);
	exponentialSmoothing(slowRight, 0.001);

	ulong startrow = getLeaderIndex();
	std::vector<double> left(rows, 0.0);
	std::vector<double> right(rows, 0.0);
	for (ulong r=startrow; r<rows; r++) {
		left[r] = fastLeft[r] - slowLeft[r] - 3;
		right[r] = slowRight[r] -fastRight[r] - 3;
		if (left[r] < 0.0) {
			left[r] = 0.0;
		}
		if (right[r] < 0.0) {
			right[r] = 0.0;
		}
		// cout << left[r] << "\t" << right[r] << endl;
	}
	// cout << endl;


	std::vector<PreTearInfo> ltear;
	std::vector<PreTearInfo> rtear;
	ltear.reserve(1000);
	rtear.reserve(1000);

	double cutoff = 3.0;

	ulong peakindex;
	double peakvalue;
	ulong startr;

	// identify left (bass) tears
	for (ulong r=startrow; r<rows; r++) {
		if (!left[r]) {
			continue;
		}
		startr = r;
		r = findPeak(left, startr, peakindex, peakvalue);
		if (peakvalue >= cutoff) {
			ltear.resize(ltear.size() + 1);
			ltear.back().peakindex = peakindex;
			ltear.back().peak= peakvalue;
			ltear.back().startindex = startr;
			ltear.back().endindex = r;
		}
	}

	for (ulong r=startrow; r<rows; r++) {
		if (!right[r]) {
			continue;
		}
		startr = r;
		r = findPeak(right, startr, peakindex, peakvalue);
		if (peakvalue >= cutoff) {
			rtear.resize(rtear.size() + 1);
			rtear.back().peakindex = peakindex;
			rtear.back().peak = peakvalue;
			rtear.back().startindex = startr;
			rtear.back().endindex = r;
		}
	}

	processTears(ltear, rtear, slowLeft, slowRight, fastLeft, fastRight);

}



//////////////////////////////
//
// RollImage::processTears --
//

void RollImage::processTears(std::vector<PreTearInfo>& ltear,
		std::vector<PreTearInfo>& rtear, std::vector<double>& lslow,
		std::vector<double>& rslow, std::vector<double>& lfast,
		std::vector<double>& rfast) {
	TearInfo *ti;
	for (ulong i=0; i<ltear.size(); i++) {
		ti = new TearInfo;
		getTearInfo(*ti, ltear.at(i), lslow, lfast, 0, rslow, rfast);
		if (ti->area == 0) {
			delete ti;
			continue;
		}
		bassTears.push_back(ti);
	}
	for (ulong i=0; i<rtear.size(); i++) {
		ti = new TearInfo;
		getTearInfo(*ti, rtear.at(i), rslow, rfast, 1, lslow, lfast);
		if (ti->area == 0) {
			delete ti;
			continue;
		}
		trebleTears.push_back(ti);
	}
}



//////////////////////////////
//
// RollImage::isGoodOppositeEdge --
//

bool RollImage::isGoodOppositeEdge(PreTearInfo& pti,
		std::vector<double>& slow, std::vector<double>& fast) {
	ulong startrow = pti.startindex;
	ulong endrow   = pti.endindex;

	double threshold = 10.0;

	double bad = 0.0;
	double good = 0.0;
	double length = endrow - startrow + 1;

	for (ulong r=startrow; r<=endrow; r++) {
		if (fabs(fast[r] - slow[r]) <= threshold) {
			good++;
		} else{
			bad++;
		}
	}

	if (good/length > 0.90) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// RollImage::getTearInfo --
//

void RollImage::getTearInfo(TearInfo& ti, PreTearInfo& pti,
		std::vector<double>& slow, std::vector<double>& fast, int side,
		std::vector<double>& oslow, std::vector<double>& ofast) {

	ulong startcol = slow[pti.startindex];
	ulong endcol   = slow[pti.endindex];

	// ulong toprow = 0;
	// ulong botrow = 0;
	ulong checkcol = slow[pti.endindex];

	if (side == 0) {
		checkcol--;
	} else {
		checkcol++;
	}

	bool goodOppositeEdge = isGoodOppositeEdge(pti, oslow, ofast);

	int trim = 1;

	if (goodOppositeEdge) {

		double avgwidth = getAverageRollWidth();
		if (side == 0) {
			startcol = oslow[pti.startindex] - avgwidth + trim;
			endcol = oslow[pti.endindex] - avgwidth + trim;
			checkcol = endcol - 1;
		} else {
			startcol = oslow[pti.startindex] + avgwidth - trim;
			endcol = oslow[pti.endindex] + avgwidth - trim;
			checkcol = endcol + 1;
		}
	}

	bool constant = false;
	if (startcol == endcol) {
		constant = true;
	}
	constant = true; // needed for avoiding diagonal escape of fill operation.

	ulong c;
	ulong middlerow = pti.startindex/2 + pti.endindex/2;
	ulong paper1 = 0;
	ulong paper2 = 0;

	// draw a vertical line along the area where the hole is.
	// Keep trying to extend the line until paper is found, but only
	// extend up to 30 pixels after the pretear identified margin.

	for (ulong r=middlerow; r<pti.endindex + 30; r++) {
		if (constant) {
			c = startcol;
		} else {
			c = slow[r];
		}
		if (pixelType.at(r).at(c) != PIX_PAPER) {
			pixelType[r][c] = PIX_TEAR;
			if (side == 0) {
				leftMarginIndex[r] = c;
			} else {
				rightMarginIndex[r] = c;
			}
			paper1 = r;
			// toprow = paper1;
		} else {
			paper1 = r - 1;
			// toprow = paper1;
			break;
		}
	}
	ulong cols = getCols();

	// If a paper pixel was not terminating the above line, then
	// draw a line to the first paper pixel towards the paper center.

	if (paper1) {
		ulong endc;
		ulong r = paper1;
		if (side == 0) {
			endc = cols;
			for (ulong c=startcol; c<=endc; c++) {
				if (pixelType[r][c] == PIX_PAPER) {
					break;
				} else {
					pixelType[r++][c] = PIX_DEBUG2;
					if (pixelType[r][c] != PIX_PAPER) {
						pixelType[r][c] = PIX_DEBUG2;
					}
				}
			}
		} else {
			endc = 0;
			for (ulong c=startcol; c>=endc; c--) {
				if (pixelType[r][c] == PIX_PAPER) {
					break;
				} else {
					pixelType[r++][c] = PIX_TEAR;
					if (pixelType[r][c] != PIX_PAPER) {
						pixelType[r][c] = PIX_DEBUG2;
					}
				}
			}

		}
	}
	paper1 = 0;

	// Do one more row towards the paper

	for (ulong r=middlerow; r<pti.endindex + 30; r++) {
		if (constant) {
			c = startcol;
		} else {
			c = slow[r];
		}
		if (side == 0) {
			c++;
		} else {
			c--;
		}
		if (pixelType[r][c] != PIX_PAPER) {
			pixelType[r][c] = PIX_TEAR;
			if (side == 0) {
				leftMarginIndex[r] = c;
			} else {
				rightMarginIndex[r] = c;
			}
			paper1 = r;
			// botrow = paper1;
		} else {
			paper1 = r - 1;
			// botrow = paper1;
			break;
		}
	}

	// Draw a line to the paper if there is no end found above.

	if (paper1) {
		ulong endc;
		ulong r = paper1;
		if (side == 0) {
			endc = cols;
			for (ulong c=startcol; c<=endc; c++) {
				if (pixelType[r][c] == PIX_PAPER) {
					break;
				} else {
					pixelType[r++][c] = PIX_TEAR;
					if (pixelType[r][c] != PIX_PAPER) {
						pixelType[r][c] = PIX_DEBUG2;
					}
				}
			}
		} else {
			endc = 0;
			for (ulong c=startcol; c>=endc; c--) {
				if (pixelType[r][c] == PIX_PAPER) {
					break;
				} else {
					pixelType[r++][c] = PIX_TEAR;
					if (pixelType[r][c] != PIX_PAPER) {
						pixelType[r][c] = PIX_DEBUG2;
					}
				}
			}

		}
	}
	paper1 = 0;


	// Now go upwards on the roll (to earlier time)

	for (ulong r=middlerow; r>=pti.startindex - 30; r--) {
		if (constant) {
			c = startcol;
		} else {
			c = slow[r];
		}
		if (pixelType[r][c] != PIX_PAPER) {
			pixelType[r][c] = PIX_TEAR;
			if (side == 0) {
				leftMarginIndex[r] = c;
			} else {
				rightMarginIndex[r] = c;
			}
			paper2 = r;
		} else {
			paper2 = r + 1;
			break;
		}
	}

	if (paper2) {
		ulong endc;
		ulong r = paper2;
		if (side == 0) {
			endc = cols;
			for (ulong c=startcol; c<=endc; c++) {
				if (pixelType[r][c] == PIX_PAPER) {
					break;
				} else {
					pixelType[r--][c] = PIX_TEAR;
					if (pixelType[r][c] != PIX_PAPER) {
						pixelType[r][c] = PIX_DEBUG2;
					}
				}
			}
		} else {
			endc = 0;
			for (ulong c=startcol; c>=endc; c--) {
				if (pixelType[r][c] == PIX_PAPER) {
					break;
				} else {
					pixelType[r--][c] = PIX_TEAR;
					if (pixelType[r][c] != PIX_PAPER) {
						pixelType[r][c] = PIX_DEBUG2;
					}
				}
			}

		}
	}
	paper2 = 0;

	for (ulong r=middlerow; r>=pti.startindex - 30; r--) {
		if (constant) {
			c = startcol;
		} else {
			c = slow[r];
		}
		if (side == 0) {
			c++;
		} else {
			c--;
		}
		if (pixelType[r][c] != PIX_PAPER) {
			pixelType[r][c] = PIX_TEAR;
			if (side == 0) {
				leftMarginIndex[r] = c;
			} else {
				rightMarginIndex[r] = c;
			}
			paper2 = r;
		} else {
			paper2 = r + 1;
			break;
		}
	}

	if (paper2) {
		ulong endc;
		ulong r = paper2;
		if (side == 0) {
			endc = cols;
			for (ulong c=startcol; c<=endc; c++) {
				if (pixelType[r][c] == PIX_PAPER) {
					break;
				} else {
					pixelType[r--][c] = PIX_TEAR;
					if (pixelType[r][c] != PIX_PAPER) {
						pixelType[r][c] = PIX_DEBUG2;
					}
				}
			}
		} else {
			endc = 0;
			for (ulong c=startcol; c>=endc; c--) {
				if (pixelType[r][c] == PIX_PAPER) {
					break;
				} else {
					pixelType[r--][c] = PIX_TEAR;
					if (pixelType[r][c] != PIX_PAPER) {
						pixelType[r][c] = PIX_DEBUG2;
					}
				}
			}

		}
	}
	paper2 = 0;

/* Don't do water-filling procedure for now, as basic fill is good enough:
	if ((toprow > 0) && (botrow > 0)) {
		ulong counter0 = 0;
		while (counter0 < 100) {
			counter0++;
			if (goodColumn(checkcol, toprow, botrow, PIX_MARGIN, 30)) {
				if (side == 0) {
					fillColumn(checkcol--, toprow, botrow, PIX_MARGIN, 30, PIX_TEAR, leftMarginIndex);
				} else {
					fillColumn(checkcol++, toprow, botrow, PIX_MARGIN, 30, PIX_TEAR, rightMarginIndex);
				}
			} else {
				break;
			}
		}
	}
*/

	ulong r = pti.peakindex;
	c = (slow[r] + fast[r])/2;

	int counter = 0;
	ti.entry.first  = r;
	ti.entry.second = c;

	fillTearInfo(ti, r, c, counter);

	ti.width.first = ti.width.first - ti.origin.first;
	ti.width.second = ti.width.second - ti.origin.second;
}



//////////////////////////////
//
// RollImage::fillColumn --
//

void RollImage::fillColumn(ulong col, ulong toprow, ulong botrow, ulong target, ulong threshold, ulong replacement,
		std::vector<int>& margin) {

	if (toprow > botrow) {
		ulong temp = toprow;
		toprow = botrow;
		botrow = temp;
	}

	ulong maxup    = toprow - threshold;
	ulong maxdown  = botrow + threshold;
	ulong midpoint = abs((int)maxup-(int)maxdown)/2 + maxup;

	for (ulong r=midpoint; r>=maxup; r--) {
		if (pixelType[r][col] == target) {
			pixelType[r][col] = (char)replacement;
			margin[r] = col;
		} else {
			break;
		}
	}

	for (ulong r=midpoint; r<=maxdown; r++) {
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

bool RollImage::goodColumn(ulong col, ulong toprow, ulong botrow, ulong ptype, ulong threshold) {
	if (toprow > botrow) {
		ulong temp = toprow;
		toprow = botrow;
		botrow = temp;
	}

	ulong maxup    = toprow - threshold;
	ulong maxdown  = botrow + threshold;
	ulong midpoint = abs((int)maxup-(int)maxdown)/2 + maxup;

	ulong toppaper = 0;
	ulong botpaper = 0;

	for (ulong r=midpoint; r>=maxup; r--) {
		cerr << "PIXEL " << r << ", " << col << " = " << (int)pixelType[r][col] << endl;
		pixelType[r][col] = PIX_DEBUG3;
		if (pixelType[r][col] == PIX_PAPER) {
			cerr << "FOUND PAPER PIXEL " << endl;
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

	for (ulong r=midpoint; r<=maxdown; r++) {
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

ulong RollImage::findPeak(std::vector<double>& array, ulong r, ulong& peakindex,
		double& peakvalue) {
	peakindex = r;
	peakvalue = array[r];
	ulong rows = getRows();
	for (ulong i=r; i<rows; i++) {
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

	ulong rows = getRows();
	std::vector<double> lmargin(rows);
	std::vector<double> rmargin(rows);

	for (ulong r=0; r<rows; r++) {
		lmargin[r] = leftMarginIndex[r];
		rmargin[r] = rightMarginIndex[r];
	}

	exponentialSmoothing(lmargin, gain);
	exponentialSmoothing(rmargin, gain);

	ulong startrow = getLeaderIndex() + 100;
	ulong endrow   = getRows() - 100;
	ulong length = endrow - startrow + 1;

	double lavg = getAverage(lmargin, startrow, length);
	double ravg = getAverage(rmargin, startrow, length);

	driftCorrection.resize(rows);
	fill(driftCorrection.begin(), driftCorrection.end(), 0.0);

	for (ulong r=startrow; r<=endrow; r++) {
		driftCorrection[r] = -((lmargin[r] - lavg) + (rmargin[r] - ravg)) / 2.0;
	}

	// Store the drift correction for each hole (at the centroid point,
	// although perhaps the leading edge drift might be better).
	for (ulong i=0; i<holes.size(); i++) {
		ulong r = holes[i]->centroid.first;
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
	ulong startrow = getLeaderIndex() + 1;
	ulong endrow   = getFirstMusicHoleStart() - 1;

	ulong cols = getCols();
	for (ulong r=startrow; r<=endrow; r++) {
		for (ulong c=0; c<cols; c++) {
			if (pixelType[r][c] != PIX_PAPER) {
				pixelType[r][c] = PIX_POSTLEADER;
			}
		}
	}

	ulong rows = getRows();
	endrow = rows - 1;
	startrow = getLastMusicHoleEnd() + 1;
	for (ulong r=startrow; r<=endrow; r++) {
		for (ulong c=0; c<cols; c++) {
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
	ulong startrow = getLeaderIndex();
	ulong endrow   = getRows();
	holes.clear();
	holes.reserve(getMaxHoleCount() + 1024);

	for (ulong r=startrow; r<endrow; r++) {
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

void RollImage::extractHole(ulong row, ulong col) {
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

	ulong testFirst = hi->origin.first;
	ulong testLast  = hi->width.first;

	// Convert lower right corner corrdinate into a width:
	hi->width.first  = hi->width.first  - hi->origin.first;
	hi->width.second = hi->width.second - hi->origin.second;

	ulong minarea = 100;
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

void RollImage::fillHoleSimple(ulong r, ulong c, int target, int type, int& counter) {
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

void RollImage::fillHoleInfo(HoleInfo& hi, ulong r, ulong c, int& counter) {
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

void RollImage::fillTearInfo(TearInfo& ti, ulong r, ulong c, int& counter) {
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
	ulong rows = getRows();
	ulong cols = getCols();

	leftMarginIndex.resize(rows);
	rightMarginIndex.resize(rows);

	for (ulong r=0; r<rows; r++) {
		std::vector<uchar>& rowdata = pixelType.at(r);
		leftMarginIndex[r] = 0;
		for (ulong c=0; c<cols; c++) {
			if (rowdata.at(c) == PIX_PAPER) {
				leftMarginIndex[r] = c - 1;
				break;
			} else {
				rowdata.at(c) = PIX_MARGIN;
				leftMarginIndex[r] = c;
			}
		}
	}


	for (ulong r=0; r<rows; r++) {
		std::vector<uchar>& rowdata = pixelType.at(r);
		rightMarginIndex[r] = 0;
		for (int c=cols-1; c>=0; c--) {
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
	ulong rows = getRows();
	ulong cols = getCols();

	for (ulong r=0; r<rows-1; r++) {
		std::vector<uchar>& row1 = pixelType[r];
		std::vector<uchar>& row2 = pixelType[r+1];
		for (ulong c=0; c<cols; c++) {
			if (row1[c] != PIX_MARGIN) {
				continue;
			}
			if (row2[c] != PIX_PAPER) {
				row2[c] = PIX_MARGIN;
				if (c < cols/2) {
					if (c > (ulong)leftMarginIndex[r+1]) {
						leftMarginIndex[r+1] = c;
					}
				} else {
					if (c < (ulong)rightMarginIndex[r+1]) {
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
	ulong rows = getRows();
	ulong cols = getCols();

	for (ulong r=rows-1; r>0; r--) {
		std::vector<uchar>& row1 = pixelType.at(r);
		std::vector<uchar>& row2 = pixelType.at(r-1);
		for (ulong c=0; c<cols; c++) {
			if (row1.at(c) != PIX_MARGIN) {
				continue;
			}
			if (row2.at(c) != PIX_PAPER) {
				row2.at(c) = PIX_MARGIN;
				if (c < cols/2) {
					if (c > (ulong)leftMarginIndex.at(r-1)) {
						leftMarginIndex.at(r-1) = c;
					}
				} else {
					if (c < (ulong)rightMarginIndex.at(r-1)) {
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
	ulong rows = getRows();
	ulong cols = getCols();

	for (ulong c=0; c<cols-1; c++) {
		for (ulong r=0; r<rows; r++) {
			if (pixelType.at(r).at(c) != PIX_MARGIN) {
				continue;
			}
			if (pixelType.at(r).at(c+1) != PIX_PAPER) {
				pixelType.at(r).at(c+1) = PIX_MARGIN;
				if (c < cols/2) {
					if (c+1 > (ulong)leftMarginIndex.at(r)) {
						leftMarginIndex.at(r) = c+1;
					}
				} else {
					if (c+1 < (ulong)rightMarginIndex.at(r)) {
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
	ulong rows = getRows();
	ulong cols = getCols();

	for (ulong c=cols-1; c>0; c--) {
		for (ulong r=0; r<rows; r++) {
			if (pixelType.at(r).at(c) != PIX_MARGIN) {
				continue;
			}
			if (pixelType.at(r).at(c-1) != PIX_PAPER) {
				pixelType.at(r).at(c-1) = PIX_MARGIN;
				if (c < cols/2) {
					if (c-1 > (ulong)leftMarginIndex.at(r)) {
						leftMarginIndex.at(r) = c-1;
					}
				} else {
					if (c-1 < (ulong)rightMarginIndex.at(r)) {
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
	ulong cols = getCols();
	ulong rows = getRows();

	double topLeftAvg  = getAverage(leftMarginIndex, 0, cols);
	double topRightAvg = getAverage(rightMarginIndex, 0, cols);
	double botLeftAvg  = getAverage(leftMarginIndex, rows-1-4096, cols);
	double botRightAvg = getAverage(rightMarginIndex, rows-1-4096, cols);

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
		exit(1);
	}

	ulong leftLeaderBoundary = 0;
	leftLeaderBoundary = findLeftLeaderBoundary(leftMarginIndex, botLeftAvg, cols, 4096*4);

	ulong rightLeaderBoundary = 0;
	rightLeaderBoundary = findRightLeaderBoundary(rightMarginIndex, botRightAvg, cols, 4096*4);

	ulong leaderBoundary = (leftLeaderBoundary + rightLeaderBoundary) / 2;
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

ulong RollImage::findLeftLeaderBoundary(std::vector<int>& margin, double avg, ulong cols,
		ulong searchlength) {
	std::vector<int> status(searchlength, 0);
	int cutoff = int(avg * 1.05 + 0.5);
	for (ulong i=0; (i<status.size()) && (i<margin.size()); i++) {
		if (margin[i] > cutoff) {
			status[i] = 1;
		}
	}
	ulong boundary = getBoundary(status);
	return boundary;
}



//////////////////////////////
//
// findRightLeaderBoundary -- Boundary between the leader and the rest of the
//   roll as a row index number.
//

ulong RollImage::findRightLeaderBoundary(std::vector<int>& margin, double avg,
		ulong cols, ulong searchlength) {
	std::vector<int> status(searchlength, 0);
	int cutoff = int(avg / 1.05 + 0.5);
	for (ulong i=0; (i<status.size()) && (i<margin.size()); i++) {
		if (margin.at(i) < cutoff) {
			status[i] = 1;
		}
	}
	ulong boundary = getBoundary(status);
	return boundary;
}



/////////////////////////////
//
// getBoundary -- Used by findRightLeaderBoundary and findLeftLeaderBondary.
//

ulong RollImage::getBoundary(std::vector<int>& status) {
	ulong windowsize = 100;
	ulong above = 0;
	ulong below = 0;
	for (ulong i=0; (i<windowsize) && (i<status.size()); i++) {
		above += status.at(i);
	}
	for (ulong i=windowsize; (i<windowsize*2) && (i<status.size()); i++) {
		below += status.at(i);
	}

	for (ulong i=windowsize+1; i<status.size()-1-windowsize; i++) {
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

	std::cerr << "COULD NOT FIND LEADER BOUNDARY" << std::endl;
	exit(1);
	return 0;
}



//////////////////////////////
//
// RollImage::mergePixelOverlay --
//

void RollImage::mergePixelOverlay(std::fstream& output) {

	std::vector<uchar> pixel(3);
	ulong offset;
	ulong rows = getRows();
	ulong cols = getCols();

	for (ulong r=0; r<rows; r++) {
		for (ulong c=0; c<cols; c++) {
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

				case PIX_HOLEBB_LEADING:   // musical hole bounding box (yellow)
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] =   0;
					break;

				case PIX_HOLEBB_TRAILING:  // musical hole bounding box (red)
					pixel[0] = 255;
					pixel[1] =   0;
					pixel[2] =   0;
					break;

				case PIX_HOLEBB_BASS:      // musical hole bounding box (red)
					pixel[0] = 255;
					pixel[1] =   0;
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
			output.seekp(offset);
			output.write((char*)pixel.data(), 3);
		}
	}
}



//////////////////////////////
//
// RollImage::extractPreleaderIndex -- Return the row in the image where the preleader
//    ends and the leader starts.
//

ulong RollImage::extractPreleaderIndex(ulong leaderBoundary) {

	std::vector<ulong> marginsum;
	ulong tolerance = 20;       // How much the positions adjacent to the min
	                            // can grow, yet still be called the preleader.
	ulong startboundary = 10;   // Avoid the very start of the image
	                            // in case there are scanning artifacts.
	ulong cols = getCols();

	marginsum.resize(leaderBoundary);
	fill(marginsum.begin(), marginsum.end(), 0);
	for (ulong i=startboundary; i<marginsum.size(); i++) {
		marginsum[i] = leftMarginIndex[i] + cols - rightMarginIndex[i];
	}
	ulong position = maxValueIndex(marginsum);

	ulong posadj = position;
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

void RollImage::analyzeHardMargins(ulong leaderBoundary) {

	ulong endboundary = 1000;

	ulong minpos = leftMarginIndex[leaderBoundary];
	ulong rows = pixelType.size();
	for (ulong r=leaderBoundary+1; r<rows-endboundary; r++) {
		if ((ulong)leftMarginIndex[r] < minpos) {
			minpos = leftMarginIndex[r];
			//minrow = r;
		}
	}
	setHardMarginLeftIndex(minpos);

	for (ulong r=leaderBoundary; r<rows; r++) {
		for (ulong c=0; c<=minpos; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_HARDMARGIN;
			}
		}
	}

	ulong maxpos = rightMarginIndex[leaderBoundary];
	for (ulong r=leaderBoundary+1; r<rows; r++) {
		if ((ulong)rightMarginIndex[r] > maxpos) {
			maxpos = rightMarginIndex[r];
		}
	}
	for (ulong r=leaderBoundary; r<rows; r++) {
		ulong cols = pixelType[r].size();
		for (ulong c=maxpos; c<cols; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_HARDMARGIN;
			}
		}
	}
	setHardMarginRightIndex(maxpos);

}



//////////////////////////////
//
// RollImage::setHardMarginLeftIndex --
//

void RollImage::setHardMarginLeftIndex(ulong index) {
	hardMarginLeftIndex = index;
}



//////////////////////////////
//
// RollImage::setHardMarginRightIndex --
//

void RollImage::setHardMarginRightIndex(ulong index) {
	hardMarginRightIndex = index;
}



//////////////////////////////
//
// RollImage::markPreleaderRegion --
//

void RollImage::markPreleaderRegion(void) {
	ulong cols = getCols();

	// mark holes in leader region as leader holes.

	for (ulong r=0; r<=preleaderIndex; r++) {
		for (ulong c=0; c<cols; c++) {
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
	ulong cols = getCols();

	// mark holes in leader region as leader holes.

	for (ulong r=0; r<leaderIndex; r++) {
		for (ulong c=0; c<cols; c++) {
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

void RollImage::setPreleaderIndex(ulong value) {
	preleaderIndex = value;
}



//////////////////////////////
//
// RollImage::setLeaderIndex -- Set the row index that is the boundary
//    between the preleader and the leader.
//

void RollImage::setLeaderIndex(ulong value) {
	leaderIndex = value;
}



//////////////////////////////
//
// RollImage::getPreleaderIndex -- return the row index that is the boundary
//    between the preleader and the leader.
//

ulong RollImage::getPreleaderIndex(void) {
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

ulong RollImage::getLeaderIndex(void) {
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

ulong RollImage::getFirstMusicHoleStart(void) {
	return firstMusicRow;
}



//////////////////////////////
//
// RollImage::recalculateFirstMusicHole --
//

void RollImage::recalculateFirstMusicHole(void) {
	ulong minrow = getRows() - 1;
	std::vector<std::vector<HoleInfo*>>& ta = trackerArray;
	ulong row;
	for (ulong i=0; i<ta.size(); i++) {
		for (ulong j=0; j<ta[i].size(); j++) {
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
	ulong limit = firstMusicRow;
	std::vector<ulong> listing;
	for (ulong i=0; i<badHoles.size(); i++) {
		if (badHoles[i]->origin.first < limit) {
			listing.push_back(i);
		}
	}

	if (listing.empty()) {
		// nothing to do
		return;
	}

	ulong start = listing.size() - 1;
	for (int i=(int)start; i >= 0; i--) {
		badHoles.erase(badHoles.begin() + listing.back());
		listing.pop_back();
	}
}



//////////////////////////////
//
// RollImage::getLastMusicHoleStartEnd --
//

ulong RollImage::getLastMusicHoleEnd(void) {
	return lastMusicRow;
}



//////////////////////////////
//
// RollImage::markHoleShifts --
//

void RollImage::markHoleShifts(void) {
	int counter;
	for (ulong i=0; i<holes.size(); i++) {
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
		ulong r = holes[i]->entry.first;
		ulong c = holes[i]->entry.second;
		int target = pixelType[r][c];
		fillHoleSimple(r, c, target, PIX_HOLE_SHIFT, counter);
	}
}



//////////////////////////////
//
// RollImage::markHoleBBs --
//

void RollImage::markHoleBBs(void) {
	for (ulong i=0; i<holes.size(); i++) {
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
	long r, c;

	// Mark upper side of box (leading edge)
	r = hi.origin.first - 1;
	for (c=-1; c<(long)hi.width.second+1; c++) {
		pixelType[r][c + (long)hi.origin.second] = PIX_HOLEBB_LEADING;
	}

	// Mark lower side of box (trailing edge
	r = hi.origin.first + hi.width.first + 1;
	for (c=-1; c<(long)hi.width.second+1; c++) {
		pixelType[r][c + (long)hi.origin.second] = PIX_HOLEBB_TRAILING;
	}

	// Mark left side of box:
	c = hi.origin.second - 1;
	for (r=-1; r<(long)hi.width.first+1; r++) {
		pixelType[r + (long)hi.origin.first][c] = PIX_HOLEBB_BASS;
	}

	// Mark right side of box:
	c = hi.origin.second + hi.width.second + 1;
	for (r=-1; r<(long)hi.width.first+1; r++) {
		pixelType[r + (long)hi.origin.first][c] = PIX_HOLEBB_TREBLE;
	}
}



//////////////////////////////
//
// RollImage::markHoleAttacks --
//

void RollImage::markHoleAttacks(void) {
	for (ulong i=0; i<holes.size(); i++) {
		if (holes[i]->isMusicHole()) {
			markHoleAttack(*holes[i]);
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
			pixelType[r][c] = PIX_HOLEBB_LEADING;
		}
	}
}



//////////////////////////////
//
// RollImage::getSoftMarginLeftWidth -- return the distance from the
//   hard margin to the left roll edge.
//

int RollImage::getSoftMarginLeftWidth(ulong rowindex) {
	if (leftMarginIndex[rowindex] > getHardMarginLeftIndex()) {
		return (ulong)leftMarginIndex[rowindex] - getHardMarginLeftIndex();
	} else {
		return 0;
	}
}



//////////////////////////////
//
// RollImage::getSoftMarginRightWidth -- return the distance from the
//   hard margin to the left roll edge.
//

int RollImage::getSoftMarginRightWidth(ulong rowindex) {
	if (rightMarginIndex[rowindex] < getHardMarginRightIndex()) {
		return getHardMarginRightIndex() - (ulong)rightMarginIndex[rowindex];
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
	ulong startrow = getFirstMusicHoleStart();
	ulong endrow = getLastMusicHoleEnd();
	for (ulong r=startrow; r<=endrow; r++) {
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
	ulong max = 0;
	ulong startrow = getFirstMusicHoleStart();
	ulong endrow = getLastMusicHoleEnd();
	for (ulong r=startrow; r<=endrow; r++) {
		ulong value = getHardMarginRightIndex() - (ulong)rightMarginIndex[r];
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
	ulong startrow = getFirstMusicHoleStart();
	ulong endrow = getLastMusicHoleEnd();
	ulong count = endrow - startrow + 1;

	// int cols = getCols();
	double sum = 0.0;
	for (ulong r=startrow; r<=endrow; r++) {
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

ulong RollImage::getLeftMarginWidth(ulong rowindex) {
	return leftMarginIndex[rowindex];
}



//////////////////////////////
//
// RollImage::getRightMarginWidth --
//

ulong RollImage::getRightMarginWidth(ulong rowindex) {
	return getCols() - rightMarginIndex[rowindex];
}



//////////////////////////////
//
// RollImage::getAverageMusicalHoleWidth --
//

double RollImage::getAverageMusicalHoleWidth(void) {
	if (holes.size() == 0) {
		return 0;
	}
	double sum = 0.0;
	for (ulong i=0; i<holes.size(); i++) {
		sum += holes[i]->width.second;
	}
	return sum / holes.size();
}


//////////////////////////////
//
// RollImage::getAverageSoftMarginTotal -- Only checking within musical region.
//

double RollImage::getAverageSoftMarginTotal(void) {
	ulong startrow = getFirstMusicHoleStart();
	ulong endrow = getLastMusicHoleEnd();
	ulong count = endrow - startrow + 1;

	double sum = 0.0;
	for (ulong r=startrow; r<=endrow; r++) {
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

void RollImage::markTrackerPositions(void) {

	ulong midiStart = 255;
	for (ulong i=0; i<256; i++) {
		if (midiToHoleMapping[i]) {
			midiStart = i;
			break;
		}
	}

	ulong midiEnd = 0;
	for (long i=255; i>=0; i--) {
		if (midiToHoleMapping[i]) {
			midiEnd = (ulong)i;
			break;
		}
	}

	ulong colstart = midiToHoleMapping[midiStart];
	ulong colend   = midiToHoleMapping[midiEnd];

	ulong startrow = getFirstMusicHoleStart() - 100;
	ulong endrow   = getLastMusicHoleEnd() + 100;

	ulong cutoff = midiToHoleMapping[65];

	ulong c;
	for (ulong r=startrow; r<=endrow; r++) {
		for (ulong i=colstart; i<=colend; i++) {
		std::vector<int> midiToHoleMapping;
			c = int(m_normalizedPosition[i] - driftCorrection[r] + 0.5);
			if (!trackerArray[i].empty()) {
				if (i < cutoff) {
					pixelType[r][c] = PIX_TRACKER_BASS;
				} else {
					pixelType[r][c] = PIX_TRACKER_TREBLE;
				}
			} else if (r % 20 < 10) {
				// dashed line to indiate no activity in track
				if (i < cutoff) {
					pixelType[r][c] = PIX_TRACKER_BASS;
				} else {
					pixelType[r][c] = PIX_TRACKER_TREBLE;
				}
			}
		}
	}
}



//////////////////////////////
//
// RollImage::drawMajorAxes --
//

void RollImage::drawMajorAxes(void) {
	for (ulong i=0; i<holes.size(); i++) {
		drawMajorAxis(*holes[i]);
	}
}



//////////////////////////////
//
// RollImage::drawMajorAxis --
//

void RollImage::drawMajorAxis(HoleInfo& hi) {
	ulong startrow = hi.origin.first;
	ulong endrow   = hi.origin.first + hi.width.first;
	double centerr = hi.centroid.first;
	double centerc = hi.centroid.second;
	double angle = hi.majoraxis * M_PI / 180.0;
	double cosangle = cos(angle);
	double side1, side2;
	double hypot;
	ulong c;
	for (ulong r=startrow; r<=endrow; r++) {
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

	ulong counter = 0;
	ulong startcol = 0;
	ulong endcol = hardMarginLeftIndex;
	ulong startrow = getFirstMusicHoleStart();
	ulong endrow   = getLastMusicHoleEnd();

	for (ulong r=startrow; r<=endrow; r++) {
		for (ulong c=startcol; c<=endcol; c++) {
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

	ulong counter = 0;
	ulong startcol = hardMarginRightIndex;
	ulong endcol = getCols() - 1;
	ulong startrow = getFirstMusicHoleStart();
	ulong endrow   = getLastMusicHoleEnd();

	for (ulong r=startrow; r<=endrow; r++) {
		for (ulong c=startcol; c<=endcol; c++) {
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
}



//////////////////////////////
//
// RollImage::sortTearsByArea -- Sort tears by area from largest to smallest.
//

void RollImage::sortTearsByArea(void) {
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
}



//////////////////////////////
//
// RollImage::getTrackerHoleCount -- Simple algorithm for now
//

int RollImage::getTrackerHoleCount(void) {
	int counter = 0;
	for (ulong i=0; i<midiToHoleMapping.size(); i++) {
		if (midiToHoleMapping[i]) {
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
	for (ulong i=0; i<holes.size(); i++) {
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

	double leftCol = m_firstHolePosition - driftCorrection[getFirstMusicHoleStart()];
	leftCol = leftCol - leftMarginIndex[getFirstMusicHoleStart()];

	double rightCol = m_lastHolePosition - driftCorrection[getFirstMusicHoleStart()];
	rightCol = rightMarginIndex[getFirstMusicHoleStart()] - rightCol;

	int trackerholes = getTrackerHoleCount();

	out << "@@BEGIN: ROLLINFO\n";
	out << "@IMAGE_WIDTH:\t\t"       << getCols()                 << "px\n";
	out << "@IMAGE_LENGTH:\t\t"      << getRows()                 << "px\n";
	out << "@ROLL_WIDTH:\t\t"        << averageRollWidth          << "px\n";
	out << "@HARD_MARGIN_BASS:\t"    << getHardMarginLeftWidth()  << "px\n";
	out << "@HARD_MARGIN_TREBLE:\t"  << getHardMarginRightWidth() << "px\n";
	out << "@MAX_BASS_DRIFT_MAX:\t"  << getSoftMarginLeftWidthMax()  << "px\n";
	out << "@MAX_RIGHT_DRIFT_MAX:\t" << getSoftMarginRightWidthMax() << "px\n";
	out << "@AVG_SOFT_MARGIN_SUM:\t" << averageSoftMarginWidth    << "px\n";
	out << "@PRELEADER_ROW:\t\t"     << getPreleaderIndex()       << "px\n";
	out << "@LEADER_ROW:\t\t"        << getLeaderIndex()          << "px\n";
	out << "@FIRST_HOLE:\t\t"        << getFirstMusicHoleStart()  << "px\n";
	out << "@LAST_HOLE:\t\t"         << getLastMusicHoleEnd()     << "px\n";
	out << "@END_MARGIN:\t\t"        << getRows() - getLastMusicHoleEnd() - 1 << "px\n";
	out << "@MUSICAL_LENGTH:\t"      << musiclength               << "px\n";
	out << "@MUSICAL_HOLES:\t\t"     << holes.size()              << "\n";
	out << "@AVG_HOLE_WIDTH:\t"      << avgholewidth              << "px\n";
	out << "@ANTIDUST_COUNT:\t"      << antidust.size()           << "\n";
	out << "@BAD_HOLE_COUNT:\t"      << badHoles.size()           << "\n";
	out << "@EDGE_TEAR_COUNT:\t"     << trebleTears.size() + bassTears.size() << "\n";
	out << "@BASS_TEAR_COUNT:\t"     << bassTears.size()          << "\n";
	out << "@TREBLE_TEAR_COUNT:\t"   << trebleTears.size()        << "\n";
	out << "@DUST_SCORE:\t\t"        << getDustScore()            << "ppm\n";
	out << "@DUST_SCORE_BASS:\t"     << getDustScoreBass()        << "ppm\n";
	out << "@DUST_SCORE_TREBLE:\t"   << getDustScoreTreble()      << "ppm\n";
	out << "@HOLE_SEPARATION:\t"     << holeSeparation            << "px\n";
	out << "@TRACKER_HOLES:\t\t"     << trackerholes              << " (estimate)\n";
	out << "@SOFTWARE_DATE:\t\t"     << __DATE__ << " " << __TIME__ << endl;
	out << "@COLOR_CHANNEL:\t\t"     << "green"                   << endl;
	out << "@CHANNEL_MD5:\t\t"       << getDataMD5Sum()           << endl;
	//out << "@BASS_TRACK_MARGIN:\t"    << leftCol                << "px\n";
	//out << "@TREBLE_TRACK_MARGIN:\t"  << rightCol               << "px\n";

	// DRIFT FUNCTION /////////////////////////////////////////////////////
	out << "\n\n";
	out << "@@ The Drift data describes the left/right shifting of the roll along the\n";
	out << "@@ length of the image.  Each data point is a list values:\n";
	out << "@@    (1) The row number in the image\n";
	out << "@@    (2) The distance in feet from the start of the roll (not the image)\n";
	out << "@@        to the given row.\n";
	out << "@@    (3) The correction to apply to the row to straighten the roll.\n";
	out << "@@ A new data point is given whenever the drift changes by more than 0.1 pixels.\n";
	out << "@@ Drift analysis starts at the first music hole on the roll.\n";
	out << "\n";
	out << "@@BEGIN: DRIFT\n";
	ulong fff = getPreleaderIndex();
	// ulong fff = getFirstMusicHoleStart();
	double lastdrift = -1.0;
	double drift;
	for (ulong i=getFirstMusicHoleStart(); i<getLastMusicHoleEnd(); i++) {
		drift = int(driftCorrection[i]*10.0+0.5)/10.0;
		if (drift == lastdrift) {
			continue;
		}
		lastdrift = drift;
		out << "\t" << i
		    << "\t" << int((i - fff)/300.25/12*10000.0+0.5)/10000.0
		    << "\t" << drift << "\n";
	}
	out << "@@END: DRIFT\n";

	// List of musical holes //////////////////////////////////////////////
	out << "\n\n";
	out << "@@ The HOLES array contains a list of all musical holes (note and expression holes).\n";
	out << "@@ They are sorted in time (or image row) from the start of the roll to the end.\n";
	out << "@@\n";
	out << "@@ Meaning of HOLE parameters:\n";
	out << "@@\n";
	out << "@@   ORIGIN_ROW:\tThe pixel row of the leading edge of the bounding box around the hole.\n";
	out << "@@   ORIGIN_COL:\tThe pixel column of the leading edge of the bounding box around the hole, bass side.\n";
	out << "@@   WIDTH_ROW:\t\tThe pixel length of the bounding box around the hole.\n";
	out << "@@   WIDTH_COL:\t\tThe pixel column of the leading edge of the hole, bass side.\n";
	out << "@@   CENTROID_ROW:\tThe center of mass row of the hole.\n";
	out << "@@   CENTROID_COL:\tThe center of mass column of the hole.\n";
	out << "@@   AREA:\t\tThe area of the hole (in pixels).\n";
	out << "@@   PERIMETER:\t\tThe perimeter of the hole (in pixels).\n";
	out << "@@   CIRCULARITY:\tThe circularity of the hole (1 = circular, 0 = very uncircular).\n";
	out << "@@   MAJOR_AXIS:\tThe major axis of the hole in degrees (0 = vertically aligned with roll).\n";
	out << "@@   HPIXCOR:\tHorizontal (column) pixel correction: shift to straighten hole columns on paper.\n";
	out << "@@      If the leading and trailing edges of musical holes do not have the same correction value,\n";
	out << "@@      then the following two parameters are given instead:\n";
	out << "@@         HPIXCOR_LEAD:\tHorizontal pixel correction of the hole's leading edge.\n";
	out << "@@         HPIXCOR_TRAIL:\tHorizontal pixel correction of the hole's trailing edge.\n";
	out << "@@\n";
	out << "\n";

	out << "@@BEGIN: HOLES\n\n";
	for (ulong i=0; i<holes.size(); i++) {
		if (holes[i]->isMusicHole()) {
			holes[i]->printAton(out);
			out << std::endl;
		}
	}
	out << "@@END: HOLES\n\n";


	// List of bad holes //////////////////////////////////////////////
	if (!badHoles.empty()) {
		sortBadHolesByArea();
		for (ulong i=0; i<badHoles.size(); i++) {
			string id = "bad";
			if (i+1 < 100) { id += "0"; }
			if (i+1 < 10 ) { id += "0"; }
			id += to_string(i+1);
			badHoles[i]->id = id;
		}
		out << "\n\n";
		out << "@@BEGIN: BADHOLES\n\n";
		for (ulong i=0; i<badHoles.size(); i++) {
			badHoles[i]->printAton(out);
			out << "\n";
		}
		out << "@@END: BADHOLES\n\n";
	}


	// List of significant edge tears /////////////////////////////////
	if (bassTears.size() + trebleTears.size() > 0) {
		sortTearsByArea();
		out << "\n@@BEGIN: TEARS\n";
		if (trebleTears.size() > 0) {
			for (ulong i=0; i<trebleTears.size(); i++) {
				string id = "trebletear";
				if (i+1 < 100) { id += "0"; }
				if (i+1 < 10 ) { id += "0"; }
				id += to_string(i+1);
				trebleTears[i]->id = id;
			}
			out << "@@BEGIN: TREBLE_TEARS\n";
			for (ulong i=0; i<trebleTears.size(); i++) {
				trebleTears[i]->printAton(out);
			}
			out << "@@END: TREBLE_TEARS\n";
		}
		if (bassTears.size() > 0) {
			for (ulong i=0; i<bassTears.size(); i++) {
				string id = "basstear";
				if (i+1 < 100) { id += "0"; }
				if (i+1 < 10 ) { id += "0"; }
				id += to_string(i+1);
				bassTears[i]->id = id;
			}
			out << "\n@@BEGIN: BASS_TEARS\n";
			for (ulong i=0; i<bassTears.size(); i++) {
				bassTears[i]->printAton(out);
			}
			out << "@@END: BASS_TEARS\n";
		}
		out << "@@END: TEARS\n";
	}


	out << "\n@@END: ROLLINFO\n";
	return out;
}



} // end of namespace prp




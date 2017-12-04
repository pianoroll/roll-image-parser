//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Dec  1 16:40:52 PST 2017
// Last Modified: Fri Dec  1 16:40:55 PST 2017
// Filename:      RollImage.cpp
// Web Address:
// Syntax:        C++;
// vim:           ts=3:nowrap:ft=text
//
// Description:   Piano-roll description parameters.
//


#include "RollImage.h"
#include "HoleInfo.h"

#include <algorithm>

using namespace std;

	

namespace tiff {


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
	analyzeBasicMargins();
	analyzeLeaders();
	analyzeAdvancedMargins();
	generateDriftCorrection();
	analyzeHoles();
	//for (ulong i=0; i<holes.size(); i++) {
	//	std::cerr << holes[i] << std::endl;
	//}
	markPosteriorLeader();
	analyzeTrackerBarSpacing();
	analyzeTrackerBarPositions();
	analyzeHorizontalHolePosition();
}



//////////////////////////////
//
// RollImage::analyzeHorizontalHolePosition -- assign holes to tracker hole
// positions.
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
		trackerArray.at(index).push_back(holes[i]);
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
		int position = holes[i]->centroid.second + 0.5;
		correctedCentroidHistogram[position]++;
	}
	//cerr << "Histogram:\n";
	//for (ulong i=0; i<correctedCentroidHistogram.size(); i++) {
	//	cerr << i << "\t" << correctedCentroidHistogram[i] << endl;
	//}

	std::vector<mycomplex> spectrum;
	int factor = 8;
	std::vector<mycomplex> input(4096 * factor);
	for (ulong i=0; i<4096; i++) {
		input.at(i).first = correctedCentroidHistogram.at(i);
		input.at(i).second = 0;
	}
	for (ulong i=4096; i<input.size(); i++) {
		input.at(i).first = 0.0;
		input.at(i).second = 0.0;
	}
	FFT(spectrum, input);

	vector<double> magnitudeSpectrum(spectrum.size());
	int maxmagi = 4;
	for (ulong i=0 ;i<spectrum.size(); i++) {
		magnitudeSpectrum.at(i) = magnitude(spectrum.at(i));
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

	double b = (y3 - y2)/2.0;
	double a = y1/2.0 - y2 + y3/2.0;
	double newi = -b / 2 / a / factor ;
	double estimate = 4096.0 * factor / maxmagi;

	//cerr << "PIXEL SEPARATION: " << estimate << " pixels\n";
	//cerr << "REFINED PIXEL SEPARATION: " << newi << " pixels\n";
	//cerr << "FINAL ANSWER: " << (estimate + newi) << " pixels\n";

	holeSeparation = estimate + newi;

	// cerr << "\n\nspectrum:\n";
	// for (ulong i=0; i<spectrum.size(); i++) {
	// 	cerr << spectrum[i].first << "\t" << spectrum[i].second << endl;
	// }

}



//////////////////////////////
//
// RollImage::generateDriftCorrection --
//

void RollImage::generateDriftCorrection(void) {

	ulong rows = getRows();
	std::vector<double> lmargin(rows);
	std::vector<double> rmargin(rows);

	for (ulong r=0; r<rows; r++) {
		lmargin[r] = leftMarginIndex[r];
		rmargin[r] = rightMarginIndex[r];
	}
	double k = 0.01;
	double nk = 1.0 - k;
	for (ulong r=1; r<rows; r++) {
		lmargin[r] = k * lmargin[r] + nk * lmargin[r-1];
		rmargin[r] = k * rmargin[r] + nk * rmargin[r-1];
	}
	for (int r=rows-2; r>=0; r--) {
		lmargin[r] = k * lmargin[r] + nk * lmargin[r+1];
		rmargin[r] = k * rmargin[r] + nk * rmargin[r+1];
	}

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

	ulong startcol = getHardMarginLeftIndex()+1;
	ulong endcol   = getHardMarginRightIndex();
	ulong startrow = getLeaderIndex();
	ulong endrow   = getRows();

	for (ulong r=startrow; r<endrow; r++) {
		for (ulong c=startcol; c<endcol; c++) {
			if (pixelType[r][c] == PIX_NONPAPER) {
				extractHole(r, c);
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
	fillHoleInfo(*hi, row, col);
	hi->centroid.first  /= hi->area;
	hi->centroid.second /= hi->area;

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
		antidust.push_back(hi);
	}
}



//////////////////////////////
//
// RollImage::fillHoleInfo --
//

void RollImage::fillHoleInfo(HoleInfo& hi, ulong r, ulong c) {
	if (pixelType[r][c] != PIX_NONPAPER) {
		return;
	} else {
		pixelType[r][c] = PIX_HOLE;
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
	hi.centroid.second += c + driftCorrection[r];

	fillHoleInfo(hi, r-1, c-1);
	fillHoleInfo(hi, r-1, c-0);
	fillHoleInfo(hi, r-1, c+1);
	fillHoleInfo(hi, r  , c-1);
	fillHoleInfo(hi, r  , c+1);
	fillHoleInfo(hi, r+1, c-1);
	fillHoleInfo(hi, r-1, c-1);
	fillHoleInfo(hi, r-1, c-1);
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
		for (ulong c=cols-1; c>=0; c--) {
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

				case PIX_HOLE:             // holes in roll (white)
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] = 255;
					break;

				case PIX_HOLEBB:           // hole bounding box (red)
					pixel[0] = 255;
					pixel[1] =   0;
					pixel[2] =   0;
					break;

				case PIX_TRACKER:           // hole for tracker position (green)
					pixel[0] =   0;
					pixel[1] = 255;
					pixel[2] =   0;
					break;

				case PIX_DEBUG:
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] = 255;
					break;

				case PIX_DEBUG2:
					pixel[0] = 255;
					pixel[1] = 0;
					pixel[2] = 127;
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

ulong RollImage::getHardMarginLeftIndex(void) {
	return hardMarginLeftIndex;
}



//////////////////////////////
//
// RollImage::getHardMarginRightIndex --
//

ulong RollImage::getHardMarginRightIndex(void) {
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
// RollImage::getLastMusicHoleStartEnd --
//

ulong RollImage::getLastMusicHoleEnd(void) {
	return lastMusicRow;
}



//////////////////////////////
//
// RollImage::markHoleBBs --
//

void RollImage::markHoleBBs(void) {
	for (ulong i=0; i<holes.size(); i++) {
		markHoleBB(*holes[i]);
	}
}



//////////////////////////////
//
// RollImage::markHoleBB --
//

void RollImage::markHoleBB(HoleInfo& hi) {
// cerr << "MARKING HOLE " << hi << endl;
	long r, c;

	// Mark upper side of box:
	r = hi.origin.first - 1;
	for (c=-1; c<(long)hi.width.second+1; c++) {
		pixelType[r][c + (long)hi.origin.second] = PIX_HOLEBB;
	}

	// Mark lower side of box:
	r = hi.origin.first + hi.width.first + 1;
	for (c=-1; c<(long)hi.width.second+1; c++) {
		pixelType[r][c + (long)hi.origin.second] = PIX_HOLEBB;
	}

	// Mark left side of box:
	c = hi.origin.second - 1;
	for (r=-1; r<(long)hi.width.first+1; r++) {
		pixelType[r + (long)hi.origin.first][c] = PIX_HOLEBB;
	}

	// Mark right side of box:
	c = hi.origin.second + hi.width.second + 1;
	for (r=-1; r<(long)hi.width.first+1; r++) {
		pixelType[r + (long)hi.origin.first][c] = PIX_HOLEBB;
	}
}



//////////////////////////////
//
// RollImage::getSoftMarginLeftWidth -- return the distance from the 
//   hard margin to the left roll edge.
//

ulong RollImage::getSoftMarginLeftWidth(ulong rowindex) { 
	if ((ulong)leftMarginIndex[rowindex] > getHardMarginLeftIndex()) {
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

ulong RollImage::getSoftMarginRightWidth(ulong rowindex) { 
	if ((ulong)rightMarginIndex[rowindex] < getHardMarginRightIndex()) {
		return getHardMarginRightIndex() - (ulong)rightMarginIndex[rowindex];
	} else {
		return 0;
	}
}



//////////////////////////////
//
// RollImage::getAverageRollWidth --  Only looks in the music region.
//

double RollImage::getAvergeRollWidth(void) {
	ulong startrow = getFirstMusicHoleStart();
	ulong endrow = getLastMusicHoleEnd();
	ulong count = endrow - startrow + 1;
	
	int cols = getCols();
	double sum = 0.0;
	for (ulong r=startrow; r<=endrow; r++) {
		sum += cols - getLeftMarginWidth(r) - getRightMarginWidth(r);
	}
	return sum / count;
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
		sum += getLeftMarginWidth(r) - getHardMarginLeftWidth();
		sum += getRightMarginWidth(r) - getHardMarginRightWidth();
	}
	return sum / count;
}



//////////////////////////////
//
// RollImage::markTrackerPositions --
//

void RollImage::markTrackerPositions(void) { 
	std::vector<double> position(trackerArray.size(), 0);
	for (ulong i=0; i<position.size(); i++) {
		position[i] = i * holeSeparation - holeOffset;
	}

	ulong colstart = 0;
	for (ulong i=0; i<trackerArray.size(); i++) {
		if (trackerArray[i].empty()) {
			continue;
		}
		colstart = i;
		m_firstHolePosition = position[i];
		break;
	}

	ulong colend = 0;
	for (ulong i=trackerArray.size()-1; i>=0; i--) {
		if (trackerArray[i].empty()) {
			continue;
		}
		colend = i;
		m_lastHolePosition = position[i];
		break;
	}


	ulong startrow = getFirstMusicHoleStart() - 100;
	ulong endrow = getLastMusicHoleEnd() + 100;
	ulong pos;

	for (ulong r=startrow; r<=endrow; r++) {
		for (ulong i=colstart; i<=colend; i++) {
			pos = int(position[i] - driftCorrection[r] + 0.5);
			if (!trackerArray[i].empty()) {
				pixelType[r][pos] = PIX_TRACKER;
			} else if (r % 20 < 10) {
				pixelType[r][pos] = PIX_TRACKER;
			}
		}
	}

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

	double averageRollWidth = getAvergeRollWidth();
	averageRollWidth = int(averageRollWidth*100.0+0.5)/100.0;
	double averageSoftMarginWidth = getAverageSoftMarginTotal();
	averageSoftMarginWidth = int(averageSoftMarginWidth*100.0+0.5)/100.0;
	
	int musiclength = getLastMusicHoleEnd() - getFirstMusicHoleStart();

	double avgholewidth = int(getAverageMusicalHoleWidth()*100.0+0.5)/100.0;

	double leftCol = m_firstHolePosition - driftCorrection[getFirstMusicHoleStart()];
	leftCol = leftCol - leftMarginIndex[getFirstMusicHoleStart()];

	double rightCol = m_lastHolePosition - driftCorrection[getFirstMusicHoleStart()];
	rightCol = rightMarginIndex[getFirstMusicHoleStart()] - rightCol;

	out << "@@BEGIN: ROLLINFO\n";
	out << "@IMAGE_WIDTH:\t\t"       << getCols()                 << "px\n";
	out << "@IMAGE_LENGTH:\t\t"      << getRows()                 << "px\n";
	out << "@ROLL_WIDTH:\t\t"        << averageRollWidth          << "px\n";
   out << "@HARD_MARGIN_LEFT:\t"    << getHardMarginLeftWidth()  << "px\n";
   out << "@HARD_MARGIN_RIGHT:\t"   << getHardMarginRightWidth() << "px\n";
   out << "@AVG_SOFT_MARIGN_SUM:\t" << averageSoftMarginWidth    << "px\n";
   //out << "@SOFT_MARGIN_LEFT:\t"  << getSoftMarginLeftWidth()  << "px\n";
   //out << "@SOFT_MARGIN_RIGHT:\t" << getSoftMarginRightWidth() << "px\n";
	out << "@PRELEADER_ROW:\t\t"     << getPreleaderIndex()       << "px\n";
	out << "@LEADER_ROW:\t\t"        << getLeaderIndex()          << "px\n";
	out << "@FIRST_HOLE:\t\t"        << getFirstMusicHoleStart()  << "px\n";
	out << "@LAST_HOLE:\t\t"         << getLastMusicHoleEnd()     << "px\n";
	out << "@END_MARGIN:\t\t"        << getRows() - getLastMusicHoleEnd() - 1 << "px\n";
	out << "@MUSICAL_LENGTH:\t"      << musiclength               << "px\n";
	out << "@MUSICAL_HOLES:\t\t"     << holes.size()              << "\n";
	out << "@AVG_HOLE_WIDTH:\t"      << avgholewidth              << "px\n";
	out << "@ANTIDUST:\t\t"          << antidust.size()           << "\n";
	out << "@HOLE_SEPARATION:\t"     << holeSeparation            << "px\n";
	//out << "@LEFT_TRACK_MARGIN:\t"   << leftCol                   << "px\n";
	//out << "@RIGHT_TRACK_MARGIN:\t"  << rightCol                  << "px\n";

	out << "\n@@BEGIN: DRIFT\n";
	ulong fff = getFirstMusicHoleStart();
	for (ulong i=getFirstMusicHoleStart(); i<getLastMusicHoleEnd(); i++) {
		out << "\t" <<  int((i - fff)/300.25/12*10000.0+0.5)/10000.0  
		    << "\t" << int(driftCorrection[i]*100.0+0.5)/100.0 << "\n";
	}
	out << "@@END: DRIFT\n";

	out << "\n@@BEGIN: HOLES\n";
	for (ulong i=0; i<holes.size(); i++) {
		out << "\t" << holes[i] << "\n";
	}
	out << "@@END: HOLES\n\n";

	out << "@@END: ROLLINFO\n";
	return out;
}



} // end of namespace tiff




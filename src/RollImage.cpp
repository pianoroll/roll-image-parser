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

#include <algorithm>


namespace tiff {


//////////////////////////////
//
// RollImage::RollImage --
//

RollImage::RollImage(void) {

}



//////////////////////////////
//
// RollImage::~RollImage --
//

RollImage::~RollImage(void) {
	close();
}



//////////////////////////////
//
// RollImage::loadGreenChannel --
//

void RollImage::loadGreenChannel(void) {
	ulong rows = getRows();
	ulong cols = getCols();
	this->getImageGreenChannel(green);
	pixelType.resize(rows);
	for (ulong r=0; r<rows; r++) {
		pixelType[r].resize(getCols());
		for (ulong c=0; c<cols; c++) {
			if (aboveThreshold(green[r][c], 255)) {
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
	analyzeMargins();
	analyzeLeader();
}



//////////////////////////////
//
// RollImage::analyzeMargins --
//

void RollImage::analyzeMargins(void) {
	getRawMargins();
	waterfallDownMargins();

	m_analyzedMargins = true;
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

	leftMargin.resize(rows);
	rightMargin.resize(rows);

	for (ulong r=0; r<rows; r++) {
		std::vector<uchar>& rowdata = pixelType.at(r);
		leftMargin[r] = 0;
		for (ulong c=0; c<cols; c++) {
			if (rowdata.at(c) == PIX_PAPER) {
				leftMargin[r] = c - 1;
				break;
			} else {
				rowdata.at(c) = PIX_MARGIN;
				leftMargin[r] = c;
			}
		}
	}

	for (ulong r=0; r<rows; r++) {
		std::vector<uchar>& rowdata = pixelType.at(r);
		rightMargin[r] = 0;
		for (ulong c=cols-1; c>=0; c--) {
			if (rowdata.at(c) == PIX_PAPER) {
				rightMargin[r] = c + 1;
				break;
			} else {
				rowdata.at(c) = PIX_MARGIN;
				rightMargin[r] = c;
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
					if (c > (ulong)leftMargin[r+1]) {
						leftMargin[r+1] = c;
					}
				} else {	
					if (c < (ulong)rightMargin[r+1]) {
						rightMargin[r+1] = c;
					}
				}
			}
		}
	}
}



//////////////////////////////
//
// RollImage::analyzeLeader --
//

void RollImage::analyzeLeader(void) {
	if (!m_analyzedMargins) {
		analyzeMargins();
	}
	ulong cols = getCols();
	ulong rows = getRows();

	double topLeftAvg  = getAverage(leftMargin, 0, cols);
	double topRightAvg = getAverage(rightMargin, 0, cols);
	double botLeftAvg  = getAverage(leftMargin, rows-1-4096, cols);
	double botRightAvg = getAverage(rightMargin, rows-1-4096, cols);

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
	leftLeaderBoundary = findLeftLeaderBoundary(leftMargin, botLeftAvg, cols, 4096*4);

	ulong rightLeaderBoundary = 0;
	rightLeaderBoundary = findRightLeaderBoundary(rightMargin, botRightAvg, cols, 4096*4);

	ulong leaderBoundary = (leftLeaderBoundary + rightLeaderBoundary) / 2;
	markLeaderRegion(leaderBoundary);

	// find pre-leader region
	ulong preleader = getPreleaderIndex(leaderBoundary);
	markPreleaderRegion(preleader);
	markHardMargin(leaderBoundary);
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
				case PIX_NONPAPER:            // undifferentited non-paper (green)
					pixel[0] = 0;
					pixel[1] = 255;
					pixel[2] = 0;
					break;

				case PIX_MARGIN:				// paper margins (blue)
					pixel[0] = 0;
					pixel[1] = 0;
					pixel[2] = 255;
					break;

				case PIX_HARDMARGIN:			// paper margins with not paper in rect.
					pixel[0] = 0;
					pixel[1] = 64;
					pixel[2] = 255;
					break;

				case PIX_LEADER:				// leader region (cyan)
					pixel[0] = 0;
					pixel[1] = 255;
					pixel[2] = 255;
					break;

				case PIX_PRELEADER:			// leader region (light blue)
					pixel[0] = 0;
					pixel[1] = 128;
					pixel[2] = 255;
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
// RollImage::getPreleaderIndex -- Return the row in the image where the preleader
//    ends and the leader starts.
//

ulong RollImage::getPreleaderIndex(ulong leaderBoundary) {

	std::vector<ulong> marginsum;
	ulong tolerance = 20;       // How much the positions adjacent to the min
                               // can grow, yet still be called the preleader.
	ulong startboundary = 10;   // Avoid the very start of the image
                               // in case there are scanning artifacts.
	ulong cols = getCols();

	marginsum.resize(leaderBoundary);
	fill(marginsum.begin(), marginsum.end(), 0);
	for (ulong i=startboundary; i<marginsum.size(); i++) {
		marginsum[i] = leftMargin[i] + cols - rightMargin[i];
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
// RollImage:markHardMargin -- Mark the rectangular area in the margin where no paper is
//     found.  Dust is expected to be already filtered out of the margin data.
//

void RollImage::markHardMargin(ulong leaderBoundary) {

	ulong endboundary = 1000;

	ulong minpos = leftMargin[leaderBoundary];
	ulong rows = pixelType.size();
	//ulong minrow = 0;
	for (ulong r=leaderBoundary+1; r<rows-endboundary; r++) {
		if ((ulong)leftMargin[r] < minpos) {
			minpos = leftMargin[r];
			//minrow = r;
		}
	}

	//std::cerr << "ROW OF HARD MARGIN LIMIT IS " << minrow << " at " << int(minrow/(double)rows*100) << "%\n";
	//for (ulong c=0; c<pixelType[minrow].size(); c++) {
	//	pixelType[minrow+2][c] = PIX_DEBUG;
	//	pixelType[minrow-2][c] = PIX_DEBUG;
	//}

	for (ulong r=leaderBoundary; r<rows; r++) {
		for (ulong c=0; c<=minpos; c++) {
			if (pixelType[r][c] == PIX_MARGIN) {
				pixelType[r][c] = PIX_HARDMARGIN;
			}
		}
	}

	ulong maxpos = rightMargin[leaderBoundary];
	for (ulong r=leaderBoundary+1; r<rows; r++) {
		if ((ulong)rightMargin[r] > maxpos) {
			maxpos = rightMargin[r];
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

}



//////////////////////////////
//
// RollImage::markPreleaderRegion --
//

void RollImage::markPreleaderRegion(ulong preleaderBoundary) {
	ulong cols = getCols();

	// mark holes in leader region as leader holes.

	for (ulong r=0; r<preleaderBoundary; r++) {
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

void RollImage::markLeaderRegion(ulong leaderBoundary) {
	ulong cols = getCols();

	// mark holes in leader region as leader holes.

	for (ulong r=0; r<leaderBoundary; r++) {
		for (ulong c=0; c<cols; c++) {
			if (pixelType[r][c]) {
				pixelType[r][c] = PIX_LEADER;
			}
		}
	}
}



} // end of namespace tiff




//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Dec  1 16:44:12 PST 2017
// Last Modified: Fri Dec  1 16:44:18 PST 2017
// Filename:      RollImage.h
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Piano-roll information structure
//

#ifndef _ROLLIMAGE_H
#define _ROLLIMAGE_H

#include <vector>
#include <fstream>
#include <iostream>

#include "TiffFile.h"
#include "HoleInfo.h"

namespace tiff  {


// Pixel functions:
#define PIX_PAPER       0    /* pixel represents paper                           */
#define PIX_NONPAPER    1    /* pixel represents uncategorized non-paper         */
#define PIX_MARGIN      2    /* pixel represents soft margins                    */
#define PIX_LEADER      3    /* pixel represents non-paper in leader region      */
#define PIX_PRELEADER   4    /* pixel represents non-paper in pre-leader region  */
#define PIX_POSTLEADER  5    /* pixel represents non-paper in post-leader region */
#define PIX_HARDMARGIN  6    /* pixel represents hard-margin                     */
#define PIX_HOLE        7    /* pixel represents hole in roll                    */
#define PIX_HOLEBB      8    /* pixel represents bounding box around hole        */
#define PIX_TRACKER     9    /* pixel represents bounding box around hole        */
#define PIX_POSTMUSIC  10    /* pixel represents after last hole in roll         */
#define PIX_DEBUG      11    /* debugging pixel type 1                           */
#define PIX_DEBUG2     12    /* debugging pixel type 2                           */

typedef unsigned char pixtype;

class RollImage : public TiffFile {
	public:
		                 RollImage                   (void);
		                ~RollImage                   ();

		void	          loadGreenChannel             (void);
		void            analyze                      (void);
		void            analyzeHoles                 (void);
		void            mergePixelOverlay            (std::fstream& output);
		void            markHoleBBs                  (void);
		std::ostream&   printRollImageProperties     (std::ostream& out = std::cout);
		int             getHardMarginLeftWidth       (void);
		int             getHardMarginRightWidth      (void);
		ulong           getHardMarginLeftIndex       (void);
		ulong           getHardMarginRightIndex      (void);
		ulong           getLeaderIndex               (void);
		ulong           getPreLeaderIndex            (void);
		ulong           getPreleaderIndex            (void);
		ulong           getFirstMusicHoleStart       (void);
		ulong           getLastMusicHoleEnd          (void);
		ulong           getSoftMarginLeftWidth       (ulong rowindex);
		ulong           getSoftMarginRightWidth      (ulong rowindex);
		double          getAvergeRollWidth           (void);
		double          getAverageMusicalHoleWidth   (void);
		ulong           getLeftMarginWidth           (ulong rowindex);
		ulong           getRightMarginWidth          (ulong rowindex);
		double          getAverageSoftMarginTotal    (void);
		void            generateDriftCorrection      (void);
		void            analyzeTrackerBarSpacing     (void);
		void            analyzeTrackerBarPositions   (void);
		void            analyzeHorizontalHolePosition(void);
		void            markTrackerPositions         (void);

		// pixelType: a bitmask which contains enumerated types for the
		// functions of pixels (the PIX_* defines above):
		std::vector<std::vector<pixtype>> pixelType;

		// monochrome: a monochrome version of the roll image (typically
		// the green channel):
		std::vector<std::vector<uchar>>   monochrome;

		// leftMarginIndex: The row-by-row margin to the left roll edge:
		std::vector<int>                  leftMarginIndex;

		// rightMarginIndex: The row-by-row margin to the right roll edge:
		std::vector<int>                  rightMarginIndex;

		// driftCorrection: column adjustment for each row in music area
		std::vector<double> driftCorrection;

		// correctedCentroidHistogram: count of centroids at each pixel.
		std::vector<int> correctedCentroidHistogram;

		// holeSeparation -- number of pixels between hole centers.
		double holeSeparation = 0.0;

		// holeOffset -- Pixel offset for holeSeparation on normalized width.
		double holeOffset = 0.0;

		// holes: List of musical holes on the piano roll (or at least
		// larger holes which are not anti-dust
		std::vector<HoleInfo*> holes;

		// trackerArray -- holes sorte by tracker position
		std::vector<std::vector<HoleInfo*>> trackerArray;
	
		// antidust: List of holes on roll which are too small to be musical.
		std::vector<HoleInfo*> antidust;

	protected:
		void       analyzeBasicMargins         (void);
		void       analyzeAdvancedMargins      (void);
		void       analyzeLeaders              (void);
		void       getRawMargins               (void);
		void       waterfallDownMargins        (void);
		ulong      findLeftLeaderBoundary      (std::vector<int>& margin, double avg,
		                                        ulong cols, ulong searchlength);
		ulong      findRightLeaderBoundary     (std::vector<int>& margin, double avg,
		                                        ulong cols, ulong searchlength);
		ulong      getBoundary                 (std::vector<int>& status);
		void       markHardMargin              (ulong leaderBoundary);
		void       markPreleaderRegion         (void);
		void       markLeaderRegion            (void);
		void       setHardMarginLeftIndex      (ulong index);
		void       setHardMarginRightIndex     (ulong index);
		ulong      extractPreleaderIndex       (ulong leaderBoundary);
		void       setPreleaderIndex           (ulong value);
		void       setLeaderIndex              (ulong value);
		void       analyzeHardMargins          (ulong leaderBoundary);
		void       fillHoleInfo                (HoleInfo& hi, ulong r, ulong c);
		void       extractHole                 (ulong row, ulong col);
		void       markPosteriorLeader         (void);
		void       markHoleBB                  (HoleInfo& hi);
		double     getTrackerShiftScore        (double shift);

	private:
		bool			m_analyzedBasicMargins    = false;
		bool			m_analyzedLeaders         = false;
		bool			m_analyzedAdvancedMargins = false;
		int         hardMarginLeftIndex       = 0;
		int         hardMarginRightIndex      = 0;
		ulong       preleaderIndex            = 0;
		ulong       leaderIndex               = 0;
		ulong       firstMusicRow             = 0;
		ulong       lastMusicRow              = 0;
		double      m_lastHolePosition        = 0.0;
		double      m_firstHolePosition       = 0.0;

};

} // end tiff namespace

#endif /* _ROLLIMAGE_H */


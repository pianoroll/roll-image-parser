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
#include <utility>
#include <string>
#include <ctime>

#ifndef DONOTUSEFFT
   #include <chrono>
#endif

#include "TiffFile.h"
#include "HoleInfo.h"
#include "ShiftInfo.h"
#include "TearInfo.h"
#include "RollOptions.h"
#include "MidiFile.h"

namespace prp  {


// Track function:
#define TRACK_UNKNOWN          0    /* unassigned meaning */
#define TRACK_SNAKEBITE        1    /* Melodic accending row */

// Pixel functions:
#define PIX_PAPER              0    /* pixel represents paper                           */
#define PIX_NONPAPER           1    /* pixel represents uncategorized non-paper         */
#define PIX_MARGIN             2    /* pixel represents soft margins                    */
#define PIX_LEADER             3    /* pixel represents non-paper in leader region      */
#define PIX_PRELEADER          4    /* pixel represents non-paper in pre-leader region  */
#define PIX_POSTLEADER         5    /* pixel represents non-paper in post-leader region */
#define PIX_HARDMARGIN         6    /* pixel represents hard-margin                     */
#define PIX_TEAR               7    /* pixel represents uncategorized non-paper         */
#define PIX_ANTIDUST           8    /* pixel represents non-musical hole in paper       */
#define PIX_HOLE               9    /* pixel represents music hole in roll              */
#define PIX_HOLE_SNAKEBITE    10    /* pixel represents snakebite                       */
#define PIX_HOLE_SHIFT        11    /* music hole which shifts left/right               */
#define PIX_BADHOLE           12    /* pixel represents non-music hole in roll          */
#define PIX_BADHOLE_SKEWED    13    /* pixel represents hole with strange skew          */
#define PIX_BADHOLE_ASPECT    14    /* pixel represents hole with strange aspect ratio  */
#define PIX_HOLEBB            15    /* pixel represents bounding box around music hole  */
#define PIX_HOLEBB_LEADING_A  16    /* bounding box leading edge                        */
#define PIX_HOLEBB_LEADING_S  17    /* bounding box leading edge                        */
#define PIX_HOLEBB_TRAILING   18    /* bounding box trailing edge                       */
#define PIX_HOLEBB_BASS       19    /* bounding box bass-face edge                      */
#define PIX_HOLEBB_TREBLE     20    /* bounding box treble-facing                       */
#define PIX_TRACKER           21    /* pixel is center of a tracker hole                */
#define PIX_TRACKER_BASS      22    /* pixel is center of bass tracker hole             */
#define PIX_TRACKER_TREBLE    23    /* pixel is center of treble tracker hole           */
#define PIX_POSTMUSIC         24    /* pixel represents after last hole in roll         */
#define PIX_DEBUG             25    /* debugging pixel type white                       */
#define PIX_DEBUG1            26    /* debugging pixel type 1 red                       */
#define PIX_DEBUG2            27    /* debugging pixel type 2 orange                    */
#define PIX_DEBUG3            28    /* debugging pixel type 3 yellow                    */
#define PIX_DEBUG4            29    /* debugging pixel type 4 green                     */
#define PIX_DEBUG5            30    /* debugging pixel type 5 light blue                */
#define PIX_DEBUG6            31    /* debugging pixel type 6 dark blue                 */
#define PIX_DEBUG7            32    /* debugging pixel type 7 purple                    */


typedef unsigned char pixtype;

class RollImage : public TiffFile, public RollOptions {
	public:
		                 RollImage                    (void);
		                ~RollImage                    ();

		void	          loadGreenChannel              (int threshold);
		void            analyze                       (void);
		void            analyzeHoles                  (void);
		void            mergePixelOverlay             (std::fstream& output);
		void            markHoleBBs                   (void);
		std::ostream&   printRollImageProperties      (std::ostream& out = std::cout);
		std::ostream&   printQualityReport            (std::ostream& out = std::cerr);
		int             getHardMarginLeftWidth        (void);
		int             getHardMarginRightWidth       (void);
		int             getHardMarginLeftIndex        (void);
		int             getHardMarginRightIndex       (void);
		ulongint        getLeaderIndex                (void);
		ulongint        getPreLeaderIndex             (void);
		ulongint        getPreleaderIndex             (void);
		ulongint        getFirstMusicHoleStart        (void);
		ulongint        getLastMusicHoleEnd           (void);
		int             getSoftMarginLeftWidth        (ulongint rowindex);
		int             getSoftMarginRightWidth       (ulongint rowindex);
		int             getSoftMarginLeftWidthMax     (void);
		int             getSoftMarginRightWidthMax    (void);
		double          getAverageRollWidth           (void);
		double          getAverageMusicalHoleWidth    (void);
		ulongint        getLeftMarginWidth            (ulongint rowindex);
		ulongint        getRightMarginWidth           (ulongint rowindex);
		double          getAverageSoftMarginTotal     (void);
		void            generateDriftCorrection       (double gain);
		void            analyzeTrackerBarSpacing      (void);
		void            analyzeTrackerBarPositions    (void);
		void            analyzeHorizontalHolePosition (void);
		void            markTrackerPositions          (bool showAll = false);
		void            analyzeMidiKeyMapping         (void);
		void            drawMajorAxes                 (void);
		double          getDustScore                  (void);
		double          getDustScoreBass              (void);
		double          getDustScoreTreble            (void);
		void            sortBadHolesByArea            (void);
		void            sortTearsByArea               (void);
		void            sortShiftsByAmount            (void);
		void            markHoleAttack                (HoleInfo& hi);
		void            markHoleAttacks               (void);
		void            markHoleShifts                (void);
		std::string     getDataMD5Sum                 (void);
		void            assignMusicHoleIds            (void);
		void            markSnakeBites                (void);
		void            markShifts                    (void);
		void            markShift                     (int index);
		void            generateNoteMidiFileHex       (ostream& output);
		void            generateNoteMidiFileBinasc    (ostream& output);
		void            generateMidifile              (MidiFile& midifile);
		void            assignMidiKeyNumbersToHoles   (void);
		void            setDebugOn                    (void);
		void            setDebugOff                   (void);
		void            setWarningOn                  (void);
		void            setWarningOff                 (void);
		void            setThreshold                  (int value);
		ucharint        getThreshold                  (void);

		// pixelType: a bitmask which contains enumerated types for the
		// functions of pixels (the PIX_* defines above):
		std::vector<std::vector<pixtype> > pixelType;

		// monochrome: a monochrome version of the roll image (typically
		// the green channel):
		std::vector<std::vector<ucharint> >   monochrome;

		// leftMarginIndex: The row-by-row margin to the left roll edge:
		std::vector<int>                  leftMarginIndex;

		// rightMarginIndex: The row-by-row margin to the right roll edge:
		std::vector<int>                  rightMarginIndex;

		// driftCorrection: column adjustment for each row in music area
		std::vector<double> driftCorrection;

		// uncorrectedCentroidHistogram: count of centroids at each pixel, uncorrected for drift.
		std::vector<int> uncorrectedCentroidHistogram;

		// correctedCentroidHistogram: count of centroids at each pixel, corrected for drift.
		std::vector<int> correctedCentroidHistogram;

		// rawRowPositions: Location of groups of holes (from correctedCentroid Histogram):
		std::vector<pair<double, int> > rawRowPositions;

		// holeSeparation -- number of pixels between hole centers.
		// double holeSeparation = 0.0;
		double holeSeparation;

		// holeOffset -- Pixel offset for holeSeparation on normalized width.
		// double holeOffset = 0.0;
		double holeOffset;

		// holes: List of musical holes on the piano roll (or at least
		// larger holes which are not anti-dust
		std::vector<HoleInfo*> holes;

		// badHoles: Holes which were initially marked as music holes, but
		// removed for some reason.  See also antidust.  Currently memory
		// management of these holes is done in holes, but maybe move them
		// here in the future and manage the memory here as well.
		std::vector<HoleInfo*> badHoles;

		// antidust: List of holes on roll which are too small to be musical.
		std::vector<HoleInfo*> antidust;

		// trackerArray -- holes sorted by tracker position
		std::vector<std::vector<HoleInfo*> > trackerArray;

		// midiToTrackMapping -- mapping from MIDI key number to hole position 
		// (in image, not roll).  Zero means no mapping (not allowed to reference
		// position 0 in trackerArray).
		std::vector<int> midiToTrackMapping;

		// trackMeaning -- the function of the hole, mostly for expression
		// and rewind hole.
		std::vector<int> trackMeaning;
	
		// midiEventCount -- 0 = no events, >0 = events (currently hole counts)
		std::vector<int> midiEventCount;

		// bassTears -- tear info for the left side of the roll
		std::vector<TearInfo*> bassTears;

		// trebleTears -- tear info for the right side of the roll
		std::vector<TearInfo*> trebleTears;

		// averageRollWidth -- the average width of a roll, 0 if uninit.
		// double averageRollWidth = 0.0;
		double averageRollWidth;

		// shifts -- list of shifts left or right, typically by the person
		// operating the scanner.
		std::vector<ShiftInfo*> shifts;


	protected:
		void       analyzeBasicMargins         (void);
		void       analyzeAdvancedMargins      (void);
		void       analyzeLeaders              (void);
		void       getRawMargins               (void);
		void       waterfallDownMargins        (void);
		void       waterfallUpMargins          (void);
		void       waterfallLeftMargins        (void);
		void       waterfallRightMargins       (void);
		ulongint   findLeftLeaderBoundary      (std::vector<int>& margin, double avg,
		                                        ulongint cols, ulongint searchlength);
		ulongint   findRightLeaderBoundary     (std::vector<int>& margin, double avg,
		                                        ulongint cols, ulongint searchlength);
		ulongint   getBoundary                 (std::vector<int>& status);
		void       markHardMargin              (ulongint leaderBoundary);
		void       markPreleaderRegion         (void);
		void       markLeaderRegion            (void);
		void       setHardMarginLeftIndex      (ulongint index);
		void       setHardMarginRightIndex     (ulongint index);
		ulongint   extractPreleaderIndex       (ulongint leaderBoundary);
		void       setPreleaderIndex           (ulongint value);
		void       setLeaderIndex              (ulongint value);
		void       analyzeHardMargins          (ulongint leaderBoundary);
		void       fillHoleInfo                (HoleInfo& hi, ulongint r, ulongint c, int& counter);
		void       fillTearInfo                (TearInfo& ti, ulongint r, ulongint c, int& counter);
		void       extractHole                 (ulongint row, ulongint col);
		void       markPosteriorLeader         (void);
		void       markHoleBB                  (HoleInfo& hi);
		double     getTrackerShiftScore        (double shift);
		void       analyzeTears                (void);
		void       analyzeShifts               (void);
		ulongint   storeShift                  (std::vector<double>& scores, ulongint startrow);
		ulongint   findPeak                    (std::vector<double>& array, ulongint r,
		                                        ulongint& peakindex, double& peakvalue);
		void       invalidateEdgeHoles         (void);
		void       fillHoleSimple              (ulongint r, ulongint c, int target, int type, int& counter);
		void       clearHole                   (HoleInfo& hi, int type);
		void       clear                       (void);
		void       calculateHoleDescriptors    (void);
		bool       calculateHolePerimeter      (HoleInfo& hole);
		int        findNextPerimeterPoint      (std::pair<ulongint, ulongint>& point, 
		                                        int dir);
		double     calculateCentralMoment      (HoleInfo& hole, int p, int q);
		double     calculateNormalCentralMoment(HoleInfo& hole, int p, int q);
		double     calculateMajorAxis          (HoleInfo& hole);
		void       invalidateSkewedHoles       (void);
		void       drawMajorAxis               (HoleInfo& hi);
		void       addAntidustToBadHoles       (ulongint areaThreshold);
		bool       goodColumn                  (ulongint col, ulongint toprow, ulongint botrow,
		                                        ulongint ptype, ulongint threshold);
		void       fillColumn                  (ulongint col, ulongint toprow, ulongint botrow,
		                                        ulongint target, ulongint threshold, ulongint replacement,
		                                        std::vector<int>& margin);
		int        getTrackerHoleCount         (void);
		void       recalculateFirstMusicHole   (void);
		void       removeBadLeaderHoles        (void);
		void       addDriftInfoToHoles         (void);
		void       groupHoles                  (void);
		void       groupHoles                  (ulongint index);
		void       describeTears               (void);
		ulongint   processTearLeft             (ulongint startrow, ulongint startcol);
		ulongint   processTearRight            (ulongint startrow, ulongint startcol);
		void       removeTearLeft              (ulongint minrow, ulongint maxrow, ulongint mincol, ulongint maxcol);
		void       removeTearRight             (ulongint minrow, ulongint maxrow, ulongint mincol, ulongint maxcol);
		void       invalidateOffTrackerHoles   (void);
		void       invalidateHolesOffTracker   (std::vector<HoleInfo*>& hi, ulongint index);
		void       analyzeSnakeBites           (void);
		void       analyzeRawRowPositions      (void);
		ulongint   storeWeightedCentroidGroup  (ulongint startindex);
		void       storeCorrectedCentroidHistogram(void);
		void       calculateTrackerSpacings2   (void);
		string     my_to_string                (int value);

	private:

//		bool       m_debug                     = false;
//		bool       m_warning                   = false;
//		bool       m_analyzedBasicMargins      = false;
//		bool       m_analyzedLeaders           = false;
//		bool       m_analyzedAdvancedMargins   = false;
//		int        hardMarginLeftIndex         = 0;
//		int        hardMarginRightIndex        = 0;
//		int        m_threshold                 = 255;
//		ulongint   preleaderIndex              = 0;
//		ulongint   leaderIndex                 = 0;
//		ulongint   firstMusicRow               = 0;
//		ulongint   lastMusicRow                = 0;
//		double     m_lastHolePosition          = 0.0;
//		double     m_firstHolePosition         = 0.0;
//		double     m_dustscore                 = -1.0;
//		double     m_dustscorebass             = -1.0;
//		double     m_dustscoretreble           = -1.0;
//		double     m_averageHoleWidth          = -1.0;

		bool       m_debug;
		bool       m_warning;
		bool       m_analyzedBasicMargins;
		bool       m_analyzedLeaders;
		bool       m_analyzedAdvancedMargins;
		int        hardMarginLeftIndex;
		int        hardMarginRightIndex;
		int        m_threshold;
		ulongint   preleaderIndex;
		ulongint   leaderIndex;
		ulongint   firstMusicRow;
		ulongint   lastMusicRow;
		double     m_lastHolePosition;
		double     m_firstHolePosition;
		double     m_dustscore;
		double     m_dustscorebass;
		double     m_dustscoretreble;
		double     m_averageHoleWidth;

#ifndef DONOTUSEFFT
		std::chrono::system_clock::time_point start_time;
		std::chrono::system_clock::time_point stop_time;
#endif
		std::vector<double> m_normalizedPosition;
		std::vector<double> m_trackerShiftScores;

};

} // end prp namespace

#endif /* _ROLLIMAGE_H */


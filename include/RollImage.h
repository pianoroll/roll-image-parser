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

#include "TiffFile.h"
#include "HoleInfo.h"
#include "TearInfo.h"
#include "RollOptions.h"

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

		void	          loadGreenChannel              (void);
		void            analyze                       (void);
		void            analyzeHoles                  (void);
		void            mergePixelOverlay             (std::fstream& output);
		void            markHoleBBs                   (void);
		std::ostream&   printRollImageProperties      (std::ostream& out = std::cout);
		int             getHardMarginLeftWidth        (void);
		int             getHardMarginRightWidth       (void);
		int             getHardMarginLeftIndex        (void);
		int             getHardMarginRightIndex       (void);
		ulong           getLeaderIndex                (void);
		ulong           getPreLeaderIndex             (void);
		ulong           getPreleaderIndex             (void);
		ulong           getFirstMusicHoleStart        (void);
		ulong           getLastMusicHoleEnd           (void);
		int             getSoftMarginLeftWidth        (ulong rowindex);
		int             getSoftMarginRightWidth       (ulong rowindex);
		int             getSoftMarginLeftWidthMax     (void);
		int             getSoftMarginRightWidthMax    (void);
		double          getAverageRollWidth           (void);
		double          getAverageMusicalHoleWidth    (void);
		ulong           getLeftMarginWidth            (ulong rowindex);
		ulong           getRightMarginWidth           (ulong rowindex);
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
		void            markHoleAttack                (HoleInfo& hi);
		void            markHoleAttacks               (void);
		void            markHoleShifts                (void);
		std::string     getDataMD5Sum                 (void);
		void            assignMusicHoleIds            (void);
		void            markSnakeBites                (void);

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

		// badHoles: Holes which were initially marked as music holes, but
		// removed for some reason.  See also antidust.  Currently memory
		// management of these holes is done in holes, but maybe move them
		// here in the future and manage the memory here as well.
		std::vector<HoleInfo*> badHoles;

		// antidust: List of holes on roll which are too small to be musical.
		std::vector<HoleInfo*> antidust;

		// trackerArray -- holes sorted by tracker position
		std::vector<std::vector<HoleInfo*>> trackerArray;

		// midiToHoleMapping -- mapping from MIDI key number to hole position 
		// (in image, not roll).  Zero means no mapping (not allowed to reference
		// position 0 in trackerArray).
		std::vector<int> midiToHoleMapping;

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
		double averageRollWidth = 0.0;


	protected:
		void       analyzeBasicMargins         (void);
		void       analyzeAdvancedMargins      (void);
		void       analyzeLeaders              (void);
		void       getRawMargins               (void);
		void       waterfallDownMargins        (void);
		void       waterfallUpMargins          (void);
		void       waterfallLeftMargins        (void);
		void       waterfallRightMargins       (void);
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
		void       fillHoleInfo                (HoleInfo& hi, ulong r, ulong c, int& counter);
		void       fillTearInfo                (TearInfo& ti, ulong r, ulong c, int& counter);
		void       extractHole                 (ulong row, ulong col);
		void       markPosteriorLeader         (void);
		void       markHoleBB                  (HoleInfo& hi);
		double     getTrackerShiftScore        (double shift);
		void       analyzeTears                (void);
		void       analyzeTearsOld             (void);
		ulong      findPeak                    (std::vector<double>& array, ulong r,
		                                        ulong& peakindex, double& peakvalue);
		void       processTearsOld             (std::vector<PreTearInfo>& ltear,
		                                        std::vector<PreTearInfo>& rtear,
		                                        std::vector<double>& lslow,
		                                        std::vector<double>& rslow,
		                                        std::vector<double>& lfast,
		                                        std::vector<double>& rfast);
		void       getTearInfoOld              (TearInfo& ti, PreTearInfo& pti,
		                                        std::vector<double>& slow,
		                                        std::vector<double>& fast,
		                                        int side,
		                                        std::vector<double>& oslow,
		                                        std::vector<double>& ofast);
		void       invalidateEdgeHoles         (void);
		void       fillHoleSimple              (ulong r, ulong c, int target, int type, int& counter);
		void       clearHole                   (HoleInfo& hi, int type);
		void       calculateHoleDescriptors    (void);
		bool       calculateHolePerimeter      (HoleInfo& hole);
		int        findNextPerimeterPoint      (std::pair<ulong, ulong>& point, 
		                                        int dir);
		double     calculateCentralMoment      (HoleInfo& hole, int p, int q);
		double     calculateNormalCentralMoment(HoleInfo& hole, int p, int q);
		double     calculateMajorAxis          (HoleInfo& hole);
		void       invalidateSkewedHoles       (void);
		bool       isGoodOppositeEdge          (PreTearInfo& pti, 
		                                        std::vector<double>& slow,
		                                        std::vector<double>& fast);
		void       drawMajorAxis               (HoleInfo& hi);
		void       addAntidustToBadHoles       (ulong areaThreshold);
		bool       goodColumn                  (ulong col, ulong toprow, ulong botrow,
		                                        ulong ptype, ulong threshold);
		void       fillColumn                  (ulong col, ulong toprow, ulong botrow,
		                                        ulong target, ulong threshold, ulong replacement,
		                                        std::vector<int>& margin);
		int        getTrackerHoleCount         (void);
		void       recalculateFirstMusicHole   (void);
		void       removeBadLeaderHoles        (void);
		void       addDriftInfoToHoles         (void);
		void       groupHoles                  (void);
		void       groupHoles                  (ulong index);
		void       describeTears               (void);
		ulong      processTearLeft             (ulong startrow, ulong startcol);
		ulong      processTearRight            (ulong startrow, ulong startcol);
		void       removeTearLeft              (ulong minrow, ulong maxrow, ulong mincol, ulong maxcol);
		void       removeTearRight             (ulong minrow, ulong maxrow, ulong mincol, ulong maxcol);
		void       invalidateOffTrackerHoles   (void);
		void       invalidateHolesOffTracker   (std::vector<HoleInfo*>& hi, ulong index);
		void       analyzeSnakeBites           (void);

	private:
		bool       m_analyzedBasicMargins      = false;
		bool       m_analyzedLeaders           = false;
		bool       m_analyzedAdvancedMargins   = false;
		int        hardMarginLeftIndex         = 0;
		int        hardMarginRightIndex        = 0;
		ulong      preleaderIndex              = 0;
		ulong      leaderIndex                 = 0;
		ulong      firstMusicRow               = 0;
		ulong      lastMusicRow                = 0;
		double     m_lastHolePosition          = 0.0;
		double     m_firstHolePosition         = 0.0;
		double     m_dustscore                 = -1.0;
		double     m_dustscorebass             = -1.0;
		double     m_dustscoretreble           = -1.0;
		double     m_averageHoleWidth          = -1.0;

		std::vector<double> m_normalizedPosition;

};

} // end prp namespace

#endif /* _ROLLIMAGE_H */


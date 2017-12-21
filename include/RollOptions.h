//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Dec  7 21:46:42 PST 2017
// Last Modified: Thu Dec  7 21:46:45 PST 2017
// Filename:      RollOptions.h
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Information about holes on piano roll
//

#ifndef _ROLLOPTIONS_H
#define _ROLLOPTIONS_H

#include <utility>
#include <iostream>

namespace prp  {

class RollOptions {
	public:
		         RollOptions               (void);
		        ~RollOptions               ();

		void     reset                     (void);

		double   getMinTrackerEdge         (void);
		void     setMinTrackerEdge         (double value);
		double   getMaxHoleTrackerWidth    (void);
		void     setMaxHoleTrackerWidth    (double value);
		double   getAspectRatioThreshold   (void);
		void     setAspectRatioThreshold   (double value);
		double   getMajorAxisCutoff        (void);
		void     setMajorAxisCutoff        (double value);
		double   getCircularityThreshold   (void);
		void     setCircularityThreshold   (double value);
		int      getMaxHoleCount           (void);
		void     setMaxHoleCount           (int value);
		int      getMaxTearFill            (void);
		void     setMaxTearFill            (int value);
		int      getAttackLineSpacing      (void);
		void     setAttackLineSpacing      (int value);
		double   getHoleShiftCutoff        (void);
		void     setHoleShiftCutoff        (double value);

	private:
		// m_minTrackerSpacingToPaperEdge: minimum distance from paper
		// edge to first tracker line on the roll.  The units are in terms
		// of spacing between tracker lines.
		double	m_minTrackerSpacingToPaperEdge;

		// m_maxHoleWidth: maximum width of music holes in units of tracker
		// bar spacings.
		double	m_maxHoleWidth;

		// m_aspectRatioThreshold: maximum w/h ratio
		double   m_aspectRatioThreshold;

		// m_majorAxisThreshold: maximum absolute degree deviation from 
		// vertical that a non-circular hole can have, yet still be called
		// a musical hole.
		double m_majorAxisThreshold;

		// m_circularityThreshold: Threshold between cicular holes and
		// elongated holes (used for detecting bad holes).
		double m_circularityThreshold;

		// m_maxHoleCount: The maxmimum number of holes expected on roll.
		int m_maxHoleCount;

		// m_maxTearFill: The max number of pixels to fill a tear with.
		int m_maxTearFill;

		// m_attackLineSpacing: distance between dots on note attack line.
		int m_attackLineSpacing;

		// m_holeShiftCutoff: the threshold in pixel shift between the
		// leading and trailing edge of a hole to decide if the paper
		// is shifting while the hole is playing.
		double m_holeShiftCutoff;

};


} // end prp namespace

#endif /* _ROLLOPTIONS_H */


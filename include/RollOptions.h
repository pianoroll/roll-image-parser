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

namespace rip  {

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

		std::string getRollType            (void);
		void     setRollTypeRedWelte       (void);
		void     setRollTypeGreenWelte     (void);
		int      getRewindHoleBassNumber   (void);
		int      getRewindHoleBassIndex    (void);
		int      getRewindHoleMidi         (void);

		double   getBridgeFactor           (void);
		int      getExpectedTrackerHoleCount(void);

	protected: // (maybe make private, but will have to create accessor functions)
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

		// m_maxHoleCount: The maxmimum number of tracker-bar holes expected on roll.
		int m_maxHoleCount;

		// m_minHoleCount: The maxmimum number of tracker-bar holes expected on roll.
		int m_minHoleCount;

		// m_maxTearFill: The max number of pixels to fill a tear with.
		int m_maxTearFill;

		// m_attackLineSpacing: distance between dots on note attack line.
		int m_attackLineSpacing;

		// m_holeShiftCutoff: the threshold in pixel shift between the
		// leading and trailing edge of a hole to decide if the paper
		// is shifting while the hole is playing.
		double m_holeShiftCutoff;

		// m_rollType: the type of roll being processed ("" = unknown).
		std::string m_rollType;

		// m_rewindHole: the tracker-bar hole position.  Indexed from 1
		// for the first hole on the left (bass) side of the roll.
		// 0 means unknown (as in an unknown roll type).
		int m_rewindHole = 0;

		// m_rewindHoleMidi: the rewind hole positions in terms of 
		// MIDI key number (0 means unknown position).
		int m_rewindHoleMidi = 0;

		// starting positions for expression and regular note tracks:
		int m_bassExpressionTrackStartNumberLeft = 0;
		int m_bassExpressionTrackStartMidi = 0;
		int m_bassNotesTrackStartNumberLeft = 0;
		int m_bassNotesTrackStartMidi = 0;
		int m_trebleNotesTrackStartNumberLeft = 0;
		int m_trebleNotesTrackStartMidi = 0;
		int m_trebleExpressionTrackStartNumberLeft = 0;
		int m_trebleExpressionTrackStartMidi = 0;

		// m_trackerHoles == number of holes in the tracker bar
		int m_trackerHoles = 0;

		// MIDI file track assignments (offset from 0, with track 0 note having notes):
		int m_bass_track       = 1;
		int m_treble_track     = 2;
		int m_bass_exp_track   = 3;
		int m_treble_exp_track = 4;

		// MIDI file channel assignments for each track, (offset from 0):
		int m_bass_exp_ch      = 0;
		int m_bass_ch          = 1;
		int m_treble_ch        = 2;
		int m_treble_exp_ch    = 3;

		// Bridging factor to merge adjacent holes:
		double m_bridgeFactor = 1.37;
};


} // end rip namespace

#endif /* _ROLLOPTIONS_H */




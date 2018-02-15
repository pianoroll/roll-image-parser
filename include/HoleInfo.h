//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  2 03:01:50 PST 2017
// Last Modified: Sat Dec  2 03:01:55 PST 2017
// Filename:      HoleInfo.h
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Information about holes on piano roll
//

// Compactness: area/perimeter^2
// http://web.cs.wpi.edu/~emmanuel/courses/cs545/S14/slides/lecture08.pdf

#ifndef _HOLEINFO_H
#define _HOLEINFO_H

#include <utility>
#include <iostream>
#include <string>

namespace prp  {

typedef int64_t  longlong;
typedef uint64_t ulonglongint;
typedef uint32_t ulongint;
typedef uint16_t ushort;
typedef uint8_t  uchar;

class HoleInfo {
	public:
		         HoleInfo     (void);
		        ~HoleInfo     ();

		std::pair<ulongint, ulongint> origin;   // Row, Column of origin (top let corner)
		std::pair<ulongint, ulongint> width;    // Row, Column widths.
		std::pair<double, double>     centroid; // Center of mass
		std::pair<ulongint, ulongint> entry;    // entry point for filling holes
		ulongint                  track;        // tracker hole index
		ulongint                  area;         // area of hole
		double                    circularity;  // circularity of hole
		double                    perimeter;    // outer contour of hole
		double                    majoraxis;    // angle of longest axis
		double                    coldrift;     // column drive in pixels
		std::string               id;           // unique identifier (if not empty)
		std::string               reason;       // reason for being a bad hole (if bad)
		double                    leadinghcor;  // leading horizontal pixel correction
		double                    trailinghcor; // trailing horizontal pixel correction
		double                    prevOff;      // distance from onset to offset of previous hole in track
		bool                      attack;       // true if note attack false otherwise
		bool                      snakebite;    // true if part of melody highlighting
		ulongint                  offtime;      // if attack==true, then this is the offtime of the note
		int                       midikey;      // MIDI key number for note

		void     clear            (void);
		bool     isMusicHole      (void) { return m_type == 1 ? 1 : 0; }
		void     setNonHole       (void) { m_type = 0; }
		std::ostream& printAton   (std::ostream& out = std::cout);
		bool      isShifting      (void);

	private:
		char     m_type;
};

std::ostream& operator<<(std::ostream& out, HoleInfo& hi);
std::ostream& operator<<(std::ostream& out, HoleInfo* hi);

} // end prp namespace

#endif /* _HOLEINFO_H */


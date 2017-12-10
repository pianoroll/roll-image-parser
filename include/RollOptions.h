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
		         RollOptions     (void);
		        ~RollOptions     ();

		void     reset           (void);

		double   getMinTrackerEdge(void);
		void     setMinTrackerEdge(double value);

	private:
		// m_minTrackerSpacingToPaperEdge: minimum distance from paper
		// edge to first tracker line on the roll.  The units are in terms
		// of spacing between tracker lines.
		double	m_minTrackerSpacingToPaperEdge;

};


} // end prp namespace

#endif /* _ROLLOPTIONS_H */


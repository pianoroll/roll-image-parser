//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Dec  2 03:01:50 PST 2017
// Last Modified: Sat Dec  2 03:01:55 PST 2017
// Filename:      TearInfo.h
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Information about holes on piano roll
//

// Compactness: area/perimeter^2
// http://web.cs.wpi.edu/~emmanuel/courses/cs545/S14/slides/lecture08.pdf

#ifndef _TEARINFO_H
#define _TEARINFO_H

#include "HoleInfo.h"

#include <utility>
#include <iostream>

namespace prp  {

class PreTearInfo {
	public:
		ulong startindex = 0;
		ulong endindex = 0;
		ulong peakindex = 0;
		double peak = 0.0;
};

class TearInfo : public HoleInfo {
	public:
		                 TearInfo     (void);
		                ~TearInfo     ();

		std::ostream&    printAton    (std::ostream& out);
};

std::ostream& operator<<(std::ostream& out, TearInfo& hi);
std::ostream& operator<<(std::ostream& out, TearInfo* hi);

} // end prp namespace

#endif /* _TEARINFO_H */


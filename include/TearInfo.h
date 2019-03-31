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

namespace rip  {

class PreTearInfo {
	PreTearInfo(void) {
		startindex = 0;
		endindex   = 0;
		peakindex  = 0;
		peak = 0.0;
	}
	public:
//		ulongint startindex = 0;
//		ulongint endindex   = 0;
//		ulongint peakindex  = 0;
//		double peak = 0.0;
		ulongint startindex;
		ulongint endindex;
		ulongint peakindex;
		double peak;
};

class TearInfo : public HoleInfo {
	public:
		                 TearInfo     (void);
		                ~TearInfo     ();

		std::ostream&    printAton    (std::ostream& out);
};

std::ostream& operator<<(std::ostream& out, TearInfo& hi);
std::ostream& operator<<(std::ostream& out, TearInfo* hi);

} // end rip namespace

#endif /* _TEARINFO_H */


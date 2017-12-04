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

namespace tiff  {

class HoleInfo {
	public:
		         HoleInfo     (void);
		        ~HoleInfo     ();

		std::pair<ulong, ulong>   origin;     // Row, Column of origin
		std::pair<ulong, ulong>   width;      // Row, Column widths.
		std::pair<double, double> centroid;   // Center of mass

		ulong    area;   // area of hole

		void     clear        (void);

};

std::ostream& operator<<(std::ostream& out, HoleInfo& hi);
std::ostream& operator<<(std::ostream& out, HoleInfo* hi);

} // end tiff namespace

#endif /* _HOLEINFO_H */


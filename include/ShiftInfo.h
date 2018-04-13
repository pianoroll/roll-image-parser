//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Dec 29 19:15:57 PST 2017
// Last Modified: Fri Dec 29 19:16:03 PST 2017
// Filename:      ShiftInfo.h
// Web Address:
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   Information about shifts on piano roll
//

#ifndef _SHIFTINFO_H
#define _SHIFTINFO_H

#include <utility>
#include <iostream>
#include <string>

namespace prp  {

// typedef uint32_t ulongint;
typedef unsigned long ulongint;

class ShiftInfo {
	public:
		         ShiftInfo     (void);
		        ~ShiftInfo     ();
		void     clear         (void);
		std::ostream& printAton(std::ostream& out);

		ulongint    row;
		double      score;
		std::string id;
};


} // end prp namespace

#endif /* _SHIFTINFO_H */


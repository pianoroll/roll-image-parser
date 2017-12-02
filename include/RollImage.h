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

#include "TiffFile.h"

namespace tiff  {


// Pixel functions:
#define PIX_PAPER      0    /* pixel represents paper */
#define PIX_NONPAPER   1    /* pixel represents uncategorized non-paper */
#define PIX_MARGIN     2    /* pixel represents soft margins */
#define PIX_LEADER     3    /* pixel represents non-paper in leader region */
#define PIX_PRELEADER  4    /* pixel represents non-paper in pre-leader region */
#define PIX_HARDMARGIN 5    /* pixel represents hard-margin */
#define PIX_DEBUG      6    /* debugging pixel type 1 */
#define PIX_DEBUG2     7    /* debugging pixel type 2 */

typedef unsigned char pixtype;

class RollImage : public TiffFile {
	public:
		            RollImage                  (void);
		           ~RollImage                  ();

		void	     loadGreenChannel            (void);
		void       analyze                     (void);
		void       mergePixelOverlay           (std::fstream& output);

		std::vector<std::vector<pixtype>> pixelType;
		std::vector<std::vector<uchar>>   green;
		std::vector<int>                  leftMargin;
		std::vector<int>                  rightMargin;

	protected:
		void       analyzeMargins              (void);
		void       analyzeLeader               (void);
		void       getRawMargins               (void);
		void       waterfallDownMargins        (void);
		ulong      findLeftLeaderBoundary      (std::vector<int>& margin, double avg,
		                                        ulong cols, ulong searchlength);
		ulong      findRightLeaderBoundary     (std::vector<int>& margin, double avg, 
		                                        ulong cols, ulong searchlength);
		ulong      getBoundary                 (std::vector<int>& status);
		ulong      getPreleaderIndex           (ulong leaderBoundary);
		void       markHardMargin              (ulong leaderBoundary);
		void       markPreleaderRegion         (ulong preleaderBoundary);
		void       markLeaderRegion            (ulong leaderBoundary);

	private:
		bool			m_analyzedMargins = false;


};

} // end tiff namespace

#endif /* _ROLLIMAGE_H */


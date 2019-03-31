//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Thu Feb 15 02:58:49 PST 2018
// Filename:      TiffFile.h
// Web Address:   
// Syntax:        C++
// vim:           ts=3:nowrap:ft=text
//
// Description:   TIFF file parsing.
//
// References:
//      https://web.archive.org/web/20160306201233/http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf (page 13)
//      https://www.loc.gov/preservation/digital/formats/content/tiff_tags.shtml
//      https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
//

#ifndef _TIFFFILE_H
#define _TIFFFILE_H

#include <vector>

#include "TiffHeader.h"

namespace rip  {


class TiffFile : public std::fstream, public TiffHeader {
	public:
		            TiffFile                    (void);
		           ~TiffFile                    ();

		void        close                       (void);
		bool        open                        (const std::string& filename);
		bool        goToByteIndex               (ulonglongint offset);
		ushortint   readLittleEndian2ByteUInt   (void);
		std::string readString                  (ulongint count);
		ucharint    read1UByte                  (void);
		void        getImageGreenChannel        (std::vector<std::vector<ucharint> >& image);
		bool        goToPixelIndex              (ulonglongint pindex);
		bool        goToRowColumnIndex          (ulongint rowindex, ulongint colindex);
		std::string getFilename                 (void);

		// header updates on disk
		bool        writeSamplesPerPixel        (int count);
		void        writeDirectoryOffset        (ulonglongint offset);

	private:
		std::string m_filename;
		// std::fstream m_input;

};

} // end rip namespace

#endif /* _TIFFFILE_H */


//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Thu Feb 15 03:26:42 PST 2018
// Filename:      TiffHeader.h
// Web Address:   
// Syntax:        C++;
// vim:           ts=3:nowrap:ft=text
//
// Description:   TIFF file header.
//
// References:
//      https://web.archive.org/web/20160306201233/http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf (page 13)
//      https://www.loc.gov/preservation/digital/formats/content/tiff_tags.shtml
//      https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
//

#ifndef _TIFFHEADER_H
#define _TIFFHEADER_H

#include "Utilities.h"

namespace prp  {


class TiffHeader {
	public:
		               TiffHeader          (void);
		              ~TiffHeader          ();
		void           clear               (void);

		ulongint       getRows             (void) const;
		ulongint       getCols             (void) const;
		int            getOrientation      (void) const;
		ulonglongint   getDataOffset       (void) const;
		ulonglongint   getDataBytes        (void) const;
		double         getRowDpi           (void) const;
		double         getColDpi           (void) const;
		ulonglongint   getPixelOffset      (ulonglongint pindex) const;
		ulonglongint   getPixelOffset      (ulongint rindex, ulongint cindex) const;
		ulonglongint   getPixelCount       (void) const;
		void           setBigTiff          (void);
		bool           isBigTiff           (void);
		bool           parseHeader         (std::fstream& input);

	protected:
		void           setOrientation      (int value);
		void           setRows             (ulongint value);
		void           setCols             (ulongint value);
		void           setRowDpi           (double value);
		void           setColDpi           (double value);
		void           setDataBytes        (ulonglongint value);
		void           setDataOffset       (ulonglongint value);
		ulonglongint   readEntryUInteger   (std::fstream& input, int datatype, ulonglongint count, int tag = -1);
		double         readType5Value      (std::fstream& input, int datatype, ulonglongint count, int tag = -1);
		bool           goToByteIndex       (std::fstream& input, ulongint offset);
		bool           goToByteIndex       (std::fstream& input, ulonglongint offset);

	private:
		bool           parseDirectory      (std::fstream& input, ulonglongint diroffset);
		bool           readDirectoryEntry  (std::fstream& input);

	private:
		ulongint       m_rows        = 0;
		ulongint       m_cols        = 0;
		int            m_orientation = 0;
		ulonglongint   m_dataoffset  = 0;
		ulonglongint   m_databytes   = 0;
		double         m_rowdpi      = 0.0;
		double         m_coldpi      = 0.0;
		bool           m_64bitQ      = false;
};


std::ostream& operator<<(std::ostream& output, const prp::TiffHeader& header);


} // end prp namespace

#endif /* _TIFFHEADER_H */


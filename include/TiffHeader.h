//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Nov 23 11:47:47 PST 2017
// Last Modified: Sat Nov 25 12:29:32 PST 2017
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

#include "TiffUtilities.h"

namespace tiff  {


class TiffHeader {
	public:
		       TiffHeader          (void);
		      ~TiffHeader          ();
		void   clear               (void);

		ulong  getRows             (void) const;
		ulong  getCols             (void) const;
		int    getOrientation      (void) const;
		ulong  getDataOffset       (void) const;
		ulong  getDataBytes        (void) const;
		double getRowDpi           (void) const;
		double getColDpi           (void) const;
		ulong  getPixelOffset      (ulong pindex) const;
		ulong  getPixelOffset      (ulong rindex, ulong cindex) const;
		ulong  getPixelCount       (void) const;

		bool   parseHeader         (std::fstream& input);

	protected:
		void   setOrientation      (int value);
		void   setRows             (ulong value);
		void   setCols             (ulong value);
		void   setRowDpi           (double value);
		void   setColDpi           (double value);
		void   setDataBytes        (ulong value);
		void   setDataOffset       (ulong value);

	private:
		bool   parseDirectory      (std::fstream& input, ulong diroffset);
		bool   readDirectoryEntry  (std::fstream& input);

	private:
		ulong  m_rows;
		ulong  m_cols;
		int    m_orientation;
		ulong  m_dataoffset;
		ulong  m_databytes;
		double m_rowdpi;
		double m_coldpi;
};


std::ostream& operator<<(std::ostream& output, const tiff::TiffHeader& header);


} // end tiff namespace

#endif /* _TIFFHEADER_H */


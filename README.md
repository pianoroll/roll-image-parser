# roll-image-parser

This repository is used to extract musical information from scans
of piano rolls that are scanned with the Stanford Libraries' scanner
for its collection of piano and organ rolls.

### Compiling

To compile the library and tools, type:

```bash
make
```

GNU make must be installed, and gcc version 4.9 or higher (or most versions of clang on macOS).

### Tools


| Name                | Description |
|---------------------|--------------------- |
| tiff2holes          | Main program to identify musical holes in a TIFF image of a piano roll.
| markholes           | The same as tiff2holes, but takes two identical images as input, analyzing the first, and then writing analysis marks on the second for debugging and quality analysis. |
| channelhistograms   | |
| checkquality        | Some basic image quality checks. |
| frameduplicates     | Check for visual defects in the TIFF images (checking for a now resolved acquisition software bug). |
| getGreenPgm         | |
| leftrightswap       | Mirror the TIFF image on a vertical axis (reversing from left to right). |
| markbright          | |
| mono2color          | |
| tifflength          | |
| tifforientation     | |

### tiff2holes output

The tiff2holes (and the markholes) tools output a text file containing
information about the holes on the paper and also drift analyses
as well as two embedded MIDI files based on the hole analysis.
[Here](https://github.com/pianoroll/roll-image-parser/blob/master/example/gg384dv5303.txt)
is an example output file.

#### Extracted parameters

At the start of the file are a list of extracted features from the image:

| Parameter          	| Description |
|-----------------------|------------------------------------------------------------- |
| DRUID			| Stanford Libraries Digital Repopsitory Unique ID.  The original scan can be accessed from https://purl.stanford.edu/DRUID (when it is put online). |
| ROLL_TYPE		| Brand/format of the piano roll, such as "welte_red" for red Welte-Mignon rolls, "welte-green", "welte-licensee", "ampico-a", "ampico-b", "duoart" |
| THRESHOLD		| Threshold byte value for non-paper boundary.  This is used to separate the paper from the not-paper parts of the image. |
| LENGTH_DPI		| Scan DPI resolution along the length of the roll. |
| IMAGE_WIDTH		| Width of the input image in pixels. |
| IMAGE_LENGTH		| Length of the input image in pixels. |
| ROLL_WIDTH		| Measured average width of the piano-roll in pixels. |
| HARD_MARGIN_BASS	| Pixel width of the margin on the bass side of the roll where the roll paper never enters. |
| HARD_MARGIN_TREBLE	| Pixel width of the margin on the treble side of the roll where the roll paper never enters. |
| MAX_BASS_DRIFT	| Maximum range of the "soft" margin on the bass side: the margin area where the roll edge will temporarily enter. |
| MAX_TREBLE_DRIFT	| Maximum range of the "soft" margin on the treble side: the margin area where the roll edge will temporarily enter. |
| AVG_SOFT_MARGIN_SUM	| Average sum of the bass and treble soft margins. |
| DRIFT_RANGE		| Total drift range in pixels. |
| DRIFT_MIN		| Leftmost drift from average position in pixels. |
| DRIFT_MAX		| Rightmost drift from average position in pixels. |
| PRELEADER_ROW		| Last pixel row of the portion of the image which contains the velcro strap that initially pulls the roll. |
| LEADER_ROW		| Last pixel row of the leader (although text on the roll may continue). |
| FIRST_HOLE		| Pixel row of the first musical hole. |
| LAST_HOLE		| Pixel row of the end of the last musical hole. Currently includes rewind holes and any punches after the rewind. |
| END_MARGIN		| IMAGE_LENGTH - LAST_HOLE. |
| MUSICAL_LENGTH	| Pixel row count from the first music hole to the end of the last music hole. |
| MUSICAL_HOLES		| Estimated number of intentional holes to be read by the tracker bar. |
| MUSICAL_NOTES		| Number of notes after grouping chained holes (expression tracks count as notes in this census). |
| AVG_HOLE_WIDTH	| Average width of a musical hole in pixels (based on holes' bounding boxes). |
| ANTIDUST_COUNT	| Number of holes in the paper with an area less than 50 pixels (the smallest music holes typically have 300 pixels) |
| BAD_HOLE_COUNT	| Number of suspcious holes pulled out for further observation. |
| EDGE_TEAR_COUNT	| Number of edge tears which are deeper than 1/10 of an inch. |
| BASS_TEAR_COUNT	| Number of tears on the bass register side of the roll. |
| TREBLE_TEAR_COUNT	| Number of tears on the treble register side of the roll. |
| DUST_SCORE		| Count of dust particles in hard margin regions in units of parts per million. |
| DUST_SCORE_BASS	| Dust particle count in bass register margin. |
| DUST_SCORE_TREBLE	| Dust particle count in bass register margin. |
| SHIFTS		| Number of automatically detected operator shifts greater than 1/100th of an inch over 1/3 of an inch. |
| HOLE_SEPARATION	| Distance between muiscal hole centers (i.e., the tracker bar hole spacings). |
| HOLE_OFFSET		| The offset of the tracker bar spacing pattern with respect to the first column of the image. |
| TRACKER_HOLES		| The (esitmated) number of tracker bar holes that reads this roll. |
| SOFTWARE_DATE		| The compiling date for the software that generates this file. |
| ANALYSIS_DATE		| The date that the analysis was done. |
| ANALYSIS_TIME		| The duration of the analysis phase of the software (excluding loading of the image data or writing the analysis overlay onto the image). |
| COLOR_CHANNEL		| The color channel used to generate this analysis data. |
| CHANNEL_MD5		| The MD5 checksum of the color channel pixels used in this analysis. |
| BRIDGE_FACTOR		| Aspect ratio merging distance for adjacent holes. |
| MANUAL_EDITS		| Set this field to "yes" if any manual edits are made to this file. |

#### Hole list

Then comes a list of the identified musical holes on the paper, both regular notes and expression holes.
The holes are sorted in time (in other words by row in the image) from the top of the image to the
bottom.  Here is the information recorded for each hole:


| Parameter     |	Description |
|---------------|--------------------------------------------------------- |
| ORIGIN_ROW	|	The pixel row of the leading edge of the bounding box around the hole. |
| ORIGIN_COL	|	The pixel column of the leading edge of the bounding box around the hole, bass side. |
| WIDTH_ROW	|	The pixel length of the bounding box around the hole. |
| WIDTH_COL	|	The pixel column of the leading edge of the hole, bass side. |
| CENTROID_ROW	|	The center of mass row of the hole. |
| CENTROID_COL	|	The center of mass column of the hole. |
| AREA		|	The area of the hole (in pixels). |
| PERIMETER	|	The perimeter of the hole (in pixels). |
| CIRCULARITY	|	The circularity of the hole (1 = circular, 0 = very uncircular). |
| MAJOR_AXIS	|	The major axis of the hole in degrees (0 = vertically aligned with roll). |
| HPIXCOR	|	Horizontal (column) pixel correction: shift to straighten hole columns on paper.  If the leading and trailing edges of musical holes do not have the same correction value, then the following two parameters are given instead by the following two parameters. |
| HPIXCOR_LEAD	|	Horizontal pixel correction of the hole's leading edge. |
| HPIXCOR_TRAIL	|	Horizontal pixel correction of the hole's trailing edge. |


Then comes a list of the "bad" holes which were excluded from the good hole list given above.  Sometimes 
bad holes prevent a re-attack of a sustained note, but otherwise "bad" holes are usually suppressed in the
conversion to MIDI data.  This will remove unwanted notes, but sometimes will also remove desired notes.
Manual post-processing of the analysis would be required to examine "bad" holes for a more accurate final result.


#### Drift analysis

Next comes a list of the tears along the edges of the roll.  The information extracted from each tear:


| Parameter     | Description |
|---------------|----------------------------- |
| ID		| Unique tear ID (enumeration of the tear along the left (bass) and right (treble) sides of the roll. |
| ORIGIN_ROW	| The starting row of the tear. |
| ORIGIN_COL	| The starting column of the tear. |
| WIDTH_ROW	| The maximum height of the tear. |
| WIDTH_COL	| The maximum width of the tear. |
| AREA		| The measured aread of the tear (actual not bounding box area). |


Then comes a table of the left/right drift of the roll along its
length in the scan.  This information is used to straighten out the
image coordinates when extracting holes, and the same information
can be used to straighten the original visual image, if desired.
The information consists of a table with two columns.  The first
column is the starting row in the image for the offset given in the
second column.  Each entry in the table updates the drift when it
changes by more than 1/10th of a pixel.


#### MIDI files

Next comes two MIDI files.  The first is one that removes bridging to group multiple holes into single notes.
The second one contains individual MIDI notes for each musical hole on the image (preserving bridging).

#### Hole histograms

The last section analyzes the vertical positions of musical holes before and after drift analysis has been done,
as well as the final vertical position assignment after the Fourier Transform analysis has been done.




#include <ctype.h>
#include <math.h>
#include <stdio.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "config.h"

#if defined(HAVE_LIBQMV) && defined(HAVE_QMV_HVECTOR_H)
#include <qmv/hvector.h>
#include <qmv/nearest.h>
#include <qmv/intersection.h>

#define MYSQL_CMD "mysql -h mapserver1 -udeghost_user -pdghstpsswrd themis2"
#define WGET_CMD "wget -q -O -"

// For every pixel DOWN in image for the ghost, we ADD this much
// fraction of a second in degrees west longitude to the longitude.
#define PIXEL_FRAC_SEC 30.0

// Number of seconds in a martian day
#define MARS_DAY 88642.663

// Defines rounding tolerance for the edges of the framelets
#define ZERO (1.0/512)

// Defines the reference slant distance used for altitude correction
// of the along-track ghosting offset values. This is the average
// slant distance for image I01076010.
//#define REFERENCE_DISTANCE 402.4
#define REFERENCE_IMAGE "I01076010"

static char ERRMSG[5000];

static Ellipsoid MARS(3397,3397,3375);

#define MOLA_DATA_FILE "/mgs/mola/megdr/megr90n000cb.img"

// The raw mola planetary radii bytes, at 4 pixels per degree, indexed
// by east-leading areocentric longitude and latitude, two bytes per
// pixel. Measured in meters, relative to a base offset of
// MOLA_OFFSET.
#define MOLA_COLS (4 * 360)
#define MOLA_ROWS (4 * 180)
#define MOLA_OFFSET 3396000.0
static unsigned char mola[MOLA_COLS * MOLA_ROWS * 2];

struct Point
 {
	int x;
	int y;

	Point()
	 : x(0)
	 , y(0)
	 { }

	Point(int x, int y)
	 {
		this->x = x;
		this->y = y;
	 }
 };

struct Point2D
 {
	double x;
	double y;
	Point2D(double x, double y)
	 {
		this->x = x;
		this->y = y;
	 }
	Point2D()
	 : x(0)
	 , y(0)
	 { }
 };

// GOOD ___ONLY___ FOR UNIT VECTORS!
static HVector
rotateP(HVector orig, HVector axis, double theta)
 {
	HVector dir = axis % orig; // need a .unit() for non-unit-vectors

	// We've set up a coordinate system: orig is x, dir is y.
	// Return angle theta in that coordinate system and we're done.
	return  orig*cos(theta) + dir*sin(theta);
 }

static double
ellipsoid_altitude(const HVector &pt)
 {
	Nearest_point np(MARS, pt);
	if(np.error)
	 {
		fprintf(stderr,
				"********* DE-GHOST STRANGENESS!!! ERROR=%d\n"
				"********* Tell Michael the OLD altitude function failed.\n",
				np.error);
		return  NAN;
	 }
 }

// Returns non-zero if mola is available, zero if it's not.
inline static int
mola_loaded()
 {
	static int load_attempted = 0;
	static int load_successful = 0;

	if(!load_attempted)
	 {
		load_attempted = 1;

		FILE *fp = fopen(MOLA_DATA_FILE, "r");
		if(!fp)
		 {
			fprintf(stderr,
					"******* DE-GHOST STRANGENESS!!!\n"
					"******* Unable to open MOLA data file "
					MOLA_DATA_FILE "\n");
			perror("******* perror says");
		 }
		else
		 {
			// We offer to read an extra byte so that we can detect if
			// the file is bigger than it should be.
			unsigned char *mola2 = new unsigned char[sizeof(mola) + 1];
			size_t bytes_read = fread(mola2, 1, sizeof(mola)+1, fp);
			if(bytes_read != sizeof(mola))
				fprintf(stderr,
						"********* DE-GHOST STRANGENESS!!!\n"
						"********* The mola data file was too big. "
						MOLA_DATA_FILE "\n");
			else
				load_successful = 1;
			memcpy(mola, mola2, sizeof(mola));
			delete[] mola2;
			fclose(fp);
		 }
	 }

	return  load_successful;
 }

static double inline
toRadians(double deg)
 {
	return  deg / 180.0 * M_PI;
 }

static double inline
toDegrees(double rad)
 {
	return  rad / M_PI * 180.0;
 }

inline static HVector
toVec(const double &lon, const double &lat)
 {
	return
		HVector(
			cos(toRadians(lat)) * cos(toRadians(-lon)),
			cos(toRadians(lat)) * sin(toRadians(-lon)),
			sin(toRadians(lat))
			);
 }

inline static HVector
toVec(const Point2D& pt)
 {
	return  toVec(pt.x, pt.y);
 }

static double
lat(const HVector& v)
 {
	return  toDegrees(asin(v.unit()[2]));
 }

static double
lon(const HVector& v)
 {
	double x = v[0];
	double y = v[1];
	double z = v[2];

	if(y > 0)
		return  toDegrees(M_PI * 2 - atan2(y, x));

	else if(y < 0)
		return  toDegrees(-atan2(y, x));

	else if(x < 0)
		return  toDegrees(M_PI);

	else
		return  0;

 }

static Point2D
toLonLat(const HVector& v)
 {
	return  Point2D(lon(v),
					lat(v));
 }

static int MSB = 0;
static int LSB = 1;

// Takes west-leading longitude, even though mola operates in
// east-leading. Returns the MOLA radius at the given point, in
// kilometers (even though MOLA's stored in meters).
inline static double
getMolaRadius(double lon, double lat)
 {
	if(!mola_loaded())
		return  NAN;

	lat = 90-lat; // north pole to zero, south pole to 180
	lon = 360 - lon; // west-leading converted to east-leading

	int idxLon = int(lon * 4 + 0.5) % MOLA_COLS;
	int idxLat = int(lat * 4 + 0.5);

	int msb = mola[(idxLon + idxLat*MOLA_COLS) * 2 + MSB];
	int lsb = mola[(idxLon + idxLat*MOLA_COLS) * 2 + LSB];

	signed short meters = msb<<8 | lsb;
//	printf("lsb\t%d\t(%d)\n", lsb, (signed short)(lsb<<8));
//	printf("msb\t%d =\t(%d)\n", msb, (signed short)(msb<<8));
//	printf("%6d\t", meters);

	return  (MOLA_OFFSET + meters) / 1000.0;
 }

// Given a lon/lat and a slant distance, calculates a vector relative
// to the ellipsoid's surface (but not in a normal-to-the-surface
// direction). Returns a vector with the given areoCENTRIC
// west-leading lon/lat, but whose radius vector extends slant_dist
// beyond the ellipsoid's surface.
inline static HVector
surfaceVector(double lon, double lat, double slant_dist)
 {
	static HVector ORIGIN(0,0,0);
	HVector unit = toVec(lon, lat);
	HVector pt = Intersection(Ray(ORIGIN,unit), MARS).point1;
	return  unit * (pt.norm() + slant_dist);
 }

// Given a lon/lat in west-leading areocentric coordinates, returns a
// radial vector whose magnitude comes from MOLA topography.
inline static HVector
molaVector(double lon, double lat)
 {
	return  toVec(lon, lat) * getMolaRadius(lon, lat);
 }

struct Cell
 {
	// Each of the cell's border points
	HVector sw;
	HVector se;
	HVector nw;
	HVector ne;
	// What order were they just specified in?
	int clockwise;
	// All of the border points, as a 5-point closed quadrilateral chain
	HVector chain[5];

	// The normals of each cell wall, pointing inwards.
	HVector s;
	HVector n;
	HVector e;
	HVector w;
	// The 4 normals in sequence, corresponding somewhat to 'chain' above
	HVector norms[4];

	// Interpolation axes
	HVector wePlane;
	HVector snPlane;
	// Interpolation data values
	double wePlaneSpan;
	double snPlaneSpan;

	// Pre-calculatable stuff for interpolate()
	HVector s_normal;
	HVector n_normal;
	HVector e_normal;
	HVector w_normal;
	HVector nperc_axis;
	HVector eperc_axis;
	double ns_maxangle;
	double ew_maxangle;

	// Distance from center of planet to the spacecraft, averaged for
	// this framelet.
	double radius;

	// Adapted (and improved) from SpatialGraphicsSpOb.uninterpolate().
	Point2D uninterpolate(HVector pt)
	 {
		HVector pt_we = (wePlane % pt).unit();
		HVector pt_sn = (snPlane % pt).unit();

		double x = separation(pt_we,w).value / wePlaneSpan;
		double y = separation(pt_sn,s).value / snPlaneSpan;

		if((pt ^ w) < 0) x = -x;
		if((pt ^ s) < 0) y = -y;

		return  Point2D(x, y);
	 }

	// Copied in from SpatialGraphicsSpOb
	HVector interpolate(double x, double y)
	 {
// 		HVector s_normal = (sw % se).unit();
// 		HVector n_normal = (nw % ne).unit();
// 		HVector e_normal = (ne % se).unit();
// 		HVector w_normal = (nw % sw).unit();

// 		HVector nperc_axis = (s_normal % n_normal).unit();
// 		HVector eperc_axis = (w_normal % e_normal).unit();

// 		double ns_maxangle = acos(s_normal ^ n_normal);
// 		double ew_maxangle = acos(e_normal ^ w_normal);

		//// ABOVE: pre-calculatable / BELOW: dynamic ////

		double n_angle = ns_maxangle * y;
		HVector ns_interp_normal = rotateP(s_normal, nperc_axis, n_angle);

		double e_angle = ew_maxangle * x;
		HVector ew_interp_normal = rotateP(w_normal, eperc_axis, e_angle);

		HVector pt = (ew_interp_normal % ns_interp_normal).unit();

		// Make sure we have the right sign on pt
		if((pt ^ nw) >= 0)
			return  pt;
		else
			return  -pt;
	 }

	int OLD_contains(const HVector& v)
	 {
		return  (n ^ v) >= -ZERO
			&&  (s ^ v) >= -ZERO
			&&  (e ^ v) >= -ZERO
			&&  (w ^ v) >= -ZERO;
	 }

	int contains(const HVector& v, Point2D& pt)
	 {
		if(!OLD_contains(v))
			return  0;
		pt = uninterpolate(v);

		if     (pt.x < 0)  if(pt.x >=  -ZERO)  pt.x = 0;  else  return  0;
		else if(pt.x > 1)  if(pt.x <= 1+ZERO)  pt.x = 1;  else  return  0;

		if     (pt.y < 0)  if(pt.y >=  -ZERO)  pt.y = 0;  else  return  0;
		else if(pt.y > 1)  if(pt.y <= 1+ZERO)  pt.y = 1;  else  return  0;

		return  1;
	 }

	void
	set(const HVector& _sw,
		const HVector& _se,
		const HVector& _ne,
		const HVector& _nw)
	 {
		this->sw = _sw;
		this->se = _se;
		this->ne = _ne;
		this->nw = _nw;
		this->radius = ( sw.normalize() +
						 se.normalize() +
						 ne.normalize() +
						 nw.normalize() ) / 4;

//		chain = new HVector[] { sw, se, ne, nw, sw };
		chain[0] = sw;
		chain[1] = se;
		chain[2] = ne;
		chain[3] = nw;
		chain[4] = sw;

		HVector _s = (sw % se).unit(); // the "real" s is declared final
		clockwise = (_s ^ ne) < 0;

		if(clockwise)
		 {
			s = -_s;
			e = (ne % se).unit();
			n = (nw % ne).unit();
			w = (sw % nw).unit();
		 }
		else
		 {
			s = _s;
			e = (se % ne).unit();
			n = (ne % nw).unit();
			w = (nw % sw).unit();
		 }

//		norms = new HVector[] { s, e, n, w };
		norms[0] = s;
		norms[1] = e;
		norms[2] = n;
		norms[3] = w;

		wePlane = (e % w).unit();
		snPlane = (n % s).unit();
		wePlaneSpan = M_PI - separation(e,w).value;
		snPlaneSpan = M_PI - separation(s,n).value;

		// pre-calculatable stuff for interpolate()

		s_normal = (sw % se).unit();
		n_normal = (nw % ne).unit();
		e_normal = (ne % se).unit();
		w_normal = (nw % sw).unit();

		nperc_axis = (s_normal % n_normal).unit();
		eperc_axis = (w_normal % e_normal).unit();

		ns_maxangle = acos(s_normal ^ n_normal);
		ew_maxangle = acos(e_normal ^ w_normal);
	 }
 };

struct FramedImage
 {
	Cell *cells;
	int count;
	int imageW, imageH;
	int cellH;
	char *imageData;
	char *ghostData;
	int bpp;
	int lastCell;

	FramedImage()
	 : lastCell(0)
	 { }

	// Returns (in lonlat) the lon/lat coordinates of the ghost for
	// pixel x,y (given downtrack offset d and cross-track offset
	// r). The actual function return value is non-zero if the offsets
	// push us past the bounds of the image.
	inline int pixel2ghostll(int x, int y,
							 const int r, const int d,
							 Point2D &lonlat)
	 {
		x -= r; // add in offsets
		y -= d;
		if(x < 0  ||  x >= imageW  ||
		   y < 0  ||  y >= imageH  )
			return  1;

		// Convert the uncorrected pixel to a lon/lat
		lonlat = toLonLat(toSpatial(x, y));

		// Subtract time out of the longitude of the spatial coordinate.
		lonlat.x += (d / (double)PIXEL_FRAC_SEC) * 360.0 / MARS_DAY;

		return  0;
	 }

	HVector toSpatial(int x, int y)
	 {
		if(x < 0  ||  x >= imageW  ||  y < 0  ||  y >= imageH)
		 {
			fprintf(stderr, "WOAH! Bad coordinate requested: %d,%d / %dx%d\n",
					x, y, imageW, imageH);
			return  HVector();
		 }

		int n = y / cellH;
		y %= cellH;
		if(n > count)
		 {
			fprintf(stderr, "WOAH! Cell %d requested, only have %d cells.\n",
					n, count);
			return  HVector();
		 }

		int realCellH = cellH;
		if(n == count-1)
			realCellH = imageH % 256;

		return  cells[n].interpolate(    x / double(   imageW - 1),
									 1 - y / double(realCellH - 1));
	 }

	int whichCell(const HVector& v, Point2D& pt)
	 {
		if(cells[lastCell].contains(v, pt))
			return  lastCell;
		for(int i=0; i<count; i++)
			if(cells[i].contains(v, pt))
				return  lastCell = i;
		return  -1;
	 }

	Point toPixel(const HVector& v)
	 {
		Point2D unitPt;
		int n = whichCell(v, unitPt);
		if(n == -1)
			return  Point(-1, -1);
//  			fprintf(stderr, "%d\t%f\t%f\n", n, unitPt.x, unitPt.y);
//  		else
//  			fprintf(stderr, "%d\n", n);

		int realCellH = cellH;
		if(n == count-1)
			realCellH = imageH % 256;

		return  Point((int) rint(    unitPt.x  * (imageW    - 1)    ),
					  (int) rint( (1-unitPt.y) * (realCellH - 1) ) + n*cellH);
	 }

	double getRadius(int y)
	 {
		if(y < 0  ||  y >= imageH)
		 {
			fprintf(stderr, "WOAH! Bad coordinate requested: %d / %d\n",
					y, imageH);
			return  NAN;
		 }

		int n = y / cellH;
		if(n > count)
		 {
			fprintf(stderr, "WOAH! Cell %d requested2, only have %d cells.\n",
					n, count);
			return  NAN;
		 }

		return  cells[n].radius;
	 }
 };

#define LINES "=============================================================\n"

static int
readCellsFromTrackServer(const char *stamp, FramedImage& fi)
 {
	fi.cells = NULL;

	///////////////////
	// Determine the parameters of the image, from the DB
	///////////////////

	char cmd[1000];
	sprintf(cmd,
			MYSQL_CMD " -B -s -e \"select start, duration from obs where filename='%s'\"",
			stamp);

	FILE *fp = popen(cmd, "r");
	if(!fp)
	 {
		sprintf(ERRMSG,
				LINES,
				"ERROR:"
				"\tUnable to run external mysql command.\n"
				"\n"
				"\tTRIED: %s\n"
				LINES,
				cmd);
		pclose(fp);
		return  0;
	 }

	char line_buff[1000];
	if(feof(fp)  ||  !fgets(line_buff, 1000, fp))
	 {
		sprintf(ERRMSG,
				LINES,
				"ERROR:"
				"\tUnable to find stamp in database!\n"
				"\n"
				"\tTRIED: %s\n"
				LINES,
				cmd);
		pclose(fp);
		return  0;
	 }
	pclose(fp);

	char *eol = strchr(line_buff, '\n');
	if(eol)
		*eol = 0;

	double start;
	double duration;
	int read_items = sscanf(line_buff,
							"%lf %lf",
							&start,
							&duration);
	if(read_items != 2)
	 {
		sprintf(ERRMSG,
				LINES
				"ERROR:"
				"\tUnable to parse geometry database data!\n"
				"\n"
				"\tTRIED: %s\n"
				"\n"
				"\tFAILED WITH THE RESULT LINE:\n"
				"\t[%s]\n"
				LINES,
				cmd, line_buff);
		return  0;
	 }

	///////////////////
	// Grab points from the track server
	///////////////////

	double xmin = start;
	double xdelta = 256/30.;
	int xcount = (int) ceil(duration / xdelta) + 1;
	double ymin = -4/15.;
	double ydelta = 8/15.;
	int ycount = 2;
	sprintf(cmd,
			WGET_CMD " 'http://cutter1.mars.asu.edu/jmars/time_track.phtml"
			"?format=C"
			"&xmin=%f&xdelta=%f&xcount=%d"
			"&ymin=%f&ydelta=%f&ycount=%d'",
			xmin, xdelta, xcount,
			ymin, ydelta, ycount
		);
	fp = popen(cmd, "r");
	if(!fp)
	 {
		sprintf(ERRMSG,
				LINES,
				"ERROR:"
				"\tUnable to run external wget command.\n"
				"\n"
				"\tTRIED: %s\n"
				LINES,
				cmd);
		pclose(fp);
		return  0;
	 }

	HVector pts[3000]; // 256 frames worth of corner pts max, WAY rounded up
	for(int i=0; i<xcount*2; i++)
	 {
		if(feof(fp)  ||  fgets(line_buff, 1000, fp) == NULL)
		 {
			sprintf(ERRMSG,
					LINES
					"ERROR:"
					"\tTrack server returned only %d points! (wanted %d)\n"
					"\n"
					"\tTRIED: %s\n"
					LINES,
					i, xcount*2, cmd);
			pclose(fp);
			return  0;
		 }

		char *eol = strchr(line_buff, '\n');
		if(eol)
			*eol = 0;

		read_items = sscanf(line_buff, "%lf %lf %lf",
							&(pts[i][0]),
							&(pts[i][1]),
							&(pts[i][2]) );

		if(read_items != 3)
		 {
			sprintf(ERRMSG,
					LINES
					"ERROR:"
					"\tUnable to parse track server data!\n"
					"\n"
					"\tTRIED: %s\n"
					"\n"
					"\tFAILED WITH THE RESULT LINE:\n"
					"\t[%s]\n"
					LINES,
					cmd, line_buff);
			pclose(fp);
			return  0;
		 }
	 }

	if(fgets(line_buff, 1000, fp) != NULL)
	 {
		sprintf(ERRMSG,
				LINES
				"ERROR:"
				"\tTrack server returned too much data!\n"
				"\n"
				"\tTRIED: %s\n"
				"\n"
				"\tFIRST LINE OF EXTRA DATA:\n"
				"\t[%s]\n"
				LINES,
				cmd, line_buff);
		pclose(fp);
		return  0;
	 }

	pclose(fp);

	fi.count = xcount - 1;
	fi.cells = new Cell[fi.count];
	for(int i=0; i<xcount-1; i++)
		fi.cells[i].set(pts[i*2+2],
						pts[i*2+3],
						pts[i*2+1],
						pts[i*2  ]);

	return  1;
 }

static int
readCellsFromGeometryDB(const char *stamp, FramedImage& fi)
 {
	static const int reverseLon = 1; // old option, deprecated
	fi.cells = NULL;

	char cmd[1000];
	sprintf(cmd,
			MYSQL_CMD " -B -s -e \"select framelet_id, find_in_set(point_id, 'LL,LR,UL,UR'), %slon, lat, slant_distance from themis3.frmgeom where file_id='%s' and point_id != 'CT' order by framelet_id, concat(point_id)\"",
			reverseLon ? "360-" : "", stamp);

	FILE *fp = popen(cmd, "r");
	if(!fp)
	 {
		sprintf(ERRMSG,
				LINES,
				"ERROR:"
				"\tUnable to run external mysql command.\n"
				"\n"
				"\tTRIED: %s\n"
				LINES,
				cmd);
		return  0;
	 }

	HVector pts[3000]; // 256 frames max, rounded up (TIMES POINTS PER FRAME!)

	// Priming values that would've resulted from record number -1
	int last_frame_id = -1;
	int last_point_type = 4;

	while(!feof(fp))
	 {
		char line_buff[1000];

		if(!fgets(line_buff, 1000, fp))
			break;
		char *eol = strchr(line_buff, '\n');
		if(eol)
			*eol = 0;

		int read_frame_id;
		int read_point_type;
		double read_lon;
		double read_lat;
		double read_dist;
		int read_items;
		read_items = sscanf(line_buff,
							"%d %d %lf %lf %lf",
							&read_frame_id,
							&read_point_type,
							&read_lon,
							&read_lat,
							&read_dist);

		if(read_items != 5)
		 {
			sprintf(ERRMSG,
					LINES
					"ERROR:"
					"\tUnable to parse geometry database data!\n"
					"\n"
					"\tTRIED: %s\n"
					"\n"
					"\tFAILED WITH THE RESULT LINE:\n"
					"\t[%s]\n"
					LINES,
					cmd, line_buff);
			pclose(fp);
			return  0;
		 }

		// Check the values
		if(read_point_type % 4 != (last_point_type + 1) % 4  ||
		   read_frame_id != last_frame_id + (last_point_type==4))
		 {
			sprintf(ERRMSG,
					LINES
					"ERROR:"
					"\tInvalid geometry data in database!\n"
					"\n"
					"\tTRIED: %s\n"
					"\n"
					"\tFAILED FROM BAD FRAME SEQUENCE:\n"
					"[%s]\n"
					LINES,
					cmd, line_buff);
			pclose(fp);
			return  0;
		 }

		// Save them!
		last_frame_id = read_frame_id;
		last_point_type = read_point_type;
		int idx = last_frame_id * 4 + last_point_type - 1;
		pts[idx] = surfaceVector(read_lon, read_lat, read_dist);
	 }
	pclose(fp);

	if(last_frame_id == -1)
	 {
		sprintf(ERRMSG,
				LINES
				"ERROR:"
				"\tNO GEOMETRY DATA FOR %s IN DATABASE!\n"
				"\n"
				"\tTRIED: %s\n"
				LINES,
				stamp, cmd);
		return  0;
	 }

	if(last_point_type != 4)
	 {
		fprintf(stderr,
				LINES
				"ERROR:"
				"\tInvalid geometry data in database!\n"
				"\tStrange final frame: missing some points.\n"
				"\n"
				"\tTRIED: %s\n"
				LINES,
				cmd);
		return  0;
	 }

	// Finally... turn the border points into full-fledged Cell objects
	fi.cells = new Cell[last_frame_id];
	for(int i=1; i<=last_frame_id; i++)
		fi.cells[i-1].set(pts[i * 4 + 0],
						  pts[i * 4 + 1],
						  pts[i * 4 + 3],
						  pts[i * 4 + 2]);

	// Success!
	fi.count = last_frame_id;
	return  1;
 }

/**
 ** The memory allocated for fi.cells must be freed by the caller,
 ** using the delete[] operator.
 **/
static int
readCells(const char *stamp, FramedImage& fi, int useTrackServer)
 {
	if(useTrackServer)
		return  readCellsFromTrackServer(stamp, fi);
	else
		return  readCellsFromGeometryDB(stamp, fi);
 }

static int
readImage(const char *fname, FramedImage& fi)
 {
	FILE *fp = fopen(fname, "r");
	if(!fp)
	 {
		fprintf(stderr, "\nUnable to open the file %s!\n", fname);
		return  0;
	 }

	char magic[10];
	int maxval;
	int count = fscanf(fp, "%5s %d %d %d",
					   magic, &fi.imageW, &fi.imageH, &maxval);
	if(count != 4  ||  strcmp(magic, "P5")  ||  !isspace(fgetc(fp)))
	 {
		fprintf(stderr, "\nUnable to read image file %s!\n", fname);
		fclose(fp);
		return  0;
	 }
	fi.cellH = 256;

	fi.bpp = 1;
	if(maxval >= 256)
	 {
		fprintf(stderr, "\nCan't read 16-bit values!\n");
		fclose(fp);
		return  0;
	 }

	int dataLength = fi.imageW * fi.imageH;
	fi.imageData = (char*) malloc(dataLength);
	fi.ghostData = (char*) calloc(dataLength, 1);
	if(fread(fi.imageData, 1, dataLength, fp) != dataLength)
	 {
		if(feof(fp))
			fprintf(stderr, "\nEarly EOF!\n");
		else
			fprintf(stderr, "\nI/O error while reading file!\n");
		fclose(fp);
		return  0;
	 }

	fclose(fp);
	return  1;
 }

static void
createGhost(FramedImage& fi, int gd0, int gr, const char *msg,
			double REF_DISTANCE)
 {
	double dist = NAN;
	int gd = gd0;
	Point2D lonlat;
	char notice[100];
	FILE *log = NULL;
	char *logfname = getenv("DVGHOST_LOG");

	if(logfname)
	 {
		fprintf(stderr, "Verbose logging to file $DVGHOST_LOG = %s\n",
				logfname);
		log = fopen(logfname, "w");
		if(!log)
			perror("UNABLE TO OPEN $DVGHOST_LOG");
	 }
	else
		fprintf(stderr, "Set $DVGHOST_LOG if you want a verbose log file.\n");

	if(log)
		fprintf(log, "#frame\tline\tsample\tlon\tlat\tdist\toffset\n");

	for(int gy=0; gy<fi.imageH; gy++)
	 {
		if(msg  &&  gy % 10 == 0)
		 {
			sprintf(notice, "%s %d / %d (dist=%.1fkm, offset=%d->%d)",
					msg, gy, fi.imageH, dist, gd0, gd);
			fprintf(stderr, "\r%s ", notice);
		 }

		// Use cell interpolation for the spacecraft vector's
		// direction, and the stored average cell radius for its
		// magnitude.
		HVector spacecraft = fi.toSpatial(fi.imageW/2, gy) * fi.getRadius(gy);

		for(int gx=0; gx<fi.imageW; gx++)
		 {
			// Calculate ghost lon/lat without altitude compensation,
			// then use mola to derive a radial vector.
			if(fi.pixel2ghostll(gx, gy, gr, gd0, lonlat)) // output in lonlat
				continue; // ghosted to outside the image
			HVector target = molaVector(lonlat.x, lonlat.y);

			// Calculate distance to target, revise downtrack offset.
			dist = (spacecraft - target).norm();
			gd = (int) round(gd0 * dist / REF_DISTANCE);

			if(log  &&  gx%8==0  &&  gy%8==0)
				fprintf(log, "%d\t%d\t%d\t%f\t%f\t%f\t%d\n",
						gy / 256, gy, gx, lonlat.x, lonlat.y, dist, gd);

			// Re-calculate ghost lon/lat with the distance-compensated offset
			if(fi.pixel2ghostll(gx, gy, gr, gd, lonlat)) // output in lonlat
				continue; // ghosted to outside the image

			// Finally, interpolate the latest lonlat back to an image pixel
			Point sourcePt = fi.toPixel(toVec(lonlat));

			int ix = sourcePt.x;
			int iy = sourcePt.y;
			if(ix == -1)
				continue;

			// Paint the ghost from the source image pixel
			int dst = (gy * fi.imageW + gx) * fi.bpp;
			int src = (iy * fi.imageW + ix) * fi.bpp;
//			if(bpp == 1)
//				fi.ghostData[dst] = fi.imageData[src];
//			else
			memcpy(fi.ghostData+dst, fi.imageData+src, fi.bpp);
		 }
	 }
	if(msg)
		fprintf(stderr, "\r%*s\r", strlen(notice), "");
 }

static void
writeGhost(FramedImage& fi, FILE *fp)
 {
	fprintf(fp, "P5\n%d %d\n255\n", fi.imageW, fi.imageH);
	fwrite(fi.ghostData, 1, fi.imageW*fi.imageH, fp);
 }

static void
dumpCells(FramedImage &fi)
 {
	for(int i=0; i<fi.count; i++)
	 {
		for(int c=0; c<4; c++)
		 {
			Point2D lonlat = toLonLat(fi.cells[i].chain[c]);
			printf("%f\t%f\n", lonlat.x, lonlat.y);
		 }
		printf("\n");
	 }
	exit(0);
 }

static void
testInterpolate(Cell& cell)
 {
	for(int c=0; c<4; c++)
	 {
		Point2D lonlat = toLonLat(cell.chain[c]);
		printf("%f\t%f\n", lonlat.x, lonlat.y);
	 }
	printf("\n");
	printf("75 -0.4\n");

	for(int y=0; y<5; y++)
		for(int x=0; x<5; x++)
		 {
			HVector v = cell.interpolate(x / 4.0, y / 4.0);
			Point2D lonlat = toLonLat(v);
			printf("%f\t%f\n", lonlat.x, lonlat.y);
		 }
	exit(0);
 }

static void
testUninterpolate(Cell& cell)
 {
	printf("%f\t%f\n", 0.1, 0.1);
	for(int y=0; y<5; y++)
		for(int x=0; x<5; x++)
		 {
			HVector v = cell.interpolate(x / 4.0, y / 4.0);
			Point2D unitPt = cell.uninterpolate(v);
			printf("%f\t%f\n", unitPt.x, unitPt.y);
		 }
	exit(0);
 }

static double getRefDistance(int gd, int gr, int useTrackServer)
 {
	static double distances[2] = { 0, 0 };

	useTrackServer = !!useTrackServer; // forces zero or one

	if(distances[useTrackServer] == 0)
	 {
		FramedImage fi;
		fi.imageW = 320;
		fi.imageH = 999999;
		fi.cellH = 256;
		readCells(REFERENCE_IMAGE, fi, useTrackServer);

		int count = 0;
		for(int i=0; i<fi.count-1; i++) // ignore the last (partial) cell
		 {
			double dist;
			int gx = fi.imageW / 2;
			int gy = fi.cellH/2 + i*fi.cellH;
			Point2D lonlat;

			// Use cell interpolation for the spacecraft vector's
			// direction, and the stored average cell radius for its
			// magnitude.
			HVector spacecraft =
				fi.toSpatial(fi.imageW/2, gy) * fi.getRadius(gy);
//			fprintf(stderr, "Spacecraft radius: %f\n", spacecraft.norm());
//			fprintf(stderr, "Cell radius: %f\n", fi.getRadius(gy));

			// Calculate ghost lon/lat without altitude compensation,
			// then use mola to derive a radial vector.
			if(fi.pixel2ghostll(gx, gy, gr, gd, lonlat)) // output in lonlat
				continue; // ghosted to outside the image
			HVector target = molaVector(lonlat.x, lonlat.y);
//			fprintf(stderr, "target radius: %f\n", target.norm());
//			fprintf(stderr, "mola radius: %f\n", getMolaRadius(lonlat.x,
//															   lonlat.y));

			// Calculate distance to target, revise downtrack offset.
			dist = (spacecraft - target).norm();

//			fprintf(stderr, "Distance = %f\n", dist);
			distances[useTrackServer] += dist;
			++count;
		 }
		distances[useTrackServer] /= count;

		fprintf(stderr, "Calculated reference distance for %s as %.1f\n",
				REFERENCE_IMAGE,
				distances[useTrackServer]);

		delete[] fi.cells;
	 }

	return  distances[useTrackServer];
 }

/**
 ** Writes into ghostDataBuff the predicted ghost image. A null return
 ** value indicates success, a non-null return value contains an error
 ** message suitable for direct user display. Any returned error
 ** message comes from a static buffer and is overwritten on
 ** successive calls.
 **
 ** The ghostDown and ghostRight arguments specify how far down and/or
 ** to the right (in pixels) the ghost image appears. The msg argument
 ** (if non-null) specifies a string to prefix the progress meter
 ** with.
 **
 ** The passed-in ghostDataBuff must already be initialized to zero
 ** (or whatever value you desire for "blank" areas of the ghost).
 **/
extern "C"
char *
createThemisGhostImage(int ghostDown,
					   int ghostRight,
					   const char *msg,
					   const char *stamp_id,
					   int imageW,
					   int imageH,
					   int bytesPerPixel,
					   void *imageDataBuff,
					   void *ghostDataBuff,
					   int useTrackServer)
 {
	FramedImage fi;
	fi.imageW = imageW;
	fi.imageH = imageH;
	fi.cellH = 256;
	fi.imageData = (char *) imageDataBuff;
	fi.ghostData = (char *) ghostDataBuff;
	fi.bpp = bytesPerPixel;

	if(!readCells(stamp_id, fi, useTrackServer))
		return  ERRMSG;

	double REF_DISTANCE = getRefDistance(ghostDown, ghostRight,
										 useTrackServer);


/*MCW
	for(int i=0; i<fi.count; i++)
	 {
		for(int c=0; c<4; c++)
			printf("%f\t%f\t%f\n",
				   fi.cells[i].chain[c][0],
				   fi.cells[i].chain[c][1],
				   fi.cells[i].chain[c][2] );
		printf("\n");
	 }
	exit(0);
*/

	createGhost(fi, ghostDown, ghostRight, msg, REF_DISTANCE);

	delete[] fi.cells;

	return  NULL;
 }

int
test_main()
 {
	char buff[100];
	while(fgets(buff, sizeof(buff), stdin))
	 {
		buff[strlen(buff)-1] = 0;
		FramedImage fi;
		fi.imageW = 320;
		fi.imageH = 999999;
		fi.cellH = 256;
		Point2D lonlat;

		int i = 0;
		int gr = 0;
		int gd = 80;

		double dist0;
		if(!readCells(buff, fi, 0))
		 {
			fprintf(stderr, "Skipping %s:\n%s\n", buff, ERRMSG);
			continue;
		 }
		else
		 {
			int gx = fi.imageW / 2;
			int gy = fi.cellH/2 + i*fi.cellH;

			// Use cell interpolation for the spacecraft vector's
			// direction, and the stored average cell radius for its
			// magnitude.
			HVector spacecraft =
				fi.toSpatial(fi.imageW/2, gy) * fi.getRadius(gy);

			// Calculate ghost lon/lat without altitude compensation,
			// then use mola to derive a radial vector.
			if(fi.pixel2ghostll(gx, gy, gr, gd, lonlat)) // output in lonlat
				continue; // ghosted to outside the image
			HVector target = molaVector(lonlat.x, lonlat.y);

			// Calculate distance to target, revise downtrack offset.
			dist0 = (spacecraft - target).norm();

			printf("%f\t%f\t", lonlat.y, dist0);
			delete[] fi.cells;
		 }

		double dist1;
		if(!readCells(buff, fi, 1))
		 {
			fprintf(stderr, "Skipping2 %s: %s\n", buff, ERRMSG);
			continue;
		 }
		else
		 {
			int gx = fi.imageW / 2;
			int gy = fi.cellH/2 + i*fi.cellH;

			// Use cell interpolation for the spacecraft vector's
			// direction, and the stored average cell radius for its
			// magnitude.
			HVector spacecraft =
				fi.toSpatial(fi.imageW/2, gy) * fi.getRadius(gy);

			// Calculate ghost lon/lat without altitude compensation,
			// then use mola to derive a radial vector.
			if(fi.pixel2ghostll(gx, gy, gr, gd, lonlat)) // output in lonlat
				continue; // ghosted to outside the image
			HVector target = molaVector(lonlat.x, lonlat.y);

			// Calculate distance to target, revise downtrack offset.
			dist1 = (spacecraft - target).norm();

			printf("%f\t%f\n", lonlat.y, dist1);
			delete[] fi.cells;
		 }
	 }
 }

#ifdef GHOST_MAIN
int main(int ac, char *av[])
 {
/*
	++av;
	--ac;
	if(!strcmp(av[ac-1],"lsb"))
	 {
		--ac;
		MSB = 1;
		LSB = 0;
	 }
		
	while(ac)
	 {
		double lon = atof(av[0]);
		double lat = atof(av[1]);
		printf("%g W\t%g\t", lon, lat);
		printf("%f\n", getMolaRadius(lon, lat));
		av += 2;
		ac -= 2;
	 }
	exit(-1);
*/
	if(ac != 4)
	 {
		fprintf(stderr, "USAGE: stampid.pgm ghostDown {ts|db}\n");
		return  -1;
	 }

	char *image_fname = av[1];
	char *stamp_id = strdup(av[1]);  stamp_id[9] = 0;
	int down = atoi(av[2]);
	int useTrackServer = !strcmp(av[3],"ts");

	FramedImage fi;

	if(!readImage(image_fname, fi))
	 {
		fprintf(stderr, "Failed to read image!\n");
		return  -1;
	 }
	fprintf(stderr, "Read image! %dx%d\n", fi.imageW, fi.imageH);

	char * err = createThemisGhostImage(down, 0, "Ghosting row", stamp_id,
										fi.imageW, fi.imageH, fi.bpp,
										fi.imageData, fi.ghostData,
										useTrackServer);
	if(err)
	 {
		fprintf(stderr, "%s", err);
		exit(-1);
	 }
	fprintf(stderr, "Created ghost!                                \n");

	writeGhost(fi, stdout);
	fprintf(stderr, "Wrote ghost image file to stdout!\n");
 }
#endif

#endif

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

#define MYSQL_CMD "dblogin2"
#define WGET_CMD "wget -q -O -"

// For every pixel DOWN in image for the ghost, we ADD this much
// fraction of a second in degrees west longitude to the longitude.
#define PIXEL_FRAC_SEC 30.0

// Number of seconds in a martian day
#define MARS_DAY 88642.663

// Defines rounding tolerance for the edges of the framelets
#define ZERO (1.0/512)

static char ERRMSG[5000];

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

static HVector
toVec(Point2D& pt)
 {
	double& lon = pt.x;
	double& lat = pt.y;
	return
		HVector(
			cos(toRadians(lat)) * cos(toRadians(-lon)),
			cos(toRadians(lat)) * sin(toRadians(-lon)),
			sin(toRadians(lat))
			);
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
		HVector s_normal = (sw % se).unit();
		HVector n_normal = (nw % ne).unit();
		HVector e_normal = (ne % se).unit();
		HVector w_normal = (nw % sw).unit();

		HVector nperc_axis = (s_normal % n_normal).unit();
		HVector eperc_axis = (w_normal % e_normal).unit();

		double ns_maxangle = acos(s_normal ^ n_normal);
		double ew_maxangle = acos(e_normal ^ w_normal);

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
	set(const HVector& sw,
		const HVector& se,
		const HVector& ne,
		const HVector& nw)
	 {
		this->sw = sw;
		this->se = se;
		this->ne = ne;
		this->nw = nw;

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
			WGET_CMD " 'http://mapserver1.la.asu.edu/jmars/time_track.phtml"
			"?format=c"
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
						pts[i*2  ] );

	return  1;
 }

static int
readCellsFromGeometryDB(const char *stamp, FramedImage& fi)
 {
	static const int reverseLon = 1; // old option, deprecated
	fi.cells = NULL;

	char cmd[1000];
	sprintf(cmd,
			MYSQL_CMD " -B -s -e \"select frame_id, find_in_set(point_type, 'LL,LR,UL,UR'), %slon, lat from geometry_detail where filename='%s' and point_type != 'CT' order by frame_id, point_type\"",
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

	Point2D pts[3000]; // 256 frames max, rounded up (TIMES POINTS PER FRAME!)

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
		int read_items;
		read_items = sscanf(line_buff,
							"%d %d %lf %lf",
							&read_frame_id,
							&read_point_type,
							&read_lon,
							&read_lat);

		if(read_items != 4)
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
		pts[last_frame_id * 4 + last_point_type - 1].x = read_lon;
		pts[last_frame_id * 4 + last_point_type - 1].y = read_lat;
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
		fi.cells[i-1].set(toVec(pts[i * 4 + 0]),
						  toVec(pts[i * 4 + 1]),
						  toVec(pts[i * 4 + 3]),
						  toVec(pts[i * 4 + 2]));

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
createGhost(FramedImage& fi, int gd, int gr, const char *msg)
 {
	for(int gy=0; gy<fi.imageH; gy++)
	 {
		if(msg  &&  gy % 10 == 0)
			fprintf(stderr, "\r%s %d / %d", msg, gy, fi.imageH);
		for(int gx=0; gx<fi.imageW; gx++)
		 {
			// Offset from target ghost pixel to an uncorrected source
			// image pixel.
			int ix = gx - gr;
			int iy = gy - gd;
			if(ix < 0  ||  ix > fi.imageW  ||
			   iy < 0  ||  iy > fi.imageH  )
				continue;

			// Convert the uncorrected pixel to a spatial coordinate.
			HVector spatial = fi.toSpatial(ix, iy);

			// Subtract time out of the longitude of the spatial coordinate.
			Point2D lonlat = toLonLat(spatial);
			lonlat.x += (gd / (double)PIXEL_FRAC_SEC) * 360.0 / MARS_DAY;
			spatial = toVec(lonlat);

			// Convert back to pixel coordinates, this is the actual
			// imaged pixel for the ghost.
//			fprintf(stderr, "%d %d\t", ix, iy);
			Point sourcePt = fi.toPixel(spatial);
			ix = sourcePt.x;
			iy = sourcePt.y;
//  			if(gx != ix  ||  gy != iy)
//  			 {
//  				printf("%d %d", gx, gy);
//  				printf(" %d", fi.whichCell(spatial));
//  				printf("    %d %d\n", ix, iy);
//  			 }
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
		fprintf(stderr, "\r%*s\r",
				int( strlen(msg) + ceil(log10(fi.imageH)) * 2 + 2),
				"");
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

	createGhost(fi, ghostDown, ghostRight, msg);

	delete[] fi.cells;

	return  NULL;
 }

#ifdef GHOST_MAIN
main(int ac, char *av[])
 {
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

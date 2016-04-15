#ifndef ISIS3INCLUDE_H_
#define ISIS3INCLUDE_H_

#define DV_NAMEBUF_MAX 1025

typedef enum _ISIS3ENUM {
	NOTHING = 0,
	I3Tile,
	BandSequential,
	UnsignedByte,
	SignedWord,
	Real,
	Lsb,
	Msb,
	NO_OBJECT,
	CUBEOBJECT,
	COREOBJECT,
	LABELOBJECT,
	HISTORYOBJECT,
	ORIGINALLABELOBJECT,
	UNKNOWN_OBJECT,
	ORIGINALBAND,
	CENTER,
	WIDTH,
	FILTERNUMBER,
	NO_GROUP,
	UNKNOWN_GROUP,
	Dimensions,
	Pixels,
	Instrument,
	Archive,
	BandBin,
	Mapping

} ISIS3ENUM;

typedef struct minISISINFO {

	/* Core */
	int StartByte;
	char *Format;
	int TileSamples;
	int TileLines;

	/* Dimensions */
	int Samples;
	int Lines;
	int Bands;

	/* Pixels */
	char *Type;
	char *ByteOrder;
	double Base;
	double Multiplier;

	/* Label */
	int Bytes;

} minIsisInfo;


typedef struct _IsisCube {
	// Object == Core
	char Core[128];		// filename if data in separate file
	int StartByte;		// offset of the start of data (1 based)
	int Format;			// Tile or BandSequential, use enum for string value
	int TileSamples;	// must be non zero if Format == Tile
	int TileLines;		// must be non zero if Format == Tile

	// Group == Dimensions
	int Samples;		// These three keywords define
	int Lines;			// the size of the cube
	int Bands;

	// Group == Pixels
	int Type;			// UnsignedByte, SignedWord,    Real  <-- These are the three possible descriptive enum's
						// 0 - 255     , signed 16-bit, 32 bit IEEE float
						// use enum for string

	int ByteOrder;		// Lsb or Msb, use enum for string
	double Base;		// realDn = diskDN * Multiplier + Base
	double Multiplier;  // These are always 0.0 and 1.0 for Type == Real

	// Group == Instrument

    char SpacecraftName[128];       // the name of the space craft
    char InstrumentId[128];         // the name of the instrument
    char TargetName[128];           // the name of the target
    char MissionPhaseName[128];     // the name of the mission phase
    char StartTime[128];            // start time as string
    char StopTime[128];             // stop time as string
    char SpacecraftClockCount[128]; // Example I had was 699921894.230, I'm gonna try a string
    int GainNumber;                 // Gain as int
    int OffsetNumber;               // Offset as int
    int MissingScanLines;           // missing as int
    char TimeDelayIntegration[64];  // Delay integration as string
    double SpacecraftClockOffset;   // I'm going to try a double here


	// Group == AlphaCube
	int AlphaSamples;	// The number of samples and lines in the original parent/alpha cube
	int AlphaLines;

	// Group == Archive
	char ImageNumber[128];			// Optional keywords that identify archival information
	char ProductId[128];
	char DataSetId[128];
	char ProducerId[128];
	char ProductVersionId[128];
	char InstrumentHostName[128];
	char InstrumentName[128];
	char ProductCreationTime[128];
	char UploadId[128];
	char ImageId[128];
	char RationalDescription[128];
	int OrbitNumber;               // Thought a number might be useful here
	char FlightSoftwareVersionId[65];
	int CommandSequenceNumber;
	char ArchiveDescription[128];

	// Group == BandBin            // These are kept as CSV lists TODO: array of numbers
	char BandBinOriginalBand[256];
	char BandBinCenter[256];
	char BandBinWidth[256];
	char BandBinFilterNumber[256];

	// Group == Mapping
	char	Mapping_ProjectionName[128];
	char	Mapping_TargetName[128];
	double	Mapping_EquatorialRadius;
	char	Mapping_EquatorialRadiusUnits[128];
	double	Mapping_PolarRadius;
	char	Mapping_PolarRadiusUnits[128];
	char	Mapping_LatitudeType[128];
	char	Mapping_LongitudeDirection[128];
	double	Mapping_LongitudeDomain;
	double	Mapping_MinimumLatitude;
	double	Mapping_MaximumLatitude;
	double	Mapping_MinimumLongitude;
	double	Mapping_MaximumLongitude;
	double	Mapping_PixelResolution;
	char	Mapping_PixelResolutionUnits[128];
	double	Mapping_Scale;
	char	Mapping_ScaleUnits[128];
	double	Mapping_TrueScaleLatitude;
	double	Mapping_CenterLongitude;
	double	Mapping_LineProjectionOffset;
	double	Mapping_SampleProjectionOffset;
	double	Mapping_UpperLeftCornerX;
	char	Mapping_UpperLeftCornerXUnits[128];
	double	Mapping_UpperLeftCornerY;
	char	Mapping_UpperLeftCornerYUnits[128];

} Isis3Cube;

typedef struct _HistoryObject { // "History", "OriginalLabel"

	char Name[128]; // "IsisCube" most likely
	int StartByte;
	int Bytes;

} HistoryObject;


typedef struct _LabelObject {
	int Bytes;
} LabelObject;



struct stringStack /* Structure definition for a string stack */
{
char *stk[5]; // stack is 5 deep
int top;
};

typedef struct stringStack STRINGSTACK;

#endif /* ISIS3INCLUDE_H_ */

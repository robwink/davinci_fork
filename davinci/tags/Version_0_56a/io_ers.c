#include "parser.h"

/**
 ** Write data as ER Mapper ERS data set
 **/

char ers_header[] = "DatasetHeader Begin\n\
	Version = \"5.0\"\n\
	DataSetType = ERStorage\n\
	DataType = Raster\n\
	ByteOrder = MSBFirst\n\
	CoordinateSpace Begin\n\
		Datum = \"RAW\"\n\
		Projection = \"RAW\"\n\
		CoordinateType = RAW\n\
		Rotation = 0:0:0.0\n\
	CoordinateSpace End\n\
	RasterInfo Begin\n\
		CellType = %s\n\
		NrOfLines = %d\n\
		NrOfCellsPerLine = %d\n\
		NrOfBands = %d\n\
	RasterInfo End\n\
DatasetHeader End\n";

int
WriteERS(Var *s, FILE *fp, char *filename)
{
    /**
     **/
    char format[256];
    char path[256];
    int x,y,z;

	strcpy(path, filename);

    x = GetSamples(V_SIZE(s), V_ORG(s));
    y = GetLines(V_SIZE(s), V_ORG(s));
    z = GetBands(V_SIZE(s), V_ORG(s));

    if (V_TYPE(s) != ID_VAL) {
        sprintf(error_buf, "Var is not a value: %s", V_NAME(s));
        parse_error(NULL);
        return 0;
    }

	if (z > 1 && V_ORG(s) != BIL) {
        parse_error("ERS files must be BIL format");
        return 0;
    }

    if (fp != NULL) {
		fclose(fp);
    }

	if ((fp = fopen(path, "w")) == NULL) {
		return 0;
	}

    /**
     ** write data
     **/
    fwrite(V_DATA(s), NBYTES(V_FORMAT(s)), V_DSIZE(s), fp);
    fclose(fp);

	strcat(path, ".ers");
	if ((fp = fopen(path, "w")) == NULL) {
		return 0;
	}

    switch(V_FORMAT(s)) {
      case BYTE:        sprintf(format, "Unsigned8BitInteger"); break;
      case SHORT: 		sprintf(format, "Signed16BitInteger"); break;
      case INT: 		sprintf(format, "Signed32BitInteger"); break;
      case FLOAT: 		sprintf(format, "IEEE4ByteReal"); break;
      case DOUBLE:		sprintf(format, "IEEE8ByteReal"); break;
    }

	fprintf(fp, ers_header, format, y, x, z);
    fclose(fp);

    return (0);
}

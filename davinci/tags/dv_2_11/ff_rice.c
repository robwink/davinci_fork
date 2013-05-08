#include "parser.h"

Var *
ff_rice(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	int x, y, z, nbytes, i, j, k, pos, start, len;
	short *in;
	char *out;
	int header = 1;
	int bits = -1;

    Alist alist[4];
	alist[0] = make_alist("object",    ID_VAL,     NULL,    &obj);
	alist[1] = make_alist("header",    INT,        NULL,    &header);
	alist[2] = make_alist("bits",      INT,        NULL,    &bits);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object or size specified", func->name);
        return(NULL);
    }

	x = GetX(obj);
	y = GetY(obj);
	z = GetZ(obj);
	nbytes = NBYTES(V_FORMAT(obj));
	start = 0;
	if (bits == -1) bits = nbytes*8;

	if (x > 4096) {
		parse_error("%s: Max buffer length is 4096 elements", func->name);
		return(NULL);
	}
	if (nbytes > 2) {
		parse_error("Only able to compress 2 byte words or less");
		return(NULL);
	}
	if (nbytes == 2 && x > 511) {
		parse_error("Too many values in a row.  Split this up.");
		return(NULL);
	}

	in = calloc(x, 2);
	out = calloc(x*y*z, 2*nbytes);

	if (header) {
		unsigned short sx = x, sy = y, sz = z;
		unsigned char sbits = bits;

		/* write out header */
		if (x > 65535 || y > 65535 || z > 65535) {
			parse_error("data block too big for header");
			return(NULL);
		}

		memcpy(out, "RICE", 4); start += 4;
		memcpy(out+start, &sx, 2); start += 2;
		memcpy(out+start, &sy, 2); start += 2;
		memcpy(out+start, &sz, 2); start += 2;
		memcpy(out+start, &sbits, 1); start += 1;
	}

	for (k = 0 ; k < z ; k++) {
		for (j = 0 ; j < y ; j++) {
			/*
			** Pack a temporary array with a whole line
			*/
			for (i = 0 ; i < x ; i++) {
				pos = cpos(i, j, k, obj);
				in[i] = extract_int(obj, pos);
			}
			len = rice_auto(in, x, bits, out+start, 0);
			start += (len+7)/8;
		}
	}
	return(newVal(BSQ, start, 1, 1, BYTE, out));
}

Var *
ff_unrice(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	int x, y, z, nbytes, i, start, len;
	int bits;
	unsigned char *in;
	int header = 1;
	int npts;
	unsigned short sx = x, sy = y, sz = z;
	unsigned char sbits = bits;
	char hdr[4];
	int format;
	int count = 0;
	int l;
	unsigned char *cout, *out;
	short *sout, *tmp;

    Alist alist[4];
	alist[0] = make_alist("object",    ID_VAL,     NULL,    &obj);
	alist[1] = make_alist("header",    ID_VAL,     NULL,    &header);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object or size specified", func->name);
        return(NULL);
    }

	len = V_DSIZE(obj);
	in = V_DATA(obj);
	start = 0;


	if (header) {
		/* read our header */
		memcpy(hdr, in, 4); start += 4;

		if (memcmp(hdr, "RICE", 4) != 0) {
			parse_error("Not a rice header");
			return(NULL);
		}

		memcpy(&sx, in+start, 2); start += 2;
		memcpy(&sy, in+start, 2); start += 2;
		memcpy(&sz, in+start, 2); start += 2;
		memcpy(&sbits, in+start, 1); start += 1;

		x = sx;
		y = sy;
		z = sz;
		bits = sbits;
		nbytes = (bits+7)/8;
		npts = x;

		out = (char *)calloc(x*y*z, nbytes);
		cout = (char *)out;
		sout = (short *)out;
		tmp = (short *)calloc(x,2);
		count = 0;

		while (start < len) {
			/*
			** we assume that each rice compressed packet is a length of X
			**
			** We should do some error checking here somewhere.
			*/
			l = rice_unauto(in+start, len, npts, bits, tmp);
			if (l <= 0) {
				parse_error("Bad return code");
				return(NULL);
			}
			for (i = 0 ; i < npts ; i++) {
				if (nbytes == 1) {
					cout[i+count*x] = tmp[i];
				} else {
					sout[i+count*x] = tmp[i];
				}
			}
			start += l;
			count++;
		}
		format = (nbytes == 1 ? BYTE : (nbytes == 2 ? SHORT : INT));
		return(newVal(BSQ, x, y, z, format, out));
	} else {
		parse_error("Data without header not supported yet.");
		return(NULL);
	}
}

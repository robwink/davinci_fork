#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include "iomedley.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** Writes data into the specified file in the current
** machine's native format.
** Move this to iom_raw.c and fix code appropriately to
** write data for either-ended machine.
*/
int iom_WriteRaw(char* fname, void* data, struct iom_iheader* h, int force_write)
{
	long item_ct_out, item_ct_in;
	FILE* fp = NULL;

	if (!force_write && file_exists(fname)) {
		if (iom_is_ok2print_errors()) {
			fprintf(stderr, "File %s already exists.\n", fname);
		}
		return 0;
	}

	if ((fp = fopen(fname, "wb")) == NULL) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "Unable to write %s. Reason: %s.\n", fname, strerror(errno));
		}
		return 0;
	}

	item_ct_in  = iom_iheaderDataSize(h);
	item_ct_out = fwrite(data, iom_NBYTESI(h->format), item_ct_in, fp);

	if (item_ct_in != item_ct_out) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "Failed to write to %s. Reason: %s.\n", fname, strerror(errno));
		}
		fclose(fp);
		unlink(fname);
		return 0;
	}

	fclose(fp);

	return (1);
}


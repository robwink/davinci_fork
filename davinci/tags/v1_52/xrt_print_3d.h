/******************************************************************************
*
* Copyright (c) 1999, KL GROUP INC.  All Rights Reserved.
* http://www.klgroup.com
*
* This file is provided for demonstration and educational uses only.
* Permission to use, copy, modify and distribute this file for
* any purpose and without fee is hereby granted, provided that the
* above copyright notice and this permission notice appear in all
* copies, and that the name of KL Group not be used in advertising
* or publicity pertaining to this material without the specific,
* prior written permission of an authorized representative of
* KL Group.
*
* KL GROUP MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY
* OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
* PURPOSE, OR NON-INFRINGEMENT. KL GROUP SHALL NOT BE LIABLE FOR ANY
* DAMAGES SUFFERED BY USERS AS A RESULT OF USING, MODIFYING OR
* DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
*
******************************************************************************/

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *
 *  Include file for the print dialog.  
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
*/

#ifndef _H_PRINT3D
#define	_H_PRINT3D

#if defined(PRINT_3D) && defined(NDEBUG)
static char sccsid_h[] = "@(#)print_3d.h	1.17 98/12/08 KL Group Inc.";
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/* function declarations */
#ifdef _NO_PROTO
extern void MtShow3dPrintDialog();
#else /* _NO_PROTO */
extern void MtShow3dPrintDialog(Widget graph_to_print, char *default_printer,char *default_file);
#endif /* _NO_PROTO */


#if defined(__cplusplus)
}
#endif

#endif /* _H_PRINT3D */

/* libpbm.h - internal header file for libpbm portable bitmap library
*/

#ifndef _LIBPBM_H_
#define _LIBPBM_H_

/* Here are some routines internal to the pbm library. */

char pbm_getc ARGS(( FILE* file ));
unsigned char pbm_getrawbyte ARGS(( FILE* file ));
int pbm_getint ARGS(( FILE* file ));

int pbm_readmagicnumber ARGS(( FILE* file ));

EXPCL_PANDA void pbm_readpbminitrest( FILE* file, int* colsP, int* rowsP );

#endif /*_LIBPBM_H_*/

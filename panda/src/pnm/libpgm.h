/* libpgm.h - internal header file for libpgm portable graymap library
*/

#ifndef _LIBPGM_H_
#define _LIBPGM_H_

/* Here are some routines internal to the pgm library. */

EXPCL_PANDA void pgm_readpgminitrest( FILE* file, int* colsP, int* rowsP, gray* maxvalP );

#endif /*_LIBPGM_H_*/

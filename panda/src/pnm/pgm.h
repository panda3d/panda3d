/* pgm.h - header file for libpgm portable graymap library
*/

#ifndef _PGM_H_
#define _PGM_H_

#include <pandabase.h>

#include "pbm.h"

#ifdef PGM_BIGGRAYS
typedef unsigned short gray;
#define PGM_MAXMAXVAL 65535
#else /*PGM_BIGGRAYS*/
typedef unsigned char gray;
#define PGM_MAXMAXVAL 255
#endif /*PGM_BIGGRAYS*/


/* Magic constants. */

#define PGM_MAGIC1 'P'
#define PGM_MAGIC2 '2'
#define RPGM_MAGIC2 '5'
#define PGM_FORMAT (PGM_MAGIC1 * 256 + PGM_MAGIC2)
#define RPGM_FORMAT (PGM_MAGIC1 * 256 + RPGM_MAGIC2)
#define PGM_TYPE PGM_FORMAT


/* Macro for turning a format number into a type number. */

#define PGM_FORMAT_TYPE(f) ((f) == PGM_FORMAT || (f) == RPGM_FORMAT ? PGM_TYPE : PBM_FORMAT_TYPE(f))


/* Declarations of routines. */

void pgm_init ARGS(( int* argcP, char* argv[] ));

#define pgm_allocarray( cols, rows ) ((gray**) pm_allocarray( cols, rows, sizeof(gray) ))
#define pgm_allocrow( cols ) ((gray*) pm_allocrow( cols, sizeof(gray) ))
#define pgm_freearray( grays, rows ) pm_freearray( (char**) grays, rows )
#define pgm_freerow( grayrow ) pm_freerow( (char*) grayrow )

gray** pgm_readpgm ARGS(( FILE* file, int* colsP, int* rowsP, gray* maxvalP ));
void pgm_readpgminit ARGS(( FILE* file, int* colsP, int* rowsP, gray* maxvalP, int* formatP ));
void pgm_readpgmrow ARGS(( FILE* file, gray* grayrow, int cols, gray maxval, int format ));

void pgm_writepgm ARGS(( FILE* file, gray** grays, int cols, int rows, gray maxval, int forceplain ));
void pgm_writepgminit ARGS(( FILE* file, int cols, int rows, gray maxval, int forceplain ));
void pgm_writepgmrow ARGS(( FILE* file, gray* grayrow, int cols, gray maxval, int forceplain ));

extern gray pgm_pbmmaxval;
/* This is the maxval used when a PGM program reads a PBM file.  Normally
** it is 1; however, for some programs, a larger value gives better results
*/

#endif /*_PGM_H_*/

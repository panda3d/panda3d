/*\
 * bitio.h - bitstream I/O
 *
 * Works for (sizeof(unsigned long)-1)*8 bits.
 *
 * Copyright (C) 1992 by David W. Sanderson.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  This software is provided "as is"
 * without express or implied warranty.
\*/

#ifndef _BITIO_H_
#define _BITIO_H_

#include "pandabase.h"
#include "pnmimage_base.h"

typedef struct bitstream *BITSTREAM;

/*
 * pm_bitinit() - allocate and return a BITSTREAM for the given FILE*.
 *
 * mode must be one of "r" or "w", according to whether you will be
 * reading from or writing to the BITSTREAM.
 *
 * Returns 0 on error.
 */

extern EXPCL_PANDA_PNMIMAGE BITSTREAM pm_bitinit(std::istream *f, const char *mode);
extern EXPCL_PANDA_PNMIMAGE BITSTREAM pm_bitinit(std::ostream *f, const char *mode);

/*
 * pm_bitfini() - deallocate the given BITSTREAM.
 *
 * You must call this after you are done with the BITSTREAM.
 *
 * It may flush some bits left in the buffer.
 *
 * Returns the number of bytes written, -1 on error.
 */

extern EXPCL_PANDA_PNMIMAGE int pm_bitfini(BITSTREAM b);

/*
 * pm_bitread() - read the next nbits into *val from the given file.
 *
 * Returns the number of bytes read, -1 on error.
 */

extern EXPCL_PANDA_PNMIMAGE int pm_bitread(BITSTREAM b, unsigned long nbits, unsigned long *val);

/*
 * pm_bitwrite() - write the low nbits of val to the given file.
 *
 * The last pm_bitwrite() must be followed by a call to pm_bitflush().
 *
 * Returns the number of bytes written, -1 on error.
 */

extern EXPCL_PANDA_PNMIMAGE int pm_bitwrite(BITSTREAM b, unsigned long nbits, unsigned long val);

#endif /* _BITIO_H_ */

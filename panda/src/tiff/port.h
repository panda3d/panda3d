/*
 * Warning, this file was automatically created by the TIFF configure script
 * VERSION:	 v3.5.6
 * RELEASE:    beta
 * DATE:	 Wed Jun 13 22:00:21 2001
 * TARGET:	
 * CCOMPILER:	 /bin/gcc-2.9.cygwin-990830
 */
#ifndef _PORT_
#define _PORT_ 1
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#define HOST_FILLORDER FILLORDER_MSB2LSB
#define HOST_BIGENDIAN	0
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
typedef double dblparam_t;
#ifdef __STRICT_ANSI__
#define	INLINE	__inline__
#else
#define	INLINE	inline
#endif
#define GLOBALDATA(TYPE,NAME)	extern TYPE NAME
#ifdef __cplusplus
}
#endif
#endif

/*
 *   xfermem.h
 *
 *   Oliver Fromme  <oliver.fromme@heim3.tu-clausthal.de>
 *   Sat Mar 29 04:41:34 MET 1997
 *
 *   This is a stand-alone module which implements a unidirectional,
 *   fast pipe using mmap().  Its primary use is to transfer large
 *   amounts of data from a parent process to its child process,
 *   with a buffer in between which decouples blocking conditions
 *   on both sides.  Control information is transferred between the
 *   processes through a socketpair.  See xftest.c for an example on
 *   how to use this module.
 */

#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

typedef struct {
	int freeindex;	/* [W] next free index */
	int readindex;	/* [R] next index to read */
	int fd[2];
	int wakeme[2];
	byte *data;
	byte *metadata;
	int size;
	int metasize;
	int buf[3];
} txfermem;
/*
 *   [W] -- May be written to by the writing process only!
 *   [R] -- May be written to by the reading process only!
 *   All other entries are initialized once.
 */

void xfermem_init (txfermem **xf, int bufsize, int msize,int skipbuf);
void xfermem_init_writer (txfermem *xf);
void xfermem_init_reader (txfermem *xf);

int  xfermem_write (txfermem *xf, byte *data, int count);
int  xfermem_read  (txfermem *xf, byte *data, int count);

int xfermem_get_freespace (txfermem *xf);
int xfermem_get_usedspace (txfermem *xf);
#define XF_CMD_WAKEUP_INFO  0x04
#define XF_CMD_WAKEUP    0x02
#define XF_CMD_TERMINATE 0x03
#define XF_WRITER 0
#define XF_READER 1
int xfermem_getcmd (int fd, int block);
int xfermem_putcmd (int fd, byte cmd);
int xfermem_block (int fd, txfermem *xf);

void xfermem_done (txfermem *xf);
#define xfermem_done_writer xfermem_init_reader
#define xfermem_done_reader xfermem_init_writer

/* EOF */

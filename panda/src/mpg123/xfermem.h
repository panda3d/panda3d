/* Filename: xfermem.h
 * Created by:  
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://www.panda3d.org/license.txt .
 *
 * To contact the maintainers of this program write to
 * panda3d@yahoogroups.com .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

typedef struct {
        int freeindex;  /* [W] next free index */
        int readindex;  /* [R] next index to read */
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

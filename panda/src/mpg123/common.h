/* Filename: common.h
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

extern void print_id3_tag(unsigned char *buf);
extern unsigned long firsthead;
extern int tabsel_123[2][3][16];
extern double compute_tpf(struct frame *fr);
extern double compute_bpf(struct frame *fr);
extern long compute_buffer_offset(struct frame *fr);

/*
struct bitstream_info {
  int bitindex;
  unsigned char *wordpointer;
};
*/



// Filename: indexParameters.h
// Created by:  drose (04Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef INDEXPARAMETERS_H
#define INDEXPARAMETERS_H

#include "pandatoolbase.h"

#include "filename.h"

// Some of these constants may be modified by command-line parameters
// from the user.

extern int max_index_width;
extern int max_index_height;

extern int thumb_width;
extern int thumb_height;
extern int thumb_caption_height;
extern int thumb_x_space;
extern int thumb_y_space;

extern int caption_font_size;

extern int reduced_width;
extern int reduced_height;

extern Filename prev_icon;
extern Filename next_icon;
extern Filename up_icon;

void finalize_parameters();

// The following parameters are all computed based on the above.

extern int thumb_count_x;
extern int thumb_count_y;
extern int max_thumbs;
extern int actual_index_width;

#endif



// Filename: bam.h
// Created by:  jason (27Jun00)
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

//This file just holds the Magic Number, Major and Minor version
//numbers that are common to both BamWriter and BamReader.

#ifndef _BAM_H
#define _BAM_H

#include <pandabase.h>

//The magic number for a BAM file and a carriage return and newline
//for detecting files damaged due to ASCII/Binary conversion
static const string _bam_header = string("pbj\0\n\r", 6);

static const unsigned short _bam_major_ver = 3;
// Bumped to major version 2 on 7/6/00 due to major changes in Character.
// Bumped to major version 3 on 12/8/00 to change float64's to float32's.

static const unsigned short _bam_minor_ver = 3;
// Bumped to minor version 1 on 12/15/00 to add FFT-style channel
// compression.
// Bumped to minor version 2 on 2/15/01 to add ModelNode::_preserve_transform.
// Bumped to minor version 3 on 4/11/01 to support correctly ordered children.


#endif

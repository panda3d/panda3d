// Filename: bam.h
// Created by:  jason (27Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

// This file just holds the Magic Number, Major and Minor version
// numbers that are common to both BamWriter and BamReader.

#ifndef _BAM_H
#define _BAM_H

#include "pandabase.h"

// The magic number for a BAM file.  It includes a carriage return and
// newline character to help detect files damaged due to faulty
// ASCII/Binary conversion.
static const string _bam_header = string("pbj\0\n\r", 6);

static const unsigned short _bam_major_ver = 6;
// Bumped to major version 2 on 7/6/00 due to major changes in Character.
// Bumped to major version 3 on 12/8/00 to change float64's to float32's.
// Bumped to major version 4 on 4/10/02 to store new scene graph.
// Bumped to major version 5 on 5/6/05 for new Geom implementation.
// Bumped to major version 6 on 2/11/06 to factor out PandaNode::CData.

static const unsigned short _bam_first_minor_ver = 14;
static const unsigned short _bam_minor_ver = 19;
// Bumped to minor version 14 on 12/19/07 to change default ColorAttrib.
// Bumped to minor version 15 on 4/9/08 to add TextureAttrib::_implicit_sort.
// Bumped to minor version 16 on 5/13/08 to add Texture::_quality_level.
// Bumped to minor version 17 on 8/6/08 to add PartBundle::_anim_preload.
// Bumped to minor version 18 on 8/14/08 to add Texture::_simple_ram_image.
// Bumped to minor version 19 on 8/14/08 to add PandaNode::_bounds_type.


#endif

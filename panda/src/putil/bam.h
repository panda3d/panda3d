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

// This file just holds the Magic Number, Major and Minor version
// numbers that are common to both BamWriter and BamReader.

#ifndef _BAM_H
#define _BAM_H

#include "pandabase.h"

// The magic number for a BAM file.  It includes a carriage return and
// newline character to help detect files damaged due to faulty
// ASCII/Binary conversion.
static const string _bam_header = string("pbj\0\n\r", 6);

static const unsigned short _bam_major_ver = 4;
// Bumped to major version 2 on 7/6/00 due to major changes in Character.
// Bumped to major version 3 on 12/8/00 to change float64's to float32's.
// Bumped to major version 4 on 4/10/02 to store new scene graph.

static const unsigned short _bam_minor_ver = 7;
// Bumped to minor version 1 on 4/10/03 to add CullFaceAttrib::reverse.
// Bumped to minor version 2 on 4/12/03 to add num_components to texture.
// Bumped to minor version 3 on 4/15/03 to add ImageBuffer::_alpha_file_channel
// Bumped to minor version 4 on 6/12/03 to add PandaNode::set_tag().
// Bumped to minor version 5 on 7/09/03 to add rawdata mode to texture.
// Bumped to minor version 6 on 7/22/03 to add shear to scene graph and animation data.
// Bumped to minor version 7 on 11/10/03 to add CollisionSolid::_effective_normal


#endif

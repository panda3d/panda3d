// Filename: bam.h
// Created by:  jason (27Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

static const unsigned short _bam_minor_ver = 7;
// Bumped to minor version 1 on 3/12/06 to add Texture::_compression.
// Bumped to minor version 2 on 3/17/06 to add PandaNode::_draw_control_mask.
// Bumped to minor version 3 on 3/21/06 to add Texture::_ram_images.
// Bumped to minor version 4 on 7/26/06 to add CharacterJoint::_character.
// Bumped to minor version 5 on 11/15/06 to add PartBundleNode::_num_bundles.
// Bumped to minor version 6 on 2/5/07 to change GeomPrimitive::_num_vertices.
// Bumped to minor version 7 on 2/15/07 to change SliderTable.


#endif

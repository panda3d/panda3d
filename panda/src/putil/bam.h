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
static const unsigned short _bam_minor_ver = 38;
// Bumped to minor version 14 on 12/19/07 to change default ColorAttrib.
// Bumped to minor version 15 on 4/9/08 to add TextureAttrib::_implicit_sort.
// Bumped to minor version 16 on 5/13/08 to add Texture::_quality_level.
// Bumped to minor version 17 on 8/6/08 to add PartBundle::_anim_preload.
// Bumped to minor version 18 on 8/14/08 to add Texture::_simple_ram_image.
// Bumped to minor version 19 on 8/14/08 to add PandaNode::_bounds_type.
// Bumped to minor version 20 on 4/21/09 to add MovingPartBase::_forced_channel.
// Bumped to minor version 21 on 2/26/08 to add BamEnums::BamObjectCode.
// Bumped to minor version 22 on 7/31/09 to add UvScrollNode R speed.
// Bumped to minor version 23 on 5/4/10 to add internal TextureAttrib overrides.
// Bumped to minor version 24 on 5/4/10 to add internal TexMatrixAttrib overrides.
// Bumped to minor version 25 on 6/22/11 to add support for caching movie files.
// Bumped to minor version 26 on 8/5/11 to add multiview (stereo) Textures.
// Bumped to minor version 27 on 10/9/11 to add stdfloat_double.
// Bumped to minor version 28 on 11/28/11 to add Texture::_auto_texture_scale.
// Bumped to minor version 29 on 12/17/11 to add GeomVertexColumn::_column_alignment.
// Bumped to minor version 30 on 1/22/12 to add Texture::_pad_*_size.
// Bumped to minor version 31 on 2/16/12 to add DepthOffsetAttrib::_min_value, _max_value.
// Bumped to minor version 32 on 6/11/12 to add Texture::_has_read_mipmaps.
// Bumped to minor version 33 on 8/17/13 to add UvScrollNode::_w_speed.
// Bumped to minor version 34 on 9/16/14 to add ScissorAttrib::_off.
// Bumped to minor version 35 on 12/3/14 to change StencilAttrib.
// Bumped to minor version 36 on 12/9/14 to add samplers and lod settings.
// Bumped to minor version 37 on 1/22/15 to add GeomVertexArrayFormat::_divisor.
// Bumped to minor version 38 on 4/15/15 to add various Bullet classes.

#endif

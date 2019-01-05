/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bam.h
 * @author jason
 * @date 2000-06-27
 */

// This file just holds the Magic Number, Major and Minor version numbers that
// are common to both BamWriter and BamReader.

#ifndef _BAM_H
#define _BAM_H

#include "pandabase.h"

// The magic number for a BAM file.  It includes a carriage return and newline
// character to help detect files damaged due to faulty ASCIIBinary
// conversion.
static const std::string _bam_header = std::string("pbj\0\n\r", 6);

static const unsigned short _bam_major_ver = 6;
// Bumped to major version 2 on 2000-07-06 due to major changes in Character.
// Bumped to major version 3 on 2000-12-08 to change float64's to float32's.
// Bumped to major version 4 on 2002-04-10 to store new scene graph.
// Bumped to major version 5 on 2005-05-06 for new Geom implementation.
// Bumped to major version 6 on 2006-02-11 to factor out PandaNode::CData.

static const unsigned short _bam_first_minor_ver = 14;
static const unsigned short _bam_minor_ver = 44;
// Bumped to minor version 14 on 2007-12-19 to change default ColorAttrib.
// Bumped to minor version 15 on 2008-04-09 to add TextureAttrib::_implicit_sort.
// Bumped to minor version 16 on 2008-05-13 to add Texture::_quality_level.
// Bumped to minor version 17 on 2008-08-06 to add PartBundle::_anim_preload.
// Bumped to minor version 18 on 2008-08-14 to add Texture::_simple_ram_image.
// Bumped to minor version 19 on 2008-08-14 to add PandaNode::_bounds_type.
// Bumped to minor version 20 on 2009-04-21 to add MovingPartBase::_forced_channel.
// Bumped to minor version 21 on 2008-02-26 to add BamEnums::BamObjectCode.
// Bumped to minor version 22 on 2009-07-31 to add UvScrollNode R speed.
// Bumped to minor version 23 on 2010-05-04 to add internal TextureAttrib overrides.
// Bumped to minor version 24 on 2010-05-04 to add internal TexMatrixAttrib overrides.
// Bumped to minor version 25 on 2011-06-22 to add support for caching movie files.
// Bumped to minor version 26 on 2011-08-05 to add multiview (stereo) Textures.
// Bumped to minor version 27 on 2011-10-09 to add stdfloat_double.
// Bumped to minor version 28 on 2011-11-28 to add Texture::_auto_texture_scale.
// Bumped to minor version 29 on 2011-12-17 to add GeomVertexColumn::_column_alignment.
// Bumped to minor version 30 on 2012-01-22 to add Texture::_pad_*_size.
// Bumped to minor version 31 on 2012-02-16 to add DepthOffsetAttrib::_min_value, _max_value.
// Bumped to minor version 32 on 2012-06-11 to add Texture::_has_read_mipmaps.
// Bumped to minor version 33 on 2013-08-17 to add UvScrollNode::_w_speed.
// Bumped to minor version 34 on 2014-09-16 to add ScissorAttrib::_off.
// Bumped to minor version 35 on 2014-12-03 to change StencilAttrib.
// Bumped to minor version 36 on 2014-12-09 to add samplers and lod settings.
// Bumped to minor version 37 on 2015-01-22 to add GeomVertexArrayFormat::_divisor.
// Bumped to minor version 38 on 2015-04-15 to add various Bullet classes.
// Bumped to minor version 39 on 2016-01-09 to change lights and materials.
// Bumped to minor version 40 on 2016-01-11 to make NodePaths writable.
// Bumped to minor version 41 on 2016-03-02 to change LensNode, Lens, and Camera.
// Bumped to minor version 42 on 2016-04-08 to expand ColorBlendAttrib.
// Bumped to minor version 43 on 2018-12-06 to expand BillboardEffect and CompassEffect.
// Bumped to minor version 44 on 2018-12-23 to rename CollisionTube to CollisionCapsule.

#endif

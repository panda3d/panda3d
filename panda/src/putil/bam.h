// Filename: bam.h
// Created by:  jason (27Jun00)
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

static const unsigned short _bam_major_ver = 2;
// Bumped to major version 2 on 7/6/00 due to major changes in Character.
static const unsigned short _bam_minor_ver = 4;
// Bumped to minor version 1 on 7/19/00 to quantize channel files.
// Bumped to minor version 2 on 8/21/00 for CollisionNode::_collide_geom.
// Bumped to minor version 3 on 11/21/00 for dual-image textures.
// Bumped to minor version 4 on 11/28/00 for anisotropic texture filtering.


#endif

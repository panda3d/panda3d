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

static const unsigned short _bam_major_ver = 3;
// Bumped to major version 2 on 7/6/00 due to major changes in Character.
// Bumped to major version 3 on 12/8/00 to change float64's to float32's.

static const unsigned short _bam_minor_ver = 0;


#endif

// Filename: mmsystem.h
// Created by:  darren (22Feb02)
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

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef MMSYSTEM_H
#define MMSYSTEM_H

typedef void* HWAVEIN;
typedef unsigned int MMRESULT;

/* wave data block header */
struct WAVEHDR {
    char*       lpData;                 /* pointer to locked data buffer */
    long        dwBufferLength;         /* length of data buffer */
    long        dwBytesRecorded;        /* used for input only */
    long*       dwUser;                 /* for client's use */
    long        dwFlags;                /* assorted flags (see defines) */
    long        dwLoops;                /* loop control counter */
    struct WAVEHDR *lpNext;     /* reserved for driver */
    long*       reserved;               /* reserved for driver */
};

struct WAVEFORMATEX
{
    short        wFormatTag;         /* format type */
    short        nChannels;          /* number of channels (i.e. mono, stereo...) */
    long       nSamplesPerSec;     /* sample rate */
    long       nAvgBytesPerSec;    /* for buffer estimation */
    short        nBlockAlign;        /* block size of data */
    short        wBitsPerSample;     /* number of bits per sample of mono data */
    short        cbSize;             /* the count in bytes of the size of */
                                    /* extra information (after cbSize) */
};

#endif

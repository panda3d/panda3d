// Filename: ioPtaDatagramLinMath.h
// Created by:  jason (26Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef _IO_PTA_DATAGRAM_LINMATH
#define _IO_PTA_DATAGRAM_LINMATH

#include <pandabase.h>

#include "luse.h"
#include "pta_Vertexf.h"
#include "pta_Colorf.h"
#include "pta_Normalf.h"
#include "pta_TexCoordf.h"

#include <pointerToArray.h>

class Datagram;
class DatagramIterator;

///////////////////////////////////////////////////////////////////
// 	 Class : IoPtaDatagramLinMath
// Description : This class is used to read and write a PTA_something
//               (where something is some kind of LinMath object like
//               LMatrix4f or LVector3f) from a Datagram, in support
//               of Bam.  It's not intended to be constructed; it's
//               just a convenient place to scope these static methods
//               which should be called directly.
////////////////////////////////////////////////////////////////////
template<class LinMathElement>
class IoPtaDatagramLinMath {
public:
  static void write_datagram(Datagram &dest, CPTA(LinMathElement) array);
  static PTA(LinMathElement) read_datagram(DatagramIterator &source);
};

#include "ioPtaDatagramLinMath.I"

// Now export all of the likely template classes for Windows' benefit.
// This must be done in this file, and not in the individual pta_*
// files, because it's important that this export command be the first
// appearance of a particular template instantiation.

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, IoPtaDatagramLinMath<Colorf>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, IoPtaDatagramLinMath<Normalf>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, IoPtaDatagramLinMath<TexCoordf>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, IoPtaDatagramLinMath<Vertexf>)


typedef IoPtaDatagramLinMath<Colorf> IPD_Colorf;
typedef IoPtaDatagramLinMath<Normalf> IPD_Normalf;
typedef IoPtaDatagramLinMath<TexCoordf> IPD_TexCoordf;
typedef IoPtaDatagramLinMath<Vertexf> IPD_Vertexf;


// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif

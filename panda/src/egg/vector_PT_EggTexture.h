// Filename: vector_PT_EggTexture.h
// Created by:  drose (01May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_PT_EGGTEXTURE_H
#define VECTOR_PT_EGGTEXTURE_H

#include <pandabase.h>

#include "eggTexture.h"
#include "pt_EggTexture.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_PT_EggTexture
// Description : A vector of PT(EggTexture)'s.  This class is defined once
//               here, and exported to PANDAEGG.DLL; other packages
//               that want to use a vector of this type (whether they
//               need to export it or not) should include this header
//               file, rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<PT_EggTexture>)
typedef vector<PT_EggTexture> vector_PT_EggTexture;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif

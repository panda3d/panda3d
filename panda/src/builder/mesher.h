// Filename: mesher.h
// Created by:  drose (17Sep97)
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
#ifndef MESHER_H
#define MESHER_H

#include "pandabase.h"

#include "mesherConfig.h"
#include "mesherFanMaker.h"
#include "mesherEdge.h"
#include "mesherStrip.h"
#include "mesherTempl.h"
#include "builderPrim.h"


#ifdef SUPPORT_FANS
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherFanMaker<BuilderPrim>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherFanMaker<BuilderPrimI>);
#endif

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherEdge<BuilderPrim>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherStrip<BuilderPrim>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherTempl<BuilderPrim>);

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherEdge<BuilderPrimI>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherStrip<BuilderPrimI>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherTempl<BuilderPrimI>);

class EXPCL_PANDAEGG Mesher : public MesherTempl<BuilderPrim> {
public:
  Mesher(BuilderBucket *bucket) : MesherTempl<BuilderPrim>(bucket) {}
};

class EXPCL_PANDAEGG MesherI : public MesherTempl<BuilderPrimI> {
public:
  MesherI(BuilderBucket *bucket) : MesherTempl<BuilderPrimI>(bucket) {}
};

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif


// Filename: mesher.h
// Created by:  drose (17Sep97)
//
////////////////////////////////////////////////////////////////////
#ifndef MESHER_H
#define MESHER_H

#include <pandabase.h>

#include "mesherFanMaker.h"
#include "mesherEdge.h"
#include "mesherStrip.h"
#include "mesherTempl.h"
#include "builderPrim.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherFanMaker<BuilderPrim>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherEdge<BuilderPrim>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherStrip<BuilderPrim>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherTempl<BuilderPrim>);

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, MesherFanMaker<BuilderPrimI>);
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


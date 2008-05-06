// Filename: tinyGraphicsStateGuardian.cxx
// Created by:  drose (24Apr08)
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

#include "tinyGraphicsStateGuardian.h"
#include "tinyGeomMunger.h"
#include "tinyTextureContext.h"
#include "config_tinydisplay.h"
#include "pStatTimer.h"
#include "geomVertexReader.h"
#include "ambientLight.h"
#include "pointLight.h"
#include "directionalLight.h"
#include "spotlight.h"
#include "bitMask.h"
#include "zgl.h"
#include "zmath.h"

TypeHandle TinyGraphicsStateGuardian::_type_handle;

PStatCollector TinyGraphicsStateGuardian::_vertices_immediate_pcollector("Vertices:Immediate mode");
PStatCollector TinyGraphicsStateGuardian::_draw_transform_pcollector("Draw:Transform");


static const ZB_fillTriangleFunc fill_tri_funcs
[2 /* depth write: zon, zoff */]
[3 /* color write: noblend, blend, nocolor */]
[3 /* alpha test: anone, aless, amore */]
[2 /* depth test: znone, zless */]
[3 /* shading: white, flat, smooth */]
[3 /* texturing: untextured, textured, perspective textured */] = {
  { // depth write zon
    { // color write noblend
      { // alpha test anone
        {
          { ZB_fillTriangleFlat_xx_zon_noblend_anone_znone,
            ZB_fillTriangleMapping_xx_zon_noblend_anone_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_noblend_anone_znone },
          { ZB_fillTriangleFlat_xx_zon_noblend_anone_znone,
            ZB_fillTriangleMappingFlat_xx_zon_noblend_anone_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_noblend_anone_znone },
          { ZB_fillTriangleSmooth_xx_zon_noblend_anone_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_noblend_anone_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_noblend_anone_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_noblend_anone_zless,
            ZB_fillTriangleMapping_xx_zon_noblend_anone_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_noblend_anone_zless },
          { ZB_fillTriangleFlat_xx_zon_noblend_anone_zless,
            ZB_fillTriangleMappingFlat_xx_zon_noblend_anone_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_noblend_anone_zless },
          { ZB_fillTriangleSmooth_xx_zon_noblend_anone_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_noblend_anone_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_noblend_anone_zless },
        },
      },
      { // alpha test aless
        {
          { ZB_fillTriangleFlat_xx_zon_noblend_aless_znone,
            ZB_fillTriangleMapping_xx_zon_noblend_aless_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_noblend_aless_znone },
          { ZB_fillTriangleFlat_xx_zon_noblend_aless_znone,
            ZB_fillTriangleMappingFlat_xx_zon_noblend_aless_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_noblend_aless_znone },
          { ZB_fillTriangleSmooth_xx_zon_noblend_aless_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_noblend_aless_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_noblend_aless_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_noblend_aless_zless,
            ZB_fillTriangleMapping_xx_zon_noblend_aless_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_noblend_aless_zless },
          { ZB_fillTriangleFlat_xx_zon_noblend_aless_zless,
            ZB_fillTriangleMappingFlat_xx_zon_noblend_aless_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_noblend_aless_zless },
          { ZB_fillTriangleSmooth_xx_zon_noblend_aless_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_noblend_aless_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_noblend_aless_zless },
        },
      },
      { // alpha test amore
        {
          { ZB_fillTriangleFlat_xx_zon_noblend_amore_znone,
            ZB_fillTriangleMapping_xx_zon_noblend_amore_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_noblend_amore_znone },
          { ZB_fillTriangleFlat_xx_zon_noblend_amore_znone,
            ZB_fillTriangleMappingFlat_xx_zon_noblend_amore_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_noblend_amore_znone },
          { ZB_fillTriangleSmooth_xx_zon_noblend_amore_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_noblend_amore_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_noblend_amore_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_noblend_amore_zless,
            ZB_fillTriangleMapping_xx_zon_noblend_amore_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_noblend_amore_zless },
          { ZB_fillTriangleFlat_xx_zon_noblend_amore_zless,
            ZB_fillTriangleMappingFlat_xx_zon_noblend_amore_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_noblend_amore_zless },
          { ZB_fillTriangleSmooth_xx_zon_noblend_amore_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_noblend_amore_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_noblend_amore_zless },
        },
      },
    },
    { // color write blend
      { // alpha test anone
        {
          { ZB_fillTriangleFlat_xx_zon_blend_anone_znone,
            ZB_fillTriangleMapping_xx_zon_blend_anone_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_blend_anone_znone },
          { ZB_fillTriangleFlat_xx_zon_blend_anone_znone,
            ZB_fillTriangleMappingFlat_xx_zon_blend_anone_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_blend_anone_znone },
          { ZB_fillTriangleSmooth_xx_zon_blend_anone_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_blend_anone_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_blend_anone_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_blend_anone_zless,
            ZB_fillTriangleMapping_xx_zon_blend_anone_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_blend_anone_zless },
          { ZB_fillTriangleFlat_xx_zon_blend_anone_zless,
            ZB_fillTriangleMappingFlat_xx_zon_blend_anone_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_blend_anone_zless },
          { ZB_fillTriangleSmooth_xx_zon_blend_anone_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_blend_anone_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_blend_anone_zless },
        },
      },
      { // alpha test aless
        {
          { ZB_fillTriangleFlat_xx_zon_blend_aless_znone,
            ZB_fillTriangleMapping_xx_zon_blend_aless_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_blend_aless_znone },
          { ZB_fillTriangleFlat_xx_zon_blend_aless_znone,
            ZB_fillTriangleMappingFlat_xx_zon_blend_aless_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_blend_aless_znone },
          { ZB_fillTriangleSmooth_xx_zon_blend_aless_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_blend_aless_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_blend_aless_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_blend_aless_zless,
            ZB_fillTriangleMapping_xx_zon_blend_aless_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_blend_aless_zless },
          { ZB_fillTriangleFlat_xx_zon_blend_aless_zless,
            ZB_fillTriangleMappingFlat_xx_zon_blend_aless_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_blend_aless_zless },
          { ZB_fillTriangleSmooth_xx_zon_blend_aless_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_blend_aless_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_blend_aless_zless },
        },
      },
      { // alpha test amore
        {
          { ZB_fillTriangleFlat_xx_zon_blend_amore_znone,
            ZB_fillTriangleMapping_xx_zon_blend_amore_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_blend_amore_znone },
          { ZB_fillTriangleFlat_xx_zon_blend_amore_znone,
            ZB_fillTriangleMappingFlat_xx_zon_blend_amore_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_blend_amore_znone },
          { ZB_fillTriangleSmooth_xx_zon_blend_amore_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_blend_amore_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_blend_amore_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_blend_amore_zless,
            ZB_fillTriangleMapping_xx_zon_blend_amore_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_blend_amore_zless },
          { ZB_fillTriangleFlat_xx_zon_blend_amore_zless,
            ZB_fillTriangleMappingFlat_xx_zon_blend_amore_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_blend_amore_zless },
          { ZB_fillTriangleSmooth_xx_zon_blend_amore_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_blend_amore_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_blend_amore_zless },
        },
      },
    },
    { // color write nocolor
      { // alpha test anone
        {
          { ZB_fillTriangleFlat_xx_zon_nocolor_anone_znone,
            ZB_fillTriangleMapping_xx_zon_nocolor_anone_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_nocolor_anone_znone },
          { ZB_fillTriangleFlat_xx_zon_nocolor_anone_znone,
            ZB_fillTriangleMappingFlat_xx_zon_nocolor_anone_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_nocolor_anone_znone },
          { ZB_fillTriangleSmooth_xx_zon_nocolor_anone_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_nocolor_anone_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_nocolor_anone_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_nocolor_anone_zless,
            ZB_fillTriangleMapping_xx_zon_nocolor_anone_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_nocolor_anone_zless },
          { ZB_fillTriangleFlat_xx_zon_nocolor_anone_zless,
            ZB_fillTriangleMappingFlat_xx_zon_nocolor_anone_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_nocolor_anone_zless },
          { ZB_fillTriangleSmooth_xx_zon_nocolor_anone_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_nocolor_anone_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_nocolor_anone_zless },
        },
      },
      { // alpha test aless
        {
          { ZB_fillTriangleFlat_xx_zon_nocolor_aless_znone,
            ZB_fillTriangleMapping_xx_zon_nocolor_aless_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_nocolor_aless_znone },
          { ZB_fillTriangleFlat_xx_zon_nocolor_aless_znone,
            ZB_fillTriangleMappingFlat_xx_zon_nocolor_aless_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_nocolor_aless_znone },
          { ZB_fillTriangleSmooth_xx_zon_nocolor_aless_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_nocolor_aless_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_nocolor_aless_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_nocolor_aless_zless,
            ZB_fillTriangleMapping_xx_zon_nocolor_aless_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_nocolor_aless_zless },
          { ZB_fillTriangleFlat_xx_zon_nocolor_aless_zless,
            ZB_fillTriangleMappingFlat_xx_zon_nocolor_aless_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_nocolor_aless_zless },
          { ZB_fillTriangleSmooth_xx_zon_nocolor_aless_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_nocolor_aless_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_nocolor_aless_zless },
        },
      },
      { // alpha test amore
        {
          { ZB_fillTriangleFlat_xx_zon_nocolor_amore_znone,
            ZB_fillTriangleMapping_xx_zon_nocolor_amore_znone,
            ZB_fillTriangleMappingPerspective_xx_zon_nocolor_amore_znone },
          { ZB_fillTriangleFlat_xx_zon_nocolor_amore_znone,
            ZB_fillTriangleMappingFlat_xx_zon_nocolor_amore_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_nocolor_amore_znone },
          { ZB_fillTriangleSmooth_xx_zon_nocolor_amore_znone,
            ZB_fillTriangleMappingSmooth_xx_zon_nocolor_amore_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_nocolor_amore_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zon_nocolor_amore_zless,
            ZB_fillTriangleMapping_xx_zon_nocolor_amore_zless,
            ZB_fillTriangleMappingPerspective_xx_zon_nocolor_amore_zless },
          { ZB_fillTriangleFlat_xx_zon_nocolor_amore_zless,
            ZB_fillTriangleMappingFlat_xx_zon_nocolor_amore_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zon_nocolor_amore_zless },
          { ZB_fillTriangleSmooth_xx_zon_nocolor_amore_zless,
            ZB_fillTriangleMappingSmooth_xx_zon_nocolor_amore_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zon_nocolor_amore_zless },
        },
      },
    },
  },
  { // depth write zoff
    { // color write noblend
      { // alpha test anone
        {
          { ZB_fillTriangleFlat_xx_zoff_noblend_anone_znone,
            ZB_fillTriangleMapping_xx_zoff_noblend_anone_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_noblend_anone_znone },
          { ZB_fillTriangleFlat_xx_zoff_noblend_anone_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_noblend_anone_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_noblend_anone_znone },
          { ZB_fillTriangleSmooth_xx_zoff_noblend_anone_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_noblend_anone_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_noblend_anone_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_noblend_anone_zless,
            ZB_fillTriangleMapping_xx_zoff_noblend_anone_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_noblend_anone_zless },
          { ZB_fillTriangleFlat_xx_zoff_noblend_anone_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_noblend_anone_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_noblend_anone_zless },
          { ZB_fillTriangleSmooth_xx_zoff_noblend_anone_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_noblend_anone_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_noblend_anone_zless },
        },
      },
      { // alpha test aless
        {
          { ZB_fillTriangleFlat_xx_zoff_noblend_aless_znone,
            ZB_fillTriangleMapping_xx_zoff_noblend_aless_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_noblend_aless_znone },
          { ZB_fillTriangleFlat_xx_zoff_noblend_aless_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_noblend_aless_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_noblend_aless_znone },
          { ZB_fillTriangleSmooth_xx_zoff_noblend_aless_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_noblend_aless_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_noblend_aless_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_noblend_aless_zless,
            ZB_fillTriangleMapping_xx_zoff_noblend_aless_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_noblend_aless_zless },
          { ZB_fillTriangleFlat_xx_zoff_noblend_aless_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_noblend_aless_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_noblend_aless_zless },
          { ZB_fillTriangleSmooth_xx_zoff_noblend_aless_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_noblend_aless_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_noblend_aless_zless },
        },
      },
      { // alpha test amore
        {
          { ZB_fillTriangleFlat_xx_zoff_noblend_amore_znone,
            ZB_fillTriangleMapping_xx_zoff_noblend_amore_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_noblend_amore_znone },
          { ZB_fillTriangleFlat_xx_zoff_noblend_amore_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_noblend_amore_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_noblend_amore_znone },
          { ZB_fillTriangleSmooth_xx_zoff_noblend_amore_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_noblend_amore_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_noblend_amore_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_noblend_amore_zless,
            ZB_fillTriangleMapping_xx_zoff_noblend_amore_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_noblend_amore_zless },
          { ZB_fillTriangleFlat_xx_zoff_noblend_amore_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_noblend_amore_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_noblend_amore_zless },
          { ZB_fillTriangleSmooth_xx_zoff_noblend_amore_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_noblend_amore_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_noblend_amore_zless },
        },
      },
    },
    { // color write blend
      { // alpha test anone
        {
          { ZB_fillTriangleFlat_xx_zoff_blend_anone_znone,
            ZB_fillTriangleMapping_xx_zoff_blend_anone_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_blend_anone_znone },
          { ZB_fillTriangleFlat_xx_zoff_blend_anone_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_blend_anone_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_blend_anone_znone },
          { ZB_fillTriangleSmooth_xx_zoff_blend_anone_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_blend_anone_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_blend_anone_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_blend_anone_zless,
            ZB_fillTriangleMapping_xx_zoff_blend_anone_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_blend_anone_zless },
          { ZB_fillTriangleFlat_xx_zoff_blend_anone_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_blend_anone_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_blend_anone_zless },
          { ZB_fillTriangleSmooth_xx_zoff_blend_anone_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_blend_anone_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_blend_anone_zless },
        },
      },
      { // alpha test aless
        {
          { ZB_fillTriangleFlat_xx_zoff_blend_aless_znone,
            ZB_fillTriangleMapping_xx_zoff_blend_aless_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_blend_aless_znone },
          { ZB_fillTriangleFlat_xx_zoff_blend_aless_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_blend_aless_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_blend_aless_znone },
          { ZB_fillTriangleSmooth_xx_zoff_blend_aless_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_blend_aless_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_blend_aless_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_blend_aless_zless,
            ZB_fillTriangleMapping_xx_zoff_blend_aless_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_blend_aless_zless },
          { ZB_fillTriangleFlat_xx_zoff_blend_aless_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_blend_aless_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_blend_aless_zless },
          { ZB_fillTriangleSmooth_xx_zoff_blend_aless_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_blend_aless_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_blend_aless_zless },
        },
      },
      { // alpha test amore
        {
          { ZB_fillTriangleFlat_xx_zoff_blend_amore_znone,
            ZB_fillTriangleMapping_xx_zoff_blend_amore_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_blend_amore_znone },
          { ZB_fillTriangleFlat_xx_zoff_blend_amore_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_blend_amore_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_blend_amore_znone },
          { ZB_fillTriangleSmooth_xx_zoff_blend_amore_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_blend_amore_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_blend_amore_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_blend_amore_zless,
            ZB_fillTriangleMapping_xx_zoff_blend_amore_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_blend_amore_zless },
          { ZB_fillTriangleFlat_xx_zoff_blend_amore_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_blend_amore_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_blend_amore_zless },
          { ZB_fillTriangleSmooth_xx_zoff_blend_amore_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_blend_amore_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_blend_amore_zless },
        },
      },
    },
    { // color write nocolor
      { // alpha test anone
        {
          { ZB_fillTriangleFlat_xx_zoff_nocolor_anone_znone,
            ZB_fillTriangleMapping_xx_zoff_nocolor_anone_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_nocolor_anone_znone },
          { ZB_fillTriangleFlat_xx_zoff_nocolor_anone_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_nocolor_anone_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_nocolor_anone_znone },
          { ZB_fillTriangleSmooth_xx_zoff_nocolor_anone_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_nocolor_anone_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_nocolor_anone_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_nocolor_anone_zless,
            ZB_fillTriangleMapping_xx_zoff_nocolor_anone_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_nocolor_anone_zless },
          { ZB_fillTriangleFlat_xx_zoff_nocolor_anone_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_nocolor_anone_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_nocolor_anone_zless },
          { ZB_fillTriangleSmooth_xx_zoff_nocolor_anone_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_nocolor_anone_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_nocolor_anone_zless },
        },
      },
      { // alpha test aless
        {
          { ZB_fillTriangleFlat_xx_zoff_nocolor_aless_znone,
            ZB_fillTriangleMapping_xx_zoff_nocolor_aless_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_nocolor_aless_znone },
          { ZB_fillTriangleFlat_xx_zoff_nocolor_aless_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_nocolor_aless_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_nocolor_aless_znone },
          { ZB_fillTriangleSmooth_xx_zoff_nocolor_aless_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_nocolor_aless_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_nocolor_aless_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_nocolor_aless_zless,
            ZB_fillTriangleMapping_xx_zoff_nocolor_aless_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_nocolor_aless_zless },
          { ZB_fillTriangleFlat_xx_zoff_nocolor_aless_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_nocolor_aless_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_nocolor_aless_zless },
          { ZB_fillTriangleSmooth_xx_zoff_nocolor_aless_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_nocolor_aless_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_nocolor_aless_zless },
        },
      },
      { // alpha test amore
        {
          { ZB_fillTriangleFlat_xx_zoff_nocolor_amore_znone,
            ZB_fillTriangleMapping_xx_zoff_nocolor_amore_znone,
            ZB_fillTriangleMappingPerspective_xx_zoff_nocolor_amore_znone },
          { ZB_fillTriangleFlat_xx_zoff_nocolor_amore_znone,
            ZB_fillTriangleMappingFlat_xx_zoff_nocolor_amore_znone,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_nocolor_amore_znone },
          { ZB_fillTriangleSmooth_xx_zoff_nocolor_amore_znone,
            ZB_fillTriangleMappingSmooth_xx_zoff_nocolor_amore_znone,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_nocolor_amore_znone },
        },
        {
          { ZB_fillTriangleFlat_xx_zoff_nocolor_amore_zless,
            ZB_fillTriangleMapping_xx_zoff_nocolor_amore_zless,
            ZB_fillTriangleMappingPerspective_xx_zoff_nocolor_amore_zless },
          { ZB_fillTriangleFlat_xx_zoff_nocolor_amore_zless,
            ZB_fillTriangleMappingFlat_xx_zoff_nocolor_amore_zless,
            ZB_fillTriangleMappingPerspectiveFlat_xx_zoff_nocolor_amore_zless },
          { ZB_fillTriangleSmooth_xx_zoff_nocolor_amore_zless,
            ZB_fillTriangleMappingSmooth_xx_zoff_nocolor_amore_zless,
            ZB_fillTriangleMappingPerspectiveSmooth_xx_zoff_nocolor_amore_zless },
        },
      },
    },
  },
};

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyGraphicsStateGuardian::
TinyGraphicsStateGuardian(GraphicsPipe *pipe,
			 TinyGraphicsStateGuardian *share_with) :
  GraphicsStateGuardian(CS_yup_right, pipe),
  _textures_lru("textures_lru", td_texture_ram)
{
  _c = NULL;
  _vertices = NULL;
  _vertices_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyGraphicsStateGuardian::
~TinyGraphicsStateGuardian() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
reset() {
  free_pointers();
  GraphicsStateGuardian::reset();

  if (_c != (GLContext *)NULL) {
    glClose(_c);
    _c = NULL;
  }

  _c = (GLContext *)gl_zalloc(sizeof(GLContext));
  glInit(_c, _current_frame_buffer);

  _c->draw_triangle_front = gl_draw_triangle_fill;
  _c->draw_triangle_back = gl_draw_triangle_fill;

  _supported_geom_rendering =
    Geom::GR_point | 
    Geom::GR_indexed_other |
    Geom::GR_flat_last_vertex;

  _max_texture_dimension = (1 << ZB_POINT_ST_FRAC_BITS);
  _max_lights = MAX_LIGHTS;

  _color_scale_via_lighting = false;
  _alpha_scale_via_texture = false;
  _runtime_color_scale = true;

  // Now that the GSG has been initialized, make it available for
  // optimizations.
  add_gsg(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::free_pointers
//       Access: Protected, Virtual
//  Description: Frees some memory that was explicitly allocated
//               within the glgsg.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
free_pointers() {
  if (_vertices != (GLVertex *)NULL) {
    PANDA_FREE_ARRAY(_vertices);
    _vertices = NULL;
  }
  _vertices_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::close_gsg
//       Access: Protected, Virtual
//  Description: This is called by the associated GraphicsWindow when
//               close_window() is called.  It should null out the
//               _win pointer and possibly free any open resources
//               associated with the GSG.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
close_gsg() {
  GraphicsStateGuardian::close_gsg();

  if (_c != (GLContext *)NULL) {
    glClose(_c);
    _c = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::depth_offset_decals
//       Access: Public, Virtual
//  Description: Returns true if this GSG can implement decals using a
//               DepthOffsetAttrib, or false if that is unreliable
//               and the three-step rendering process should be used
//               instead.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
depth_offset_decals() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::make_geom_munger
//       Access: Public, Virtual
//  Description: Creates a new GeomMunger object to munge vertices
//               appropriate to this GSG for the indicated state.
////////////////////////////////////////////////////////////////////
PT(GeomMunger) TinyGraphicsStateGuardian::
make_geom_munger(const RenderState *state, Thread *current_thread) {
  PT(TinyGeomMunger) munger = new TinyGeomMunger(this, state);
  return GeomMunger::register_munger(munger, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::clear
//       Access: Public
//  Description: Clears the framebuffer within the current
//               DisplayRegion, according to the flags indicated by
//               the given DrawableRegion object.
//
//               This does not set the DisplayRegion first.  You
//               should call prepare_display_region() to specify the
//               region you wish the clear operation to apply to.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
clear(DrawableRegion *clearable) {
  PStatTimer timer(_clear_pcollector);

  if ((!clearable->get_clear_color_active())&&
      (!clearable->get_clear_depth_active())&&
      (!clearable->get_clear_stencil_active())) {
    return;
  }
  
  set_state_and_transform(RenderState::make_empty(), _internal_transform);

  bool clear_color = false;
  int r, g, b, a;
  if (clearable->get_clear_color_active()) {
    Colorf v = clearable->get_clear_color();
    r = (int)(v[0] * 0xffff);
    g = (int)(v[1] * 0xffff);
    b = (int)(v[2] * 0xffff);
    a = (int)(v[3] * 0xffff);
    clear_color = true;
  }
  
  bool clear_z = false;
  int z;
  if (clearable->get_clear_depth_active()) {
    // We ignore the specified depth clear value, since we don't
    // support alternate depth compare functions anyway.
    z = 0;
    clear_z = true;
  }

  ZB_clear_viewport(_c->zb, clear_z, z,
                    clear_color, r, g, b, a,
                    _c->viewport.xmin, _c->viewport.ymin,
                    _c->viewport.xsize, _c->viewport.ysize);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::prepare_display_region
//       Access: Public, Virtual
//  Description: Prepare a display region for rendering (set up
//               scissor region and viewport)
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
prepare_display_region(DisplayRegionPipelineReader *dr,
                       Lens::StereoChannel stereo_channel) {
  nassertv(dr != (DisplayRegionPipelineReader *)NULL);
  GraphicsStateGuardian::prepare_display_region(dr, stereo_channel);

  int xmin, ymin, xsize, ysize;
  dr->get_region_pixels_i(xmin, ymin, xsize, ysize);

  _c->viewport.xmin = xmin;
  _c->viewport.ymin = ymin;
  _c->viewport.xsize = xsize;
  _c->viewport.ysize = ysize;
  gl_eval_viewport(_c);

  GLViewport *v = &_c->viewport;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::calc_projection_mat
//       Access: Public, Virtual
//  Description: Given a lens, calculates the appropriate projection
//               matrix for use with this gsg.  Note that the
//               projection matrix depends a lot upon the coordinate
//               system of the rendering API.
//
//               The return value is a TransformState if the lens is
//               acceptable, NULL if it is not.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TinyGraphicsStateGuardian::
calc_projection_mat(const Lens *lens) {
  if (lens == (Lens *)NULL) {
    return NULL;
  }

  if (!lens->is_linear()) {
    return NULL;
  }

  // The projection matrix must always be right-handed Y-up, even if
  // our coordinate system of choice is otherwise, because certain GL
  // calls (specifically glTexGen(GL_SPHERE_MAP)) assume this kind of
  // a coordinate system.  Sigh.  In order to implement a Z-up (or
  // other arbitrary) coordinate system, we'll use a Y-up projection
  // matrix, and store the conversion to our coordinate system of
  // choice in the modelview matrix.

  LMatrix4f result =
    LMatrix4f::convert_mat(CS_yup_right, _current_lens->get_coordinate_system()) *
    lens->get_projection_mat(_current_stereo_channel);

  if (_scene_setup->get_inverted()) {
    // If the scene is supposed to be inverted, then invert the
    // projection matrix.
    result *= LMatrix4f::scale_mat(1.0f, -1.0f, 1.0f);
  }

  return TransformState::make_mat(result);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::prepare_lens
//       Access: Public, Virtual
//  Description: Makes the current lens (whichever lens was most
//               recently specified with set_scene()) active, so
//               that it will transform future rendered geometry.
//               Normally this is only called from the draw process,
//               and usually it is called by set_scene().
//
//               The return value is true if the lens is acceptable,
//               false if it is not.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
prepare_lens() {
  _transform_stale = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_frame
//       Access: Public, Virtual
//  Description: Called before each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup before
//               beginning the frame.
//
//               The return value is true if successful (in which case
//               the frame will be drawn and end_frame() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_frame() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
begin_frame(Thread *current_thread) {
  if (!GraphicsStateGuardian::begin_frame(current_thread)) {
    return false;
  }

  _c->zb = _current_frame_buffer;

#ifdef DO_PSTATS
  _vertices_immediate_pcollector.clear_level();
#endif

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::begin_scene
//       Access: Public, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the beginning of drawing commands for a "scene"
//               (usually a particular DisplayRegion) within a frame.
//               All 3-D drawing commands, except the clear operation,
//               must be enclosed within begin_scene() .. end_scene().
//
//               The return value is true if successful (in which case
//               the scene will be drawn and end_scene() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_scene() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
begin_scene() {
  return GraphicsStateGuardian::begin_scene();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::end_scene
//       Access: Protected, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the end of drawing commands for a "scene" (usually a
//               particular DisplayRegion) within a frame.  All 3-D
//               drawing commands, except the clear operation, must be
//               enclosed within begin_scene() .. end_scene().
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_scene() {
  GraphicsStateGuardian::end_scene();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_frame(Thread *current_thread) {
  GraphicsStateGuardian::end_frame(current_thread);

  // Flush any PCollectors specific to this kind of GSG.
  _vertices_immediate_pcollector.flush_level();

  // Evict any textures that exceed our texture memory.
  _textures_lru.begin_epoch();
}


////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::begin_draw_primitives
//       Access: Public, Virtual
//  Description: Called before a sequence of draw_primitive()
//               functions are called, this should prepare the vertex
//               data for rendering.  It returns true if the vertices
//               are ok, false to abort this group of primitives.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
begin_draw_primitives(const GeomPipelineReader *geom_reader,
                      const GeomMunger *munger,
                      const GeomVertexDataPipelineReader *data_reader,
                      bool force) {
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << "begin_draw_primitives: " << *(data_reader->get_object()) << "\n";
  }
#endif  // NDEBUG

  if (!GraphicsStateGuardian::begin_draw_primitives(geom_reader, munger, data_reader, force)) {
    return false;
  }
  nassertr(_data_reader != (GeomVertexDataPipelineReader *)NULL, false);

  PStatTimer timer(_draw_transform_pcollector);

  // Set up the proper transform.
  if (_data_reader->is_vertex_transformed()) {
    // If the vertex data claims to be already transformed into clip
    // coordinates, wipe out the current projection and modelview
    // matrix (so we don't attempt to transform it again).
    const TransformState *ident = TransformState::make_identity();
    load_matrix(&_c->matrix_model_view, ident);
    load_matrix(&_c->matrix_projection, ident);
    load_matrix(&_c->matrix_model_view_inv, ident);
    load_matrix(&_c->matrix_model_projection, ident);
    _c->matrix_model_projection_no_w_transform = 1;
    _transform_stale = true;

  } else if (_transform_stale) {
    // Load the actual transform.

    if (_c->lighting_enabled) {
      // With the lighting equation, we need to keep the modelview and
      // projection matrices separate.

      load_matrix(&_c->matrix_model_view, _internal_transform);
      load_matrix(&_c->matrix_projection, _projection_mat);

      /* precompute inverse modelview */
      M4 tmp;
      gl_M4_Inv(&tmp, &_c->matrix_model_view);
      gl_M4_Transpose(&_c->matrix_model_view_inv, &tmp);

    }

    // Compose the modelview and projection matrices.
    load_matrix(&_c->matrix_model_projection, 
                _projection_mat->compose(_internal_transform));

    /* test to accelerate computation */
    _c->matrix_model_projection_no_w_transform = 0;
    float *m = &_c->matrix_model_projection.m[0][0];
    if (m[12] == 0.0 && m[13] == 0.0 && m[14] == 0.0) {
      _c->matrix_model_projection_no_w_transform = 1;
    }
    _transform_stale = false;
  }

  // Figure out the subset of vertices we will be using in this
  // operation.
  int num_vertices = data_reader->get_num_rows();
  _min_vertex = num_vertices;
  _max_vertex = 0;
  int num_prims = geom_reader->get_num_primitives();
  int i;
  for (i = 0; i < num_prims; ++i) {
    CPT(GeomPrimitive) prim = geom_reader->get_primitive(i);
    int nv = prim->get_min_vertex();
    _min_vertex = min(_min_vertex, nv);
    int xv = prim->get_max_vertex();
    _max_vertex = max(_max_vertex, xv);
  }
  if (_min_vertex > _max_vertex) {
    return false;
  }

  // Now copy all of those vertices into our working table,
  // transforming into screen space them as we go.
  int num_used_vertices = _max_vertex - _min_vertex + 1;
  if (_vertices_size < num_used_vertices) {
    if (_vertices_size == 0) {
      _vertices_size = 1;
    }
    while (_vertices_size < num_used_vertices) {
      _vertices_size *= 2;
    }
    if (_vertices != (GLVertex *)NULL) {
      PANDA_FREE_ARRAY(_vertices);
    }
    _vertices = (GLVertex *)PANDA_MALLOC_ARRAY(_vertices_size * sizeof(GLVertex));
  }

  GeomVertexReader rtexcoord, rcolor, rnormal;

  // We only support single-texturing, so only bother with the first
  // texture stage.
  bool needs_texcoord = false;
  bool needs_texmat = false;
  LMatrix4f texmat;
  const InternalName *texcoord_name = InternalName::get_texcoord();
  int max_stage_index = _effective_texture->get_num_on_ff_stages();
  if (max_stage_index > 0) {
    TextureStage *stage = _effective_texture->get_on_ff_stage(0);
    rtexcoord = GeomVertexReader(data_reader, stage->get_texcoord_name());
    rtexcoord.set_row(_min_vertex);
    needs_texcoord = rtexcoord.has_column();

    if (needs_texcoord && _target._tex_matrix->has_stage(stage)) {
      needs_texmat = true;
      texmat = _target._tex_matrix->get_mat(stage);
    }
  }

  bool needs_color = false;
  if (_vertex_colors_enabled) {
    rcolor = GeomVertexReader(data_reader, InternalName::get_color());
    rcolor.set_row(_min_vertex);
    needs_color = rcolor.has_column();
  }

  if (!needs_color) {
    if (_has_scene_graph_color) {
      const Colorf &d = _scene_graph_color;
      _c->current_color.X = d[0];
      _c->current_color.Y = d[1];
      _c->current_color.Z = d[2];
      _c->current_color.W = d[3];
      
    } else {
      _c->current_color.X = 1.0f;
      _c->current_color.Y = 1.0f;
      _c->current_color.Z = 1.0f;
      _c->current_color.W = 1.0f;
    }
  }

  bool needs_normal = false;
  if (_c->lighting_enabled) {
    rnormal = GeomVertexReader(data_reader, InternalName::get_normal());
    rnormal.set_row(_min_vertex);
    needs_normal = rnormal.has_column();
  }

  GeomVertexReader rvertex(data_reader, InternalName::get_vertex()); 
  rvertex.set_row(_min_vertex);

  if (!needs_color && _color_material_flags) {
    if (_color_material_flags & CMF_ambient) {
      _c->materials[0].ambient = _c->current_color;
      _c->materials[1].ambient = _c->current_color;
    }
    if (_color_material_flags & CMF_diffuse) {
      _c->materials[0].diffuse = _c->current_color;
      _c->materials[1].diffuse = _c->current_color;
    }
  }

  for (i = 0; i < num_used_vertices; ++i) {
    GLVertex *v = &_vertices[i];
    const LVecBase4f &d = rvertex.get_data4f();
    
    v->coord.X = d[0];
    v->coord.Y = d[1];
    v->coord.Z = d[2];
    v->coord.W = d[3];

    if (needs_texmat) {
      // Transform texcoords as a four-component vector for most generality.
      LVecBase4f d = rtexcoord.get_data4f() * texmat;
      v->tex_coord.X = d[0];
      v->tex_coord.Y = d[1];

    } else if (needs_texcoord) {
      // No need to transform, so just extract as two-component.
      const LVecBase2f &d = rtexcoord.get_data2f();
      v->tex_coord.X = d[0];
      v->tex_coord.Y = d[1];
    }

    if (needs_color) {
      const Colorf &d = rcolor.get_data4f();
      _c->current_color.X = d[0];
      _c->current_color.Y = d[1];
      _c->current_color.Z = d[2];
      _c->current_color.W = d[3];
      
      if (_color_material_flags) {
        if (_color_material_flags & CMF_ambient) {
          _c->materials[0].ambient = _c->current_color;
          _c->materials[1].ambient = _c->current_color;
        }
        if (_color_material_flags & CMF_diffuse) {
          _c->materials[0].diffuse = _c->current_color;
          _c->materials[1].diffuse = _c->current_color;
        }
      }
    }

    v->color = _c->current_color;

    if (needs_normal) {
      const LVecBase3f &d = rnormal.get_data3f();
      _c->current_normal.X = d[0];
      _c->current_normal.Y = d[1];
      _c->current_normal.Z = d[2];
      _c->current_normal.W = 0.0f;
    }

    gl_vertex_transform(_c, v);

    if (_c->lighting_enabled) {
      gl_shade_vertex(_c, v);
    }

    if (v->clip_code == 0) {
      gl_transform_to_viewport(_c, v);
    }

    v->edge_flag = 1;
  }

  // Set up the appropriate function callback for filling triangles,
  // according to the current state.

  int depth_write_state = 0;
  if (_target._depth_write->get_mode() != DepthWriteAttrib::M_on) {
    depth_write_state = 1;
  }

  int color_write_state = 0;
  switch (_target._transparency->get_mode()) {
  case TransparencyAttrib::M_alpha:
  case TransparencyAttrib::M_dual:
    color_write_state = 1;
    break;

  default:
    break;
  }

  unsigned int color_channels =
    _target._color_write->get_channels() & _color_write_mask;
  if (color_channels == ColorWriteAttrib::C_off) {
    color_write_state = 2;
  }

  int alpha_test_state = 0;
  switch (_target._alpha_test->get_mode()) {
  case AlphaTestAttrib::M_none:
  case AlphaTestAttrib::M_never:
  case AlphaTestAttrib::M_always:
  case AlphaTestAttrib::M_equal:
  case AlphaTestAttrib::M_not_equal:
    alpha_test_state = 0;
    break;

  case AlphaTestAttrib::M_less:
  case AlphaTestAttrib::M_less_equal:
    alpha_test_state = 1;
    _c->zb->reference_alpha = (unsigned int)_target._alpha_test->get_reference_alpha() * 0xff00;
    break;

  case AlphaTestAttrib::M_greater:
  case AlphaTestAttrib::M_greater_equal:
    alpha_test_state = 2;
    _c->zb->reference_alpha = (unsigned int)_target._alpha_test->get_reference_alpha() * 0xff00;
    break;
  }

  int depth_test_state = 1;
  _c->depth_test = 1;  // set this for ZB_line
  if (_target._depth_test->get_mode() == DepthTestAttrib::M_none) {
    depth_test_state = 0;
    _c->depth_test = 0;
  }
  
  ShadeModelAttrib::Mode shade_model = _target._shade_model->get_mode();
  if (!needs_normal && !needs_color) {
    // With no per-vertex lighting, and no per-vertex colors, we might
    // as well use the flat shading model.
    shade_model = ShadeModelAttrib::M_flat;
  }
  int shading_state = 2;  // smooth
  _c->smooth_shade_model = true;

  if (shade_model == ShadeModelAttrib::M_flat) {
    _c->smooth_shade_model = false;
    shading_state = 1;  // flat
    if (_c->current_color.X == 1.0f &&
        _c->current_color.Y == 1.0f &&
        _c->current_color.Z == 1.0f &&
        _c->current_color.W == 1.0f) {
      shading_state = 0;  // white
    }
  }

  int texturing_state = 0;  // untextured
  if (_c->texture_2d_enabled) {
    texturing_state = 2;  // perspective-correct textures
    if (_c->matrix_model_projection_no_w_transform) {
      texturing_state = 1;  // non-perspective-correct textures
    }
  }

  _c->zb_fill_tri = fill_tri_funcs[depth_write_state][color_write_state][alpha_test_state][depth_test_state][shading_state][texturing_state];
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::draw_triangles
//       Access: Public, Virtual
//  Description: Draws a series of disconnected triangles.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
draw_triangles(const GeomPrimitivePipelineReader *reader, bool force) {
  PStatTimer timer(_draw_primitive_pcollector, reader->get_current_thread());

#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << "draw_triangles: " << *(reader->get_object()) << "\n";
  }
#endif  // NDEBUG

  int num_vertices = reader->get_num_vertices();
  _vertices_immediate_pcollector.add_level(num_vertices);

  if (reader->is_indexed()) {
    switch (reader->get_index_type()) {
    case Geom::NT_uint8:
      {
        PN_uint8 *index = (PN_uint8 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; i += 3) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          GLVertex *v2 = &_vertices[index[i + 2] - _min_vertex];
          gl_draw_triangle(_c, v0, v1, v2);
        }
      }
      break;

    case Geom::NT_uint16:
      {
        PN_uint16 *index = (PN_uint16 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; i += 3) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          GLVertex *v2 = &_vertices[index[i + 2] - _min_vertex];
          gl_draw_triangle(_c, v0, v1, v2);
        }
      }
      break;

    case Geom::NT_uint32:
      {
        PN_uint32 *index = (PN_uint32 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; i += 3) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          GLVertex *v2 = &_vertices[index[i + 2] - _min_vertex];
          gl_draw_triangle(_c, v0, v1, v2);
        }
      }
      break;

    default:
      break;
    }

  } else {
    int delta = reader->get_first_vertex() - _min_vertex;
    for (int vi = 0; vi < num_vertices; vi += 3) {
      GLVertex *v0 = &_vertices[vi + delta];
      GLVertex *v1 = &_vertices[vi + delta + 1];
      GLVertex *v2 = &_vertices[vi + delta + 2];
      gl_draw_triangle(_c, v0, v1, v2);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::draw_lines
//       Access: Public, Virtual
//  Description: Draws a series of disconnected line segments.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
draw_lines(const GeomPrimitivePipelineReader *reader, bool force) {
  PStatTimer timer(_draw_primitive_pcollector, reader->get_current_thread());
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << "draw_lines: " << *(reader->get_object()) << "\n";
  }
#endif  // NDEBUG

  int num_vertices = reader->get_num_vertices();
  _vertices_immediate_pcollector.add_level(num_vertices);

  if (reader->is_indexed()) {
    switch (reader->get_index_type()) {
    case Geom::NT_uint8:
      {
        PN_uint8 *index = (PN_uint8 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; i += 2) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          gl_draw_line(_c, v0, v1);
        }
      }
      break;

    case Geom::NT_uint16:
      {
        PN_uint16 *index = (PN_uint16 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; i += 2) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          gl_draw_line(_c, v0, v1);
        }
      }
      break;

    case Geom::NT_uint32:
      {
        PN_uint32 *index = (PN_uint32 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; i += 2) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          GLVertex *v1 = &_vertices[index[i + 1] - _min_vertex];
          gl_draw_line(_c, v0, v1);
        }
      }
      break;

    default:
      break;
    }

  } else {
    int delta = reader->get_first_vertex() - _min_vertex;
    for (int vi = 0; vi < num_vertices; vi += 2) {
      GLVertex *v0 = &_vertices[vi + delta];
      GLVertex *v1 = &_vertices[vi + delta + 1];
      gl_draw_line(_c, v0, v1);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::draw_points
//       Access: Public, Virtual
//  Description: Draws a series of disconnected points.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
draw_points(const GeomPrimitivePipelineReader *reader, bool force) {
  PStatTimer timer(_draw_primitive_pcollector, reader->get_current_thread());
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << "draw_points: " << *(reader->get_object()) << "\n";
  }
#endif  // NDEBUG

  int num_vertices = reader->get_num_vertices();
  _vertices_immediate_pcollector.add_level(num_vertices);

  if (reader->is_indexed()) {
    switch (reader->get_index_type()) {
    case Geom::NT_uint8:
      {
        PN_uint8 *index = (PN_uint8 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; ++i) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          gl_draw_point(_c, v0);
        }
      }
      break;

    case Geom::NT_uint16:
      {
        PN_uint16 *index = (PN_uint16 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; ++i) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          gl_draw_point(_c, v0);
        }
      }
      break;

    case Geom::NT_uint32:
      {
        PN_uint32 *index = (PN_uint32 *)reader->get_read_pointer(true);
        for (int i = 0; i < num_vertices; ++i) {
          GLVertex *v0 = &_vertices[index[i] - _min_vertex];
          gl_draw_point(_c, v0);
        }
      }
      break;

    default:
      break;
    }

  } else {
    int delta = reader->get_first_vertex() - _min_vertex;
    for (int vi = 0; vi < num_vertices; ++vi) {
      GLVertex *v0 = &_vertices[vi + delta];
      gl_draw_point(_c, v0);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::end_draw_primitives()
//       Access: Public, Virtual
//  Description: Called after a sequence of draw_primitive()
//               functions are called, this should do whatever cleanup
//               is appropriate.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
end_draw_primitives() {
  GraphicsStateGuardian::end_draw_primitives();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::framebuffer_copy_to_texture
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display
//               region from the framebuffer into texture memory.
//
//               If z > -1, it is the cube map index into which to
//               copy.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
framebuffer_copy_to_texture(Texture *tex, int z, const DisplayRegion *dr,
                            const RenderBuffer &rb) {
  nassertv(tex != NULL && dr != NULL);
  
  int xo, yo, w, h;
  dr->get_region_pixels_i(xo, yo, w, h);

  tex->setup_2d_texture(w, h, Texture::T_unsigned_byte, Texture::F_rgba);

  TextureContext *tc = tex->prepare_now(get_prepared_objects(), this);
  nassertv(tc != (TextureContext *)NULL);
  TinyTextureContext *gtc = DCAST(TinyTextureContext, tc);

  GLTexture *gltex = gtc->_gltex;
  setup_gltex(gltex, tex->get_x_size(), tex->get_y_size());

  PIXEL *ip = gltex->pixmap;
  PIXEL *fo = _c->zb->pbuf + xo + yo * _c->zb->linesize / PSZB;
  for (int y = 0; y < gltex->ysize; ++y) {
    memcpy(ip, fo, gltex->xsize * PSZB);
    ip += gltex->xsize;
    fo += _c->zb->linesize / PSZB;
  }

  gtc->update_data_size_bytes(gltex->xsize * gltex->ysize * 4);
  gtc->mark_loaded();
  gtc->enqueue_lru(&_textures_lru);
}


////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::framebuffer_copy_to_ram
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display region
//               from the framebuffer into system memory, not texture
//               memory.  Returns true on success, false on failure.
//
//               This completely redefines the ram image of the
//               indicated texture.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
framebuffer_copy_to_ram(Texture *tex, int z, const DisplayRegion *dr,
                        const RenderBuffer &rb) {
  nassertr(tex != NULL && dr != NULL, false);
  
  int xo, yo, w, h;
  dr->get_region_pixels_i(xo, yo, w, h);

  Texture::TextureType texture_type;
  int z_size;
  if (z >= 0) {
    texture_type = Texture::TT_cube_map;
    z_size = 6;
  } else {
    texture_type = Texture::TT_2d_texture;
    z_size = 1;
  }

  Texture::ComponentType component_type = Texture::T_unsigned_byte;
  Texture::Format format = Texture::F_rgba;

  if (tex->get_x_size() != w || tex->get_y_size() != h ||
      tex->get_z_size() != z_size ||
      tex->get_component_type() != component_type ||
      tex->get_format() != format ||
      tex->get_texture_type() != texture_type) {
    // Re-setup the texture; its properties have changed.
    tex->setup_texture(texture_type, w, h, z_size,
                       component_type, format);
  }

  unsigned char *image_ptr = tex->modify_ram_image();
  size_t image_size = tex->get_ram_image_size();
  if (z >= 0) {
    nassertr(z < tex->get_z_size(), false);
    image_size = tex->get_expected_ram_page_size();
    image_ptr += z * image_size;
  }

  PIXEL *ip = (PIXEL *)(image_ptr + image_size);
  PIXEL *fo = _c->zb->pbuf + xo + yo * _c->zb->linesize / PSZB;
  for (int y = 0; y < h; ++y) {
    ip -= w;
    memcpy(ip, fo, w * PSZB);
    fo += _c->zb->linesize / PSZB;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::set_state_and_transform
//       Access: Public, Virtual
//  Description: Simultaneously resets the render state and the
//               transform state.
//
//               This transform specified is the "internal" net
//               transform, already converted into the GSG's internal
//               coordinate space by composing it to
//               get_cs_transform().  (Previously, this used to be the
//               "external" net transform, with the assumption that
//               that GSG would convert it internally, but that is no
//               longer the case.)
//
//               Special case: if (state==NULL), then the target
//               state is already stored in _target.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
set_state_and_transform(const RenderState *target,
                        const TransformState *transform) {
#ifndef NDEBUG
  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam()
      << "Setting GSG state to " << (void *)target << ":\n";
    target->write(tinydisplay_cat.spam(false), 2);
  }
#endif

  _state_pcollector.add_level(1);
  PStatTimer timer1(_draw_set_state_pcollector);

  if (transform != _internal_transform) {
    PStatTimer timer(_draw_set_state_transform_pcollector);
    _state_pcollector.add_level(1);
    _internal_transform = transform;
    do_issue_transform();
  }

  if (target == _state_rs) {
    return;
  }
  _target_rs = target;
  _target.clear_to_defaults();
  target->store_into_slots(&_target);
  _state_rs = 0;

  if (_target._color != _state._color ||
      _target._color_scale != _state._color_scale) {
    PStatTimer timer(_draw_set_state_color_pcollector);
    do_issue_color();
    do_issue_color_scale();
    _state._color = _target._color;
    _state._color_scale = _target._color_scale;
  }

  if (_target._cull_face != _state._cull_face) {
    PStatTimer timer(_draw_set_state_cull_face_pcollector);
    do_issue_cull_face();
    _state._cull_face = _target._cull_face;
  }

  if (_target._render_mode != _state._render_mode) {
    PStatTimer timer(_draw_set_state_render_mode_pcollector);
    do_issue_render_mode();
    _state._render_mode = _target._render_mode;
  }

  if (_target._texture != _state._texture) {
    PStatTimer timer(_draw_set_state_texture_pcollector);
    determine_effective_texture();
    do_issue_texture();
    _state._texture = _target._texture;
  }
  
  if (_target._material != _state._material) {
    PStatTimer timer(_draw_set_state_material_pcollector);
    do_issue_material();
    _state._material = _target._material;
  }

  if (_target._light != _state._light) {
    PStatTimer timer(_draw_set_state_light_pcollector);
    do_issue_light();
    _state._light = _target._light;
  }

  _state_rs = _target_rs;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::prepare_texture
//       Access: Public, Virtual
//  Description: Creates whatever structures the GSG requires to
//               represent the texture internally, and returns a
//               newly-allocated TextureContext object with this data.
//               It is the responsibility of the calling function to
//               later call release_texture() with this same pointer
//               (which will also delete the pointer).
//
//               This function should not be called directly to
//               prepare a texture.  Instead, call Texture::prepare().
////////////////////////////////////////////////////////////////////
TextureContext *TinyGraphicsStateGuardian::
prepare_texture(Texture *tex) {
  if (tex->get_texture_type() != Texture::TT_2d_texture) {
    tinydisplay_cat.info()
      << "not loading texture " << tex->get_name() << ": "
      << tex->get_texture_type() << "\n";
    return NULL;
  }
  if (tex->get_ram_image_compression() != Texture::CM_off) {
    tinydisplay_cat.info()
      << "not loading texture " << tex->get_name() << ": "
      << tex->get_ram_image_compression() << "\n";
    return NULL;
  }

  TinyTextureContext *gtc = new TinyTextureContext(_prepared_objects, tex);
  gtc->_gltex = (GLTexture *)gl_zalloc(sizeof(GLTexture));

  return gtc;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.  This function should never be called
//               directly; instead, call Texture::release() (or simply
//               let the Texture destruct).
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
release_texture(TextureContext *tc) {
  TinyTextureContext *gtc = DCAST(TinyTextureContext, tc);

  GLTexture *gltex = gtc->_gltex;
  gtc->_gltex = NULL;

  if (_c->current_texture == gltex) {
    _c->current_texture = NULL;
    _c->texture_2d_enabled = false;
  }

  if (gltex->pixmap != NULL) {
    gl_free(gltex->pixmap);
  }

  gl_free(gltex);
  gtc->dequeue_lru();

  delete gtc;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_light
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_light() {
  // Initialize the current ambient light total and newly enabled
  // light list
  Colorf cur_ambient_light(0.0f, 0.0f, 0.0f, 0.0f);

  int num_enabled = 0;
  int num_on_lights = 0;

  if (display_cat.is_spam()) {
    display_cat.spam()
      << "do_issue_light: " << _target._light << "\n";
  }

  // First, release all of the previously-assigned lights.
  _c->lighting_enabled = false;

  GLLight *gl_light = _c->first_light;
  while (gl_light != (GLLight *)NULL) {
    GLLight *next = gl_light->next;
    gl_light->next = NULL;
    gl_light = next;
  }
  _c->first_light = NULL;

  // Now, assign new lights.
  if (_target._light != (LightAttrib *)NULL) {
    CPT(LightAttrib) new_light = _target._light->filter_to_max(_max_lights);
    if (display_cat.is_spam()) {
      new_light->write(display_cat.spam(false), 2);
    }

    num_on_lights = new_light->get_num_on_lights();
    for (int li = 0; li < num_on_lights; li++) {
      NodePath light = new_light->get_on_light(li);
      nassertv(!light.is_empty());
      Light *light_obj = light.node()->as_light();
      nassertv(light_obj != (Light *)NULL);

      _lighting_enabled = true;
      _c->lighting_enabled = true;

      if (light_obj->get_type() == AmbientLight::get_class_type()) {
        // Accumulate all of the ambient lights together into one.
        cur_ambient_light += light_obj->get_color();

      } else {
        // Other kinds of lights each get their own GLLight object.
        nassertv(num_enabled < MAX_LIGHTS);
        GLLight *gl_light = &_c->lights[num_enabled];
        memset(gl_light, 0, sizeof(GLLight));

        gl_light->next = _c->first_light;
        _c->first_light = gl_light;

        const Colorf &diffuse = light_obj->get_color();
        gl_light->diffuse.X = diffuse[0];
        gl_light->diffuse.Y = diffuse[1];
        gl_light->diffuse.Z = diffuse[2];
        gl_light->diffuse.W = diffuse[3];

        light_obj->bind(this, light, num_enabled);
        num_enabled++;
      }
    }
  }

  _c->ambient_light_model.X = cur_ambient_light[0];
  _c->ambient_light_model.Y = cur_ambient_light[1];
  _c->ambient_light_model.Z = cur_ambient_light[2];
  _c->ambient_light_model.W = cur_ambient_light[3];
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
bind_light(PointLight *light_obj, const NodePath &light, int light_id) {
  GLLight *gl_light = _c->first_light;
  nassertv(gl_light != (GLLight *)NULL);

  const Colorf &specular = light_obj->get_specular_color();
  gl_light->specular.X = specular[0];
  gl_light->specular.Y = specular[1];
  gl_light->specular.Z = specular[2];
  gl_light->specular.W = specular[3];

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  CPT(TransformState) render_transform =
    _cs_transform->compose(_scene_setup->get_world_transform());

  CPT(TransformState) transform = light.get_transform(_scene_setup->get_scene_root().get_parent());
  CPT(TransformState) net_transform = render_transform->compose(transform);

  LPoint3f pos = light_obj->get_point() * net_transform->get_mat();
  gl_light->position.X = pos[0];
  gl_light->position.Y = pos[1];
  gl_light->position.Z = pos[2];
  gl_light->position.W = 1.0f;

  // Exponent == 0 implies uniform light distribution
  gl_light->spot_exponent = 0.0f;

  // Cutoff == 180 means uniform point light source
  gl_light->spot_cutoff = 180.0f;

  const LVecBase3f &att = light_obj->get_attenuation();
  gl_light->attenuation[0] = att[0];
  gl_light->attenuation[1] = att[1];
  gl_light->attenuation[2] = att[2];
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
bind_light(DirectionalLight *light_obj, const NodePath &light, int light_id) {
  GLLight *gl_light = _c->first_light;
  nassertv(gl_light != (GLLight *)NULL);

  const Colorf &specular = light_obj->get_specular_color();
  gl_light->specular.X = specular[0];
  gl_light->specular.Y = specular[1];
  gl_light->specular.Z = specular[2];
  gl_light->specular.W = specular[3];

  // Position needs to specify x, y, z, and w
  // w == 0 implies light is at infinity
  CPT(TransformState) render_transform =
    _cs_transform->compose(_scene_setup->get_world_transform());

  CPT(TransformState) transform = light.get_transform(_scene_setup->get_scene_root().get_parent());
  CPT(TransformState) net_transform = render_transform->compose(transform);

  LVector3f dir = light_obj->get_direction() * net_transform->get_mat();
  gl_light->position.X = -dir[0];
  gl_light->position.Y = -dir[1];
  gl_light->position.Z = -dir[2];
  gl_light->position.W = 0.0f;

  gl_light->norm_position.X = -dir[0];
  gl_light->norm_position.Y = -dir[1];
  gl_light->norm_position.Z = -dir[2];
  gl_V3_Norm(&gl_light->norm_position);

  // Exponent == 0 implies uniform light distribution
  gl_light->spot_exponent = 0.0f;

  // Cutoff == 180 means uniform point light source
  gl_light->spot_cutoff = 180.0f;

  // Default attenuation values (only spotlight and point light can
  // modify these)
  gl_light->attenuation[0] = 1.0f;
  gl_light->attenuation[1] = 0.0f;
  gl_light->attenuation[2] = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
bind_light(Spotlight *light_obj, const NodePath &light, int light_id) {
  GLLight *gl_light = _c->first_light;
  nassertv(gl_light != (GLLight *)NULL);

  const Colorf &specular = light_obj->get_specular_color();
  gl_light->specular.X = specular[0];
  gl_light->specular.Y = specular[1];
  gl_light->specular.Z = specular[2];
  gl_light->specular.W = specular[3];
  
  Lens *lens = light_obj->get_lens();
  nassertv(lens != (Lens *)NULL);

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  CPT(TransformState) render_transform =
    _cs_transform->compose(_scene_setup->get_world_transform());

  CPT(TransformState) transform = light.get_transform(_scene_setup->get_scene_root().get_parent());
  CPT(TransformState) net_transform = render_transform->compose(transform);

  const LMatrix4f &light_mat = net_transform->get_mat();
  LPoint3f pos = lens->get_nodal_point() * light_mat;
  LVector3f dir = lens->get_view_vector() * light_mat;

  gl_light->position.X = pos[0];
  gl_light->position.Y = pos[1];
  gl_light->position.Z = pos[2];
  gl_light->position.W = 1.0f;

  gl_light->spot_direction.X = dir[0];
  gl_light->spot_direction.Y = dir[1];
  gl_light->spot_direction.Z = dir[2];

  gl_light->norm_spot_direction.X = dir[0];
  gl_light->norm_spot_direction.Y = dir[1];
  gl_light->norm_spot_direction.Z = dir[2];
  gl_V3_Norm(&gl_light->norm_spot_direction);

  gl_light->spot_exponent = light_obj->get_exponent();
  gl_light->spot_cutoff = lens->get_hfov() * 0.5f;

  const LVecBase3f &att = light_obj->get_attenuation();
  gl_light->attenuation[0] = att[0];
  gl_light->attenuation[1] = att[1];
  gl_light->attenuation[2] = att[2];
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_transform
//       Access: Protected
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
//
//               This transform is the internal_transform, already
//               converted into the GSG's internal coordinate system.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_transform() {
  _transform_state_pcollector.add_level(1);
  _transform_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_render_mode
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_render_mode() {
  const RenderModeAttrib *attrib = _target._render_mode;

  switch (attrib->get_mode()) {
  case RenderModeAttrib::M_unchanged:
  case RenderModeAttrib::M_filled:
    _c->draw_triangle_front = gl_draw_triangle_fill;
    _c->draw_triangle_back = gl_draw_triangle_fill;
    break;

  case RenderModeAttrib::M_wireframe:
    _c->draw_triangle_front = gl_draw_triangle_line;
    _c->draw_triangle_back = gl_draw_triangle_line;
    break;

  case RenderModeAttrib::M_point:
    _c->draw_triangle_front = gl_draw_triangle_point;
    _c->draw_triangle_back = gl_draw_triangle_point;
    break;

  default:
    tinydisplay_cat.error()
      << "Unknown render mode " << (int)attrib->get_mode() << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_cull_face
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_cull_face() {
  const CullFaceAttrib *attrib = _target._cull_face;
  CullFaceAttrib::Mode mode = attrib->get_effective_mode();

  switch (mode) {
  case CullFaceAttrib::M_cull_none:
    _c->cull_face_enabled = false;
    break;
  case CullFaceAttrib::M_cull_clockwise:
    _c->cull_face_enabled = true;
    _c->cull_clockwise = true;
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
    _c->cull_face_enabled = true;
    _c->cull_clockwise = false;
    break;
  default:
    tinydisplay_cat.error()
      << "invalid cull face mode " << (int)mode << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_material
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_material() {
  static Material empty;
  const Material *material;
  if (_target._material == (MaterialAttrib *)NULL ||
      _target._material->is_off()) {
    material = &empty;
  } else {
    material = _target._material->get_material();
  }

  // Apply the material parameters to the front face.
  setup_material(&_c->materials[0], material);

  if (material->get_twoside()) {
    // Also apply the material parameters to the back face.
    setup_material(&_c->materials[1], material);
  }

  _c->local_light_model = material->get_local();
  _c->light_model_two_side = material->get_twoside();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::do_issue_texture
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
do_issue_texture() {
  _c->texture_2d_enabled = false;

  int num_stages = _effective_texture->get_num_on_ff_stages();
  if (num_stages == 0) {
    // No texturing.
    return;
  }
  nassertv(num_stages == 1);

  TextureStage *stage = _effective_texture->get_on_ff_stage(0);
  Texture *texture = _effective_texture->get_on_texture(stage);
  nassertv(texture != (Texture *)NULL);
    
  TextureContext *tc = texture->prepare_now(_prepared_objects, this);
  if (tc == (TextureContext *)NULL) {
    // Something wrong with this texture; skip it.
    return;
  }
    
  // Then, turn on the current texture mode.
  apply_texture(tc);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::apply_texture
//       Access: Protected
//  Description: Updates TinyGL with the current information for this
//               texture, and makes it the current texture available
//               for rendering.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
apply_texture(TextureContext *tc) {
  TinyTextureContext *gtc = DCAST(TinyTextureContext, tc);

  gtc->set_active(true);
  _c->current_texture = gtc->_gltex;
  _c->texture_2d_enabled = true;

  GLTexture *gltex = gtc->_gltex;

  if (gtc->was_image_modified() || gltex->pixmap == NULL) {
    // If the texture image was modified, reload the texture.
    if (!upload_texture(gtc)) {
      _c->texture_2d_enabled = false;
    }
    gtc->mark_loaded();
  }
  gtc->enqueue_lru(&_textures_lru);

  _c->zb->current_texture.pixmap = gltex->pixmap;
  _c->zb->current_texture.s_mask = gltex->s_mask;
  _c->zb->current_texture.t_mask = gltex->t_mask;
  _c->zb->current_texture.t_shift = gltex->t_shift;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::upload_texture
//       Access: Protected
//  Description: Uploads the texture image to TinyGL.
//
//               The return value is true if successful, or false if
//               the texture has no image.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsStateGuardian::
upload_texture(TinyTextureContext *gtc) {
  Texture *tex = gtc->get_texture();
  PStatTimer timer(_load_texture_pcollector);

  CPTA_uchar src_image = tex->get_ram_image();
  if (src_image.is_null()) {
    return false;
  }

  if (tinydisplay_cat.is_debug()) {
    tinydisplay_cat.debug()
      << "loading texture " << tex->get_name() << "\n";
  }
#ifdef DO_PSTATS
  _data_transferred_pcollector.add_level(tex->get_ram_image_size());
#endif
  GLTexture *gltex = gtc->_gltex;
  setup_gltex(gltex, tex->get_x_size(), tex->get_y_size());

  switch (tex->get_format()) {
  case Texture::F_rgb:
  case Texture::F_rgb5:
  case Texture::F_rgb8:
  case Texture::F_rgb12:
  case Texture::F_rgb332:
    copy_rgb_image(gltex, tex);
    break;

  case Texture::F_rgba:
  case Texture::F_rgbm:
  case Texture::F_rgba4:
  case Texture::F_rgba5:
  case Texture::F_rgba8:
  case Texture::F_rgba12:
  case Texture::F_rgba16:
  case Texture::F_rgba32:
    copy_rgba_image(gltex, tex);
    break;

  case Texture::F_luminance:
    copy_lum_image(gltex, tex);
    break;

  case Texture::F_red:
    copy_one_channel_image(gltex, tex, 0);
    break;

  case Texture::F_green:
    copy_one_channel_image(gltex, tex, 1);
    break;

  case Texture::F_blue:
    copy_one_channel_image(gltex, tex, 2);
    break;

  case Texture::F_alpha:
    copy_alpha_image(gltex, tex);
    break;

  case Texture::F_luminance_alphamask:
  case Texture::F_luminance_alpha:
    copy_la_image(gltex, tex);
    break;
  }

  int bytecount = gltex->xsize * gltex->ysize * 4;
  gtc->update_data_size_bytes(bytecount);
  
  tex->texture_uploaded();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::setup_gltex
//       Access: Private
//  Description: Sets the GLTexture size, bits, and masks appropriate,
//               and allocates space for a pixmap.  Does not fill the
//               pixmap contents.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
setup_gltex(GLTexture *gltex, int orig_x_size, int orig_y_size) {
  // Ensure we have a power of two size, no more than our max.
  int s_size, s_bits;
  choose_tex_size(s_size, s_bits, orig_x_size);

  int t_size, t_bits;
  choose_tex_size(t_size, t_bits, orig_y_size);

  int bytecount = s_size * t_size * 4;

  gltex->xsize = s_size;
  gltex->ysize = t_size;

  if (gltex->pixmap != NULL) {
    gl_free(gltex->pixmap);
  }
  gltex->pixmap = (PIXEL *)gl_malloc(bytecount);

  gltex->s_max = 1 << (s_bits + ZB_POINT_ST_FRAC_BITS);
  gltex->s_mask = (1 << (s_bits + ZB_POINT_ST_FRAC_BITS)) - (1 << ZB_POINT_ST_FRAC_BITS);
  gltex->t_max = 1 << (t_bits + ZB_POINT_ST_FRAC_BITS);
  gltex->t_mask = (1 << (t_bits + ZB_POINT_ST_FRAC_BITS)) - (1 << ZB_POINT_ST_FRAC_BITS);
  gltex->t_shift = (ZB_POINT_ST_FRAC_BITS - s_bits);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::choose_tex_size
//       Access: Private
//  Description: Chooses the suitable texture size that is the closest
//               power-of-2 match to the indicated original size.
//               Also calculates the bit shift count, such that (1 <<
//               bits) == size.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
choose_tex_size(int &size, int &bits, int orig_size) {
  unsigned int filled = flood_bits_down((unsigned int)orig_size);
  size = filled + 1;
  if (size > orig_size) {
    // Round down, not up, to next lowest power of 2.
    size >>= 1;
  }
  if (size > _max_texture_dimension) {
    size = _max_texture_dimension;
  }

  bits = count_bits_in_word((unsigned int)size - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_lum_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel luminance image
//               from the texture into the indicated GLTexture.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_lum_image(GLTexture *gltex, Texture *tex) {
  nassertv(tex->get_num_components() == 1);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  int xsize_dest = gltex->xsize;
  int ysize_dest = gltex->ysize;
  unsigned char *dest = (unsigned char *)gltex->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 1 * cw;
      
      dpix[0] = spix[co];
      dpix[1] = spix[co];
      dpix[2] = spix[co];
      dpix[3] = 0xff;
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_alpha_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel alpha image
//               from the texture into the indicated GLTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_alpha_image(GLTexture *gltex, Texture *tex) {
  nassertv(tex->get_num_components() == 1);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  int xsize_dest = gltex->xsize;
  int ysize_dest = gltex->ysize;
  unsigned char *dest = (unsigned char *)gltex->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 1 * cw;
      
      dpix[0] = 0xff;
      dpix[1] = 0xff;
      dpix[2] = 0xff;
      dpix[3] = spix[co];
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_one_channel_image
//       Access: Private, Static
//  Description: Copies and scales the one-channel image (with a
//               single channel, e.g. red, green, or blue) from
//               the texture into the indicated GLTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_one_channel_image(GLTexture *gltex, Texture *tex, int channel) {
  nassertv(tex->get_num_components() == 1);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  int xsize_dest = gltex->xsize;
  int ysize_dest = gltex->ysize;
  unsigned char *dest = (unsigned char *)gltex->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 1 * cw;

      dpix[0] = 0;
      dpix[1] = 0;
      dpix[2] = 0;
      dpix[3] = 0xff;
      dpix[channel] = spix[co];
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_la_image
//       Access: Private, Static
//  Description: Copies and scales the two-channel luminance-alpha
//               image from the texture into the indicated GLTexture
//               pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_la_image(GLTexture *gltex, Texture *tex) {
  nassertv(tex->get_num_components() == 2);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  int xsize_dest = gltex->xsize;
  int ysize_dest = gltex->ysize;
  unsigned char *dest = (unsigned char *)gltex->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 2 * cw;
      
      dpix[0] = spix[co];
      dpix[1] = spix[co];
      dpix[2] = spix[co];
      dpix[3] = spix[cw + co];
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_rgb_image
//       Access: Private, Static
//  Description: Copies and scales the three-channel RGB image from
//               the texture into the indicated GLTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_rgb_image(GLTexture *gltex, Texture *tex) {
  nassertv(tex->get_num_components() == 3);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  int xsize_dest = gltex->xsize;
  int ysize_dest = gltex->ysize;
  unsigned char *dest = (unsigned char *)gltex->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 3 * cw;
      
      dpix[0] = spix[co];
      dpix[1] = spix[cw + co];
      dpix[2] = spix[cw + cw + co];
      dpix[3] = 0xff;
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::copy_rgba_image
//       Access: Private, Static
//  Description: Copies and scales the four-channel RGBA image from
//               the texture into the indicated GLTexture pixmap.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
copy_rgba_image(GLTexture *gltex, Texture *tex) {
  nassertv(tex->get_num_components() == 4);

  int xsize_src = tex->get_x_size();
  int ysize_src = tex->get_y_size();
  CPTA_uchar src_image = tex->get_ram_image();
  nassertv(!src_image.is_null());
  const unsigned char *src = src_image.p();

  // Component width, and offset to the high-order byte.
  int cw = tex->get_component_width();
#ifdef WORDS_BIGENDIAN
  // Big-endian: the high-order byte is always first.
  static const int co = 0;
#else
  // Little-endian: the high-order byte is last.
  int co = cw - 1;
#endif

  int xsize_dest = gltex->xsize;
  int ysize_dest = gltex->ysize;
  unsigned char *dest = (unsigned char *)gltex->pixmap;
  nassertv(dest != NULL);

  int sx_inc = (int)((float)(xsize_src) / (float)(xsize_dest));
  int sy_inc = (int)((float)(ysize_src) / (float)(ysize_dest));

  unsigned char *dpix = dest;
  int syn = 0;
  for (int dy = 0; dy < ysize_dest; dy++) {
    int sy = syn / ysize_dest;
    int sxn = 0;
    for (int dx = 0; dx < xsize_dest; dx++) {
      int sx = sxn / xsize_dest;
      const unsigned char *spix = src + (sy * xsize_src + sx) * 4 * cw;
      
      dpix[0] = spix[co];
      dpix[1] = spix[cw + co];
      dpix[2] = spix[cw + cw + co];
      dpix[3] = spix[cw + cw + cw + co];
      
      dpix += 4;
      sxn += xsize_src;
    }
    syn += ysize_src;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::setup_material
//       Access: Private
//  Description: Applies the desired parametesr to the indicated
//               GLMaterial object.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
setup_material(GLMaterial *gl_material, const Material *material) {
  const Colorf &specular = material->get_specular();
  gl_material->specular.X = specular[0];
  gl_material->specular.Y = specular[1];
  gl_material->specular.Z = specular[2];
  gl_material->specular.W = specular[3];

  const Colorf &emission = material->get_emission();
  gl_material->emission.X = emission[0];
  gl_material->emission.Y = emission[1];
  gl_material->emission.Z = emission[2];
  gl_material->emission.W = emission[3];

  gl_material->shininess = material->get_shininess();
  gl_material->shininess_i = (int)((material->get_shininess() / 128.0f) * SPECULAR_BUFFER_RESOLUTION);

  _color_material_flags = CMF_ambient | CMF_diffuse;

  if (material->has_ambient()) {
    const Colorf &ambient = material->get_ambient();
    gl_material->ambient.X = ambient[0];
    gl_material->ambient.Y = ambient[1];
    gl_material->ambient.Z = ambient[2];
    gl_material->ambient.W = ambient[3];

    _color_material_flags &= ~CMF_ambient;
  }

  if (material->has_diffuse()) {
    const Colorf &diffuse = material->get_diffuse();
    gl_material->diffuse.X = diffuse[0];
    gl_material->diffuse.Y = diffuse[1];
    gl_material->diffuse.Z = diffuse[2];
    gl_material->diffuse.W = diffuse[3];

    _color_material_flags &= ~CMF_diffuse;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsStateGuardian::load_matrix
//       Access: Private, Static
//  Description: Copies the Panda matrix stored in the indicated
//               TransformState object into the indicated TinyGL
//               matrix.
////////////////////////////////////////////////////////////////////
void TinyGraphicsStateGuardian::
load_matrix(M4 *matrix, const TransformState *transform) {
  const LMatrix4f &pm = transform->get_mat();
  for (int i = 0; i < 4; ++i) {
    matrix->m[0][i] = pm.get_cell(i, 0);
    matrix->m[1][i] = pm.get_cell(i, 1);
    matrix->m[2][i] = pm.get_cell(i, 2);
    matrix->m[3][i] = pm.get_cell(i, 3);
  }
}

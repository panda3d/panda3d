// Filename: geomIssuer.cxx
// Created by:  drose (03Feb99)
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

#include "geomIssuer.h"
#include "geom.h"
#include "graphicsStateGuardianBase.h"

static void
issue_vertex_noop(const Geom *, Geom::VertexIterator &,
                  GraphicsStateGuardianBase *) {
}

static void
issue_normal_noop(const Geom *, Geom::NormalIterator &,
                  GraphicsStateGuardianBase *) {
}

static void
issue_texcoord_noop(const Geom *, Geom::TexCoordIterator &,
                  GraphicsStateGuardianBase *) {
}

static void
issue_multitexcoord_noop(const Geom *, Geom::MultiTexCoordIterator &,
                         GraphicsStateGuardianBase *) {
}

static void
issue_color_noop(const Geom *, Geom::ColorIterator &, 
                 GraphicsStateGuardianBase *) {
}

static GeomIssuer noop_issuer;


////////////////////////////////////////////////////////////////////
//     Function: GeomIssuer::Constructor
//       Access: Public
//  Description: The default constructor makes a no-op GeomIssuer.
//               It's primarily intended to create the static
//               _noop_issuer once and only once; normally, the real
//               constructor, below, will be used.
////////////////////////////////////////////////////////////////////
GeomIssuer::
GeomIssuer() {
  for (int i = 0; i < num_GeomBindTypes; i++) {
    _vertex_command[i] = issue_vertex_noop;
    _normal_command[i] = issue_normal_noop;
    _texcoord_command[i] = issue_texcoord_noop;
    _multitexcoord_command[i] = issue_multitexcoord_noop;
    _color_command[i] = issue_color_noop;
  }
  _geom = NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomIssuer::Constructor
//       Access: Public
//  Description: This is the real constructor.  Given a Geom, the
//               current gsg, and a series of functions that, when
//               called, will actually issue the vertex/normal/whatnot
//               to the rendering engine.  It will construct a
//               GeomIssuer with these pointers in the appropriate
//               places to either issue the component or do nothing,
//               according to the requirements of the geom and of the
//               current state of the gsg.
//
//               This constructor is deprecated; it only supports
//               single-stage texturing.  Use the next constructor for
//               multitexturing support.
////////////////////////////////////////////////////////////////////
GeomIssuer::
GeomIssuer(const Geom *geom,
           GraphicsStateGuardianBase *gsg,
           IssueVertex *vertex,
           IssueNormal *normal,
           IssueTexCoord *texcoord,
           IssueColor *color) {
  memcpy(this, &noop_issuer, sizeof(GeomIssuer));
  _geom = geom;
  _gsg = gsg;

  // Issue vertices by default (we might not want to if we're doing
  // performance analysis)
  if (vertex != NULL) {
    _vertex_command[geom->get_binding(G_COORD)] = vertex;
  }

  // Issue normals only if we have them and the gsg says we should.
  if (normal != NULL && gsg->wants_normals()) {
    _normal_command[geom->get_binding(G_NORMAL)] = normal;
  }

  // Issue texcoords if we have them and the gsg wants them.
  if (texcoord != NULL && gsg->wants_texcoords()) {
    _texcoord_command[geom->get_binding(G_TEXCOORD)] = texcoord;
  }

  // And ditto for colors.
  if (color != NULL && gsg->wants_colors()) {
    _color_command[geom->get_binding(G_COLOR)] = color;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomIssuer::Constructor
//       Access: Public
//  Description: This constructor supports switchably issuing
//               multitexture calls.  If the MultiTexCoordIterator
//               indicates that multitexturing is in use (e.g. texture
//               coordinates are to be issued for any stage other than
//               stage 0), then multi_texcoord() will be called for
//               each issue_texcoord() call.  Otherwise,
//               single_texcoord() will be called.
////////////////////////////////////////////////////////////////////
GeomIssuer::
GeomIssuer(const Geom *geom,
           GraphicsStateGuardianBase *gsg,
           IssueVertex *vertex,
           IssueNormal *normal,
           IssueColor *color,
           IssueMultiTexCoord *single_texcoord,
           IssueMultiTexCoord *multi_texcoord,
           const Geom::MultiTexCoordIterator &ti) {
  memcpy(this, &noop_issuer, sizeof(GeomIssuer));
  _geom = geom;
  _gsg = gsg;

  // Issue vertices by default (we might not want to if we're doing
  // performance analysis)
  if (vertex != NULL) {
    _vertex_command[geom->get_binding(G_COORD)] = vertex;
  }

  // Issue normals only if we have them and the gsg says we should.
  if (normal != NULL && gsg->wants_normals()) {
    _normal_command[geom->get_binding(G_NORMAL)] = normal;
  }

  // And ditto for colors.
  if (color != NULL && gsg->wants_colors()) {
    _color_command[geom->get_binding(G_COLOR)] = color;
  }

  // Issue texcoords if we have them and the gsg wants them.
  if ((single_texcoord != NULL && multi_texcoord != NULL) && 
      ti._num_stages > 0 && gsg->wants_texcoords()) {
    if (ti._num_stages > 1 || ti._stage_index[0] != 0) {
      _multitexcoord_command[G_PER_VERTEX] = multi_texcoord;
    } else {
      _multitexcoord_command[G_PER_VERTEX] = single_texcoord;
    }
  }
}

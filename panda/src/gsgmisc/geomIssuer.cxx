// Filename: geomIssuer.cxx
// Created by:  drose (03Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "geomIssuer.h"
#include <geom.h>
#include <graphicsStateGuardianBase.h>

static void
issue_vertex_noop(const Geom *, Geom::VertexIterator &) {
}

static void
issue_normal_noop(const Geom *, Geom::NormalIterator &) {
}

static void
issue_texcoord_noop(const Geom *, Geom::TexCoordIterator &) {
}

static void
issue_color_noop(const Geom *, Geom::ColorIterator &, const GraphicsStateGuardianBase *) {
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
////////////////////////////////////////////////////////////////////
GeomIssuer::
GeomIssuer(const Geom *geom,
	   const GraphicsStateGuardianBase *gsg,
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

// Filename: geomIssuer.h
// Created by:  drose (03Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef GEOMISSUER_H
#define GEOMISSUER_H

#include <pandabase.h>

#include <nodeTransition.h>
#include <luse.h>
#include <typedef.h>
#include <geom.h>

class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : GeomIssuer
// Description : This is a utility class used by the various
//               GraphicsStateGuardians to issue the vertex/normal/etc
//               commands to the rendering engine.  Given a geom and a
//               gsg, as well as a set of functions that actually do
//               the work of issuing vertices etc. to the rendering
//               backend, it configures itself so that subsequent
//               calls to issue_normal() (for instance) will either do
//               nothing or issue a normal, depending on whether the
//               requested binding type matches the geom's actual
//               binding type for normals, and on whether vertices are
//               required by the current state.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomIssuer {
public:

  // Declare some function types.  This declares several typenames
  // which are pointers to function types--these are not themselves
  // functions.  A function pointed to by a variable of this type,
  // when given a Geom and an associated iterator, will issue the
  // vertex (or whatever) referenced by the iterator to the rendering
  // backend, and increment the iterator.
  typedef void IssueVertex(const Geom *, Geom::VertexIterator &);
  typedef void IssueNormal(const Geom *, Geom::NormalIterator &);
  typedef void IssueTexCoord(const Geom *, Geom::TexCoordIterator &);
  typedef void IssueColor(const Geom *, Geom::ColorIterator &, const GraphicsStateGuardianBase *gsg);

  GeomIssuer();
  GeomIssuer(const Geom *geom,
             const GraphicsStateGuardianBase *gsg,
             IssueVertex *vertex,
             IssueNormal *normal,
             IssueTexCoord *texcoord,
             IssueColor *color);

  INLINE void issue_vertex(GeomBindType bind,
                           Geom::VertexIterator &i);
  INLINE void issue_normal(GeomBindType bind,
                           Geom::NormalIterator &i);
  INLINE void issue_texcoord(GeomBindType bind,
                             Geom::TexCoordIterator &i);
  INLINE void issue_color(GeomBindType bind,
                          Geom::ColorIterator &i);

protected:
  const Geom *_geom;
  const GraphicsStateGuardianBase *_gsg;
  IssueVertex *_vertex_command[num_GeomBindTypes];
  IssueNormal *_normal_command[num_GeomBindTypes];
  IssueTexCoord *_texcoord_command[num_GeomBindTypes];
  IssueColor *_color_command[num_GeomBindTypes];
};

#include "geomIssuer.I"
 
#endif

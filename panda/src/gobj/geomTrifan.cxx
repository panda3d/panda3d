// Filename: geomTrifan.cxx
// Created by:  charles (13Jul00)
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

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "ioPtaDatagramShort.h"
#include "ioPtaDatagramInt.h"
#include "ioPtaDatagramLinMath.h"
#include "graphicsStateGuardianBase.h"

#include "geomTri.h"
#include "geomTrifan.h"

TypeHandle GeomTrifan::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifan::get_num_tris
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int GeomTrifan::
get_num_tris() const {
  int numtris = 0;
  for (int i = 0; i < _numprims; i++) {
    numtris += _primlengths[i] - 2;
  }
  return numtris;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifan::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomTrifan::
make_copy() const {
  return new GeomTrifan(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifan::print_draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomTrifan::
print_draw_immediate( void ) const
{
  /*
    int i;
    int j;
    int nprims = _numprims;
    int* plen = _primlengths;
    Vertexf* tcoords = _coords;
    Normalf* tnorms = _norms;
    Colorf* tcolors = _colors;
    TexCoordf* ttexcoords = _texcoords;
    ushort* tvindex = _vindex;
    ushort* tnindex = _nindex;
    ushort* tcindex = _cindex;
    ushort* ttindex = _tindex;

    // Draw overall
    if ( _color_command[G_OVERALL] != _issue_color_noop )
    {
        nout << "Color (Overall): ";
        if ( tcindex )
        {
            nout << "idx: " << *tcindex << " ";
            nout << tcolors[*(tcindex++)];
        }
        else
            nout << *(tcolors++);
        nout << endl;
    }
    if ( _normal_command[G_OVERALL] != _issue_normal_noop )
    {
        nout << "Normal (Overall): ";
        if ( tnindex )
        {
            nout << "idx: " << *tnindex << " ";
            nout << tnorms[*(tnindex++)];
        }
        else
            nout << *(tnorms++);
        nout << endl;
    }

    for ( i = nprims; i > 0; i-- )
    {
        // Draw per primitive
        if ( _color_command[G_PER_PRIM] != _issue_color_noop )
        {
            nout << "Color (Per Prim): ";
            if ( tcindex )
            {
                nout << "idx: " << *tcindex << " ";
                nout << tcolors[*(tcindex++)];
            }
            else
                nout << *(tcolors++);
            nout << endl;
        }
        if ( _normal_command[G_PER_PRIM] != _issue_normal_noop )
        {
            nout << "Normal (Per Prim): ";
            if ( tnindex )
            {
                nout << "idx: " << *tnindex << " ";
                nout << tnorms[*(tnindex++)];
            }
            else
                nout << *(tnorms++);
            nout << endl;
        }

        nout << "BeginGfx()" << endl;

        for ( j = *(plen++); j > 0; j-- )
        {
            // Draw per vertex
            if ( _color_command[G_PER_VERTEX] != _issue_color_noop )
            {
                nout << "Color (Per Vertex): ";
                if ( tcindex )
                {
                    nout << "idx: " << *tcindex << " ";
                    nout << tcolors[*(tcindex++)];
                }
                else
                    nout << *(tcolors++);
                nout << endl;
            }
            if ( _normal_command[G_PER_VERTEX] != _issue_normal_noop )
            {
                nout << "Normal (Per Vertex): ";
                if ( tnindex )
                {
                    nout << "idx: " << *tnindex << " ";
                    nout << tnorms[*(tnindex++)];
                }
                else
                    nout << *(tnorms++);
                nout << endl;
            }
            if ( _texcoord_command[G_PER_VERTEX] != _issue_texcoord_noop )
            {
                nout << "TexCoord (Per Vertex): ";
                if ( ttindex )
                {
                    nout << "idx: " << *ttindex << " ";
                    nout << ttexcoords[*(ttindex++)];
                }
                else
                    nout << *(ttexcoords++);
                nout << endl;
            }
            if ( _vertex_command[G_PER_VERTEX] != _issue_vertex_noop )
            {
                nout << "Vertex (Per Vertex): ";
                if ( tvindex )
                {
                    nout << "idx: " << *tvindex << " ";
                    nout << tcoords[*(tvindex++)];
                }
                else
                    nout << *(tcoords++);
                nout << endl;
            }
        }

        nout << "EndGfx()" << endl;
    }
    */
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifan::draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomTrifan::
draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc) {
  gsg->draw_trifan(this, gc);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifan::explode
//       Access: Public, Virtual
//  Description: Allocate and return a new Geom which represents the
//               individual triangles that make up the trifan.
////////////////////////////////////////////////////////////////////
Geom *GeomTrifan::
explode() const {
  PTA_Vertexf coords=PTA_Vertexf::empty_array(0);
  PTA_Normalf normals=PTA_Normalf::empty_array(0);
  PTA_TexCoordf texcoords=PTA_TexCoordf::empty_array(0);
  PTA_Colorf colors=PTA_Colorf::empty_array(0);

  VertexIterator vi = make_vertex_iterator();
  NormalIterator ni = make_normal_iterator();
  TexCoordIterator ti = make_texcoord_iterator();
  ColorIterator ci = make_color_iterator();

  // Get overall values
  if (get_binding(G_COLOR) == G_OVERALL) {
    colors.push_back(get_next_color(ci));
  }
  if (get_binding(G_NORMAL) == G_OVERALL) {
    normals.push_back(get_next_normal(ni));
  }

  int num_tris = 0;
  int num_prims = get_num_prims();

  for (int i = 0; i < num_prims; i++) {
    // Get per-primitive values
    Colorf per_trifan_color;
    Normalf per_trifan_normal;
    if (get_binding(G_COLOR) == G_PER_PRIM) {
      per_trifan_color = get_next_color(ci);
    }
    if (get_binding(G_NORMAL) == G_PER_PRIM) {
      per_trifan_normal = get_next_normal(ni);
    }

    int num_verts = get_length(i);
    assert(num_verts >= 3);

    // A few temporary arrays to hold the three most recent per-vertex
    // values.
    Vertexf f3vertex[3];
    Normalf f3normal[3];
    Colorf f3color[3];
    TexCoordf f3texcoord[3];

    // Get the first two vertices.  We get these into positions [0]
    // and [2].

    int v;
    for (v = 0; v <= 2; v += 2) {
      if (get_binding(G_COORD) == G_PER_VERTEX) {
        f3vertex[v] = get_next_vertex(vi);
      }
      if (get_binding(G_NORMAL) == G_PER_VERTEX) {
        f3normal[v] = get_next_normal(ni);
      }
      if (get_binding(G_TEXCOORD) == G_PER_VERTEX) {
        f3texcoord[v] = get_next_texcoord(ti);
      }
      if (get_binding(G_COLOR) == G_PER_VERTEX) {
        f3color[v] = get_next_color(ci);
      }
    }

    // Now fill each triangle.  Each vertex from this point on defines
    // a new triangle.
    for (v = 2; v < num_verts; v++) {
      num_tris++;

      f3vertex[1] = f3vertex[2];
      f3normal[1] = f3normal[2];
      f3texcoord[1] = f3texcoord[2];
      f3color[1] = f3color[2];

      if (get_binding(G_COLOR) == G_PER_PRIM) {
        colors.push_back(per_trifan_color);
      } else if (get_binding(G_COLOR) == G_PER_COMPONENT) {
        colors.push_back(get_next_color(ci));
      }

      if (get_binding(G_NORMAL) == G_PER_PRIM) {
        normals.push_back(per_trifan_normal);
      } else if (get_binding(G_NORMAL) == G_PER_COMPONENT) {
        normals.push_back(get_next_normal(ni));
      }

      // Per-vertex attributes.
      assert(get_binding(G_COORD) == G_PER_VERTEX);
      f3vertex[2] = get_next_vertex(vi);
      coords.push_back(f3vertex[0]);
      coords.push_back(f3vertex[1]);
      coords.push_back(f3vertex[2]);

      if (get_binding(G_NORMAL) == G_PER_VERTEX) {
        f3normal[2] = get_next_normal(ni);
        normals.push_back(f3normal[0]);
        normals.push_back(f3normal[1]);
        normals.push_back(f3normal[2]);
      }
      if (get_binding(G_TEXCOORD) == G_PER_VERTEX) {
        f3texcoord[2] = get_next_texcoord(ti);
        texcoords.push_back(f3texcoord[0]);
        texcoords.push_back(f3texcoord[1]);
        texcoords.push_back(f3texcoord[2]);
      }
      if (get_binding(G_COLOR) == G_PER_VERTEX) {
        f3color[2] = get_next_color(ci);
        colors.push_back(f3color[0]);
        colors.push_back(f3color[1]);
        colors.push_back(f3color[2]);
      }
    }
  }

  Geom *tris = new GeomTri;
  tris->set_coords(coords);

  tris->set_normals(normals,
                    get_binding(G_NORMAL) == G_PER_COMPONENT ?
                    G_PER_PRIM : get_binding(G_NORMAL));
  tris->set_texcoords(texcoords, get_binding(G_TEXCOORD));
  tris->set_colors(colors,
                   get_binding(G_COLOR) == G_PER_COMPONENT ?
                   G_PER_PRIM : get_binding(G_COLOR));
  tris->set_num_prims(num_tris);

  return tris;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifan::get_tris
//       Access: Public, Virtual
//  Description: This is similar in principle to explode(), except it
//               returns only a list of triangle vertex indices, with
//               no information about color or whatever.  The array
//               returned is a set of indices into the geom's _coords
//               array, as retrieve by get_coords(); there will be 3*n
//               elements in the array, where n is the number of
//               triangles described by the geometry.  This is useful
//               when it's important to determine the physical
//               structure of the geometry, without necessarily
//               worrying about its rendering properties, and when
//               performance considerations are not overwhelming.
////////////////////////////////////////////////////////////////////
PTA_ushort GeomTrifan::
get_tris() const {
  int num_tris = get_num_tris();
  PTA_ushort tris;
  tris.reserve(num_tris * 3);

  int k = 0;

  for (int i = 0; i < _numprims; i++) {
    ushort v0, v1, v2;
    if (_vindex.empty()) {
      v0 = k++;
      v1 = k++;
    } else {
      v0 = _vindex[k++];
      v1 = _vindex[k++];
    }

    int len = _primlengths[i] - 2;

    for (int j = 0; j < len; j++) {
      if (_vindex.empty()) {
        v2 = k++;
      } else {
        v2 = _vindex[k++];
      }

      tris.push_back(v0);
      tris.push_back(v1);
      tris.push_back(v2);

      v1 = v2;
    }
  }

  nassertr((int)tris.size() == num_tris * 3, PTA_ushort());
  return tris;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifan::make_GeomTrifan
//       Access: Protected
//  Description: Factory method to generate a GeomTrifan object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomTrifan::
make_GeomTrifan(const FactoryParams &params) {
  GeomTrifan *me = new GeomTrifan;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifan::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomTrifan object
////////////////////////////////////////////////////////////////////
void GeomTrifan::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomTrifan);
}


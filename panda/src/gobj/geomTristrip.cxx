// Filename: geomTristrip.cxx
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
#include "geomTristrip.h"

TypeHandle GeomTristrip::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: GeomTristrip::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomTristrip::
make_copy() const {
  return new GeomTristrip(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrip::print_draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomTristrip::print_draw_immediate( void ) const
{
  /*
    int i;
    int j;
    int k;
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

        for ( j = *(plen++); j > 1; j -= 2 )
        {
            // Draw per component
            if ( _color_command[G_PER_COMPONENT] != _issue_color_noop )
            {
                nout << "Color (Per Component): ";
                if ( tcindex )
                {
                    nout << "idx: " << *tcindex << " ";
                    nout << tcolors[*(tcindex++)];
                }
                else
                    nout << *(tcolors++);
                nout << endl;
            }
            if ( _normal_command[G_PER_COMPONENT] != _issue_normal_noop )
            {
                nout << "Normal (Per Component): ";
                if ( tnindex )
                {
                    nout << "idx: " << *tnindex << " ";
                    nout << tnorms[*(tnindex++)];
                }
                else
                    nout << *(tnorms++);
                nout << endl;
            }

            for ( k = 0; k < 2; k++ )
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
        }
        if ( !j )
        {
            nout << "EndGfx()" << endl;
            continue;
        }
        else
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

            nout << "EndGfx()" << endl;
            continue;
        }
    }
    */
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrip::draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomTristrip::draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc) {
  gsg->draw_tristrip(this, gc);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrip::get_num_tris
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int
GeomTristrip::get_num_tris() const {
  int numtris = 0;
  for (int i = 0; i < _numprims; i++) {
    numtris += _primlengths[i] - 2;
  }
  return numtris;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrip::explode
//       Access: Public, Virtual
//  Description: Allocate and return a new Geom which represents the
//               individual triangles that make up the tristrip.
////////////////////////////////////////////////////////////////////
Geom *GeomTristrip::
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
    Colorf per_tristrip_color;
    Normalf per_tristrip_normal;
    if (get_binding(G_COLOR) == G_PER_PRIM) {
      per_tristrip_color = get_next_color(ci);
    }
    if (get_binding(G_NORMAL) == G_PER_PRIM) {
      per_tristrip_normal = get_next_normal(ni);
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
    // and [2], to prepare for the back-and-forth switchback logic in
    // the following loop.

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

      if (get_binding(G_COLOR) == G_PER_PRIM) {
        colors.push_back(per_tristrip_color);
      } else if (get_binding(G_COLOR) == G_PER_COMPONENT) {
        colors.push_back(get_next_color(ci));
      }

      if (get_binding(G_NORMAL) == G_PER_PRIM) {
        normals.push_back(per_tristrip_normal);
      } else if (get_binding(G_NORMAL) == G_PER_COMPONENT) {
        normals.push_back(get_next_normal(ni));
      }

      if ((v & 1) == 0) {
        // Even.  The triangle is counter-clockwise.
        f3vertex[1] = f3vertex[2];
        f3normal[1] = f3normal[2];
        f3texcoord[1] = f3texcoord[2];
        f3color[1] = f3color[2];
      } else {
        // Odd.  The triangle is clockwise.
        f3vertex[0] = f3vertex[2];
        f3normal[0] = f3normal[2];
        f3texcoord[0] = f3texcoord[2];
        f3color[0] = f3color[2];
      }

      // Per-vertex attributes.
      if (get_binding(G_COORD) == G_PER_VERTEX) {
        f3vertex[2] = get_next_vertex(vi);
        coords.push_back(f3vertex[0]);
        coords.push_back(f3vertex[1]);
        coords.push_back(f3vertex[2]);
      }
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
//     Function: GeomTristrip::get_tris
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
PTA_ushort GeomTristrip::
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

    bool even = true;
    int len = _primlengths[i];

    for (int j = 2; j < len; j++) {
      if (_vindex.empty()) {
        v2 = k++;
      } else {
        v2 = _vindex[k++];
      }

      even = !even;

      if (even) {
        ushort vtmp = v1;
        v1 = v2;
        v2 = vtmp;
      }

      tris.push_back(v0);
      tris.push_back(v1);
      tris.push_back(v2);

      if (even) {
        v0 = v2;
      } else {
        v0 = v1;
        v1 = v2;
      }
    }
  }

  nassertr((int)tris.size() == num_tris * 3, PTA_ushort());
  return tris;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrip::make_GeomTristrip
//       Access: Protected
//  Description: Factory method to generate a GeomTristrip object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomTristrip::
make_GeomTristrip(const FactoryParams &params) {
  GeomTristrip *me = new GeomTristrip;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrip::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomTristrip object
////////////////////////////////////////////////////////////////////
void GeomTristrip::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomTristrip);
}

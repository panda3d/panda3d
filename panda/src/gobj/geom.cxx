// Filename: geom.cxx
// Created by:  mike (09Jan97)
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

#include "geom.h"
#include "config_gobj.h"

#include "graphicsStateGuardianBase.h"
#include "geometricBoundingVolume.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "ioPtaDatagramShort.h"
#include "ioPtaDatagramInt.h"
#include "ioPtaDatagramLinMath.h"
#include "preparedGraphicsObjects.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////

TypeHandle Geom::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: get_*_nonindexed
//  Description: Retrieves the next component of the nonindexed array.
////////////////////////////////////////////////////////////////////
static const Vertexf &get_vertex_nonindexed(Geom::VertexIterator &vi) {
  return *(vi._array++);
}
static const Normalf &get_normal_nonindexed(Geom::NormalIterator &vi) {
  return *(vi._array++);
}
static const TexCoordf &get_texcoord_nonindexed(Geom::TexCoordIterator &vi) {
  return *(vi._array++);
}
static const Colorf &get_color_nonindexed(Geom::ColorIterator &vi) {
  return *(vi._array++);
}

////////////////////////////////////////////////////////////////////
//     Function: get_*_indexed
//  Description: Retrieves the next component of the indexed array.
////////////////////////////////////////////////////////////////////
static const Vertexf &get_vertex_indexed(Geom::VertexIterator &vi) {
  return vi._array[*(vi._index++)];
}
static const Normalf &get_normal_indexed(Geom::NormalIterator &vi) {
  return vi._array[*(vi._index++)];
}
static const TexCoordf &get_texcoord_indexed(Geom::TexCoordIterator &vi) {
  return vi._array[*(vi._index++)];
}
static const Colorf &get_color_indexed(Geom::ColorIterator &vi) {
  return vi._array[*(vi._index++)];
}

////////////////////////////////////////////////////////////////////
//     Function: get_*_noop
//  Description: Doesn't retrieve anything at all.
////////////////////////////////////////////////////////////////////
static const Vertexf &get_vertex_noop(Geom::VertexIterator &) {
  static Vertexf nothing;
  return nothing;
}
static const Normalf &get_normal_noop(Geom::NormalIterator &) {
  static Normalf nothing;
  return nothing;
}
static const TexCoordf &get_texcoord_noop(Geom::TexCoordIterator &) {
  static TexCoordf nothing;
  return nothing;
}
static const Colorf &get_color_noop(Geom::ColorIterator &) {
  static Colorf nothing;
  return nothing;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomBindType output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, GeomBindType t) {
  switch (t) {
  case G_OFF:
    return out << "off";
  case G_OVERALL:
    return out << "overall";
  case G_PER_PRIM:
    return out << "per prim";
  case G_PER_COMPONENT:
    return out << "per component";
  case G_PER_VERTEX:
    return out << "per vertex";
  }
  return out << "(**invalid**)";
}


////////////////////////////////////////////////////////////////////
//     Function: GeomAttrType output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, GeomAttrType t) {
  switch (t) {
  case G_COORD:
    return out << "coord";
  case G_COLOR:
    return out << "color";
  case G_NORMAL:
    return out << "normal";
  case G_TEXCOORD:
    return out << "texcoord";
  }
  return out << "(**invalid**)";
}

////////////////////////////////////////////////////////////////////
//     Function: describe_attr
//  Description: A handy helper function for output_verbose,
//               below.
////////////////////////////////////////////////////////////////////
template <class VecType>
static void
describe_attr(ostream &out, const Geom *geom,
              GeomBindType bind, const PTA(VecType) &array,
              bool newline, int indent_level) {
  PTA_int lengths = geom->get_lengths();
  int num_prims = geom->get_num_prims();
  bool components = geom->uses_components();

  int i, j, vi;
  switch (bind) {
  case G_PER_VERTEX:
    indent(out, indent_level)
      << "Per vertex:";
    vi = 0;
    int num_verts;
    num_verts = geom->get_num_vertices_per_prim();
    for (i = 0; i < num_prims; i++) {
      if (components) {
        num_verts = lengths[i];
      }
      out << "\n";
      indent(out, indent_level) << "[ ";
      if (num_verts > 0) {
        out << array[vi++];
        for (j = 1; j < num_verts; j++) {
          if (newline) {
            out << "\n";
            indent(out, indent_level + 2);
          } else {
            out << " ";
          }
          out << array[vi++];
        }
      }
      out << " ]";
    }
    break;

  case G_PER_COMPONENT:
    if (!components) {
      indent(out, indent_level)
        << "Invalid per-component attribute specified!";
    } else {
      indent(out, indent_level)
        << "Per component:";
      vi = 0;
      for (i = 0; i < num_prims; i++) {
        num_verts = lengths[i] - geom->get_num_more_vertices_than_components();
        out << "\n";
        indent(out, indent_level) << "[ ";
        if (num_verts > 0) {
          out << array[vi++];
          for (j = 1; j < num_verts; j++) {
            if (newline) {
              out << "\n";
              indent(out, indent_level + 2);
            } else {
              out << " ";
            }
            out << array[vi++];
          }
          out << " ]";
        }
      }
    }
    break;

  case G_PER_PRIM:
    indent(out, indent_level)
      << "Per prim:";
    for (i = 0; i < num_prims; i++) {
      if (newline) {
        out << "\n";
        indent(out, indent_level + 2);
      } else {
        out << " ";
      }
      out << array[i];
    }
    break;

  case G_OVERALL:
    indent(out, indent_level)
      << "Overall:";
    if (newline) {
      out << "\n";
      indent(out, indent_level + 2);
    } else {
      out << " ";
    }
    out << array[0];

  case G_OFF:
    break;
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Geom::
Geom() {
  _all_dirty_flags = 0;
  init();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Geom::
Geom(const Geom &copy) : dDrawable() {
  _all_dirty_flags = 0;
  *this = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Geom::
~Geom() {
  release_all();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
operator = (const Geom &copy) {
  _coords = copy._coords;
  _norms = copy._norms;
  _colors = copy._colors;

  _vindex = copy._vindex;
  _nindex = copy._nindex;
  _cindex = copy._cindex;

  _texcoords_by_name = copy._texcoords_by_name;

  _numprims = copy._numprims;
  _num_vertices = copy._num_vertices;
  _primlengths = copy._primlengths;
  for (int i = 0; i < num_GeomAttrTypes; i++) {
    _bind[i] = copy._bind[i];
  }

  _get_vertex = copy._get_vertex;
  _get_normal = copy._get_normal;
  _get_color = copy._get_color;
  _get_texcoord = copy._get_texcoord;

  mark_bound_stale();
  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::transform_vertices
//       Access: Published
//  Description: Applies the indicated transform to all of the
//               vertices in the Geom.  If the Geom happens to share a
//               vertex table with another Geom, this operation will
//               duplicate the vertex table instead of breaking the
//               other Geom; however, if multiple Geoms with shared
//               tables are transformed by the same matrix, they will
//               no longer share tables after the operation.  Consider
//               using the GeomTransformer if you will be applying the
//               same transform to multiple Geoms.
////////////////////////////////////////////////////////////////////
void Geom::
transform_vertices(const LMatrix4f &mat) {
  PTA_Vertexf coords;
  PTA_ushort index;
  get_coords(coords, index);
  PTA_Vertexf new_coords;
  new_coords.reserve(coords.size());
  PTA_Vertexf::const_iterator vi;
  for (vi = coords.begin(); vi != coords.end(); ++vi) {
    new_coords.push_back((*vi) * mat);
  }
  nassertv(new_coords.size() == coords.size());
  set_coords(new_coords, index);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_coords
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
set_coords(const PTA_Vertexf &coords, 
           const PTA_ushort &vindex) {
  _coords = coords;
  _bind[G_COORD] = G_PER_VERTEX;
  _vindex = vindex;

  mark_bound_stale();
  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_coords
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
set_coords(const PTA_Vertexf &coords, GeomBindType bind,
           const PTA_ushort &vindex) {
  nassertv(bind==G_PER_VERTEX);
  set_coords(coords, vindex);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_normals
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
set_normals(const PTA_Normalf &norms, GeomBindType bind,
            const PTA_ushort &nindex) {
  _norms = norms;
  _bind[G_NORMAL] = bind;
  _nindex = nindex;

  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_colors
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
set_colors(const PTA_Colorf &colors, GeomBindType bind,
           const PTA_ushort &cindex) {
  _colors = colors;
  _bind[G_COLOR] = bind;
  _cindex = cindex;

  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_texcoords
//       Access: Published
//  Description: This single-texturing version of set_texcoords() only
//               changes the default texcoords name.  Use the version
//               of set_texcoords() that takes a TexCoordName
//               parameter to set up different texture coordinates for
//               different stages of a multitexture pipeline.
////////////////////////////////////////////////////////////////////
void Geom::
set_texcoords(const PTA_TexCoordf &texcoords, GeomBindType bind,
              const PTA_ushort &tindex) {
  nassertv(bind == G_PER_VERTEX || bind == G_OFF);

  if (bind == G_OFF) {
    remove_texcoords(TexCoordName::get_default());
  } else {
    set_texcoords(TexCoordName::get_default(), texcoords, tindex);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_texcoords
//       Access: Published
//  Description: Sets the texture coordinates for a particular name of
//               the pipeline.  This implicitly sets the binding of
//               these texture coordinates to G_PER_VERTEX.  See also
//               remove_texcoords().
////////////////////////////////////////////////////////////////////
void Geom::
set_texcoords(const TexCoordName *name, const PTA_TexCoordf &texcoords,
              const PTA_ushort &tindex) {
  nassertv(name != (TexCoordName *)NULL);

#ifndef NDEBUG
  // All of the texture coordinates must be either nonindexed, or all
  // must be indexed.  If there are already (different) texture
  // coordinates set on the Geom, then ensure this is so.
  remove_texcoords(name);
  if (!_texcoords_by_name.empty()) {
    TexCoordDef &def = (*_texcoords_by_name.begin()).second;
    bool previous_is_indexed = (def._tindex != (ushort *)NULL);
    bool this_is_indexed = (tindex != (ushort *)NULL);
    nassertv(this_is_indexed == previous_is_indexed);
  }
#endif

  TexCoordDef &def = _texcoords_by_name[name];
  def._texcoords = texcoords;
  def._tindex = tindex;

  if (name == TexCoordName::get_default()) {
    _bind[G_TEXCOORD] = G_PER_VERTEX;
  }

  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::remove_texcoords
//       Access: Published
//  Description: Removes the texture coordinates for a particular name
//               of the pipeline.  This implicitly sets the binding of
//               these texture coordinates to G_OFF.  See also
//               set_texcoords().
////////////////////////////////////////////////////////////////////
void Geom::
remove_texcoords(const TexCoordName *name) {
  _texcoords_by_name.erase(name);

  if (name == TexCoordName::get_default()) {
    _bind[G_TEXCOORD] = G_OFF;
  }

  make_dirty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_coords
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
get_coords(PTA_Vertexf &coords, PTA_ushort &vindex) const {
  coords = _coords;
  vindex = _vindex;

  // G_PER_VERTEX is implicit binding
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_coords
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
get_coords(PTA_Vertexf &coords, GeomBindType &bind,
           PTA_ushort &vindex) const {
  coords = _coords;
  bind = _bind[G_COORD];
  vindex = _vindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_normals
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
get_normals(PTA_Normalf &norms, GeomBindType &bind,
            PTA_ushort &nindex) const {
  norms = _norms;
  bind = _bind[G_NORMAL];
  nindex = _nindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_colors
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
get_colors(PTA_Colorf &colors, GeomBindType &bind,
           PTA_ushort &cindex) const {
  colors = _colors;
  bind = _bind[G_COLOR];
  cindex = _cindex;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_texcoords
//       Access: Public
//  Description: Returns the texcoords associated with the default
//               name only.  See has_texcoords(),
//               get_texcoords_array(), and get_texcoords_index() to
//               get the texcoords associated with an arbitrary name
//               of a multitexture pipeline.
////////////////////////////////////////////////////////////////////
void Geom::
get_texcoords(PTA_TexCoordf &texcoords, GeomBindType &bind,
              PTA_ushort &tindex) const { 
  TexCoordsByName::const_iterator tci = 
    _texcoords_by_name.find(TexCoordName::get_default());
  if (tci != _texcoords_by_name.end()) {
    const TexCoordDef &def = (*tci).second;
    texcoords = def._texcoords;
    bind = G_PER_VERTEX;
    tindex = def._tindex;
  } else {
    texcoords.clear();
    bind = G_OFF;
    tindex.clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::is_dynamic
//       Access: Published, Virtual
//  Description: Returns true if the Geom has any dynamic properties
//               that are expected to change from one frame to the
//               next, or false if the Geom is largely static.  For
//               now, this is the same thing as asking whether its
//               vertices are indexed.
////////////////////////////////////////////////////////////////////
bool Geom::
is_dynamic() const {
  return (_vindex != (ushort*)0L);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::prepare
//       Access: Published
//  Description: Indicates that the geom should be enqueued to be
//               prepared in the indicated prepared_objects at the
//               beginning of the next frame.  This will ensure the
//               geom is already loaded into the GSG if it is expected
//               to be rendered soon.
//
//               Use this function instead of prepare_now() to preload
//               geoms from a user interface standpoint.
////////////////////////////////////////////////////////////////////
void Geom::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_geom(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::explode
//       Access: Published, Virtual
//  Description: If the Geom is a composite type such as a tristrip,
//               this allocates and returns a new Geom that represents
//               the same geometry as a simple type, for instance a
//               set of triangles.  If the Geom is already a simple
//               type, this allocates and returns a copy.  This is
//               just a convenience function for dealing with
//               composite types when performance is less than
//               paramount.
////////////////////////////////////////////////////////////////////
Geom *Geom::
explode() const {
  return make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_tris
//       Access: Published, Virtual
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
PTA_ushort Geom::
get_tris() const {
  return PTA_ushort();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
output(ostream &out) const {
  out << get_type() << " (" << _numprims << ")";

  /*
      << " v:" << _coords.size()
      << " n:" << _norms.size()
      << " c:" << _colors.size()
      << " t:" << _texcoords.size()
      << " vi:" << _vindex.size()
      << " ni:" << _nindex.size()
      << " ci:" << _cindex.size()
      << " ti:" << _tindex.size();
  */
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::write_verbose
//       Access: Public
//  Description: Writes to the indicated ostream a formatted picture
//               of the contents of the Geom, in detail--but hopefully
//               not too much detail.
////////////////////////////////////////////////////////////////////
void Geom::
write_verbose(ostream &out, int indent_level) const {
  GeomBindType bind_normals;
  GeomBindType bind_colors;

  PTA_Vertexf g_coords;
  PTA_Normalf g_normals;
  PTA_Colorf g_colors;

  PTA_ushort i_coords;
  PTA_ushort i_normals;
  PTA_ushort i_colors;

  get_coords(g_coords, i_coords);
  get_normals(g_normals, bind_normals, i_normals);
  get_colors(g_colors, bind_colors, i_colors);

  out << "\n";
  indent(out, indent_level)
    << get_type() << " contains "
    << get_num_prims() << " primitives:\n";

  if ((i_coords == (ushort *)NULL) && (g_coords == (Vertexf *)NULL)) {
    indent(out, indent_level)
      << "No coords\n";
  } else if (i_coords!=(ushort*)0L) {
    indent(out, indent_level)
      << "Indexed coords = " << (void *)g_coords << ", length = "
      << g_coords.size() << ":\n";
    describe_attr(out, this, G_PER_VERTEX, i_coords, false, indent_level + 2);
  } else {
    indent(out, indent_level)
      << "Nonindexed coords:\n";
    describe_attr(out, this, G_PER_VERTEX, g_coords, true, indent_level + 2);
  }

  if (bind_colors == G_OFF) {
    indent(out, indent_level)
      << "No colors\n";
  } else if (i_colors!=(ushort*)0L) {
    indent(out, indent_level)
      << "Indexed colors = " << (void *)g_colors << ", length = "
      << g_colors.size() << "\n";
    describe_attr(out, this, bind_colors, i_colors, false, indent_level + 2);
  } else {
    indent(out, indent_level)
      << "Nonindexed colors:\n";
    describe_attr(out, this, bind_colors, g_colors, true, indent_level + 2);
  }

  if (bind_normals == G_OFF) {
    indent(out, indent_level)
      << "No normals\n";
  } else if (i_normals!=(ushort*)0L) {
    indent(out, indent_level)
      << "Indexed normals = " << (void *)g_normals << ", length = "
      << g_normals.size() << "\n";
    describe_attr(out, this, bind_normals, i_normals, false, indent_level + 2);
  } else {
    indent(out, indent_level)
      << "Nonindexed normals:\n";
    describe_attr(out, this, bind_normals, g_normals, true, indent_level + 2);
  }

  if (_texcoords_by_name.empty()) {
    indent(out, indent_level)
      << "No texcoords\n";

  } else {
    TexCoordsByName::const_iterator tci;
    for (tci = _texcoords_by_name.begin(); 
         tci != _texcoords_by_name.end();
         ++tci) {
      const TexCoordName *name = (*tci).first;
      const TexCoordDef &def = (*tci).second;
      if (def._tindex != (ushort *)NULL) {
        indent(out, indent_level)
          << "Indexed texcoords \"" << name->get_name() << "\" = "
          << def._texcoords << ", length = " << def._texcoords.size() << "\n";
        describe_attr(out, this, G_PER_VERTEX, def._tindex, false, 
                      indent_level + 2);

      } else {
        indent(out, indent_level)
          << "Nonindexed texcoords \"" << name->get_name() << "\":\n";
        describe_attr(out, this, G_PER_VERTEX, def._texcoords, true, 
                      indent_level + 2);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::setup_multitexcoord_iterator
//       Access: Public
//  Description: Fills in the values on the indicated
//               MultiTexCoordIterator to prepare it to walk through
//               the texture coordinates on this Geom, to issue the
//               appropriate texture coordinates for the currently
//               active stages.
////////////////////////////////////////////////////////////////////
void Geom::
setup_multitexcoord_iterator(MultiTexCoordIterator &iterator,
                             const ActiveTextureStages &active_stages,
                             const NoTexCoordStages &no_texcoords) const {
  check_config();
  iterator._num_stages = 0;
  int max_stage_index = (int)active_stages.size();
  int i = 0;
  for (int stage_index = 0; stage_index < max_stage_index; stage_index++) {
    TextureStage *stage = active_stages[stage_index];
    if (no_texcoords.find(stage) == no_texcoords.end()) {
      // This stage is not one of the stages that doesn't need
      // texcoords issued for it.
      const TexCoordName *name = stage->get_texcoord_name();
      TexCoordsByName::const_iterator tci = _texcoords_by_name.find(name);
      if (tci != _texcoords_by_name.end()) {
        // This Geom does have texcoords for this stage.
        const TexCoordDef &def = (*tci).second;
        
        nassertv(i < max_geom_texture_stages);
        iterator._stages[i]._array = def._texcoords;
        iterator._stages[i]._index = def._tindex;
        iterator._stage_index[i] = stage_index;
        i++;
      }
    }
  }

  iterator._num_stages = i;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::prepare_now
//       Access: Public
//  Description: Creates a context for the geom on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) GeomContext.  This assumes that the
//               GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               geoms.  If this is not necessarily the case, you
//               should use prepare() instead.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian; a geom does not need to be
//               explicitly prepared by the user before it may be
//               rendered.
////////////////////////////////////////////////////////////////////
GeomContext *Geom::
prepare_now(PreparedGraphicsObjects *prepared_objects, 
            GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  GeomContext *gc = prepared_objects->prepare_geom_now(this, gsg);
  if (gc != (GeomContext *)NULL) {
    _contexts[prepared_objects] = gc;

    // Now that we have a new GeomContext with zero dirty flags, our
    // intersection of all dirty flags must be zero.  This doesn't mean
    // that some other contexts aren't still dirty, but at least one
    // context isn't.
    _all_dirty_flags = 0;
  }
  return gc;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::release
//       Access: Public
//  Description: Frees the geom context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool Geom::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    GeomContext *gc = (*ci).second;
    prepared_objects->release_geom(gc);
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_geom(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::release_all
//       Access: Public
//  Description: Frees the context allocated on all objects for which
//               the geom has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int Geom::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_geom(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    GeomContext *gc = (*ci).second;
    prepared_objects->release_geom(gc);
  }

  // Now that we've called release_geom() on every known context,
  // the _contexts list should have completely emptied itself.
  nassertr(_contexts.empty(), num_freed);

  return num_freed;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::draw
//       Access: Public, Virtual
//  Description: Actually draws the Geom with the indicated GSG.
////////////////////////////////////////////////////////////////////
void Geom::
draw(GraphicsStateGuardianBase *gsg) const {
  PreparedGraphicsObjects *prepared_objects = gsg->get_prepared_objects();
  if (is_dirty()) {
    ((Geom *)this)->config(); 
    ((Geom *)this)->release(prepared_objects);
  }

  if (retained_mode) {
    GeomContext *gc = ((Geom *)this)->prepare_now(gsg->get_prepared_objects(), gsg);
    ((Geom *)this)->draw_immediate(gsg, gc);
  } else {
    ((Geom *)this)->draw_immediate(gsg, NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::config
//       Access: Public, Virtual
//  Description: Configure rendering based on current settings
////////////////////////////////////////////////////////////////////
void Geom::
config() {
  WritableConfigurable::config();

  // Only per vertex binding makes any sense
  if (_coords != (Vertexf*)0L && _bind[G_COORD] != G_OFF) {
    _get_vertex =
      (_vindex == (ushort*)0L) ? get_vertex_nonindexed : get_vertex_indexed;
  } else {
    gobj_cat.error()
      << "Geom::Config() - no vertex array!" << endl;
  }

  // Set up normal rendering configuration
  if (_norms != (Normalf*)0L && _bind[G_NORMAL] != G_OFF) {
    _get_normal =
      (_nindex == (ushort*)0L) ? get_normal_nonindexed : get_normal_indexed;
  } else {
    _get_normal = get_normal_noop;
  }

  // Set up texture coordinate rendering configuration
  if (_texcoords_by_name.empty()) {
    _get_texcoord = get_texcoord_noop;

  } else {
    _get_texcoord = get_texcoord_indexed;

    // If any of the texture coordinates are nonindexed, all of them
    // must be.
    TexCoordsByName::const_iterator tci;
    for (tci = _texcoords_by_name.begin(); 
         tci != _texcoords_by_name.end();
         ++tci) {
      if ((*tci).second._tindex == (ushort *)NULL) {
        _get_texcoord = get_texcoord_nonindexed;
        break;
      }
    }
  }

  // Set up color rendering configuration
  if (_colors != (Colorf*)0L && _bind[G_COLOR] != G_OFF) {
    _get_color =
      (_cindex == (ushort*)0L) ? get_color_nonindexed : get_color_indexed;
  } else {
    _get_color = get_color_noop;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::calc_tight_bounds
//       Access: Public
//  Description: Expands min_point and max_point to include all of the
//               vertices in the Geom, if any.  found_any is set true
//               if any points are found.  It is the caller's
//               responsibility to initialize min_point, max_point,
//               and found_any before calling this function.
////////////////////////////////////////////////////////////////////
void Geom::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point, 
                  bool &found_any) const {
  Geom::VertexIterator vi = make_vertex_iterator();
  int num_prims = get_num_prims();
    
  for (int p = 0; p < num_prims; p++) {
    int length = get_length(p);
    for (int v = 0; v < length; v++) {
      Vertexf vertex = get_next_vertex(vi);
      
      if (found_any) {
        min_point.set(min(min_point[0], vertex[0]),
                      min(min_point[1], vertex[1]),
                      min(min_point[2], vertex[2]));
        max_point.set(max(max_point[0], vertex[0]),
                      max(max_point[1], vertex[1]),
                      max(max_point[2], vertex[2]));
      } else {
        min_point = vertex;
        max_point = vertex;
        found_any = true;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::init
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
init() {
  int i;

  _coords.clear();
  _norms.clear();
  _colors.clear();
  _vindex.clear();
  _nindex.clear();
  _cindex.clear();
  _texcoords_by_name.clear();
  _primlengths.clear();

  for ( i = 0; i < num_GeomAttrTypes; i++ )
    _bind[i] = G_OFF;

  _get_vertex = get_vertex_noop;
  _get_normal = get_normal_noop;
  _get_texcoord = get_texcoord_noop;
  _get_color = get_color_noop;

  WritableConfigurable::config();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this Geom.
//               This includes all of the vertices.
////////////////////////////////////////////////////////////////////
BoundingVolume *Geom::
recompute_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = BoundedObject::recompute_bound();
  nassertr(bound != (BoundingVolume*)0L, bound);

  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now actually compute the bounding volume by putting it around all
  // of our vertices.
  pvector<LPoint3f> vertices;
  VertexIterator vi = make_vertex_iterator();

  for (int p = 0; p < get_num_prims(); p++) {
    for (int v = 0; v < get_length(p); v++) {
      vertices.push_back(get_next_vertex(vi));
    }
  }

  const LPoint3f *vertices_begin = &vertices[0];
  const LPoint3f *vertices_end = vertices_begin + vertices.size();

  gbv->around(vertices_begin, vertices_end);

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the Geom's table, without actually releasing
//               the geom.  This is intended to be called only from
//               PreparedGraphicsObjects::release_geom(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void Geom::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a
    // prepared_objects which the geom didn't know about.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::sum_lengths
//       Access: Private, Static
//  Description: Returns the total number of vertices named in the
//               lengths array.
////////////////////////////////////////////////////////////////////
int Geom::
sum_lengths(const PTA_int &lengths) {
  int num_vertices = 0;

  for (PTA_int::const_iterator li = lengths.begin();
       li != lengths.end();
       ++li) {
    num_vertices += (*li);
  }

  return num_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Geom::
write_datagram(BamWriter *manager, Datagram &me) {
  int i;

  // Coordinates
  WRITE_PTA(manager, me, IPD_Vertexf::write_datagram, _coords);
  // Normals
  WRITE_PTA(manager, me, IPD_Normalf::write_datagram, _norms);
  // Colors
  WRITE_PTA(manager, me, IPD_Colorf::write_datagram, _colors);
  /*
  // pre bam 4.11
  // Texture Coordinates
  WRITE_PTA(manager, me, IPD_TexCoordf::write_datagram, get_texcoords_array());
  */
  // write the multitexture data
  nassertv(_texcoords_by_name.size() < 256);
  me.add_uint8(_texcoords_by_name.size());  // write the size of the map
  // write the TexCoordsByName pointers if any
  TexCoordsByName::const_iterator tci;
  for (tci = _texcoords_by_name.begin(); tci != _texcoords_by_name.end(); ++tci) {
    CPT(TexCoordName) tc = (*tci).first;
    manager->write_pointer(me, tc);
    WRITE_PTA(manager, me, IPD_TexCoordf::write_datagram, (*tci).second._texcoords);
    WRITE_PTA(manager, me, IPD_ushort::write_datagram, (*tci).second._tindex);
  }

  // Now write out the indices for each array
  WRITE_PTA(manager, me, IPD_ushort::write_datagram, _vindex);
  WRITE_PTA(manager, me, IPD_ushort::write_datagram, _nindex);
  WRITE_PTA(manager, me, IPD_ushort::write_datagram, _cindex);
  /*
  // pre bam 4.11
  WRITE_PTA(manager, me, IPD_ushort::write_datagram, get_texcoords_index());
  */
  // the texcoord indices are already written with the texcoords

  me.add_uint16(_numprims);
  WRITE_PTA(manager, me, IPD_int::write_datagram, _primlengths);

  // Write out the bindings for vertices, normals, colors and texture
  // coordinates
  for(i = 0; i < num_GeomAttrTypes; i++) {
    me.add_uint8(_bind[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int Geom::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = dDrawable::complete_pointers(p_list, manager);

  // complete the pointers and build the _texcoords_by_name
  TexCoordDefSet::iterator ci;
  for (ci = _temp_texcoord_set.begin();
       ci != _temp_texcoord_set.end();
       ++ci) {
    CPT(TexCoordName) tc = DCAST(TexCoordName, p_list[pi++]);
    TexCoordDef *def = (*ci);
    _texcoords_by_name[tc] = *def;
    /*
    cerr << "TexCoordName from Geom " << (void *)this
         << " complete pointers " << tc << " " << *tc 
         << " = " << def->_texcoords << " and " << def->_tindex << "\n";
    */
    delete def;
    if (tc == TexCoordName::get_default()) {
      _bind[G_TEXCOORD] = G_PER_VERTEX;
    }
  }
  _temp_texcoord_set.clear();

  make_dirty();

  return pi;
}


////////////////////////////////////////////////////////////////////
//     Function: Geom::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void Geom::
fillin(DatagramIterator& scan, BamReader* manager) {
  int i;

  // Coordinates
  READ_PTA(manager, scan, IPD_Vertexf::read_datagram, _coords);
  // Normals
  READ_PTA(manager, scan, IPD_Normalf::read_datagram, _norms);
  // Colors
  READ_PTA(manager, scan, IPD_Colorf::read_datagram, _colors);
  // Texture Coordinates
  PTA_TexCoordf texcoords;
  if (manager->get_file_minor_ver() < 11) {
    READ_PTA(manager, scan, IPD_TexCoordf::read_datagram, texcoords);
  } else {
    // read the multitexture data
    int num_texcoords_by_names = scan.get_uint8();
    // read the TexCoordsByName pointers of num_texcoords_by_names
    _temp_texcoord_set.reserve(num_texcoords_by_names);
    for (int i=0; i<num_texcoords_by_names; ++i) {
      manager->read_pointer(scan);
      TexCoordDef *tcd = new TexCoordDef;
      READ_PTA(manager, scan, IPD_TexCoordf::read_datagram, tcd->_texcoords);
      READ_PTA(manager, scan, IPD_ushort::read_datagram, tcd->_tindex);
      _temp_texcoord_set.push_back(tcd);
    }
  }
      
  // Now read in the indices for each array
  READ_PTA(manager, scan, IPD_ushort::read_datagram, _vindex);
  READ_PTA(manager, scan, IPD_ushort::read_datagram, _nindex);
  READ_PTA(manager, scan, IPD_ushort::read_datagram, _cindex);
  PTA_ushort tindex;
  if (manager->get_file_minor_ver() < 11) {
    READ_PTA(manager, scan, IPD_ushort::read_datagram, tindex);
  }

  _numprims = scan.get_uint16();

  // is there any point in doing this for uses_components()==false?
  READ_PTA(manager, scan, IPD_int::read_datagram, _primlengths);

  if (uses_components()) {
    _num_vertices = sum_lengths(_primlengths);

  } else {
    // except for strips & fans with the length arrays, total verts
    // will be simply this
    _num_vertices = _numprims * get_num_vertices_per_prim();
  }

  // Read in the bindings for vertices, normals, colors and texture
  // coordinates
  for(i = 0; i < num_GeomAttrTypes; i++) {
    _bind[i] = (enum GeomBindType) scan.get_uint8();
  }

  if (manager->get_file_minor_ver() < 11) {
    if (_bind[G_TEXCOORD] == G_PER_VERTEX) {
      set_texcoords(texcoords, G_PER_VERTEX, tindex);
    }
  }
}

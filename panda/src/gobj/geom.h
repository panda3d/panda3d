// Filename: geom.h
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
#ifndef GEOM_H
#define GEOM_H

#include "pandabase.h"

#include "drawable.h"

#include "vector_typedWritable.h"
#include "ordered_vector.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "typedef.h"
#include "luse.h"
#include "pta_Vertexf.h"
#include "pta_Normalf.h"
#include "pta_Colorf.h"
#include "pta_TexCoordf.h"
#include "pta_ushort.h"
#include "pta_int.h"
#include "internalName.h"
#include "textureStage.h"
#include "pset.h"

class Datagram;
class DatagramIterator;
class BamReader;
class BamWriter;
class GeomContext;
class qpGeomVertexData;
class PreparedGraphicsObjects;

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
BEGIN_PUBLISH
enum GeomBindType
{
    G_OFF,
    G_OVERALL,
    G_PER_PRIM,
    G_PER_COMPONENT,
    G_PER_VERTEX
};
END_PUBLISH
static const int num_GeomBindTypes = 5;

enum GeomAttrType
{
    G_COORD,
    G_COLOR,
    G_NORMAL,
    G_TEXCOORD
};
static const int num_GeomAttrTypes = 4;

ostream &operator << (ostream &out, GeomBindType t);
ostream &operator << (ostream &out, GeomAttrType t);

// This is a totally arbitrary limit and may be increased almost
// without penalty.  It is just used to control the static size of the
// array stored in the MultiTexcoordIterator, below.
static const int max_geom_texture_stages = 32;

////////////////////////////////////////////////////////////////////
//       Class : Geom
// Description : Geometry parent class
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Geom : public dDrawable {
public:

  // These classes are used to iterate through all the vertices,
  // normals, etc.  They're returned by make_vertex_iterator(), and
  // operated on by get_next_vertex(), etc.
  class VertexIterator {
  public:
    const Vertexf *_array;
    const ushort *_index;
  };
  class NormalIterator {
  public:
    const Normalf *_array;
    const ushort *_index;
  };
  class TexCoordIterator {
  public:
    const TexCoordf *_array;
    const ushort *_index;
  };
  class MultiTexCoordIterator {
  public:
    int _num_stages;
    TexCoordIterator _stages[max_geom_texture_stages];
    int _stage_index[max_geom_texture_stages];
  };
  class ColorIterator {
  public:
    const Colorf *_array;
    const ushort *_index;
  };

  // Declare some function types.  This declares several typenames
  // which are pointers to function types--these are not themselves
  // functions.  A function pointed to by a variable of this type,
  // when given an iterator of the appropriate type from the Geom,
  // will retrieve the next element from the array and increment the
  // iterator appropriately for next time.
  typedef const Vertexf &GetNextVertex(VertexIterator &);
  typedef const Normalf &GetNextNormal(NormalIterator &);
  typedef const TexCoordf &GetNextTexCoord(TexCoordIterator &);
  typedef const Colorf &GetNextColor(ColorIterator &);


  Geom();
  Geom(const Geom &copy);
  virtual ~Geom();

  void operator = (const Geom &copy);
  virtual Geom *make_copy() const=0;

PUBLISHED:
  virtual void transform_vertices(const LMatrix4f &mat);
  virtual bool check_valid() const;

  void set_coords(const PTA_Vertexf &coords,
                  const PTA_ushort &vindex = PTA_ushort());
  void set_coords(const PTA_Vertexf &coords, GeomBindType bind,
                  const PTA_ushort &vindex = PTA_ushort());
  void set_normals(const PTA_Normalf &norms, GeomBindType bind,
                   const PTA_ushort &nindex = PTA_ushort());
  void set_colors(const PTA_Colorf &colors, GeomBindType bind,
                  const PTA_ushort &cindex = PTA_ushort());
  void set_texcoords(const PTA_TexCoordf &texcoords, GeomBindType bind,
                     const PTA_ushort &tindex = PTA_ushort());
  void set_texcoords(const InternalName *name, const PTA_TexCoordf &texcoords,
                     const PTA_ushort &tindex = PTA_ushort());
  void remove_texcoords(const InternalName *name);

public:
  // These can't be published because of the pass-by-reference
  // primitive types.
  void get_coords(PTA_Vertexf &coords, GeomBindType &bind,
                  PTA_ushort &vindex) const;

  void get_coords(PTA_Vertexf &coords, PTA_ushort &vindex) const;

  void get_normals(PTA_Normalf &norms, GeomBindType &bind,
                   PTA_ushort &nindex) const;
  void get_colors(PTA_Colorf &colors,
                  GeomBindType &bind, PTA_ushort &cindex) const;
  void get_texcoords(PTA_TexCoordf &texcoords, GeomBindType &bind,
                     PTA_ushort &tindex) const;


PUBLISHED:
  virtual bool is_dynamic() const;

  INLINE GeomBindType get_binding(int attr) const;
  INLINE bool has_any_texcoords() const;
  INLINE bool has_texcoords(const InternalName *name) const;

  INLINE PTA_Vertexf get_coords_array() const;
  INLINE PTA_Normalf get_normals_array() const;
  INLINE PTA_Colorf get_colors_array() const;
  INLINE PTA_TexCoordf get_texcoords_array() const;
  INLINE PTA_TexCoordf get_texcoords_array(const InternalName *name) const;

  INLINE PTA_ushort get_coords_index() const;
  INLINE PTA_ushort get_normals_index() const;
  INLINE PTA_ushort get_colors_index() const;
  INLINE PTA_ushort get_texcoords_index() const;
  INLINE PTA_ushort get_texcoords_index(const InternalName *name) const;
  INLINE bool are_texcoords_indexed() const;

  void prepare(PreparedGraphicsObjects *prepared_objects);

  INLINE void set_num_prims(int num);
  INLINE int get_num_prims() const;

  INLINE void set_lengths(const PTA_int &lengths);
  INLINE PTA_int get_lengths() const;

  virtual int get_num_vertices_per_prim() const;
  virtual int get_num_more_vertices_than_components() const;
  virtual bool uses_components() const;

  INLINE int get_num_vertices() const;

  // Returns the length of the indicated primitive.  Often this is the
  // same for all primitives in the Geom.  However, geoms which use
  // the lengths array will redefine this appropriately.
  virtual int get_length(int prim) const;

  virtual Geom *explode() const;
  virtual PTA_ushort get_tris() const;

  void write(ostream &out, int indent_level = 0) const;
  virtual void output(ostream &out) const;
  void write_verbose(ostream &out, int indent_level) const;

public:
  typedef pvector< PT(TextureStage) > ActiveTextureStages;
  typedef pset<TextureStage *> NoTexCoordStages;

  INLINE VertexIterator make_vertex_iterator() const;
  INLINE const Vertexf &get_next_vertex(VertexIterator &viterator) const;

  INLINE NormalIterator make_normal_iterator() const;
  INLINE const Normalf &get_next_normal(NormalIterator &niterator) const;

  INLINE TexCoordIterator make_texcoord_iterator() const;
  INLINE TexCoordIterator make_texcoord_iterator(const InternalName *texcoord_name) const;
  INLINE const TexCoordf &get_next_texcoord(TexCoordIterator &tciterator) const;
  void setup_multitexcoord_iterator(MultiTexCoordIterator &iterator,
                                    const ActiveTextureStages &active_stages,
                                    const NoTexCoordStages &no_texcoords) const;
  INLINE const TexCoordf &get_next_multitexcoord(MultiTexCoordIterator &tciterator, int n) const;

  INLINE ColorIterator make_color_iterator() const;
  INLINE const Colorf &get_next_color(ColorIterator &citerator) const;

  GeomContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                           GraphicsStateGuardianBase *gsg);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

  // From parent dDrawable
  virtual void draw(GraphicsStateGuardianBase *gsg, 
                    const qpGeomMunger *munger,
                    const qpGeomVertexData *vertex_data) const;

  // From parent Configurable
  virtual void config();

  // Immediate mode drawing functions - issue graphics commands
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc);
  virtual void print_draw_immediate() const;

  void calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                         bool &found_any) const;

protected:
  void init();
  virtual BoundingVolume *recompute_bound();

protected:

  PTA_Vertexf _coords;
  PTA_Normalf _norms;
  PTA_Colorf _colors;

  PTA_ushort _vindex;
  PTA_ushort _nindex;
  PTA_ushort _cindex;

  int _numprims,_num_vertices;
  PTA_int _primlengths;
  enum GeomBindType _bind[num_GeomAttrTypes];

  class TexCoordDef {
  public:
    PTA_TexCoordf _texcoords;
    PTA_ushort _tindex;
  };

  // A pmap, not a phash_map, to save space because it will probably
  // have only one element.
  typedef pmap<CPT(InternalName), TexCoordDef> TexCoordsByName;
  TexCoordsByName _texcoords_by_name;

  // Functions to extract component values, one at a time.
  GetNextVertex *_get_vertex;
  GetNextNormal *_get_normal;
  GetNextColor *_get_color;
  GetNextTexCoord *_get_texcoord;

  // temporary storage until complete_pointers fills in _texcoords_by_name's InternalName *
  typedef pvector<TexCoordDef *> TexCoordDefSet;
  TexCoordDefSet _temp_texcoord_set;


private:
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);
  static int sum_lengths(const PTA_int &lengths);

  // A Geom keeps a list (actually, a map) of all the
  // PreparedGraphicsObjects tables that it has been prepared into.
  // Each PGO conversely keeps a list (a set) of all the Geoms that
  // have been prepared there.  When either destructs, it removes
  // itself from the other's list.
  typedef pmap<PreparedGraphicsObjects *, GeomContext *> Contexts;
  Contexts _contexts;

  // This value represents the intersection of all the dirty flags of
  // the various GeomContexts that might be associated with this
  // geom.
  int _all_dirty_flags;

public:
  //static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

  //static TypedWritable *make_Generic(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    dDrawable::init_type();
    register_type(_type_handle, "Geom",
                  dDrawable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GeomContext;
  friend class PreparedGraphicsObjects;
};

INLINE ostream &operator <<(ostream &out, const Geom &geom) {
  geom.output(out);
  return out;
}

#include "geom.I"

#endif

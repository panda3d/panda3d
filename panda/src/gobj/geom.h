// Filename: geom.h
// Created by:  mike (09Jan97)
//
//
#ifndef GEOM_H
#define GEOM_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "drawable.h"

#include <vector_typedWriteable.h>
#include <pointerTo.h>
#include <pointerToArray.h>
#include <typedef.h>
#include <luse.h>
#include <pta_Vertexf.h>
#include <pta_Normalf.h>
#include <pta_Colorf.h>
#include <pta_TexCoordf.h>
#include <pta_ushort.h>
#include <pta_int.h>
#include "texture.h"

class Datagram;
class DatagramIterator;
class BamReader;
class BamWriter;

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
enum GeomBindType
{
    G_OFF,
    G_OVERALL,
    G_PER_PRIM,
    G_PER_COMPONENT,
    G_PER_VERTEX
};
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

	
  Geom( void );
  Geom( const Geom& copy );
  ~Geom( void );
 
  void operator = ( const Geom &copy );
  virtual Geom *make_copy() const=0;

PUBLISHED: 
  void write(ostream &out, int indent_level = 0) const;
  virtual void output(ostream &out) const;
  void write_verbose(ostream &out, int indent_level) const;

public: 
  // From parent dDrawable
  virtual void draw(GraphicsStateGuardianBase *gsg) {
    dDrawable::draw(gsg); draw_immediate(gsg);
  }
 
  // From parent Configurable
  virtual void config( void );
 
  // Immediate mode drawing functions - issue graphics commands
  virtual void draw_immediate(GraphicsStateGuardianBase *) const = 0;
  virtual void print_draw_immediate( void ) const = 0;
 
public:
 
  void get_min_max( Vertexf& min, Vertexf& max ) const;
 
  GeomBindType get_binding( int attr ) const {
    return _bind[attr];
  }
 
  void set_coords( const PTA_Vertexf &coords,
		   GeomBindType bind,
		   const PTA_ushort &vindex =
		   PTA_ushort() );
  void set_normals( const PTA_Normalf &norms,
		    GeomBindType bind,
		    const PTA_ushort &nindex =
		    PTA_ushort() );
  void set_colors( const PTA_Colorf &colors,
		   GeomBindType bind,
		   const PTA_ushort &cindex =
		   PTA_ushort() );
  void set_texcoords( const PTA_TexCoordf &texcoords,
		      GeomBindType bind,
		      const PTA_ushort &tindex =
		      PTA_ushort() );
 
  void get_coords( PTA_Vertexf &coords,
		   GeomBindType &bind,
		   PTA_ushort &vindex) const;
  void get_normals( PTA_Normalf &norms,
		    GeomBindType &bind,
		    PTA_ushort &nindex) const;
  void get_colors( PTA_Colorf &colors,
		   GeomBindType &bind,
		   PTA_ushort &cindex) const;
  void get_texcoords( PTA_TexCoordf &texcoords,
		      GeomBindType &bind,
		      PTA_ushort &tindex) const;
 
  INLINE void set_num_prims(int num) { _numprims = num; make_dirty(); }
  INLINE int get_num_prims(void) const { return _numprims; }
 
  void set_lengths( const PTA_int &lengths );
  PTA_int get_lengths() const;
 
  virtual int get_num_vertices_per_prim() const=0;
  virtual int get_num_more_vertices_than_components() const=0;
  virtual bool uses_components() const=0;

  // Returns the length of the indicated primitive.  Often this is the
  // same for all primitives in the Geom.  However, geoms which use
  // the lengths array will redefine this appropriately.
  virtual int get_length(int prim) const=0;


  virtual Geom *explode() const;
  virtual PTA_ushort get_tris() const;


  INLINE VertexIterator make_vertex_iterator() const;
  INLINE const Vertexf &get_next_vertex(VertexIterator &viterator) const;

  INLINE NormalIterator make_normal_iterator() const;
  INLINE const Normalf &get_next_normal(NormalIterator &niterator) const;

  INLINE TexCoordIterator make_texcoord_iterator() const;
  INLINE const TexCoordf &get_next_texcoord(TexCoordIterator &tciterator) const;

  INLINE ColorIterator make_color_iterator() const;
  INLINE const Colorf &get_next_color(ColorIterator &citerator) const;

protected:

  void init( void );
  virtual void recompute_bound();
 
protected:
 
  PTA_Vertexf _coords;
  PTA_Normalf _norms;
  PTA_Colorf _colors;
  PTA_TexCoordf _texcoords;
 
  PTA_ushort _vindex;
  PTA_ushort _nindex;
  PTA_ushort _cindex;
  PTA_ushort _tindex;
 
  int _numprims;
  PTA_int _primlengths;
  enum GeomBindType _bind[num_GeomAttrTypes];
 
  // Functions to extract component values, one at a time.
  GetNextVertex *_get_vertex;
  GetNextNormal *_get_normal;
  GetNextTexCoord *_get_texcoord;
  GetNextColor *_get_color;


public:
  //static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  //static TypedWriteable *make_Generic(const FactoryParams &params);

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

};

INLINE ostream &operator <<(ostream &out, const Geom &geom) {
  geom.output(out);
  return out;
}

#include "geom.I"

#endif

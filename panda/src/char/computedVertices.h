// Filename: computedVertices.h
// Created by:  drose (01Mar99)
//
////////////////////////////////////////////////////////////////////

#ifndef COMPUTEDVERTICES_H
#define COMPUTEDVERTICES_H

#include <pandabase.h>

#include "dynamicVertices.h"
#include "computedVerticesMorph.h"

#include <vector_ushort.h>
#include <typedWritableReferenceCount.h>

class Character;
class CharacterJoint;

////////////////////////////////////////////////////////////////////
//       Class : ComputedVertices
// Description : These are the vertices that must be animated as part
//               of the normal character animation.  This includes
//               vertices animated into one or more joints, as well as
//               morph vertices.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ComputedVertices : public TypedWritableReferenceCount {
public:
  INLINE ComputedVertices();

  void update(Character *character);
  void make_orig(Character *character);

  void write(ostream &out, Character *character) const;

private:
  typedef vector_ushort Vertices;

  class EXPCL_PANDA VertexTransform {
  public:
    INLINE VertexTransform();
    VertexTransform(const VertexTransform &copy);

    short _joint_index;
    float _effect;
    Vertices _vindex;
    Vertices _nindex;
    INLINE bool operator < (const VertexTransform &other) const;
  public:
    void write_datagram(Datagram &dest);  
    void read_datagram(DatagramIterator &source);  
  };

  typedef vector<VertexTransform> VertexTransforms;

  VertexTransforms _transforms;

  typedef vector<ComputedVerticesMorphVertex> VertexMorphs;
  typedef vector<ComputedVerticesMorphNormal> NormalMorphs;
  typedef vector<ComputedVerticesMorphTexCoord> TexCoordMorphs;
  typedef vector<ComputedVerticesMorphColor> ColorMorphs;
  VertexMorphs _vertex_morphs;
  NormalMorphs _normal_morphs;
  TexCoordMorphs _texcoord_morphs;
  ColorMorphs _color_morphs;

  PTA_Vertexf _orig_coords;
  PTA_Normalf _orig_norms;
  PTA_Colorf _orig_colors;
  PTA_TexCoordf _orig_texcoords;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWritable *make_ComputedVertices(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ComputedVertices",
		  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ComputedVerticesMaker;
};

#include "computedVertices.I"

#endif

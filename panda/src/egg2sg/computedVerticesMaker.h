// Filename: computedVerticesMaker.h
// Created by:  drose (01Mar99)
//
////////////////////////////////////////////////////////////////////

#ifndef COMPUTEDVERTICESMAKER_H
#define COMPUTEDVERTICESMAKER_H

#include <pandabase.h>

#include "computedVerticesMakerEntity.h"

#include <computedVerticesMorph.h>
#include <pointerToArray.h>
#include <luse.h>
#include <typedef.h>
#include <eggMorphList.h>
#include <pta_Vertexf.h>
#include <pta_Normalf.h>
#include <pta_Colorf.h>
#include <pta_TexCoordf.h>

#include <set>
#include <map>

class ComputedVertices;
class CharacterMaker;
class EggNode;

///////////////////////////////////////////////////////////////////
// 	 Class : ComputedVerticesMaker
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG ComputedVerticesMaker {
public:
  ComputedVerticesMaker();
 
  void begin_new_space();
  void add_joint(EggNode *joint, double membership);
  void mark_space();

  int add_vertex(const Vertexd &vertex, const EggMorphVertexList &morphs,
		 const LMatrix4d &transform);
  int add_normal(const Normald &normal, const EggMorphNormalList &morphs,
		 const LMatrix4d &transform);
  int add_texcoord(const TexCoordd &texcoord,
		   const EggMorphTexCoordList &morphs,
		   const LMatrix3d &transform);
  int add_color(const Colorf &color, const EggMorphColorList &morphs);

  ComputedVertices *make_computed_vertices(Character *character,
					   CharacterMaker &char_maker);

  void write(ostream &out) const;

public:
  PTA_Vertexf _coords;
  PTA_Normalf _norms;
  PTA_Colorf _colors;
  PTA_TexCoordf _texcoords;

protected:
  typedef map<int, LVector3f> VertexMorphList;
  typedef map<int, LVector3f> NormalMorphList;
  typedef map<int, LVector2f> TexCoordMorphList;
  typedef map<int, LVector4f> ColorMorphList;
  class MorphList {
  public:
    VertexMorphList _vmorphs;
    NormalMorphList _nmorphs;
    TexCoordMorphList _tmorphs;
    ColorMorphList _cmorphs;
  };

  typedef map<string, MorphList> Morphs;
  Morphs _morphs;

  typedef set<int> Vertices;

  Vertices _cindex;
  Vertices _tindex;

  ComputedVerticesMakerTexCoordMap _tmap;
  ComputedVerticesMakerColorMap _cmap;

#ifdef WIN32_VC
public:
#endif

  class JointWeights: public map<EggNode *, double> {
  public:
    bool operator < (const JointWeights &other) const;
    void normalize_weights();

    void output(ostream &out) const;
  };

protected:
  class VertexCollection {
  public:
    Vertices _vindex;
    Vertices _nindex;

    ComputedVerticesMakerVertexMap _vmap;
    ComputedVerticesMakerNormalMap _nmap;
  };

  typedef map<JointWeights, VertexCollection> TransformSpaces;
  TransformSpaces _transforms;

  class VertexTransform {
  public:
    EggNode *_joint;
    float _effect;
  };

  JointWeights _current_jw;
  VertexCollection *_current_vc;

  friend inline ostream &operator << (ostream &, const JointWeights &);
};

inline ostream &
operator << (ostream &out, const ComputedVerticesMaker::JointWeights &jw) {
  jw.output(out);
  return out;
}

#include "computedVerticesMaker.I"

#endif
 

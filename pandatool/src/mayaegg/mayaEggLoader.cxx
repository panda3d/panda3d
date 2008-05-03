// Filename: mayaEggImport.cxx
// Created by:  jyelon (20Jul05)
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
//
// This file contains the code for class MayaEggLoader.  This class
// does the actual work of copying an EggData tree into the maya scene.
//
////////////////////////////////////////////////////////////////////


#include "pandatoolbase.h"
#include "notifyCategoryProxy.h"

#include "eggData.h"
#include "eggTable.h"
#include "eggComment.h"
#include "eggXfmSAnim.h"
#include "eggSAnimData.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "eggPrimitive.h"
#include "eggGroupNode.h"
#include "eggPolysetMaker.h"
#include "eggBin.h"

#include "pre_maya_include.h"
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnLambertShader.h>
#include <maya/MPlug.h>
#include <maya/MFnSet.h>
#include <maya/MDGModifier.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MPlugArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MAnimControl.h>
#include <maya/MFnAnimCurve.h>
#include "post_maya_include.h"

#include "mayaEggLoader.h"

class MayaEggGroup;
class MayaEggMesh;
class MayaEggJoint;
class MayaEggTex;
class MayaAnim;

NotifyCategoryDeclNoExport(mayaloader);
NotifyCategoryDef(mayaloader, "");

class MayaEggLoader
{
public:
  bool ConvertEggData(EggData *data,    bool merge, bool model, bool anim, bool respect_normals);
  bool ConvertEggFile(const char *name, bool merge, bool model, bool anim, bool respect_normals);
  
  
public:
  void          TraverseEggNode(EggNode *node, EggGroup *context, string delim);
  MayaEggMesh  *GetMesh(EggVertexPool *pool, EggGroup *parent);
  MayaEggJoint *FindJoint(EggGroup *joint);
  MayaEggJoint *MakeJoint(EggGroup *joint, EggGroup *context);
  MayaEggGroup *FindGroup(EggGroup *group);
  MayaEggGroup *MakeGroup(EggGroup *group, EggGroup *context);
  MayaEggTex   *GetTex(const string &name, const string &fn);
  void          CreateSkinCluster(MayaEggMesh *M);

  MayaAnim *GetAnim(EggXfmSAnim *pool);
  MObject MayaEggLoader::GetDependencyNode(string givenName);

  typedef phash_map<EggVertexPool *, MayaEggMesh *, pointer_hash> MeshTable;
  typedef phash_map<EggXfmSAnim *, MayaAnim *, pointer_hash> AnimTable;
  typedef phash_map<EggGroup *, MayaEggJoint *, pointer_hash> JointTable;
  typedef phash_map<EggGroup *, MayaEggGroup *, pointer_hash> GroupTable;
  typedef phash_map<string, MayaEggTex *, string_hash> TexTable;

  MeshTable        _mesh_tab;
  AnimTable        _anim_tab;
  JointTable       _joint_tab;
  GroupTable       _group_tab;
  TexTable         _tex_tab;

  int _start_frame;
  int _end_frame;
  int _frame_rate;
  MTime::Unit     _timeUnit;

  void ParseFrameInfo(string comment);
};

MFloatPoint MakeMayaPoint(const LVector3d &vec)
{
  return MFloatPoint(vec[0], vec[1], vec[2]);
}

MVector MakeMayaVector(const LVector3d &vec)
{
  return MVector(vec[0], vec[1], vec[2]);
}

MColor MakeMayaColor(const Colorf &vec)
{
  return MColor(vec[0], vec[1], vec[2], vec[3]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MayaEggTex
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MayaEggTex
{
public:
  string  _name;
  string  _path;
  MObject _file_texture;
  MObject _shader;
  MObject _shading_group;
  
  MFnSingleIndexedComponent _component;
  void AssignNames(void);
};

void MayaEggTex::AssignNames(void)
{
  if (_name == "") {
    return;
  }
  MFnDependencyNode shader(_shader);
  MFnDependencyNode sgroup(_shading_group);
  MFnDependencyNode filetex(_file_texture);
  shader.setName(MString(_name.c_str())+"Shader");
  sgroup.setName(MString(_name.c_str()));
  if (_file_texture != MObject::kNullObj) {
    filetex.setName(MString(_name.c_str())+"File");
  }
}

MayaEggTex *MayaEggLoader::GetTex(const string &name, const string &fn)
{
  if (_tex_tab.count(fn)) {
    return _tex_tab[fn];
  }

  MStatus status;
  MFnLambertShader shader;
  MFnDependencyNode filetex;
  MFnSet sgroup;

  if (fn=="") {
    MSelectionList selection;
    MObject initGroup;
    selection.clear();
    MGlobal::getSelectionListByName("initialShadingGroup",selection);
    selection.getDependNode(0, initGroup);
    sgroup.setObject(initGroup);
  } else {
    MPlugArray oldplugs;
    MDGModifier dgmod;
    
    shader.create(true,&status);
    sgroup.create(MSelectionList(), MFnSet::kRenderableOnly, &status);
    MPlug surfplug = sgroup.findPlug("surfaceShader");
    if (surfplug.connectedTo(oldplugs,true,false)) {
      for (unsigned int i=0; i<oldplugs.length(); i++) {
        MPlug src = oldplugs[i];
        status = dgmod.disconnect(src, surfplug);
        if (status != MStatus::kSuccess) {
          status.perror("Disconnecting old shader");
        }
      }
    }
    status = dgmod.connect(shader.findPlug("outColor"),surfplug);
    if (status != MStatus::kSuccess) {
      status.perror("Connecting shader");
    }
    if (fn != "") {
      filetex.create("file",&status);
      MString fn_str(fn.c_str());
      filetex.findPlug("fileTextureName").setValue(fn_str);
      dgmod.connect(filetex.findPlug("outColor"),shader.findPlug("color"));
    }
    status = dgmod.doIt();
    if (status != MStatus::kSuccess) {
      status.perror("DGMod doIt");
    }
  }

  MayaEggTex *res = new MayaEggTex;
  res->_name = name;
  res->_path = fn;
  res->_file_texture = filetex.object();
  res->_shader = shader.object();
  res->_shading_group = sgroup.object();
  
  _tex_tab[fn] = res;
  return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MayaEggGroup
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MayaEggGroup
{
public:
  string  _name;
  MObject _parent;
  MObject _group;
};

MayaEggGroup *MayaEggLoader::MakeGroup(EggGroup *group, EggGroup *context)
{
  MStatus status;
  MayaEggGroup *pg = FindGroup(context);
  MayaEggGroup *result = new MayaEggGroup;
  MFnDagNode dgn;

  MObject parent = MObject::kNullObj;
  if (pg) {
    parent = pg->_group;
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "parent (group) :" << ((MFnDagNode)parent).name() << endl;
    }
  }

  result->_name = group->get_name();
  result->_group = dgn.create("transform", MString(result->_name.c_str()), parent, &status);

  if (status != MStatus::kSuccess) {
    status.perror("MFnDagNode:create failed!");
  }
  _group_tab[group] = result;
  return result;
}

MayaEggGroup *MayaEggLoader::FindGroup(EggGroup *group)
{
  if (group==0) {
    return 0;
  }
  return _group_tab[group];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MayaEggJoint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MayaEggJoint
{
public:
  LMatrix4d      _trans;
  LVector3d      _endpos;
  LVector3d      _perp;
  double         _thickness;
  MObject        _joint;
  MMatrix        _joint_abs;
  MDagPath       _joint_dag_path;
  bool           _inskin;
  int            _index;
  EggGroup       *_egg_joint;
  EggGroup       *_egg_parent;
  MayaEggJoint   *_parent;
  vector <MayaEggJoint *> _children;

public:
  void GetRotation(LVector3d &xv, LVector3d &yv, LVector3d &zv);
  LVector3d GetPos(void) { return _trans.get_row3(3); }
  MayaEggJoint *ChooseBestChild(LVector3d dir);
  void ChooseEndPos(double thickness);
  void CreateMayaBone(MayaEggGroup *eggParent);
  void AssignNames(void);
};

void MayaEggJoint::GetRotation(LVector3d &xv, LVector3d &yv, LVector3d &zv)
{
  xv = _trans.get_row3(0);
  yv = _trans.get_row3(1);
  zv = _trans.get_row3(2);
  xv.normalize();
  yv.normalize();
  zv = xv.cross(yv);
  zv.normalize();
  yv = zv.cross(xv);
}

void MayaEggJoint::AssignNames(void)
{
  string name = _egg_joint->get_name();
  MFnDependencyNode joint(_joint);
  joint.setName(name.c_str());
}

MayaEggJoint *MayaEggLoader::FindJoint(EggGroup *joint)
{
  if (joint==(EggGroup *)NULL) {
    return 0;
  }
  if (!joint->is_joint()) {
    return 0;
  }
  return _joint_tab[joint];
}

MayaEggJoint *MayaEggLoader::MakeJoint(EggGroup *joint, EggGroup *context)
{
  MayaEggJoint *parent = FindJoint(context);
  if (mayaloader_cat.is_debug()) {
    string parent_name = "";
    if (parent)
      parent_name = context->get_name();
  }
  MayaEggJoint *result = new MayaEggJoint;
  LMatrix4d t = joint->get_transform3d();
  if (parent) {
    result->_trans = t * parent->_trans;
  } else {
    result->_trans = t;
  }
  result->_endpos = LVector3d(0,0,0);
  result->_perp = LVector3d(0,0,0);
  result->_thickness = 0.0;
  result->_egg_joint = joint;
  result->_egg_parent = context;
  result->_parent = parent;
  result->_joint = MObject::kNullObj;
  result->_inskin = false;
  result->_index = -1;
  if (parent) {
    parent->_children.push_back(result);
  }
  _joint_tab[joint] = result;

  return result;
}

MayaEggJoint *MayaEggJoint::ChooseBestChild(LVector3d dir)
{
  if (dir.length() < 0.001) {
    return 0;
  }
  dir.normalize();
  double firstbest = -1000;
  MayaEggJoint *firstchild = 0;
  LVector3d firstpos = GetPos();
  double secondbest = 0;
  for (unsigned int i=0; i<_children.size(); i++) {
    MayaEggJoint *child = _children[i];
    LVector3d tryfwd = child->GetPos() - GetPos();
    if ((child->GetPos() != firstpos) && (tryfwd.length() > 0.001)) {
      LVector3d trydir = tryfwd;
      trydir.normalize();
      double quality = trydir.dot(dir);
      if (quality > firstbest) {
        secondbest = firstbest;
        firstbest = quality;
        firstpos = child->GetPos();
        firstchild = child;
      } else if (quality > secondbest) {
        secondbest = quality;
      }
    }
  }
  if (firstbest > secondbest + 0.1) {
    return firstchild;
  }
  return 0;
}

void MayaEggJoint::ChooseEndPos(double thickness)
{
  LVector3d parentpos(0,0,0);
  LVector3d parentendpos(0,0,1);
  if (_parent) {
    parentpos = _parent->GetPos();
    parentendpos = _parent->_endpos;
  }
  LVector3d fwd = GetPos() - parentpos;
  if (fwd.length() < 0.001) {
    fwd = parentendpos - parentpos;
  }
  //mayaloader_cat.debug() << "fwd : " << fwd << endl;
  fwd.normalize();
  MayaEggJoint *child = ChooseBestChild(fwd);
  if (child == 0) {
    _endpos = fwd * thickness * 0.8 + GetPos();
    _thickness = thickness * 0.8;
  } else {
    _endpos = child->GetPos();
    _thickness = (_endpos - GetPos()).length();
    if (_thickness > thickness) _thickness = thickness;
  }
  LVector3d orient = _endpos - GetPos();
  orient.normalize();
  LVector3d altaxis = orient.cross(LVector3d(0,-1,0));
  if (altaxis.length() < 0.001) {
    altaxis = orient.cross(LVector3d(0,0,1));
  }
  _perp = altaxis.cross(orient);
  _perp.normalize();
}

void MayaEggJoint::CreateMayaBone(MayaEggGroup *eggParent)
{
  LVector3d rxv, ryv, rzv;
  GetRotation(rxv, ryv, rzv);
  MFloatPoint xv(MakeMayaPoint(rxv));
  MFloatPoint yv(MakeMayaPoint(ryv));
  MFloatPoint zv(MakeMayaPoint(rzv));
  MFloatPoint pos(MakeMayaPoint(GetPos()));
  MFloatPoint endpos(MakeMayaPoint(_endpos));
  MFloatPoint tzv(MakeMayaPoint(_perp));
  
  double m[4][4];
  m[0][0]=xv.x;  m[0][1]=xv.y;  m[0][2]=xv.z;  m[0][3]=0;
  m[1][0]=yv.x;  m[1][1]=yv.y;  m[1][2]=yv.z;  m[1][3]=0;
  m[2][0]=zv.x;  m[2][1]=zv.y;  m[2][2]=zv.z;  m[2][3]=0;
  m[3][0]=pos.x; m[3][1]=pos.y; m[3][2]=pos.z; m[3][3]=1;
  MMatrix trans(m);
  _joint_abs = trans;
  if (_parent) {
    trans = trans * _parent->_joint_abs.inverse();
  }
  MTransformationMatrix mtm(trans);

  MFnIkJoint ikj;
  if (_parent) {
    ikj.create(_parent->_joint);
  }
  else {
    if (eggParent) {
      // must be part of a group that is not a joint
      ikj.create(eggParent->_group);
    } else {
      ikj.create();
    }
  }
  ikj.set(mtm);
  
  _joint = ikj.object();
  ikj.getPath(_joint_dag_path);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MayaEggMesh
//
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef pair<double, EggGroup *> MayaEggWeight;

struct MayaEggVertex
{
  Vertexd               _pos;
  Normald               _normal;
  vector<MayaEggWeight> _weights;
  int                   _index;
};

struct MEV_Compare: public stl_hash_compare<MayaEggVertex>
{
  size_t operator()(const MayaEggVertex &key) const
  {
    return key._pos.add_hash(key._normal.get_hash());
  }
  bool operator()(const MayaEggVertex &k1, const MayaEggVertex &k2) const
  {
    int n = k1._pos.compare_to(k2._pos);
    if (n < 0) {
      return true;
    }
    if (n > 0) {
      return false;
    }
    n = k1._normal.compare_to(k2._normal);
    if (n < 0) {
      return true;
    }
    if (n > 0) {
      return false;
    }
    n = k1._weights.size() - k2._weights.size();
    if (n < 0) {
      return true;
    }
    if (n > 0) {
      return false;
    }
    for (unsigned int i=0; i<k1._weights.size(); i++) {
      double d = k1._weights[i].first - k2._weights[i].first;
      if (d < 0) {
        return true;
      }
      if (d > 0) {
        return false;
      }
      EggGroup *g1 = k1._weights[i].second;
      EggGroup *g2 = k2._weights[i].second;
      if (g1 < g2) {
        return true;
      }
      if (g1 > g2) {
        return false;
      }
    }
    return false;
  }
};

typedef phash_set<MayaEggVertex, MEV_Compare> VertTable;
typedef phash_map<TexCoordd, int>             TVertTable;
typedef phash_map<Colorf, int>                CVertTable;

class MayaEggMesh
{
public:
  
  EggVertexPool      *_pool;
  MFloatPointArray    _vertexArray;
  MVectorArray        _normalArray;
  MColorArray         _vertColorArray;
  MIntArray           _vertColorIndices;
  MIntArray           _vertNormalIndices;
  MColorArray         _faceColorArray;
  MIntArray           _faceIndices;
  MIntArray           _polygonCounts;
  MIntArray           _polygonConnects;
  MFloatArray         _uarray;
  MFloatArray         _varray;
  MIntArray           _uvIds;
  MObject             _transNode;
  MObject             _shapeNode;
  EggGroup            *_parent;
  MDagPath            _shape_dag_path;
  int                 _vert_count;
  int                 _tvert_count;
  int                 _cvert_count;
  int                 _face_count;
  vector<MayaEggTex*> _face_tex;
  
  VertTable  _vert_tab;
  TVertTable _tvert_tab;
  CVertTable _cvert_tab;
  
  int GetVert(EggVertex *vert, EggGroup *context);
  int GetTVert(TexCoordd uv);
  int GetCVert(Colorf col);
  int AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2, MayaEggTex *tex);
  EggGroup *GetControlJoint(void);
  void ConnectTextures(void);
  void AssignNames(void);
};

#define CTRLJOINT_DEFORM ((EggGroup*)((char*)(-1)))

int MayaEggMesh::GetVert(EggVertex *vert, EggGroup *context)
{
  MayaEggVertex vtx;
  vtx._pos = vert->get_pos3();
  if (vert->has_normal()) {
    vtx._normal = vert->get_normal();
  }
  vtx._index = 0;

  EggVertex::GroupRef::const_iterator gri;
  for (gri = vert->gref_begin(); gri != vert->gref_end(); ++gri) {
    EggGroup *egg_joint = (*gri);
    double membership = egg_joint->get_vertex_membership(vert);
    vtx._weights.push_back(MayaEggWeight(membership, egg_joint));
  }
  if (vtx._weights.size()==0) {
    if (context != 0) {
      vtx._weights.push_back(MayaEggWeight(1.0, context));
    }
  }
  
  VertTable::const_iterator vti = _vert_tab.find(vtx);
  if (vti != _vert_tab.end()) {
    return vti->_index;
  }
  
  vtx._index = _vert_count++;
  _vertexArray.append(MakeMayaPoint(vtx._pos));
  if (vert->has_normal()) {
    _normalArray.append(MakeMayaVector(vtx._normal));
    _vertNormalIndices.append(vtx._index);
  }
  if (vert->has_color()) {
    mayaloader_cat.info() << "found a vertex color\n";
    _vertColorArray.append(MakeMayaColor(vert->get_color()));
    _vertColorIndices.append(vtx._index);
  }
  _vert_tab.insert(vtx);
  return vtx._index;
}

int MayaEggMesh::GetTVert(TexCoordd uv)
{
  if (_tvert_tab.count(uv)) {
    return _tvert_tab[uv];
  }
  int idx = _tvert_count++;
  _uarray.append(uv.get_x());
  _varray.append(uv.get_y());
  _tvert_tab[uv] = idx;
  return idx;
}

int MayaEggMesh::GetCVert(Colorf col)
{
  //  if (_cvert_tab.count(col))
  //    return _cvert_tab[col];
  //  if (_cvert_count == _mesh->numCVerts) {
  //    int nsize = _cvert_count*2 + 100;
  //    _mesh->setNumVertCol(nsize, _cvert_count?TRUE:FALSE);
  //  }
  //  int idx = _cvert_count++;
  //  _mesh->vertCol[idx] = Point3(col.get_x(), col.get_y(), col.get_z());
  //  _cvert_tab[col] = idx;
  //  return idx;
  return 0;
}

void MayaEggMesh::AssignNames(void)
{
  string name = _pool->get_name();
  int nsize = name.size();
  if ((nsize > 6) && (name.rfind(".verts")==(nsize-6))) {
    name.resize(nsize-6);
  }
  MFnDependencyNode dnshape(_shapeNode);
  MFnDependencyNode dntrans(_transNode);
  dnshape.setName(MString(name.c_str())+"Shape");
  dntrans.setName(MString(name.c_str()));
}

MayaEggMesh *MayaEggLoader::GetMesh(EggVertexPool *pool, EggGroup *parent)
{
  MayaEggMesh *result = _mesh_tab[pool];
  if (result == 0) {
    result = new MayaEggMesh;
    result->_pool = pool;
    result->_parent = parent;
    result->_vert_count = 0;
    result->_tvert_count = 0;
    result->_cvert_count = 0;
    result->_face_count = 0;
    result->_vertColorArray.clear();
    result->_vertNormalIndices.clear();
    result->_vertColorIndices.clear();
    result->_faceColorArray.clear();
    result->_faceIndices.clear();
    _mesh_tab[pool] = result;
  }
  return result;
}

int MayaEggMesh::AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2, MayaEggTex *tex)
{
  int idx = _face_count++;
  _polygonCounts.append(3);
  _polygonConnects.append(v0);
  _polygonConnects.append(v1);
  _polygonConnects.append(v2);
  _uvIds.append(tv0);
  _uvIds.append(tv1);
  _uvIds.append(tv2);
  _face_tex.push_back(tex);
  return idx;
}

void MayaEggMesh::ConnectTextures(void)
{
  bool subtex = false;
  for (int i=1; i<_face_count; i++) {
    if (_face_tex[i] != _face_tex[0]) {
      subtex = true;
    }
  }
  if (!subtex) {
    MFnSet sg(_face_tex[0]->_shading_group);
    sg.addMember(_shapeNode);
    return;
  }
  for (int i=0; i<_face_count; i++) {
    MayaEggTex *tex = _face_tex[i];
    if (tex->_component.object()==MObject::kNullObj) {
      tex->_component.create(MFn::kMeshPolygonComponent);
    }
    tex->_component.addElement(i);
  }
  for (int i=0; i<_face_count; i++) {
    MayaEggTex *tex = _face_tex[i];
    if (tex->_component.object()!=MObject::kNullObj) {
      MFnSet sg(tex->_shading_group);
      sg.addMember(_shape_dag_path, tex->_component.object());
      tex->_component.setObject(MObject::kNullObj);
    }
  }
}

EggGroup *MayaEggMesh::GetControlJoint(void)
{
  EggGroup *result;
  VertTable::const_iterator vert = _vert_tab.begin();
  if (vert == _vert_tab.end()) {
    return 0;
  }
  switch (vert->_weights.size()) {
  case 0: 
    for (++vert; vert != _vert_tab.end(); ++vert) {
      if (vert->_weights.size() != 0) {
        return CTRLJOINT_DEFORM;
      }
    }
    return 0;
  case 1:
    result = vert->_weights[0].second;
    for (++vert; vert != _vert_tab.end(); ++vert) {
      if ((vert->_weights.size() != 1) || (vert->_weights[0].second != result)) {
        return CTRLJOINT_DEFORM;
      }
    }
    return result;
  default:
    return CTRLJOINT_DEFORM;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// MayaAnim: 
//
///////////////////////////////////////////////////////////////////////////////////////////////////
class MayaAnim
{
public:
  string  _name;
  EggTable *_joint;
  EggXfmSAnim *_pool;
  void PrintData(void);
};

MayaAnim *MayaEggLoader::GetAnim(EggXfmSAnim *pool)
{
  MayaAnim *result = _anim_tab[pool];
  if (result == 0) {
    result = new MayaAnim;
    result->_pool = pool;
    result->_name = pool->get_name();
    _anim_tab[pool] = result;
    EggNode *jointNode = (DCAST(EggNode, pool))->get_parent();
    EggTable *joint = DCAST(EggTable, jointNode);
    result->_joint = joint;

  }
  return result;
}

void MayaAnim::PrintData(void)
{
  if (mayaloader_cat.is_debug()) {
    mayaloader_cat.debug() << "anim on joint : " << _joint->get_name() << endl;
  }
  _pool->write(mayaloader_cat.debug(), 0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// MayaEggLoader functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

void MayaEggLoader::CreateSkinCluster(MayaEggMesh *M)
{
  MString cmd("skinCluster -mi ");
  vector <MayaEggJoint *> joints;

  VertTable::const_iterator vert;
  int maxInfluences = 0;
  for (vert=M->_vert_tab.begin(); vert != M->_vert_tab.end(); ++vert) {
    if ((int)(vert->_weights.size()) > maxInfluences) {
      maxInfluences = vert->_weights.size();
    }
    for (unsigned int i=0; i<vert->_weights.size(); i++) {
      double strength = vert->_weights[i].first;
      MayaEggJoint *joint = FindJoint(vert->_weights[i].second);
      if (!joint->_inskin) {
        joint->_inskin = true;
        joint->_index = joints.size();
        joints.push_back(joint);
      }
    }
  }
  cmd += maxInfluences;

  mayaloader_cat.spam() << joints.size() << " joints have weights on " << M->_pool->get_name() << endl;
  
  for (unsigned int i=0; i<joints.size(); i++) {
    MFnDependencyNode joint(joints[i]->_joint);
    cmd = cmd + " ";
    cmd = cmd + joint.name();
  }
  
  MFnDependencyNode shape(M->_shapeNode);
  cmd = cmd + " ";
  cmd = cmd + shape.name();
  
  MStatus status;
  MDGModifier dgmod;
  status = dgmod.commandToExecute(cmd);
  if (status != MStatus::kSuccess) { 
    perror("skinCluster commandToExecute");
    return; 
  }
  status = dgmod.doIt();
  if (status != MStatus::kSuccess) {
    perror("skinCluster doIt");
    return; 
  }
  
  MPlugArray oldplugs;
  MPlug inPlug = shape.findPlug("inMesh");
  if ((!inPlug.connectedTo(oldplugs,true,false))||(oldplugs.length() != 1)) {
    cerr << "skinCluster command failed";
    return;
  }
  MFnSkinCluster skinCluster(oldplugs[0].node());
  MIntArray influenceIndices;
  MFnSingleIndexedComponent component;
  component.create(MFn::kMeshVertComponent);
  component.setCompleteData(M->_vert_count);
  
  for (unsigned int i=0; i<joints.size(); i++) {
    unsigned int index = skinCluster.indexForInfluenceObject(joints[i]->_joint_dag_path, &status);
    if (status != MStatus::kSuccess) {
      perror("skinCluster index"); 
      return;
    }
    influenceIndices.append((int)index);
  }

  MDagPathArray paths;
  unsigned infcount = skinCluster.influenceObjects(paths, &status);
  if (status != MStatus::kSuccess) {
    perror("influenceObjects");
    return;
  }
  for (unsigned int i=0; i<infcount; i++) {
    unsigned int index = skinCluster.indexForInfluenceObject(paths[i], &status);
    if (status != MStatus::kSuccess) {
      perror("skinCluster index");
      return;
    }
    skinCluster.setWeights(M->_shape_dag_path, component.object(), index, 0.0, false, NULL);
  }

  MFloatArray values;
  int tot = M->_vert_count * joints.size();
  values.setLength(tot);
  for (int i=0; i<tot; i++) {
    values[i] = 0.0;
  }
  for (vert=M->_vert_tab.begin(); vert != M->_vert_tab.end(); ++vert) {
    for (unsigned int i=0; i<vert->_weights.size(); i++) {
      double strength = vert->_weights[i].first;
      MayaEggJoint *joint = FindJoint(vert->_weights[i].second);
      values[vert->_index * joints.size() + joint->_index] = (float)strength;
    }
  }
  skinCluster.setWeights(M->_shape_dag_path, component.object(), influenceIndices, values, false, NULL);

  for (unsigned int i=0; i<joints.size(); i++) {
    joints[i]->_inskin = false;
    joints[i]->_index = -1;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TraverseEggData
//
// We have an EggData in memory, and now we're going to copy that
// over into the maya scene graph.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void MayaEggLoader::TraverseEggNode(EggNode *node, EggGroup *context, string delim)
{
  vector<int> vertIndices;
  vector<int> tvertIndices;
  vector<int> cvertIndices;
  
  string delstring = " ";
  
  if (node->is_of_type(EggPolygon::get_class_type())) {
    EggPolygon *poly = DCAST(EggPolygon, node);

    MayaEggTex *tex = 0;
    LMatrix3d uvtrans = LMatrix3d::ident_mat();
 
    if (poly->has_texture()) {
      EggTexture *etex = poly->get_texture(0);
      tex = GetTex(etex->get_name(), etex->get_fullpath().to_os_specific());
      if (etex->has_transform())
        uvtrans = etex->get_transform2d();
    } else {
      tex = GetTex("","");
    }
    
    EggPolygon::const_iterator ci;
    MayaEggMesh *mesh = GetMesh(poly->get_pool(), context);
    vertIndices.clear();
    tvertIndices.clear();
    cvertIndices.clear();
    int numPolys = 0;
    for (ci = poly->begin(); ci != poly->end(); ++ci) {
      EggVertex *vtx = (*ci);
      EggVertexPool *pool = poly->get_pool();
      TexCoordd uv(0,0);
      if (vtx->has_uv()) {
        uv = vtx->get_uv();
      }
      vertIndices.push_back(mesh->GetVert(vtx, context));
      tvertIndices.push_back(mesh->GetTVert(uv * uvtrans));
      cvertIndices.push_back(mesh->GetCVert(vtx->get_color()));
      numPolys++;
    }
    for (unsigned int i=1; i<vertIndices.size()-1; i++) {
      if (poly->has_color()) {
        if (mayaloader_cat.is_spam()) {
          mayaloader_cat.spam() << "found a face color\n";
        }
        mesh->_faceIndices.append(mesh->_face_count);
        mesh->_faceColorArray.append(MakeMayaColor(poly->get_color()));
      }
      mesh->AddFace(vertIndices[0], vertIndices[i], vertIndices[i+1],
                    tvertIndices[0], tvertIndices[i], tvertIndices[i+1],
                    cvertIndices[0], cvertIndices[i], cvertIndices[i+1],
                    tex);
    }
  } else if (node->is_of_type(EggComment::get_class_type())) {
    string comment = (DCAST(EggComment, node))->get_comment();
    if (comment.find("2egg") != string::npos) {
      if (mayaloader_cat.is_spam()) {
        mayaloader_cat.spam() << delim+delstring << "found an EggComment: " << comment << endl;
      }
      if (comment.find("chan") != string::npos) {
        ParseFrameInfo(comment);
      }
    }
  } else if (node->is_of_type(EggSAnimData::get_class_type())) {
    if (mayaloader_cat.is_debug()) {
      mayaloader_cat.debug() << delim+delstring << "found an EggSAnimData: " << node->get_name() << endl;
    }
    //EggSAnimData *anim = DCAST(EggSAnimData, node);
    //MayaAnimData *animData = GetAnimData(anim, DCAST(EggXfmSAnim, node->get_parent()));
    //animData->PrintData();
    //if (_end_frame < animData->_pool->get_num_rows()) {
    //  _end_frame = animData->_pool->get_num_rows();
    //}
  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, node);
    if (node->is_of_type(EggGroup::get_class_type())) {
      EggGroup *group = DCAST(EggGroup, node);
      string parent_name = "";
      if (context)
        parent_name = context->get_name();
      if (group->is_joint()) {
        if (mayaloader_cat.is_debug()) {
          mayaloader_cat.debug() << delim+delstring << group->get_name() << ":" << parent_name << endl;
        }
        MakeJoint(group, context);
        context = group;
      } else {
        // lets create a group node for it so that it is reflected in Maya
        if (mayaloader_cat.is_debug()) {
          mayaloader_cat.debug() << delim+delstring << group->get_name() << "@" << parent_name << endl;
        }
        MakeGroup(group, context);
        context = group;
      }
    } else if (node->is_of_type(EggTable::get_class_type())) {
      //EggTable *anim = DCAST(EggTable, node);
      if (mayaloader_cat.is_debug()) {
        mayaloader_cat.debug() << delim+delstring << "found an EggTable: " << node->get_name() << endl;
      }
    } else if (node->is_of_type(EggXfmSAnim::get_class_type())) {
      MayaAnim *anim = GetAnim(DCAST(EggXfmSAnim, node));
      //anim->PrintData();
      if (mayaloader_cat.is_debug()) {
        mayaloader_cat.debug() << delim+delstring << "found an EggXfmSAnim: " << node->get_name() << endl;
      }
    }
    
    EggGroupNode::const_iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      TraverseEggNode(*ci, context, delim+delstring);
    }
  } 
}

bool MayaEggLoader::ConvertEggData(EggData *data, bool merge, bool model, bool anim, bool respect_normals)
{
  if (!merge) {
    mayaloader_cat.error() << "Currently, only 'merge' mode is implemented.\n";
    return false;
  }

  /*  
  if ((anim) || (!model)) {
    mayaloader_cat.error() << "Currently, only model-loading is implemented.\n";
    return false;
  }
  */

  _start_frame = 0;
  _end_frame = 0;
  _frame_rate = 24;
  _timeUnit = MTime::kFilm;

  MeshTable::const_iterator ci;
  JointTable::const_iterator ji;
  TexTable::const_iterator ti;

  AnimTable::const_iterator ei;
  
  if (MGlobal::isYAxisUp()) {
    data->set_coordinate_system(CS_yup_right);
  } else {
    data->set_coordinate_system(CS_zup_right);
  }

  if (mayaloader_cat.is_debug()) {
    mayaloader_cat.debug() << "root node: " << data->get_type() << endl;
  }
  TraverseEggNode(data, NULL, "");
  
  MStatus status;
  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MayaEggMesh *mesh = (*ci).second;
    if (mesh->_face_count==0) {
      continue;
    }

    //    MStatus status;
    MFnMesh mfn;
    MString cset;
    
    MayaEggGroup *parentNode = FindGroup(mesh->_parent);
    MObject parent = MObject::kNullObj;
    if (parentNode) {
      parent = parentNode->_group;
      if (mayaloader_cat.is_debug()) {
        mayaloader_cat.debug() << "mesh's parent (group) : " << parentNode->_name << endl;
      }
    } else {
      if (mayaloader_cat.is_debug()) {
        mayaloader_cat.debug() << "mesh's parent (null) : " << endl;
      }
    }
    mesh->_transNode = mfn.create(mesh->_vert_count, mesh->_face_count,
                                  mesh->_vertexArray, mesh->_polygonCounts, mesh->_polygonConnects,
                                  mesh->_uarray, mesh->_varray,
                                  parent, &status);
    mesh->_shapeNode = mfn.object();
    mfn.getPath(mesh->_shape_dag_path);
    mesh->ConnectTextures();
    mfn.getCurrentUVSetName(cset);
    mfn.assignUVs(mesh->_polygonCounts, mesh->_uvIds, &cset);

    // lets try to set normals per vertex 
    if (respect_normals) {
      status = mfn.setVertexNormals(mesh->_normalArray, mesh->_vertNormalIndices, MSpace::kTransform);
      if (status != MStatus::kSuccess) {
        status.perror("setVertexNormals failed!");
      }
    }
   
    // lets try to set colors per vertex
    /*
    MDGModifier dgmod;
    status = dgmod.doIt();
    if (status != MStatus::kSuccess) {
      status.perror("setVertexColors doIt");
    }
    status = mfn.setVertexColors(mesh->_vertColorArray, mesh->_vertColorIndices, &dgmod);
    */
    status = mfn.setVertexColors(mesh->_vertColorArray, mesh->_vertColorIndices);
    if (status != MStatus::kSuccess) {
      status.perror("setVertexColors failed!");
    }
    status = mfn.setFaceColors(mesh->_faceColorArray, mesh->_faceIndices);
    /*
    if (status != MStatus::kSuccess) {
      status.perror("setFaceColors failed!");
    }
    */
  }

  double thickness = 0.0;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    MayaEggJoint *joint = (*ji).second;
    double dfo = ((*ji).second->GetPos()).length();
    if (dfo > thickness) {
      thickness = dfo;
    }
  }
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "thickness from joints: " << thickness << endl;
  }
  thickness = thickness * 0.025;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    MayaEggJoint *joint = (*ji).second;
    mayaloader_cat.spam() << "creating a joint: " << joint->_egg_joint->get_name() << endl;
    joint->ChooseEndPos(thickness);
    joint->CreateMayaBone(FindGroup(joint->_egg_parent));
  }
  mayaloader_cat.spam() << "went past all the joints" << endl;
  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MayaEggMesh *mesh = (*ci).second;
    EggGroup *joint = mesh->GetControlJoint();
    if (joint) {
      CreateSkinCluster(mesh);
    }
  }
  mayaloader_cat.spam() << "went past creating skin cluster" << endl;
  for (ci = _mesh_tab.begin();  ci != _mesh_tab.end();  ++ci) {
    (*ci).second->AssignNames();
  }
  mayaloader_cat.spam() << "went past mesh AssignNames" << endl;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    (*ji).second->AssignNames();
  }
  mayaloader_cat.spam() << "went past joint AssignNames" << endl;
  for (ti = _tex_tab.begin();   ti != _tex_tab.end();   ++ti) {
    (*ti).second->AssignNames();
  }
  mayaloader_cat.spam() << "went past tex AssignNames" << endl;
 
  if (mayaloader_cat.is_debug()) {
    mayaloader_cat.debug() << "-fri: " << _frame_rate << " -sf: " << _start_frame 
                           << " -ef: " << _end_frame << endl;
  }

  // masad: keep track of maximum frames of animation on all these joints
  MTime maxFrame(_start_frame - 1, _timeUnit);

  for (ei = _anim_tab.begin(); ei != _anim_tab.end(); ++ei) {
    MayaAnim *anim = (*ei).second;
    MObject node = GetDependencyNode(anim->_joint->get_name());
    MFnDagNode mfnNode(node, &status);

    MMatrix mMat = mfnNode.transformationMatrix(&status);

    MObject attrTX = mfnNode.attribute("translateX", &status);
    MObject attrTY = mfnNode.attribute("translateY", &status);
    MObject attrTZ = mfnNode.attribute("translateZ", &status);
    MObject attrRX = mfnNode.attribute("rotateX", &status);
    MObject attrRY = mfnNode.attribute("rotateY", &status);
    MObject attrRZ = mfnNode.attribute("rotateZ", &status);
    MObject attrSX = mfnNode.attribute("scaleX", &status);
    MObject attrSY = mfnNode.attribute("scaleY", &status);
    MObject attrSZ = mfnNode.attribute("scaleZ", &status);

    MFnAnimCurve mfnAnimCurveTX;
    MFnAnimCurve mfnAnimCurveTY;
    MFnAnimCurve mfnAnimCurveTZ;
    MFnAnimCurve mfnAnimCurveRX;
    MFnAnimCurve mfnAnimCurveRY;
    MFnAnimCurve mfnAnimCurveRZ;
    MFnAnimCurve mfnAnimCurveSX;
    MFnAnimCurve mfnAnimCurveSY;
    MFnAnimCurve mfnAnimCurveSZ;

    mfnAnimCurveTX.create(node, attrTX, MFnAnimCurve::kAnimCurveTL, NULL, &status);
    mfnAnimCurveTY.create(node, attrTY, MFnAnimCurve::kAnimCurveTL, NULL, &status);
    mfnAnimCurveTZ.create(node, attrTZ, MFnAnimCurve::kAnimCurveTL, NULL, &status);
    mfnAnimCurveRX.create(node, attrRX, MFnAnimCurve::kAnimCurveTA, NULL, &status);
    mfnAnimCurveRY.create(node, attrRY, MFnAnimCurve::kAnimCurveTA, NULL, &status);
    mfnAnimCurveRZ.create(node, attrRZ, MFnAnimCurve::kAnimCurveTA, NULL, &status);
    mfnAnimCurveSX.create(node, attrSX, MFnAnimCurve::kAnimCurveTU, NULL, &status);
    mfnAnimCurveSY.create(node, attrSY, MFnAnimCurve::kAnimCurveTU, NULL, &status);
    mfnAnimCurveSZ.create(node, attrSZ, MFnAnimCurve::kAnimCurveTU, NULL, &status);

    MTransformationMatrix matrix( mMat );
    MVector trans = matrix.translation(MSpace::kTransform, &status);
      
    double rot[3];
    MTransformationMatrix::RotationOrder order = MTransformationMatrix::kXYZ;
    status = matrix.getRotation(rot, order);

    double scale[3];
    status = matrix.getScale(scale, MSpace::kTransform);
    MFnAnimCurve::TangentType tangent = MFnAnimCurve::kTangentClamped;
    MTime time(_start_frame - 1, _timeUnit);

    mfnAnimCurveTX.addKey(time, trans.x, tangent, tangent, NULL, &status);
    mfnAnimCurveTY.addKey(time, trans.y, tangent, tangent, NULL, &status);
    mfnAnimCurveTZ.addKey(time, trans.z, tangent, tangent, NULL, &status);
    mfnAnimCurveRX.addKey(time, rot[0], tangent, tangent, NULL, &status);
    mfnAnimCurveRY.addKey(time, rot[1], tangent, tangent, NULL, &status);
    mfnAnimCurveRZ.addKey(time, rot[2], tangent, tangent, NULL, &status);
    mfnAnimCurveSX.addKey(time, scale[0], tangent, tangent, NULL, &status);
    mfnAnimCurveSY.addKey(time, scale[1], tangent, tangent, NULL, &status);
    mfnAnimCurveSZ.addKey(time, scale[2], tangent, tangent, NULL, &status);

    for (int frame = 0; frame < anim->_pool->get_num_rows(); frame++)
    {
      LMatrix4d tMat;
      anim->_pool->get_value(frame, tMat);

      double matData[4][4] = {{tMat.get_cell(0,0), tMat.get_cell(0,1), tMat.get_cell(0,2), tMat.get_cell(0,3)},
                  {tMat.get_cell(1,0), tMat.get_cell(1,1), tMat.get_cell(1,2), tMat.get_cell(1,3)},
                  {tMat.get_cell(2,0), tMat.get_cell(2,1), tMat.get_cell(2,2), tMat.get_cell(2,3)},
                  {tMat.get_cell(3,0), tMat.get_cell(3,1), tMat.get_cell(3,2), tMat.get_cell(3,3)}};
      MMatrix mat(matData);

      matrix = MTransformationMatrix(mat);
      trans = matrix.translation(MSpace::kTransform, &status);
      status = matrix.getRotation(rot, order);
      status = matrix.getScale(scale, MSpace::kTransform);
      time = MTime(frame + _start_frame, _timeUnit);

      mfnAnimCurveTX.addKey(time, trans.x, tangent, tangent, NULL, &status);
      mfnAnimCurveTY.addKey(time, trans.y, tangent, tangent, NULL, &status);
      mfnAnimCurveTZ.addKey(time, trans.z, tangent, tangent, NULL, &status);
      mfnAnimCurveRX.addKey(time, rot[0], tangent, tangent, NULL, &status);
      mfnAnimCurveRY.addKey(time, rot[1], tangent, tangent, NULL, &status);
      mfnAnimCurveRZ.addKey(time, rot[2], tangent, tangent, NULL, &status);
      mfnAnimCurveSX.addKey(time, scale[0], tangent, tangent, NULL, &status);
      mfnAnimCurveSY.addKey(time, scale[1], tangent, tangent, NULL, &status);
      mfnAnimCurveSZ.addKey(time, scale[2], tangent, tangent, NULL, &status);
    }
    if (maxFrame < time) {
      maxFrame = time;
    }
  }
  if (anim) {
    // masad: set the control's max time with maxFrame
    MAnimControl::setMaxTime(maxFrame);
  }

  for (ci = _mesh_tab.begin();  ci != _mesh_tab.end();  ++ci) {
    delete (*ci).second;
  }
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    delete (*ji).second;
  }
  for (ti = _tex_tab.begin();   ti != _tex_tab.end();   ++ti) {
    delete (*ti).second;
  }
  for (ei = _anim_tab.begin();  ei != _anim_tab.end();  ++ei) {
    delete (*ei).second;
  }

  //  ResumeSetKeyMode();
  //  ResumeAnimate();
  
  mayaloader_cat.info() << "Egg import successful\n";
  return true;
}

void MayaEggLoader::ParseFrameInfo(string comment)
{
  int length = 0;
  int pos, ls, le;

  pos = comment.find("-fri");
  if (pos != string::npos) {
    ls = comment.find(" ", pos+4);
    le = comment.find(" ", ls+1);
    if (mayaloader_cat.is_debug()) {
      mayaloader_cat.debug() <<    comment.substr(ls+1, le-ls-1) << endl;
    }
    _frame_rate = atoi(comment.substr(ls+1,le-ls-1).data());
    //mayaloader_cat.debug() << "le = " << le << "; and ls = " << ls << "; frame_rate = " << _frame_rate << endl;

    switch (_frame_rate) {
    case 15:
      _timeUnit = MTime::kGames;
      break;
    case 24:
      _timeUnit = MTime::kFilm;
      break;
    case 25:
      _timeUnit = MTime::kPALFrame;
      break;
    case 30:
      _timeUnit = MTime::kNTSCFrame;
      break;
    case 48:
      _timeUnit = MTime::kShowScan;
      break;
    case 50:
      _timeUnit = MTime::kPALField;
      break;
    case 60:
      _timeUnit = MTime::kNTSCField;
      break;
    case 2:
      _timeUnit = MTime::k2FPS;
      break;
    case 3:
      _timeUnit = MTime::k3FPS;
      break;
    case 4:
      _timeUnit = MTime::k4FPS;
      break;
    case 5:
      _timeUnit = MTime::k5FPS;
      break;
    case 6:
      _timeUnit = MTime::k6FPS;
      break;
    case 8:
      _timeUnit = MTime::k8FPS;
      break;
    case 10:
      _timeUnit = MTime::k10FPS;
      break;
    case 12:
      _timeUnit = MTime::k12FPS;
      break;
    case 16:
      _timeUnit = MTime::k16FPS;
      break;
    case 20:
      _timeUnit = MTime::k20FPS;
      break;
    case 40:
      _timeUnit = MTime::k40FPS;
      break;
    case 75:
      _timeUnit = MTime::k75FPS;
      break;
    case 80:
      _timeUnit = MTime::k80FPS;
      break;
    case 100:
      _timeUnit = MTime::k100FPS;
      break;
    default:
      _timeUnit = MTime::kFilm;
    }

  }

  pos = comment.find("-sf");
  if (pos != string::npos) {
    ls = comment.find(" ", pos+3);
    le = comment.find(" ", ls+1);
    if (mayaloader_cat.is_debug()) {
      mayaloader_cat.debug() <<    comment.substr(ls+1, le-ls-1) << endl;
    }
    if (le == string::npos) {
      _start_frame = atoi(comment.substr(ls+1,le).data());
    } else {
      _start_frame = atoi(comment.substr(ls+1,le-ls-1).data());
    }
    //mayaloader_cat.debug() << "le = " << le << "; and ls = " << ls << "; start_frame = " << _start_frame << endl;
  }
  pos = comment.find("-ef");
  if (pos != string::npos) {
    ls = comment.find(" ", pos+3);
    le = comment.find(" ", ls+1);
    if (mayaloader_cat.is_debug()) {
      mayaloader_cat.debug() <<    comment.substr(ls+1, le-ls-1) << endl;
    }
    if (le == string::npos) {
      _end_frame = atoi(comment.substr(ls+1,le).data());
    } else {
      _end_frame = atoi(comment.substr(ls+1,le-ls-1).data());
    }
    //mayaloader_cat.debug() << "le = " << le << "; and ls = " << ls << "; end_frame = " << _end_frame << endl;
  }


}

bool MayaEggLoader::ConvertEggFile(const char *name, bool merge, bool model, bool anim, bool respect_normals)
{
  EggData data;
  Filename datafn = Filename::from_os_specific(name);
  if (!data.read(datafn)) {
    mayaloader_cat.error() << "Cannot read Egg file for import\n";
    return false;
  }
  return ConvertEggData(&data, merge, model, anim, respect_normals);
}

MObject MayaEggLoader::GetDependencyNode(string givenName)
{
  MObject node;
  int pos;
  string name;

  pos = givenName.find(":");
  if (pos != string::npos) {
    name = givenName.substr(pos+1);
  } else
    name = givenName;

  MeshTable::const_iterator ci;
  JointTable::const_iterator ji;
  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MayaEggMesh *mesh = (*ci).second;
    
    string meshName = mesh->_pool->get_name();
    int nsize = meshName.size();
    if ((nsize > 6) && (meshName.rfind(".verts")==(nsize-6))) {
      meshName.resize(nsize-6);
    }
    if (meshName == name)
    {
      node = mesh->_transNode;
      return node;
    }
  }

  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    MayaEggJoint *joint = (*ji).second;
    mayaloader_cat.spam() << "traversing a joint: " << joint->_egg_joint->get_name() << endl;
    string jointName = joint->_egg_joint->get_name();
    if (jointName == name)
    {
      node = joint->_joint;
      return node;
    }
  }  
  
  return node;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The two global functions that form the API of this module.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool MayaLoadEggData(EggData *data, bool merge, bool model, bool anim, bool respect_normals)
{
  MayaEggLoader loader;
  return loader.ConvertEggData(data, merge, model, anim, respect_normals);
}

bool MayaLoadEggFile(const char *name, bool merge, bool model, bool anim, bool respect_normals)
{
  MayaEggLoader loader;
  return loader.ConvertEggFile(name, merge, model, anim, respect_normals);
}


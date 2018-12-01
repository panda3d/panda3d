/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maxEggLoader.cxx
 * @author jyelon
 * @date 2005-07-15
 *
 * This file contains the code for class MaxEggLoader.  This class
 * does the actual work of copying an EggData tree into the max scene.
 */

#include "pandatoolbase.h"
#include "notifyCategoryProxy.h"

#include "eggData.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "eggPrimitive.h"
#include "eggGroupNode.h"
#include "eggPolysetMaker.h"
#include "eggBin.h"

using std::min;
using std::max;

#include <stdio.h>
#include <Max.h>
#include <istdplug.h>
#include <stdmat.h>
#include <decomp.h>
#include <shape.h>
#include <simpobj.h>
#include <iparamb2.h>
#include <iskin.h>
#include <modstack.h>

#include "maxEggLoader.h"

using std::vector;

class MaxEggMesh;
class MaxEggJoint;
class MaxEggTex;

NotifyCategoryDeclNoExport(maxloader);
NotifyCategoryDef(maxloader, "");

class MaxEggLoader
{
public:
  bool ConvertEggData(EggData *data,    bool merge, bool model, bool anim);
  bool ConvertEggFile(const char *name, bool merge, bool model, bool anim);

public:
  void         TraverseEggNode(EggNode *node, EggGroup *context);
  MaxEggMesh  *GetMesh(EggVertexPool *pool);
  MaxEggJoint *FindJoint(EggGroup *joint);
  MaxEggJoint *MakeJoint(EggGroup *joint, EggGroup *context);
  MaxEggTex   *GetTex(const Filename &fn);
  void         CreateSkinModifier(MaxEggMesh *M);

  typedef phash_map<EggVertexPool *, MaxEggMesh *> MeshTable;
  typedef second_of_pair_iterator<MeshTable::const_iterator> MeshIterator;
  typedef phash_map<EggGroup *, MaxEggJoint *> JointTable;
  typedef second_of_pair_iterator<JointTable::const_iterator> JointIterator;
  typedef phash_map<std::string, MaxEggTex *> TexTable;
  typedef second_of_pair_iterator<TexTable::const_iterator> TexIterator;

  MeshTable        _mesh_tab;
  JointTable       _joint_tab;
  TexTable         _tex_tab;
  int              _next_tex;
};

Point3 MakeMaxPoint(LVector3d vec)
{
  return Point3(vec[0], vec[1], vec[2]);
}

// MaxEggTex

class MaxEggTex
{
public:
  Filename   _path;
  int        _id;
  StdMat    *_mat;
  BitmapTex *_bmt;
};

MaxEggTex *MaxEggLoader::GetTex(const Filename &fn)
{
  if (_tex_tab.count(fn))
    return _tex_tab[fn];

  BitmapTex *bmt = NewDefaultBitmapTex();
#ifdef _UNICODE
  bmt->SetMapName((TCHAR*) fn.to_os_specific_w().c_str());
#else
  bmt->SetMapName((char*) fn.to_os_specific().c_str());
#endif

  StdMat *mat = NewDefaultStdMat();
  mat->SetSubTexmap(ID_DI, bmt);
  mat->SetTexmapAmt(ID_DI, 1.0, 0);
  mat->EnableMap(ID_DI, TRUE);
  mat->SetActiveTexmap(bmt);
  GetCOREInterface()->ActivateTexture(bmt, mat);

  MaxEggTex *res = new MaxEggTex;
  res->_path = fn;
  res->_id = _next_tex ++;
  res->_bmt = bmt;
  res->_mat = mat;

  _tex_tab[fn] = res;
  return res;
}

// MaxEggJoint

class MaxEggJoint
{
public:
  LMatrix4d      _trans;
  LVector3d      _endpos;
  LVector3d      _perp;
  double         _thickness;
  bool           _inskin;
  SimpleObject2 *_bone;
  INode         *_node;
  EggGroup      *_egg_joint;
  MaxEggJoint   *_parent;
  vector <MaxEggJoint *> _children;

public:
  void GetRotation(LVector3d &xv, LVector3d &yv, LVector3d &zv);
  LVector3d GetPos(void) { return _trans.get_row3(3); }
  MaxEggJoint *ChooseBestChild(LVector3d dir);
  void ChooseEndPos(double thickness);
  void CreateMaxBone(void);
};

void MaxEggJoint::GetRotation(LVector3d &xv, LVector3d &yv, LVector3d &zv)
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

MaxEggJoint *MaxEggLoader::FindJoint(EggGroup *joint)
{
  if (joint==0) return 0;
  return _joint_tab[joint];
}

MaxEggJoint *MaxEggLoader::MakeJoint(EggGroup *joint, EggGroup *context)
{
  MaxEggJoint *parent = FindJoint(context);
  MaxEggJoint *result = new MaxEggJoint;
  LMatrix4d t = joint->get_transform3d();
  if (parent) {
    result->_trans = t * parent->_trans;
  } else {
    result->_trans = t;
  }
  result->_endpos = LVector3d(0,0,0);
  result->_perp = LVector3d(0,0,0);
  result->_thickness = 0.0;
  result->_inskin = false;
  result->_bone = 0;
  result->_node = 0;
  result->_egg_joint = joint;
  result->_parent = parent;
  if (parent) parent->_children.push_back(result);
  _joint_tab[joint] = result;
  return result;
}

MaxEggJoint *MaxEggJoint::ChooseBestChild(LVector3d dir)
{
  if (dir.length() < 0.001) return 0;
  dir.normalize();
  double firstbest = -1000;
  MaxEggJoint *firstchild = 0;
  LVector3d firstpos = GetPos();
  double secondbest = 0;
  for (int i=0; i<_children.size(); i++) {
    MaxEggJoint *child = _children[i];
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
  if (firstbest > secondbest + 0.1)
    return firstchild;
  return 0;
}

void MaxEggJoint::ChooseEndPos(double thickness)
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
  fwd.normalize();
  MaxEggJoint *child = ChooseBestChild(fwd);
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
  if (altaxis.length() < 0.001) altaxis = orient.cross(LVector3d(0,0,1));
  _perp = altaxis.cross(orient);
  _perp.normalize();
}

void MaxEggJoint::CreateMaxBone(void)
{
  LVector3d rxv,ryv,rzv;
  GetRotation(rxv, ryv, rzv);
  Point3 xv(MakeMaxPoint(rxv));
  Point3 yv(MakeMaxPoint(ryv));
  Point3 zv(MakeMaxPoint(rzv));
  Point3 pos(MakeMaxPoint(GetPos()));
  Point3 endpos(MakeMaxPoint(_endpos));
  Point3 tzv(MakeMaxPoint(_perp));

  Point3 fwd = endpos - pos;
  double len = fwd.Length();
  Point3 txv = fwd * ((PN_stdfloat)(1.0/len));
  Point3 tyv = tzv ^ txv;
  Point3 row1 = Point3(txv % xv, txv % yv, txv % zv);
  Point3 row2 = Point3(tyv % xv, tyv % yv, tyv % zv);
  Point3 row3 = Point3(tzv % xv, tzv % yv, tzv % zv);
  Matrix3 oomat(row1,row2,row3,Point3(0,0,0));
  Quat ooquat(oomat);
  _bone = (SimpleObject2*)CreateInstance(GEOMOBJECT_CLASS_ID, BONE_OBJ_CLASSID);
  _node = GetCOREInterface()->CreateObjectNode(_bone);
  _node->SetNodeTM(0, Matrix3(xv, yv, zv, pos));
  IParamBlock2 *blk = _bone->pblock2;
  for (int i=0; i<blk->NumParams(); i++) {
    TSTR n = blk->GetLocalName(i);
    if      (_tcscmp(n, _T("Length")) == 0) blk->SetValue(i, 0, (PN_stdfloat) len);
    else if (_tcscmp(n, _T("Width")) == 0)  blk->SetValue(i, 0, (PN_stdfloat) _thickness);
    else if (_tcscmp(n, _T("Height")) == 0) blk->SetValue(i, 0, (PN_stdfloat) _thickness);
  }
  Point3 boneColor = GetUIColor(COLOR_BONES);
  _node->SetWireColor(RGB(int(boneColor.x*255.0f), int(boneColor.y*255.0f), int(boneColor.z*255.0f) ));
  _node->SetBoneNodeOnOff(TRUE, 0);
  _node->SetRenderable(FALSE);

#ifdef _UNICODE
  TCHAR wname[1024];
  wname[1023] = 0;
  mbstowcs(wname, _egg_joint->get_name().c_str(), 1023);
  _node->SetName(wname);
#else
  _node->SetName((char*) _egg_joint->get_name().c_str());
#endif

  _node->SetObjOffsetRot(ooquat);
  if (_parent) {
    _node->Detach(0, 1);
    _parent->_node->AttachChild(_node, 1);
  }
}

// MaxEggMesh

typedef std::pair<double, EggGroup *> MaxEggWeight;

struct MaxEggVertex
{
  LVertexd              _pos;
  LNormald              _normal;
  vector<MaxEggWeight> _weights;
  int                  _index;
};

struct MEV_Compare: public stl_hash_compare<MaxEggVertex>
{
  size_t operator()(const MaxEggVertex &key) const
  {
    return key._pos.add_hash(key._normal.get_hash());
  }
  bool operator()(const MaxEggVertex &k1, const MaxEggVertex &k2) const
  {
    int n = k1._pos.compare_to(k2._pos);
    if (n < 0) return true;
    if (n > 0) return false;
    n = k1._normal.compare_to(k2._normal);
    if (n < 0) return true;
    if (n > 0) return false;
    n = k1._weights.size() - k2._weights.size();
    if (n < 0) return true;
    if (n > 0) return false;
    for (int i=0; i<k1._weights.size(); i++) {
      double d = k1._weights[i].first - k2._weights[i].first;
      if (d < 0) return true;
      if (d > 0) return false;
      EggGroup *g1 = k1._weights[i].second;
      EggGroup *g2 = k2._weights[i].second;
      if (g1 < g2) return true;
      if (g1 > g2) return false;
    }
    return false;
  }
};

typedef phash_set<MaxEggVertex, MEV_Compare> VertTable;
typedef phash_map<LTexCoordd, int>            TVertTable;
typedef phash_map<LColor, int>               CVertTable;

class MaxEggMesh
{
public:

  std::string           _name;
  TriObject       *_obj;
  Mesh            *_mesh;
  INode           *_node;
  IDerivedObject  *_dobj;
  Modifier        *_skin_mod;
  ISkin           *_iskin;
  ISkinImportData *_iskin_import;
  int              _vert_count;
  int              _tvert_count;
  int              _cvert_count;
  int              _face_count;

  VertTable  _vert_tab;
  TVertTable _tvert_tab;
  CVertTable _cvert_tab;

  int GetVert(EggVertex *vert, EggGroup *context);
  int GetTVert(const LTexCoordd &uv);
  int GetCVert(const LColor &col);
  int AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2, int tex);
  EggGroup *GetControlJoint(void);
};

#define CTRLJOINT_DEFORM ((EggGroup*)((char*)(-1)))

int MaxEggMesh::GetVert(EggVertex *vert, EggGroup *context)
{
  MaxEggVertex vtx;
  vtx._pos = vert->get_pos3();
  vtx._normal = vert->get_normal();
  vtx._index = 0;

  EggVertex::GroupRef::const_iterator gri;
  for (gri = vert->gref_begin(); gri != vert->gref_end(); ++gri) {
    EggGroup *egg_joint = (*gri);
    double membership = egg_joint->get_vertex_membership(vert);
    vtx._weights.push_back(MaxEggWeight(membership, egg_joint));
  }
  if (vtx._weights.size()==0) {
    if (context != 0)
      vtx._weights.push_back(MaxEggWeight(1.0, context));
  }

  VertTable::const_iterator vti = _vert_tab.find(vtx);
  if (vti != _vert_tab.end())
    return vti->_index;

  if (_vert_count == _mesh->numVerts) {
    int nsize = _vert_count*2 + 100;
    _mesh->setNumVerts(nsize, _vert_count?TRUE:FALSE);
  }
  vtx._index = _vert_count++;
  _vert_tab.insert(vtx);
  _mesh->setVert(vtx._index, MakeMaxPoint(vtx._pos));
  return vtx._index;
}

int MaxEggMesh::GetTVert(const LTexCoordd &uv)
{
  if (_tvert_tab.count(uv))
    return _tvert_tab[uv];
  if (_tvert_count == _mesh->numTVerts) {
    int nsize = _tvert_count*2 + 100;
    _mesh->setNumTVerts(nsize, _tvert_count?TRUE:FALSE);
  }
  int idx = _tvert_count++;
  _mesh->setTVert(idx, uv.get_x(), uv.get_y(), 0.0);
  _tvert_tab[uv] = idx;
  return idx;
}

int MaxEggMesh::GetCVert(const LColor &col)
{
  if (_cvert_tab.count(col))
    return _cvert_tab[col];
  if (_cvert_count == _mesh->numCVerts) {
    int nsize = _cvert_count*2 + 100;
    _mesh->setNumVertCol(nsize, _cvert_count?TRUE:FALSE);
  }
  int idx = _cvert_count++;
  _mesh->vertCol[idx] = Point3(col.get_x(), col.get_y(), col.get_z());
  _cvert_tab[col] = idx;
  return idx;
}

MaxEggMesh *MaxEggLoader::GetMesh(EggVertexPool *pool)
{
  MaxEggMesh *result = _mesh_tab[pool];
  if (result == 0) {
    std::string name = pool->get_name();
    int nsize = name.size();
    if ((nsize > 6) && (name.rfind(".verts")==(nsize-6)))
      name.resize(nsize-6);
    result = new MaxEggMesh;
    result->_name = name;
    result->_obj  = CreateNewTriObject();
    result->_mesh = &result->_obj->GetMesh();
    result->_mesh->setMapSupport(0, TRUE);
    result->_node = GetCOREInterface()->CreateObjectNode(result->_obj);
    result->_dobj = 0;
    result->_skin_mod = 0;
    result->_iskin = 0;
    result->_iskin_import = 0;
    result->_vert_count = 0;
    result->_tvert_count = 0;
    result->_cvert_count = 0;
    result->_face_count = 0;
#ifdef _UNICODE
    TCHAR wname[1024];
    wname[1023] = 0;
    mbstowcs(wname, name.c_str(), 1023);
    result->_node->SetName(wname);
#else
    result->_node->SetName((char*) name.c_str());
#endif
    _mesh_tab[pool] = result;
  }
  return result;
}

int MaxEggMesh::AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2, int tex)
{
  static int dump = 0;
  if (_face_count == _mesh->numFaces) {
    int nsize = _face_count*2 + 100;
    BOOL keep = _mesh->numFaces ? TRUE : FALSE;
    _mesh->setNumFaces(nsize, keep);
    _mesh->setNumTVFaces(nsize, keep, _face_count);
    _mesh->setNumVCFaces(nsize, keep, _face_count);
  }
  int idx = _face_count++;
  _mesh->faces[idx].setVerts(v0,v1,v2);
  _mesh->faces[idx].smGroup = 1;
  _mesh->faces[idx].flags = EDGE_ALL | HAS_TVERTS;
  _mesh->faces[idx].setMatID(tex);
  _mesh->tvFace[idx].setTVerts(tv0,tv1,tv2);
  _mesh->vcFace[idx].setTVerts(cv0,cv1,cv2);
  return idx;
}

EggGroup *MaxEggMesh::GetControlJoint(void)
{
  EggGroup *result;
  VertTable::const_iterator vert = _vert_tab.begin();
  if (vert == _vert_tab.end()) return 0;
  switch (vert->_weights.size()) {
  case 0:
    for (++vert; vert != _vert_tab.end(); ++vert)
      if (vert->_weights.size() != 0)
        return CTRLJOINT_DEFORM;
    return 0;
  case 1:
    result = vert->_weights[0].second;
    for (++vert; vert != _vert_tab.end(); ++vert)
      if ((vert->_weights.size() != 1) || (vert->_weights[0].second != result))
        return CTRLJOINT_DEFORM;
    return result;
  default:
    return CTRLJOINT_DEFORM;
  }
}

void MaxEggLoader::CreateSkinModifier(MaxEggMesh *M)
{
  vector <MaxEggJoint *> joints;

  M->_dobj = CreateDerivedObject(M->_obj);
  M->_node->SetObjectRef(M->_dobj);
  M->_skin_mod = (Modifier*)CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);
  M->_iskin = (ISkin*)M->_skin_mod->GetInterface(I_SKIN);
  M->_iskin_import = (ISkinImportData*)M->_skin_mod->GetInterface(I_SKINIMPORTDATA);
  M->_dobj->SetAFlag(A_LOCK_TARGET);
  M->_dobj->AddModifier(M->_skin_mod);
  M->_dobj->ClearAFlag(A_LOCK_TARGET);
  GetCOREInterface()->ForceCompleteRedraw();

  VertTable::const_iterator vert;
  for (vert=M->_vert_tab.begin(); vert != M->_vert_tab.end(); ++vert) {
    for (int i=0; i<vert->_weights.size(); i++) {
      double strength = vert->_weights[i].first;
      MaxEggJoint *joint = FindJoint(vert->_weights[i].second);
      if (!joint->_inskin) {
        joint->_inskin = true;
        joints.push_back(joint);
      }
    }
  }
  for (int i=0; i<joints.size(); i++) {
    BOOL last = (i == (joints.size()-1)) ? TRUE : FALSE;
    M->_iskin_import->AddBoneEx(joints[i]->_node, last);
    joints[i]->_inskin = false;
  }

  GetCOREInterface()->SetCommandPanelTaskMode(TASK_MODE_MODIFY);
  GetCOREInterface()->SelectNode(M->_node);
  GetCOREInterface()->ForceCompleteRedraw();

  for (vert=M->_vert_tab.begin(); vert != M->_vert_tab.end(); ++vert) {
    Tab<INode*> maxJoints;
    Tab<PN_stdfloat> maxWeights;
    maxJoints.ZeroCount();
    maxWeights.ZeroCount();
    for (int i=0; i<vert->_weights.size(); i++) {
      PN_stdfloat strength = (PN_stdfloat)(vert->_weights[i].first);
      MaxEggJoint *joint = FindJoint(vert->_weights[i].second);
      maxWeights.Append(1,&strength);
      maxJoints.Append(1,&(joint->_node));
    }
    M->_iskin_import->AddWeights(M->_node, vert->_index, maxJoints, maxWeights);
  }
}

// TraverseEggData We have an EggData in memory, and now we're going to copy
// that over into the max scene graph.

void MaxEggLoader::TraverseEggNode(EggNode *node, EggGroup *context)
{
  vector<int> vertIndices;
  vector<int> tvertIndices;
  vector<int> cvertIndices;

  if (node->is_of_type(EggPolygon::get_class_type())) {
    EggPolygon *poly = DCAST(EggPolygon, node);

    int texid;
    LMatrix3d uvtrans = LMatrix3d::ident_mat();
    if (poly->has_texture()) {
      EggTexture *tex = poly->get_texture(0);
      texid = GetTex(tex->get_fullpath())->_id;
      if (tex->has_transform())
        uvtrans = tex->get_transform2d();
    } else {
      texid = GetTex(Filename())->_id;
    }

    EggPolygon::const_iterator ci;
    MaxEggMesh *mesh = GetMesh(poly->get_pool());
    vertIndices.clear();
    tvertIndices.clear();
    cvertIndices.clear();
    for (ci = poly->begin(); ci != poly->end(); ++ci) {
      EggVertex *vtx = (*ci);
      EggVertexPool *pool = poly->get_pool();
      LTexCoordd uv = vtx->get_uv();
      vertIndices.push_back(mesh->GetVert(vtx, context));
      tvertIndices.push_back(mesh->GetTVert(uv * uvtrans));
      cvertIndices.push_back(mesh->GetCVert(vtx->get_color()));
    }
    for (int i=1; i<vertIndices.size()-1; i++)
      mesh->AddFace(vertIndices[0], vertIndices[i], vertIndices[i+1],
                    tvertIndices[0], tvertIndices[i], tvertIndices[i+1],
                    cvertIndices[0], cvertIndices[i], cvertIndices[i+1],
                    texid);
  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, node);
    if (node->is_of_type(EggGroup::get_class_type())) {
      EggGroup *group = DCAST(EggGroup, node);
      if (group->is_joint()) {
        MakeJoint(group, context);
        context = group;
      }
    }
    EggGroupNode::const_iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      TraverseEggNode(*ci, context);
    }
  }
}

bool MaxEggLoader::ConvertEggData(EggData *data, bool merge, bool model, bool anim)
{
  if (!merge) {
    maxloader_cat.error() << "Currently, only 'merge' mode is implemented.\n";
    return false;
  }

  if ((anim) || (!model)) {
    maxloader_cat.error() << "Currently, only model-loading is implemented.\n";
    return false;
  }

  MeshIterator ci;
  JointIterator ji;
  TexIterator ti;

  data->set_coordinate_system(CS_zup_right);

  SuspendAnimate();
  SuspendSetKeyMode();
  AnimateOff();
  _next_tex = 0;

  TraverseEggNode(data, nullptr);

  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MaxEggMesh *mesh = (*ci);
    mesh->_mesh->setNumVerts(mesh->_vert_count, TRUE);
    mesh->_mesh->setNumTVerts(mesh->_tvert_count, TRUE);
    mesh->_mesh->setNumVertCol(mesh->_cvert_count, TRUE);
    mesh->_mesh->setNumFaces(mesh->_face_count, TRUE);
    mesh->_mesh->setNumTVFaces(mesh->_face_count, TRUE, mesh->_face_count);
    mesh->_mesh->setNumVCFaces(mesh->_face_count, TRUE, mesh->_face_count);
    mesh->_mesh->InvalidateTopologyCache();
    mesh->_mesh->InvalidateGeomCache();
    mesh->_mesh->buildNormals();
  }

  double thickness = 0.0;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    double dfo = ((*ji)->GetPos()).length();
    if (dfo > thickness) thickness = dfo;
  }
  thickness = thickness * 0.025;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    MaxEggJoint *joint = *ji;
    joint->ChooseEndPos(thickness);
    joint->CreateMaxBone();
  }

  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MaxEggMesh *mesh = (*ci);
    EggGroup *joint = mesh->GetControlJoint();
    if (joint) CreateSkinModifier(mesh);
  }

  if (_next_tex) {
    TSTR name;
    MultiMtl *mtl = NewDefaultMultiMtl();
    mtl->SetNumSubMtls(_next_tex);
    for (ti = _tex_tab.begin(); ti != _tex_tab.end(); ++ti) {
      MaxEggTex *tex = *ti;
      mtl->SetSubMtlAndName(tex->_id, tex->_mat, name);
    }
    for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
      MaxEggMesh *mesh = *ci;
      mesh->_node->SetMtl(mtl);
    }
  }

  for (ci = _mesh_tab.begin();  ci != _mesh_tab.end();  ++ci) delete *ci;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) delete *ji;
  for (ti = _tex_tab.begin();   ti != _tex_tab.end();   ++ti) delete *ti;

  ResumeSetKeyMode();
  ResumeAnimate();

  maxloader_cat.info() << "Egg import successful\n";
  return true;
}

bool MaxEggLoader::ConvertEggFile(const char *name, bool merge, bool model, bool anim)
{
  EggData data;
  Filename datafn = Filename::from_os_specific(name);
  if (!data.read(datafn)) {
    maxloader_cat.error() << "Cannot read Egg file for import\n";
    return false;
  }
  return ConvertEggData(&data, merge, model, anim);
}

// The two global functions that form the API of this module.

bool MaxLoadEggData(EggData *data, bool merge, bool model, bool anim)
{
  MaxEggLoader loader;
  return loader.ConvertEggData(data, merge, model, anim);
}

bool MaxLoadEggFile(const char *name, bool merge, bool model, bool anim)
{
  MaxEggLoader loader;
  return loader.ConvertEggFile(name, merge, model, anim);
}

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
#include "post_maya_include.h"

#include "mayaEggLoader.h"

class MayaEggMesh;
class MayaEggJoint;
class MayaEggTex;

NotifyCategoryDeclNoExport(mayaloader);
NotifyCategoryDef(mayaloader, "");

class MayaEggLoader
{
public:
  bool ConvertEggData(EggData *data,    bool merge, bool model, bool anim);
  bool ConvertEggFile(const char *name, bool merge, bool model, bool anim);
  
public:
  void          TraverseEggNode(EggNode *node, EggGroup *context);
  MayaEggMesh  *GetMesh(EggVertexPool *pool);
  MayaEggJoint *FindJoint(EggGroup *joint);
  MayaEggJoint *MakeJoint(EggGroup *joint, EggGroup *context);
  MayaEggTex   *GetTex(const string &name, const string &fn);
  void          CreateSkinModifier(MayaEggMesh *M);

  typedef phash_map<EggVertexPool *, MayaEggMesh *> MeshTable;
  typedef second_of_pair_iterator<MeshTable::const_iterator> MeshIterator;
  typedef phash_map<EggGroup *, MayaEggJoint *> JointTable;
  typedef second_of_pair_iterator<JointTable::const_iterator> JointIterator;
  typedef phash_map<string, MayaEggTex *> TexTable;
  typedef second_of_pair_iterator<TexTable::const_iterator> TexIterator;

  MeshTable        _mesh_tab;
  JointTable       _joint_tab;
  TexTable         _tex_tab;
  CoordinateSystem _coord_sys;
};

MFloatPoint ConvertCoordSys(CoordinateSystem sys, LVector3d vec)
{
  if (sys == CS_zup_right) {
    return MFloatPoint(vec[0], vec[2], -vec[1]);
  } else {
    return MFloatPoint(vec[0], vec[1], vec[2]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MayaEggTex
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MayaEggTex
{
public:
  string  _path;
  MObject _file_texture;
  MObject _shader;
  MObject _shading_group;
  
  MFnSingleIndexedComponent _component;
};

MayaEggTex *MayaEggLoader::GetTex(const string &name, const string &fn)
{
  if (_tex_tab.count(fn))
    return _tex_tab[fn];

  MStatus status;
  MFnLambertShader shader;
  MFnDependencyNode filetex;
  MFnSet sgroup;
  MPlugArray oldplugs;
  MDGModifier dgmod;

  shader.create(true,&status);
  shader.setName(MString(name.c_str())+"Shader");
  sgroup.create(MSelectionList(), MFnSet::kRenderableOnly, &status);
  sgroup.setName(MString(name.c_str()));
  MPlug surfplug = sgroup.findPlug("surfaceShader");
  if (surfplug.connectedTo(oldplugs,true,false)) {
    for (int i=0; i<oldplugs.length(); i++) {
      MPlug src = oldplugs[i];
      status = dgmod.disconnect(src, surfplug);
      if (status != MStatus::kSuccess) status.perror("Disconnecting old shader");
    }
  }
  status = dgmod.connect(shader.findPlug("outColor"),surfplug);
  if (status != MStatus::kSuccess) status.perror("Connecting shader");
  if (fn != "") {
    filetex.create("file",&status);
    filetex.setName(MString(name.c_str())+"File");
    filetex.findPlug("fileTextureName").setValue(MString(fn.c_str()));
    dgmod.connect(filetex.findPlug("outColor"),shader.findPlug("color"));
  }
  status = dgmod.doIt();
  if (status != MStatus::kSuccess) status.perror("DGMod doIt");

  MayaEggTex *res = new MayaEggTex;
  res->_path = fn;
  res->_file_texture = filetex.object();
  res->_shader = shader.object();
  res->_shading_group = sgroup.object();
  
  _tex_tab[fn] = res;
  return res;
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
  //  bool           _inskin;
  //  SimpleObject2 *_bone;
  //  INode         *_node;
  EggGroup      *_egg_joint;
  MayaEggJoint   *_parent;
  vector <MayaEggJoint *> _children;

public:
  LVector3d GetXV(void) { return _trans.get_row3(0); }
  LVector3d GetYV(void) { return _trans.get_row3(1); }
  LVector3d GetZV(void) { return _trans.get_row3(2); }
  LVector3d GetPos(void) { return _trans.get_row3(3); }
  MayaEggJoint *ChooseBestChild(LVector3d dir);
  void ChooseEndPos(double thickness);
  void CreateMayaBone(CoordinateSystem sys);
};

MayaEggJoint *MayaEggLoader::FindJoint(EggGroup *joint)
{
  if (joint==0) return 0;
  return _joint_tab[joint];
}

MayaEggJoint *MayaEggLoader::MakeJoint(EggGroup *joint, EggGroup *context)
{
  MayaEggJoint *parent = FindJoint(context);
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
  //  result->_inskin = false;
  //  result->_bone = 0;
  //  result->_node = 0;
  result->_egg_joint = joint;
  result->_parent = parent;
  if (parent) parent->_children.push_back(result);
  _joint_tab[joint] = result;
  return result;
}

MayaEggJoint *MayaEggJoint::ChooseBestChild(LVector3d dir)
{
  if (dir.length() < 0.001) return 0;
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
  if (firstbest > secondbest + 0.1)
    return firstchild;
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
  if (altaxis.length() < 0.001) altaxis = orient.cross(LVector3d(0,0,1));
  _perp = altaxis.cross(orient);
  _perp.normalize();
}

void MayaEggJoint::CreateMayaBone(CoordinateSystem sys)
{
  //  Point3 xv(ConvertCoordSys(sys, GetXV()));
  //  Point3 yv(ConvertCoordSys(sys, GetYV()));
  //  Point3 zv(ConvertCoordSys(sys, GetZV()));
  //  Point3 pos(ConvertCoordSys(sys, GetPos()));
  //  Point3 endpos(ConvertCoordSys(sys, _endpos));
  //  Point3 tzv(ConvertCoordSys(sys, _perp));
  //  
  //  Point3 fwd = endpos - pos;
  //  double len = fwd.Length();
  //  Point3 txv = fwd * ((float)(1.0/len));
  //  Point3 tyv = tzv ^ txv;
  //  Point3 row1 = Point3(txv % xv, txv % yv, txv % zv);
  //  Point3 row2 = Point3(tyv % xv, tyv % yv, tyv % zv);
  //  Point3 row3 = Point3(tzv % xv, tzv % yv, tzv % zv);
  //  Matrix3 oomat(row1,row2,row3,Point3(0,0,0));
  //  Quat ooquat(oomat);
  //  _bone = (SimpleObject2*)CreateInstance(GEOMOBJECT_CLASS_ID, BONE_OBJ_CLASSID);
  //  _node = GetCOREInterface()->CreateObjectNode(_bone);
  //  _node->SetNodeTM(0, Matrix3(xv, yv, zv, pos));
  //  IParamBlock2 *blk = _bone->pblock2;
  //  for (int i=0; i<blk->NumParams(); i++) {
  //    TSTR n = blk->GetLocalName(i);
  //    if      (strcmp(n, "Length")==0) blk->SetValue(i,0,(float)len); 
  //    else if (strcmp(n, "Width")==0)  blk->SetValue(i,0,(float)_thickness);
  //    else if (strcmp(n, "Height")==0) blk->SetValue(i,0,(float)_thickness);
  //  }
  //  Point3 boneColor = GetUIColor(COLOR_BONES);
  //  _node->SetWireColor(RGB(int(boneColor.x*255.0f), int(boneColor.y*255.0f), int(boneColor.z*255.0f) ));
  //  _node->SetBoneNodeOnOff(TRUE, 0);
  //  _node->SetRenderable(FALSE);
  //  _node->SetName((TCHAR*)(_egg_joint->get_name().c_str()));
  //  _node->SetObjOffsetRot(ooquat);
  //  if (_parent) {
  //    _node->Detach(0, 1);
  //    _parent->_node->AttachChild(_node, 1);
  //  }
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
    if (n < 0) return true;
    if (n > 0) return false;
    n = k1._normal.compare_to(k2._normal);
    if (n < 0) return true;
    if (n > 0) return false;
    n = k1._weights.size() - k2._weights.size();
    if (n < 0) return true;
    if (n > 0) return false;
    for (unsigned int i=0; i<k1._weights.size(); i++) {
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

typedef phash_set<MayaEggVertex, MEV_Compare> VertTable;
typedef phash_map<TexCoordd, int>             TVertTable;
typedef phash_map<Colorf, int>                CVertTable;

class MayaEggMesh
{
public:
  
  string              _name;
  MFloatPointArray    _vertexArray;
  MIntArray           _polygonCounts;
  MIntArray           _polygonConnects;
  MFloatArray         _uarray;
  MFloatArray         _varray;
  MIntArray           _uvIds;
  MObject             _transNode;
  MObject             _shapeNode;
  MDagPath            _shape_dag_path;
  int                 _vert_count;
  int                 _tvert_count;
  int                 _cvert_count;
  int                 _face_count;
  vector<MayaEggTex*> _face_tex;
  
  VertTable  _vert_tab;
  TVertTable _tvert_tab;
  CVertTable _cvert_tab;
  
  int GetVert(EggVertex *vert, EggGroup *context, CoordinateSystem sys);
  int GetTVert(TexCoordd uv);
  int GetCVert(Colorf col);
  int AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2, MayaEggTex *tex);
  EggGroup *GetControlJoint(void);
  void ConnectTextures(void);
};

#define CTRLJOINT_DEFORM ((EggGroup*)((char*)(-1)))

int MayaEggMesh::GetVert(EggVertex *vert, EggGroup *context, CoordinateSystem sys)
{
  MayaEggVertex vtx;
  vtx._pos = vert->get_pos3();
  vtx._normal = vert->get_normal();
  vtx._index = 0;

  EggVertex::GroupRef::const_iterator gri;
  for (gri = vert->gref_begin(); gri != vert->gref_end(); ++gri) {
    EggGroup *egg_joint = (*gri);
    double membership = egg_joint->get_vertex_membership(vert);
    vtx._weights.push_back(MayaEggWeight(membership, egg_joint));
  }
  if (vtx._weights.size()==0) {
    if (context != 0)
      vtx._weights.push_back(MayaEggWeight(1.0, context));
  }
  
  VertTable::const_iterator vti = _vert_tab.find(vtx);
  if (vti != _vert_tab.end())
    return vti->_index;
  
  vtx._index = _vert_count++;
  _vertexArray.append(ConvertCoordSys(sys, vtx._pos));
  _vert_tab.insert(vtx);
  return vtx._index;
}

int MayaEggMesh::GetTVert(TexCoordd uv)
{
  if (_tvert_tab.count(uv))
    return _tvert_tab[uv];
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

MayaEggMesh *MayaEggLoader::GetMesh(EggVertexPool *pool)
{
  MayaEggMesh *result = _mesh_tab[pool];
  if (result == 0) {
    string name = pool->get_name();
    int nsize = name.size();
    if ((nsize > 6) && (name.rfind(".verts")==(nsize-6)))
      name.resize(nsize-6);
    result = new MayaEggMesh;
    result->_name = name;
    result->_vert_count = 0;
    result->_tvert_count = 0;
    result->_cvert_count = 0;
    result->_face_count = 0;
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
  for (int i=1; i<_face_count; i++)
    if (_face_tex[i] != _face_tex[0])
      subtex = true;
  if (!subtex) {
    MFnSet sg(_face_tex[0]->_shading_group);
    sg.addMember(_shapeNode);
    return;
  }
  for (int i=0; i<_face_count; i++) {
    MayaEggTex *tex = _face_tex[i];
    if (tex->_component.object()==MObject::kNullObj)
      tex->_component.create(MFn::kMeshPolygonComponent);
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

void MayaEggLoader::CreateSkinModifier(MayaEggMesh *M)
{
  //  vector <MayaEggJoint *> joints;
  //
  //  M->_dobj = CreateDerivedObject(M->_obj);
  //  M->_node->SetObjectRef(M->_dobj);
  //  M->_skin_mod = (Modifier*)CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);
  //  M->_iskin = (ISkin*)M->_skin_mod->GetInterface(I_SKIN);
  //  M->_iskin_import = (ISkinImportData*)M->_skin_mod->GetInterface(I_SKINIMPORTDATA);
  //  M->_dobj->SetAFlag(A_LOCK_TARGET);
  //  M->_dobj->AddModifier(M->_skin_mod);
  //  M->_dobj->ClearAFlag(A_LOCK_TARGET);
  //  GetCOREInterface()->ForceCompleteRedraw();
  //
  //  VertTable::const_iterator vert;
  //  for (vert=M->_vert_tab.begin(); vert != M->_vert_tab.end(); ++vert) {
  //    for (int i=0; i<vert->_weights.size(); i++) {
  //      double strength = vert->_weights[i].first;
  //      MayaEggJoint *joint = FindJoint(vert->_weights[i].second);
  //      if (!joint->_inskin) {
  //        joint->_inskin = true;
  //        joints.push_back(joint);
  //      }
  //    }
  //  }
  //  for (int i=0; i<joints.size(); i++) {
  //    BOOL last = (i == (joints.size()-1)) ? TRUE : FALSE;
  //    M->_iskin_import->AddBoneEx(joints[i]->_node, last);
  //    joints[i]->_inskin = false;
  //  }
  //
  //  GetCOREInterface()->SetCommandPanelTaskMode(TASK_MODE_MODIFY);
  //  GetCOREInterface()->SelectNode(M->_node);
  //  GetCOREInterface()->ForceCompleteRedraw();
  //
  //  for (vert=M->_vert_tab.begin(); vert != M->_vert_tab.end(); ++vert) {
  //    Tab<INode*> mayaJoints;
  //    Tab<float> mayaWeights;
  //    mayaJoints.ZeroCount();
  //    mayaWeights.ZeroCount();
  //    for (int i=0; i<vert->_weights.size(); i++) {
  //      float strength = (float)(vert->_weights[i].first);
  //      MayaEggJoint *joint = FindJoint(vert->_weights[i].second);
  //      mayaWeights.Append(1,&strength);
  //      mayaJoints.Append(1,&(joint->_node));
  //    }
  //    M->_iskin_import->AddWeights(M->_node, vert->_index, mayaJoints, mayaWeights);
  //  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TraverseEggData
//
// We have an EggData in memory, and now we're going to copy that
// over into the maya scene graph.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void MayaEggLoader::TraverseEggNode(EggNode *node, EggGroup *context)
{
  vector<int> vertIndices;
  vector<int> tvertIndices;
  vector<int> cvertIndices;
  
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
    MayaEggMesh *mesh = GetMesh(poly->get_pool());
    vertIndices.clear();
    tvertIndices.clear();
    cvertIndices.clear();
    for (ci = poly->begin(); ci != poly->end(); ++ci) {
      EggVertex *vtx = (*ci);
      EggVertexPool *pool = poly->get_pool();
      TexCoordd uv = vtx->get_uv();
      vertIndices.push_back(mesh->GetVert(vtx, context, _coord_sys));
      tvertIndices.push_back(mesh->GetTVert(uv * uvtrans));
      cvertIndices.push_back(mesh->GetCVert(vtx->get_color()));
    }
    for (unsigned int i=1; i<vertIndices.size()-1; i++)
      mesh->AddFace(vertIndices[0], vertIndices[i], vertIndices[i+1],
                    tvertIndices[0], tvertIndices[i], tvertIndices[i+1],
                    cvertIndices[0], cvertIndices[i], cvertIndices[i+1],
                    tex);
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

bool MayaEggLoader::ConvertEggData(EggData *data, bool merge, bool model, bool anim)
{
  if (!merge) {
    mayaloader_cat.error() << "Currently, only 'merge' mode is implemented.\n";
    return false;
  }
  
  if ((anim) || (!model)) {
    mayaloader_cat.error() << "Currently, only model-loading is implemented.\n";
    return false;
  }

  MeshIterator ci;
  JointIterator ji;
  TexIterator ti;

  _coord_sys = data->get_coordinate_system();

  TraverseEggNode(data, NULL);

  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MayaEggMesh *mesh = (*ci);
    if (mesh->_face_count==0) continue;

    MStatus status;
    MFnMesh mfn;
    MString cset;
    
    mfn.setName(mesh->_name.c_str());
    mesh->_transNode = mfn.create(mesh->_vert_count, mesh->_face_count,
                                  mesh->_vertexArray, mesh->_polygonCounts, mesh->_polygonConnects,
                                  //                             mesh->_uarray, mesh->_varray,
                                  MObject::kNullObj, &status);
    mesh->_shapeNode = mfn.object();
    MFnDependencyNode mdn(mesh->_transNode);
    mfn.getPath(mesh->_shape_dag_path);
    mfn.setName(MString(mesh->_name.c_str())+"Shape");
    mdn.setName(MString(mesh->_name.c_str()));
    mesh->ConnectTextures();
    
    //    mfn.getCurrentUVSetName(cset);
    //    mfn.assignUVs(mesh->_polygonCounts, mesh->_uvIds, &cset); 
  }
  
  double thickness = 0.0;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    double dfo = ((*ji)->GetPos()).length();
    if (dfo > thickness) thickness = dfo;
  }
  thickness = thickness * 0.025;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    MayaEggJoint *joint = *ji;
    joint->ChooseEndPos(thickness);
    joint->CreateMayaBone(_coord_sys);
  }
  
  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MayaEggMesh *mesh = (*ci);
    EggGroup *joint = mesh->GetControlJoint();
    if (joint) CreateSkinModifier(mesh);
  }
  
  for (ci = _mesh_tab.begin();  ci != _mesh_tab.end();  ++ci) delete *ci;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) delete *ji;
  for (ti = _tex_tab.begin();   ti != _tex_tab.end();   ++ti) delete *ti;
  
  //  ResumeSetKeyMode();
  //  ResumeAnimate();
  
  mayaloader_cat.info() << "Egg import successful\n";
  return true;
}

bool MayaEggLoader::ConvertEggFile(const char *name, bool merge, bool model, bool anim)
{
  EggData data;
  Filename datafn = Filename::from_os_specific(name);
  if (!data.read(datafn)) {
    mayaloader_cat.error() << "Cannot read Egg file for import\n";
    return false;
  }
  return ConvertEggData(&data, merge, model, anim);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The two global functions that form the API of this module.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool MayaLoadEggData(EggData *data, bool merge, bool model, bool anim)
{
  MayaEggLoader loader;
  return loader.ConvertEggData(data, merge, model, anim);
}

bool MayaLoadEggFile(const char *name, bool merge, bool model, bool anim)
{
  MayaEggLoader loader;
  return loader.ConvertEggFile(name, merge, model, anim);
}


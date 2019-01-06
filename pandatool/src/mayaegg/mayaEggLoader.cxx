/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaEggLoader.cxx
 * @author jyelon
 * @date 2005-07-20
 *
 * This file contains the code for class MayaEggLoader.  This class
 * does the actual work of copying an EggData tree into the maya scene.
 */

#include "pandatoolbase.h"
#include "notifyCategoryProxy.h"

#include "eggBin.h"
#include "eggData.h"
#include "eggTable.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "eggComment.h"
#include "eggXfmSAnim.h"
#include "eggSAnimData.h"
#include "eggPrimitive.h"
#include "eggGroupNode.h"
#include "eggVertexPool.h"
#include "eggPolysetMaker.h"
#include "eggNurbsSurface.h"
#include "texture.h"
#include "texturePool.h"

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
#include <maya/MPointArray.h>
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
#include <maya/MFnDoubleIndexedComponent.h>
#include <maya/MPlugArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MAnimControl.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnSet.h>
#include "post_maya_include.h"

#include "mayaEggLoader.h"

using std::cerr;
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;

class MayaEggGroup;
class MayaEggGeom;
class MayaEggMesh;
class MayaEggJoint;
class MayaEggTex;
class MayaAnim;
class MayaEggNurbsSurface;

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
  MayaEggTex   *GetTex(EggTexture *etex);
  void          CreateSkinCluster(MayaEggGeom *M);

  MayaAnim *GetAnim(EggXfmSAnim *pool);
  MObject GetDependencyNode(string givenName);

  MayaEggNurbsSurface  *GetSurface(EggVertexPool *pool, EggGroup *parent);

  typedef phash_map<EggGroup *, MayaEggMesh *, pointer_hash> MeshTable;
  typedef phash_map<EggXfmSAnim *, MayaAnim *, pointer_hash> AnimTable;
  typedef phash_map<EggGroup *, MayaEggJoint *, pointer_hash> JointTable;
  typedef phash_map<EggGroup *, MayaEggGroup *, pointer_hash> GroupTable;
  typedef phash_map<string, MayaEggTex *, string_hash> TexTable;
  typedef phash_map<EggGroup *, MayaEggNurbsSurface *, pointer_hash> SurfaceTable;

  MeshTable        _mesh_tab;
  AnimTable        _anim_tab;
  JointTable       _joint_tab;
  GroupTable       _group_tab;
  TexTable         _tex_tab;
  SurfaceTable     _surface_tab;

  vector <MayaEggJoint *> _joint_list;

  int _start_frame;
  int _end_frame;
  int _frame_rate;
  MTime::Unit     _timeUnit;

  void ParseFrameInfo(string comment);
  void PrintData(MayaEggMesh *mesh);

private:
  int _unnamed_idx;
  MSelectionList _collision_nodes;
};

MPoint MakeMPoint(const LVector3d &vec)
{
  return MPoint(vec[0], vec[1], vec[2]);
}

MFloatPoint MakeMayaPoint(const LVector3d &vec)
{
  return MFloatPoint(vec[0], vec[1], vec[2]);
}

MVector MakeMayaVector(const LVector3d &vec)
{
  return MVector(vec[0], vec[1], vec[2]);
}

MColor MakeMayaColor(const LColor &vec)
{
  return MColor(vec[0], vec[1], vec[2], vec[3]);
}

// [gjeon] to create enum attribute, fieldNames is a stringArray of enum
// names, and filedIndex is the default index value
MStatus create_enum_attribute(MObject &node, MString fullName, MString briefName,
                              MStringArray fieldNames, unsigned fieldIndex) {
  MStatus stat;

  MFnDependencyNode fnDN( node, &stat );
  if ( MS::kSuccess != stat ) {
    mayaloader_cat.error()
      << "Could not create MFnDependencyNode" << "\n";
    return stat;
  }

  MFnEnumAttribute fnAttr;
  MObject newAttr = fnAttr.create( fullName, briefName,
                                   0, &stat );
  if ( MS::kSuccess != stat ) {
    mayaloader_cat.error()
      << "Could not create new enum attribute " << fullName.asChar() << "\n";
    return stat;
  }
  for (unsigned i = 0; i < fieldNames.length(); i++){
    fnAttr.addField(fieldNames[i], i);
  }

  stat = fnAttr.setDefault(fieldIndex);
  if ( MS::kSuccess != stat ) {
    mayaloader_cat.error()
      << "Could not set value for enum attribute " << fullName.asChar() << "\n";
    return stat;
  }

  fnAttr.setKeyable( true );
  fnAttr.setReadable( true );
  fnAttr.setWritable( true );
  fnAttr.setStorable( true );

  // Now add the new attribute to this dependency node
  stat = fnDN.addAttribute(newAttr, MFnDependencyNode::kLocalDynamicAttr);
  if ( MS::kSuccess != stat ) {
    mayaloader_cat.error()
      << "Could not add new enum attribute " << fullName.asChar() << "\n";
    return stat;
  }

  return stat;
}

// MayaEggTex

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

MayaEggTex *MayaEggLoader::GetTex(EggTexture* etex)
{
  string name = "";
  string fn = "";
  if (etex != nullptr) {
    name = etex->get_name();
    fn = etex->get_fullpath().to_os_specific();
  }

  if (_tex_tab.count(fn)) {
    return _tex_tab[fn];
  }

  MStatus status;
  MFnLambertShader shader;
  MFnDependencyNode filetex;
  MFnSet sgroup;
  MPlugArray oldplugs;
  MDGModifier dgmod;

  /*
  if (fn=="") {
    MSelectionList selection;
    MObject initGroup;
    selection.clear();
    MGlobal::getSelectionListByName("initialShadingGroup",selection);
    selection.getDependNode(0, initGroup);
    sgroup.setObject(initGroup);
  } else {
  */
  if (1) {
    shader.create(true,&status);
    MColor firstColor(1.0,1.0,1.0,1.0);
    status = shader.setColor(firstColor);
    if (status != MStatus::kSuccess) {
      mayaloader_cat.error() << "setColor failed on LambertShader\n";
      status.perror("shader setColor failed!");
    }
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

      // [gjeon] to create alpha channel connection
      LoaderOptions options;
      PT(Texture) tex = TexturePool::load_texture(etex->get_fullpath(), 0, false, options);
      if (((tex != nullptr) && (tex->get_num_components() == 4))
          || (etex->get_format() == EggTexture::F_alpha)
          || (etex->get_format() == EggTexture::F_luminance_alpha))
        dgmod.connect(filetex.findPlug("outTransparency"),shader.findPlug("transparency"));
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

// MayaEggGroup

class MayaEggGroup
{
public:
  string  _name;
  MObject _parent;
  MObject _group;

  bool _addedEggFlag;
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
    if (mayaloader_cat.is_debug()) {
      mayaloader_cat.debug() << "parent (group) :" << ((MFnDagNode)parent).name().asChar() << endl;
    }
  }

  result->_name = group->get_name();
  result->_group = dgn.create("transform", MString(result->_name.c_str()), parent, &status);
  result->_addedEggFlag = false;

  if (group->get_cs_type() != EggGroup::CST_none)
    _collision_nodes.add(result->_group, true);

  if (group->has_transform3d()) {
    LMatrix4d tMat = group->get_transform3d();
    double matData[4][4] = {{tMat.get_cell(0,0), tMat.get_cell(0,1), tMat.get_cell(0,2), tMat.get_cell(0,3)},
                  {tMat.get_cell(1,0), tMat.get_cell(1,1), tMat.get_cell(1,2), tMat.get_cell(1,3)},
                  {tMat.get_cell(2,0), tMat.get_cell(2,1), tMat.get_cell(2,2), tMat.get_cell(2,3)},
                  {tMat.get_cell(3,0), tMat.get_cell(3,1), tMat.get_cell(3,2), tMat.get_cell(3,3)}};
    MMatrix mat(matData);

    MTransformationMatrix matrix = MTransformationMatrix(mat);
    MFnTransform tFn(result->_group, &status);
    if (status != MStatus::kSuccess) {
      status.perror("MFnTransformNode:create failed!");
    } else {
      tFn.set(matrix);
    }
  }

  if (status != MStatus::kSuccess) {
    status.perror("MFnDagNode:create failed!");
  }

  if ((pg) && (pg->_addedEggFlag == false)){
    // [gjeon] to handle other flags
    MStringArray eggFlags;
    for (int i = 0; i < context->get_num_object_types(); i++) {
      eggFlags.append(MString(context->get_object_type(i).c_str()));
    }

    for (unsigned i = 0; i < eggFlags.length(); i++) {
      MString attrName = "eggObjectTypes";
      attrName += (int)(i + 1);
      status = create_enum_attribute(parent, attrName, attrName, eggFlags, i);
      if (status != MStatus::kSuccess) {
        status.perror("create_enum_attribute failed!");
      }
    }
    pg->_addedEggFlag = true;
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

// MayaEggJoint

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
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "joint " << joint.name().asChar() << ": -> " << name << endl;
  }
}

MayaEggJoint *MayaEggLoader::FindJoint(EggGroup *joint)
{
  if (joint==nullptr) {
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "joint:" << joint->get_name() << " is null: " << endl;
    }
    return 0;
  }
  if (!joint->is_joint()) {
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "joint:" << joint->get_name() << " is not a joint: " << endl;
    }
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

  // [gjeon] since _joint_tab is not always properly sorted
  _joint_list.push_back(result);

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
  // mayaloader_cat.debug() << "fwd : " << fwd << endl;
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
  // GetRotation(rxv, ryv, rzv); [gjeon] I think we shouldn't need to use this
  // GetRotation function here since this function removes scale information
  // from the matrix.  Let's just use the matrix directly.
  rxv = _trans.get_row3(0);
  ryv = _trans.get_row3(1);
  rzv = _trans.get_row3(2);

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


// MayaEggGeom : base abstract class of MayaEggMesh and MayaEggNurbsSurface

typedef std::pair<double, EggGroup *> MayaEggWeight;

struct MayaEggVertex
{
  LVertexd               _pos;
  LNormald               _normal;
  LTexCoordd             _uv;
  vector<MayaEggWeight> _weights;
  double                _sumWeights; // [gjeon] to be used in normalizing weights
  int                   _index;
  int                   _external_index; // masad: use egg's index directly
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
    n = k1._uv.compare_to(k2._uv);
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
    n = k1._external_index - k2._external_index;

    if (n < 0) {
      return true;
    }
    if (n > 0) {
      return false;
    }

    return false;
  }
};

typedef phash_set<MayaEggVertex, MEV_Compare> VertTable;

class MayaEggGeom
{
public:

  EggVertexPool      *_pool;
  MObject             _transNode;
  MObject             _shapeNode;
  EggGroup            *_parent;
  MDagPath            _shape_dag_path;
  int                 _vert_count;

  string _name;

  MFloatPointArray    _vertexArray;
  MVectorArray        _normalArray;
  MColorArray         _vertColorArray;
  MIntArray           _vertColorIndices;
  MIntArray           _vertNormalIndices;

  MStringArray        _eggObjectTypes;
  VertTable  _vert_tab;

  bool                _renameTrans;

  int GetVert(EggVertex *vert, EggGroup *context);
  EggGroup *GetControlJoint(void);

  virtual void ConnectTextures(void) = 0;
  void AssignNames(void);
  void AddEggFlag(MString);
};

// [gjeon] moved from MayaEggMesh to MayaEggGeom
int MayaEggGeom::GetVert(EggVertex *vert, EggGroup *context)
{
  MayaEggVertex vtx;
  vtx._sumWeights = 0.0;

  const LMatrix4d &xform = context->get_vertex_to_node();

  vtx._pos = vert->get_pos3() * xform;
  if (vert->has_normal()) {
    vtx._normal = vert->get_normal() * xform;
  }
  if (vert->has_uv()) {
    vtx._uv = vert->get_uv();
  }
  vtx._index = 0;
  vtx._external_index = vert->get_index()-1;

  EggVertex::GroupRef::const_iterator gri;
  // double remaining_weight = 1.0;
  for (gri = vert->gref_begin(); gri != vert->gref_end(); ++gri) {
    EggGroup *egg_joint = (*gri);
    double membership = egg_joint->get_vertex_membership(vert);

    if (membership < 0)
    {
      mayaloader_cat.warning() << "negative weight value " << membership << " is replaced with 0 on: " << context->get_name() << endl;
      membership = 0.0;
    }
    // remaining_weight -= membership;
    vtx._weights.push_back(MayaEggWeight(membership, egg_joint));
    vtx._sumWeights += membership; // [gjeon] to be used in normalizing weights
  }

  if (vtx._weights.size()==0) {
    if (context != 0) {
      vtx._weights.push_back(MayaEggWeight(1.0, context));
      vtx._sumWeights = 1.0; // [gjeon] to be used in normalizing weights
    }
    // remaining_weight = 0.0;
  }/* else {
    // some soft models came up short of 1.0 on vertex membership add the
    // remainder of the weight on first joint in the membership
        if ((remaining_weight) > 0.01) {
      gri = vert->gref_begin();
      EggGroup *egg_joint = (*gri);
      double membership = egg_joint->get_vertex_membership(vert);
      vtx._weights.push_back(MayaEggWeight(membership+remaining_weight, egg_joint));
      vtx._sumWeights += (membership + remaining_weight);
    }
    }*/ //[gjeon] we had better nomarlize weights than add remaining weight to first weight

  VertTable::const_iterator vti = _vert_tab.find(vtx);
  if (vti != _vert_tab.end()) {
    /*    if ((remaining_weight) > 0.01) {
      mayaloader_cat.warning() << "weight munged to 1.0 by " << remaining_weight << " on: " << context->get_name() << " idx:" << vti->_index << endl;
      } */
    if (mayaloader_cat.is_spam()) {
      ostringstream stream;
      stream << "(" << vti->_pos << " " << vti->_normal << " " << vti->_uv << ")\n";
      stream << "[" << vtx._pos << " " << vtx._normal << " " << vtx._uv << "]\n";
      stream << "{" << vert->get_pos3() << " ";
      if (vert->has_normal()) {
        stream << vert->get_normal() << " ";
      }
      if (vert->has_uv()) {
        stream << vert->get_uv();
      }
      stream << "}";
      mayaloader_cat.spam() << "found a matching vertex: " << *vert << endl << stream.str() << endl;
    }
    return vti->_index;
  }

  // _vert_count++;
  vtx._index = _vert_count++;
  /*
  if ((remaining_weight) > 0.01) {
    mayaloader_cat.warning() << "weight munged to 1.0 by " << remaining_weight << " on: " << context->get_name() << " idx:" << vtx._index << endl;
    } */

  _vertexArray.append(MakeMayaPoint(vtx._pos));
  if (vert->has_normal()) {
    _normalArray.append(MakeMayaVector(vtx._normal));
    _vertNormalIndices.append(vtx._index);
  }
  if (vert->has_color()) {
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "found a vertex color\n";
    }
    _vertColorArray.append(MakeMayaColor(vert->get_color()));
    _vertColorIndices.append(vtx._index);
  }
  _vert_tab.insert(vtx);
  return vtx._index;
}

// [gjeon] moved from MayaEggMesh to MayaEggGeom
void MayaEggGeom::AssignNames(void)
{
  string name = _pool->get_name();
  size_t nsize = name.size();
  if (nsize > 6 && name.rfind(".verts") == (nsize - 6)) {
    name.resize(nsize - 6);
  }
  if (nsize > 4 && name.rfind(".cvs") == (nsize - 4)) {
    name.resize(nsize - 4);
  }

  MFnDependencyNode dnshape(_shapeNode);
  MFnDependencyNode dntrans(_transNode);

  if (_renameTrans) {
    dntrans.setName(MString(name.c_str()));
  }

  string shape_name = string(dntrans.name().asChar());
  string numbers ("0123456789");
  size_t found;

  found=shape_name.find_last_not_of(numbers);
  if (found!=string::npos)
    shape_name.insert(found+1, "Shape");
  else
    shape_name.append("Shape");

  dnshape.setName(MString(shape_name.c_str()));
}

#define CTRLJOINT_DEFORM ((EggGroup*)((char*)(-1)))

// [gjeon] moved from MayaEggMesh to MayaEggGeom
EggGroup *MayaEggGeom::GetControlJoint(void)
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

void MayaEggGeom::AddEggFlag(MString fieldName) {
  bool addNewFlag = true;
  for (unsigned i = 0; i < _eggObjectTypes.length(); i++) {
    if (_eggObjectTypes[i] == fieldName) {
      addNewFlag = false;
      break;
    }
  }
  if (addNewFlag) {
    _eggObjectTypes.append(fieldName);
  }
}

// MayaEggMesh
typedef phash_map<LTexCoordd, int>             TVertTable;
typedef phash_map<LColor, int>                CVertTable;

class MayaEggMesh final : public MayaEggGeom
{
public:
  MColorArray         _faceColorArray;
  MIntArray           _faceIndices;
  MIntArray           _polygonCounts;
  MIntArray           _polygonConnects;
  MFloatArray         _uarray;
  MFloatArray         _varray;
  MIntArray           _uvIds;


  int                 _tvert_count;
  int                 _cvert_count;
  int                 _face_count;
  vector<MayaEggTex*> _face_tex;

  TVertTable _tvert_tab;
  CVertTable _cvert_tab;

  int GetTVert(const LTexCoordd &uv);
  int GetCVert(const LColor &col);
  int AddFace(unsigned numVertices, MIntArray mvertIndices, MIntArray mtvertIndices, MayaEggTex *tex);

  void ConnectTextures(void) override;
};

int MayaEggMesh::GetTVert(const LTexCoordd &uv)
{
  if (_tvert_tab.count(uv)) {
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "found uv coords idx: " << _tvert_tab[uv] << endl;
    }
    return _tvert_tab[uv];
  }
  int idx = _tvert_count++;
  _uarray.append(uv.get_x());
  _varray.append(uv.get_y());
  _tvert_tab[uv] = idx;
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "adding uv coords idx:" << idx << endl;
  }
  return idx;
}

int MayaEggMesh::GetCVert(const LColor &col)
{
/*
 * if (_cvert_tab.count(col)) return _cvert_tab[col]; if (_cvert_count ==
 * _mesh->numCVerts) { int nsize = _cvert_count*2 + 100;
 * _mesh->setNumVertCol(nsize, _cvert_count?TRUE:FALSE); } int idx =
 * _cvert_count++; _mesh->vertCol[idx] = Point3(col.get_x(), col.get_y(),
 * col.get_z()); _cvert_tab[col] = idx; return idx;
 */
  return 0;
}

MayaEggMesh *MayaEggLoader::GetMesh(EggVertexPool *pool, EggGroup *parent)
{
  MayaEggMesh *result = _mesh_tab[parent];
  if (result == 0) {
    result = new MayaEggMesh;
    if (parent != nullptr) {
      result->_name = parent->get_name();
    }
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
    result->_eggObjectTypes.clear();
    result->_renameTrans = false;
    _mesh_tab[parent] = result;
  }
  return result;
}

int MayaEggMesh::AddFace(unsigned numVertices, MIntArray mvertIndices, MIntArray mtvertIndices, MayaEggTex *tex)
{
  int idx = _face_count++;
  _polygonCounts.append(numVertices);
  for (unsigned i = 0; i < mvertIndices.length(); i++)
  {
    _polygonConnects.append(mvertIndices[i]);
    _uvIds.append(mtvertIndices[i]);
  }
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


// MayaEggNurbsSurface
class MayaEggNurbsSurface : public MayaEggGeom
{
public:


  MPointArray         _cvArray;
  MDoubleArray        _uKnotArray;
  MDoubleArray        _vKnotArray;
  unsigned            _uDegree;
  unsigned            _vDegree;
  unsigned            _uNumCvs;
  unsigned            _vNumCvs;

  MFnNurbsSurface::Form _uForm;
  MFnNurbsSurface::Form _vForm;

  MayaEggTex          *_tex;

  void ConnectTextures(void);
  void PrintData(void);
};

MayaEggNurbsSurface *MayaEggLoader::GetSurface(EggVertexPool *pool, EggGroup *parent)
{
  MayaEggNurbsSurface *result = _surface_tab[parent];
  if (result == 0) {
    result = new MayaEggNurbsSurface;
    result->_pool = pool;
    result->_parent = parent;
    result->_name = parent->get_name();

    result->_vert_count = 0;
    result->_vertColorArray.clear();
    result->_vertNormalIndices.clear();
    result->_vertColorIndices.clear();

    result->_cvArray.clear();
    result->_uKnotArray.clear();
    result->_vKnotArray.clear();

    result->_uDegree = 0;
    result->_vDegree = 0;
    result->_uNumCvs = 0;
    result->_vNumCvs = 0;
    result->_uForm = MFnNurbsSurface::kClosed;
    result->_vForm = MFnNurbsSurface::kClosed;

    result->_eggObjectTypes.clear();
    result->_renameTrans = false;
    _surface_tab[parent] = result;
  }
  return result;
}

void MayaEggNurbsSurface::ConnectTextures(void)
{
  // masad: since nurbs surfaces do not support vertex colors I am infusing
  // the surface's first vertex color (if any) into the shader to achive the
  // color.  masad: check if there is any vertex color for this surface
  MStatus status;
  MColor firstColor(0.5,0.5,0.5,1.0);
  if (_vertColorArray.length() > 0) {
    firstColor = _vertColorArray[0];
    MFnLambertShader sh(_tex->_shader);
    status = sh.setColor(firstColor);
    if (status != MStatus::kSuccess) {
      mayaloader_cat.error() << "setColor failed on " << _name;
      status.perror("shader setColor failed!");
    }
  }
  MFnSet sg(_tex->_shading_group);
  status = sg.addMember(_shapeNode);
  if (status != MStatus::kSuccess) {
    mayaloader_cat.error() << "addMember failed on " << _name;
    status.perror("shader addMember failed!");
  }
  return;
}

void MayaEggNurbsSurface::PrintData(void)
{
  if (mayaloader_cat.is_debug()) {
    mayaloader_cat.debug() << "nurbsSurface : " << _name << endl;

    mayaloader_cat.debug() << "u_form : " << _uForm << endl;
    mayaloader_cat.debug() << "v_form : " << _vForm << endl;
  }

  /*
  for (unsigned i = 0; i < _cvArray.length(); i++)
  {
    MPoint cv =_cvArray[i];
    mayaloader_cat.debug() << cv[0] << " " << cv[1] << " " << cv[2] << endl;
  }

  for (unsigned i = 0; i < _uKnotArray.length(); i++)
  {
    mayaloader_cat.debug() << _uKnotArray[i] << endl;
  }

  for (unsigned i = 0; i < _vKnotArray.length(); i++)
  {
    mayaloader_cat.debug() << _vKnotArray[i] << endl;
  }
  */
}

// MayaAnim:
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

// MayaEggLoader functions

void MayaEggLoader::CreateSkinCluster(MayaEggGeom *M)
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
      MayaEggJoint *joint = FindJoint(vert->_weights[i].second);
      if (joint && !joint->_inskin) {
        joint->_inskin = true;
        joint->_index = joints.size();
        joints.push_back(joint);
        /*
        if (mayaloader_cat.is_spam()) {
          mayaloader_cat.spam() << joints[i]->_egg_joint->get_name() << ": adding to skin\n";
        }
        */
      }
    }
  }
  cmd += maxInfluences;

  /*
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << joints.size() << " joints have weights on " << M->_pool->get_name() << endl;
  }
  */
  if (joints.size() == 0) {
    // no need to cluster; there are no weights
    return;
  }

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
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << cmd.asChar() << endl;
    string spamCmd = M->_pool->get_name();
    for (unsigned int i=0; i<joints.size(); i++) {
      spamCmd = spamCmd + " ";
      spamCmd = spamCmd + joints[i]->_egg_joint->get_name();
    }
    mayaloader_cat.spam() << spamCmd << ": total = " << joints.size() << endl;
  }
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
  MPlug inPlug;
  if (shape.typeName() == "mesh") {
    inPlug = shape.findPlug("inMesh");
  } else if (shape.typeName() == "nurbsSurface") {
    inPlug = shape.findPlug("create");
  } else {
    // we only support mesh and nurbsSurface
    return;
  }

  if ((!inPlug.connectedTo(oldplugs,true,false))||(oldplugs.length() != 1)) {
    cerr << "skinCluster command failed";
    return;
  }
  MFnSkinCluster skinCluster(oldplugs[0].node());
  MIntArray influenceIndices;
  MFnSingleIndexedComponent component;
  component.create(MFn::kMeshVertComponent); // [gjeon] Interestingly, we can use MFn::kMeshVertComponent for NURBS surface, too
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
    skinCluster.setWeights(M->_shape_dag_path, component.object(), index, 0.0, false, nullptr);
  }

  MFloatArray values;
  int tot = M->_vert_count * joints.size();
  values.setLength(tot);
  for (int i=0; i<tot; i++) {
    values[i] = 0.0;
  }
  for (vert=M->_vert_tab.begin(); vert != M->_vert_tab.end(); ++vert) {
    for (unsigned int i=0; i<vert->_weights.size(); i++) {
      double strength = vert->_weights[i].first / vert->_sumWeights; // [gjeon] nomalizing weights
      MayaEggJoint *joint = FindJoint(vert->_weights[i].second);
      values[vert->_index * joints.size() + joint->_index] = (PN_stdfloat)strength;
    }
  }
  skinCluster.setWeights(M->_shape_dag_path, component.object(), influenceIndices, values, false, nullptr);

  for (unsigned int i=0; i<joints.size(); i++) {
    /*
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << joints[i]->_egg_joint->get_name() << ": clearing skin\n";
    }
    */
    joints[i]->_inskin = false;
    joints[i]->_index = -1;
  }
}

// TraverseEggData We have an EggData in memory, and now we're going to copy
// that over into the maya scene graph.

void MayaEggLoader::TraverseEggNode(EggNode *node, EggGroup *context, string delim)
{
  vector<int> vertIndices;
  vector<int> tvertIndices;
  vector<int> cvertIndices;

  string delstring = " ";

  if (node->is_of_type(EggPolygon::get_class_type())) {
    /*
    if (mayaloader_cat.is_debug()) {
      mayaloader_cat.debug() << delim+delstring << "found an EggMesh: " << node->get_name() << endl;
    }
    */
    EggPolygon *poly = DCAST(EggPolygon, node);
    if (poly->empty()) {
      return;
    }
    poly->cleanup();

    MayaEggTex *tex = 0;
    LMatrix3d uvtrans = LMatrix3d::ident_mat();

    if (poly->has_texture()) {
      EggTexture *etex = poly->get_texture(0);
      if (mayaloader_cat.is_spam()) {
        mayaloader_cat.spam() << "Texture format : " << etex->get_format() << endl;
      }
      tex = GetTex(etex);
      if (etex->has_transform())
        uvtrans = etex->get_transform2d();
    } else {
      tex = GetTex(nullptr);
    }

    EggPolygon::const_iterator ci;
    MayaEggMesh *mesh = GetMesh(poly->get_pool(), context);
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "traverse mesh pointer " << mesh << "\n";
    }
    vertIndices.clear();
    tvertIndices.clear();
    cvertIndices.clear();
    int numVertices = 0;
    for (ci = poly->begin(); ci != poly->end(); ++ci) {
      EggVertex *vtx = (*ci);
      LTexCoordd uv(0,0);
      if (vtx->has_uv()) {
        uv = vtx->get_uv();
      }
      vertIndices.push_back(mesh->GetVert(vtx, context));
      tvertIndices.push_back(mesh->GetTVert(uv * uvtrans));
      cvertIndices.push_back(mesh->GetCVert(vtx->get_color()));
      numVertices++;
    }
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "num vertices: " << vertIndices.size() << "\n";
    }

    if (numVertices < 3)
      return;

    MIntArray mvertIndices;
    MIntArray mtvertIndices;
    for (int i = 0; i < numVertices; i++) {
      mvertIndices.append(vertIndices[i]);
      mtvertIndices.append(tvertIndices[i]);
    }
    if (poly->has_color()) {
      if (mayaloader_cat.is_spam()) {
          mayaloader_cat.spam() << "found a face color of " << poly->get_color() << endl;
      }
      mesh->_faceIndices.append(mesh->_face_count);
      mesh->_faceColorArray.append(MakeMayaColor(poly->get_color()));
    }
    mesh->AddFace(numVertices, mvertIndices, mtvertIndices, tex);

    // [gjeon] to handle double-sided flag
    if (poly->get_bface_flag()) {
      mesh->AddEggFlag("double-sided");
    }

    // [gjeon] to handle model flag
    if (context->get_model_flag()) {
      mesh->AddEggFlag("model");
    }

    // [gjeon] to handle billboard flag
    switch (context->get_billboard_type()) {
    case EggGroup::BT_axis:
      mesh->AddEggFlag("billboard");
      break;

    case EggGroup::BT_point_camera_relative:
      mesh->AddEggFlag("billboard-point");
      break;

    default:
      ;
    }

    // [gjeon] to handle other flags
    for (int i = 0; i < context->get_num_object_types(); i++) {
      mesh->AddEggFlag(MString(context->get_object_type(i).c_str()));
    }

  } else if (node->is_of_type(EggNurbsSurface::get_class_type())) {
    // [gjeon] to convert nurbsSurface
    EggNurbsSurface *eggNurbsSurface = DCAST(EggNurbsSurface, node);

    EggNurbsSurface::const_iterator ci;
    EggVertexPool *pool = eggNurbsSurface->get_pool();
    MayaEggNurbsSurface *surface = GetSurface(pool, context);

    for (ci = eggNurbsSurface->begin(); ci != eggNurbsSurface->end(); ++ci) {
      EggVertex *vtx = (*ci);
      surface->GetVert(vtx, context);
    }

    // [gjeon] finding textures
    MayaEggTex *tex = 0;
    LMatrix3d uvtrans = LMatrix3d::ident_mat();

    if (eggNurbsSurface->has_texture()) {
      EggTexture *etex = eggNurbsSurface->get_texture(0);
      tex = GetTex(etex);
      if (etex->has_transform())
      {
        mayaloader_cat.debug() << "uvtrans?" << endl;
        uvtrans = etex->get_transform2d();
      }
    } else {
      tex = GetTex(nullptr);
    }

    surface->_tex = tex;
    surface->_uNumCvs = eggNurbsSurface->get_num_u_cvs();
    surface->_vNumCvs = eggNurbsSurface->get_num_v_cvs();

    // [gjeon] building cvArray
    for (uint ui = 0; ui < surface->_uNumCvs; ui++) {
      for (uint vi = 0; vi < surface->_vNumCvs; vi++) {
        EggVertex *vtx = eggNurbsSurface->get_vertex(eggNurbsSurface->get_vertex_index(ui, vi));
        surface->_cvArray.append(MakeMPoint(vtx->get_pos3()));
      }
    }

    // [gjeon] building u knotArray
    for (int i = 1; i < eggNurbsSurface->get_num_u_knots()-1; i++) {
      surface->_uKnotArray.append(eggNurbsSurface->get_u_knot(i));
    }

    // [gjeon] building v knotArray
    for (int i = 1; i < eggNurbsSurface->get_num_v_knots()-1; i++) {
      surface->_vKnotArray.append(eggNurbsSurface->get_v_knot(i));
    }

    surface->_uDegree = eggNurbsSurface->get_u_degree();
    surface->_vDegree = eggNurbsSurface->get_v_degree();

    if (eggNurbsSurface->is_closed_u()) {
      surface->_uForm = MFnNurbsSurface::kClosed;
    } else {
      surface->_vForm = MFnNurbsSurface::kOpen;
    }

    if (eggNurbsSurface->is_closed_v()) {
      surface->_vForm = MFnNurbsSurface::kClosed;
    } else {
      surface->_vForm = MFnNurbsSurface::kOpen;
    }

    // [gjeon] to handle double-sided flag
    if (eggNurbsSurface->get_bface_flag()) {
      surface->AddEggFlag("double-sided");
    }

    // [gjeon] to handle model flag
    if (context->get_model_flag()) {
      surface->AddEggFlag("model");
    }

    // [gjeon] to handle other flags
    for (int i = 0; i < context->get_num_object_types(); i++) {
     surface->AddEggFlag(MString(context->get_object_type(i).c_str()));
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
    // EggSAnimData *anim = DCAST(EggSAnimData, node); MayaAnimData *animData
    // = GetAnimData(anim, DCAST(EggXfmSAnim, node->get_parent()));
    // animData->PrintData(); if (_end_frame <
    // animData->_pool->get_num_rows()) { _end_frame =
    // animData->_pool->get_num_rows(); }
  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, node);
    if (node->is_of_type(EggGroup::get_class_type())) {
      EggGroup *group = DCAST(EggGroup, node);

      if (group->get_name() == "") {
        ostringstream stream;
        stream << _unnamed_idx;
        group->set_name("unnamed" + stream.str());
        _unnamed_idx++;
      }

      string group_name = group->get_name();
      size_t found = group_name.find(":");
      if (found != string::npos)
        group->set_name(group_name.replace(int(found), 1, "_"));

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
      // EggTable *anim = DCAST(EggTable, node);
      if (mayaloader_cat.is_debug()) {
        mayaloader_cat.debug() << delim+delstring << "found an EggTable: " << node->get_name() << endl;
      }
    } else if (node->is_of_type(EggXfmSAnim::get_class_type())) {
      //MayaAnim *anim = GetAnim(DCAST(EggXfmSAnim, node));
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
  _unnamed_idx = 1;

  MeshTable::const_iterator ci;
  JointTable::const_iterator ji;
  TexTable::const_iterator ti;
  SurfaceTable::const_iterator si;
  AnimTable::const_iterator ei;

  if (MGlobal::isYAxisUp()) {
    data->set_coordinate_system(CS_yup_right);
  } else {
    data->set_coordinate_system(CS_zup_right);
  }

  if (mayaloader_cat.is_debug()) {
    mayaloader_cat.debug() << "root node: " << data->get_type() << endl;
  }
  TraverseEggNode(data, nullptr, "");

  MStatus status;

  MFnSet collision_set;
  collision_set.create(_collision_nodes, MFnSet::kNone, &status);

  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "num meshes : " << _mesh_tab.size() << endl;
  }
  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MayaEggMesh *mesh = (*ci).second;
    if (mesh->_face_count==0) {
      continue;
    }

    // MStatus status;
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
      mesh->_renameTrans = true;
      if (mayaloader_cat.is_debug()) {
        mayaloader_cat.debug() << "mesh's parent (null) : " << endl;
      }
    }
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "mesh pointer : " << mesh << " and parent_pointer: " << &parent << endl;
      mayaloader_cat.spam() << "mesh vert_count : " << mesh->_vert_count << endl;
      mayaloader_cat.spam() << "mesh face_count : " << mesh->_face_count << endl;
      mayaloader_cat.spam() << "mesh vertexArray size: " << mesh->_vertexArray.length() << endl;
      mayaloader_cat.spam() << "mesh polygonCounts size: " << mesh->_polygonCounts.length() << endl;
      mayaloader_cat.spam() << "mesh polygonConnects size: " << mesh->_polygonConnects.length() << endl;
      mayaloader_cat.spam() << "mesh uarray size: " << mesh->_uarray.length() << endl;
      mayaloader_cat.spam() << "mesh varray size: " << mesh->_varray.length() << endl;
    }
    mesh->_transNode = mfn.create(mesh->_vert_count, mesh->_face_count,
                                  mesh->_vertexArray, mesh->_polygonCounts, mesh->_polygonConnects,
                                  mesh->_uarray, mesh->_varray,
                                  parent, &status);
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "transNode created." << endl;
    }

    if (!mesh->_renameTrans) {
      mesh->_transNode = parent;
    }

    // [gjeon] add eggFlag attributes if any exists
    for (unsigned i = 0; i < mesh->_eggObjectTypes.length(); i++) {
      MString attrName = "eggObjectTypes";
      attrName += (int)(i + 1);
      status = create_enum_attribute(mesh->_transNode, attrName, attrName, mesh->_eggObjectTypes, i);
      if (status != MStatus::kSuccess) {
        status.perror("create_enum_attribute failed!");
      }
    }

    // Check the "Display Colors" box by default, so that vertex colors (if
    // any) will be visible.
    MPlug displayColors = mfn.findPlug("displayColors");
    displayColors.setValue((bool)true);

    mesh->_shapeNode = mfn.object();
    mfn.getPath(mesh->_shape_dag_path);
    mesh->ConnectTextures();

    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "textures connected." << endl;
    }

    mfn.getCurrentUVSetName(cset);
    status = mfn.assignUVs(mesh->_polygonCounts, mesh->_uvIds, &cset);

    if (status != MStatus::kSuccess) {
      status.perror("assignUVs failed");
      if (mayaloader_cat.is_spam()) {
        PrintData(mesh);
      }
    }
    else {
      if (mayaloader_cat.is_spam()) {
        mayaloader_cat.spam() << "uvs assigned." << endl;
      }
    }

    // lets try to set normals per vertex
    if (respect_normals) {
      status = mfn.setVertexNormals(mesh->_normalArray, mesh->_vertNormalIndices, MSpace::kTransform);
      if (status != MStatus::kSuccess) {
        status.perror("setVertexNormals failed!");
      }
    }

    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "vertex normals set." << endl;
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

  for (si = _surface_tab.begin(); si != _surface_tab.end(); ++si) {
    MayaEggNurbsSurface *surface = (*si).second;
    if (surface->_cvArray.length()==0) {
      continue;
    }

    // MStatus status;
    MFnNurbsSurface mfnNurbsSurface;

    MayaEggGroup *parentNode = FindGroup(surface->_parent);
    MObject parent = MObject::kNullObj;
    if (parentNode) {
      parent = parentNode->_group;
      if (mayaloader_cat.is_debug()) {
        mayaloader_cat.debug() << "surface's parent (group) : " << parentNode->_name << endl;
      }
    } else {
      surface->_renameTrans = true;
      if (mayaloader_cat.is_debug()) {
        mayaloader_cat.debug() << "surface's parent (null) : " << endl;
      }
    }

    surface->_transNode = mfnNurbsSurface.create(surface->_cvArray, surface->_uKnotArray, surface->_vKnotArray,
                                                 surface->_uDegree, surface->_vDegree, surface->_uForm, surface->_vForm,
                                                 true, parent, &status);

    if (!surface->_renameTrans) {
      surface->_transNode = parent;
    }

    // [gjeon] add eggFlag attributes if any exists
    for (unsigned i = 0; i < surface->_eggObjectTypes.length(); i++) {
      MString attrName = "eggObjectTypes";
      attrName += (int)(i + 1);
      status = create_enum_attribute(surface->_transNode, attrName, attrName, surface->_eggObjectTypes, i);
      if (status != MStatus::kSuccess) {
        status.perror("create_enum_attribute failed!");
      }
    }
    surface->_shapeNode = mfnNurbsSurface.object();
    mfnNurbsSurface.getPath(surface->_shape_dag_path);
    surface->ConnectTextures();

    mayaloader_cat.debug() << status.errorString().asChar() << endl;
  }


  double thickness = 0.0;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    MayaEggJoint *joint = (*ji).second;
    double dfo = (joint->GetPos()).length();
    if (dfo > thickness) {
      thickness = dfo;
    }
  }
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "thickness from joints: " << thickness << endl;
  }
  thickness = thickness * 0.025;
  for (unsigned int i=0; i<_joint_list.size(); i++) {
    MayaEggJoint *joint = _joint_list[i];
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "creating a joint: " << joint->_egg_joint->get_name() << endl;
    }
    joint->ChooseEndPos(thickness);
    joint->CreateMayaBone(FindGroup(joint->_egg_parent));
  }
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "went past all the joints" << endl;
  }
  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    MayaEggMesh *mesh = (*ci).second;
    EggGroup *joint = mesh->GetControlJoint();
    if (joint) {
      CreateSkinCluster(mesh);
    }
  }
  for (si = _surface_tab.begin(); si != _surface_tab.end(); ++si) {
    MayaEggNurbsSurface *surface = (*si).second;
    EggGroup *joint = surface->GetControlJoint();
    if (joint) {
      CreateSkinCluster(surface);
    }
  }
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "went past creating skin cluster" << endl;
  }
  for (ci = _mesh_tab.begin();  ci != _mesh_tab.end();  ++ci) {
    (*ci).second->AssignNames();
  }
  for (si = _surface_tab.begin();  si != _surface_tab.end();  ++si) {
    (*si).second->AssignNames();
  }
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "went past mesh AssignNames" << endl;
  }
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    (*ji).second->AssignNames();
  }
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "went past joint AssignNames" << endl;
  }
  for (ti = _tex_tab.begin();   ti != _tex_tab.end();   ++ti) {
    (*ti).second->AssignNames();
  }
  if (mayaloader_cat.is_spam()) {
    mayaloader_cat.spam() << "went past tex AssignNames" << endl;
  }

  if (mayaloader_cat.is_debug()) {
    mayaloader_cat.debug() << "-fri: " << _frame_rate << " -sf: " << _start_frame
                           << " -ef: " << _end_frame << endl;
  }

  // masad: keep track of maximum frames of animation on all these joints
  MTime maxFrame(_start_frame - 1, _timeUnit);
  MTime minFrame = maxFrame;

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

    mfnAnimCurveTX.create(node, attrTX, MFnAnimCurve::kAnimCurveTL, nullptr, &status);
    mfnAnimCurveTY.create(node, attrTY, MFnAnimCurve::kAnimCurveTL, nullptr, &status);
    mfnAnimCurveTZ.create(node, attrTZ, MFnAnimCurve::kAnimCurveTL, nullptr, &status);
    mfnAnimCurveRX.create(node, attrRX, MFnAnimCurve::kAnimCurveTA, nullptr, &status);
    mfnAnimCurveRY.create(node, attrRY, MFnAnimCurve::kAnimCurveTA, nullptr, &status);
    mfnAnimCurveRZ.create(node, attrRZ, MFnAnimCurve::kAnimCurveTA, nullptr, &status);
    mfnAnimCurveSX.create(node, attrSX, MFnAnimCurve::kAnimCurveTU, nullptr, &status);
    mfnAnimCurveSY.create(node, attrSY, MFnAnimCurve::kAnimCurveTU, nullptr, &status);
    mfnAnimCurveSZ.create(node, attrSZ, MFnAnimCurve::kAnimCurveTU, nullptr, &status);

    MTransformationMatrix matrix( mMat );
    MVector trans = matrix.translation(MSpace::kTransform, &status);

    double rot[3];
    MTransformationMatrix::RotationOrder order = MTransformationMatrix::kXYZ;
    status = matrix.getRotation(rot, order);

    double scale[3];
    status = matrix.getScale(scale, MSpace::kTransform);
    MFnAnimCurve::TangentType tangent = MFnAnimCurve::kTangentClamped;
    MTime time(_start_frame - 1, _timeUnit);

    mfnAnimCurveTX.addKey(time, trans.x, tangent, tangent, nullptr, &status);
    mfnAnimCurveTY.addKey(time, trans.y, tangent, tangent, nullptr, &status);
    mfnAnimCurveTZ.addKey(time, trans.z, tangent, tangent, nullptr, &status);
    mfnAnimCurveRX.addKey(time, rot[0], tangent, tangent, nullptr, &status);
    mfnAnimCurveRY.addKey(time, rot[1], tangent, tangent, nullptr, &status);
    mfnAnimCurveRZ.addKey(time, rot[2], tangent, tangent, nullptr, &status);
    mfnAnimCurveSX.addKey(time, scale[0], tangent, tangent, nullptr, &status);
    mfnAnimCurveSY.addKey(time, scale[1], tangent, tangent, nullptr, &status);
    mfnAnimCurveSZ.addKey(time, scale[2], tangent, tangent, nullptr, &status);

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

      mfnAnimCurveTX.addKey(time, trans.x, tangent, tangent, nullptr, &status);
      mfnAnimCurveTY.addKey(time, trans.y, tangent, tangent, nullptr, &status);
      mfnAnimCurveTZ.addKey(time, trans.z, tangent, tangent, nullptr, &status);
      mfnAnimCurveRX.addKey(time, rot[0], tangent, tangent, nullptr, &status);
      mfnAnimCurveRY.addKey(time, rot[1], tangent, tangent, nullptr, &status);
      mfnAnimCurveRZ.addKey(time, rot[2], tangent, tangent, nullptr, &status);
      mfnAnimCurveSX.addKey(time, scale[0], tangent, tangent, nullptr, &status);
      mfnAnimCurveSY.addKey(time, scale[1], tangent, tangent, nullptr, &status);
      mfnAnimCurveSZ.addKey(time, scale[2], tangent, tangent, nullptr, &status);
    }
    if (maxFrame < time) {
      maxFrame = time;
    }
  }
  if (anim) {
    // masad: set the control's max time with maxFrame
    MAnimControl::setMaxTime(maxFrame);
    MAnimControl::setMinTime(minFrame);
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

  // ResumeSetKeyMode(); ResumeAnimate();

  mayaloader_cat.info() << "Egg import successful\n";
  return true;
}

void MayaEggLoader::PrintData(MayaEggMesh *mesh)
{
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "Mesh: " << mesh->_name << endl;
      mayaloader_cat.spam() << "num vertexArray: " << mesh->_vertexArray.length() << endl;
      ostringstream stream3;
      for (unsigned int i=0; i < mesh->_vertexArray.length(); ++i) {
        stream3 << "[" << mesh->_vertexArray[i].x << " " << mesh->_vertexArray[i].y << " " << mesh->_vertexArray[i].z << "]" << endl;
      }

      mayaloader_cat.spam() << "vertexArray: \n" << stream3.str() << endl;
      mayaloader_cat.spam() << "num polygonConnects: " << mesh->_polygonConnects.length() << endl;
      mayaloader_cat.spam() << "num uvCounts: " << mesh->_polygonCounts.length() << endl;
      mayaloader_cat.spam() << "num uvIds: " << mesh->_uvIds.length() << endl;
      ostringstream stream1, stream4;
      unsigned int k=0;
      for (unsigned int i=0; i < mesh->_polygonCounts.length(); ++i) {
        stream1 << mesh->_polygonCounts[i] << ":->";
        stream4 << mesh->_polygonCounts[i] << ":->";
        for (int j=0; j < mesh->_polygonCounts[i]; ++j, ++k) {
          stream1 << mesh->_uvIds[k] << ",";
          stream4 << mesh->_polygonConnects[k] << ",";
        }
        stream1 << endl;
        stream4 << endl;
      }
      mayaloader_cat.spam() << "uvCounts:->uvIds " << endl << stream1.str() << endl;
      mayaloader_cat.spam() << "vertexCount:->polygonConnects" << endl << stream4.str() << endl;
    }
}

void MayaEggLoader::ParseFrameInfo(string comment)
{
  size_t pos, ls, le;

  pos = comment.find("-fri");
  if (pos != string::npos) {
    ls = comment.find(" ", pos+4);
    le = comment.find(" ", ls+1);
    if (mayaloader_cat.is_debug()) {
      mayaloader_cat.debug() <<    comment.substr(ls+1, le-ls-1) << endl;
    }
    _frame_rate = atoi(comment.substr(ls+1,le-ls-1).data());
    // mayaloader_cat.debug() << "le = " << le << "; and ls = " << ls << ";
    // frame_rate = " << _frame_rate << endl;

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
    // mayaloader_cat.debug() << "le = " << le << "; and ls = " << ls << ";
    // start_frame = " << _start_frame << endl;
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
    // mayaloader_cat.debug() << "le = " << le << "; and ls = " << ls << ";
    // end_frame = " << _end_frame << endl;
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
  MObject node = MObject::kNullObj;
  size_t pos;
  string name;

  pos = givenName.find(":");
  if (pos != string::npos) {
    name = givenName.substr(pos+1);
  } else
    name = givenName;

  /*
  // masad: I do not think you want to return a mesh node because keyframes
  // should only apply to joint nodes.
  MeshTable::const_iterator ci;
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
      cerr << "foo get dependency node returning a mesh's transNode? why? : " << givenName << endl;
      return node;
    }
  }
  */

  JointTable::const_iterator ji;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    MayaEggJoint *joint = (*ji).second;
    if (mayaloader_cat.is_spam()) {
      mayaloader_cat.spam() << "traversing a joint: " << joint->_egg_joint->get_name() << endl;
    }
    string jointName = joint->_egg_joint->get_name();
    if (jointName == name)
    {
      node = joint->_joint;
      return node;
    }
  }

  return node;
}

// The two global functions that form the API of this module.

bool MayaLoadEggData(EggData *data, bool merge, bool model, bool anim, bool respect_normals)
{
  MayaEggLoader loader;
  bool temp = loader.ConvertEggData(data, merge, model, anim, respect_normals);
  return temp;
}

bool MayaLoadEggFile(const char *name, bool merge, bool model, bool anim, bool respect_normals)
{
  MayaEggLoader loader;
  return loader.ConvertEggFile(name, merge, model, anim, respect_normals);
}

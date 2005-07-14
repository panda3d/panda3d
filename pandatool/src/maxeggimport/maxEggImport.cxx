////////////////////////////////////////////////////////////////////////////////////////////////////
//
// maxEggImport.cxx - Egg Importer for 3D Studio Max.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Max.h"
#include "maxImportRes.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "simpobj.h"
#include "iparamb2.h"

#include "eggData.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "eggPrimitive.h"
#include "eggGroupNode.h"
#include "eggPolysetMaker.h"
#include "eggBin.h"

#include <stdio.h>

static FILE *lgfile;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The MaxEggImporter class
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MaxEggMesh;
class MaxEggJoint;
class MaxEggTex;
class BoneFunctions;

class MaxEggImporter : public SceneImport 
{
public:
  // GUI-related methods
  MaxEggImporter();
  ~MaxEggImporter();
  int		    ExtCount();        // Number of extensions supported 
  const TCHAR * Ext(int n);        // Extension #n (i.e. "EGG")
  const TCHAR * LongDesc();        // Long ASCII description (i.e. "Egg Importer") 
  const TCHAR * ShortDesc();       // Short ASCII description (i.e. "Egg")
  const TCHAR * AuthorName();      // ASCII Author name
  const TCHAR * CopyrightMessage();// ASCII Copyright message 
  const TCHAR * OtherMessage1();   // Other message #1
  const TCHAR * OtherMessage2();   // Other message #2
  unsigned int Version();          // Version number * 100 (i.e. v3.01 = 301) 
  void	ShowAbout(HWND hWnd);      // Show DLL's "About..." box
  int	DoImport(const TCHAR *name,ImpInterface *ei,Interface *i, BOOL suppressPrompts);

public:
  // GUI-related fields
  static Interface     *_ip;
  static ImpInterface  *_impip;
  static BOOL           _merge;
  static BOOL           _importmodel;
  static BOOL           _importanim;

public:
  // Import-related methods:
  void  TraverseEggData(EggData *data);
  void  TraverseEggNode(EggNode *node, EggGroup *context);
  MaxEggMesh  *GetMesh(EggVertexPool *pool);
  MaxEggJoint *FindJoint(EggGroup *joint);
  MaxEggJoint *MakeJoint(EggGroup *joint, EggGroup *context);
  MaxEggTex   *GetTex(const string &fn);

public:
  // Import-related fields:
  typedef phash_map<EggVertexPool *, MaxEggMesh *> MeshTable;
  typedef second_of_pair_iterator<MeshTable::const_iterator> MeshIterator;
  typedef phash_map<EggGroup *, MaxEggJoint *> JointTable;
  typedef second_of_pair_iterator<JointTable::const_iterator> JointIterator;
  typedef phash_map<string, MaxEggTex *> TexTable;
  typedef second_of_pair_iterator<TexTable::const_iterator> TexIterator;
  MeshTable  _mesh_tab;
  JointTable _joint_tab;
  TexTable   _tex_tab;
  int        _next_tex;
};

Interface     *MaxEggImporter::_ip;
ImpInterface  *MaxEggImporter::_impip;
BOOL MaxEggImporter::_merge       = TRUE;
BOOL MaxEggImporter::_importmodel = TRUE;
BOOL MaxEggImporter::_importanim  = TRUE;

MaxEggImporter::MaxEggImporter()
{
}

MaxEggImporter::~MaxEggImporter()
{
}

int MaxEggImporter::ExtCount()
{
  // Number of different extensions handled by this importer.
  return 1;
}

const TCHAR * MaxEggImporter::Ext(int n)
{
  // Fetch the extensions handled by this importer.
  switch(n) {
  case 0: return _T("egg");
  default: return _T("");
  }
}

const TCHAR * MaxEggImporter::LongDesc()
{
  return _T("Panda3D Egg Importer");
}

const TCHAR * MaxEggImporter::ShortDesc()
{
  return _T("Panda3D Egg");
}

const TCHAR * MaxEggImporter::AuthorName() 
{
  return _T("Joshua Yelon");
}

const TCHAR * MaxEggImporter::CopyrightMessage() 
{
  return _T("Copyight (c) 2005 Josh Yelon");
}

const TCHAR * MaxEggImporter::OtherMessage1() 
{
  return _T("");
}

const TCHAR * MaxEggImporter::OtherMessage2() 
{
  return _T("");
}

unsigned int MaxEggImporter::Version()
{
  return 100;
}

static BOOL CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
  case WM_INITDIALOG:
    CenterWindow(hWnd, GetParent(hWnd)); 
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      EndDialog(hWnd, 1);
      break;
    }
    break;
  default:
    return FALSE;
  }
  return TRUE;
}      

void MaxEggImporter::ShowAbout(HWND hWnd)
{
  DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX),
                 hWnd, AboutBoxDlgProc, 0);
}


static BOOL CALLBACK ImportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  MaxEggImporter *imp = (MaxEggImporter*)GetWindowLong(hWnd,GWL_USERDATA); 
  switch (msg) {
  case WM_INITDIALOG:
    imp = (MaxEggImporter*)lParam;
    SetWindowLong(hWnd,GWL_USERDATA,lParam); 
    CenterWindow(hWnd, GetParent(hWnd)); 
    CheckDlgButton(hWnd, IDC_MERGE,       imp->_merge);
    CheckDlgButton(hWnd, IDC_IMPORTMODEL, imp->_importmodel);
    CheckDlgButton(hWnd, IDC_IMPORTANIM,  imp->_importanim);
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      imp->_merge       = IsDlgButtonChecked(hWnd, IDC_MERGE); 
      imp->_importmodel = IsDlgButtonChecked(hWnd, IDC_IMPORTMODEL); 
      imp->_importanim  = IsDlgButtonChecked(hWnd, IDC_IMPORTANIM); 
      EndDialog(hWnd, 1);
      break;
    case IDCANCEL:
      EndDialog(hWnd, 0);
      break;
    }
    break;
  default:
    return FALSE;
  }
  return TRUE;
}       

int MaxEggImporter::DoImport(const TCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts) 
{
  // Grab the interface pointer.
  _ip = i;
  _impip = ii;
  
  // Prompt the user with our dialogbox.
  if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_IMPORT_DLG),
                      _ip->GetMAXHWnd(), ImportDlgProc, (LPARAM)this)) {
    return 1;
  }

  // Read in the egg file.
  EggData data;
  Filename datafn = Filename::from_os_specific(name);
  MessageBox(NULL, datafn.c_str(), "Panda3D Egg Importer", MB_OK);
  if (!data.read(datafn)) {
    MessageBox(NULL, "Cannot read Egg file", "Panda3D Egg Importer", MB_OK);
    return 1;
  }
  
  // Do all the good stuff.
  TraverseEggData(&data);
  MessageBox(NULL, "Import Complete", "Panda3D Egg Importer", MB_OK);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MaxEggTex
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MaxEggTex
{
public:
  string path;
  int    id;
};

MaxEggTex *MaxEggImporter::GetTex(const string &fn)
{
  if (_tex_tab.count(fn))
    return _tex_tab[fn];
  MaxEggTex *res = new MaxEggTex;
  res->path = fn;
  res->id = _next_tex ++;
  _tex_tab[fn] = res;
  return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MaxEggMesh
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MaxEggMesh
{
public:

  EggVertexPool *_pool;
  TriObject     *_obj;
  Mesh          *_mesh;
  INode         *_node;
  int            _vert_count;
  int            _tvert_count;
  int            _cvert_count;
  int            _face_count;
  
  typedef pair<Vertexd, Normald> VertexPos;
  typedef pair<VertexPos, EggGroup *> VertexContext;
  phash_map<VertexContext, int> _vert_tab;
  phash_map<TexCoordd,     int> _tvert_tab;
  phash_map<Colorf,        int> _cvert_tab;
  
  int GetVert(Vertexd pos, Normald norm, EggGroup *context);
  int GetTVert(TexCoordd uv);
  int GetCVert(Colorf col);
  int AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2, int tex);
};

int MaxEggMesh::GetVert(Vertexd pos, Normald norm, EggGroup *context)
{
  VertexContext key = VertexContext(VertexPos(pos,norm),context);
  if (_vert_tab.count(key))
    return _vert_tab[key];
  if (_vert_count == _mesh->numVerts) {
    int nsize = _vert_count*2 + 100;
    _mesh->setNumVerts(nsize, _vert_count?TRUE:FALSE);
  }
  int idx = _vert_count++;
  _mesh->setVert(idx, pos.get_x(), pos.get_y(), pos.get_z());
  _vert_tab[key] = idx;
  return idx;
}

int MaxEggMesh::GetTVert(TexCoordd uv)
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

int MaxEggMesh::GetCVert(Colorf col)
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

MaxEggMesh *MaxEggImporter::GetMesh(EggVertexPool *pool)
{
  MaxEggMesh *result = _mesh_tab[pool];
  if (result == 0) {
    string name = pool->get_name();
    int nsize = name.size();
    if ((nsize > 6) && (name.rfind(".verts")==(nsize-6)))
      name.resize(nsize-6);
    result = new MaxEggMesh;
    result->_pool = pool;
    result->_obj  = CreateNewTriObject();
    result->_mesh = &result->_obj->GetMesh();
    result->_mesh->setMapSupport(0, TRUE);
    result->_node = _ip->CreateObjectNode(result->_obj);
    result->_vert_count = 0;
    result->_tvert_count = 0;
    result->_cvert_count = 0;
    result->_face_count = 0;
    // result->_node->SetName(name.c_str());
    _mesh_tab[pool] = result;
  }
  return result;
}

int MaxEggMesh::AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2, int tex)
{
  static int dump = 0;
  if (_face_count == _mesh->numFaces) {
    int nsize = _face_count*2 + 100;
    BOOL keep = _mesh->numFaces ? TRUE:FALSE;
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

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MaxEggJoint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MaxEggJoint
{
public:
  Point3         _xv,_yv,_zv;
  Point3         _pos;
  Point3         _endpos;
  Point3         _zaxis;
  double         _thickness;
  bool           _anyvertex;
  SimpleObject2 *_bone;
  INode         *_node;
  EggGroup      *_egg_joint;
  MaxEggJoint   *_parent;
  vector <MaxEggJoint *> _children;

public:
  MaxEggJoint *ChooseBestChild(Point3 dir);
  void ChooseEndPos(double thickness);
  void CreateMaxBone(void);
};

MaxEggJoint *MaxEggImporter::FindJoint(EggGroup *joint)
{
  if (joint==0) return 0;
  return _joint_tab[joint];
}

MaxEggJoint *MaxEggImporter::MakeJoint(EggGroup *joint, EggGroup *context)
{
  MaxEggJoint *parent = FindJoint(context);
  MaxEggJoint *result = new MaxEggJoint;
  LMatrix4d t = joint->get_transform();
  if (parent) {
    result->_xv  = parent->_xv*((float)t(0,0)) + parent->_yv*((float)t(0,1)) + parent->_zv*((float)t(0,2));
    result->_yv  = parent->_xv*((float)t(1,0)) + parent->_yv*((float)t(1,1)) + parent->_zv*((float)t(1,2));
    result->_zv  = parent->_xv*((float)t(2,0)) + parent->_yv*((float)t(2,1)) + parent->_zv*((float)t(2,2));
    result->_pos = parent->_xv*((float)t(3,0)) + parent->_yv*((float)t(3,1)) + parent->_zv*((float)t(3,2)) + parent->_pos;
  } else {
    result->_xv  = Point3(((float)t(0,0)), ((float)t(0,1)), ((float)t(0,2)));
    result->_yv  = Point3(((float)t(1,0)), ((float)t(1,1)), ((float)t(1,2)));
    result->_zv  = Point3(((float)t(2,0)), ((float)t(2,1)), ((float)t(2,2)));
    result->_pos = Point3(((float)t(3,0)), ((float)t(3,1)), ((float)t(3,2)));
  }
  result->_endpos = Point3(0,0,0);
  result->_zaxis = Point3(0,0,0);
  result->_thickness = 0.0;
  result->_anyvertex = false;
  result->_bone = 0;
  result->_node = 0;
  result->_egg_joint = joint;
  result->_parent = parent;
  if (parent) parent->_children.push_back(result);
  _joint_tab[joint] = result;
  return result;
}

MaxEggJoint *MaxEggJoint::ChooseBestChild(Point3 dir)
{
  if (dir.Length() < 0.001) return 0;
  dir = dir.Normalize();
  double firstbest = -1000;
  MaxEggJoint *firstchild = 0;
  Point3 firstpos = _pos;
  double secondbest = 0;
  for (int i=0; i<_children.size(); i++) {
    MaxEggJoint *child = _children[i];
    Point3 tryfwd = child->_pos - _pos;
    if ((child->_pos != firstpos) && (tryfwd.Length() > 0.001)) {
      Point3 trydir = tryfwd.Normalize();
      double quality = trydir % dir;
      if (quality > firstbest) {
        secondbest = firstbest;
        firstbest = quality;
        firstpos = child->_pos;
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
  Point3 parentpos(0,0,0);
  Point3 parentendpos(0,0,1);
  if (_parent) {
    parentpos = _parent->_pos;
    parentendpos = _parent->_endpos;
  }
  Point3 fwd = _pos - parentpos;
  if (fwd.Length() < 0.001) {
    fwd = parentendpos - parentpos;
  }
  fwd = fwd.Normalize();
  MaxEggJoint *child = ChooseBestChild(fwd);
  if (child == 0) {
    _endpos = fwd * ((float)(thickness * 0.8)) + _pos;
    _thickness = thickness * 0.8;
  } else {
    _endpos = child->_pos;
    _thickness = (_endpos - _pos).Length();
    if (_thickness > thickness) _thickness = thickness;
  }
  Point3 orient = (_endpos - _pos).Normalize();
  Point3 altaxis = orient ^ Point3(0,-1,0);
  if (altaxis.Length() < 0.001) altaxis = orient ^ Point3(0,0,1);
  _zaxis = (altaxis ^ orient).Normalize();
}

void MaxEggJoint::CreateMaxBone(void)
{
  Point3 fwd = _endpos - _pos;
  double len = fwd.Length();
  Point3 txv = fwd * ((float)(1.0/len));
  Point3 tzv = _zaxis;
  Point3 tyv = tzv ^ txv;
  Point3 row1 = Point3(txv % _xv, txv % _yv, txv % _zv);
  Point3 row2 = Point3(tyv % _xv, tyv % _yv, tyv % _zv);
  Point3 row3 = Point3(tzv % _xv, tzv % _yv, tzv % _zv);
  _bone = (SimpleObject2*)CreateInstance(GEOMOBJECT_CLASS_ID, BONE_OBJ_CLASSID);
  _node = (MaxEggImporter::_ip)->CreateObjectNode(_bone);
  if (_parent) {
    _node->Detach(0, 1);
    _parent->_node->AttachChild(_node, 1);
  }
  _node->SetNodeTM(0, Matrix3(_xv, _yv, _zv, _pos));
  _node->SetObjOffsetRot(Quat(Matrix3(row1, row2, row3, Point3(0,0,0))));
  IParamBlock2 *blk = _bone->pblock2;
  for (int i=0; i<blk->NumParams(); i++) {
    TSTR n = blk->GetLocalName(i);
    if      (strcmp(n, "Length")==0) blk->SetValue(i,0,(float)len); 
    else if (strcmp(n, "Width")==0)  blk->SetValue(i,0,(float)_thickness);
    else if (strcmp(n, "Height")==0) blk->SetValue(i,0,(float)_thickness);
  }
  Point3 boneColor = GetUIColor(COLOR_BONES);
  _node->SetWireColor(RGB(int(boneColor.x*255.0f), int(boneColor.y*255.0f), int(boneColor.z*255.0f) ));
  _node->SetBoneNodeOnOff(TRUE, 0);
  _node->SetRenderable(FALSE);
  _node->SetName((TCHAR*)(_egg_joint->get_name().c_str()));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TraverseEggData
//
// We have an EggData in memory, and now we're going to copy that
// over into the max scene graph.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void MaxEggImporter::TraverseEggNode(EggNode *node, EggGroup *context)
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
      texid = GetTex(tex->get_fullpath().to_os_specific())->id;
      if (tex->has_transform())
        uvtrans = tex->get_transform();
    } else {
      texid = GetTex("")->id;
    }
    
    EggPolygon::const_iterator ci;
    MaxEggMesh *mesh = GetMesh(poly->get_pool());
    vertIndices.clear();
    tvertIndices.clear();
    cvertIndices.clear();
    for (ci = poly->begin(); ci != poly->end(); ++ci) {
      EggVertex *vtx = (*ci);
      EggVertexPool *pool = poly->get_pool();
      TexCoordd uv = vtx->get_uv();
      vertIndices.push_back(mesh->GetVert(vtx->get_pos3(), vtx->get_normal(), context));
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

void MaxEggImporter::TraverseEggData(EggData *data)
{
  MeshIterator ci;
  JointIterator ji;
  TexIterator ti;
  lgfile = fopen("MaxEggImporter.log","w");

  SuspendAnimate();
  SuspendSetKeyMode();
  AnimateOff();
  _next_tex = 0;

  TraverseEggNode(data, NULL);

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
    double dfo = ((*ji)->_pos).Length();
    if (dfo > thickness) thickness = dfo;
  }
  thickness = thickness * 0.025;
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    MaxEggJoint *joint = *ji;
    joint->ChooseEndPos(thickness);
    joint->CreateMaxBone();
  }
  
  if (_next_tex) {
    BitmapTex *bmt = 0;
    MultiMtl *mtl = NewDefaultMultiMtl();
    mtl->SetNumSubMtls(_next_tex);
    for (ti = _tex_tab.begin(); ti != _tex_tab.end(); ++ti) {
      MaxEggTex *tex = *ti;
      BitmapTex *bmt = NewDefaultBitmapTex();
      bmt->SetMapName((TCHAR*)(tex->path.c_str()));
      StdMat *mat = NewDefaultStdMat();
      mat->SetSubTexmap(ID_DI, bmt);
      mat->SetTexmapAmt(ID_DI, 1.0, 0);
      mat->EnableMap(ID_DI, TRUE);
      mat->SetActiveTexmap(bmt);
      _ip->ActivateTexture(bmt, mat);
      TSTR name;
      mtl->SetSubMtlAndName(tex->id, mat, name);
    }
    for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
      MaxEggMesh *mesh = *ci;
      mesh->_node->SetMtl(mtl);
    }
  }

  ResumeSetKeyMode();
  ResumeAnimate();
  
  for (ci = _mesh_tab.begin(); ci != _mesh_tab.end(); ++ci) {
    delete *ci;
  }
  for (ji = _joint_tab.begin(); ji != _joint_tab.end(); ++ji) {
    delete *ji;
  }
  for (ti = _tex_tab.begin(); ti != _tex_tab.end(); ++ti) {
    delete *ti;
  }
  
  if (lgfile) fclose(lgfile);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Plugin Initialization
//
// The following code enables Max to load this DLL, get a list
// of the classes defined in this DLL, and provides a means for
// Max to create instances of those classes.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
  static int controlsInit = FALSE;
  hInstance = hinstDLL;
  
  if (!controlsInit) {
    controlsInit = TRUE;
    InitCustomControls(hInstance);
    InitCommonControls();
  }
  
  return (TRUE);
}

#define PANDAEGGIMP_CLASS_ID1      0x377193ab
#define PANDAEGGIMP_CLASS_ID2      0x897afe12

class MaxEggImporterClassDesc: public ClassDesc
{
public:
  int            IsPublic() {return 1;}
  void          *Create(BOOL loading = FALSE) {return new MaxEggImporter;} 
  const TCHAR   *ClassName() {return _T("MaxEggImporter");}
  SClass_ID      SuperClassID() {return SCENE_IMPORT_CLASS_ID;} 
  Class_ID       ClassID() {return Class_ID(PANDAEGGIMP_CLASS_ID1,PANDAEGGIMP_CLASS_ID2);}
  const TCHAR   *Category() {return _T("Chrutilities");}
};

static MaxEggImporterClassDesc MaxEggImporterDesc;

__declspec( dllexport ) const TCHAR* LibDescription() 
{
  return _T("Panda3D Egg Importer");
}

__declspec( dllexport ) int LibNumberClasses() 
{
  return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
  switch(i) {
  case 0: return &MaxEggImporterDesc;
  default: return 0;
  }
}

__declspec( dllexport ) ULONG LibVersion() 
{
  return VERSION_3DSMAX;
}


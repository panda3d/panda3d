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
#include "splshape.h"
#include "dummy.h"

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

class MaxEggImporter : public SceneImport 
{
public:
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
  MaxEggMesh *GetMesh(EggVertexPool *pool);
  void  TraverseEggData(EggData *data);
  void  TraverseEggNode(EggNode *node, EggGroup *context);
  
public:
  Interface    *_ip;
  ImpInterface *_impip;
  static BOOL   _merge;
  static BOOL   _importmodel;
  static BOOL   _importanim;

  typedef pmap<EggVertexPool *, MaxEggMesh *> MeshTable;
  typedef second_of_pair_iterator<MeshTable::const_iterator> MeshIterator;
  MeshTable _mesh_tab;
};


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
  DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
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
  int AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2);
  void LogData(void);
};

void MaxEggMesh::LogData(void)
{
  fprintf(lgfile,"Mesh %08x faces: %d\n",_mesh,_mesh->numFaces);
  for (int i=0; i<_mesh->numFaces; i++) {
    fprintf(lgfile," -- %d %d %d\n",_mesh->tvFace[i].t[0],_mesh->tvFace[i].t[1],_mesh->tvFace[i].t[2]);
  }
}

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

int MaxEggMesh::AddFace(int v0, int v1, int v2, int tv0, int tv1, int tv2, int cv0, int cv1, int cv2)
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
  _mesh->tvFace[idx].setTVerts(tv0,tv1,tv2);
  _mesh->vcFace[idx].setTVerts(cv0,cv1,cv2);
  return idx;
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

    EggPolygon::const_iterator ci;
    MaxEggMesh *mesh = GetMesh(poly->get_pool());
    vertIndices.clear();
    tvertIndices.clear();
    cvertIndices.clear();
    for (ci = poly->begin(); ci != poly->end(); ++ci) {
      EggVertex *vtx = (*ci);
      EggVertexPool *pool = poly->get_pool();
      vertIndices.push_back(mesh->GetVert(vtx->get_pos3(), vtx->get_normal(), context));
      tvertIndices.push_back(mesh->GetTVert(vtx->get_uv()));
      cvertIndices.push_back(mesh->GetCVert(vtx->get_color()));
    }
    for (int i=1; i<vertIndices.size()-1; i++)
      mesh->AddFace(vertIndices[0], vertIndices[i], vertIndices[i+1],
                    tvertIndices[0], tvertIndices[i], tvertIndices[i+1],
                    cvertIndices[0], cvertIndices[i], cvertIndices[i+1]);
  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, node);
    if (node->is_of_type(EggGroup::get_class_type())) {
      EggGroup *group = DCAST(EggGroup, node);
      if (group->is_joint()) context = group;
    }
    EggGroupNode::const_iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      TraverseEggNode(*ci, context);
    }
  }
}

void MaxEggImporter::TraverseEggData(EggData *data)
{
  lgfile = fopen("MaxEggImporter.log","w");
  TraverseEggNode(data, NULL);
  MeshIterator ci;
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


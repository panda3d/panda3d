/*
  MaxEgg.h
  Created by Steven "Sauce" Osman, Jan03
  Modified and maintained by Ken Strickland, (02/01/03)-(05/15/03)
  Modified and maintained by Corey Revilla, (05/22/03)-present
  Carnegie Mellon University, Entetainment Technology Center

  This file contains a 3dsMax exporter derived from discreet's own SceneExport
  plug-in class; this exporter is basically a wrapper around the MaxToEgg
  Panda-converter class, and just sets up the interface and environment
  in which the MaxToEgg class can be "run" as if it were a standalone app.
*/
#ifndef __MaxEgg__H
#define __MaxEgg__H

#include "pandatoolbase.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <crtdbg.h>
#include <errno.h>

using std::min;
using std::max;

#include "eggGroup.h"
#include "eggTable.h"
#include "eggXfmSAnim.h"
#include "eggData.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"

#include <iostream>
#include <fstream>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windef.h>
#include <windows.h>

#include <Max.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <istdplug.h>
#include <iskin.h>
#include <stdmat.h>
#include <phyexp.h>
#include <surf_api.h>
#include <bipexp.h>
#include <modstack.h>

#include "eggCoordinateSystem.h"
#include "eggGroup.h"
#include "eggPolygon.h"
#include "eggTextureCollection.h"
#include "eggTexture.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggNurbsCurve.h"
#include "pandatoolbase.h"
#include "eggXfmSAnim.h"
#include "pathStore.h"

#include "maxNodeDesc.h"
#include "maxNodeTree.h"
#include "maxOptionsDialog.h"
#include "maxResource.h"
#include "maxToEggConverter.h"

#define MaxEggPlugin_CLASS_ID   Class_ID(0x7ac0d6b7, 0x55731ef6)

#pragma conform(forScope, off)

/* Externed Globals
 */
extern HINSTANCE hInstance;

/* Global Functions
 */
extern TCHAR *GetString(int id);

/* This class defines the 3D Studio Max exporter itself.  It is basically a
   shell that is invoked by 3D Studio Max's export API.  It then sets up
   MaxToEgg instance and attempts to "fool it" into thinking that it is
   actually being invoked as a standalone program.  The thought behind this
   is that some day MaxToEgg may well be a standalone program, provided that
   a suitable interface to Max files can be connected from a standalone
   program instead of a plugin.
*/

#if MAX_VERSION_MAJOR < 9
  #define DefaultRemapDir NoRemap
#endif

class MaxEggPlugin : public HelperObject
{
  MaxOptionsDialog **eggList;
  int numEggs;
  int maxEggs;

 public:
  bool autoOverwrite;
  bool pview;
  bool logOutput;

  // Class vars
  static Mesh mesh;           // This plugin generates no geometry, this mesh is not passed on to 3D Studio.
  static short meshBuilt;
  static HWND hMaxEggParams;
  static IObjParam *iObjParams;

  // ConstructorDestructor
  MaxEggPlugin();
  virtual ~MaxEggPlugin();

  // Other class Methods
  void DoExport();
  void UpdateUI();
  void SaveCheckState();
  void BuildMesh();

  void AddEgg(MaxOptionsDialog *newEgg);
  void RemoveEgg(int i);
  MaxOptionsDialog *GetEgg(int i) { return (i >= 0 && i < numEggs) ? eggList[i] : nullptr; }

  // Required implimented virtual methods: inherited virtual methods for
  // Reference-management
  RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );
  void GetMat(TimeValue t, INode* inod, ViewExp *vpt, Matrix3& mat);

  // From BaseObject
  int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
  int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
  CreateMouseCallBack* GetCreateMouseCallBack();
  void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
  void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
#if MAX_VERSION_MAJOR < 15
  TCHAR *GetObjectName() { return GetString(IDS_LIBDESCRIPTION); }
#else
  const TCHAR *GetObjectName() { return GetString(IDS_LIBDESCRIPTION); }
#endif

  // From Object
  ObjectState Eval(TimeValue time);
  void InitNodeName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }
  Interval ObjectValidity(TimeValue time);
  void Invalidate();
  int DoOwnSelectHilite() { return 1; }

  // From GeomObject
  int IntersectRay(TimeValue t, Ray& r, PN_stdfloat& at) { return 0; }
  void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
  void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
  void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );

  // Animatable methods
  void DeleteThis() { delete this; }
  Class_ID ClassID() { return MaxEggPlugin_CLASS_ID; }
  void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_CLASS_NAME)); }
  TSTR SubAnimName(int i) { return TSTR(GetString(IDS_CLASS_NAME)); }

  // From ref
  RefTargetHandle Clone(RemapDir& remap = DefaultRemapDir());

  // IO
  IOResult Save(ISave *isave);
  IOResult Load(ILoad *iload);
};


#endif // __MaxEgg__H

// Filename: maxEggImport.cxx
// Created by:  jyelon (15Jul05)
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
// This is the wrapper code for the max importer plugin.
// It includes:
//
//   - user interface dialogs and popups
//   - plugin initialization/registration
//
// It does not include the actual code to traverse the EggData.
//
////////////////////////////////////////////////////////////////////

// MAX includes
#include "maxEggLoader.h"
#include "Max.h"
#include "maxImportRes.h"
#include "istdplug.h"

// panda includes.
#include "notifyCategoryProxy.h"

#include <iostream>
#include <sstream>

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
  static BOOL           _merge;
  static BOOL           _importmodel;
  static BOOL           _importanim;
};

BOOL MaxEggImporter::_merge       = TRUE;
BOOL MaxEggImporter::_importmodel = TRUE;
BOOL MaxEggImporter::_importanim  = FALSE;

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
  // Prompt the user with our dialogbox.
  if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_IMPORT_DLG),
                      i->GetMAXHWnd(), ImportDlgProc, (LPARAM)this)) {
    return 1;
  }

  std::ostringstream log;
  Notify::ptr()->set_ostream_ptr(&log, false);
  bool ok = MaxLoadEggFile(name, _merge ? true:false, _importmodel ? true:false, _importanim ? true:false);
  string txt = log.str();
  if (txt != "") MessageBox(NULL, txt.c_str(), "Panda3D Importer", MB_OK);
  else {
    if (!ok) MessageBox(NULL, "Import Failed, unknown reason\n", "Panda3D Importer", MB_OK);
  }
  Notify::ptr()->set_ostream_ptr(NULL, false);
  return 1;
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


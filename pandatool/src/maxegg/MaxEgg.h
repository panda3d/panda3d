/*
  MaxEgg.h 
  Created by Steven "Sauce" Osman, 01/??/03
  Modified and maintained by Ken Strickland, (02/01/03)-(05/15/03)
  Modified and maintained by Corey Revilla, (05/22/03)-present
  Carnegie Mellon University, Entetainment Technology Center

  This file contains a 3dsMax exporter derived from discreet's own SceneExport 
  plug-in class; this exporter is basically a wrapper around the MaxToEgg
  Panda-converter class, and just sets up the interface and environment
  in which the MaxToEgg class can be "run" as if it were a standalone app.
*/
#ifndef __MaxEggPlugin__H
#define __MaxEggPlugin__H

#pragma conform(forScope, off)

#include "pandatoolbase.h"

//Includes & Definitions
#include "MaxToEgg.h"
#include "windef.h"

/* Error-Reporting Includes
 */
#include "Logger.h"
#define ME Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM6

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
class MaxEggPlugin : public SceneExport 
{
 public:
  static HWND hParams;
  bool confirmExport;
  bool makeBam;
  bool animation;
  enum Anim_Type {
    AT_none,
    AT_model,
    AT_chan,
    AT_pose,
    AT_strobe,
    AT_both
  };
  Anim_Type anim_type;
  
  // Number of extensions supported
  int ExtCount();
  // Extension #n (i.e. "3DS")
  const TCHAR *Ext(int n);					
  // Long ASCII description (i.e. "Autodesk 3D Studio File")
  const TCHAR *LongDesc();
  // Short ASCII description (i.e. "3D Studio")
  const TCHAR *ShortDesc();
  // ASCII Author name
  const TCHAR *AuthorName();
  // ASCII Copyright message
  const TCHAR *CopyrightMessage();
  // Other message #1
  const TCHAR *OtherMessage1();
  // Other message #2
  const TCHAR *OtherMessage2();
  // Version number * 100 (i.e. v3.01 = 301)
  unsigned int Version();
  // Show DLL's "About..." box
  void ShowAbout(HWND hWnd);

  BOOL SupportsOptions(int ext, DWORD options);
  int DoExport(const TCHAR *name,ExpInterface *ei,
	       Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

  //Constructor/Destructor
  MaxEggPlugin();
  virtual ~MaxEggPlugin();
};

#endif // __MaxEggPlugin__H

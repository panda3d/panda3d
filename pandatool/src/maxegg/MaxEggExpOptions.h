/*
  MaxEggExpOptions.h 
  Created by Phillip Saltzman, 2/15/05
  Carnegie Mellon University, Entetainment Technology Center

  This file contains a class that allows users to specify
  export options, and then execute the export
*/

#ifndef __MaxEggExpOptions__H
#define __MaxEggExpOptions__H

#pragma conform(forScope, off)

//Includes & Definitions
#include "maxToEgg.h"
#include "windef.h"

/* Error-Reporting Includes */
#include "Logger.h"
#define ME Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM6
#define MNEG Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM2
#define MNEG_GEOMETRY_GENERATION Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM3

/* Externed Globals */
extern HINSTANCE hInstance;

//Save/load chunk definitions
#define CHUNK_OVERWRITE_FLAG  0x1000
#define CHUNK_PVIEW_FLAG      0x1001
#define CHUNK_LOG_OUTPUT      0x1002
#define CHUNK_EGG_EXP_OPTIONS 0x1100
#define CHUNK_ANIM_TYPE       0x1101
#define CHUNK_EGG_CHECKED     0x1102
#define CHUNK_DBL_SIDED       0x1103
#define CHUNK_SF              0x1104
#define CHUNK_EF              0x1105
#define CHUNK_FILENAME        0x1106
#define CHUNK_SHORTNAME       0x1107
#define CHUNK_EXPORT_FULL     0x1108
#define CHUNK_NODE_LIST       0x1200
#define CHUNK_NODE_HANDLE     0x1201

//Global functions
void ChunkSave(ISave *isave, int chunkid, int value);
void ChunkSave(ISave *isave, int chunkid, bool value);
void ChunkSave(ISave *isave, int chunkid, char *value);
char *ChunkLoadString(ILoad *iload, char *buffer, int maxBytes);
int ChunkLoadInt(ILoad *iload);
bool ChunkLoadBool(ILoad *iload);
void SetICustEdit(HWND wnd, int nIDDlgItem, char *text);
BOOL CALLBACK MaxEggExpOptionsProc( HWND hWnd, UINT message, 
					                          WPARAM wParam, LPARAM lParam );

class MaxEggExpOptions
{
  friend class MaxEggPlugin;
  public:
    enum Anim_Type {
    AT_none,
    AT_model,
    AT_chan,
    AT_pose,
    AT_strobe,
    AT_both
  };

 protected:
  Anim_Type anim_type;
  int sf, ef;
  int minFrame, maxFrame;
  bool dblSided, expWholeScene;
  char shortName[256];

  bool successful;

  ULONG *nodeList;
  int numNodes;
  int maxNodes;

 public:
  bool checked;
  char filename[2048];
  IObjParam *maxInterface;
  bool choosingNodes;
  Anim_Type prev_type;

  MaxEggExpOptions();
  virtual ~MaxEggExpOptions() { delete [] nodeList;}

  bool DoExport(IObjParam *ip, bool autoOverwrite, bool saveLog);
  
  void UpdateUI(HWND hWnd);
  bool UpdateFromUI(HWND hWnd);
  void RefreshNodeList(HWND hWnd);
  void SetAnimRange();

  bool FindNode(ULONG INodeHandle); //returns true if the node is already in the list
  void AddNode(ULONG INodeHandle);
  void RemoveNode(int i);
  void RemoveNodeByHandle(ULONG INodeHandle);
  void ClearNodeList(HWND hWnd);
  void CullBadNodes();

  ULONG GetNode(int i) { return (i >= 0 && i < maxNodes) ? nodeList[i] : ULONG_MAX; }

  IOResult Load(ILoad *iload);
  IOResult Save(ISave *isave);

  //int DoExport(const TCHAR *name,ExpInterface *ei,
	//       Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
};

#endif // __MaxEggExpOptions__H

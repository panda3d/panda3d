/*
  maxEggExpOptions.cxx
  Created by Phillip Saltzman, 2/15/05
  Carnegie Mellon University, Entetainment Technology Center

  This file implements the classes that are used to choose
  what to export from 3D Studio max
*/

#include "maxEggExpOptions.h"

//Disable the forcing int to true or false performance warning
#pragma warning(disable: 4800)

void SetICustEdit(HWND wnd, int nIDDlgItem, char *text)
{ 
  ICustEdit *edit = GetICustEdit(GetDlgItem(wnd, nIDDlgItem));
  edit->SetText(text);
  ReleaseICustEdit(edit);
}

void SetICustEdit(HWND wnd, int nIDDlgItem, int n)
{
  char text[80];
  sprintf(text, "%d", n);
  ICustEdit *edit = GetICustEdit(GetDlgItem(wnd, nIDDlgItem));
  edit->SetText(text);
  ReleaseICustEdit(edit);
}

char *GetICustEditT(HWND wnd)
{
  static char buffer[2084];
  ICustEdit *edit = GetICustEdit(wnd);
  edit->GetText(buffer,2084);
  ReleaseICustEdit(edit);
  return buffer;
}

int GetICustEditI(HWND wnd, BOOL *valid)
{
  ICustEdit *edit = GetICustEdit(wnd);
  int i = edit->GetInt(valid);
  ReleaseICustEdit(edit);
  return i;
}

void ChunkSave(ISave *isave, int chunkid, int value)
{
  ULONG nb;
  isave->BeginChunk(chunkid);
  isave->Write(&value, sizeof(int), &nb);
  isave->EndChunk();
}

void ChunkSave(ISave *isave, int chunkid, ULONG value)
{
  ULONG nb;
  isave->BeginChunk(chunkid);
  isave->Write(&value, sizeof(ULONG), &nb);
  isave->EndChunk();
}

void ChunkSave(ISave *isave, int chunkid, bool value)
{
  ULONG nb;
  isave->BeginChunk(chunkid);
  isave->Write(&value, sizeof(bool), &nb);
  isave->EndChunk();
}

void ChunkSave(ISave *isave, int chunkid, char *value)
{
  ULONG nb;
  isave->BeginChunk(chunkid);
  int bytes = strlen(value) + 1;
  isave->Write(&bytes, sizeof(int), &nb);
  isave->Write(value, bytes, &nb);
  isave->EndChunk();
}

char *ChunkLoadString(ILoad *iload, char *buffer, int maxBytes)
{
  ULONG nb;
  int bytes;
  IOResult res;
  res = iload->Read(&bytes, sizeof(int), &nb);
  assert(res == IO_OK && bytes <= maxBytes);
  res = iload->Read(buffer, bytes, &nb);
  assert(res == IO_OK);
  return buffer;
}

int ChunkLoadInt(ILoad *iload)
{
  ULONG nb;
  int value;
  IOResult res;
  res = iload->Read(&value, sizeof(int), &nb);
  assert(res == IO_OK);
  return value;
}

int ChunkLoadULONG(ILoad *iload)
{
  ULONG nb, value;
  IOResult res;
  res = iload->Read(&value, sizeof(ULONG), &nb);
  assert(res == IO_OK);
  return value;
}

bool ChunkLoadBool(ILoad *iload)
{
  ULONG nb;
  bool value;
  IOResult res;
  res = iload->Read(&value, sizeof(bool), &nb);
  assert(res == IO_OK);
  return value;
}

void showAnimControls(HWND hWnd, BOOL val) {
  ShowWindow(GetDlgItem(hWnd, IDC_EF_LABEL), val);
  ShowWindow(GetDlgItem(hWnd, IDC_EF), val);
  ShowWindow(GetDlgItem(hWnd, IDC_SF_LABEL), val);
  SetWindowText(GetDlgItem(hWnd, IDC_EXP_SEL_FRAMES),
                val ? "Use Range:" : "Use Frame:");
}

void enableAnimControls(HWND hWnd, BOOL val) {
  EnableWindow(GetDlgItem(hWnd, IDC_SF_LABEL), val);
  EnableWindow(GetDlgItem(hWnd, IDC_SF), val);
  EnableWindow(GetDlgItem(hWnd, IDC_EF_LABEL), val);
  EnableWindow(GetDlgItem(hWnd, IDC_EF), val);
}

void enableChooserControls(HWND hWnd, BOOL val) {
  EnableWindow(GetDlgItem(hWnd, IDC_LIST_EXPORT), val);
  EnableWindow(GetDlgItem(hWnd, IDC_ADD_EXPORT), val);
  EnableWindow(GetDlgItem(hWnd, IDC_REMOVE_EXPORT), val);
}

#define ANIM_RAD_NONE 0
#define ANIM_RAD_EXPALL 1
#define ANIM_RAD_EXPSEL 2
#define ANIM_RAD_ALL    3
void enableAnimRadios(HWND hWnd, int mask) {
  EnableWindow(GetDlgItem(hWnd, IDC_EXP_ALL_FRAMES), mask & ANIM_RAD_EXPALL);
  EnableWindow(GetDlgItem(hWnd, IDC_EXP_SEL_FRAMES), mask & ANIM_RAD_EXPSEL);
}

// Handles the work of actually picking the target.
class AddNodeCB : public HitByNameDlgCallback
{
public:
  MaxEggExpOptions *ph; //Pointer to the parent class
  HWND hWnd;            //Handle to the parent dialog

  AddNodeCB (MaxEggExpOptions *instance, HWND wnd) : 
    ph(instance), hWnd(wnd) {}

  virtual TCHAR *dialogTitle() {return _T("Objects to Export");}
  virtual TCHAR *buttonText()  {return _T("Select");}
  virtual int filter(INode *node);
  virtual void proc(INodeTab &nodeTab);
};

//This tells what should be in the list
//Allow only triangular objects, nurbs, and joints
int AddNodeCB::filter(INode *node) {
  if (!node) return 0;
  
  Object *obj = node->EvalWorldState(0).obj;
  Control *c = node->GetTMController();
  NURBSSet getSet;
  bool is_bone = (node->GetBoneNodeOnOff() ||     //True for bones
     (c &&                                          //True for bipeds
     ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
     (c->ClassID() == BIPBODY_CONTROL_CLASS_ID) ||
     (c->ClassID() == FOOTPRINT_CLASS_ID))));

  
  if (IsDlgButtonChecked(hWnd, IDC_ANIMATION) == BST_CHECKED)
    return is_bone && !ph->FindNode(node->GetHandle());
  else
    return (
    !is_bone &&
    ((obj->SuperClassID() == GEOMOBJECT_CLASS_ID && //Allow geometrics
      obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) ||
     (obj->SuperClassID() == SHAPE_CLASS_ID &&      //Allow CV NURBS
      obj->ClassID() == EDITABLE_SURF_CLASS_ID &&
      GetNURBSSet(obj, 0, getSet, TRUE) &&
      getSet.GetNURBSObject(0)->GetType() == kNCVCurve)) &&
    !ph->FindNode(node->GetHandle()));             //Only allow items not already selected
}

//Adds all of the selected items to the list
void AddNodeCB::proc(INodeTab &nodeTab) {
  for (int i = 0; i < nodeTab.Count(); i++)
    ph->AddNode(nodeTab[i]->GetHandle());
  ph->RefreshNodeList(hWnd);
}

//This callback class generates a list of nodes that have previously been selected
class RemoveNodeCB : public HitByNameDlgCallback
{
public:
  MaxEggExpOptions *ph; //Pointer to the parent class
  HWND hWnd;            //Handle to the parent dialog

  RemoveNodeCB (MaxEggExpOptions *instance, HWND wnd) : 
    ph(instance), hWnd(wnd) {}

  virtual TCHAR *dialogTitle() {return _T("Objects to Remove");}
  virtual TCHAR *buttonText()  {return _T("Remove");}
  virtual int filter(INode *node) {return (node && ph->FindNode(node->GetHandle()));}
  virtual void proc(INodeTab &nodeTab);
};


//Adds all of the selected items to the list
void RemoveNodeCB::proc(INodeTab &nodeTab) {
  for (int i = 0; i < nodeTab.Count(); i++)
    ph->RemoveNodeByHandle(nodeTab[i]->GetHandle());
  ph->RefreshNodeList(hWnd);
}

MaxEggExpOptions::MaxEggExpOptions () :
checked(true), anim_type(AT_model), sf(INT_MIN), ef(INT_MIN), 
expWholeScene(true), dblSided(false), numNodes(0), maxNodes(5),
choosingNodes(false), prev_type(AT_model) {
  *filename = NULL;
  nodeList = new ULONG[maxNodes];
}

BOOL CALLBACK MaxEggExpOptionsProc( HWND hWnd, UINT message, 
					                          WPARAM wParam, LPARAM lParam ) 
{
  char tempFilename[2048];
  //We pass in our plugin through the lParam variable. Let's convert it back.
  MaxEggExpOptions *imp = (MaxEggExpOptions*)GetWindowLongPtr(hWnd,GWLP_USERDATA); 
  if ( !imp && message != WM_INITDIALOG ) return FALSE;

  switch(message) 
    {
    // When we start, center the window.
    case WM_INITDIALOG:
      // this line is very necessary to pass the plugin as the lParam
      SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam); 
      ((MaxEggExpOptions*)lParam)->UpdateUI(hWnd);
      return TRUE; break;
    
    case WM_CLOSE:
      EndDialog(hWnd, FALSE);
      return TRUE; break;

    // A control was modified
    case WM_COMMAND:
      //The modified control is found in the lower word of the wParam long.
      switch( LOWORD(wParam) ) {
        case IDC_MODEL:
          if (HIWORD(wParam) == BN_CLICKED) {
            SetWindowText(GetDlgItem(hWnd, IDC_EXPORT_SELECTED),
              "Export Meshes:");
            enableAnimRadios(hWnd, ANIM_RAD_NONE);
            showAnimControls(hWnd, TRUE);
            enableAnimControls(hWnd, FALSE);
            if (imp->prev_type == MaxEggExpOptions::AT_chan) imp->ClearNodeList(hWnd);
            imp->prev_type = MaxEggExpOptions::AT_model;
            return TRUE;
          }
          break;

        case IDC_ANIMATION:
          if (HIWORD(wParam) == BN_CLICKED) {
            SetWindowText(GetDlgItem(hWnd, IDC_EXPORT_SELECTED),
              "Export Bones:");
            enableAnimRadios(hWnd, ANIM_RAD_ALL);
            showAnimControls(hWnd, TRUE);
            enableAnimControls(hWnd, IsDlgButtonChecked(hWnd, IDC_EXP_SEL_FRAMES));
            if (imp->prev_type != MaxEggExpOptions::AT_chan) imp->ClearNodeList(hWnd);
            imp->prev_type = MaxEggExpOptions::AT_chan;
            return TRUE;
          }
          break;
        case IDC_BOTH:
          if (HIWORD(wParam) == BN_CLICKED) {
            SetWindowText(GetDlgItem(hWnd, IDC_EXPORT_SELECTED),
              "Export Models:");
            enableAnimRadios(hWnd, ANIM_RAD_ALL);
            showAnimControls(hWnd, TRUE);
            enableAnimControls(hWnd, IsDlgButtonChecked(hWnd, IDC_EXP_SEL_FRAMES));
            if (imp->prev_type == MaxEggExpOptions::AT_chan) imp->ClearNodeList(hWnd);
            imp->prev_type = MaxEggExpOptions::AT_both;
            return TRUE;
          }
          break;
        case IDC_POSE:
          if (HIWORD(wParam) == BN_CLICKED) {
            SetWindowText(GetDlgItem(hWnd, IDC_EXPORT_SELECTED),
              "Export Meshes:");
            enableAnimRadios(hWnd, ANIM_RAD_EXPSEL);
            showAnimControls(hWnd, FALSE);
            enableAnimControls(hWnd, TRUE);
            CheckRadioButton(hWnd, IDC_EXP_ALL_FRAMES, IDC_EXP_SEL_FRAMES, IDC_EXP_SEL_FRAMES);
            if (imp->prev_type == MaxEggExpOptions::AT_chan) imp->ClearNodeList(hWnd);
            imp->prev_type = MaxEggExpOptions::AT_pose;
            return TRUE;
          }
          break;
        case IDC_EXP_ALL_FRAMES:
          if (HIWORD(wParam) == BN_CLICKED) {
            enableAnimControls(hWnd, FALSE);
            return TRUE;
          }
          break;

        case IDC_EXP_SEL_FRAMES:
          if (HIWORD(wParam) == BN_CLICKED) {
            enableAnimControls(hWnd, TRUE);
            return TRUE;
          }
          break;

        case IDC_EXPORT_ALL:
          if (HIWORD(wParam) == BN_CLICKED) {
            enableChooserControls(hWnd, FALSE);
            return TRUE;
          }
          break;

        case IDC_EXPORT_SELECTED:
          if (HIWORD(wParam) == BN_CLICKED) {
            enableChooserControls(hWnd, TRUE);
            return TRUE;
          }
          break;

        case IDC_ADD_EXPORT:
        {
          if (!imp->choosingNodes) {
            AddNodeCB PickCB(imp, hWnd);
            imp->choosingNodes = true;
            imp->maxInterface->DoHitByNameDialog(&PickCB);
            imp->choosingNodes = false;
          }
        }
        return TRUE; break;

        case IDC_REMOVE_EXPORT:
        {
          if (!imp->choosingNodes) {
            imp->choosingNodes = true;
            RemoveNodeCB PickCB(imp, hWnd);
            imp->maxInterface->DoHitByNameDialog(&PickCB);
            imp->choosingNodes = false;
          }
        }
        return TRUE; break;

        case IDC_OK:
          if (imp->UpdateFromUI(hWnd)) EndDialog(hWnd, TRUE);
          return TRUE; break;
        case IDC_CANCEL:
          EndDialog(hWnd, FALSE);
          return TRUE; break;
        case IDC_BROWSE:
          OPENFILENAME ofn;
          strcpy(tempFilename, GetICustEditT(GetDlgItem(hWnd, IDC_FILENAME)));

          memset(&ofn, 0, sizeof(ofn));
          ofn.nMaxFile = 2047;
          ofn.lpstrFile = tempFilename;
          ofn.lStructSize = sizeof(ofn);
          ofn.hwndOwner = hWnd;
          ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
          ofn.lpstrDefExt = "egg";
          ofn.lpstrFilter = "Panda3D Egg Files (*.egg)\0*.egg\0All Files (*.*)\0*.*\0";

          SetFocus(GetDlgItem(hWnd, IDC_FILENAME));
          if (GetSaveFileName(&ofn))
            SetICustEdit(hWnd, IDC_FILENAME, ofn.lpstrFile);
          //else {
          //  char buf[255];
          //  sprintf(buf, "%d", CommDlgExtendedError());
          //  MessageBox(hWnd, buf, "Error on GetSaveFileName", MB_OK);
          //}
          return TRUE; break;
        case IDC_CHECK1:
          if (IsDlgButtonChecked(hWnd, IDC_CHECK1))
            if (MessageBox(hWnd, "Warning: Exporting double-sided polygons can severely hurt "
              "performance in Panda3D.\n\nAre you sure you want to export them?",
              "Panda3D Exporter", MB_YESNO | MB_ICONQUESTION) != IDYES)
              CheckDlgButton(hWnd, IDC_CHECK1, BST_UNCHECKED);
          return TRUE; break;
        default:
          //char buf[255];
          //sprintf(buf, "%d", LOWORD(wParam));
          //MessageBox(hWnd, buf, "Unknown WParam", MB_OK);
          break;
      }
    }
  return FALSE;
}

void MaxEggExpOptions::SetAnimRange() {
  // Get the start and end frames and the animation frame rate from Max
  Interval anim_range = maxInterface->GetAnimRange();
  minFrame = anim_range.Start()/GetTicksPerFrame();
  maxFrame = anim_range.End()/GetTicksPerFrame();
}

void MaxEggExpOptions::UpdateUI(HWND hWnd) {
  int typeButton = IDC_MODEL;
  int anim_exp = sf == INT_MIN ? IDC_EXP_ALL_FRAMES : IDC_EXP_SEL_FRAMES;
  int model_exp = expWholeScene ? IDC_EXPORT_ALL : IDC_EXPORT_SELECTED;

  switch (anim_type) {
    case AT_chan: typeButton = IDC_ANIMATION; break;
    case AT_both: typeButton = IDC_BOTH; break;
    case AT_pose: typeButton = IDC_POSE; break;
  }

  prev_type = anim_type;

  CheckRadioButton(hWnd, IDC_MODEL, IDC_POSE, typeButton);
  SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(typeButton, BN_CLICKED), 0);
  CheckRadioButton(hWnd, IDC_EXPORT_ALL, IDC_EXPORT_SELECTED, model_exp);
  SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(model_exp, BN_CLICKED), 0);
  CheckRadioButton(hWnd, IDC_EXP_ALL_FRAMES, IDC_EXP_SEL_FRAMES, anim_exp);
  SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(anim_exp, BN_CLICKED), 0);

  CheckDlgButton(hWnd, IDC_CHECK1,
                 dblSided ? BST_CHECKED : BST_UNCHECKED);

  SetICustEdit(hWnd, IDC_FILENAME, filename);
  if (sf != INT_MIN) {
    SetICustEdit(hWnd, IDC_SF, sf);
    SetICustEdit(hWnd, IDC_EF, ef);
  }
  else {
    SetICustEdit(hWnd, IDC_SF, minFrame);
    SetICustEdit(hWnd, IDC_EF, maxFrame);
  }

  RefreshNodeList(hWnd);
}

void MaxEggExpOptions::ClearNodeList(HWND hWnd) {
  numNodes = 0;
  RefreshNodeList(hWnd);
}

void MaxEggExpOptions::RefreshNodeList(HWND hWnd) {
  //Clear and repopulate the node box
  HWND nodeLB = GetDlgItem(hWnd, IDC_LIST_EXPORT);
  SendMessage(nodeLB, LB_RESETCONTENT, 0, 0);
  for (int i = 0; i < numNodes; i++) {
    INode *temp = maxInterface->GetINodeByHandle(GetNode(i));
    TCHAR *name = _T("Unknown Node");
    if (temp) name = temp->GetName();
    SendMessage(nodeLB, LB_ADDSTRING, 0, (LPARAM)name);
  }
}

bool MaxEggExpOptions::UpdateFromUI(HWND hWnd) {
  BOOL valid;
  Anim_Type newAnimType;
  int newSF = INT_MIN, newEF = INT_MIN;
  char msg[1024];

  if (IsDlgButtonChecked(hWnd, IDC_MODEL))          newAnimType = AT_model;
  else if (IsDlgButtonChecked(hWnd, IDC_ANIMATION)) newAnimType = AT_chan;
  else if (IsDlgButtonChecked(hWnd, IDC_BOTH))      newAnimType = AT_both;
  else                                              newAnimType = AT_pose;

  if (newAnimType != AT_model && IsDlgButtonChecked(hWnd, IDC_EXP_SEL_FRAMES)) {
    newSF = GetICustEditI(GetDlgItem(hWnd, IDC_SF), &valid);
    if (!valid) {
      MessageBox(hWnd, "Start Frame must be an integer", "Invalid Value", MB_OK | MB_ICONEXCLAMATION);
      return false;
    }
    if (newSF < minFrame) {
      sprintf(msg, "Start Frame must be at least %d", minFrame);
      MessageBox(hWnd, msg, "Invalid Value", MB_OK | MB_ICONEXCLAMATION);
      return false;
    }
    if (newSF > maxFrame) {
      sprintf(msg, "Start Frame must be at most %d", maxFrame);
      MessageBox(hWnd, msg, "Invalid Value", MB_OK | MB_ICONEXCLAMATION);
      return false;
    }
    if (newAnimType != AT_pose) {
      newEF = GetICustEditI(GetDlgItem(hWnd, IDC_EF), &valid);
      if (!valid) {
        MessageBox(hWnd, "End Frame must be an integer", "Invalid Value", MB_OK | MB_ICONEXCLAMATION);
        return false;
      }
      if (newEF > maxFrame) {
        sprintf(msg, "End Frame must be at most %d", maxFrame);
        MessageBox(hWnd, msg, "Invalid Value", MB_OK | MB_ICONEXCLAMATION);
        return false;
      }
      if (newEF < newSF) {
        MessageBox(hWnd, "End Frame must be greater than the start frame", "Invalid Value", MB_OK | MB_ICONEXCLAMATION);
        return false;
      }
    }
  }

  char *temp = GetICustEditT(GetDlgItem(hWnd, IDC_FILENAME));
  if (!strlen(temp)) {
    MessageBox(hWnd, "The filename cannot be empty", "Invalid Value", MB_OK | MB_ICONEXCLAMATION);
    return false;
  }

  if (strlen(temp) < 4 || strncmp(".egg", temp+(strlen(temp) - 4), 4))
    sprintf(filename, "%s.egg", temp);
  else strcpy(filename, temp);

  temp = strrchr(filename, '\\');
  if (!temp) temp = filename;
  else temp++;

  if (strlen(temp) > sizeof(shortName))
    sprintf(shortName, "%.*s...", sizeof(shortName)-4, temp);
  else {
    strcpy(shortName, temp);
    shortName[strlen(shortName) - 4] = NULL; //Cut off the .egg
  }
  
  dblSided = IsDlgButtonChecked(hWnd, IDC_CHECK1);
  expWholeScene = IsDlgButtonChecked(hWnd, IDC_EXPORT_ALL);
  sf = newSF; ef = newEF; anim_type = newAnimType;
  return true;
}

bool MaxEggExpOptions::FindNode(ULONG INodeHandle) {
  for (int i = 0; i < numNodes; i++) 
    if (nodeList[i] == INodeHandle) return true;
  return false;
}

void MaxEggExpOptions::AddNode(ULONG INodeHandle) {
  if (FindNode(INodeHandle)) return; 
  if (numNodes >= maxNodes) {
    ULONG *newList;
    maxNodes *= 2;
    newList = new ULONG[maxNodes];
    for (int i = 0; i < numNodes; i++) newList[i] = nodeList[i];
    delete [] nodeList;
    nodeList = newList;
  }

  nodeList[numNodes++] = INodeHandle;
}

void MaxEggExpOptions::CullBadNodes() {
  if (!maxInterface) return;
  int i = 0, j = 0;
  while (j < numNodes) {
    if (!maxInterface->GetINodeByHandle(nodeList[i])) j++;
    else nodeList[i++] = nodeList[j++];
  }
  numNodes = i;
}

void MaxEggExpOptions::RemoveNode(int i) {
  if (i >= 0 && i < numNodes) {
    for (int j = i+1; j < numNodes;) nodeList[i++] = nodeList[j++];
    --numNodes;
  }
}

void MaxEggExpOptions::RemoveNodeByHandle(ULONG INodeHandle) {
  for (int i = 0; i < numNodes; i++)
    if (nodeList[i] == INodeHandle) {
      RemoveNode(i);
      return;
    }
}

IOResult MaxEggExpOptions::Save(ISave *isave) {
  isave->BeginChunk(CHUNK_EGG_EXP_OPTIONS);
  ChunkSave(isave, CHUNK_ANIM_TYPE, anim_type);
  ChunkSave(isave, CHUNK_FILENAME, filename);
  ChunkSave(isave, CHUNK_SHORTNAME, shortName);
  ChunkSave(isave, CHUNK_SF, sf);
  ChunkSave(isave, CHUNK_EF, ef);
  ChunkSave(isave, CHUNK_DBL_SIDED, dblSided);
  ChunkSave(isave, CHUNK_EGG_CHECKED, checked);
  ChunkSave(isave, CHUNK_EXPORT_FULL, expWholeScene);
  isave->BeginChunk(CHUNK_NODE_LIST);
  for (int i = 0; i < numNodes; i++)
    ChunkSave(isave, CHUNK_NODE_HANDLE, nodeList[i]);
  isave->EndChunk();
  isave->EndChunk();

  return IO_OK;
} 

IOResult MaxEggExpOptions::Load(ILoad *iload) {
  IOResult res = iload->OpenChunk();
  
  while (res == IO_OK) {
    switch(iload->CurChunkID()) {
    case CHUNK_ANIM_TYPE: anim_type = (Anim_Type)ChunkLoadInt(iload); break;
    case CHUNK_FILENAME: ChunkLoadString(iload, filename, sizeof(filename)); break;
    case CHUNK_SHORTNAME: ChunkLoadString(iload, shortName, sizeof(shortName)); break;
    case CHUNK_SF: sf = ChunkLoadInt(iload); break;
    case CHUNK_EF: ef = ChunkLoadInt(iload); break;
    case CHUNK_DBL_SIDED: dblSided = ChunkLoadBool(iload); break;
    case CHUNK_EGG_CHECKED: checked = ChunkLoadBool(iload); break;
    case CHUNK_EXPORT_FULL: expWholeScene = ChunkLoadBool(iload); break;
    case CHUNK_NODE_LIST:
      res = iload->OpenChunk();
      while (res == IO_OK) {
        if (iload->CurChunkID() == CHUNK_NODE_HANDLE) AddNode(ChunkLoadULONG(iload));
        iload->CloseChunk();
        res = iload->OpenChunk();
      }
    break;
    }
    iload->CloseChunk();
    res = iload->OpenChunk();
  }
  
  if (res == IO_END) return IO_OK;
  return IO_ERROR;
}

/*!
 * This method creates and triggers the exporter.  Basically it takes the
 * user's options and builds a command-line parameter list from it.
 * It then invokes the converter pretending it was invoked as a standalone
 * program.  BIG WARNING:  The converter stuff often does exit() if the
 * command line arguments are displeasing.
 */
bool MaxEggExpOptions::DoExport(IObjParam *ip, bool autoOverwrite, bool saveLog) 
{
  MaxToEgg *pmteConverter = new MaxToEgg();
  char *apcParameters[64];
  char pszSF[10], pszEF[10];
  char acOutputFilename[MAX_PATH];
  char curFilename[MAX_PATH];
  int iParameterCount=0;

  //Initialize our global error logger
  if (saveLog)
    Logger::globalLoggingInstance = new Logger( Logger::PIPE_TO_FILE, "MaxEggLog.txt" );
  else Logger::globalLoggingInstance = new Logger( Logger::PIPE_TO_DEV_NULL, "" );

  //Set the various logging levels for the subsystems.
  Logger::SetOneErrorMask( ME, Logger::SAT_ALL );
  Logger::SetOneErrorMask( MTE, Logger::SAT_NULL_ERROR | 
			   Logger::SAT_CRITICAL_ERROR | 
			   Logger::SAT_PARAMETER_INVALID_ERROR | 
			   Logger::SAT_OTHER_ERROR | Logger::SAT_HIGH_LEVEL );
  Logger::SetOneErrorMask( MTEC, Logger::SAT_NULL_ERROR | 
			   Logger::SAT_CRITICAL_ERROR |
			   Logger::SAT_PARAMETER_INVALID_ERROR | 
			   Logger::SAT_OTHER_ERROR | Logger::SAT_HIGH_LEVEL |
			   Logger::SAT_MEDIUM_LEVEL | Logger::SAT_LOW_LEVEL |
			   Logger::SAT_DEBUG_SPAM_LEVEL );
  Logger::SetOneErrorMask( MNEG, Logger::SAT_NULL_ERROR |
			   Logger::SAT_CRITICAL_ERROR |
			   Logger::SAT_PARAMETER_INVALID_ERROR | 
			   Logger::SAT_OTHER_ERROR | Logger::SAT_HIGH_LEVEL |
			   Logger::SAT_MEDIUM_LEVEL | Logger::SAT_LOW_LEVEL );
  Logger::SetOneErrorMask( MNEG_GEOMETRY_GENERATION, Logger::SAT_NULL_ERROR |
			   Logger::SAT_CRITICAL_ERROR |
			   Logger::SAT_PARAMETER_INVALID_ERROR | 
			   Logger::SAT_OTHER_ERROR | Logger::SAT_HIGH_LEVEL |
			   Logger::SAT_MEDIUM_LEVEL | Logger::SAT_LOW_LEVEL );
  Logger::SetOneErrorMask( LLOGGING, Logger::SAT_ALL );
	
  Logger::FunctionEntry( "MaxEggPlugin::DoExport" );

  // Copy the output filename so that it can be modified if necessary
  strncpy(acOutputFilename, filename, MAX_PATH-1);
  acOutputFilename[MAX_PATH-1]=0;

  // Panda reaaaaaaaaly wants the extension to be in lower case.
  // So if we see a .egg at the end, lower case it.
  if ((strlen(acOutputFilename)>4) &&
      (stricmp(acOutputFilename+strlen(acOutputFilename)-4,".egg")==0)) {
    strlwr(acOutputFilename+strlen(acOutputFilename)-4);
  }
  pmteConverter->SetMaxInterface((Interface*)ip);
  
  // Set the command-line arguments
  // ARGV[0] = program name
  apcParameters[iParameterCount++]="MaxEggPlugin";
  
  // ARGV[1] = Input file
  // Use a bogus input filename that exists
  apcParameters[iParameterCount++]="nul.max";
  
  // ARGV[2,3] = Animation options
  // Check if there is an animation to be saved and what type of animation it
  // will be saved as.  Then set the animation option.
  apcParameters[iParameterCount++]="-a";
  switch (anim_type) {
    case AT_chan: apcParameters[iParameterCount++]="chan"; break;
    case AT_pose: apcParameters[iParameterCount++]="pose"; break;
    case AT_both: apcParameters[iParameterCount++]="both"; break;
    case AT_model: 
    default: apcParameters[iParameterCount++]="model"; break;
  }

  //Start Frame
  //If the export options need a start frame and we have one
  //then use it
  if (sf != INT_MIN && anim_type != AT_model) {
    sprintf(pszSF, "%d", sf);
    apcParameters[iParameterCount++] = "-sf";
    apcParameters[iParameterCount++] = pszSF;
  }

  //Start Frame
  //If the export options need an end frame and we have one
  //then use it
  if (sf != INT_MIN && (anim_type == AT_chan || anim_type == AT_both)) {
    sprintf(pszEF, "%d", ef);
    apcParameters[iParameterCount++] = "-ef";
    apcParameters[iParameterCount++] = pszEF;
  }

  // Doublesided: This option may be diabled in the converter
  // but this is here for when it is enabled
  if (dblSided) apcParameters[iParameterCount++] = "-bface";

  // Final ARGV = Output file
  // Pass in the output filename
  // Output file has to be passed in with the -o parameter in order to be able 
  // to overwrite an existing file
  if (autoOverwrite) apcParameters[iParameterCount++]="-o";
  apcParameters[iParameterCount++]=acOutputFilename;

  apcParameters[iParameterCount]=0;

  // Parse the command line and run the converter
  pmteConverter->parse_command_line(iParameterCount, apcParameters);
  if (expWholeScene) pmteConverter->Run(NULL, 0);
  else pmteConverter->Run(nodeList, numNodes);
  
  successful = pmteConverter->IsSuccessful();
  
  // This was put in try block because originally deleting pmteConverter 
  // would throw an exception.  That no longer happens, but this is still
  // here for good measure
  try {
		Logger::Log(MTEC, Logger::SAT_MEDIUM_LEVEL, "before deleting pmteconverter");
    delete pmteConverter; 
  } catch (...) {
		Logger::Log(MTEC, Logger::SAT_MEDIUM_LEVEL, "before error message window");
    MessageBox(ip->GetMAXHWnd(), "I just got an unknown exception.",
	       "Panda3D Converter", MB_OK);
  }
		Logger::Log(MTEC, Logger::SAT_MEDIUM_LEVEL, "before logger function exit");
  Logger::FunctionExit();
  //Free the error logger
  if ( Logger::globalLoggingInstance )
    delete Logger::globalLoggingInstance;
 
  return successful;
}

/*
  maxEggExpOptions.cxx
  Created by Phillip Saltzman, 2/15/05
  Carnegie Mellon University, Entetainment Technology Center

  This file implements the classes that are used to choose
  what to export from 3D Studio max

  Updated by Fei Wang, Carnegie Mellon University Entertainment
  Technology Center student, 14Aug2009:  added enableAddCollisionChoices
*/

#include "maxEgg.h"

// Disable the forcing int to true or false performance warning
#pragma warning(disable: 4800)

void SetICustEdit(HWND wnd, int nIDDlgItem, TCHAR *text)
{
  ICustEdit *edit = GetICustEdit(GetDlgItem(wnd, nIDDlgItem));
  edit->SetText(text);
  ReleaseICustEdit(edit);
}

void SetICustEdit(HWND wnd, int nIDDlgItem, int n)
{
  TCHAR text[80];
  _stprintf(text, _T("%d"), n);
  ICustEdit *edit = GetICustEdit(GetDlgItem(wnd, nIDDlgItem));
  edit->SetText(text);
  ReleaseICustEdit(edit);
}

TCHAR *GetICustEditT(HWND wnd)
{
  static TCHAR buffer[2084];
  ICustEdit *edit = GetICustEdit(wnd);
  edit->GetText(buffer, 2084);
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

void ChunkSave(ISave *isave, int chunkid, TCHAR *value)
{
  ULONG nb;
  isave->BeginChunk(chunkid);
  int length = _tcslen(value) + 1;
  isave->Write(&length, sizeof(int), &nb);
  isave->Write(value, length * sizeof(TCHAR), &nb);
  isave->EndChunk();
}

TCHAR *ChunkLoadString(ILoad *iload, TCHAR *buffer, int maxLength)
{
  ULONG nb;
  int length;
  IOResult res;
  res = iload->Read(&length, sizeof(int), &nb);
  assert(res == IO_OK && length <= maxLength);
  res = iload->Read(buffer, length * sizeof(TCHAR), &nb);
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
                val ? _T("Use Range:") : _T("Use Frame:"));
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
  MaxOptionsDialog *ph; //Pointer to the parent class
  HWND hWnd;            //Handle to the parent dialog

  AddNodeCB (MaxOptionsDialog *instance, HWND wnd) :
    ph(instance), hWnd(wnd) {}

#if MAX_VERSION_MAJOR < 15
  virtual TCHAR *dialogTitle() {return _T("Objects to Export");}
  virtual TCHAR *buttonText()  {return _T("Select");}
#else
  virtual const MCHAR *dialogTitle() {return _M("Objects to Export");}
  virtual const MCHAR *buttonText()  {return _M("Select");}
#endif

  virtual int filter(INode *node);
  virtual void proc(INodeTab &nodeTab);
};

// This tells what should be in the list Allow only triangular objects, nurbs,
// and joints
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
    is_bone || ((obj->SuperClassID() == HELPER_CLASS_ID && obj->ClassID() != MaxEggPlugin_CLASS_ID)) ||
    ((obj->SuperClassID() == GEOMOBJECT_CLASS_ID && //Allow geometrics
      obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) ||
     (obj->SuperClassID() == SHAPE_CLASS_ID &&      //Allow CV NURBS
      obj->ClassID() == EDITABLE_SURF_CLASS_ID &&
      GetNURBSSet(obj, 0, getSet, TRUE) &&
      getSet.GetNURBSObject(0)->GetType() == kNCVCurve)) &&
    !ph->FindNode(node->GetHandle()));             //Only allow items not already selected
}

// Adds all of the selected items to the list
void AddNodeCB::proc(INodeTab &nodeTab) {

  for (int i = 0; i < nodeTab.Count(); i++)
    ph->AddNode(nodeTab[i]->GetHandle());
  ph->RefreshNodeList(hWnd);
}

// This callback class generates a list of nodes that have previously been
// selected
class RemoveNodeCB : public HitByNameDlgCallback
{
public:
    MaxOptionsDialog *ph; //Pointer to the parent class
    HWND hWnd;            //Handle to the parent dialog

    RemoveNodeCB (MaxOptionsDialog *instance, HWND wnd) :
        ph(instance), hWnd(wnd) {}

#if MAX_VERSION_MAJOR < 15
    virtual TCHAR *dialogTitle() {return _T("Objects to Remove");}
    virtual TCHAR *buttonText()  {return _T("Remove");}
#else
    virtual const MCHAR *dialogTitle() {return _M("Objects to Remove");}
    virtual const MCHAR *buttonText()  {return _M("Remove");}
#endif

    virtual int filter(INode *node) {return (node && ph->FindNode(node->GetHandle()));}
    virtual void proc(INodeTab &nodeTab);
};


// Adds all of the selected items to the list
void RemoveNodeCB::proc(INodeTab &nodeTab) {
    for (int i = 0; i < nodeTab.Count(); i++)
        ph->RemoveNodeByHandle(nodeTab[i]->GetHandle());
    ph->RefreshNodeList(hWnd);
}

MaxEggOptions::MaxEggOptions() {
    _max_interface = nullptr;
    _anim_type = MaxEggOptions::AT_model;
    _start_frame = INT_MIN;
    _end_frame = INT_MIN;
    _double_sided = false;


    _file_name[0]=0;
    _short_name[0]=0;
    _path_replace = new PathReplace;
    _path_replace->_path_store = PS_relative;
    _export_whole_scene = true;
    _export_all_frames = true;
    _successful = false;
}

INT_PTR CALLBACK MaxOptionsDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    TCHAR tempFilename[2048];
    // We pass in our plugin through the lParam variable.  Let's convert it
    // back.
    MaxOptionsDialog *imp = (MaxOptionsDialog*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
    if ( !imp && message != WM_INITDIALOG ) return FALSE;

    switch(message) {

    case WM_INITDIALOG:
        // this line is very necessary to pass the plugin as the lParam
        SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
        ((MaxOptionsDialog*)lParam)->UpdateUI(hWnd);

        return TRUE; break;

    case WM_CLOSE:
        EndDialog(hWnd, FALSE);
        return TRUE; break;

    case WM_COMMAND:
        // The modified control is found in the lower word of the wParam long.
        switch( LOWORD(wParam) ) {
        case IDC_MODEL:
            if (HIWORD(wParam) == BN_CLICKED) {
                SetWindowText(GetDlgItem(hWnd, IDC_EXPORT_SELECTED),
                              _T("Export Meshes:"));
                enableAnimRadios(hWnd, ANIM_RAD_NONE);
                showAnimControls(hWnd, TRUE);
                enableAnimControls(hWnd, FALSE);
                if (imp->_prev_type == MaxEggOptions::AT_chan) imp->ClearNodeList(hWnd);
                imp->_prev_type = MaxEggOptions::AT_model;

                return TRUE;
            }
            break;

        case IDC_ANIMATION:
            if (HIWORD(wParam) == BN_CLICKED) {
                SetWindowText(GetDlgItem(hWnd, IDC_EXPORT_SELECTED),
                              _T("Export Bones:"));
                enableAnimRadios(hWnd, ANIM_RAD_ALL);
                showAnimControls(hWnd, TRUE);
                enableAnimControls(hWnd, IsDlgButtonChecked(hWnd, IDC_EXP_SEL_FRAMES));
                if (imp->_prev_type != MaxEggOptions::AT_chan) imp->ClearNodeList(hWnd);
                imp->_prev_type = MaxEggOptions::AT_chan;
                CheckRadioButton(hWnd, IDC_EXP_ALL_FRAMES, IDC_EXP_SEL_FRAMES, IDC_EXP_ALL_FRAMES);

                return TRUE;
            }
            break;
        case IDC_BOTH:
            if (HIWORD(wParam) == BN_CLICKED) {
                SetWindowText(GetDlgItem(hWnd, IDC_EXPORT_SELECTED),
                              _T("Export Models:"));
                enableAnimRadios(hWnd, ANIM_RAD_ALL);
                showAnimControls(hWnd, TRUE);
                enableAnimControls(hWnd, IsDlgButtonChecked(hWnd, IDC_EXP_SEL_FRAMES));
                if (imp->_prev_type == MaxEggOptions::AT_chan) imp->ClearNodeList(hWnd);
                imp->_prev_type = MaxEggOptions::AT_both;
                CheckRadioButton(hWnd, IDC_EXP_ALL_FRAMES, IDC_EXP_SEL_FRAMES, IDC_EXP_ALL_FRAMES);

                return TRUE;
            }
            break;
        case IDC_POSE:
            if (HIWORD(wParam) == BN_CLICKED) {
                SetWindowText(GetDlgItem(hWnd, IDC_EXPORT_SELECTED),
                              _T("Export Meshes:"));
                enableAnimRadios(hWnd, ANIM_RAD_EXPSEL);
                showAnimControls(hWnd, FALSE);
                enableAnimControls(hWnd, TRUE);
                CheckRadioButton(hWnd, IDC_EXP_ALL_FRAMES, IDC_EXP_SEL_FRAMES, IDC_EXP_SEL_FRAMES);
                if (imp->_prev_type == MaxEggOptions::AT_chan) imp->ClearNodeList(hWnd);
                imp->_prev_type = MaxEggOptions::AT_pose;

                return TRUE;
            }
            break;
        case IDC_EXP_ALL_FRAMES:
            if (HIWORD(wParam) == BN_CLICKED) {
                enableAnimControls(hWnd, FALSE);
                if(imp->_prev_type == MaxEggOptions::AT_chan)
                {
                  CheckRadioButton(hWnd, IDC_MODEL, IDC_POSE, IDC_ANIMATION);
                }
                if(imp->_prev_type == MaxEggOptions::AT_both)
                {
                  CheckRadioButton(hWnd, IDC_MODEL, IDC_POSE, IDC_BOTH);
                }
                return TRUE;
            }
            break;

        case IDC_EXP_SEL_FRAMES:
            if (HIWORD(wParam) == BN_CLICKED) {
                enableAnimControls(hWnd, TRUE);
                if(imp->_prev_type == MaxEggOptions::AT_chan)
                {
                  CheckRadioButton(hWnd, IDC_MODEL, IDC_POSE, IDC_ANIMATION);
                }
                if(imp->_prev_type == MaxEggOptions::AT_both)
                {
                  CheckRadioButton(hWnd, IDC_MODEL, IDC_POSE, IDC_BOTH);
                }
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


        // deal with adding meshes
        case IDC_ADD_EXPORT:
            {
                if (!imp->_choosing_nodes) {
                    AddNodeCB PickCB(imp, hWnd);
                    imp->_choosing_nodes = true;
                    imp->_max_interface->DoHitByNameDialog(&PickCB);
                    imp->_choosing_nodes = false;
                }
            }
            return TRUE; break;

        case IDC_REMOVE_EXPORT:
            {
                if (!imp->_choosing_nodes) {
                    imp->_choosing_nodes = true;
                    RemoveNodeCB PickCB(imp, hWnd);
                    imp->_max_interface->DoHitByNameDialog(&PickCB);
                    imp->_choosing_nodes = false;
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
            _tcscpy(tempFilename, GetICustEditT(GetDlgItem(hWnd, IDC_FILENAME)));

            memset(&ofn, 0, sizeof(ofn));
            ofn.nMaxFile = 2047;
            ofn.lpstrFile = tempFilename;
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
            ofn.lpstrDefExt = _T("egg");
            ofn.lpstrFilter = _T("Panda3D Egg Files (*.egg)\0*.egg\0All Files (*.*)\0*.*\0");

            SetFocus(GetDlgItem(hWnd, IDC_FILENAME));
            if (GetSaveFileName(&ofn))
                SetICustEdit(hWnd, IDC_FILENAME, ofn.lpstrFile);
            // else { char buf[255]; sprintf(buf, "%d",
            // CommDlgExtendedError()); MessageBox(hWnd, buf, "Error on
            // GetSaveFileName", MB_OK); }
            return TRUE; break;
        case IDC_CHECK1:
            if (IsDlgButtonChecked(hWnd, IDC_CHECK1))
                if (MessageBox(hWnd, _T("Warning: Exporting double-sided polygons can severely hurt ")
                               _T("performance in Panda3D.\n\nAre you sure you want to export them?"),
                               _T("Panda3D Exporter"), MB_YESNO | MB_ICONQUESTION) != IDYES)
                    CheckDlgButton(hWnd, IDC_CHECK1, BST_UNCHECKED);
            return TRUE; break;

        default:
            // char buf[255]; sprintf(buf, "%d", LOWORD(wParam));
            // MessageBox(hWnd, buf, "Unknown WParam", MB_OK);
            break;
        }
    }
    return FALSE;
}

void MaxOptionsDialog::SetAnimRange() {
    // Get the start and end frames and the animation frame rate from Max
    Interval anim_range = _max_interface->GetAnimRange();
    _min_frame = anim_range.Start()/GetTicksPerFrame();
    _max_frame = anim_range.End()/GetTicksPerFrame();
}

MaxOptionsDialog::MaxOptionsDialog() :
    MaxEggOptions(),
    _min_frame(0),
    _max_frame(0),
    _checked(true),
    _choosing_nodes(false),
    _prev_type(AT_model)
{
}

MaxOptionsDialog::~MaxOptionsDialog ()
{
}


void MaxOptionsDialog::UpdateUI(HWND hWnd) {
    int typeButton = IDC_MODEL;
    int anim_exp = _export_all_frames ? IDC_EXP_ALL_FRAMES : IDC_EXP_SEL_FRAMES;
    int model_exp = _export_whole_scene ? IDC_EXPORT_ALL : IDC_EXPORT_SELECTED;

    switch (_anim_type) {
    case MaxEggOptions::AT_chan:  typeButton = IDC_ANIMATION; break;
    case MaxEggOptions::AT_both:  typeButton = IDC_BOTH; break;
    case MaxEggOptions::AT_pose:  typeButton = IDC_POSE; break;
    case MaxEggOptions::AT_model: typeButton = IDC_MODEL; break;
    }

    _prev_type = _anim_type;

    CheckRadioButton(hWnd, IDC_MODEL, IDC_POSE, typeButton);
    SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(typeButton, BN_CLICKED), 0);
    CheckRadioButton(hWnd, IDC_EXPORT_ALL, IDC_EXPORT_SELECTED, model_exp);
    SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(model_exp, BN_CLICKED), 0);
    CheckRadioButton(hWnd, IDC_EXP_ALL_FRAMES, IDC_EXP_SEL_FRAMES, anim_exp);
    SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(anim_exp, BN_CLICKED), 0);

    CheckDlgButton(hWnd, IDC_CHECK1,
                   _double_sided ? BST_CHECKED : BST_UNCHECKED);


    SetICustEdit(hWnd, IDC_FILENAME, _file_name);
    if (_start_frame != INT_MIN) {
        SetICustEdit(hWnd, IDC_SF, _start_frame);
        SetICustEdit(hWnd, IDC_EF, _end_frame);
    } else {
        SetICustEdit(hWnd, IDC_SF, _min_frame);
        SetICustEdit(hWnd, IDC_EF, _max_frame);
    }

    RefreshNodeList(hWnd);
}

void MaxOptionsDialog::ClearNodeList(HWND hWnd) {
    _node_list.clear();
    RefreshNodeList(hWnd);
}

void MaxOptionsDialog::RefreshNodeList(HWND hWnd) {
  // Clear and repopulate the node box
  HWND nodeLB = GetDlgItem(hWnd, IDC_LIST_EXPORT);
  SendMessage(nodeLB, LB_RESETCONTENT, 0, 0);
  for (int i = 0; i < _node_list.size(); i++) {
      INode *temp = _max_interface->GetINodeByHandle(_node_list[i]);
      const TCHAR *name = _T("Unknown Node");
      if (temp) name = temp->GetName();
      SendMessage(nodeLB, LB_ADDSTRING, 0, (LPARAM)name);
  }
}

bool MaxOptionsDialog::UpdateFromUI(HWND hWnd) {
  BOOL valid;
  Anim_Type newAnimType;
  int newSF = INT_MIN, newEF = INT_MIN;
  TCHAR msg[1024];

  if (IsDlgButtonChecked(hWnd, IDC_MODEL))          newAnimType = MaxEggOptions::AT_model;
  else if (IsDlgButtonChecked(hWnd, IDC_ANIMATION)) newAnimType = MaxEggOptions::AT_chan;
  else if (IsDlgButtonChecked(hWnd, IDC_BOTH))      newAnimType = MaxEggOptions::AT_both;
  else                                              newAnimType = MaxEggOptions::AT_pose;

  if (newAnimType != MaxEggOptions::AT_model && IsDlgButtonChecked(hWnd, IDC_EXP_SEL_FRAMES)) {
      newSF = GetICustEditI(GetDlgItem(hWnd, IDC_SF), &valid);
      if (!valid) {
          MessageBox(hWnd, _T("Start Frame must be an integer"), _T("Invalid Value"), MB_OK | MB_ICONEXCLAMATION);
          return false;
      }
      if (newSF < _min_frame) {
          _stprintf(msg, _T("Start Frame must be at least %d"), _min_frame);
          MessageBox(hWnd, msg, _T("Invalid Value"), MB_OK | MB_ICONEXCLAMATION);
          return false;
      }
      if (newSF > _max_frame) {
          _stprintf(msg, _T("Start Frame must be at most %d"), _max_frame);
          MessageBox(hWnd, msg, _T("Invalid Value"), MB_OK | MB_ICONEXCLAMATION);
          return false;
      }
      if (newAnimType != MaxEggOptions::AT_pose) {
          newEF = GetICustEditI(GetDlgItem(hWnd, IDC_EF), &valid);
          if (!valid) {
              MessageBox(hWnd, _T("End Frame must be an integer"), _T("Invalid Value"), MB_OK | MB_ICONEXCLAMATION);
              return false;
          }
          if (newEF > _max_frame) {
              _stprintf(msg, _T("End Frame must be at most %d"), _max_frame);
              MessageBox(hWnd, msg, _T("Invalid Value"), MB_OK | MB_ICONEXCLAMATION);
              return false;
          }
          if (newEF < newSF) {
              MessageBox(hWnd, _T("End Frame must be greater than the start frame"), _T("Invalid Value"), MB_OK | MB_ICONEXCLAMATION);
              return false;
          }
      }
  }

  TCHAR *temp = GetICustEditT(GetDlgItem(hWnd, IDC_FILENAME));
  if (!_tcslen(temp)) {
    MessageBox(hWnd, _T("The filename cannot be empty"), _T("Invalid Value"), MB_OK | MB_ICONEXCLAMATION);
    return false;
  }

  if (_tcslen(temp) < 4 || _tcsncmp(_T(".egg"), temp+(_tcslen(temp) - 4), 4)) {
    _stprintf(_file_name, _T("%s.egg"), temp);
  } else {
    _tcscpy(_file_name, temp);
  }

  temp = _tcsrchr(_file_name, '\\');
  if (!temp) temp = _file_name;
  else temp++;

  if (_tcslen(temp) > sizeof(_short_name))
    _stprintf(_short_name, _T("%.*s..."), sizeof(_short_name)-4, temp);
  else {
    _tcscpy(_short_name, temp);
    _short_name[_tcslen(_short_name) - 4] = 0; //Cut off the .egg
  }

  _start_frame = newSF;
  _end_frame = newEF;
  _anim_type = newAnimType;
  _double_sided = IsDlgButtonChecked(hWnd, IDC_CHECK1);


  _export_whole_scene = IsDlgButtonChecked(hWnd, IDC_EXPORT_ALL);
  _export_all_frames = IsDlgButtonChecked(hWnd, IDC_EXP_ALL_FRAMES);

  return true;
}

bool MaxOptionsDialog::FindNode(ULONG INodeHandle) {
    for (int i = 0; i < _node_list.size(); i++)
        if (_node_list[i] == INodeHandle) return true;
    return false;
}

void MaxOptionsDialog::AddNode(ULONG INodeHandle) {
  if (FindNode(INodeHandle)) return;
  _node_list.push_back(INodeHandle);
}

void MaxOptionsDialog::CullBadNodes() {
  if (!_max_interface) return;
  std::vector<ULONG> good;
  for (int i=0; i<_node_list.size(); i++) {
      ULONG handle = _node_list[i];
      if (_max_interface->GetINodeByHandle(handle)) {
          good.push_back(handle);
      }
  }
  _node_list = good;
}

void MaxOptionsDialog::RemoveNode(int i) {
    if (i >= 0 && i < _node_list.size()) {
        for (int j = i+1; j < _node_list.size(); j++)
            _node_list[i++] = _node_list[j++];
        _node_list.pop_back();
    }
}

void MaxOptionsDialog::RemoveNodeByHandle(ULONG INodeHandle) {
    for (int i = 0; i < _node_list.size(); i++) {
        if (_node_list[i] == INodeHandle) {
            RemoveNode(i);
            return;
        }
    }
}

IOResult MaxOptionsDialog::Save(ISave *isave) {
    isave->BeginChunk(CHUNK_EGG_EXP_OPTIONS);
    ChunkSave(isave, CHUNK_ANIM_TYPE, _anim_type);
    ChunkSave(isave, CHUNK_FILENAME, _file_name);
    ChunkSave(isave, CHUNK_SHORTNAME, _short_name);
    ChunkSave(isave, CHUNK_SF, _start_frame);
    ChunkSave(isave, CHUNK_EF, _end_frame);
    ChunkSave(isave, CHUNK_DBL_SIDED, _double_sided);
    ChunkSave(isave, CHUNK_EGG_CHECKED, _checked);
    ChunkSave(isave, CHUNK_ALL_FRAMES, _export_all_frames);
    ChunkSave(isave, CHUNK_EXPORT_FULL, _export_whole_scene);

    isave->BeginChunk(CHUNK_NODE_LIST);
    for (int i = 0; i < _node_list.size(); i++)
        ChunkSave(isave, CHUNK_NODE_HANDLE, _node_list[i]);
    isave->EndChunk();
    isave->EndChunk();
    return IO_OK;
}

IOResult MaxOptionsDialog::Load(ILoad *iload) {
    IOResult res = iload->OpenChunk();

    while (res == IO_OK) {
        switch(iload->CurChunkID()) {
        case CHUNK_ANIM_TYPE: _anim_type = (Anim_Type)ChunkLoadInt(iload); break;
        case CHUNK_FILENAME: ChunkLoadString(iload, _file_name, sizeof(_file_name)); break;
        case CHUNK_SHORTNAME: ChunkLoadString(iload, _short_name, sizeof(_short_name)); break;
        case CHUNK_SF: _start_frame = ChunkLoadInt(iload); break;
        case CHUNK_EF: _end_frame = ChunkLoadInt(iload); break;
        case CHUNK_DBL_SIDED: _double_sided = ChunkLoadBool(iload); break;
        case CHUNK_EGG_CHECKED: _checked = ChunkLoadBool(iload); break;
        case CHUNK_ALL_FRAMES: _export_all_frames = ChunkLoadBool(iload); break;
        case CHUNK_EXPORT_FULL: _export_whole_scene = ChunkLoadBool(iload); break;

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

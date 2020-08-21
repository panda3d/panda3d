/**
 * @file maxEgg.cxx
 * @author Steven "Sauce" Osman
 * @date 2003-01
 * @author Ken Strickland
 * @date 2003-02-25
 *
 * This file implements the classes that are used in the Panda 3D file
 * exporter for 3D Studio Max.
 */

#include "maxEgg.h"


const double meshVerts[252][3] = {
    {0.729464, -0.919852, 0.714986},
    {0.466137, -0.594656, 0.160201},
    {-0.265897, -1.14704, 0.714986},
    {0.466137, -0.594656, 0.160201},
    {-0.177333, -0.741523, 0.160201},
    {-0.265897, -1.14704, 0.714986},
    {-0.265897, -1.14704, 0.714986},
    {-0.177333, -0.741523, 0.160201},
    {-1.06411, -0.510479, 0.714986},
    {-0.177333, -0.741523, 0.160201},
    {-0.693356, -0.330009, 0.160201},
    {-1.06411, -0.510479, 0.714986},
    {-1.06411, 0.510479, 0.714986},
    {-1.06411, -0.510479, 0.714986},
    {-0.693356, 0.330009, 0.160201},
    {-0.693356, -0.330009, 0.160201},
    {-0.693356, 0.330009, 0.160201},
    {-1.06411, -0.510479, 0.714986},
    {-0.265897, 1.14704, 0.714986},
    {-1.06411, 0.510479, 0.714986},
    {-0.177333, 0.741523, 0.160201},
    {-0.693356, 0.330009, 0.160201},
    {-0.177333, 0.741523, 0.160201},
    {-1.06411, 0.510479, 0.714986},
    {0.729464, 0.919852, 0.714986},
    {-0.265897, 1.14704, 0.714986},
    {0.466137, 0.594656, 0.160201},
    {-0.177333, 0.741523, 0.160201},
    {0.466137, 0.594656, 0.160201},
    {-0.265897, 1.14704, 0.714986},
    {1.17244, 0.0, 0.714986},
    {0.729464, 0.919852, 0.714986},
    {0.752508, 0.0, 0.160201},
    {0.466137, 0.594656, 0.160201},
    {0.752508, 0.0, 0.160201},
    {0.729464, 0.919852, 0.714986},
    {0.729464, -0.919852, 0.714986},
    {1.17244, 0.0, 0.714986},
    {0.466137, -0.594656, 0.160201},
    {0.752508, 0.0, 0.160201},
    {0.466137, -0.594656, 0.160201},
    {1.17244, 0.0, 0.714986},
    {-0.286334, -1.26822, 1.44995},
    {0.814187, -1.01703, 1.44995},
    {-0.265897, -1.14704, 0.714986},
    {0.729464, -0.919852, 0.714986},
    {-0.265897, -1.14704, 0.714986},
    {0.814187, -1.01703, 1.44995},
    {-1.16888, -0.564412, 1.44995},
    {-0.286334, -1.26822, 1.44995},
    {-1.06411, -0.510479, 0.714986},
    {-0.265897, -1.14704, 0.714986},
    {-1.06411, -0.510479, 0.714986},
    {-0.286334, -1.26822, 1.44995},
    {-1.16888, 0.564411, 1.44995},
    {-1.16888, -0.564412, 1.44995},
    {-1.06411, 0.510479, 0.714986},
    {-1.06411, -0.510479, 0.714986},
    {-1.06411, 0.510479, 0.714986},
    {-1.16888, -0.564412, 1.44995},
    {-1.16888, 0.564411, 1.44995},
    {-1.06411, 0.510479, 0.714986},
    {-0.286334, 1.26822, 1.44995},
    {-1.06411, 0.510479, 0.714986},
    {-0.265897, 1.14704, 0.714986},
    {-0.286334, 1.26822, 1.44995},
    {-0.286334, 1.26822, 1.44995},
    {-0.265897, 1.14704, 0.714986},
    {0.814187, 1.01703, 1.44995},
    {-0.265897, 1.14704, 0.714986},
    {0.729464, 0.919852, 0.714986},
    {0.814187, 1.01703, 1.44995},
    {1.30396, 0.0, 1.44995},
    {0.814187, 1.01703, 1.44995},
    {1.17244, 0.0, 0.714986},
    {0.729464, 0.919852, 0.714986},
    {1.17244, 0.0, 0.714986},
    {0.814187, 1.01703, 1.44995},
    {1.30396, 0.0, 1.44995},
    {1.17244, 0.0, 0.714986},
    {0.814187, -1.01703, 1.44995},
    {1.17244, 0.0, 0.714986},
    {0.729464, -0.919852, 0.714986},
    {0.814187, -1.01703, 1.44995},
    {-0.286334, -1.26822, 1.44995},
    {-0.227573, -1.05763, 2.16723},
    {0.814187, -1.01703, 1.44995},
    {0.814187, -1.01703, 1.44995},
    {-0.227573, -1.05763, 2.16723},
    {0.690208, -0.848157, 2.16723},
    {-1.16888, -0.564412, 1.44995},
    {-0.963577, -0.470692, 2.16723},
    {-0.286334, -1.26822, 1.44995},
    {-0.286334, -1.26822, 1.44995},
    {-0.963577, -0.470692, 2.16723},
    {-0.227573, -1.05763, 2.16723},
    {-1.16888, -0.564412, 1.44995},
    {-1.16888, 0.564411, 1.44995},
    {-0.963577, -0.470692, 2.16723},
    {-1.16888, 0.564411, 1.44995},
    {-0.963577, 0.470692, 2.16723},
    {-0.963577, -0.470692, 2.16723},
    {-1.16888, 0.564411, 1.44995},
    {-0.286334, 1.26822, 1.44995},
    {-0.963577, 0.470692, 2.16723},
    {-0.286334, 1.26822, 1.44995},
    {-0.227574, 1.05763, 2.16723},
    {-0.963577, 0.470692, 2.16723},
    {-0.286334, 1.26822, 1.44995},
    {0.814187, 1.01703, 1.44995},
    {-0.227574, 1.05763, 2.16723},
    {0.814187, 1.01703, 1.44995},
    {0.690208, 0.848157, 2.16723},
    {-0.227574, 1.05763, 2.16723},
    {0.814187, 1.01703, 1.44995},
    {1.30396, 0.0, 1.44995},
    {0.690208, 0.848157, 2.16723},
    {1.30396, 0.0, 1.44995},
    {1.09866, 0.0, 2.16723},
    {0.690208, 0.848157, 2.16723},
    {0.814187, -1.01703, 1.44995},
    {0.690208, -0.848157, 2.16723},
    {1.30396, 0.0, 1.44995},
    {1.30396, 0.0, 1.44995},
    {0.690208, -0.848157, 2.16723},
    {1.09866, 0.0, 2.16723},
    {-0.227573, -1.05763, 2.16723},
    {-0.154893, -0.759032, 2.72566},
    {0.690208, -0.848157, 2.16723},
    {0.690208, -0.848157, 2.16723},
    {-0.154893, -0.759032, 2.72566},
    {0.50377, -0.608696, 2.72566},
    {-0.963577, -0.470692, 2.16723},
    {-0.683099, -0.337801, 2.72566},
    {-0.227573, -1.05763, 2.16723},
    {-0.227573, -1.05763, 2.16723},
    {-0.683099, -0.337801, 2.72566},
    {-0.154893, -0.759032, 2.72566},
    {-0.963577, -0.470692, 2.16723},
    {-0.963577, 0.470692, 2.16723},
    {-0.683099, -0.337801, 2.72566},
    {-0.963577, 0.470692, 2.16723},
    {-0.683099, 0.337801, 2.72566},
    {-0.683099, -0.337801, 2.72566},
    {-0.963577, 0.470692, 2.16723},
    {-0.227574, 1.05763, 2.16723},
    {-0.683099, 0.337801, 2.72566},
    {-0.227574, 1.05763, 2.16723},
    {-0.154893, 0.759032, 2.72566},
    {-0.683099, 0.337801, 2.72566},
    {-0.227574, 1.05763, 2.16723},
    {0.690208, 0.848157, 2.16723},
    {-0.154893, 0.759032, 2.72566},
    {0.690208, 0.848157, 2.16723},
    {0.50377, 0.608696, 2.72566},
    {-0.154893, 0.759032, 2.72566},
    {0.690208, 0.848157, 2.16723},
    {1.09866, 0.0, 2.16723},
    {0.50377, 0.608696, 2.72566},
    {1.09866, 0.0, 2.16723},
    {0.796903, 0.0, 2.72566},
    {0.50377, 0.608696, 2.72566},
    {1.09866, 0.0, 2.16723},
    {0.690208, -0.848157, 2.16723},
    {0.796903, 0.0, 2.72566},
    {0.690208, -0.848157, 2.16723},
    {0.50377, -0.608696, 2.72566},
    {0.796903, 0.0, 2.72566},
    {0.50377, -0.608696, 2.72566},
    {-0.154893, -0.759032, 2.72566},
    {0.259722, -0.299638, 3.11175},
    {-0.154893, -0.759032, 2.72566},
    {-0.0645132, -0.373643, 3.11175},
    {0.259722, -0.299638, 3.11175},
    {-0.154893, -0.759032, 2.72566},
    {-0.683099, -0.337801, 2.72566},
    {-0.0645132, -0.373643, 3.11175},
    {-0.683099, -0.337801, 2.72566},
    {-0.324529, -0.166287, 3.11175},
    {-0.0645132, -0.373643, 3.11175},
    {-0.683099, -0.337801, 2.72566},
    {-0.683099, 0.337801, 2.72566},
    {-0.324529, -0.166287, 3.11175},
    {-0.683099, 0.337801, 2.72566},
    {-0.324529, 0.166287, 3.11175},
    {-0.324529, -0.166287, 3.11175},
    {-0.683099, 0.337801, 2.72566},
    {-0.154893, 0.759032, 2.72566},
    {-0.324529, 0.166287, 3.11175},
    {-0.154893, 0.759032, 2.72566},
    {-0.0645132, 0.373642, 3.11175},
    {-0.324529, 0.166287, 3.11175},
    {-0.154893, 0.759032, 2.72566},
    {0.50377, 0.608696, 2.72566},
    {-0.0645132, 0.373642, 3.11175},
    {0.50377, 0.608696, 2.72566},
    {0.259722, 0.299638, 3.11175},
    {-0.0645132, 0.373642, 3.11175},
    {0.50377, 0.608696, 2.72566},
    {0.796903, 0.0, 2.72566},
    {0.259722, 0.299638, 3.11175},
    {0.796903, 0.0, 2.72566},
    {0.40402, 0.0, 3.11175},
    {0.259722, 0.299638, 3.11175},
    {0.796903, 0.0, 2.72566},
    {0.50377, -0.608696, 2.72566},
    {0.40402, 0.0, 3.11175},
    {0.50377, -0.608696, 2.72566},
    {0.259722, -0.299638, 3.11175},
    {0.40402, 0.0, 3.11175},
    {-0.177333, -0.741523, 0.160201},
    {0.466137, -0.594656, 0.160201},
    {-0.00334214, 0.0, 0.00443203},
    {-0.693356, -0.330009, 0.160201},
    {-0.177333, -0.741523, 0.160201},
    {-0.00334214, 0.0, 0.00443203},
    {-0.693356, 0.330009, 0.160201},
    {-0.693356, -0.330009, 0.160201},
    {-0.00334214, 0.0, 0.00443203},
    {-0.177333, 0.741523, 0.160201},
    {-0.693356, 0.330009, 0.160201},
    {-0.00334214, 0.0, 0.00443203},
    {0.466137, 0.594656, 0.160201},
    {-0.177333, 0.741523, 0.160201},
    {-0.00334214, 0.0, 0.00443203},
    {0.752508, 0.0, 0.160201},
    {0.466137, 0.594656, 0.160201},
    {-0.00334214, 0.0, 0.00443203},
    {0.466137, -0.594656, 0.160201},
    {0.752508, 0.0, 0.160201},
    {-0.00334214, 0.0, 0.00443203},
    {0.259722, -0.299638, 3.11175},
    {-0.0645132, -0.373643, 3.11175},
    {0.0207683, 0.0, 3.20912},
    {-0.0645132, -0.373643, 3.11175},
    {-0.324529, -0.166287, 3.11175},
    {0.0207683, 0.0, 3.20912},
    {-0.324529, -0.166287, 3.11175},
    {-0.324529, 0.166287, 3.11175},
    {0.0207683, 0.0, 3.20912},
    {-0.324529, 0.166287, 3.11175},
    {-0.0645132, 0.373642, 3.11175},
    {0.0207683, 0.0, 3.20912},
    {-0.0645132, 0.373642, 3.11175},
    {0.259722, 0.299638, 3.11175},
    {0.0207683, 0.0, 3.20912},
    {0.259722, 0.299638, 3.11175},
    {0.40402, 0.0, 3.11175},
    {0.0207683, 0.0, 3.20912},
    {0.40402, 0.0, 3.11175},
    {0.259722, -0.299638, 3.11175},
    {0.0207683, 0.0, 3.20912}
};


// Disable the forcing int to true or false performance warning
#pragma warning(disable: 4800)

/* MaxEggPluginClassDesc - A class that describes 3DS Plugin support.
   This basically says "Yes, I am a helper object!"
*/

class MaxEggPluginClassDesc : public ClassDesc
{
public:
  int          IsPublic() { return TRUE; }
  void         *Create(BOOL loading = FALSE) { return new MaxEggPlugin(); }
  const TCHAR  *ClassName() { return GetString(IDS_CLASS_NAME); }
  SClass_ID    SuperClassID() { return HELPER_CLASS_ID; }
  Class_ID     ClassID() { return MaxEggPlugin_CLASS_ID; }
  const TCHAR  *Category() { return GetString(IDS_CATEGORY); }
  // returns fixed parsable name (scripter-visible name)
  const TCHAR *InternalName() { return _T("MaxEggPlugin"); }
};

// Our private global instance of the above class
static MaxEggPluginClassDesc MaxEggPluginDesc;

// Called by LibClassDesc to find out what the plugin is
ClassDesc* GetMaxEggPluginDesc() { return &MaxEggPluginDesc; }

// Initialize class-static variables
Mesh MaxEggPlugin::mesh;
short MaxEggPlugin::meshBuilt=0;
HWND MaxEggPlugin::hMaxEggParams = nullptr;
IObjParam *MaxEggPlugin::iObjParams;

/* MaxEggPluginOptionsDlgProc() - This is the callback function for the
   dialog box that appears at the beginning of the conversion process.
 */

INT_PTR CALLBACK MaxEggPluginOptionsDlgProc( HWND hWnd, UINT message,
                                          WPARAM wParam, LPARAM lParam )
{
  MaxOptionsDialog *tempEgg;
  int sel, res;

  // We pass in our plugin through the lParam variable.  Let's convert it
  // back.
  MaxEggPlugin *imp = (MaxEggPlugin*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
  if ( !imp && message != WM_INITDIALOG ) return FALSE;

  switch(message)
    {
    // When we start, center the window.
    case WM_INITDIALOG:
      // this line is very necessary to pass the plugin as the lParam
      SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
      SetDlgFont( hWnd, imp->iObjParams->GetAppHFont() );
      MaxEggPlugin::hMaxEggParams = hWnd;
      return TRUE; break;

    case WM_MOUSEACTIVATE:
      imp->iObjParams->RealizeParamPanel();
      return TRUE; break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
      imp->iObjParams->RollupMouseMessage(hWnd,message,wParam,lParam);
      return TRUE; break;

    // A control was modified
    case WM_COMMAND:
      // The modified control is found in the lower word of the wParam long.
      switch( LOWORD(wParam) ) {
        case IDC_OVERWRITE_CHECK:
          imp->autoOverwrite =
            (IsDlgButtonChecked(hWnd, IDC_OVERWRITE_CHECK) == BST_CHECKED);
          return TRUE; break;
        case IDC_PVIEW_CHECK:
          imp->pview =
            (IsDlgButtonChecked(hWnd, IDC_PVIEW_CHECK) == BST_CHECKED);
          return TRUE; break;
        case IDC_LOGGING:
          imp->logOutput =
            (IsDlgButtonChecked(hWnd, IDC_LOGGING) == BST_CHECKED);
          return TRUE; break;
        case IDC_ADD_EGG:
          tempEgg = new MaxOptionsDialog();
          tempEgg->SetMaxInterface(imp->iObjParams);
          tempEgg->SetAnimRange();
          res = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EGG_DETAILS),
                               hWnd, MaxOptionsDialogProc, (LPARAM)tempEgg);
          if (res == TRUE) {
            imp->SaveCheckState();
            imp->AddEgg(tempEgg);
            imp->UpdateUI();
          }
          else delete tempEgg;
          return TRUE; break;
        case IDC_EDIT_EGG:
          sel = ListView_GetSelectionMark(GetDlgItem(hWnd, IDC_LIST_EGGS));
          if (sel != -1) {
            MaxOptionsDialog *tempEgg = imp->GetEgg(sel);
            if (tempEgg) {
                tempEgg->SetAnimRange();
                tempEgg->CullBadNodes();
                DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EGG_DETAILS),
                               hWnd, MaxOptionsDialogProc, (LPARAM)tempEgg);
            }
            imp->SaveCheckState();
            imp->UpdateUI();
          }
          return TRUE; break;
        case IDC_REMOVE_EGG:
          sel = ListView_GetSelectionMark(GetDlgItem(hWnd, IDC_LIST_EGGS));
          if (sel != -1) {
            imp->SaveCheckState();
            imp->RemoveEgg(sel);
            imp->UpdateUI();
          }
          return TRUE; break;
        case IDC_EXPORT:
          imp->DoExport();
          return TRUE; break;
      }
    }
  return FALSE;
}

MaxEggPlugin::MaxEggPlugin() :
autoOverwrite(false), pview(true), logOutput(false), numEggs(0), maxEggs(5)
{
    eggList = new MaxOptionsDialog*[maxEggs];
    BuildMesh();
}

MaxEggPlugin::~MaxEggPlugin() {
    for (int i = 0; i < numEggs; i++) delete eggList[i];
    delete [] eggList;
}

void MaxEggPlugin::AddEgg(MaxOptionsDialog *newEgg) {
    if (numEggs >= maxEggs) {
        MaxOptionsDialog **newList;
        maxEggs *= 2;
        newList = new MaxOptionsDialog*[maxEggs];
        for (int i = 0; i < numEggs; i++) newList[i] = eggList[i];
        delete [] eggList;
        eggList = newList;
    }

    eggList[numEggs++] = newEgg;
}

void MaxEggPlugin::RemoveEgg(int i) {
    if (i >= 0 && i < numEggs) {
        delete eggList[i];
        for (int j = i+1; j < numEggs;) eggList[i++] = eggList[j++];
        --numEggs;
    }
}

void MaxEggPlugin::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
    iObjParams = ip;
    for (int i=0; i<numEggs; i++) {
        eggList[i]->SetMaxInterface(ip);
    }

    if ( !hMaxEggParams ) {
        hMaxEggParams = ip->AddRollupPage(hInstance,
                                          MAKEINTRESOURCE(IDD_PANEL),
                                          MaxEggPluginOptionsDlgProc,
                                          GetString(IDS_PARAMS),
                                          (LPARAM)this );
        ip->RegisterDlgWnd(hMaxEggParams);
    } else {
        SetWindowLongPtr( hMaxEggParams, GWLP_USERDATA, (LPARAM)this );
    }

    UpdateUI();
}

void MaxEggPlugin::EndEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
    SaveCheckState();
    if ( flags&END_EDIT_REMOVEUI ) {
        ip->UnRegisterDlgWnd(hMaxEggParams);
        ip->DeleteRollupPage(hMaxEggParams);
        hMaxEggParams = nullptr;
    } else {
        SetWindowLongPtr( hMaxEggParams, GWLP_USERDATA, 0L );
    }
}

void MaxEggPlugin::SaveCheckState() {
    if (!hMaxEggParams) return;
    HWND lv = GetDlgItem(hMaxEggParams, IDC_LIST_EGGS);
    for (int i = 0; i < numEggs; i++)
        eggList[i]->_checked = ListView_GetCheckState(lv, i);
}

void MaxEggPlugin::UpdateUI() {
    HWND lv = GetDlgItem(hMaxEggParams, IDC_LIST_EGGS);
    LV_COLUMN pCol;

    if (ListView_GetColumnWidth(lv, 1) <= 0 || ListView_GetColumnWidth(lv, 1) > 10000) {
        // Columns have not been setup, so initialize the control
        ListView_SetExtendedListViewStyleEx(lv, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT,
                                            LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

        pCol.fmt = LVCFMT_LEFT;
        pCol.cx = 96;
        pCol.pszText = _T("Filename");
        pCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        pCol.iSubItem = 0;
        ListView_InsertColumn(lv, 0, &pCol);

        pCol.cx = 44;
        pCol.pszText = _T("Type");
        ListView_InsertColumn(lv, 1, &pCol);
    }

    // Add the eggs to the list
    ListView_DeleteAllItems(lv);
    LV_ITEM Item;
    Item.mask = LVIF_TEXT;

    for (int i = 0; i < numEggs; i++) {
        Item.iItem = i;
        Item.iSubItem = 0;
        Item.pszText = eggList[i]->_short_name;
        ListView_InsertItem(lv, &Item);
        Item.iSubItem = 1;
        switch(eggList[i]->_anim_type) {
        case MaxEggOptions::AT_chan:  Item.pszText = _T("Animation"); break;
        case MaxEggOptions::AT_both:  Item.pszText = _T("Both"); break;
        case MaxEggOptions::AT_pose:  Item.pszText = _T("Static"); break;
        case MaxEggOptions::AT_model: Item.pszText = _T("Model"); break;
        default:                      Item.pszText = _T("Model"); break;
        }
        ListView_SetItem(lv, &Item);
        ListView_SetCheckState(lv, i, eggList[i]->_checked);
    }

    // Set the "Overwrite Existing Files" and "Pview" checkboxes
    CheckDlgButton(hMaxEggParams, IDC_OVERWRITE_CHECK,
                   autoOverwrite ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hMaxEggParams, IDC_PVIEW_CHECK,
                   pview ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hMaxEggParams, IDC_LOGGING,
                   logOutput ? BST_CHECKED : BST_UNCHECKED);
}

void MaxEggPlugin::DoExport() {
    int good = 0, bad = 0;

    std::basic_stringstream<TCHAR> status;

    SaveCheckState();

    for (int i = 0; i < numEggs; i++) {
        if (eggList[i]->_checked) {
            // If "auto overwrite" was not checked and the file exists, ask if
            // the user wishes to overwrite the file
            bool do_write = true;

            if (!autoOverwrite && GetFileAttributes(eggList[i]->_file_name) != INVALID_FILE_ATTRIBUTES) {
                TCHAR msg[1024];
                _stprintf(msg, _T("Overwrite file \"%s.egg\"?"), eggList[i]->_short_name);
                do_write = (MessageBox(hMaxEggParams, msg, _T("Panda3D Exporter"), MB_YESNO | MB_ICONQUESTION) == IDYES);
            }
            if (do_write) {
                MaxToEggConverter converter;
                if (converter.convert((MaxEggOptions*)eggList[i])) {
                    good += 1;
                    status << _T("Successfully created ") << eggList[i]->_short_name << _T(".egg\n");
                } else {
                    bad += 1;
                    status << _T("Could not export ") << eggList[i]->_short_name << _T(".egg\n");
                }
            } else {
                bad += 1;
                status << _T("Skipped file ") << eggList[i]->_short_name << _T(".egg\n");
            }
        }
    }

    UINT mask = MB_OK;

    if (good == 0 && bad == 0) {
        mask |= MB_ICONEXCLAMATION;
        MessageBox(hMaxEggParams, _T("Nothing to export!"), _T("Panda3D Export results"), mask);
    } else {
        if (bad > 0) mask |= MB_ICONEXCLAMATION;
        else mask |= MB_ICONINFORMATION;
        MessageBox(hMaxEggParams, status.str().c_str(), _T("Panda3D Export results"), mask);
    }

    int pviewed = 0;
    if (pview && good > 0) {
        for (i = 0; i < numEggs; i++) {
            if (eggList[i]->_checked && eggList[i]->_successful) {
                if (eggList[i]->_anim_type != MaxEggOptions::AT_chan) {
                    PROCESS_INFORMATION pi;
                    STARTUPINFO si;

                    memset(&si, 0, sizeof(si));
                    si.cb = sizeof(si);

                    TCHAR cmdLine[2048];
                    // If we have just one model and animation file, pview
                    // them both
                    if (numEggs == 2 && eggList[i]->_anim_type == MaxEggOptions::AT_model &&
                        eggList[1-i]->_checked && eggList[1-i]->_successful &&
                        eggList[1-i]->_anim_type == MaxEggOptions::AT_chan) {
                        _stprintf(cmdLine, _T("pview \"%s\" \"%s\""), eggList[i]->_file_name, eggList[1-i]->_file_name);
                    } else {
                        _stprintf(cmdLine, _T("pview \"%s\""), eggList[i]->_file_name);
                    }
                    CreateProcess(nullptr, cmdLine, nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE,
                                  nullptr, nullptr, &si, &pi);
                    pviewed += 1;
                }
            }
        }
    }
}


void MaxEggPlugin::BuildMesh()
{
    int i;
    if(meshBuilt) return;

    mesh.setNumVerts(252);
    mesh.setNumFaces(84);
    mesh.setSmoothFlags(0);
    mesh.setNumTVerts (0);
    mesh.setNumTVFaces (0);

    for (i=0; i<252; i++)
        mesh.setVert(i, meshVerts[i][0]*10, meshVerts[i][1]*10, meshVerts[i][2]*10);
    for (i=0; i<84; i++) {
        mesh.faces[i].setEdgeVisFlags(1, 1, 0);
        mesh.faces[i].setSmGroup(0);
        mesh.faces[i].setVerts(i*3, i*3+1, i*3+2);
    }

    mesh.InvalidateGeomCache();
    mesh.BuildStripsAndEdges();

    meshBuilt = TRUE;
}

// The creation callback - sets the initial position of the helper in the
// scene.

class MaxEggPluginCreateMouseCallBack: public CreateMouseCallBack
{
public:
    int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
};

int MaxEggPluginCreateMouseCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat )
{
    if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
        switch(point) {
        case 0:
            mat.SetTrans(vpt->SnapPoint(m,m,nullptr,SNAP_IN_PLANE));
            break;
        case 1:
            mat.SetTrans(vpt->SnapPoint(m,m,nullptr,SNAP_IN_PLANE));
            if (msg==MOUSE_POINT) return CREATE_STOP;
            break;
        }
    } else if (msg == MOUSE_ABORT) {
        return CREATE_ABORT;
    }
    return CREATE_CONTINUE;
}

static MaxEggPluginCreateMouseCallBack MaxEggCreateMouseCB;

CreateMouseCallBack* MaxEggPlugin::GetCreateMouseCallBack()
{ return &MaxEggCreateMouseCB; }

// Boilerplate functions for dealing with the display of the plugin

void MaxEggPlugin::GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm)
{
    tm = inode->GetObjectTM(t);
    tm.NoScale();
    PN_stdfloat scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(PN_stdfloat)360.0;
    tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}

void MaxEggPlugin::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
    box = mesh.getBoundingBox(tm);
}

void MaxEggPlugin::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
    Matrix3 m = inode->GetObjectTM(t);
    Point3 pt;
    Point3 q[4];
    PN_stdfloat scaleFactor = vpt->GetVPWorldWidth(m.GetTrans())/360.0f;
    box = mesh.getBoundingBox();
    box.Scale(scaleFactor);
}

void MaxEggPlugin::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
    int i, nv; Matrix3 tm; Point3 pt;
    GetMat(t,inode,vpt,tm);
    nv = mesh.getNumVerts();
    box.Init();
    for (i=0; i<nv; i++)
        box += tm*mesh.getVert(i);
}

int MaxEggPlugin::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
    HitRegion hitRegion;
    DWORD savedLimits;
    Matrix3 m;
    GraphicsWindow *gw = vpt->getGW();
    Material *mtl = gw->getMaterial();
    MakeHitRegion(hitRegion,type,crossing,4,p);
    gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
    GetMat(t,inode,vpt,m);
    gw->setTransform(m);
    gw->clearHitCode();
    if (mesh.select( gw, mtl, &hitRegion, flags & HIT_ABORTONHIT ))
        return TRUE;
    return FALSE;
}

int MaxEggPlugin::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
    Matrix3 m;
    GraphicsWindow *gw = vpt->getGW();
    Material *mtl = gw->getMaterial();

    GetMat(t,inode,vpt,m);
    gw->setTransform(m);
    DWORD rlim = gw->getRndLimits();
    gw->setRndLimits(GW_WIREFRAME|GW_BACKCULL);
    if (inode->Selected())
        gw->setColor( LINE_COLOR, GetSelColor());
    else if(!inode->IsFrozen())
        gw->setColor( LINE_COLOR, GetUIColor(COLOR_TAPE_OBJ));
    mesh.render( gw, mtl, nullptr, COMP_ALL);
    return 0;
}

RefResult MaxEggPlugin::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message )
{
    UpdateUI();
    return REF_SUCCEED;
}

ObjectState MaxEggPlugin::Eval(TimeValue time)
{
    return ObjectState(this);
}

Interval MaxEggPlugin::ObjectValidity(TimeValue t)
{
    Interval ivalid;
    ivalid.SetInfinite();
    return ivalid;
}

RefTargetHandle MaxEggPlugin::Clone(RemapDir& remap)
{
    MaxEggPlugin* newob = new MaxEggPlugin();
    return(newob);
}

// Loading and saving the plugin

IOResult MaxEggPlugin::Save(ISave *isave) {
    SaveCheckState();
    for (int i = 0; i < numEggs; i++)
        eggList[i]->Save(isave);
    ChunkSave(isave,   CHUNK_OVERWRITE_FLAG,  autoOverwrite);
    ChunkSave(isave,   CHUNK_PVIEW_FLAG,      pview);
    ChunkSave(isave,   CHUNK_LOG_OUTPUT,      logOutput);

    return IO_OK;
}

IOResult MaxEggPlugin::Load(ILoad *iload) {
    IOResult res = iload->OpenChunk();
    MaxOptionsDialog *temp;

    while (res == IO_OK) {
        switch(iload->CurChunkID()) {
        case CHUNK_OVERWRITE_FLAG: autoOverwrite = ChunkLoadBool(iload); break;
        case CHUNK_PVIEW_FLAG:     pview = ChunkLoadBool(iload); break;
        case CHUNK_LOG_OUTPUT:     logOutput = ChunkLoadBool(iload); break;
        case CHUNK_EGG_EXP_OPTIONS:
            temp = new MaxOptionsDialog();
            temp->SetMaxInterface(iObjParams);
            temp->Load(iload);
            AddEgg(temp);
            break;
        }
        iload->CloseChunk();
        res = iload->OpenChunk();
    }

    return IO_OK;
}

/**********************************************************************
 *
 * DLL Initialization
 *
 **********************************************************************/

extern ClassDesc* GetMaxEggPluginDesc();

HINSTANCE hInstance;
int controlsInit = FALSE;

// This function is called by Windows when the DLL is loaded.  This function
// may also be called many times during time critical operations like
// rendering.  Therefore developers need to be careful what they do inside
// this function.  In the code below, note how after the DLL is loaded the
// first time only a few statements are executed.

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
        hInstance = hinstDLL;                           // Hang on to this DLL's instance handle.

        if (!controlsInit) {
                controlsInit = TRUE;

#if MAX_VERSION_MAJOR < 14
                // It appears that InitCustomControls is deprecated in 2012.
                // I'm not sure if we can just remove it like this, but I've
                // heard that it seems to work, so let's do it like this.
                InitCustomControls(hInstance);  // Initialize MAX's custom controls
#endif
                InitCommonControls();                   // Initialize Win95 controls
        }

        return (TRUE);
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR* LibDescription()
{
        return GetString(IDS_LIBDESCRIPTION);
}

// This function returns the number of plug-in classes this DLL operates on.
// TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
        return 1;
}

// This function returns the descriptions of the plug-in classes this DLL
// operates on.
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
        switch(i) {
                case 0: return GetMaxEggPluginDesc();
                default: return nullptr;
        }
}

// This function returns a pre-defined constant indicating the version of the
// system under which it was compiled.  It is used to allow the system to
// catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion()
{
        return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
        static TCHAR buf[256];

        if (hInstance)
                return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : nullptr;
        return nullptr;
}

//
// P3DActiveX.rc.pp
//
// This file defines the script to auto-generate P3DActiveX.rc at
// ppremake time.  We use this to fill in the DLL version correctly.
//

#output P3DActiveX.rc notouch
/$[]/#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
/$[]/################################# DO NOT EDIT ###########################

// Microsoft Visual C++ generated resource script.
//
#$[]include "resource.h"

#$[]define APSTUDIO_READONLY_SYMBOLS
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 2 resource.
//
#$[]include "afxres.h"

/////////////////////////////////////////////////////////////////////////////
#$[]undef APSTUDIO_READONLY_SYMBOLS

/////////////////////////////////////////////////////////////////////////////
// English (U.S.) resources

#$[]if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_ENU)
#$[]ifdef _WIN32
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
#$[]pragma code_page(1252)
#$[]endif //_WIN32

#$[]ifdef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// TEXTINCLUDE
//

1 TEXTINCLUDE 
BEGIN
    "resource.h\0"
END

2 TEXTINCLUDE 
BEGIN
    "#$[]include ""afxres.h""\r\n"
    "\0"
END

3 TEXTINCLUDE 
BEGIN
    "1 TYPELIB ""P3DActiveX.tlb""\r\n"
    "\0"
END

#$[]endif    // APSTUDIO_INVOKED


/////////////////////////////////////////////////////////////////////////////
//
// Version
//

VS_VERSION_INFO VERSIONINFO
 FILEVERSION $[P3D_PLUGIN_DLL_COMMA_VERSION]
 PRODUCTVERSION $[P3D_PLUGIN_DLL_COMMA_VERSION]
 FILEFLAGSMASK 0x3fL
#$[]ifdef _DEBUG
 FILEFLAGS 0x1L
#$[]else
 FILEFLAGS 0x0L
#$[]endif
 FILEOS 0x4L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904e4"
        BEGIN
            VALUE "FileDescription", "Panda3D Game Engine Plug-in $[P3D_PLUGIN_VERSION_STR]\0"
            Value "CompanyName", "$[PANDA_DISTRIBUTOR]"
            VALUE "FileVersion", "$[P3D_PLUGIN_DLL_DOT_VERSION]"
            VALUE "InternalName", "P3DActiveX.ocx"
            VALUE "LegalTrademarks", "\0"
            VALUE "FileOpenName", "Panda3D applet\0"
            VALUE "OLESelfRegister", "\0"
            VALUE "OriginalFilename", "P3DActiveX.ocx"
            VALUE "ProductName", "Panda3D Game Engine Plug-in $[P3D_PLUGIN_VERSION_STR]\0"
            VALUE "ProductVersion", "$[P3D_PLUGIN_DLL_DOT_VERSION]"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END


/////////////////////////////////////////////////////////////////////////////
//
// Bitmap
//

IDB_P3DACTIVEX          BITMAP                  "P3DActiveXCtrl.bmp"

/////////////////////////////////////////////////////////////////////////////
//
// Dialog
//

IDD_PROPPAGE_P3DACTIVEX DIALOG  0, 0, 250, 62
STYLE DS_SETFONT | WS_CHILD
FONT 8, "MS Sans Serif"
BEGIN
    LTEXT           "TODO: Place controls to manipulate properties of P3DActiveX Control on this dialog.",
                    IDC_STATIC,7,25,229,16
END


/////////////////////////////////////////////////////////////////////////////
//
// DESIGNINFO
//

#$[]ifdef APSTUDIO_INVOKED
GUIDELINES DESIGNINFO 
BEGIN
    IDD_PROPPAGE_P3DACTIVEX, DIALOG
    BEGIN
        LEFTMARGIN, 7
        RIGHTMARGIN, 243
        TOPMARGIN, 7
        BOTTOMMARGIN, 55
    END
END
#$[]endif    // APSTUDIO_INVOKED


/////////////////////////////////////////////////////////////////////////////
//
// String Table
//

STRINGTABLE 
BEGIN
    IDS_P3DACTIVEX          "P3DActiveX Control"
    IDS_P3DACTIVEX_PPG      "P3DActiveX Property Page"
END

STRINGTABLE 
BEGIN
    IDS_P3DACTIVEX_PPG_CAPTION "General"
END

#$[]endif    // English (U.S.) resources
/////////////////////////////////////////////////////////////////////////////



#$[]ifndef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 3 resource.
//
1 TYPELIB "P3DActiveX.tlb"

/////////////////////////////////////////////////////////////////////////////
#$[]endif    // not APSTUDIO_INVOKED


#end P3DActiveX.rc


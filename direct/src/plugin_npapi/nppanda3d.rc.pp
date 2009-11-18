//
// nppanda3d.rc.pp
//
// This file defines the script to auto-generate nppanda3d.rc at
// ppremake time.  We use this to fill in the DLL version correctly.
//
// This resource file is required on Windows to load the appropriate
// text into the DLL, so Mozilla will recognize the DLL as a plugin.
// It also defines the MIME type supported by the plugin.

#output nppanda3d.rc notouch
/$[]/#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
/$[]/################################# DO NOT EDIT ###########################

#$[]define APSTUDIO_READONLY_SYMBOLS
#$[]include "winresrc.h"
#$[]undef APSTUDIO_READONLY_SYMBOLS


VS_VERSION_INFO VERSIONINFO
 FILEVERSION $[P3D_PLUGIN_DLL_COMMA_VERSION]
 PRODUCTVERSION $[P3D_PLUGIN_DLL_COMMA_VERSION]
 FILEFLAGSMASK 0x3fL
#$[]ifdef _DEBUG
 FILEFLAGS 0x1L
#$[]else
 FILEFLAGS 0x0L
#$[]endif
 FILEOS 0x40004L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904e4"
        BEGIN
            VALUE "FileDescription", "Runs 3-D games and interactive applets\0"
            VALUE "FileVersion", "$[P3D_PLUGIN_DLL_DOT_VERSION]"
            VALUE "LegalTrademarks", "\0"
            VALUE "MIMEType", "application/x-panda3d\0"
            VALUE "FileExtents", "p3d\0"
            VALUE "FileOpenName", "Panda3D applet\0"
            VALUE "OriginalFilename", "nppanda3d.dll\0"
            VALUE "ProductName", "Panda3D Game Engine Plug-In $[P3D_PLUGIN_VERSION_STR]\0"
            VALUE "ProductVersion", "$[P3D_PLUGIN_DLL_DOT_VERSION]"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END

#end nppanda3d.rc


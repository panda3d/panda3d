// Filename: dxgsg7base.cxx
// Created by:  drose (02Jan03)
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

#include "dxgsg7base.h"

#include <dxfile.h>

const char *
ConvD3DErrorToString(HRESULT error) {
  switch (error) {
  case E_FAIL:
    return "Unspecified error E_FAIL";
    
  case DD_OK:
    return "No error.";
  case D3DERR_BADMAJORVERSION      : // (700)
    return "D3DERR_BADMAJORVERSION";
  case D3DERR_BADMINORVERSION      : // (701)
    return "D3DERR_BADMINORVERSION";
  case D3DERR_INVALID_DEVICE   : // (705)
    return "D3DERR_INVALID_DEVICE";
  case D3DERR_INITFAILED       : // (706)
    return "D3DERR_INITFAILED";
  case D3DERR_DEVICEAGGREGATED : // (707)
    return "D3DERR_DEVICEAGGREGATED";
  case D3DERR_EXECUTE_CREATE_FAILED    : // (710)
    return "D3DERR_EXECUTE_CREATE_FAILED";
  case D3DERR_EXECUTE_DESTROY_FAILED   : // (711)
    return "D3DERR_EXECUTE_DESTROY_FAILED";
  case D3DERR_EXECUTE_LOCK_FAILED  : // (712)
    return "D3DERR_EXECUTE_LOCK_FAILED";
  case D3DERR_EXECUTE_UNLOCK_FAILED    : // (713)
    return "D3DERR_EXECUTE_UNLOCK_FAILED";
  case D3DERR_EXECUTE_LOCKED       : // (714)
    return "D3DERR_EXECUTE_LOCKED";
  case D3DERR_EXECUTE_NOT_LOCKED   : // (715)
    return "D3DERR_EXECUTE_NOT_LOCKED";
  case D3DERR_EXECUTE_FAILED       : // (716)
    return "D3DERR_EXECUTE_FAILED";
  case D3DERR_EXECUTE_CLIPPED_FAILED   : // (717)
    return "D3DERR_EXECUTE_CLIPPED_FAILED";
  case D3DERR_TEXTURE_NO_SUPPORT   : // (720)
    return "D3DERR_TEXTURE_NO_SUPPORT";
  case D3DERR_TEXTURE_CREATE_FAILED    : // (721)
    return "D3DERR_TEXTURE_CREATE_FAILED";
  case D3DERR_TEXTURE_DESTROY_FAILED   : // (722)
    return "D3DERR_TEXTURE_DESTROY_FAILED";
  case D3DERR_TEXTURE_LOCK_FAILED  : // (723)
    return "D3DERR_TEXTURE_LOCK_FAILED";
  case D3DERR_TEXTURE_UNLOCK_FAILED    : // (724)
    return "D3DERR_TEXTURE_UNLOCK_FAILED";
  case D3DERR_TEXTURE_LOAD_FAILED  : // (725)
    return "D3DERR_TEXTURE_LOAD_FAILED";
  case D3DERR_TEXTURE_SWAP_FAILED  : // (726)
    return "D3DERR_TEXTURE_SWAP_FAILED";
  case D3DERR_TEXTURE_LOCKED       : // (727)
    return "D3DERR_TEXTURE_LOCKED";
  case D3DERR_TEXTURE_NOT_LOCKED   : // (728)
    return "D3DERR_TEXTURE_NOT_LOCKED";
  case D3DERR_TEXTURE_GETSURF_FAILED   : // (729)
    return "D3DERR_TEXTURE_GETSURF_FAILED";
  case D3DERR_MATRIX_CREATE_FAILED : // (730)
    return "D3DERR_MATRIX_CREATE_FAILED";
  case D3DERR_MATRIX_DESTROY_FAILED    : // (731)
    return "D3DERR_MATRIX_DESTROY_FAILED";
  case D3DERR_MATRIX_SETDATA_FAILED    : // (732)
    return "D3DERR_MATRIX_SETDATA_FAILED";
  case D3DERR_MATRIX_GETDATA_FAILED    : // (733)
    return "D3DERR_MATRIX_GETDATA_FAILED";
  case D3DERR_SETVIEWPORTDATA_FAILED   : // (734)
    return "D3DERR_SETVIEWPORTDATA_FAILED";
  case D3DERR_INVALIDCURRENTVIEWPORT   : // (735)
    return "D3DERR_INVALIDCURRENTVIEWPORT";
  case D3DERR_INVALIDPRIMITIVETYPE     : // (736)
    return "D3DERR_INVALIDPRIMITIVETYPE";
  case D3DERR_INVALIDVERTEXTYPE        : // (737)
    return "D3DERR_INVALIDVERTEXTYPE";
  case D3DERR_TEXTURE_BADSIZE          : // (738)
    return "D3DERR_TEXTURE_BADSIZE";
  case D3DERR_INVALIDRAMPTEXTURE       : // (739)
    return "D3DERR_INVALIDRAMPTEXTURE";
  case D3DERR_MATERIAL_CREATE_FAILED   : // (740)
    return "D3DERR_MATERIAL_CREATE_FAILED";
  case D3DERR_MATERIAL_DESTROY_FAILED  : // (741)
    return "D3DERR_MATERIAL_DESTROY_FAILED";
  case D3DERR_MATERIAL_SETDATA_FAILED  : // (742)
    return "D3DERR_MATERIAL_SETDATA_FAILED";
  case D3DERR_MATERIAL_GETDATA_FAILED  : // (743)
    return "D3DERR_MATERIAL_GETDATA_FAILED";
  case D3DERR_INVALIDPALETTE           : // (744)
    return "D3DERR_INVALIDPALETTE";
  case D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY : // (745)
    return "D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY";
  case D3DERR_ZBUFF_NEEDS_VIDEOMEMORY  : // (746)
    return "D3DERR_ZBUFF_NEEDS_VIDEOMEMORY";
  case D3DERR_SURFACENOTINVIDMEM       : // (747)
    return "D3DERR_SURFACENOTINVIDMEM";
  case D3DERR_LIGHT_SET_FAILED     : // (750)
    return "D3DERR_LIGHT_SET_FAILED";
  case D3DERR_LIGHTHASVIEWPORT     : // (751)
    return "D3DERR_LIGHTHASVIEWPORT";
  case D3DERR_LIGHTNOTINTHISVIEWPORT           : // (752)
    return "D3DERR_LIGHTNOTINTHISVIEWPORT";
  case D3DERR_SCENE_IN_SCENE       : // (760)
    return "D3DERR_SCENE_IN_SCENE";
  case D3DERR_SCENE_NOT_IN_SCENE   : // (761)
    return "D3DERR_SCENE_NOT_IN_SCENE";
  case D3DERR_SCENE_BEGIN_FAILED   : // (762)
    return "D3DERR_SCENE_BEGIN_FAILED";
  case D3DERR_SCENE_END_FAILED     : // (763)
    return "D3DERR_SCENE_END_FAILED";
  case D3DERR_INBEGIN                  : // (770)
    return "D3DERR_INBEGIN";
  case D3DERR_NOTINBEGIN               : // (771)
    return "D3DERR_NOTINBEGIN";
  case D3DERR_NOVIEWPORTS              : // (772)
    return "D3DERR_NOVIEWPORTS";
  case D3DERR_VIEWPORTDATANOTSET       : // (773)
    return "D3DERR_VIEWPORTDATANOTSET";
  case D3DERR_VIEWPORTHASNODEVICE      : // (774)
    return "D3DERR_VIEWPORTHASNODEVICE";
  case D3DERR_NOCURRENTVIEWPORT        : // (775)
    return "D3DERR_NOCURRENTVIEWPORT";
  case D3DERR_INVALIDVERTEXFORMAT              : // (2048)
    return "D3DERR_INVALIDVERTEXFORMAT";
  case D3DERR_COLORKEYATTACHED                 : // (2050)
    return "D3DERR_COLORKEYATTACHED";
  case D3DERR_VERTEXBUFFEROPTIMIZED            : // (2060)
    return "D3DERR_VERTEXBUFFEROPTIMIZED";
  case D3DERR_VBUF_CREATE_FAILED               : // (2061)
    return "D3DERR_VBUF_CREATE_FAILED";
  case D3DERR_VERTEXBUFFERLOCKED               : // (2062)
    return "D3DERR_VERTEXBUFFERLOCKED";
  case D3DERR_ZBUFFER_NOTPRESENT               : // (2070)
    return "D3DERR_ZBUFFER_NOTPRESENT";
  case D3DERR_STENCILBUFFER_NOTPRESENT         : // (2071)
    return "D3DERR_STENCILBUFFER_NOTPRESENT";
  case D3DERR_WRONGTEXTUREFORMAT               : // (2072)
    return "D3DERR_WRONGTEXTUREFORMAT";
  case D3DERR_UNSUPPORTEDCOLOROPERATION        : // (2073)
    return "D3DERR_UNSUPPORTEDCOLOROPERATION";
  case D3DERR_UNSUPPORTEDCOLORARG              : // (2074)
    return "D3DERR_UNSUPPORTEDCOLORARG";
  case D3DERR_UNSUPPORTEDALPHAOPERATION        : // (2075)
    return "D3DERR_UNSUPPORTEDALPHAOPERATION";
  case D3DERR_UNSUPPORTEDALPHAARG              : // (2076)
    return "D3DERR_UNSUPPORTEDALPHAARG";
  case D3DERR_TOOMANYOPERATIONS                : // (2077)
    return "D3DERR_TOOMANYOPERATIONS";
  case D3DERR_CONFLICTINGTEXTUREFILTER         : // (2078)
    return "D3DERR_CONFLICTINGTEXTUREFILTER";
  case D3DERR_UNSUPPORTEDFACTORVALUE           : // (2079)
    return "D3DERR_UNSUPPORTEDFACTORVALUE";
  case D3DERR_CONFLICTINGRENDERSTATE           : // (2081)
    return "D3DERR_CONFLICTINGRENDERSTATE";
  case D3DERR_UNSUPPORTEDTEXTUREFILTER         : // (2082)
    return "D3DERR_UNSUPPORTEDTEXTUREFILTER";
  case D3DERR_TOOMANYPRIMITIVES                : // (2083)
    return "D3DERR_TOOMANYPRIMITIVES";
  case D3DERR_INVALIDMATRIX                    : // (2084)
    return "D3DERR_INVALIDMATRIX";
  case D3DERR_TOOMANYVERTICES                  : // (2085)
    return "D3DERR_TOOMANYVERTICES";
  case D3DERR_CONFLICTINGTEXTUREPALETTE        : // (2086)
    return "D3DERR_CONFLICTINGTEXTUREPALETTE";
    //#if DX7
  case D3DERR_VERTEXBUFFERUNLOCKFAILED         : // (2063)
    return "D3DERR_VERTEXBUFFERUNLOCKFAILED";
  case D3DERR_INVALIDSTATEBLOCK        : // (2100)
    return "D3DERR_INVALIDSTATEBLOCK";
  case D3DERR_INBEGINSTATEBLOCK        : // (2101)
    return "D3DERR_INBEGINSTATEBLOCK";
  case D3DERR_NOTINBEGINSTATEBLOCK     : // (2102)
    return "D3DERR_NOTINBEGINSTATEBLOCK";
    //case D3DERR_INOVERLAYSTATEBLOCK      : // (2103)
    //  return "D3DERR_INOVERLAYSTATEBLOCK";
  case DDERR_NOSTEREOHARDWARE       : // ( 181 )
    return "DDERR_NOSTEREOHARDWARE      ";
  case DDERR_NOSURFACELEFT              : // ( 182 )
    return "DDERR_NOSURFACELEFT             ";
  case DDERR_DDSCAPSCOMPLEXREQUIRED            : // ( 542 )
    return "DDERR_DDSCAPSCOMPLEXREQUIRED";
  case DDERR_NOTONMIPMAPSUBLEVEL               : // ( 603 )
    return "DDERR_NOTONMIPMAPSUBLEVEL";
  case DDERR_TESTFINISHED                      : // ( 692 )
    return "DDERR_TESTFINISHED";
  case DDERR_NEWMODE                           : // ( 693 )
    return "DDERR_NEWMODE";
    //#endif
    //case D3DERR_COMMAND_UNPARSED              : // (3000)
    /// return "case";//D3DERR_COMMAND_UNPARSED              : // (3000)

  case DDERR_ALREADYINITIALIZED     : // ( 5 )
    return "DDERR_ALREADYINITIALIZED        ";
  case DDERR_CANNOTATTACHSURFACE        : // ( 10 )
    return "DDERR_CANNOTATTACHSURFACE       ";
  case DDERR_CANNOTDETACHSURFACE        : // ( 20 )
    return "DDERR_CANNOTDETACHSURFACE       ";
  case DDERR_CURRENTLYNOTAVAIL          : // ( 40 )
    return "DDERR_CURRENTLYNOTAVAIL         ";
  case DDERR_EXCEPTION              : // ( 55 )
    return "DDERR_EXCEPTION             ";
  case DDERR_HEIGHTALIGN            : // ( 90 )
    return "DDERR_HEIGHTALIGN           ";
  case DDERR_INCOMPATIBLEPRIMARY        : // ( 95 )
    return "DDERR_INCOMPATIBLEPRIMARY       ";
  case DDERR_INVALIDCAPS            : // ( 100 )
    return "DDERR_INVALIDCAPS           ";
  case DDERR_INVALIDCLIPLIST            : // ( 110 )
    return "DDERR_INVALIDCLIPLIST           ";
  case DDERR_INVALIDMODE            : // ( 120 )
    return "DDERR_INVALIDMODE           ";
  case DDERR_INVALIDOBJECT          : // ( 130 )
    return "DDERR_INVALIDOBJECT         ";
  case DDERR_INVALIDPIXELFORMAT     : // ( 145 )
    return "DDERR_INVALIDPIXELFORMAT        ";
  case DDERR_INVALIDRECT            : // ( 150 )
    return "DDERR_INVALIDRECT           ";
  case DDERR_LOCKEDSURFACES         : // ( 160 )
    return "DDERR_LOCKEDSURFACES            ";
  case DDERR_NO3D               : // ( 170 )
    return "DDERR_NO3D              ";
  case DDERR_NOALPHAHW              : // ( 180 )
    return "DDERR_NOALPHAHW             ";
  case DDERR_NOCLIPLIST         : // ( 205 )
    return "DDERR_NOCLIPLIST            ";
  case DDERR_NOCOLORCONVHW          : // ( 210 )
    return "DDERR_NOCOLORCONVHW         ";
  case DDERR_NOCOOPERATIVELEVELSET      : // ( 212 )
    return "DDERR_NOCOOPERATIVELEVELSET     ";
  case DDERR_NOCOLORKEY         : // ( 215 )
    return "DDERR_NOCOLORKEY            ";
  case DDERR_NOCOLORKEYHW           : // ( 220 )
    return "DDERR_NOCOLORKEYHW          ";
  case DDERR_NODIRECTDRAWSUPPORT        : // ( 222 )
    return "DDERR_NODIRECTDRAWSUPPORT       ";
  case DDERR_NOEXCLUSIVEMODE            : // ( 225 )
    return "DDERR_NOEXCLUSIVEMODE           ";
  case DDERR_NOFLIPHW               : // ( 230 )
    return "DDERR_NOFLIPHW              ";
  case DDERR_NOGDI              : // ( 240 )
    return "DDERR_NOGDI             ";
  case DDERR_NOMIRRORHW         : // ( 250 )
    return "DDERR_NOMIRRORHW            ";
  case DDERR_NOTFOUND               : // ( 255 )
    return "DDERR_NOTFOUND              ";
  case DDERR_NOOVERLAYHW            : // ( 260 )
    return "DDERR_NOOVERLAYHW           ";
  case DDERR_OVERLAPPINGRECTS           : // ( 270 )
    return "DDERR_OVERLAPPINGRECTS          ";
  case DDERR_NORASTEROPHW           : // ( 280 )
    return "DDERR_NORASTEROPHW          ";
  case DDERR_NOROTATIONHW           : // ( 290 )
    return "DDERR_NOROTATIONHW          ";
  case DDERR_NOSTRETCHHW            : // ( 310 )
    return "DDERR_NOSTRETCHHW           ";
  case DDERR_NOT4BITCOLOR           : // ( 316 )
    return "DDERR_NOT4BITCOLOR          ";
  case DDERR_NOT4BITCOLORINDEX          : // ( 317 )
    return "DDERR_NOT4BITCOLORINDEX         ";
  case DDERR_NOT8BITCOLOR           : // ( 320 )
    return "DDERR_NOT8BITCOLOR          ";
  case DDERR_NOTEXTUREHW            : // ( 330 )
    return "DDERR_NOTEXTUREHW           ";
  case DDERR_NOVSYNCHW              : // ( 335 )
    return "DDERR_NOVSYNCHW             ";
  case DDERR_NOZBUFFERHW            : // ( 340 )
    return "DDERR_NOZBUFFERHW           ";
  case DDERR_NOZOVERLAYHW           : // ( 350 )
    return "DDERR_NOZOVERLAYHW          ";
  case DDERR_OUTOFCAPS              : // ( 360 )
    return "DDERR_OUTOFCAPS             ";
  case DDERR_OUTOFVIDEOMEMORY           : // ( 380 )
    return "DDERR_OUTOFVIDEOMEMORY          ";
  case DDERR_OVERLAYCANTCLIP            : // ( 382 )
    return "DDERR_OVERLAYCANTCLIP           ";
  case DDERR_OVERLAYCOLORKEYONLYONEACTIVE   : // ( 384 )
    return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE  ";
  case DDERR_PALETTEBUSY            : // ( 387 )
    return "DDERR_PALETTEBUSY           ";
  case DDERR_COLORKEYNOTSET         : // ( 400 )
    return "DDERR_COLORKEYNOTSET            ";
  case DDERR_SURFACEALREADYATTACHED     : // ( 410 )
    return "DDERR_SURFACEALREADYATTACHED        ";
  case DDERR_SURFACEALREADYDEPENDENT        : // ( 420 )
    return "DDERR_SURFACEALREADYDEPENDENT       ";
  case DDERR_SURFACEBUSY            : // ( 430 )
    return "DDERR_SURFACEBUSY           ";
  case DDERR_CANTLOCKSURFACE                   : // ( 435 )
    return "DDERR_CANTLOCKSURFACE";
  case DDERR_SURFACEISOBSCURED          : // ( 440 )
    return "DDERR_SURFACEISOBSCURED         ";
  case DDERR_SURFACELOST            : // ( 450 )
    return "DDERR_SURFACELOST           ";
  case DDERR_SURFACENOTATTACHED     : // ( 460 )
    return "DDERR_SURFACENOTATTACHED        ";
  case DDERR_TOOBIGHEIGHT           : // ( 470 )
    return "DDERR_TOOBIGHEIGHT          ";
  case DDERR_TOOBIGSIZE         : // ( 480 )
    return "DDERR_TOOBIGSIZE            ";
  case DDERR_TOOBIGWIDTH            : // ( 490 )
    return "DDERR_TOOBIGWIDTH           ";
  case DDERR_UNSUPPORTEDFORMAT          : // ( 510 )
    return "DDERR_UNSUPPORTEDFORMAT         ";
  case DDERR_UNSUPPORTEDMASK            : // ( 520 )
    return "DDERR_UNSUPPORTEDMASK           ";
  case DDERR_INVALIDSTREAM                     : // ( 521 )
    return "DDERR_INVALIDSTREAM";
  case DDERR_VERTICALBLANKINPROGRESS        : // ( 537 )
    return "DDERR_VERTICALBLANKINPROGRESS       ";
  case DDERR_WASSTILLDRAWING            : // ( 540 )
    return "DDERR_WASSTILLDRAWING           ";
  case DDERR_XALIGN             : // ( 560 )
    return "DDERR_XALIGN                ";
  case DDERR_INVALIDDIRECTDRAWGUID      : // ( 561 )
    return "DDERR_INVALIDDIRECTDRAWGUID     ";
  case DDERR_DIRECTDRAWALREADYCREATED       : // ( 562 )
    return "DDERR_DIRECTDRAWALREADYCREATED      ";
  case DDERR_NODIRECTDRAWHW         : // ( 563 )
    return "DDERR_NODIRECTDRAWHW            ";
  case DDERR_PRIMARYSURFACEALREADYEXISTS    : // ( 564 )
    return "DDERR_PRIMARYSURFACEALREADYEXISTS   ";
  case DDERR_NOEMULATION            : // ( 565 )
    return "DDERR_NOEMULATION           ";
  case DDERR_REGIONTOOSMALL         : // ( 566 )
    return "DDERR_REGIONTOOSMALL            ";
  case DDERR_CLIPPERISUSINGHWND     : // ( 567 )
    return "DDERR_CLIPPERISUSINGHWND        ";
  case DDERR_NOCLIPPERATTACHED          : // ( 568 )
    return "DDERR_NOCLIPPERATTACHED         ";
  case DDERR_NOHWND             : // ( 569 )
    return "DDERR_NOHWND                ";
  case DDERR_HWNDSUBCLASSED         : // ( 570 )
    return "DDERR_HWNDSUBCLASSED            ";
  case DDERR_HWNDALREADYSET         : // ( 571 )
    return "DDERR_HWNDALREADYSET            ";
  case DDERR_NOPALETTEATTACHED          : // ( 572 )
    return "DDERR_NOPALETTEATTACHED         ";
  case DDERR_NOPALETTEHW            : // ( 573 )
    return "DDERR_NOPALETTEHW           ";
  case DDERR_BLTFASTCANTCLIP            : // ( 574 )
    return "DDERR_BLTFASTCANTCLIP           ";
  case DDERR_NOBLTHW                : // ( 575 )
    return "DDERR_NOBLTHW               ";
  case DDERR_NODDROPSHW         : // ( 576 )
    return "DDERR_NODDROPSHW            ";
  case DDERR_OVERLAYNOTVISIBLE          : // ( 577 )
    return "DDERR_OVERLAYNOTVISIBLE         ";
  case DDERR_NOOVERLAYDEST          : // ( 578 )
    return "DDERR_NOOVERLAYDEST         ";
  case DDERR_INVALIDPOSITION            : // ( 579 )
    return "DDERR_INVALIDPOSITION           ";
  case DDERR_NOTAOVERLAYSURFACE     : // ( 580 )
    return "DDERR_NOTAOVERLAYSURFACE        ";
  case DDERR_EXCLUSIVEMODEALREADYSET        : // ( 581 )
    return "DDERR_EXCLUSIVEMODEALREADYSET       ";
  case DDERR_NOTFLIPPABLE           : // ( 582 )
    return "DDERR_NOTFLIPPABLE          ";
  case DDERR_CANTDUPLICATE          : // ( 583 )
    return "DDERR_CANTDUPLICATE         ";
  case DDERR_NOTLOCKED              : // ( 584 )
    return "DDERR_NOTLOCKED             ";
  case DDERR_CANTCREATEDC           : // ( 585 )
    return "DDERR_CANTCREATEDC          ";
  case DDERR_NODC               : // ( 586 )
    return "DDERR_NODC              ";
  case DDERR_WRONGMODE              : // ( 587 )
    return "DDERR_WRONGMODE             ";
  case DDERR_IMPLICITLYCREATED          : // ( 588 )
    return "DDERR_IMPLICITLYCREATED         ";
  case DDERR_NOTPALETTIZED          : // ( 589 )
    return "DDERR_NOTPALETTIZED         ";
  case DDERR_UNSUPPORTEDMODE            : // ( 590 )
    return "DDERR_UNSUPPORTEDMODE           ";
  case DDERR_NOMIPMAPHW         : // ( 591 )
    return "DDERR_NOMIPMAPHW            ";
  case DDERR_INVALIDSURFACETYPE                : // ( 592 )
    return "DDERR_INVALIDSURFACETYPE";
  case DDERR_NOOPTIMIZEHW                      : // ( 600 )
    return "DDERR_NOOPTIMIZEHW";
  case DDERR_NOTLOADED                         : // ( 601 )
    return "DDERR_NOTLOADED";
  case DDERR_NOFOCUSWINDOW                     : // ( 602 )
    return "DDERR_NOFOCUSWINDOW";
  case DDERR_DCALREADYCREATED           : // ( 620 )
    return "DDERR_DCALREADYCREATED          ";
  case DDERR_NONONLOCALVIDMEM                  : // ( 630 )
    return "DDERR_NONONLOCALVIDMEM";
  case DDERR_CANTPAGELOCK           : // ( 640 )
    return "DDERR_CANTPAGELOCK          ";
  case DDERR_CANTPAGEUNLOCK         : // ( 660 )
    return "DDERR_CANTPAGEUNLOCK            ";
  case DDERR_NOTPAGELOCKED          : // ( 680 )
    return "DDERR_NOTPAGELOCKED         ";
  case DDERR_MOREDATA                   : // ( 690 )
    return "DDERR_MOREDATA                  ";
  case DDERR_EXPIRED                           : // ( 691 )
    return "DDERR_EXPIRED";
  case DDERR_VIDEONOTACTIVE             : // ( 695 )
    return "DDERR_VIDEONOTACTIVE            ";
  case DDERR_DEVICEDOESNTOWNSURFACE         : // ( 699 )
    return "DDERR_DEVICEDOESNTOWNSURFACE        ";
  case DXFILEERR_BADOBJECT                 : // (850)
    return "DXFILEERR_BADOBJECT";
  case DXFILEERR_BADVALUE                  : // (851)
    return "DXFILEERR_BADVALUE";
  case DXFILEERR_BADTYPE                   : // (852)
    return "DXFILEERR_BADTYPE";
  case DXFILEERR_BADSTREAMHANDLE           : // (853)
    return "DXFILEERR_BADSTREAMHANDLE";
  case DXFILEERR_BADALLOC                  : // (854)
    return "DXFILEERR_BADALLOC";
  case DXFILEERR_NOTFOUND                  : // (855)
    return "DXFILEERR_NOTFOUND";
  case DXFILEERR_NOTDONEYET                : // (856)
    return "DXFILEERR_NOTDONEYET";
  case DXFILEERR_FILENOTFOUND              : // (857)
    return "DXFILEERR_FILENOTFOUND";
  case DXFILEERR_RESOURCENOTFOUND          : // (858)
    return "DXFILEERR_RESOURCENOTFOUND";
  case DXFILEERR_URLNOTFOUND               : // (859)
    return "DXFILEERR_URLNOTFOUND";
  case DXFILEERR_BADRESOURCE               : // (860)
    return "DXFILEERR_BADRESOURCE";
  case DXFILEERR_BADFILETYPE               : // (861)
    return "DXFILEERR_BADFILETYPE";
  case DXFILEERR_BADFILEVERSION            : // (862)
    return "DXFILEERR_BADFILEVERSION";
  case DXFILEERR_BADFILEFLOATSIZE          : // (863)
    return "DXFILEERR_BADFILEFLOATSIZE";
  case DXFILEERR_BADFILECOMPRESSIONTYPE    : // (864)
    return "DXFILEERR_BADFILECOMPRESSIONTYPE";
  case DXFILEERR_BADFILE                   : // (865)
    return "DXFILEERR_BADFILE";
  case DXFILEERR_PARSEERROR                : // (866)
    return "DXFILEERR_PARSEERROR";
  case DXFILEERR_NOTEMPLATE                : // (867)
    return "DXFILEERR_NOTEMPLATE";
  case DXFILEERR_BADARRAYSIZE              : // (868)
    return "DXFILEERR_BADARRAYSIZE";
  case DXFILEERR_BADDATAREFERENCE          : // (869)
    return "DXFILEERR_BADDATAREFERENCE";
  case DXFILEERR_INTERNALERROR             : // (870)
    return "DXFILEERR_INTERNALERROR";
  case DXFILEERR_NOMOREOBJECTS             : // (871)
    return "DXFILEERR_NOMOREOBJECTS";
  case DXFILEERR_BADINTRINSICS             : // (872)
    return "DXFILEERR_BADINTRINSICS";
  case DXFILEERR_NOMORESTREAMHANDLES       : // (873)
    return "DXFILEERR_NOMORESTREAMHANDLES";
  case DXFILEERR_NOMOREDATA                : // (874)
    return "DXFILEERR_NOMOREDATA";
  case DXFILEERR_BADCACHEFILE              : // (875)
    return "DXFILEERR_BADCACHEFILE";
  case DXFILEERR_NOINTERNET                : // (876)
    return "DXFILEERR_NOINTERNET";

  case E_UNEXPECTED                     :
    return "E_UNEXPECTED                     ";
  case E_NOTIMPL                        :
    return "E_NOTIMPL                        ";
  case E_OUTOFMEMORY                    :
    return "E_OUTOFMEMORY                    ";
  case E_INVALIDARG                     :
    return "E_INVALIDARG or DDERR_INVALIDPARAMS";
  case E_NOINTERFACE                    :
    return "E_NOINTERFACE                    ";
  case E_POINTER                        :
    return "E_POINTER                        ";
  case E_HANDLE                         :
    return "E_HANDLE                         ";
  case E_ABORT                          :
    return "E_ABORT                          ";
    //    case E_FAIL                           :
    //    return "E_FAIL                           ";
  case E_ACCESSDENIED                   :
    return "E_ACCESSDENIED                   ";
  case E_PENDING                        :
    return "E_PENDING                        ";
  case CO_E_INIT_TLS                    :
    return "CO_E_INIT_TLS                    ";
  case CO_E_INIT_SHARED_ALLOCATOR       :
    return "CO_E_INIT_SHARED_ALLOCATOR       ";
  case CO_E_INIT_MEMORY_ALLOCATOR       :
    return "CO_E_INIT_MEMORY_ALLOCATOR       ";
  case CO_E_INIT_CLASS_CACHE            :
    return "CO_E_INIT_CLASS_CACHE            ";
  case CO_E_INIT_RPC_CHANNEL            :
    return "CO_E_INIT_RPC_CHANNEL            ";
  case CO_E_INIT_TLS_SET_CHANNEL_CONTROL :
    return "CO_E_INIT_TLS_SET_CHANNEL_CONTROL ";
  case CO_E_INIT_TLS_CHANNEL_CONTROL    :
    return "CO_E_INIT_TLS_CHANNEL_CONTROL    ";
  case CO_E_INIT_UNACCEPTED_USER_ALLOCATOR :
    return "CO_E_INIT_UNACCEPTED_USER_ALLOCATOR ";
  case CO_E_INIT_SCM_MUTEX_EXISTS       :
    return "CO_E_INIT_SCM_MUTEX_EXISTS       ";
  case CO_E_INIT_SCM_FILE_MAPPING_EXISTS :
    return "CO_E_INIT_SCM_FILE_MAPPING_EXISTS ";
  case CO_E_INIT_SCM_MAP_VIEW_OF_FILE   :
    return "CO_E_INIT_SCM_MAP_VIEW_OF_FILE   ";
  case CO_E_INIT_SCM_EXEC_FAILURE       :
    return "CO_E_INIT_SCM_EXEC_FAILURE       ";
  case CO_E_INIT_ONLY_SINGLE_THREADED   :
    return "CO_E_INIT_ONLY_SINGLE_THREADED   ";
  case CO_E_CANT_REMOTE                 :
    return "CO_E_CANT_REMOTE                 ";
  case CO_E_BAD_SERVER_NAME             :
    return "CO_E_BAD_SERVER_NAME             ";
  case CO_E_WRONG_SERVER_IDENTITY       :
    return "CO_E_WRONG_SERVER_IDENTITY       ";
  case CO_E_OLE1DDE_DISABLED            :
    return "CO_E_OLE1DDE_DISABLED            ";
  case CO_E_RUNAS_SYNTAX                :
    return "CO_E_RUNAS_SYNTAX                ";
  case CO_E_CREATEPROCESS_FAILURE       :
    return "CO_E_CREATEPROCESS_FAILURE       ";
  case CO_E_RUNAS_CREATEPROCESS_FAILURE :
    return "CO_E_RUNAS_CREATEPROCESS_FAILURE ";
  case CO_E_RUNAS_LOGON_FAILURE         :
    return "CO_E_RUNAS_LOGON_FAILURE         ";
  case CO_E_LAUNCH_PERMSSION_DENIED     :
    return "CO_E_LAUNCH_PERMSSION_DENIED     ";
  case CO_E_START_SERVICE_FAILURE       :
    return "CO_E_START_SERVICE_FAILURE       ";
  case CO_E_REMOTE_COMMUNICATION_FAILURE :
    return "CO_E_REMOTE_COMMUNICATION_FAILURE ";
  case CO_E_SERVER_START_TIMEOUT        :
    return "CO_E_SERVER_START_TIMEOUT        ";
  case CO_E_CLSREG_INCONSISTENT         :
    return "CO_E_CLSREG_INCONSISTENT         ";
  case CO_E_IIDREG_INCONSISTENT         :
    return "CO_E_IIDREG_INCONSISTENT         ";
  case CO_E_NOT_SUPPORTED               :
    return "CO_E_NOT_SUPPORTED               ";
  case CO_E_RELOAD_DLL                  :
    return "CO_E_RELOAD_DLL                  ";
  case CO_E_MSI_ERROR                   :
    return "CO_E_MSI_ERROR                   ";
  case OLE_E_OLEVERB                    :
    return "OLE_E_OLEVERB                    ";
  case OLE_E_ADVF                       :
    return "OLE_E_ADVF                       ";
  case OLE_E_ENUM_NOMORE                :
    return "OLE_E_ENUM_NOMORE                ";
  case OLE_E_ADVISENOTSUPPORTED         :
    return "OLE_E_ADVISENOTSUPPORTED         ";
  case OLE_E_NOCONNECTION               :
    return "OLE_E_NOCONNECTION               ";
  case OLE_E_NOTRUNNING                 :
    return "OLE_E_NOTRUNNING                 ";
  case OLE_E_NOCACHE                    :
    return "OLE_E_NOCACHE                    ";
  case OLE_E_BLANK                      :
    return "OLE_E_BLANK                      ";
  case OLE_E_CLASSDIFF                  :
    return "OLE_E_CLASSDIFF                  ";
  case OLE_E_CANT_GETMONIKER            :
    return "OLE_E_CANT_GETMONIKER            ";
  case OLE_E_CANT_BINDTOSOURCE          :
    return "OLE_E_CANT_BINDTOSOURCE          ";
  case OLE_E_STATIC                     :
    return "OLE_E_STATIC                     ";
  case OLE_E_PROMPTSAVECANCELLED        :
    return "OLE_E_PROMPTSAVECANCELLED        ";
  case OLE_E_INVALIDRECT                :
    return "OLE_E_INVALIDRECT                ";
  case OLE_E_WRONGCOMPOBJ               :
    return "OLE_E_WRONGCOMPOBJ               ";
  case OLE_E_INVALIDHWND                :
    return "OLE_E_INVALIDHWND                ";
  case OLE_E_NOT_INPLACEACTIVE          :
    return "OLE_E_NOT_INPLACEACTIVE          ";
  case OLE_E_CANTCONVERT                :
    return "OLE_E_CANTCONVERT                ";
  case OLE_E_NOSTORAGE                  :
    return "OLE_E_NOSTORAGE                  ";
  case DV_E_FORMATETC                   :
    return "DV_E_FORMATETC                   ";
  case DV_E_DVTARGETDEVICE              :
    return "DV_E_DVTARGETDEVICE              ";
  case DV_E_STGMEDIUM                   :
    return "DV_E_STGMEDIUM                   ";
  case DV_E_STATDATA                    :
    return "DV_E_STATDATA                    ";
  case DV_E_LINDEX                      :
    return "DV_E_LINDEX                      ";
  case DV_E_TYMED                       :
    return "DV_E_TYMED                       ";
  case DV_E_CLIPFORMAT                  :
    return "DV_E_CLIPFORMAT                  ";
  case DV_E_DVASPECT                    :
    return "DV_E_DVASPECT                    ";
  case DV_E_DVTARGETDEVICE_SIZE         :
    return "DV_E_DVTARGETDEVICE_SIZE         ";
  case DV_E_NOIVIEWOBJECT               :
    return "DV_E_NOIVIEWOBJECT               ";
  case DRAGDROP_E_NOTREGISTERED         :
    return "DRAGDROP_E_NOTREGISTERED         ";
  case DRAGDROP_E_ALREADYREGISTERED     :
    return "DRAGDROP_E_ALREADYREGISTERED     ";
  case DRAGDROP_E_INVALIDHWND           :
    return "DRAGDROP_E_INVALIDHWND           ";
  case CLASS_E_NOAGGREGATION            :
    return "CLASS_E_NOAGGREGATION            ";
  case CLASS_E_CLASSNOTAVAILABLE        :
    return "CLASS_E_CLASSNOTAVAILABLE        ";
  case CLASS_E_NOTLICENSED              :
    return "CLASS_E_NOTLICENSED              ";
  case VIEW_E_DRAW                      :
    return "VIEW_E_DRAW                      ";
  case REGDB_E_READREGDB                :
    return "REGDB_E_READREGDB                ";
  case REGDB_E_WRITEREGDB               :
    return "REGDB_E_WRITEREGDB               ";
  case REGDB_E_KEYMISSING               :
    return "REGDB_E_KEYMISSING               ";
  case REGDB_E_INVALIDVALUE             :
    return "REGDB_E_INVALIDVALUE             ";
  case REGDB_E_CLASSNOTREG              :
    return "REGDB_E_CLASSNOTREG              ";
  case REGDB_E_IIDNOTREG                :
    return "REGDB_E_IIDNOTREG                ";
  case CAT_E_CATIDNOEXIST               :
    return "CAT_E_CATIDNOEXIST               ";
  case CAT_E_NODESCRIPTION              :
    return "CAT_E_NODESCRIPTION              ";
  case CS_E_PACKAGE_NOTFOUND            :
    return "CS_E_PACKAGE_NOTFOUND            ";
  case CS_E_NOT_DELETABLE               :
    return "CS_E_NOT_DELETABLE               ";
  case CS_E_CLASS_NOTFOUND              :
    return "CS_E_CLASS_NOTFOUND              ";
  case CS_E_INVALID_VERSION             :
    return "CS_E_INVALID_VERSION             ";
  case CS_E_NO_CLASSSTORE               :
    return "CS_E_NO_CLASSSTORE               ";
  case CACHE_E_NOCACHE_UPDATED          :
    return "CACHE_E_NOCACHE_UPDATED          ";
  case OLEOBJ_E_NOVERBS                 :
    return "OLEOBJ_E_NOVERBS                 ";
  case OLEOBJ_E_INVALIDVERB             :
    return "OLEOBJ_E_INVALIDVERB             ";
  case INPLACE_E_NOTUNDOABLE            :
    return "INPLACE_E_NOTUNDOABLE            ";
  case INPLACE_E_NOTOOLSPACE            :
    return "INPLACE_E_NOTOOLSPACE            ";
  case CONVERT10_E_OLESTREAM_GET        :
    return "CONVERT10_E_OLESTREAM_GET        ";
  case CONVERT10_E_OLESTREAM_PUT        :
    return "CONVERT10_E_OLESTREAM_PUT        ";
  case CONVERT10_E_OLESTREAM_FMT        :
    return "CONVERT10_E_OLESTREAM_FMT        ";
  case CONVERT10_E_OLESTREAM_BITMAP_TO_DIB :
    return "CONVERT10_E_OLESTREAM_BITMAP_TO_DIB ";
  case CONVERT10_E_STG_FMT              :
    return "CONVERT10_E_STG_FMT              ";
  case CONVERT10_E_STG_NO_STD_STREAM    :
    return "CONVERT10_E_STG_NO_STD_STREAM    ";
  case CONVERT10_E_STG_DIB_TO_BITMAP    :
    return "CONVERT10_E_STG_DIB_TO_BITMAP    ";
  case CLIPBRD_E_CANT_OPEN              :
    return "CLIPBRD_E_CANT_OPEN              ";
  case CLIPBRD_E_CANT_EMPTY             :
    return "CLIPBRD_E_CANT_EMPTY             ";
  case CLIPBRD_E_CANT_SET               :
    return "CLIPBRD_E_CANT_SET               ";
  case CLIPBRD_E_BAD_DATA               :
    return "CLIPBRD_E_BAD_DATA               ";
  case CLIPBRD_E_CANT_CLOSE             :
    return "CLIPBRD_E_CANT_CLOSE             ";
  case MK_E_CONNECTMANUALLY             :
    return "MK_E_CONNECTMANUALLY             ";
  case MK_E_EXCEEDEDDEADLINE            :
    return "MK_E_EXCEEDEDDEADLINE            ";
  case MK_E_NEEDGENERIC                 :
    return "MK_E_NEEDGENERIC                 ";
  case MK_E_UNAVAILABLE                 :
    return "MK_E_UNAVAILABLE                 ";
  case MK_E_SYNTAX                      :
    return "MK_E_SYNTAX                      ";
  case MK_E_NOOBJECT                    :
    return "MK_E_NOOBJECT                    ";
  case MK_E_INVALIDEXTENSION            :
    return "MK_E_INVALIDEXTENSION            ";
  case MK_E_INTERMEDIATEINTERFACENOTSUPPORTED :
    return "MK_E_INTERMEDIATEINTERFACENOTSUPPORTED ";
  case MK_E_NOTBINDABLE                 :
    return "MK_E_NOTBINDABLE                 ";
  case MK_E_NOTBOUND                    :
    return "MK_E_NOTBOUND                    ";
  case MK_E_CANTOPENFILE                :
    return "MK_E_CANTOPENFILE                ";
  case MK_E_MUSTBOTHERUSER              :
    return "MK_E_MUSTBOTHERUSER              ";
  case MK_E_NOINVERSE                   :
    return "MK_E_NOINVERSE                   ";
  case MK_E_NOSTORAGE                   :
    return "MK_E_NOSTORAGE                   ";
  case MK_E_NOPREFIX                    :
    return "MK_E_NOPREFIX                    ";
  case MK_E_ENUMERATION_FAILED          :
    return "MK_E_ENUMERATION_FAILED          ";
  case CO_E_NOTINITIALIZED              :
    return "CO_E_NOTINITIALIZED              ";
  case CO_E_ALREADYINITIALIZED          :
    return "CO_E_ALREADYINITIALIZED          ";
  case CO_E_CANTDETERMINECLASS          :
    return "CO_E_CANTDETERMINECLASS          ";
  case CO_E_CLASSSTRING                 :
    return "CO_E_CLASSSTRING                 ";
  case CO_E_IIDSTRING                   :
    return "CO_E_IIDSTRING                   ";
  case CO_E_APPNOTFOUND                 :
    return "CO_E_APPNOTFOUND                 ";
  case CO_E_APPSINGLEUSE                :
    return "CO_E_APPSINGLEUSE                ";
  case CO_E_ERRORINAPP                  :
    return "CO_E_ERRORINAPP                  ";
  case CO_E_DLLNOTFOUND                 :
    return "CO_E_DLLNOTFOUND                 ";
  case CO_E_ERRORINDLL                  :
    return "CO_E_ERRORINDLL                  ";
  case CO_E_WRONGOSFORAPP               :
    return "CO_E_WRONGOSFORAPP               ";
  case CO_E_OBJNOTREG                   :
    return "CO_E_OBJNOTREG                   ";
  case CO_E_OBJISREG                    :
    return "CO_E_OBJISREG                    ";
  case CO_E_OBJNOTCONNECTED             :
    return "CO_E_OBJNOTCONNECTED             ";
  case CO_E_APPDIDNTREG                 :
    return "CO_E_APPDIDNTREG                 ";
  case CO_E_RELEASED                    :
    return "CO_E_RELEASED                    ";
  case CO_E_FAILEDTOIMPERSONATE         :
    return "CO_E_FAILEDTOIMPERSONATE         ";
  case CO_E_FAILEDTOGETSECCTX           :
    return "CO_E_FAILEDTOGETSECCTX           ";
  case CO_E_FAILEDTOOPENTHREADTOKEN     :
    return "CO_E_FAILEDTOOPENTHREADTOKEN     ";
  case CO_E_FAILEDTOGETTOKENINFO        :
    return "CO_E_FAILEDTOGETTOKENINFO        ";
  case CO_E_TRUSTEEDOESNTMATCHCLIENT    :
    return "CO_E_TRUSTEEDOESNTMATCHCLIENT    ";
  case CO_E_FAILEDTOQUERYCLIENTBLANKET  :
    return "CO_E_FAILEDTOQUERYCLIENTBLANKET  ";
  case CO_E_FAILEDTOSETDACL             :
    return "CO_E_FAILEDTOSETDACL             ";
  case CO_E_ACCESSCHECKFAILED           :
    return "CO_E_ACCESSCHECKFAILED           ";
  case CO_E_NETACCESSAPIFAILED          :
    return "CO_E_NETACCESSAPIFAILED          ";
  case CO_E_WRONGTRUSTEENAMESYNTAX      :
    return "CO_E_WRONGTRUSTEENAMESYNTAX      ";
  case CO_E_INVALIDSID                  :
    return "CO_E_INVALIDSID                  ";
  case CO_E_CONVERSIONFAILED            :
    return "CO_E_CONVERSIONFAILED            ";
  case CO_E_NOMATCHINGSIDFOUND          :
    return "CO_E_NOMATCHINGSIDFOUND          ";
  case CO_E_LOOKUPACCSIDFAILED          :
    return "CO_E_LOOKUPACCSIDFAILED          ";
  case CO_E_NOMATCHINGNAMEFOUND         :
    return "CO_E_NOMATCHINGNAMEFOUND         ";
  case CO_E_LOOKUPACCNAMEFAILED         :
    return "CO_E_LOOKUPACCNAMEFAILED         ";
  case CO_E_SETSERLHNDLFAILED           :
    return "CO_E_SETSERLHNDLFAILED           ";
  case CO_E_FAILEDTOGETWINDIR           :
    return "CO_E_FAILEDTOGETWINDIR           ";
  case CO_E_PATHTOOLONG                 :
    return "CO_E_PATHTOOLONG                 ";
  case CO_E_FAILEDTOGENUUID             :
    return "CO_E_FAILEDTOGENUUID             ";
  case CO_E_FAILEDTOCREATEFILE          :
    return "CO_E_FAILEDTOCREATEFILE          ";
  case CO_E_FAILEDTOCLOSEHANDLE         :
    return "CO_E_FAILEDTOCLOSEHANDLE         ";
  case CO_E_EXCEEDSYSACLLIMIT           :
    return "CO_E_EXCEEDSYSACLLIMIT           ";
  case CO_E_ACESINWRONGORDER            :
    return "CO_E_ACESINWRONGORDER            ";
  case CO_E_INCOMPATIBLESTREAMVERSION   :
    return "CO_E_INCOMPATIBLESTREAMVERSION   ";
  case CO_E_FAILEDTOOPENPROCESSTOKEN    :
    return "CO_E_FAILEDTOOPENPROCESSTOKEN    ";
  case CO_E_DECODEFAILED                :
    return "CO_E_DECODEFAILED                ";
  case CO_E_ACNOTINITIALIZED            :
    return "CO_E_ACNOTINITIALIZED            ";
  case OLE_S_USEREG                     :
    return "OLE_S_USEREG                     ";
  case OLE_S_STATIC                     :
    return "OLE_S_STATIC                     ";
  case OLE_S_MAC_CLIPFORMAT             :
    return "OLE_S_MAC_CLIPFORMAT             ";
  case DRAGDROP_S_DROP                  :
    return "DRAGDROP_S_DROP                  ";
  case DRAGDROP_S_CANCEL                :
    return "DRAGDROP_S_CANCEL                ";
  case DRAGDROP_S_USEDEFAULTCURSORS     :
    return "DRAGDROP_S_USEDEFAULTCURSORS     ";
  case DATA_S_SAMEFORMATETC             :
    return "DATA_S_SAMEFORMATETC             ";
  case VIEW_S_ALREADY_FROZEN            :
    return "VIEW_S_ALREADY_FROZEN            ";
  case CACHE_S_FORMATETC_NOTSUPPORTED   :
    return "CACHE_S_FORMATETC_NOTSUPPORTED   ";
  case CACHE_S_SAMECACHE                :
    return "CACHE_S_SAMECACHE                ";
  case CACHE_S_SOMECACHES_NOTUPDATED    :
    return "CACHE_S_SOMECACHES_NOTUPDATED    ";
  case OLEOBJ_S_INVALIDVERB             :
    return "OLEOBJ_S_INVALIDVERB             ";
  case OLEOBJ_S_CANNOT_DOVERB_NOW       :
    return "OLEOBJ_S_CANNOT_DOVERB_NOW       ";
  case OLEOBJ_S_INVALIDHWND             :
    return "OLEOBJ_S_INVALIDHWND             ";
  case INPLACE_S_TRUNCATED              :
    return "INPLACE_S_TRUNCATED              ";
  case CONVERT10_S_NO_PRESENTATION      :
    return "CONVERT10_S_NO_PRESENTATION      ";
  case MK_S_REDUCED_TO_SELF             :
    return "MK_S_REDUCED_TO_SELF             ";
  case MK_S_ME                          :
    return "MK_S_ME                          ";
  case MK_S_HIM                         :
    return "MK_S_HIM                         ";
  case MK_S_US                          :
    return "MK_S_US                          ";
  case MK_S_MONIKERALREADYREGISTERED    :
    return "MK_S_MONIKERALREADYREGISTERED    ";
  case CO_E_CLASS_CREATE_FAILED         :
    return "CO_E_CLASS_CREATE_FAILED         ";
  case CO_E_SCM_ERROR                   :
    return "CO_E_SCM_ERROR                   ";
  case CO_E_SCM_RPC_FAILURE             :
    return "CO_E_SCM_RPC_FAILURE             ";
  case CO_E_BAD_PATH                    :
    return "CO_E_BAD_PATH                    ";
  case CO_E_SERVER_EXEC_FAILURE         :
    return "CO_E_SERVER_EXEC_FAILURE         ";
  case CO_E_OBJSRV_RPC_FAILURE          :
    return "CO_E_OBJSRV_RPC_FAILURE          ";
  case MK_E_NO_NORMALIZED               :
    return "MK_E_NO_NORMALIZED               ";
  case CO_E_SERVER_STOPPING             :
    return "CO_E_SERVER_STOPPING             ";
  case MEM_E_INVALID_ROOT               :
    return "MEM_E_INVALID_ROOT               ";
  case MEM_E_INVALID_LINK               :
    return "MEM_E_INVALID_LINK               ";
  case MEM_E_INVALID_SIZE               :
    return "MEM_E_INVALID_SIZE               ";
  case CO_S_NOTALLINTERFACES            :
    return "CO_S_NOTALLINTERFACES            ";
  case DISP_E_UNKNOWNINTERFACE          :
    return "DISP_E_UNKNOWNINTERFACE          ";
  case DISP_E_MEMBERNOTFOUND            :
    return "DISP_E_MEMBERNOTFOUND            ";
  case DISP_E_PARAMNOTFOUND             :
    return "DISP_E_PARAMNOTFOUND             ";
  case DISP_E_TYPEMISMATCH              :
    return "DISP_E_TYPEMISMATCH              ";
  case DISP_E_UNKNOWNNAME               :
    return "DISP_E_UNKNOWNNAME               ";
  case DISP_E_NONAMEDARGS               :
    return "DISP_E_NONAMEDARGS               ";
  case DISP_E_BADVARTYPE                :
    return "DISP_E_BADVARTYPE                ";
  case DISP_E_EXCEPTION                 :
    return "DISP_E_EXCEPTION                 ";
  case DISP_E_OVERFLOW                  :
    return "DISP_E_OVERFLOW                  ";
  case DISP_E_BADINDEX                  :
    return "DISP_E_BADINDEX                  ";
  case DISP_E_UNKNOWNLCID               :
    return "DISP_E_UNKNOWNLCID               ";
  case DISP_E_ARRAYISLOCKED             :
    return "DISP_E_ARRAYISLOCKED             ";
  case DISP_E_BADPARAMCOUNT             :
    return "DISP_E_BADPARAMCOUNT             ";
  case DISP_E_PARAMNOTOPTIONAL          :
    return "DISP_E_PARAMNOTOPTIONAL          ";
  case DISP_E_BADCALLEE                 :
    return "DISP_E_BADCALLEE                 ";
  case DISP_E_NOTACOLLECTION            :
    return "DISP_E_NOTACOLLECTION            ";
  case DISP_E_DIVBYZERO                 :
    return "DISP_E_DIVBYZERO                 ";
  case TYPE_E_BUFFERTOOSMALL            :
    return "TYPE_E_BUFFERTOOSMALL            ";
  case TYPE_E_FIELDNOTFOUND             :
    return "TYPE_E_FIELDNOTFOUND             ";
  case TYPE_E_INVDATAREAD               :
    return "TYPE_E_INVDATAREAD               ";
  case TYPE_E_UNSUPFORMAT               :
    return "TYPE_E_UNSUPFORMAT               ";
  case TYPE_E_REGISTRYACCESS            :
    return "TYPE_E_REGISTRYACCESS            ";
  case TYPE_E_LIBNOTREGISTERED          :
    return "TYPE_E_LIBNOTREGISTERED          ";
  case TYPE_E_UNDEFINEDTYPE             :
    return "TYPE_E_UNDEFINEDTYPE             ";
  case TYPE_E_QUALIFIEDNAMEDISALLOWED   :
    return "TYPE_E_QUALIFIEDNAMEDISALLOWED   ";
  case TYPE_E_INVALIDSTATE              :
    return "TYPE_E_INVALIDSTATE              ";
  case TYPE_E_WRONGTYPEKIND             :
    return "TYPE_E_WRONGTYPEKIND             ";
  case TYPE_E_ELEMENTNOTFOUND           :
    return "TYPE_E_ELEMENTNOTFOUND           ";
  case TYPE_E_AMBIGUOUSNAME             :
    return "TYPE_E_AMBIGUOUSNAME             ";
  case TYPE_E_NAMECONFLICT              :
    return "TYPE_E_NAMECONFLICT              ";
  case TYPE_E_UNKNOWNLCID               :
    return "TYPE_E_UNKNOWNLCID               ";
  case TYPE_E_DLLFUNCTIONNOTFOUND       :
    return "TYPE_E_DLLFUNCTIONNOTFOUND       ";
  case TYPE_E_BADMODULEKIND             :
    return "TYPE_E_BADMODULEKIND             ";
  case TYPE_E_SIZETOOBIG                :
    return "TYPE_E_SIZETOOBIG                ";
  case TYPE_E_DUPLICATEID               :
    return "TYPE_E_DUPLICATEID               ";
  case TYPE_E_INVALIDID                 :
    return "TYPE_E_INVALIDID                 ";
  case TYPE_E_TYPEMISMATCH              :
    return "TYPE_E_TYPEMISMATCH              ";
  case TYPE_E_OUTOFBOUNDS               :
    return "TYPE_E_OUTOFBOUNDS               ";
  case TYPE_E_IOERROR                   :
    return "TYPE_E_IOERROR                   ";
  case TYPE_E_CANTCREATETMPFILE         :
    return "TYPE_E_CANTCREATETMPFILE         ";
  case TYPE_E_CANTLOADLIBRARY           :
    return "TYPE_E_CANTLOADLIBRARY           ";
  case TYPE_E_INCONSISTENTPROPFUNCS     :
    return "TYPE_E_INCONSISTENTPROPFUNCS     ";
  case TYPE_E_CIRCULARTYPE              :
    return "TYPE_E_CIRCULARTYPE              ";
  case STG_E_INVALIDFUNCTION            :
    return "STG_E_INVALIDFUNCTION            ";
  case STG_E_FILENOTFOUND               :
    return "STG_E_FILENOTFOUND               ";
  case STG_E_PATHNOTFOUND               :
    return "STG_E_PATHNOTFOUND               ";
  case STG_E_TOOMANYOPENFILES           :
    return "STG_E_TOOMANYOPENFILES           ";
  case STG_E_ACCESSDENIED               :
    return "STG_E_ACCESSDENIED               ";
  case STG_E_INVALIDHANDLE              :
    return "STG_E_INVALIDHANDLE              ";
  case STG_E_INSUFFICIENTMEMORY         :
    return "STG_E_INSUFFICIENTMEMORY         ";
  case STG_E_INVALIDPOINTER             :
    return "STG_E_INVALIDPOINTER             ";
  case STG_E_NOMOREFILES                :
    return "STG_E_NOMOREFILES                ";
  case STG_E_DISKISWRITEPROTECTED       :
    return "STG_E_DISKISWRITEPROTECTED       ";
  case STG_E_SEEKERROR                  :
    return "STG_E_SEEKERROR                  ";
  case STG_E_WRITEFAULT                 :
    return "STG_E_WRITEFAULT                 ";
  case STG_E_READFAULT                  :
    return "STG_E_READFAULT                  ";
  case STG_E_SHAREVIOLATION             :
    return "STG_E_SHAREVIOLATION             ";
  case STG_E_LOCKVIOLATION              :
    return "STG_E_LOCKVIOLATION              ";
  case STG_E_FILEALREADYEXISTS          :
    return "STG_E_FILEALREADYEXISTS          ";
  case STG_E_INVALIDPARAMETER           :
    return "STG_E_INVALIDPARAMETER           ";
  case STG_E_MEDIUMFULL                 :
    return "STG_E_MEDIUMFULL                 ";
  case STG_E_PROPSETMISMATCHED          :
    return "STG_E_PROPSETMISMATCHED          ";
  case STG_E_ABNORMALAPIEXIT            :
    return "STG_E_ABNORMALAPIEXIT            ";
  case STG_E_INVALIDHEADER              :
    return "STG_E_INVALIDHEADER              ";
  case STG_E_INVALIDNAME                :
    return "STG_E_INVALIDNAME                ";
  case STG_E_UNKNOWN                    :
    return "STG_E_UNKNOWN                    ";
  case STG_E_UNIMPLEMENTEDFUNCTION      :
    return "STG_E_UNIMPLEMENTEDFUNCTION      ";
  case STG_E_INVALIDFLAG                :
    return "STG_E_INVALIDFLAG                ";
  case STG_E_INUSE                      :
    return "STG_E_INUSE                      ";
  case STG_E_NOTCURRENT                 :
    return "STG_E_NOTCURRENT                 ";
  case STG_E_REVERTED                   :
    return "STG_E_REVERTED                   ";
  case STG_E_CANTSAVE                   :
    return "STG_E_CANTSAVE                   ";
  case STG_E_OLDFORMAT                  :
    return "STG_E_OLDFORMAT                  ";
  case STG_E_OLDDLL                     :
    return "STG_E_OLDDLL                     ";
  case STG_E_SHAREREQUIRED              :
    return "STG_E_SHAREREQUIRED              ";
  case STG_E_NOTFILEBASEDSTORAGE        :
    return "STG_E_NOTFILEBASEDSTORAGE        ";
  case STG_E_EXTANTMARSHALLINGS         :
    return "STG_E_EXTANTMARSHALLINGS         ";
  case STG_E_DOCFILECORRUPT             :
    return "STG_E_DOCFILECORRUPT             ";
  case STG_E_BADBASEADDRESS             :
    return "STG_E_BADBASEADDRESS             ";
  case STG_E_INCOMPLETE                 :
    return "STG_E_INCOMPLETE                 ";
  case STG_E_TERMINATED                 :
    return "STG_E_TERMINATED                 ";
  case STG_S_CONVERTED                  :
    return "STG_S_CONVERTED                  ";
  case STG_S_BLOCK                      :
    return "STG_S_BLOCK                      ";
  case STG_S_RETRYNOW                   :
    return "STG_S_RETRYNOW                   ";
  case STG_S_MONITORING                 :
    return "STG_S_MONITORING                 ";
  case STG_S_MULTIPLEOPENS              :
    return "STG_S_MULTIPLEOPENS              ";
  case STG_S_CONSOLIDATIONFAILED        :
    return "STG_S_CONSOLIDATIONFAILED        ";
  case STG_S_CANNOTCONSOLIDATE          :
    return "STG_S_CANNOTCONSOLIDATE          ";
  case RPC_E_CALL_REJECTED              :
    return "RPC_E_CALL_REJECTED              ";
  case RPC_E_CALL_CANCELED              :
    return "RPC_E_CALL_CANCELED              ";
  case RPC_E_CANTPOST_INSENDCALL        :
    return "RPC_E_CANTPOST_INSENDCALL        ";
  case RPC_E_CANTCALLOUT_INASYNCCALL    :
    return "RPC_E_CANTCALLOUT_INASYNCCALL    ";
  case RPC_E_CANTCALLOUT_INEXTERNALCALL :
    return "RPC_E_CANTCALLOUT_INEXTERNALCALL ";
  case RPC_E_CONNECTION_TERMINATED      :
    return "RPC_E_CONNECTION_TERMINATED      ";
  case RPC_E_SERVER_DIED                :
    return "RPC_E_SERVER_DIED                ";
  case RPC_E_CLIENT_DIED                :
    return "RPC_E_CLIENT_DIED                ";
  case RPC_E_INVALID_DATAPACKET         :
    return "RPC_E_INVALID_DATAPACKET         ";
  case RPC_E_CANTTRANSMIT_CALL          :
    return "RPC_E_CANTTRANSMIT_CALL          ";
  case RPC_E_CLIENT_CANTMARSHAL_DATA    :
    return "RPC_E_CLIENT_CANTMARSHAL_DATA    ";
  case RPC_E_CLIENT_CANTUNMARSHAL_DATA  :
    return "RPC_E_CLIENT_CANTUNMARSHAL_DATA  ";
  case RPC_E_SERVER_CANTMARSHAL_DATA    :
    return "RPC_E_SERVER_CANTMARSHAL_DATA    ";
  case RPC_E_SERVER_CANTUNMARSHAL_DATA  :
    return "RPC_E_SERVER_CANTUNMARSHAL_DATA  ";
  case RPC_E_INVALID_DATA               :
    return "RPC_E_INVALID_DATA               ";
  case RPC_E_INVALID_PARAMETER          :
    return "RPC_E_INVALID_PARAMETER          ";
  case RPC_E_CANTCALLOUT_AGAIN          :
    return "RPC_E_CANTCALLOUT_AGAIN          ";
  case RPC_E_SERVER_DIED_DNE            :
    return "RPC_E_SERVER_DIED_DNE            ";
  case RPC_E_SYS_CALL_FAILED            :
    return "RPC_E_SYS_CALL_FAILED            ";
  case RPC_E_OUT_OF_RESOURCES           :
    return "RPC_E_OUT_OF_RESOURCES           ";
  case RPC_E_ATTEMPTED_MULTITHREAD      :
    return "RPC_E_ATTEMPTED_MULTITHREAD      ";
  case RPC_E_NOT_REGISTERED             :
    return "RPC_E_NOT_REGISTERED             ";
  case RPC_E_FAULT                      :
    return "RPC_E_FAULT                      ";
  case RPC_E_SERVERFAULT                :
    return "RPC_E_SERVERFAULT                ";
  case RPC_E_CHANGED_MODE               :
    return "RPC_E_CHANGED_MODE               ";
  case RPC_E_INVALIDMETHOD              :
    return "RPC_E_INVALIDMETHOD              ";
  case RPC_E_DISCONNECTED               :
    return "RPC_E_DISCONNECTED               ";
  case RPC_E_RETRY                      :
    return "RPC_E_RETRY                      ";
  case RPC_E_SERVERCALL_RETRYLATER      :
    return "RPC_E_SERVERCALL_RETRYLATER      ";
  case RPC_E_SERVERCALL_REJECTED        :
    return "RPC_E_SERVERCALL_REJECTED        ";
  case RPC_E_INVALID_CALLDATA           :
    return "RPC_E_INVALID_CALLDATA           ";
  case RPC_E_CANTCALLOUT_ININPUTSYNCCALL :
    return "RPC_E_CANTCALLOUT_ININPUTSYNCCALL ";
  case RPC_E_WRONG_THREAD               :
    return "RPC_E_WRONG_THREAD               ";
  case RPC_E_THREAD_NOT_INIT            :
    return "RPC_E_THREAD_NOT_INIT            ";
  case RPC_E_VERSION_MISMATCH           :
    return "RPC_E_VERSION_MISMATCH           ";
  case RPC_E_INVALID_HEADER             :
    return "RPC_E_INVALID_HEADER             ";
  case RPC_E_INVALID_EXTENSION          :
    return "RPC_E_INVALID_EXTENSION          ";
  case RPC_E_INVALID_IPID               :
    return "RPC_E_INVALID_IPID               ";
  case RPC_E_INVALID_OBJECT             :
    return "RPC_E_INVALID_OBJECT             ";
  case RPC_S_CALLPENDING                :
    return "RPC_S_CALLPENDING                ";
  case RPC_S_WAITONTIMER                :
    return "RPC_S_WAITONTIMER                ";
  case RPC_E_CALL_COMPLETE              :
    return "RPC_E_CALL_COMPLETE              ";
  case RPC_E_UNSECURE_CALL              :
    return "RPC_E_UNSECURE_CALL              ";
  case RPC_E_TOO_LATE                   :
    return "RPC_E_TOO_LATE                   ";
  case RPC_E_NO_GOOD_SECURITY_PACKAGES  :
    return "RPC_E_NO_GOOD_SECURITY_PACKAGES  ";
  case RPC_E_ACCESS_DENIED              :
    return "RPC_E_ACCESS_DENIED              ";
  case RPC_E_REMOTE_DISABLED            :
    return "RPC_E_REMOTE_DISABLED            ";
  case RPC_E_INVALID_OBJREF             :
    return "RPC_E_INVALID_OBJREF             ";
  case RPC_E_NO_CONTEXT                 :
    return "RPC_E_NO_CONTEXT                 ";
  case RPC_E_TIMEOUT                    :
    return "RPC_E_TIMEOUT                    ";
  case RPC_E_NO_SYNC                    :
    return "RPC_E_NO_SYNC                    ";
  case RPC_E_UNEXPECTED                 :
    return "RPC_E_UNEXPECTED                 ";
  case NTE_BAD_UID                      :
    return "NTE_BAD_UID                      ";
  case NTE_BAD_HASH                     :
    return "NTE_BAD_HASH                     ";
    //case NTE_BAD_HASH                     :
    //return "NTE_BAD_HASH                     ";
  case NTE_BAD_KEY                      :
    return "NTE_BAD_KEY                      ";
  case NTE_BAD_LEN                      :
    return "NTE_BAD_LEN                      ";
  case NTE_BAD_DATA                     :
    return "NTE_BAD_DATA                     ";
  case NTE_BAD_SIGNATURE                :
    return "NTE_BAD_SIGNATURE                ";
  case NTE_BAD_VER                      :
    return "NTE_BAD_VER                      ";
  case NTE_BAD_ALGID                    :
    return "NTE_BAD_ALGID                    ";
  case NTE_BAD_FLAGS                    :
    return "NTE_BAD_FLAGS                    ";
  case NTE_BAD_TYPE                     :
    return "NTE_BAD_TYPE                     ";
  case NTE_BAD_KEY_STATE                :
    return "NTE_BAD_KEY_STATE                ";
  case NTE_BAD_HASH_STATE               :
    return "NTE_BAD_HASH_STATE               ";
  case NTE_NO_KEY                       :
    return "NTE_NO_KEY                       ";
  case NTE_NO_MEMORY                    :
    return "NTE_NO_MEMORY                    ";
  case NTE_EXISTS                       :
    return "NTE_EXISTS                       ";
  case NTE_PERM                         :
    return "NTE_PERM                         ";
  case NTE_NOT_FOUND                    :
    return "NTE_NOT_FOUND                    ";
  case NTE_DOUBLE_ENCRYPT               :
    return "NTE_DOUBLE_ENCRYPT               ";
  case NTE_BAD_PROVIDER                 :
    return "NTE_BAD_PROVIDER                 ";
  case NTE_BAD_PROV_TYPE                :
    return "NTE_BAD_PROV_TYPE                ";
  case NTE_BAD_PUBLIC_KEY               :
    return "NTE_BAD_PUBLIC_KEY               ";
  case NTE_BAD_KEYSET                   :
    return "NTE_BAD_KEYSET                   ";
  case NTE_PROV_TYPE_NOT_DEF            :
    return "NTE_PROV_TYPE_NOT_DEF            ";
  case NTE_PROV_TYPE_ENTRY_BAD          :
    return "NTE_PROV_TYPE_ENTRY_BAD          ";
  case NTE_KEYSET_NOT_DEF               :
    return "NTE_KEYSET_NOT_DEF               ";
  case NTE_KEYSET_ENTRY_BAD             :
    return "NTE_KEYSET_ENTRY_BAD             ";
  case NTE_PROV_TYPE_NO_MATCH           :
    return "NTE_PROV_TYPE_NO_MATCH           ";
  case NTE_SIGNATURE_FILE_BAD           :
    return "NTE_SIGNATURE_FILE_BAD           ";
  case NTE_PROVIDER_DLL_FAIL            :
    return "NTE_PROVIDER_DLL_FAIL            ";
  case NTE_PROV_DLL_NOT_FOUND           :
    return "NTE_PROV_DLL_NOT_FOUND           ";
  case NTE_BAD_KEYSET_PARAM             :
    return "NTE_BAD_KEYSET_PARAM             ";
  case NTE_FAIL                         :
    return "NTE_FAIL                         ";
  case NTE_SYS_ERR                      :
    return "NTE_SYS_ERR                      ";
  case CRYPT_E_MSG_ERROR                :
    return "CRYPT_E_MSG_ERROR                ";
  case CRYPT_E_UNKNOWN_ALGO             :
    return "CRYPT_E_UNKNOWN_ALGO             ";
  case CRYPT_E_OID_FORMAT               :
    return "CRYPT_E_OID_FORMAT               ";
  case CRYPT_E_INVALID_MSG_TYPE         :
    return "CRYPT_E_INVALID_MSG_TYPE         ";
  case CRYPT_E_UNEXPECTED_ENCODING      :
    return "CRYPT_E_UNEXPECTED_ENCODING      ";
  case CRYPT_E_AUTH_ATTR_MISSING        :
    return "CRYPT_E_AUTH_ATTR_MISSING        ";
  case CRYPT_E_HASH_VALUE               :
    return "CRYPT_E_HASH_VALUE               ";
  case CRYPT_E_INVALID_INDEX            :
    return "CRYPT_E_INVALID_INDEX            ";
  case CRYPT_E_ALREADY_DECRYPTED        :
    return "CRYPT_E_ALREADY_DECRYPTED        ";
  case CRYPT_E_NOT_DECRYPTED            :
    return "CRYPT_E_NOT_DECRYPTED            ";
  case CRYPT_E_RECIPIENT_NOT_FOUND      :
    return "CRYPT_E_RECIPIENT_NOT_FOUND      ";
  case CRYPT_E_CONTROL_TYPE             :
    return "CRYPT_E_CONTROL_TYPE             ";
  case CRYPT_E_ISSUER_SERIALNUMBER      :
    return "CRYPT_E_ISSUER_SERIALNUMBER      ";
  case CRYPT_E_SIGNER_NOT_FOUND         :
    return "CRYPT_E_SIGNER_NOT_FOUND         ";
  case CRYPT_E_ATTRIBUTES_MISSING       :
    return "CRYPT_E_ATTRIBUTES_MISSING       ";
  case CRYPT_E_STREAM_MSG_NOT_READY     :
    return "CRYPT_E_STREAM_MSG_NOT_READY     ";
  case CRYPT_E_STREAM_INSUFFICIENT_DATA :
    return "CRYPT_E_STREAM_INSUFFICIENT_DATA ";
  case CRYPT_E_BAD_LEN                  :
    return "CRYPT_E_BAD_LEN                  ";
  case CRYPT_E_BAD_ENCODE               :
    return "CRYPT_E_BAD_ENCODE               ";
  case CRYPT_E_FILE_ERROR               :
    return "CRYPT_E_FILE_ERROR               ";
  case CRYPT_E_NOT_FOUND                :
    return "CRYPT_E_NOT_FOUND                ";
  case CRYPT_E_EXISTS                   :
    return "CRYPT_E_EXISTS                   ";
  case CRYPT_E_NO_PROVIDER              :
    return "CRYPT_E_NO_PROVIDER              ";
  case CRYPT_E_SELF_SIGNED              :
    return "CRYPT_E_SELF_SIGNED              ";
  case CRYPT_E_DELETED_PREV             :
    return "CRYPT_E_DELETED_PREV             ";
  case CRYPT_E_NO_MATCH                 :
    return "CRYPT_E_NO_MATCH                 ";
  case CRYPT_E_UNEXPECTED_MSG_TYPE      :
    return "CRYPT_E_UNEXPECTED_MSG_TYPE      ";
  case CRYPT_E_NO_KEY_PROPERTY          :
    return "CRYPT_E_NO_KEY_PROPERTY          ";
  case CRYPT_E_NO_DECRYPT_CERT          :
    return "CRYPT_E_NO_DECRYPT_CERT          ";
  case CRYPT_E_BAD_MSG                  :
    return "CRYPT_E_BAD_MSG                  ";
  case CRYPT_E_NO_SIGNER                :
    return "CRYPT_E_NO_SIGNER                ";
  case CRYPT_E_PENDING_CLOSE            :
    return "CRYPT_E_PENDING_CLOSE            ";
  case CRYPT_E_REVOKED                  :
    return "CRYPT_E_REVOKED                  ";
  case CRYPT_E_NO_REVOCATION_DLL        :
    return "CRYPT_E_NO_REVOCATION_DLL        ";
  case CRYPT_E_NO_REVOCATION_CHECK      :
    return "CRYPT_E_NO_REVOCATION_CHECK      ";
  case CRYPT_E_REVOCATION_OFFLINE       :
    return "CRYPT_E_REVOCATION_OFFLINE       ";
  case CRYPT_E_NOT_IN_REVOCATION_DATABASE :
    return "CRYPT_E_NOT_IN_REVOCATION_DATABASE ";
  case CRYPT_E_INVALID_NUMERIC_STRING   :
    return "CRYPT_E_INVALID_NUMERIC_STRING   ";
  case CRYPT_E_INVALID_PRINTABLE_STRING :
    return "CRYPT_E_INVALID_PRINTABLE_STRING ";
  case CRYPT_E_INVALID_IA5_STRING       :
    return "CRYPT_E_INVALID_IA5_STRING       ";
  case CRYPT_E_INVALID_X500_STRING      :
    return "CRYPT_E_INVALID_X500_STRING      ";
  case CRYPT_E_NOT_CHAR_STRING          :
    return "CRYPT_E_NOT_CHAR_STRING          ";
  case CRYPT_E_FILERESIZED              :
    return "CRYPT_E_FILERESIZED              ";
  case CRYPT_E_SECURITY_SETTINGS        :
    return "CRYPT_E_SECURITY_SETTINGS        ";
  case CRYPT_E_NO_VERIFY_USAGE_DLL      :
    return "CRYPT_E_NO_VERIFY_USAGE_DLL      ";
  case CRYPT_E_NO_VERIFY_USAGE_CHECK    :
    return "CRYPT_E_NO_VERIFY_USAGE_CHECK    ";
  case CRYPT_E_VERIFY_USAGE_OFFLINE     :
    return "CRYPT_E_VERIFY_USAGE_OFFLINE     ";
  case CRYPT_E_NOT_IN_CTL               :
    return "CRYPT_E_NOT_IN_CTL               ";
  case CRYPT_E_NO_TRUSTED_SIGNER        :
    return "CRYPT_E_NO_TRUSTED_SIGNER        ";
  case CRYPT_E_OSS_ERROR                :
    return "CRYPT_E_OSS_ERROR                ";
  case CERTSRV_E_BAD_REQUESTSUBJECT     :
    return "CERTSRV_E_BAD_REQUESTSUBJECT     ";
  case CERTSRV_E_NO_REQUEST             :
    return "CERTSRV_E_NO_REQUEST             ";
  case CERTSRV_E_BAD_REQUESTSTATUS      :
    return "CERTSRV_E_BAD_REQUESTSTATUS      ";
  case CERTSRV_E_PROPERTY_EMPTY         :
    return "CERTSRV_E_PROPERTY_EMPTY         ";
    //case CERTDB_E_JET_ERROR               :
    //return "CERTDB_E_JET_ERROR               ";
  case TRUST_E_SYSTEM_ERROR             :
    return "TRUST_E_SYSTEM_ERROR             ";
  case TRUST_E_NO_SIGNER_CERT           :
    return "TRUST_E_NO_SIGNER_CERT           ";
  case TRUST_E_COUNTER_SIGNER           :
    return "TRUST_E_COUNTER_SIGNER           ";
  case TRUST_E_CERT_SIGNATURE           :
    return "TRUST_E_CERT_SIGNATURE           ";
  case TRUST_E_TIME_STAMP               :
    return "TRUST_E_TIME_STAMP               ";
  case TRUST_E_BAD_DIGEST               :
    return "TRUST_E_BAD_DIGEST               ";
  case TRUST_E_BASIC_CONSTRAINTS        :
    return "TRUST_E_BASIC_CONSTRAINTS        ";
  case TRUST_E_FINANCIAL_CRITERIA       :
    return "TRUST_E_FINANCIAL_CRITERIA       ";
  case TRUST_E_PROVIDER_UNKNOWN         :
    return "TRUST_E_PROVIDER_UNKNOWN         ";
  case TRUST_E_ACTION_UNKNOWN           :
    return "TRUST_E_ACTION_UNKNOWN           ";
  case TRUST_E_SUBJECT_FORM_UNKNOWN     :
    return "TRUST_E_SUBJECT_FORM_UNKNOWN     ";
  case TRUST_E_SUBJECT_NOT_TRUSTED      :
    return "TRUST_E_SUBJECT_NOT_TRUSTED      ";
  case DIGSIG_E_ENCODE                  :
    return "DIGSIG_E_ENCODE                  ";
  case DIGSIG_E_DECODE                  :
    return "DIGSIG_E_DECODE                  ";
  case DIGSIG_E_EXTENSIBILITY           :
    return "DIGSIG_E_EXTENSIBILITY           ";
  case DIGSIG_E_CRYPTO                  :
    return "DIGSIG_E_CRYPTO                  ";
  case PERSIST_E_SIZEDEFINITE           :
    return "PERSIST_E_SIZEDEFINITE           ";
  case PERSIST_E_SIZEINDEFINITE         :
    return "PERSIST_E_SIZEINDEFINITE         ";
  case PERSIST_E_NOTSELFSIZING          :
    return "PERSIST_E_NOTSELFSIZING          ";
  case TRUST_E_NOSIGNATURE              :
    return "TRUST_E_NOSIGNATURE              ";
  case CERT_E_EXPIRED                   :
    return "CERT_E_EXPIRED                   ";
  case CERT_E_VALIDITYPERIODNESTING     :
    return "CERT_E_VALIDITYPERIODNESTING     ";
  case CERT_E_ROLE                      :
    return "CERT_E_ROLE                      ";
  case CERT_E_PATHLENCONST              :
    return "CERT_E_PATHLENCONST              ";
  case CERT_E_CRITICAL                  :
    return "CERT_E_CRITICAL                  ";
  case CERT_E_PURPOSE                   :
    return "CERT_E_PURPOSE                   ";
  case CERT_E_ISSUERCHAINING            :
    return "CERT_E_ISSUERCHAINING            ";
  case CERT_E_MALFORMED                 :
    return "CERT_E_MALFORMED                 ";
  case CERT_E_UNTRUSTEDROOT             :
    return "CERT_E_UNTRUSTEDROOT             ";
  case CERT_E_CHAINING                  :
    return "CERT_E_CHAINING                  ";
  case TRUST_E_FAIL                     :
    return "TRUST_E_FAIL                     ";
  case CERT_E_REVOKED                   :
    return "CERT_E_REVOKED                   ";
  case CERT_E_UNTRUSTEDTESTROOT         :
    return "CERT_E_UNTRUSTEDTESTROOT         ";
  case CERT_E_REVOCATION_FAILURE        :
    return "CERT_E_REVOCATION_FAILURE        ";
  case CERT_E_CN_NO_MATCH               :
    return "CERT_E_CN_NO_MATCH               ";
  case CERT_E_WRONG_USAGE               :
    return "CERT_E_WRONG_USAGE               ";
  case SPAPI_E_EXPECTED_SECTION_NAME    :
    return "SPAPI_E_EXPECTED_SECTION_NAME    ";
  case SPAPI_E_BAD_SECTION_NAME_LINE    :
    return "SPAPI_E_BAD_SECTION_NAME_LINE    ";
  case SPAPI_E_SECTION_NAME_TOO_LONG    :
    return "SPAPI_E_SECTION_NAME_TOO_LONG    ";
  case SPAPI_E_GENERAL_SYNTAX           :
    return "SPAPI_E_GENERAL_SYNTAX           ";
  case SPAPI_E_WRONG_INF_STYLE          :
    return "SPAPI_E_WRONG_INF_STYLE          ";
  case SPAPI_E_SECTION_NOT_FOUND        :
    return "SPAPI_E_SECTION_NOT_FOUND        ";
  case SPAPI_E_LINE_NOT_FOUND           :
    return "SPAPI_E_LINE_NOT_FOUND           ";
  case SPAPI_E_NO_ASSOCIATED_CLASS      :
    return "SPAPI_E_NO_ASSOCIATED_CLASS      ";
  case SPAPI_E_CLASS_MISMATCH           :
    return "SPAPI_E_CLASS_MISMATCH           ";
  case SPAPI_E_DUPLICATE_FOUND          :
    return "SPAPI_E_DUPLICATE_FOUND          ";
  case SPAPI_E_NO_DRIVER_SELECTED       :
    return "SPAPI_E_NO_DRIVER_SELECTED       ";
  case SPAPI_E_KEY_DOES_NOT_EXIST       :
    return "SPAPI_E_KEY_DOES_NOT_EXIST       ";
  case SPAPI_E_INVALID_DEVINST_NAME     :
    return "SPAPI_E_INVALID_DEVINST_NAME     ";
  case SPAPI_E_INVALID_CLASS            :
    return "SPAPI_E_INVALID_CLASS            ";
  case SPAPI_E_DEVINST_ALREADY_EXISTS   :
    return "SPAPI_E_DEVINST_ALREADY_EXISTS   ";
  case SPAPI_E_DEVINFO_NOT_REGISTERED   :
    return "SPAPI_E_DEVINFO_NOT_REGISTERED   ";
  case SPAPI_E_INVALID_REG_PROPERTY     :
    return "SPAPI_E_INVALID_REG_PROPERTY     ";
  case SPAPI_E_NO_INF                   :
    return "SPAPI_E_NO_INF                   ";
  case SPAPI_E_NO_SUCH_DEVINST          :
    return "SPAPI_E_NO_SUCH_DEVINST          ";
  case SPAPI_E_CANT_LOAD_CLASS_ICON     :
    return "SPAPI_E_CANT_LOAD_CLASS_ICON     ";
  case SPAPI_E_INVALID_CLASS_INSTALLER  :
    return "SPAPI_E_INVALID_CLASS_INSTALLER  ";
  case SPAPI_E_DI_DO_DEFAULT            :
    return "SPAPI_E_DI_DO_DEFAULT            ";
  case SPAPI_E_DI_NOFILECOPY            :
    return "SPAPI_E_DI_NOFILECOPY            ";
  case SPAPI_E_INVALID_HWPROFILE        :
    return "SPAPI_E_INVALID_HWPROFILE        ";
  case SPAPI_E_NO_DEVICE_SELECTED       :
    return "SPAPI_E_NO_DEVICE_SELECTED       ";
  case SPAPI_E_DEVINFO_LIST_LOCKED      :
    return "SPAPI_E_DEVINFO_LIST_LOCKED      ";
  case SPAPI_E_DEVINFO_DATA_LOCKED      :
    return "SPAPI_E_DEVINFO_DATA_LOCKED      ";
  case SPAPI_E_DI_BAD_PATH              :
    return "SPAPI_E_DI_BAD_PATH              ";
  case SPAPI_E_NO_CLASSINSTALL_PARAMS   :
    return "SPAPI_E_NO_CLASSINSTALL_PARAMS   ";
  case SPAPI_E_FILEQUEUE_LOCKED         :
    return "SPAPI_E_FILEQUEUE_LOCKED         ";
  case SPAPI_E_BAD_SERVICE_INSTALLSECT  :
    return "SPAPI_E_BAD_SERVICE_INSTALLSECT  ";
  case SPAPI_E_NO_CLASS_DRIVER_LIST     :
    return "SPAPI_E_NO_CLASS_DRIVER_LIST     ";
  case SPAPI_E_NO_ASSOCIATED_SERVICE    :
    return "SPAPI_E_NO_ASSOCIATED_SERVICE    ";
  case SPAPI_E_NO_DEFAULT_DEVICE_INTERFACE :
    return "SPAPI_E_NO_DEFAULT_DEVICE_INTERFACE ";
  case SPAPI_E_DEVICE_INTERFACE_ACTIVE  :
    return "SPAPI_E_DEVICE_INTERFACE_ACTIVE  ";
  case SPAPI_E_DEVICE_INTERFACE_REMOVED :
    return "SPAPI_E_DEVICE_INTERFACE_REMOVED ";
  case SPAPI_E_BAD_INTERFACE_INSTALLSECT :
    return "SPAPI_E_BAD_INTERFACE_INSTALLSECT ";
  case SPAPI_E_NO_SUCH_INTERFACE_CLASS  :
    return "SPAPI_E_NO_SUCH_INTERFACE_CLASS  ";
  case SPAPI_E_INVALID_REFERENCE_STRING :
    return "SPAPI_E_INVALID_REFERENCE_STRING ";
  case SPAPI_E_INVALID_MACHINENAME      :
    return "SPAPI_E_INVALID_MACHINENAME      ";
  case SPAPI_E_REMOTE_COMM_FAILURE      :
    return "SPAPI_E_REMOTE_COMM_FAILURE      ";
  case SPAPI_E_MACHINE_UNAVAILABLE      :
    return "SPAPI_E_MACHINE_UNAVAILABLE      ";
  case SPAPI_E_NO_CONFIGMGR_SERVICES    :
    return "SPAPI_E_NO_CONFIGMGR_SERVICES    ";
  case SPAPI_E_INVALID_PROPPAGE_PROVIDER :
    return "SPAPI_E_INVALID_PROPPAGE_PROVIDER ";
  case SPAPI_E_NO_SUCH_DEVICE_INTERFACE :
    return "SPAPI_E_NO_SUCH_DEVICE_INTERFACE ";
  case SPAPI_E_DI_POSTPROCESSING_REQUIRED :
    return "SPAPI_E_DI_POSTPROCESSING_REQUIRED ";
  case SPAPI_E_INVALID_COINSTALLER      :
    return "SPAPI_E_INVALID_COINSTALLER      ";
  case SPAPI_E_NO_COMPAT_DRIVERS        :
    return "SPAPI_E_NO_COMPAT_DRIVERS        ";
  case SPAPI_E_NO_DEVICE_ICON           :
    return "SPAPI_E_NO_DEVICE_ICON           ";
  case SPAPI_E_INVALID_INF_LOGCONFIG    :
    return "SPAPI_E_INVALID_INF_LOGCONFIG    ";
  case SPAPI_E_DI_DONT_INSTALL          :
    return "SPAPI_E_DI_DONT_INSTALL          ";
  case SPAPI_E_INVALID_FILTER_DRIVER    :
    return "SPAPI_E_INVALID_FILTER_DRIVER    ";
  case SPAPI_E_ERROR_NOT_INSTALLED      :
    return "SPAPI_E_ERROR_NOT_INSTALLED      ";

  default:
    static char buff[1000];
    sprintf(buff, "Unrecognized error value: %08X\0", error);

    return buff;
  }
}

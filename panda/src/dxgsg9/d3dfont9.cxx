//-----------------------------------------------------------------------------
// File: D3DFont.cpp
//
// Desc: Texture-based font class
//       modified from a modified version of DXSDK CD3DFont from http://www.lafaqmfc.com/directx.htm
//       Note that this is faster than ID3DXFont, which calls GDI in Draw()
//-----------------------------------------------------------------------------
#ifndef STRICT
#define STRICT
#endif

#include "dxgsg9base.h"
#include <stdio.h>
#include <tchar.h>
#include <d3dx9.h>
#include "d3dfont9.h"

//-----------------------------------------------------------------------------
// Custom vertex types for rendering text
//-----------------------------------------------------------------------------

struct FONT2DVERTEX {
    D3DXVECTOR4 p;   DWORD color;     FLOAT tu, tv;
};
struct FONT3DVERTEX {
    D3DXVECTOR3 p;   D3DXVECTOR3 n;   FLOAT tu, tv;
};

inline FONT2DVERTEX InitFont2DVertex( const D3DXVECTOR4& p, D3DCOLOR color,
                                      FLOAT tu, FLOAT tv ) {
    FONT2DVERTEX v;   v.p = p;   v.color = color;   v.tu = tu;   v.tv = tv;
    return v;
}

inline FONT3DVERTEX InitFont3DVertex( const D3DXVECTOR3& p, const D3DXVECTOR3& n,
                                      FLOAT tu, FLOAT tv ) {
    FONT3DVERTEX v;   v.p = p;   v.n = n;   v.tu = tu;   v.tv = tv;
    return v;
}

//-----------------------------------------------------------------------------
// Name: CD3DFont()
// Desc: Font class constructor
//-----------------------------------------------------------------------------
CD3DFont::CD3DFont( TCHAR* strFontName, DWORD dwHeight, DWORD dwFlags ) {
    _tcscpy( m_strFontName, strFontName );
    m_dwFontHeight         = dwHeight;
    m_dwFontFlags          = dwFlags;

    m_pd3dDevice           = NULL;
    m_pTexture             = NULL;
    m_pVB                  = NULL;

    m_pSBSavedStateBlock    = NULL;
    m_pSBDrawTextStateBlock = NULL;

    ClearBeginEndData ( ) ; 
    m_bBeginText = false ; 
}

//-----------------------------------------------------------------------------
// Name: ~CD3DFont()
// Desc: Font class destructor
//-----------------------------------------------------------------------------
CD3DFont::~CD3DFont() {
    DeleteDeviceObjects();
}

//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initializes device-dependent objects, including the vertex buffer used
//       for rendering text and the texture map which stores the font image.
//-----------------------------------------------------------------------------
HRESULT CD3DFont::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice ) {
    HRESULT hr;

    // Keep a local copy of the device
    m_pd3dDevice = pd3dDevice;

    // Establish the font and texture size
    m_fTextScale  = 1.0f; // Draw fonts into texture without scaling

    // Large fonts need larger textures 
    // We can be generous at this step, this is an estimate
    if(m_dwFontHeight > 40)
        m_dwTexWidth = m_dwTexHeight = 2048;
    else if(m_dwFontHeight > 32)
        m_dwTexWidth = m_dwTexHeight = 1024;
    else if(m_dwFontHeight > 16)
        m_dwTexWidth = m_dwTexHeight = 512;
    else
        m_dwTexWidth = m_dwTexHeight = 256;

    // Prepare to create a bitmap
    DWORD*      pBitmapBits;
    BITMAPINFO bmi;
    ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  (int)m_dwTexWidth;
    bmi.bmiHeader.biHeight      = -(int)m_dwTexHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;

    // Create a DC and a bitmap for the font
    HDC     hDC       = CreateCompatibleDC( NULL );
    HBITMAP hbmBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                                          (VOID**)&pBitmapBits, NULL, 0 );
    SetMapMode( hDC, MM_TEXT );

    // Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
    // antialiased font, but this is not guaranteed.
    INT nHeight    = -MulDiv( m_dwFontHeight, 
                              (INT)(GetDeviceCaps(hDC, LOGPIXELSY) * m_fTextScale), 72 );
    DWORD dwBold   = (m_dwFontFlags&D3DFONT_BOLD)   ? FW_BOLD : FW_NORMAL;
    DWORD dwItalic = (m_dwFontFlags&D3DFONT_ITALIC) ? TRUE    : FALSE;
    HFONT hFont    = CreateFont( nHeight, 0, 0, 0, dwBold, 

                                 FALSE ,   // dwItalic, // NO! We should not do that...
                                    // See below comment about GetTextExtentPoint32

                                 FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                 VARIABLE_PITCH, m_strFontName );
    if(NULL==hFont) {
        dxgsg9_cat.error() << "CD3DFont InitDeviceObjects(): initial CreateFont failed!  GetLastError=" << GetLastError() << endl;
        return E_FAIL;
    }

    HBITMAP hbmOld = ( HBITMAP ) SelectObject ( hDC, hbmBitmap );
    HFONT   hfOld  = ( HFONT ) SelectObject ( hDC, hFont );

    // Set text properties
    SetTextColor( hDC, RGB(255,255,255) );
    SetBkColor(   hDC, 0x00000000 );
    SetTextAlign( hDC, TA_TOP );

    // First Loop through all printable characters 
    // in order to determine the smallest necessary texture
    DWORD x = 0;
    DWORD y = 0;
    TCHAR str[2] = _T("x");
    SIZE size;
    SIZE sizes [ 127 - 32 ] ;

    TCHAR c;
    for(c=32; c<127; c++) {
        str[0] = c;
        // GetTextExtentPoint32 does not care that the font is Italic or not, it will 
        // return the same value. However, if we specify an Italic font, the output 
        // on the bitmap will use more pixels. 
        // If the font is Italic we have to output the standard character 
        // and bend our vertices.
        GetTextExtentPoint32 ( hDC, str, 1, & sizes [ c - 32 ] );
    } ; 

    static DWORD TexturesSizes [ 5 ] = { 128 , 256 , 512 , 1024 , 2048} ; 
    DWORD dwTexSize = 0 ;  
    for(DWORD iTex = 0 ; iTex < 5 ; ++ iTex) {
        // fake the tex coord loop calculation 
        x = 0 ; 
        y = 0 ; 
        for(TCHAR c=32; c<127; c++) {
            if((DWORD)( x + sizes[ c - 32 ].cx+1) > TexturesSizes [ iTex ]) {
                x  = 0;
                y += sizes[ c - 32 ].cy+1;
                // new y size 
                if((DWORD) ( y + sizes[ 0 ].cy + 1 ) >= TexturesSizes [ iTex ]) {
                    // does not fit, let's try a larger one 
                    break ; 
                }; 
            }; 
            x += sizes[ c - 32 ].cx + 2 ;
        } ; 

        if((DWORD) ( y + sizes[ 0 ].cy + 1 ) < TexturesSizes [ iTex ]) {
            // Yahoo! it fits! 
            dwTexSize = TexturesSizes [ iTex ] ; 
            break ; 
        }; 
    } ; 

    m_dwTexWidth = m_dwTexHeight = dwTexSize ;

    // Select old objects ( added ) 
    // Is this needed for compatible DCs? 
    // The old handles are not not NULL, so it is done "by the book"
    SelectObject ( hDC, hbmOld );
    SelectObject ( hDC, hfOld );

    // delete our gdi objects
    DeleteObject( hbmBitmap );
    DeleteObject( hFont );
    DeleteDC( hDC );

    // moved here to allow deletion of GDI objects 
    if(dwTexSize == 0) {
        dxgsg9_cat.error() << "CD3DFont InitDeviceObjects() error: Texture didnt fit, creation failed!\n";
        return E_FAIL;
    }

    // Re-Create new GDI stuff with the optimal size 
    // 
    // Prepare to create a bitmap
    ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  (int)m_dwTexWidth;
    bmi.bmiHeader.biHeight      = -(int)m_dwTexHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;

    // Create a DC and a bitmap for the font
    hDC       = CreateCompatibleDC( NULL );
    hbmBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                                  (VOID**)&pBitmapBits, NULL, 0 );
    SetMapMode( hDC, MM_TEXT );

    // Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
    // antialiased font, but this is not guaranteed.
    nHeight  = -MulDiv( m_dwFontHeight, 
                        (INT)(GetDeviceCaps(hDC, LOGPIXELSY) * m_fTextScale), 72 );
    dwBold   = (m_dwFontFlags&D3DFONT_BOLD)   ? FW_BOLD : FW_NORMAL;
    dwItalic = (m_dwFontFlags&D3DFONT_ITALIC) ? TRUE    : FALSE;
    hFont    = CreateFont( nHeight, 0, 0, 0, dwBold, 
                           FALSE , // was dwItalic, // see above 
                           FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                           VARIABLE_PITCH, m_strFontName );

    if(NULL==hFont) {
        dxgsg9_cat.error() << "CD3DFont InitDeviceObjects(): optimal CreateFont failed!  GetLastError=" << GetLastError() << endl;
        return E_FAIL;
    }

    hbmOld = ( HBITMAP ) SelectObject ( hDC, hbmBitmap );
    hfOld  = ( HFONT ) SelectObject ( hDC, hFont );

    // Set text properties
    SetTextColor( hDC, RGB(255,255,255) );
    SetBkColor(   hDC, 0x00000000 );
    SetTextAlign( hDC, TA_TOP );

    // If requested texture is too big, use a smaller texture and smaller font,
    // and scale up when rendering.
    D3DCAPS9 d3dCaps;
    m_pd3dDevice->GetDeviceCaps( &d3dCaps );

    if(m_dwTexWidth > d3dCaps.MaxTextureWidth) {
        m_fTextScale = (FLOAT)d3dCaps.MaxTextureWidth / (FLOAT)m_dwTexWidth;
        m_dwTexWidth = m_dwTexHeight = d3dCaps.MaxTextureWidth;
    }; 

    // Create a new texture for the font
    hr = m_pd3dDevice->CreateTexture( m_dwTexWidth, m_dwTexHeight, 1,
                                      0, D3DFMT_A4R4G4B4,
                                      D3DPOOL_MANAGED, &m_pTexture, NULL);

    if(FAILED(hr)) {
        SelectObject ( hDC, hbmOld );
        SelectObject ( hDC, hfOld );

        DeleteObject( hbmBitmap );
        DeleteObject( hFont );
        DeleteDC( hDC );

        dxgsg9_cat.error() << "CD3DFont InitDeviceObjs CreateTexture failed!" << D3DERRORSTRING(hr);
        return hr;
    }; 

    // Loop through all printable character and output them to the bitmap..
    // Meanwhile, keep track of the corresponding tex coords for each character.
    x = 0 ; 
    y = 0 ; 

    for(c=32; c<127; c++) {
        str[0] = c;
        GetTextExtentPoint32( hDC, str, 1, &size );
        if((DWORD)(x+size.cx+1) > m_dwTexWidth) {
            x  = 0;
            y += size.cy+1;
        }

        // We need one pixel on both sides 

        // plus one here for one pixel on the left
        ExtTextOut( hDC, x + 1, y, ETO_OPAQUE, NULL, str, 1, NULL );

        m_fTexCoords[c-32][0] = ( (FLOAT) x + 0.5f ) / m_dwTexWidth ;
        m_fTexCoords[c-32][1] = ( (FLOAT) y + 0.5f ) / m_dwTexHeight ;
        m_fTexCoords[c-32][2] = ( (FLOAT) x + 0.5f + size.cx ) / m_dwTexWidth;
        m_fTexCoords[c-32][3] = ( (FLOAT) y + 0.5f + size.cy ) / m_dwTexHeight;

        // plus two here because we also need one more pixel on the right side
        x += size.cx + 2 ;
    }

    // Lock the surface and write the alpha values for the set pixels
    D3DLOCKED_RECT d3dlr;
    m_pTexture->LockRect( 0, &d3dlr, 0, 0 );
    BYTE* pDstRow = (BYTE*)d3dlr.pBits;
    WORD* pDst16;
    BYTE bAlpha; // 4-bit measure of pixel intensity

    for(y=0; y < m_dwTexHeight; y++) {
        pDst16 = (WORD*)pDstRow;
        for(x=0; x < m_dwTexWidth; x++) {
            bAlpha = (BYTE)((pBitmapBits[m_dwTexWidth*y + x] & 0xff) >> 4);
            if(bAlpha > 0) {
                *pDst16++ = (bAlpha << 12) | 0x0fff;
            } else {
                *pDst16++ = 0x0000;
            }
        }
        pDstRow += d3dlr.Pitch;
    }

    // Done updating texture, so clean up used objects
    m_pTexture->UnlockRect(0);

    SelectObject ( hDC, hbmOld );
    SelectObject ( hDC, hfOld );

    DeleteObject( hbmBitmap );
    DeleteObject( hFont );
    DeleteDC( hDC );

    return RestoreDeviceObjects();
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DFont::RestoreDeviceObjects() {
    HRESULT hr;

    // Create vertex buffer for the letters
    if(FAILED( hr = m_pd3dDevice->CreateVertexBuffer(
            // can be rendered 3d 
                                                    MAX_NUM_VERTICES*sizeof(FONT3DVERTEX /*FONT2DVERTEX */ ),
                                                    D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0,
                                                    D3DPOOL_DEFAULT,  // D3DUSAGE_DYNAMIC makes D3DPOOL_MANAGED impossible
                                                    &m_pVB, NULL ) )) {
        dxgsg9_cat.error() << "CD3DFont CreateVB failed!" << D3DERRORSTRING(hr);
        return hr;
    }

    PRINT_REFCNT(dxgsg9,m_pd3dDevice);

    // Create the state blocks for rendering text
    for(UINT which=0; which<2; which++) {
        m_pd3dDevice->BeginStateBlock();
        m_pd3dDevice->SetTexture( 0, m_pTexture );

        if(D3DFONT_ZENABLE & m_dwFontFlags)
            m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
        else
            m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );

        m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
        m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
        m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
        m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
        m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );
        m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
        m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
        m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
        m_pd3dDevice->SetRenderState( D3DRS_CLIPPING,         TRUE );
        m_pd3dDevice->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE,    FALSE );
        m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
        m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
        m_pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
        m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
        m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
        m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
        m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

        if(which==0)
            m_pd3dDevice->EndStateBlock( &m_pSBSavedStateBlock );
        else
            m_pd3dDevice->EndStateBlock( &m_pSBDrawTextStateBlock );
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
HRESULT CD3DFont::InvalidateDeviceObjects() {
    HRESULT hr;

    PRINT_REFCNT(dxgsg9,m_pd3dDevice);

    if(IS_VALID_PTR(m_pd3dDevice)) {
        // undo SetStreamSource before releasing VB

        IDirect3DVertexBuffer9 *pStreamData=NULL;
        UINT StreamStride;
        hr = m_pd3dDevice->GetStreamSource(0,&pStreamData,NULL,&StreamStride);
        SAFE_RELEASE(pStreamData);  // undo GetStreamSource AddRef
        if(pStreamData==m_pVB)
            hr = m_pd3dDevice->SetStreamSource(0,NULL,0,0);
    }

    PRINT_REFCNT(dxgsg9,m_pVB);

    RELEASE(m_pVB,dxgsg9,"d3dfont VB",RELEASE_ONCE);

    PRINT_REFCNT(dxgsg9,m_pd3dDevice);

    /* not necessary in DX9
    // Delete the state blocks
    if(m_pd3dDevice) {
        assert(IS_VALID_PTR(m_pd3dDevice));
        if(m_pSBSavedStateBlock)
            m_pd3dDevice->DeleteStateBlock( m_pSBSavedStateBlock );
        if(m_pSBDrawTextStateBlock)
            m_pd3dDevice->DeleteStateBlock( m_pSBDrawTextStateBlock );
    }
    */

    PRINT_REFCNT(dxgsg9,m_pd3dDevice);

    m_pSBSavedStateBlock    = NULL;
    m_pSBDrawTextStateBlock = NULL;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
HRESULT CD3DFont::DeleteDeviceObjects() {
    PRINT_REFCNT(dxgsg9,m_pd3dDevice);

    InvalidateDeviceObjects();

    SAFE_RELEASE( m_pTexture );

    PRINT_REFCNT(dxgsg9,m_pd3dDevice);

    m_pd3dDevice = NULL;

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: GetTextExtent()
// Desc: Get the dimensions of a text string
//-----------------------------------------------------------------------------
HRESULT CD3DFont::GetTextExtent( TCHAR* strText, SIZE* pSize ) {
    if(NULL==strText || NULL==pSize)
        return E_FAIL;

    FLOAT fRowWidth  = 0.0f;
    FLOAT fRowHeight = (m_fTexCoords[0][3]-m_fTexCoords[0][1])*m_dwTexHeight;
    FLOAT fWidth     = 0.0f;
    FLOAT fHeight    = fRowHeight;

    while(*strText) {
        TCHAR c = *strText++;

        if(c == _T('\n')) {
            fRowWidth = 0.0f;
            fHeight  += fRowHeight;
        }
        if(c < _T(' '))
            continue;

        FLOAT tx1 = m_fTexCoords[c-32][0];
        FLOAT tx2 = m_fTexCoords[c-32][2];

        fRowWidth += (tx2-tx1)*m_dwTexWidth;

        if(fRowWidth > fWidth)
            fWidth = fRowWidth;
    }

    pSize->cx = (int)fWidth;
    pSize->cy = (int)fHeight;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DrawTextScaled()
// Desc: Draws scaled 2D text.  Note that x and y are in viewport coordinates
//       (ranging from -1 to +1).  fXScale and fYScale are the size fraction 
//       relative to the entire viewport.  For example, a fXScale of 0.25 is
//       1/8th of the screen width.  This allows you to output text at a fixed
//       fraction of the viewport, even if the screen or window size changes.
//-----------------------------------------------------------------------------
HRESULT CD3DFont::DrawTextScaled( FLOAT x, FLOAT y, FLOAT z,
                                  FLOAT fXScale, FLOAT fYScale, DWORD dwColor,
                                  TCHAR* strText, DWORD dwFlags ) {
    if(m_pd3dDevice == NULL)
        return E_FAIL;

    HRESULT hr ; 
    if(m_bBeginText) {
        hr = DeferedDrawTextScaled ( x, y, z, fXScale, fYScale, dwColor, strText, dwFlags ) ; 
    } else {
        BeginText ( ) ; 
        hr = DeferedDrawTextScaled ( x, y, z, fXScale, fYScale, dwColor, strText, dwFlags ) ; 
        if(! FAILED ( hr ))
            EndText ( ) ;
    } ; 

    return hr ;
}

//-----------------------------------------------------------------------------
// Name: DrawText()
// Desc: Draws 2D text
//-----------------------------------------------------------------------------
HRESULT CD3DFont::DrawText( FLOAT sx, FLOAT sy, DWORD dwColor,
                            TCHAR* strText, DWORD dwFlags ) {
    if(m_pd3dDevice == NULL)
        return E_FAIL;

    HRESULT hr ; 
    if(m_bBeginText) {
        hr = DeferedDrawText ( sx, sy, dwColor, strText, dwFlags ) ; 
    } else {
        BeginText(); 
        hr = DeferedDrawText ( sx, sy, dwColor, strText, dwFlags ) ; 
        if(! FAILED ( hr ))
            EndText ( ) ;
    } ; 

    return hr ;
}


void CD3DFont::ClearBeginEndData ( void ) {
    m_nDeferedCalls = 0 ; 
    m_TextBuffer [ 0 ] = 0 ; 
    m_pTextBuffer = & m_TextBuffer [ 0 ] ; 
} 

HRESULT CD3DFont::BeginText ( void ) {
    m_bBeginText = true ; 
    ClearBeginEndData() ; 

    return S_OK ; 
} 

HRESULT CD3DFont::DeferedDrawTextScaled
( FLOAT x, FLOAT y, FLOAT z, 
  FLOAT fXScale, FLOAT fYScale, DWORD dwColor, 
  TCHAR* strText, DWORD dwFlags ) {
    return 
    DeferedDraw ( true , x, y, z, fXScale, fYScale, dwColor, strText, dwFlags ) ; 
} 

HRESULT CD3DFont::DeferedDrawText
( FLOAT x, FLOAT y, DWORD dwColor, 
  TCHAR* strText, DWORD dwFlags ) {
    return 
    DeferedDraw ( false , x, y, 0.0f , 0.0f , 0.0f , dwColor, strText, dwFlags ) ; 
} 

HRESULT CD3DFont::DeferedDraw
( bool bScaled , 
  FLOAT x, FLOAT y, FLOAT z, 
  FLOAT fXScale, FLOAT fYScale, DWORD dwColor, 
  TCHAR* strText, DWORD dwFlags ) {
    if(m_nDeferedCalls >= MaxCalls) {
        dxgsg9_cat.error() << "CD3DFont DeferedDraw() error, MaxCalls exceeded!\n";
        return E_FAIL ;
    }

    // we need to make a deep copy of the string 
    // the user object might have fallen out of scope
    // when it will be time to render 
    int nStrLen = strlen ( strText ) + 1 ; 
    int nUsed = m_pTextBuffer - & m_pTextBuffer [ 0 ] ; 
    if(nUsed + nStrLen > TextBufferLength) {
        dxgsg9_cat.error() << "CD3DFont DeferedDraw() error, TextBufferLength exceeded!\n";
        return E_FAIL ;
    }

    strcpy ( m_pTextBuffer , strText ) ; 
    m_DTArgs [ m_nDeferedCalls ].m_strText = m_pTextBuffer ; 
    m_pTextBuffer += nStrLen ; 

    m_DTArgs [ m_nDeferedCalls ].m_bScaled = bScaled ; 
    m_DTArgs [ m_nDeferedCalls ].m_x = x ; 
    m_DTArgs [ m_nDeferedCalls ].m_y = y ; 
    m_DTArgs [ m_nDeferedCalls ].m_z = z ; 
    m_DTArgs [ m_nDeferedCalls ].m_fXScale = fXScale ; 
    m_DTArgs [ m_nDeferedCalls ].m_fYScale = fYScale ; 
    m_DTArgs [ m_nDeferedCalls ].m_dwColor = dwColor ; 
    m_DTArgs [ m_nDeferedCalls ].m_dwFlags = dwFlags ; 

    m_nDeferedCalls ++ ; 

    return S_OK ; 
} 

HRESULT CD3DFont::EndText ( void ) {
    if(m_pd3dDevice == NULL)
        return E_FAIL;
    HRESULT hr;

    assert(IS_VALID_PTR(m_pVB));

    UINT SavedStreamStride;
    IDirect3DVertexBuffer9 *pSavedStreamData=NULL;
    IDirect3DVertexShader9 *pSavedVertexShader=NULL;
    IDirect3DPixelShader9 *pSavedPixelShader=NULL;

    hr = m_pd3dDevice->GetVertexShader(&pSavedVertexShader);
    hr = m_pd3dDevice->GetPixelShader(&pSavedPixelShader);

    // Set up renderstate
    hr = m_pSBSavedStateBlock->Capture();
    hr = m_pSBDrawTextStateBlock->Apply();
    /*
    if(pSavedVertexShader!=D3DFVF_FONT2DVERTEX)
        hr = m_pd3dDevice->SetVertexShader(D3DFVF_FONT2DVERTEX);
    if(pSavedPixelShader!=NULL)
        hr = m_pd3dDevice->SetPixelShader(NULL);
    */
    hr = m_pd3dDevice->SetVertexShader(NULL);
    hr = m_pd3dDevice->SetPixelShader(NULL);

    hr = m_pd3dDevice->GetStreamSource(0,&pSavedStreamData,NULL,&SavedStreamStride);
    if(FAILED(hr)) {
        dxgsg9_cat.error() << "CD3DFont EndText GetStreamSource() failed!" << D3DERRORSTRING(hr);
        return E_FAIL;
    }

    // undo GetStreamSource AddRef
    SAFE_RELEASE(pSavedStreamData);

    if((pSavedStreamData!=m_pVB)||(SavedStreamStride!=sizeof(FONT2DVERTEX))) {
        hr = m_pd3dDevice->SetStreamSource(0,m_pVB,0,sizeof(FONT2DVERTEX));
        if(FAILED(hr)) {
            dxgsg9_cat.error() << "CD3DFont EndText initial SetStreamSource() failed!" << D3DERRORSTRING(hr);
            return E_FAIL;
        }
    }

    // Set filter states
    //
    // filter if any in our list is specified filtered 
    //
    // This functionality is different from the original D3DFont 
    // but is a significant speed increase
    //
    // User will make another batch if necessary
    //  
    bool bFiltered = false ; 
    UINT i;
    for(i = 0 ; i < m_nDeferedCalls ; ++ i) {
        DWORD   dwFlags = m_DTArgs [ i ].m_dwFlags ; 
        if(dwFlags & D3DFONT_FILTERED) {
            bFiltered = true ; 
            break ; 
        }
    } ; 
    if(bFiltered) {
        m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
        m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    }; 

    // useless if nothing is scaled but should be fast enough 
    D3DVIEWPORT9 vp;
    m_pd3dDevice->GetViewport( &vp );
    FLOAT fLineHeight = ( m_fTexCoords[0][3] - m_fTexCoords[0][1] ) * m_dwTexHeight;

    // Fill vertex buffer
    FONT2DVERTEX* pVertices;
    DWORD         dwNumTriangles = 0L;
    m_pVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );

    bool bItalic = 0 != ( m_dwFontFlags & D3DFONT_ITALIC ) ; 
    // loop on our batched sets of arguments 
    for(i = 0 ; i < m_nDeferedCalls ; ++ i) {
        bool    bScaled = m_DTArgs [ i ].m_bScaled ; 
        FLOAT   x       = m_DTArgs [ i ].m_x       ; 
        FLOAT   y       = m_DTArgs [ i ].m_y       ; 
        FLOAT   z       = m_DTArgs [ i ].m_z       ; 
        FLOAT   fXScale = m_DTArgs [ i ].m_fXScale ; 
        FLOAT   fYScale = m_DTArgs [ i ].m_fYScale ; 
        DWORD   dwColor = m_DTArgs [ i ].m_dwColor ; 
        TCHAR * strText = m_DTArgs [ i ].m_strText ; 

        if(bScaled) {

            FLOAT sx  = (x+1.0f)*vp.Width/2;
            FLOAT sy  = (y-1.0f)*vp.Height/2;
            FLOAT sz  = z;
            FLOAT rhw = 1.0f;
            FLOAT fStartX = sx;

            FLOAT fBend = 0.0f ; 
            if(bItalic)
                fBend = fYScale*vp.Height / 4.0f ;

            while(*strText) {
                TCHAR c = *strText++;

                if(c == _T('\n')) {
                    sx  = fStartX;
                    sy += fYScale*vp.Height;
                }
                if(c < _T(' '))
                    continue;

                FLOAT tx1 = m_fTexCoords[c-32][0];
                FLOAT ty1 = m_fTexCoords[c-32][1];
                FLOAT tx2 = m_fTexCoords[c-32][2];
                FLOAT ty2 = m_fTexCoords[c-32][3];

                FLOAT w = (tx2-tx1)*m_dwTexWidth;
                FLOAT h = (ty2-ty1)*m_dwTexHeight;

                w *= (fXScale*vp.Height)/fLineHeight;
                h *= (fYScale*vp.Height)/fLineHeight;

                if(c != _T(' ')) {
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+h-0.5f,sz,rhw), dwColor, tx1, ty2 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f + fBend,sy+0-0.5f,sz,rhw), dwColor, tx1, ty1 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,sz,rhw), dwColor, tx2, ty2 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f + fBend,sy+0-0.5f,sz,rhw), dwColor, tx2, ty1 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,sz,rhw), dwColor, tx2, ty2 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f + fBend,sy+0-0.5f,sz,rhw), dwColor, tx1, ty1 );
                    dwNumTriangles += 2;

                    if(dwNumTriangles*3 > (MAX_NUM_VERTICES-6)) {
                        // Unlock, render, and relock the vertex buffer
                        m_pVB->Unlock();
                        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
                        m_pVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
                        dwNumTriangles = 0L;
                    }
                }

                sx += w;
            } ; 
        } else {  // not scaled 
            FLOAT fBend = 0.0f ; 
            if(bItalic)
                fBend = fLineHeight / 4.0f ;

            // Lazy guy...
            FLOAT sx = x ; 
            FLOAT sy = y ; 

            FLOAT fStartX = sx;
            while(*strText) {
                TCHAR c = *strText++;

                if(c == _T('\n')) {
                    sx = fStartX ;
                    sy += fLineHeight ;
                }
                if(c < _T(' '))
                    continue;

                FLOAT tx1 = m_fTexCoords[c-32][0];
                FLOAT ty1 = m_fTexCoords[c-32][1];
                FLOAT tx2 = m_fTexCoords[c-32][2];
                FLOAT ty2 = m_fTexCoords[c-32][3];

                FLOAT w = (tx2-tx1) *  m_dwTexWidth / m_fTextScale;
                FLOAT h = (ty2-ty1) * m_dwTexHeight / m_fTextScale;

                if(c != _T(' ')) {
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+h-0.5f,0.9f,1.0f), dwColor, tx1, ty2 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f + fBend,sy+0-0.5f,0.9f,1.0f), dwColor, tx1, ty1 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,0.9f,1.0f), dwColor, tx2, ty2 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f + fBend,sy+0-0.5f,0.9f,1.0f), dwColor, tx2, ty1 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,0.9f,1.0f), dwColor, tx2, ty2 );
                    *pVertices++ = InitFont2DVertex( D3DXVECTOR4(sx+0-0.5f + fBend,sy+0-0.5f,0.9f,1.0f), dwColor, tx1, ty1 );
                    dwNumTriangles += 2;

                    if(dwNumTriangles*3 > (MAX_NUM_VERTICES-6)) {
                        // Unlock, render, and relock the vertex buffer
                        m_pVB->Unlock();
                        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
                        pVertices = NULL;
                        m_pVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
                        dwNumTriangles = 0L;
                    }
                };  // endif not blank 

                sx += w;
            } ; // end while 

        } ; // end if else scaled 

    } ; // end for 

    m_bBeginText = false ; 
    ClearBeginEndData ( ) ; 

    // Unlock and render the vertex buffer
    m_pVB->Unlock();
    if(dwNumTriangles > 0)
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );

    // Restore the modified renderstates
    m_pSBSavedStateBlock->Apply();
    /*
    if((hSavedVertexShader!=NULL) && (hSavedVertexShader!=D3DFVF_FONT2DVERTEX))
        m_pd3dDevice->SetVertexShader(hSavedVertexShader);
    if(hSavedPixelShader!=NULL)
        m_pd3dDevice->SetPixelShader(hSavedPixelShader);
    */
    m_pd3dDevice->SetVertexShader(pSavedVertexShader);
    m_pd3dDevice->SetPixelShader(pSavedPixelShader);

    if(IS_VALID_PTR(pSavedStreamData) && ((pSavedStreamData!=m_pVB)||(SavedStreamStride!=sizeof(FONT2DVERTEX)))) {
        hr = m_pd3dDevice->SetStreamSource(0,pSavedStreamData,0,SavedStreamStride);
        if(FAILED(hr)) {
            dxgsg9_cat.error() << "CD3DFont EndText restore SetStreamSource() failed!" << D3DERRORSTRING(hr);
            return E_FAIL;
        }
        pSavedStreamData->Release();
    }

    return S_OK;
} 

#if 0
// dont need this now
//-----------------------------------------------------------------------------
// Name: Render3DText()
// Desc: Renders 3D text
//-----------------------------------------------------------------------------
HRESULT CD3DFont::Render3DText( TCHAR* strText, DWORD dwFlags ) {
    if(m_pd3dDevice == NULL)
        return E_FAIL;

    // Setup renderstate
    m_pd3dDevice->CaptureStateBlock( m_pSBSavedStateBlock );
    m_pd3dDevice->ApplyStateBlock( m_pSBDrawTextStateBlock );
    m_pd3dDevice->SetVertexShader( D3DFVF_FONT3DVERTEX );
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(FONT3DVERTEX) );

    // Set filter states
    if(dwFlags & D3DFONT_FILTERED) {
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    }

    // Position for each text element
    FLOAT x = 0.0f;
    FLOAT y = 0.0f;

    // Center the text block at the origin
    if(dwFlags & D3DFONT_CENTERED) {
        SIZE sz;
        GetTextExtent( strText, &sz );
        x = -(((FLOAT)sz.cx)/10.0f)/2.0f;
        y = -(((FLOAT)sz.cy)/10.0f)/2.0f;
    }

    // Turn off culling for two-sided text
    if(dwFlags & D3DFONT_TWOSIDED)
        m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    FLOAT fStartX = x;
    TCHAR c;

    // Fill vertex buffer
    FONT3DVERTEX* pVertices;
    // DWORD         dwVertex       = 0L; // not ref'ed
    DWORD         dwNumTriangles = 0L;
    m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );

    bool bItalic = 0 != ( m_dwFontFlags & D3DFONT_ITALIC ) ; 
    FLOAT fBend = 0.0f ; 
    if(bItalic)
        fBend = ( ( m_fTexCoords[0][3]-m_fTexCoords[0][1])*m_dwTexHeight/10.0f ) / 4.0f ;

    while(c = *strText++) {
        if(c == '\n') {
            x = fStartX;
            y -= (m_fTexCoords[0][3]-m_fTexCoords[0][1])*m_dwTexHeight/10.0f;
        }
        if(c < 32)
            continue;

        FLOAT tx1 = m_fTexCoords[c-32][0];
        FLOAT ty1 = m_fTexCoords[c-32][1];
        FLOAT tx2 = m_fTexCoords[c-32][2];
        FLOAT ty2 = m_fTexCoords[c-32][3];

        FLOAT w = (tx2-tx1) * m_dwTexWidth  / ( 10.0f * m_fTextScale );
        FLOAT h = (ty2-ty1) * m_dwTexHeight / ( 10.0f * m_fTextScale );

        if(c != _T(' ')) {
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+0,y+0,0), D3DXVECTOR3(0,0,-1), tx1, ty2 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+0 + fBend ,y+h,0), D3DXVECTOR3(0,0,-1), tx1, ty1 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+w,y+0,0), D3DXVECTOR3(0,0,-1), tx2, ty2 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+w + fBend ,y+h,0), D3DXVECTOR3(0,0,-1), tx2, ty1 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+w,y+0,0), D3DXVECTOR3(0,0,-1), tx2, ty2 );
            *pVertices++ = InitFont3DVertex( D3DXVECTOR3(x+0 + fBend ,y+h,0), D3DXVECTOR3(0,0,-1), tx1, ty1 );
            dwNumTriangles += 2;

            if(dwNumTriangles*3 > (MAX_NUM_VERTICES-6)) {
                // Unlock, render, and relock the vertex buffer
                m_pVB->Unlock();
                m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
                m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );
                dwNumTriangles = 0L;
            }
        }

        x += w;
    }

    // Unlock and render the vertex buffer
    m_pVB->Unlock();
    if(dwNumTriangles > 0)
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );

    // Restore the modified renderstates
    m_pd3dDevice->ApplyStateBlock( m_pSBSavedStateBlock );

    return S_OK;
}
#endif

//--------------------------------------------------------------------------------------------
// File: D3DFont.h
//
// Desc: Texture-based font class
//       based on a modified version of DXSDK CD3DFont from http://www.lafaqmfc.com/directx.htm
//       Note that this is faster than ID3DXFont, which calls GDI in Draw()
//---------------------------------------------------------------------------------------------
#ifndef D3DFONT_H
#define D3DFONT_H
#include <tchar.h>
#include <d3d9.h>

// Font creation flags
#define D3DFONT_BOLD        0x0001
#define D3DFONT_ITALIC      0x0002
#define D3DFONT_ZENABLE     0x0004

// Font rendering flags
#define D3DFONT_CENTERED    0x0001
#define D3DFONT_TWOSIDED    0x0002
#define D3DFONT_FILTERED    0x0004

//-----------------------------------------------------------------------------
// Name: class CD3DFont
// Desc: Texture-based font class for doing text in a 3D scene.
//-----------------------------------------------------------------------------
class CD3DFont
{
    enum 
    { 
        D3DFVF_FONT2DVERTEX = (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1) , 
        D3DFVF_FONT3DVERTEX = (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) , 
        TextBufferLength = 1024 , 
        MAX_NUM_VERTICES = TextBufferLength * 6 , 
        MaxCalls = 30 
    } ; 

    TCHAR   m_strFontName[80];            // Font properties
    DWORD   m_dwFontHeight;
    DWORD   m_dwFontFlags;

    LPDIRECT3DDEVICE9       m_pd3dDevice; // A D3DDevice used for rendering
    LPDIRECT3DTEXTURE9      m_pTexture;   // The d3d texture for this font
    LPDIRECT3DVERTEXBUFFER9 m_pVB;        // VertexBuffer for rendering text
    DWORD   m_dwTexWidth;                 // Texture dimensions
    DWORD   m_dwTexHeight;
    FLOAT   m_fTextScale;
    FLOAT   m_fTexCoords[128-32][4];

    // Stateblocks for setting and restoring render states
    IDirect3DStateBlock9* m_pSBSavedStateBlock;
    IDirect3DStateBlock9* m_pSBDrawTextStateBlock;

    struct DrawTextArgs 
    {
        bool  m_bScaled ; 
        FLOAT m_x ;  FLOAT m_y ;  FLOAT m_z ; 
        FLOAT m_fXScale ; FLOAT m_fYScale ; 
        DWORD m_dwColor ;  
        TCHAR *m_strText ; 
        DWORD m_dwFlags ; 
    } ; 

    DrawTextArgs m_DTArgs [ MaxCalls ] ; 
    char    m_TextBuffer  [ TextBufferLength ] ; 
    char   *m_pTextBuffer ; 
    UINT    m_nDeferedCalls ;
    bool    m_bBeginText ; 
    inline  HRESULT DeferedDrawText( FLOAT x, FLOAT y, DWORD dwColor, 
                      TCHAR* strText, DWORD dwFlags=0L );
    inline  HRESULT DeferedDrawTextScaled ( FLOAT x, FLOAT y, FLOAT z, 
                            FLOAT fXScale, FLOAT fYScale, DWORD dwColor, 
                            TCHAR* strText, DWORD dwFlags=0L ) ; 
    inline  HRESULT DeferedDraw
            ( bool bScaled , 
              FLOAT x, FLOAT y, FLOAT z, 
              FLOAT fXScale, FLOAT fYScale, DWORD dwColor, 
              TCHAR* strText, DWORD dwFlags ) ; 

    inline  void ClearBeginEndData (void ) ; 


public:
    // 2D and 3D text drawing functions
    HRESULT BeginText ( void ) ;
    HRESULT EndText ( void ) ;

    HRESULT DrawText( FLOAT x, FLOAT y, DWORD dwColor, 
                      TCHAR* strText, DWORD dwFlags=0L );
    HRESULT DrawTextScaled ( FLOAT x, FLOAT y, FLOAT z, 
                            FLOAT fXScale, FLOAT fYScale, DWORD dwColor, 
                            TCHAR* strText, DWORD dwFlags=0L ) ; 

   // HRESULT Render3DText( TCHAR* strText, DWORD dwFlags=0L );
    
    // Function to get extent of text
    HRESULT GetTextExtent( TCHAR* strText, SIZE* pSize );

    // Initializing and destroying device-dependent objects
    HRESULT InitDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();

    // Constructor / destructor
    CD3DFont( TCHAR* strFontName, DWORD dwHeight, DWORD dwFlags=0L );
    ~CD3DFont();
};

#endif



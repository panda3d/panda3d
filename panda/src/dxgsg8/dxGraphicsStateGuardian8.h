// Filename: dxGraphicsStateGuardian.h
// Created by:  mike (02Feb99)
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

#ifndef DXGRAPHICSSTATEGUARDIAN_H
#define DXGRAPHICSSTATEGUARDIAN_H

//#define GSG_VERBOSE 1

#include "dxgsg8base.h"
#include "dxTextureContext8.h"
#include "d3dfont8.h"
#include "config_dxgsg8.h"

#include "graphicsStateGuardian.h"
#include "geomprimitives.h"
#include "texture.h"
#include "pixelBuffer.h"
#include "displayRegion.h"
#include "material.h"
#include "depthTestAttrib.h"
#include "renderModeAttrib.h"
#include "textureApplyAttrib.h"
#include "fog.h"
#include "pointerToArray.h"

class Light;

//#if defined(NOTIFY_DEBUG) || defined(DO_PSTATS)
//#ifdef _DEBUG
// is there something in DX8 to replace this?
#if 0
// This function now serves both to print a debug message to the
// console, as well as to notify PStats about the change in texture
// memory.  Thus, we compile it in if we are building with support for
// either notify debug messages or PStats; otherwise, we compile it
// out.
extern void dbgPrintVidMem(LPDIRECTDRAW7 pDD, LPDDSCAPS2 lpddsCaps,const char *pMsg);
#define PRINTVIDMEM(pDD,pCaps,pMsg) dbgPrintVidMem(pDD,pCaps,pMsg)
#else
#define PRINTVIDMEM(pDD,pCaps,pMsg)
#endif

////////////////////////////////////////////////////////////////////
//   Class : DXGraphicsStateGuardian8
// Description : A GraphicsStateGuardian specialized for rendering
//               into DX.  There should be no DX calls
//               outside of this object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGraphicsStateGuardian8 : public GraphicsStateGuardian {
  friend class wdxGraphicsWindow8;
  friend class wdxGraphicsPipe8;
  friend class wdxGraphicsWindowGroup8;
  friend class DXTextureContext8;

public:
  DXGraphicsStateGuardian8(const FrameBufferProperties &properties);
  ~DXGraphicsStateGuardian8();

  virtual void reset();

  virtual void do_clear(const RenderBuffer &buffer);

  virtual void prepare_display_region();
  virtual bool prepare_lens();

  virtual void draw_point(GeomPoint *geom, GeomContext *gc);
  virtual void draw_line(GeomLine *geom, GeomContext *gc);
  virtual void draw_linestrip(GeomLinestrip *geom, GeomContext *gc);
  void draw_linestrip_base(Geom *geom, GeomContext *gc, bool bConnectEnds);
  virtual void draw_sprite(GeomSprite *geom, GeomContext *gc);
  virtual void draw_polygon(GeomPolygon *geom, GeomContext *gc);
  virtual void draw_quad(GeomQuad *geom, GeomContext *gc);
  virtual void draw_tri(GeomTri *geom, GeomContext *gc);
  virtual void draw_tristrip(GeomTristrip *geom, GeomContext *gc);
  virtual void draw_trifan(GeomTrifan *geom, GeomContext *gc);
  virtual void draw_sphere(GeomSphere *geom, GeomContext *gc);

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void apply_texture(TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual void copy_texture(Texture *tex, const DisplayRegion *dr);
  virtual void copy_texture(Texture *tex, const DisplayRegion *dr,
                            const RenderBuffer &rb);

  virtual void texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb);
  virtual void texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb,
                const DisplayRegion *dr);

  virtual bool copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr);
  virtual bool copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                                 const RenderBuffer &rb);

  virtual void apply_material(const Material *material);
  virtual void apply_fog(Fog *fog);

  virtual void issue_transform(const TransformState *transform);
  virtual void issue_tex_matrix(const TexMatrixAttrib *attrib);
  virtual void issue_texture(const TextureAttrib *attrib);
  virtual void issue_material(const MaterialAttrib *attrib);
  virtual void issue_render_mode(const RenderModeAttrib *attrib);
  virtual void issue_rescale_normal(const RescaleNormalAttrib *attrib);
  virtual void issue_texture_apply(const TextureApplyAttrib *attrib);
  virtual void issue_alpha_test(const AlphaTestAttrib *attrib);
  virtual void issue_depth_test(const DepthTestAttrib *attrib);
  virtual void issue_depth_write(const DepthWriteAttrib *attrib);
  virtual void issue_color_write(const ColorWriteAttrib *attrib);
  virtual void issue_cull_face(const CullFaceAttrib *attrib);
  virtual void issue_fog(const FogAttrib *attrib);
  virtual void issue_depth_offset(const DepthOffsetAttrib *attrib);
  virtual void issue_tex_gen(const TexGenAttrib *attrib);

  virtual void bind_light(PointLight *light, int light_id);
  virtual void bind_light(DirectionalLight *light, int light_id);
  virtual void bind_light(Spotlight *light, int light_id);

  virtual bool begin_frame();
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame();

  virtual bool wants_texcoords(void) const;

  INLINE float compute_distance_to(const LPoint3f &point) const;
  virtual void set_color_clear_value(const Colorf& value);

public:
  // recreate_tex_callback needs these to be public
  DXScreenData *_pScrn;
  LPDIRECT3DDEVICE8 _pD3DDevice;  // same as pScrn->_pD3DDevice, cached for spd
  IDirect3DSwapChain8 *_pSwapChain;
  D3DPRESENT_PARAMETERS _PresReset;  // This is built during reset device

protected:
  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);

  virtual bool slot_new_clip_plane(int plane_id);
  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void bind_clip_plane(PlaneNode *plane, int plane_id);

  virtual void set_blend_mode(ColorWriteAttrib::Mode color_write_mode,
                              ColorBlendAttrib::Mode color_blend_mode,
                              TransparencyAttrib::Mode transparency_mode);

  void free_nondx_resources();            // free local internal buffers
  void free_d3d_device(void);
  virtual PT(SavedFrameBuffer) save_frame_buffer(const RenderBuffer &buffer,
                         CPT(DisplayRegion) dr);
  virtual void restore_frame_buffer(SavedFrameBuffer *frame_buffer);

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  INLINE void add_to_FVFBuf(void *data,  size_t bytes) ;

  void do_auto_rescale_normal();

  bool                  _bDXisReady;
  HRESULT               _last_testcooplevel_result;
  DXTextureContext8  *_pCurTexContext;

  bool              _bTransformIssued;  // decaling needs to tell when a transform has been issued
  D3DMATRIX         _SavedTransform;

  RenderBuffer::Type _cur_read_pixel_buffer;  // source for copy_pixel_buffer operation
  bool _auto_rescale_normal;

  void GenerateSphere(void *pVertexSpace,DWORD dwVertSpaceByteSize,
                    void *pIndexSpace,DWORD dwIndexSpaceByteSize,
                    D3DXVECTOR3 *pCenter, float fRadius,
                    DWORD wNumRings, DWORD wNumSections, float sx, float sy, float sz,
                    DWORD *pNumVertices,DWORD *pNumTris,DWORD fvfFlags,DWORD dwVertSize);
  HRESULT ReleaseAllDeviceObjects(void);
  HRESULT RecreateAllDeviceObjects(void);
  HRESULT DeleteAllDeviceObjects(void);

/*
  INLINE void enable_multisample_alpha_one(bool val);
  INLINE void enable_multisample_alpha_mask(bool val);
  INLINE void enable_multisample(bool val);
*/

  INLINE void enable_color_material(bool val);
  INLINE void enable_fog(bool val);
  INLINE void enable_zwritemask(bool val);
  INLINE void set_color_writemask(UINT color_writemask);
  INLINE void enable_gouraud_shading(bool val);
  INLINE void set_vertex_format(DWORD NewFvfType);

  INLINE D3DTEXTUREADDRESS get_texture_wrap_mode(Texture::WrapMode wm) const;
  INLINE D3DFOGMODE get_fog_mode_type(Fog::Mode m) const;

  INLINE void enable_primitive_clipping(bool val);
  INLINE void enable_alpha_test(bool val);
  INLINE void enable_line_smooth(bool val);
  INLINE void enable_blend(bool val);
  INLINE void enable_point_smooth(bool val);
  INLINE void enable_texturing(bool val);
  INLINE void call_dxLightModelAmbient(const Colorf& color);
  INLINE void call_dxAlphaFunc(D3DCMPFUNC func, float refval);
  INLINE void call_dxBlendFunc(D3DBLEND sfunc, D3DBLEND dfunc);
  static D3DBLEND get_blend_func(ColorBlendAttrib::Operand operand);
  INLINE void enable_dither(bool val);
  INLINE void enable_stencil_test(bool val);
  void report_texmgr_stats();
  void draw_multitri(Geom *geom, D3DPRIMITIVETYPE tri_id);

  void draw_prim_inner_loop(int nVerts, const Geom *geom, ushort perFlags);
  void draw_prim_inner_loop_coordtexonly(int nVerts, const Geom *geom);
  size_t draw_prim_setup(const Geom *geom) ;

  //   for drawing primitives
  Normalf   p_normal;  // still used to hold G_OVERALL, G_PER_PRIM values
  TexCoordf p_texcoord;
  D3DCOLOR  _curD3Dcolor;
  DWORD     _perPrim,_perVertex,_perComp;   //  these hold DrawLoopFlags bitmask values
  DWORD     _CurFVFType;
  // for storage of the flexible vertex format
  BYTE *_pCurFvfBufPtr,*_pFvfBufBasePtr;
  WORD *_index_buf;  // base of malloced array

  D3DCOLOR _scene_graph_color_D3DCOLOR;
  D3DCOLOR _d3dcolor_clear_value;
//  D3DSHADEMODE _CurShadeMode;
  bool _bGouraudShadingOn;
  UINT _color_writemask;
  bool _bDrawPrimDoSetupVertexBuffer;       // if true, draw methods just copy vertex data into pCurrentGeomContext

  // iterators for primitives
  Geom::VertexIterator vi;
  Geom::NormalIterator ni;
  Geom::TexCoordIterator ti;
  Geom::ColorIterator ci;

  // these are used for fastpaths that bypass the iterators above
  // pointers to arrays in current geom, used to traverse indexed and non-indexed arrays
  Vertexf *_coord_array,*_pCurCoord;
  ushort *_coordindex_array,*_pCurCoordIndex;

  TexCoordf *_texcoord_array,*_pCurTexCoord;
  ushort *_texcoordindex_array,*_pCurTexCoordIndex;

/*
  PTA_Normalf _norms;
  PTA_Colorf _colors;
  PTA_ushort _cindexes,_nindexes;
*/

  Colorf _lmodel_ambient;
  float _material_ambient;
  float _material_diffuse;
  float _material_specular;
  float _material_shininess;
  float _material_emission;

  typedef enum {None,
                PerVertexFog=D3DRS_FOGVERTEXMODE,
                PerPixelFog=D3DRS_FOGTABLEMODE
               } DxgsgFogType;
  DxgsgFogType _doFogType;
  bool _fog_enabled;
/*
  TODO: cache fog state
  float _fog_start,_fog_end,_fog_density,float _fog_color;
*/

  float      _alpha_func_refval;  // d3d stores UINT, panda stores this as float.  we store float
  D3DCMPFUNC _alpha_func;

  D3DBLEND _blend_source_func;
  D3DBLEND _blend_dest_func;

  bool _line_smooth_enabled;
  bool _color_material_enabled;
  bool _texturing_enabled;
  bool _clipping_enabled;
  bool _dither_enabled;
  bool _stencil_test_enabled;
  bool _blend_enabled;
  bool _depth_test_enabled;
  bool _depth_write_enabled;
  bool _alpha_test_enabled;
  DWORD _clip_plane_bits;

  RenderModeAttrib::Mode _current_fill_mode;  //poinr/wireframe/solid

  // unused right now
  //GraphicsChannel *_panda_gfx_channel;  // cache the 1 channel dx supports

  // Cur Texture State
  TextureApplyAttrib::Mode _CurTexBlendMode;
  D3DTEXTUREFILTERTYPE _CurTexMagFilter,_CurTexMinFilter,_CurTexMipFilter;
  DWORD _CurTexAnisoDegree;
  Texture::WrapMode _CurTexWrapModeU,_CurTexWrapModeV;
  LMatrix4f _current_projection_mat;
  int _projection_mat_stack_count;

  CPT(DisplayRegion) _actual_display_region;

  // Color/Alpha Matrix Transition stuff
  INLINE void transform_color(Colorf &InColor,D3DCOLOR &OutColor);

  bool _overlay_windows_supported;

#if 0
  // This is here just as a temporary hack so this file will still
  // compile.  However, it is never initialized and will certainly
  // cause the code to crash when it is referenced.  (This used to be
  // inherited from the base class, but the new design requires that a
  // GSG may be used for multiple windows, so it doesn't make sense to
  // store a single window pointer any more.)
  GraphicsWindow *_win;
#endif

public:
  static GraphicsStateGuardian*
  make_DXGraphicsStateGuardian8(const FactoryParams &params);
  void set_context(DXScreenData *pNewContextData);
  void set_render_target();

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  INLINE void SetDXReady(bool status)  { _bDXisReady = status; }
  INLINE bool GetDXReady(void)  { return _bDXisReady;}
  void DXGraphicsStateGuardian8::SetTextureBlendMode(TextureApplyAttrib::Mode TexBlendMode,bool bJustEnable);

  void  dx_cleanup(bool bRestoreDisplayMode,bool bAtExitFnCalled);
  void reset_panda_gsg(void);
  HRESULT reset_d3d_device(D3DPRESENT_PARAMETERS *pPresParams, DXScreenData **pScrn=NULL);

  #define DO_REACTIVATE_WINDOW true
  bool CheckCooperativeLevel(bool bDoReactivateWindow = false);

  void show_frame(bool bNoNewFrameDrawn = false);
  void dx_init(void);

  void support_overlay_window(bool flag);

  bool create_swap_chain (DXScreenData *pNewContextData);
  bool release_swap_chain (DXScreenData *pNewContextData);
  void copy_pres_reset(DXScreenData *pNewContextData);

private:
  static TypeHandle _type_handle;
};

HRESULT CreateDX8Cursor(LPDIRECT3DDEVICE8 pd3dDevice, HCURSOR hCursor,BOOL bAddWatermark);

#include "DXGraphicsStateGuardian8.I"
#endif


// Filename: dxGraphicsStateGuardian.h
// Created by:  mike (02Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef DXGRAPHICSSTATEGUARDIAN_H
#define DXGRAPHICSSTATEGUARDIAN_H

//#define GSG_VERBOSE

#include <pandabase.h>
#include <graphicsStateGuardian.h>
#include <geomprimitives.h>
#include <texture.h>
#include <pixelBuffer.h>
#include <displayRegion.h>
#include <material.h>
#include <textureApplyProperty.h>
#include <depthTestProperty.h>
#include <stencilProperty.h>
#include <fog.h>
#include <renderModeProperty.h>
#include <colorMatrixTransition.h>
#include <alphaTransformTransition.h>
#include <pointerToArray.h>
#include <planeNode.h>

#include "dxgsg8base.h"
#include "dxGeomNodeContext8.h"
#include "dxTextureContext8.h"
#include <vector>

class PlaneNode;
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
//   Class : DXGraphicsStateGuardian
// Description : A GraphicsStateGuardian specialized for rendering
//               into DX.  There should be no DX calls
//               outside of this object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGraphicsStateGuardian : public GraphicsStateGuardian {
  friend class wdxGraphicsWindow;
  friend class wdxGraphicsPipe;
  friend class wdxGraphicsWindowGroup;
  friend class DXTextureContext;

public:
  DXGraphicsStateGuardian(GraphicsWindow *win);
  ~DXGraphicsStateGuardian();

  virtual void reset();

  virtual void clear(const RenderBuffer &buffer);
  virtual void clear(const RenderBuffer &buffer, const DisplayRegion* region);

  virtual void prepare_display_region();

  virtual void render_frame();
  virtual void render_scene(Node *root, LensNode *projnode);
  virtual void render_subgraph(RenderTraverser *traverser,
                   Node *subgraph, LensNode *projnode,
                   const AllTransitionsWrapper &net_trans);
  virtual void render_subgraph(RenderTraverser *traverser,
                   Node *subgraph,
                   const AllTransitionsWrapper &net_trans);

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

  virtual GeomNodeContext *prepare_geom_node(GeomNode *node);
  virtual void draw_geom_node(GeomNode *node, GeomNodeContext *gnc);
  virtual void release_geom_node(GeomNodeContext *gnc);

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void apply_texture(TextureContext *tc);
  virtual void release_texture(TextureContext *tc);

  virtual void copy_texture(TextureContext *tc, const DisplayRegion *dr);
  virtual void copy_texture(TextureContext *tc, const DisplayRegion *dr,
                            const RenderBuffer &rb);
  virtual void draw_texture(TextureContext *tc, const DisplayRegion *dr);
  virtual void draw_texture(TextureContext *tc, const DisplayRegion *dr,
                            const RenderBuffer &rb);

  virtual void texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb);
  virtual void texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb,
                const DisplayRegion *dr);

  virtual void copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr);
  virtual void copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                                 const RenderBuffer &rb);
  virtual void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                 const NodeTransitions& na=NodeTransitions());
  virtual void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                                 const RenderBuffer &rb,
                 const NodeTransitions& na=NodeTransitions());

  virtual void apply_material(const Material *material);
  virtual void apply_fog(Fog *fog);

  virtual void apply_light(PointLight* light);
  virtual void apply_light(DirectionalLight* light);
  virtual void apply_light(Spotlight* light);
  virtual void apply_light(AmbientLight* light);

  virtual void issue_transform(const TransformTransition *attrib);
  virtual void issue_tex_matrix(const TexMatrixTransition *attrib);
  virtual void issue_color(const ColorTransition *attrib);
  virtual void issue_color_transform(const ColorMatrixTransition *);
  virtual void issue_alpha_transform(const AlphaTransformTransition *);
  virtual void issue_texture(const TextureTransition *attrib);
  virtual void issue_light(const LightTransition *attrib);
  virtual void issue_material(const MaterialTransition *attrib);
  virtual void issue_render_mode(const RenderModeTransition *attrib);
  virtual void issue_color_blend(const ColorBlendTransition *attrib);
  virtual void issue_texture_apply(const TextureApplyTransition *attrib);
  virtual void issue_color_mask(const ColorMaskTransition *attrib);
  virtual void issue_depth_test(const DepthTestTransition *attrib);
  virtual void issue_depth_write(const DepthWriteTransition *attrib);
  virtual void issue_tex_gen(const TexGenTransition *attrib);
  virtual void issue_cull_face(const CullFaceTransition *attrib);
  virtual void issue_stencil(const StencilTransition *attrib);
  virtual void issue_clip_plane(const ClipPlaneTransition *attrib);
  virtual void issue_transparency(const TransparencyTransition *attrib);
  virtual void issue_fog(const FogTransition *attrib);
  virtual void issue_linesmooth(const LinesmoothTransition *attrib);

  virtual bool wants_normals(void) const;
  virtual bool wants_texcoords(void) const;
  virtual bool wants_colors(void) const;

  virtual void begin_decal(GeomNode *base_geom, AllTransitionsWrapper &attrib);
  virtual void end_decal(GeomNode *base_geom);

  INLINE float compute_distance_to(const LPoint3f &point) const;
  virtual void set_color_clear_value(const Colorf& value);

public:
  // recreate_tex_callback needs these to be public
  DXScreenData scrn;

protected:
  void free_pointers();            // free local internal buffers
  void free_dxgsg_objects(void);   // free the DirectX objects we create
  virtual PT(SavedFrameBuffer) save_frame_buffer(const RenderBuffer &buffer,
                         CPT(DisplayRegion) dr);
  virtual void restore_frame_buffer(SavedFrameBuffer *frame_buffer);

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  INLINE void add_to_FVFBuf(void *data,  size_t bytes) ;

  bool                  _bDXisReady;
  HRESULT               _last_testcooplevel_result;
  bool                  _bShowFPSMeter;
//  HDC               _front_hdc;
  DXTextureContext  *_pCurTexContext;

  bool              _bTransformIssued;  // decaling needs to tell when a transform has been issued
  D3DMATRIX         _SavedTransform;   

  RenderBuffer::Type _cur_read_pixel_buffer;  // source for copy_pixel_buffer operation

  void GenerateSphere(void *pVertexSpace,DWORD dwVertSpaceByteSize,
                    void *pIndexSpace,DWORD dwIndexSpaceByteSize,
                    D3DXVECTOR3 *pCenter, float fRadius,
                    DWORD wNumRings, DWORD wNumSections, float sx, float sy, float sz,
                    DWORD *pNumVertices,DWORD *pNumTris,DWORD fvfFlags,DWORD dwVertSize);
  HRESULT ReleaseAllDeviceObjects(void);
  HRESULT RestoreAllDeviceObjects(void);
  HRESULT RecreateAllDeviceObjects(void);
  HRESULT DeleteAllDeviceObjects(void);

/*
  INLINE void enable_multisample_alpha_one(bool val);
  INLINE void enable_multisample_alpha_mask(bool val);
  INLINE void enable_multisample(bool val);
*/  

  INLINE void enable_color_material(bool val);
  INLINE void enable_clip_plane(int clip_plane_id, bool val);
  INLINE void enable_fog(bool val);
  INLINE void enable_zwritemask(bool val);
  INLINE void set_color_writemask(UINT color_writemask);
  INLINE void enable_gouraud_shading(bool val);
  INLINE void set_vertex_format(DWORD NewFvfType);

  INLINE D3DTEXTUREADDRESS get_texture_wrap_mode(Texture::WrapMode wm) const;
  INLINE D3DCMPFUNC get_depth_func_type(DepthTestProperty::Mode m) const;
  INLINE D3DFOGMODE get_fog_mode_type(Fog::Mode m) const;

  INLINE D3DCMPFUNC get_stencil_func_type(StencilProperty::Mode m) const;
  INLINE D3DSTENCILOP get_stencil_action_type(StencilProperty::Action a) const;

  INLINE void enable_primitive_clipping(bool val);
  INLINE void enable_alpha_test(bool val);
  INLINE void enable_line_smooth(bool val);
  INLINE void enable_blend(bool val);
  INLINE void enable_point_smooth(bool val);
  INLINE void enable_texturing(bool val);
  INLINE void call_dxLightModelAmbient(const Colorf& color);
  INLINE void call_dxAlphaFunc(D3DCMPFUNC func, DWORD ref);
  INLINE void call_dxBlendFunc(D3DBLEND sfunc, D3DBLEND dfunc);
  INLINE void enable_lighting(bool val);
  INLINE void enable_dither(bool val);
  INLINE void enable_stencil_test(bool val);
  bool enable_light(int light, bool val);
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

  bool  _issued_color_enabled;      // WBD ADDED
  bool  _enable_all_color;
  Colorf _issued_color;           // WBD ADDED
  D3DCOLOR _issued_color_D3DCOLOR;           // WBD ADDED
  D3DCOLOR _d3dcolor_clear_value;
//  D3DSHADEMODE _CurShadeMode;
  bool _bGouraudShadingOn;
  UINT _color_writemask;
  bool _bDrawPrimDoSetupVertexBuffer;       // if true, draw methods just copy vertex data into pCurrentGeomContext
  DXGeomNodeContext *_pCurrentGeomContext;  // used in vertex buffer setup

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
  float      _alpha_func_ref;
  D3DCMPFUNC _alpha_func;

  D3DBLEND _blend_source_func;
  D3DBLEND _blend_dest_func;

  bool _line_smooth_enabled;
  bool* _light_enabled;      // bool[_max_lights]
  bool _color_material_enabled;
  bool _lighting_enabled;
  bool _lighting_enabled_this_frame;
  bool _texturing_enabled;
  bool  _clipping_enabled;
  bool _dither_enabled;
  bool _stencil_test_enabled;
  bool* _clip_plane_enabled;      // bool[_max_clip_planes]
  bool _blend_enabled;
  bool _depth_test_enabled;
  bool _depth_write_enabled;
  bool _alpha_test_enabled;
  int _decal_level;

  RenderModeProperty::Mode _current_fill_mode;  //poinr/wireframe/solid
  GraphicsChannel *_panda_gfx_channel;  // cache the 1 channel dx supports

  // Cur Texture State
  TextureApplyProperty::Mode _CurTexBlendMode;
  Texture::FilterType _CurTexMagFilter,_CurTexMinFilter;
  DWORD _CurTexAnisoDegree;
  Texture::WrapMode _CurTexWrapModeU,_CurTexWrapModeV;

  PTA(Light*) _available_light_ids;
  int _max_lights;
  bool* _cur_light_enabled;
  int _cur_light_id;
  float _max_light_range;
  Colorf _cur_ambient_light;
  LMatrix4f _current_projection_mat;
  int _projection_mat_stack_count;

  PTA(PlaneNode*) _available_clip_plane_ids;
  int _max_clip_planes;
  bool* _cur_clip_plane_enabled;
  int _cur_clip_plane_id;

  CPT(DisplayRegion) _actual_display_region;

  // Color/Alpha Matrix Transition stuff
  INLINE void transform_color(Colorf &InColor,D3DCOLOR &OutColor);
  bool _color_transform_required;  // _color_transform_enabled || _alpha_transform_enabled
  bool _color_transform_enabled;
  bool _alpha_transform_enabled;
  LMatrix4f _current_color_mat;
  float _current_alpha_offset;
  float _current_alpha_scale;

  // vars for frames/sec meter
  DWORD _start_time;
  DWORD _start_frame_count;
  DWORD _cur_frame_count;
  float _current_fps;
  DWORD *_fpsmeter_verts;
  DWORD _fpsmeter_fvfflags;
//  LPDIRECTDRAWSURFACE7 _fpsmeter_font_surf;
  void *_fpsmeter_font_surf;
  float _fps_u_usedwidth,_fps_v_usedheight;  // fraction of fps font texture actually used
  DWORD _fps_vertexsize;   // size of verts used to render fps meter
  void  SetFPSMeterPosition(void);
  void  FillFPSMeterTexture(void);

public:
  static GraphicsStateGuardian*
  make_DXGraphicsStateGuardian(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  INLINE void SetDXReady(bool status)  { _bDXisReady = status; }
  INLINE bool GetDXReady(void)  { return _bDXisReady;}
  void DXGraphicsStateGuardian::SetTextureBlendMode(TextureApplyProperty::Mode TexBlendMode,bool bJustEnable);

  void  dx_cleanup(bool bRestoreDisplayMode,bool bAtExitFnCalled);
  void reset_panda_gsg(void);

  #define DO_REACTIVATE_WINDOW true
  bool CheckCooperativeLevel(bool bDoReactivateWindow = false);

  void  show_frame();
  void dx_init(HCURSOR hMouseCursor);
  
private:
  static TypeHandle _type_handle;
};

#include "dxGraphicsStateGuardian8.I"

#endif


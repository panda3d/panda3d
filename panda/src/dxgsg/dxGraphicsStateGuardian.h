// Filename: dxGraphicsStateGuardian.h
// Created by:  mike (02Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef DXGRAPHICSSTATEGUARDIAN_H
#define DXGRAPHICSSTATEGUARDIAN_H

//#define GSG_VERBOSE

#include <pandabase.h>

#define D3D_OVERLOADS   //  get D3DVECTOR '+' operator, etc from d3dtypes.h

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

#include <colorMatrixAttribute.h>
#include <colorMatrixTransition.h>
#include <alphaTransformAttribute.h>
#include <alphaTransformTransition.h>

#include "dxTextureContext.h"

extern char * ConvD3DErrorToString(const HRESULT &error);   // defined in wdxGraphicsPipe.cxx

// Must include windows.h before gl.h on NT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN


#include <ddraw.h>
#include <d3d.h>

#include <pointerToArray.h>

class PlaneNode;
class Light;

#ifdef GSG_VERBOSE
ostream &output_gl_enum(ostream &out, GLenum v);
INLINE ostream &operator << (ostream &out, GLenum v) {
  return output_gl_enum(out, v);
}
#endif

#define DX_DECLARE_CLEAN(type, var) \
    type var;                       \
    ZeroMemory(&var, sizeof(type));  \
    var.dwSize = sizeof(type);   

#define RELEASE(object) if (object!=NULL) {object->Release(); object = NULL;}

#if defined(NOTIFY_DEBUG) || defined(DO_PSTATS)
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
public:
  DXGraphicsStateGuardian(GraphicsWindow *win);
  ~DXGraphicsStateGuardian();

  virtual void reset();

  virtual void clear(const RenderBuffer &buffer);
  virtual void clear(const RenderBuffer &buffer, const DisplayRegion* region);

  virtual void prepare_display_region();

  virtual void render_frame(const AllAttributesWrapper &initial_state);
  virtual void render_scene(Node *root, ProjectionNode *projnode,
                const AllAttributesWrapper &initial_state);
  virtual void render_subgraph(RenderTraverser *traverser, 
                   Node *subgraph, ProjectionNode *projnode,
                   const AllAttributesWrapper &initial_state,
                   const AllTransitionsWrapper &net_trans);
  virtual void render_subgraph(RenderTraverser *traverser,
                   Node *subgraph,
                   const AllAttributesWrapper &initial_state,
                   const AllTransitionsWrapper &net_trans);

  virtual void draw_point(const GeomPoint *geom);
  virtual void draw_line(const GeomLine *geom);
  virtual void draw_linestrip(const GeomLinestrip *geom);
  virtual void draw_sprite(const GeomSprite *geom);
  virtual void draw_polygon(const GeomPolygon *geom);
  virtual void draw_quad(const GeomQuad *geom);
  virtual void draw_tri(const GeomTri *geom);
  virtual void draw_tristrip(const GeomTristrip *geom);
  virtual void draw_trifan(const GeomTrifan *geom);
  virtual void draw_sphere(const GeomSphere *geom);

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
                 const NodeAttributes& na=NodeAttributes());
  virtual void draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                                 const RenderBuffer &rb,
                 const NodeAttributes& na=NodeAttributes());

  virtual void apply_material(const Material *material);
  virtual void apply_fog(Fog *fog);

  virtual void apply_light(PointLight* light);
  virtual void apply_light(DirectionalLight* light);
  virtual void apply_light(Spotlight* light);
  virtual void apply_light(AmbientLight* light);

  virtual void issue_transform(const TransformAttribute *attrib);
  virtual void issue_tex_matrix(const TexMatrixAttribute *attrib);
  virtual void issue_color(const ColorAttribute *attrib);
  virtual void issue_color_transform(const ColorMatrixAttribute *);
  virtual void issue_alpha_transform(const AlphaTransformAttribute *);
  virtual void issue_texture(const TextureAttribute *attrib);
  virtual void issue_light(const LightAttribute *attrib);
  virtual void issue_material(const MaterialAttribute *attrib);
  virtual void issue_render_mode(const RenderModeAttribute *attrib);
  virtual void issue_color_blend(const ColorBlendAttribute *attrib);
  virtual void issue_texture_apply(const TextureApplyAttribute *attrib);
  virtual void issue_color_mask(const ColorMaskAttribute *attrib);
  virtual void issue_depth_test(const DepthTestAttribute *attrib);
  virtual void issue_depth_write(const DepthWriteAttribute *attrib);
  virtual void issue_tex_gen(const TexGenAttribute *attrib);
  virtual void issue_cull_face(const CullFaceAttribute *attrib);
  virtual void issue_stencil(const StencilAttribute *attrib);
  virtual void issue_clip_plane(const ClipPlaneAttribute *attrib);
  virtual void issue_transparency(const TransparencyAttribute *attrib);
  virtual void issue_fog(const FogAttribute *attrib);
  virtual void issue_linesmooth(const LinesmoothAttribute *attrib);

  virtual bool wants_normals(void) const;
  virtual bool wants_texcoords(void) const;
  virtual bool wants_colors(void) const;

  virtual void begin_decal(GeomNode *base_geom);
  virtual void end_decal(GeomNode *base_geom);

  INLINE float compute_distance_to(const LPoint3f &point) const;
  
  void reset_ambient();

protected:
  void free_pointers();
  virtual PT(SavedFrameBuffer) save_frame_buffer(const RenderBuffer &buffer,
                         CPT(DisplayRegion) dr);
  virtual void restore_frame_buffer(SavedFrameBuffer *frame_buffer);

  INLINE void activate();

  void set_draw_buffer(const RenderBuffer &rb);
  void set_read_buffer(const RenderBuffer &rb);

  void bind_texture(TextureContext *tc);

  // for storage of the flexible vertex format
  char *_pCurFvfBufPtr,*_pFvfBufBasePtr;   
  INLINE void add_to_FVFBuf(void *data,  size_t bytes) ;
  WORD *_index_buf;  // base of malloced array
  
  bool                  _dx_ready;
  bool                  _bIsTNLDevice;
  LPDIRECTDRAWSURFACE7  _back;
  LPDIRECTDRAWSURFACE7  _zbuf;
  LPDIRECT3D7           _d3d;
  LPDIRECT3DDEVICE7     _d3dDevice;
  LPDIRECTDRAWSURFACE7  _pri;
  LPDIRECTDRAW7         _pDD;

  RECT                _view_rect;
  RECT                clip_rect;
  HDC               _hdc;
  int               _cNumTexPixFmts;
  LPDDPIXELFORMAT   _pTexPixFmts;
  DXTextureContext  *_pCurTexContext;

  RenderBuffer::Type _cur_read_pixel_buffer;  // source for copy_pixel_buffer operation 

  D3DDEVICEDESC7    _D3DDevDesc;

  void set_clipper(RECT cliprect);
  void GenerateSphere(void *pVertexSpace,DWORD dwVertSpaceByteSize,
                    void *pIndexSpace,DWORD dwIndexSpaceByteSize,
                    D3DVECTOR *pCenter, float fRadius, 
                    DWORD wNumRings, DWORD wNumSections, float sx, float sy, float sz,
                    DWORD *pNumVertices,DWORD *pNumIndices,DWORD fvfFlags,DWORD dwVertSize);
  HRESULT DXGraphicsStateGuardian::RestoreAllVideoSurfaces(void);

  INLINE void set_pack_alignment(int alignment);
  INLINE void set_unpack_alignment(int alignment);

  INLINE void enable_multisample_alpha_one(bool val);
  INLINE void enable_multisample_alpha_mask(bool val);
  INLINE void enable_multisample(bool val);
  INLINE void enable_color_material(bool val);
  INLINE void enable_clip_plane(int clip_plane, bool val);
  INLINE void enable_fog(bool val);
  INLINE void set_shademode(D3DSHADEMODE val);

  INLINE D3DTEXTUREADDRESS get_texture_wrap_mode(Texture::WrapMode wm) const;
  INLINE D3DCMPFUNC get_depth_func_type(DepthTestProperty::Mode m) const;
  INLINE D3DFOGMODE get_fog_mode_type(Fog::Mode m) const;

  #if 0
    INLINE D3DCMPFUNC get_stencil_func_type(StencilProperty::Mode m) const;
    INLINE D3DSTENCILOP get_stencil_action_type(StencilProperty::Action a) const;
  #endif

/*  INLINE void enable_multisample_alpha_one(bool val);
  INLINE void enable_multisample_alpha_mask(bool val);
  INLINE void enable_multisample(bool val, LPDIRECT3DDEVICE7 d3dDevice);
*/ 
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

  void draw_prim_inner_loop(int nVerts, const Geom *geom, DWORD perFlags);
  size_t draw_prim_setup(const Geom *geom) ;
  void draw_multitri(const Geom *geom, D3DPRIMITIVETYPE tri_id);

#ifdef DO_PSTATS
  void report_texmgr_stats();
#endif

  //   for drawing primitives
  Colorf    p_color;
  Normalf   p_normal;
  Vertexf   p_vertex;
  TexCoordf p_texcoord;
  D3DCOLOR  p_colr;

  int       p_flags;
  short     perPrim;
  short     perVertex;
  short     perComp;

  // iterators for primitives
  Geom::VertexIterator vi; 
  Geom::NormalIterator ni;
  Geom::TexCoordIterator ti;
  Geom::ColorIterator ci;

  float  _clear_color_red, _clear_color_green, _clear_color_blue,_clear_color_alpha;
  double _clear_depth;
  int    _clear_stencil;
  float  _clear_accum_red, _clear_accum_green, _clear_accum_blue,_clear_accum_alpha;
  int _scissor_x;
  int _scissor_y;
  int _scissor_width;
  int _scissor_height;
  int _viewport_x;
  int _viewport_y;
  int _viewport_width;
  int _viewport_height;
  Colorf _lmodel_ambient;
  float _material_ambient;
  float _material_diffuse;
  float _material_specular;
  float _material_shininess;
  float _material_emission;
  float _line_width;
  float _point_size;
  bool  _depth_mask;
  int   _fog_mode;
  float _fog_start;
  float _fog_end;
  float _fog_density;
  float _fog_color;
  float _alpha_func_ref;
  D3DCMPFUNC  _alpha_func;
  D3DBLEND _blend_source_func;
  D3DBLEND _blend_dest_func;

//  int _pack_alignment;
//  int _unpack_alignment;

  bool  _issued_color_enabled;      // WBD ADDED
  D3DCOLOR _issued_color;           // WBD ADDED

  D3DSHADEMODE _CurShadeMode;

  bool _multisample_enabled;
  bool _line_smooth_enabled;
  bool _point_smooth_enabled;
  bool* _light_enabled;      // bool[_max_lights]
  bool _color_material_enabled;
  bool _lighting_enabled;
  bool _lighting_enabled_this_frame;
  bool _texturing_enabled;
  bool _dither_enabled;
  bool _stencil_test_enabled;
  bool* _clip_plane_enabled;      // bool[_max_clip_planes]
  bool _multisample_alpha_one_enabled;
  bool _multisample_alpha_mask_enabled;
  bool _blend_enabled;
  bool _depth_test_enabled;
  bool _depth_write_enabled;
  DWORD _old_colormaskval;
  bool _fog_enabled;
  bool _alpha_test_enabled;
  int _decal_level;

  // Cur Texture State
  TextureApplyProperty::Mode _CurTexBlendMode;
  Texture::FilterType _CurTexMagFilter,_CurTexMinFilter;
  DWORD _CurTexAnisoDegree;
  Texture::WrapMode _CurTexWrapModeU,_CurTexWrapModeV;

  PTA(Light*) _available_light_ids;
  int _max_lights;
  bool* _cur_light_enabled;
  int _cur_light_id;
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
  bool _color_transform_enabled;
  bool _alpha_transform_enabled;
  LMatrix4f _current_color_mat;
  float _current_alpha_offset;
  float _current_alpha_scale;

  int _pass_number;

  // vars for frames/sec meter
  DWORD _start_time;
  DWORD _start_frame_count;
  DWORD _cur_frame_count;
  float _current_fps;

public:
  static GraphicsStateGuardian*
  make_DXGraphicsStateGuardian(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  LPDIRECT3DDEVICE7 GetD3DDevice()  {  return _d3dDevice; }
  LPDIRECTDRAW7 GetDDInterface()  {  return _pDD; }
  LPDIRECTDRAWSURFACE7 GetBackBuffer()  {  return _back; }
  LPDIRECTDRAWSURFACE7 GetZBuffer()  {  return _zbuf; }
  INLINE void  Set_HDC(HDC hdc)  {  _hdc = hdc;  }
  void adjust_view_rect(int x, int y);
  INLINE void SetDXReady(bool stat)  {  _dx_ready = stat; }
  INLINE bool GetDXReady(void)  { return _dx_ready;}
  void DXGraphicsStateGuardian::SetTextureBlendMode(TextureApplyProperty::Mode TexBlendMode,bool bJustEnable);

  void  dx_cleanup();
  void  dx_setup_after_resize(RECT viewrect,HWND mwindow) ;
  void  show_frame();
  void  show_full_screen_frame();
  void  show_windowed_frame();
  void  init_dx(  LPDIRECTDRAW7     context,
          LPDIRECTDRAWSURFACE7  pri,
          LPDIRECTDRAWSURFACE7  back,
          LPDIRECTDRAWSURFACE7  zbuf,
          LPDIRECT3D7          d3d,
          LPDIRECT3DDEVICE7    d3dDevice,
          RECT  viewrect);
   friend HRESULT CALLBACK EnumTexFmtsCallback( LPDDPIXELFORMAT pddpf, VOID* param );
private:
  static TypeHandle _type_handle;
};

#include "dxGraphicsStateGuardian.I"

#endif


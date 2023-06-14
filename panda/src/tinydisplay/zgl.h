#ifndef _tgl_zgl_h_
#define _tgl_zgl_h_

#include "dtoolbase.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "zbuffer.h"
#include "zmath.h"
#include "zfeatures.h"

/* initially # of allocated GLVertexes (will grow when necessary) */
#define POLYGON_MAX_VERTEX 16

/* Max # of specular light pow buffers */
#define MAX_SPECULAR_BUFFERS 8
/* # of entries in specular buffer */
#define SPECULAR_BUFFER_SIZE 1024
/* specular buffer granularity */
#define SPECULAR_BUFFER_RESOLUTION 1024


#define MAX_MODELVIEW_STACK_DEPTH  32
#define MAX_PROJECTION_STACK_DEPTH 8
#define MAX_TEXTURE_STACK_DEPTH    8
#define MAX_NAME_STACK_DEPTH       64
#define MAX_TEXTURE_LEVELS         11
#define MAX_LIGHTS                 16

#define VERTEX_HASH_SIZE 1031

#define MAX_DISPLAY_LISTS 1024
#define OP_BUFFER_MAX_SIZE 512

#define TGL_OFFSET_FILL    0x1
#define TGL_OFFSET_LINE    0x2
#define TGL_OFFSET_POINT   0x4

typedef struct GLSpecBuf {
  int shininess_i;
  int last_used;
  PN_stdfloat buf[SPECULAR_BUFFER_SIZE+1];
  struct GLSpecBuf *next;
} GLSpecBuf;

typedef struct GLLight {
  V4 ambient;
  V4 diffuse;
  V4 specular;
  V4 position;
  V3 spot_direction;
  PN_stdfloat spot_exponent;
  PN_stdfloat spot_cutoff;
  PN_stdfloat attenuation[3];
  /* precomputed values */
  PN_stdfloat cos_spot_cutoff;
  V3 norm_spot_direction;
  V3 norm_position;
  struct GLLight *next;
} GLLight;

typedef struct GLMaterial {
  V4 emission;
  V4 ambient;
  V4 diffuse;
  V4 specular;
  PN_stdfloat shininess;

  /* computed values */
  int shininess_i;
  int do_specular;  
} GLMaterial;


typedef struct GLViewport {
  int xmin,ymin,xsize,ysize;
  V3 scale;
  V3 trans;
  int updated;
} GLViewport;

typedef struct GLScissor {
  PN_stdfloat left, right, bottom, top;
} GLScissor;

typedef union {
  int op;
  PN_stdfloat f;
  int i;
  unsigned int ui;
  void *p;
} GLParam;

typedef struct GLParamBuffer {
  GLParam ops[OP_BUFFER_MAX_SIZE];
  struct GLParamBuffer *next;
} GLParamBuffer;

typedef struct GLList {
  GLParamBuffer *first_op_buffer;
  /* TODO: extensions for an hash table or a better allocating scheme */
} GLList;

typedef struct GLVertex {
  int edge_flag;
  V3 normal;
  V4 coord;
  V2 tex_coord[MAX_TEXTURE_STAGES];
  V4 color;
  
  /* computed values */
  V4 ec;                /* eye coordinates */
  V4 pc;                /* coordinates in the normalized volume */
  int clip_code;        /* clip code */
  ZBufferPoint zp;      /* integer coordinates for the rasterization */
} GLVertex;

/* textures */

typedef struct ZTextureView {
  ZTextureLevel levels[MAX_MIPMAP_LEVELS];
} ZTextureView;

/* The combination of all mipmap levels: one complete texture. */
typedef struct GLTexture {
  ZTextureView *views;
  int num_views;
  int num_levels;
  int xsize, ysize;
  int s_max, t_max;
  V4 border_color;

  void *allocated_buffer;
  int total_bytecount;
} GLTexture;

struct GLContext;

typedef void (*gl_draw_triangle_func)(struct GLContext *c,
                                      GLVertex *p0,GLVertex *p1,GLVertex *p2);

/* display context */

typedef struct GLContext {
  /* Z buffer */
  ZBuffer *zb;

  /* lights */
  GLLight lights[MAX_LIGHTS];
  GLLight *first_light;
  V4 ambient_light_model;
  int local_light_model;
  int lighting_enabled;
  int light_model_two_side;

  /* materials */
  GLMaterial materials[2];

  /* textures */
  GLTexture *current_textures[MAX_TEXTURE_STAGES];
  int num_textures_enabled;
 
  /* matrix */
  M4 matrix_projection;
  M4 matrix_model_view;
  M4 matrix_model_view_inv;
  M4 matrix_model_projection;
  int matrix_model_projection_updated;
  int matrix_model_projection_no_w_transform; 
  int apply_texture_matrix;

  /* viewport */
  GLViewport viewport;
  GLScissor scissor;

  /* current state */
  int smooth_shade_model;
  int cull_face_enabled;
  int cull_clockwise;
  int normalize_enabled;
  PN_stdfloat normal_scale;

  gl_draw_triangle_func draw_triangle_front,draw_triangle_back;
  ZB_fillTriangleFunc zb_fill_tri;

  /* current vertex state */
  V4 current_color;
  V4 current_normal;

  /* depth test */
  int depth_test;
  int zbias;
  bool has_zrange;
  double zmin, zrange;
  
  /* specular buffer. could probably be shared between contexts, 
    but that wouldn't be 100% thread safe */
  GLSpecBuf *specbuf_first;
  int specbuf_used_counter;
  int specbuf_num_buffers;
} GLContext;

/* init.c */
void glInit(GLContext *c, ZBuffer *zbuffer);
void glClose(GLContext *c);

/* clip.c */
void gl_transform_to_viewport(GLContext *c,GLVertex *v);
void gl_draw_triangle(GLContext *c,GLVertex *p0,GLVertex *p1,GLVertex *p2);
void gl_draw_line(GLContext *c,GLVertex *p0,GLVertex *p1);
void gl_draw_point(GLContext *c,GLVertex *p0);

void gl_draw_triangle_point(GLContext *c,
                            GLVertex *p0,GLVertex *p1,GLVertex *p2);
void gl_draw_triangle_line(GLContext *c,
                           GLVertex *p0,GLVertex *p1,GLVertex *p2);
void gl_draw_triangle_fill(GLContext *c,
                           GLVertex *p0,GLVertex *p1,GLVertex *p2);

/* light.c */
void gl_enable_disable_light(GLContext *c,int light,int v);
void gl_shade_vertex(GLContext *c,GLVertex *v);

/* vertex.c */
void gl_eval_viewport(GLContext *c);
void gl_vertex_transform(GLContext * c, GLVertex * v);

/* image_util.c */
void gl_convertRGB_to_5R6G5B(unsigned short *pixmap,unsigned char *rgb,
                             int xsize,int ysize);
void gl_convertRGB_to_8A8R8G8B(unsigned int *pixmap, unsigned char *rgb,
                               int xsize, int ysize);
void gl_resizeImage(unsigned char *dest,int xsize_dest,int ysize_dest,
                    unsigned char *src,int xsize_src,int ysize_src);
void gl_resizeImageNoInterpolate(unsigned char *dest,int xsize_dest,int ysize_dest,
                                 unsigned char *src,int xsize_src,int ysize_src);

GLContext *gl_get_context(void);

void gl_fatal_error(const char *format, ...);


/* specular buffer "api" */
GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, 
                              const PN_stdfloat shininess);

/* this clip epsilon is needed to avoid some rounding errors after
   several clipping stages */

#define CLIP_EPSILON (1E-5f)

static inline int gl_clipcode(PN_stdfloat x,PN_stdfloat y,PN_stdfloat z,PN_stdfloat w1)
{
  PN_stdfloat w;

  w = w1 * (1.0f + CLIP_EPSILON);
  return (x<-w) |
    ((x>w)<<1) |
    ((y<-w)<<2) |
    ((y>w)<<3) |
    ((z<-w)<<4) | 
    ((z>w)<<5) ;
}

#endif /* _tgl_zgl_h_ */

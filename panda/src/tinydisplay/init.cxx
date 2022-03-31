#include "zgl.h"

GLContext *gl_ctx;

void glInit(GLContext *c, ZBuffer *zbuffer)
{
  GLViewport *v;
  int i;

  c->zb=zbuffer;
  
  /* viewport */
  v=&c->viewport;
  v->xmin=0;
  v->ymin=0;
  v->xsize=zbuffer->xsize;
  v->ysize=zbuffer->ysize;
  v->updated=1;

  /* lights */
  c->first_light=nullptr;
  c->ambient_light_model=gl_V4_New(0.2, 0.2, 0.2, 1.0f);
  c->local_light_model=0;
  c->lighting_enabled=0;
  c->light_model_two_side = 0;
  c->normalize_enabled = 0;
  c->normal_scale = 1.0f;

  /* default materials */
  for(i=0;i<2;i++) {
    GLMaterial *m=&c->materials[i];
    m->emission=gl_V4_New(0.0f, 0.0f, 0.0f, 1.0f);
    m->ambient=gl_V4_New(0.2, 0.2, 0.2, 1.0f);
    m->diffuse=gl_V4_New(0.8f, 0.8f, 0.8f, 1.0f);
    m->specular=gl_V4_New(0.0f, 0.0f, 0.0f, 1.0f);
    m->shininess=0;
  }

  /* default state */
  c->current_color.v[0]=1.0f;
  c->current_color.v[1]=1.0f;
  c->current_color.v[2]=1.0f;
  c->current_color.v[3]=1.0f;

  c->current_normal.v[0]=1.0f;
  c->current_normal.v[1]=0.0f;
  c->current_normal.v[2]=0.0f;
  c->current_normal.v[3]=0.0f;

  c->cull_face_enabled=0;
  
  /* specular buffer */
  c->specbuf_first = nullptr;
  c->specbuf_used_counter = 0;
  c->specbuf_num_buffers = 0;

  /* depth test */
  c->depth_test = 0;
  c->zbias = 0;
  c->has_zrange = 0;
  c->zmin = 0.0;
  c->zrange = 1.0;
}

void glClose(GLContext *c)
{
  gl_free(c);
}

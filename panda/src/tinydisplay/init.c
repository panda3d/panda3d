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
  c->first_light=NULL;
  c->ambient_light_model=gl_V4_New(0.2,0.2,0.2,1);
  c->local_light_model=0;
  c->lighting_enabled=0;
  c->light_model_two_side = 0;

  /* default materials */
  for(i=0;i<2;i++) {
    GLMaterial *m=&c->materials[i];
    m->emission=gl_V4_New(0,0,0,1);
    m->ambient=gl_V4_New(0.2,0.2,0.2,1);
    m->diffuse=gl_V4_New(0.8,0.8,0.8,1);
    m->specular=gl_V4_New(0,0,0,1);
    m->shininess=0;
  }

  /* default state */
  c->current_color.X=1.0;
  c->current_color.Y=1.0;
  c->current_color.Z=1.0;
  c->current_color.W=1.0;

  c->current_normal.X=1.0;
  c->current_normal.Y=0.0;
  c->current_normal.Z=0.0;
  c->current_normal.W=0.0;

  c->current_tex_coord.X=0;
  c->current_tex_coord.Y=0;
  c->current_tex_coord.Z=0;
  c->current_tex_coord.W=1;

  c->cull_face_enabled=0;
  
  /* specular buffer */
  c->specbuf_first = NULL;
  c->specbuf_used_counter = 0;
  c->specbuf_num_buffers = 0;

  /* depth test */
  c->depth_test = 0;
}

void glClose(GLContext *c)
{
  gl_free(c);
}

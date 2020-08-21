#include "zgl.h"
#include <string.h>

void gl_eval_viewport(GLContext * c) {
  GLViewport *v = &c->viewport;
  GLScissor *s = &c->scissor;
  
  PN_stdfloat xsize = v->xsize * (s->right - s->left);
  PN_stdfloat xmin = v->xmin + v->xsize * s->left;
  PN_stdfloat ysize = v->ysize * (s->top - s->bottom);
  PN_stdfloat ymin = v->ymin + v->ysize * (1.0f - s->top);
  PN_stdfloat zsize = (PN_stdfloat)(1 << (ZB_Z_BITS + ZB_POINT_Z_FRAC_BITS));

  v->trans.v[0] = ((xsize - 0.5f) / 2.0f) + xmin;
  v->trans.v[1] = ((ysize - 0.5f) / 2.0f) + ymin;
  v->trans.v[2] = ((zsize - 0.5f) / 2.0f) + ((1 << ZB_POINT_Z_FRAC_BITS)) / 2;
  
  v->scale.v[0] = (xsize - 0.5f) / 2.0f;
  v->scale.v[1] = -(ysize - 0.5f) / 2.0f;
  v->scale.v[2] = -((zsize - 0.5f) / 2.0f);
}

/* coords, tranformation , clip code and projection */
/* TODO : handle all cases */
void 
gl_vertex_transform(GLContext * c, GLVertex * v) {
  PN_stdfloat *m;
  V4 *n;

  if (c->lighting_enabled) {
    /* eye coordinates needed for lighting */

    m = &c->matrix_model_view.m[0][0];
    v->ec.v[0] = (v->coord.v[0] * m[0] + v->coord.v[1] * m[1] +
                  v->coord.v[2] * m[2] + m[3]);
    v->ec.v[1] = (v->coord.v[0] * m[4] + v->coord.v[1] * m[5] +
                  v->coord.v[2] * m[6] + m[7]);
    v->ec.v[2] = (v->coord.v[0] * m[8] + v->coord.v[1] * m[9] +
                  v->coord.v[2] * m[10] + m[11]);
    v->ec.v[3] = (v->coord.v[0] * m[12] + v->coord.v[1] * m[13] +
                  v->coord.v[2] * m[14] + m[15]);

    /* projection coordinates */
    m = &c->matrix_projection.m[0][0];
    v->pc.v[0] = (v->ec.v[0] * m[0] + v->ec.v[1] * m[1] +
                  v->ec.v[2] * m[2] + v->ec.v[3] * m[3]);
    v->pc.v[1] = (v->ec.v[0] * m[4] + v->ec.v[1] * m[5] +
                  v->ec.v[2] * m[6] + v->ec.v[3] * m[7]);
    v->pc.v[2] = (v->ec.v[0] * m[8] + v->ec.v[1] * m[9] +
                  v->ec.v[2] * m[10] + v->ec.v[3] * m[11]);
    v->pc.v[3] = (v->ec.v[0] * m[12] + v->ec.v[1] * m[13] +
                  v->ec.v[2] * m[14] + v->ec.v[3] * m[15]);

    m = &c->matrix_model_view_inv.m[0][0];
    n = &c->current_normal;

    v->normal.v[0] = (n->v[0] * m[0] + n->v[1] * m[1] + n->v[2] * m[2]) * c->normal_scale;
    v->normal.v[1] = (n->v[0] * m[4] + n->v[1] * m[5] + n->v[2] * m[6]) * c->normal_scale;
    v->normal.v[2] = (n->v[0] * m[8] + n->v[1] * m[9] + n->v[2] * m[10]) * c->normal_scale;

    if (c->normalize_enabled) {
      gl_V3_Norm(&v->normal);
    }
  } else {
    /* no eye coordinates needed, no normal */
    /* NOTE: W = 1 is assumed */
    m = &c->matrix_model_projection.m[0][0];

    v->pc.v[0] = (v->coord.v[0] * m[0] + v->coord.v[1] * m[1] +
                  v->coord.v[2] * m[2] + m[3]);
    v->pc.v[1] = (v->coord.v[0] * m[4] + v->coord.v[1] * m[5] +
                  v->coord.v[2] * m[6] + m[7]);
    v->pc.v[2] = (v->coord.v[0] * m[8] + v->coord.v[1] * m[9] +
                  v->coord.v[2] * m[10] + m[11]);
    if (c->matrix_model_projection_no_w_transform) {
      v->pc.v[3] = m[15];
    } else {
      v->pc.v[3] = (v->coord.v[0] * m[12] + v->coord.v[1] * m[13] +
                    v->coord.v[2] * m[14] + m[15]);
    }
  }

  v->clip_code = gl_clipcode(v->pc.v[0], v->pc.v[1], v->pc.v[2], v->pc.v[3]);
}

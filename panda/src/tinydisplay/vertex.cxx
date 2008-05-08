#include "zgl.h"
#include "string.h"

void gl_eval_viewport(GLContext * c)
{
    GLViewport *v;
    float zsize = ((long long)1 << (ZB_Z_BITS + ZB_POINT_Z_FRAC_BITS));

    v = &c->viewport;

    v->trans.X = ((v->xsize - 0.5f) / 2.0f) + v->xmin;
    v->trans.Y = ((v->ysize - 0.5f) / 2.0f) + v->ymin;
    v->trans.Z = ((zsize - 0.5f) / 2.0f);

    v->scale.X = (v->xsize - 0.5f) / 2.0f;
    v->scale.Y = -(v->ysize - 0.5f) / 2.0f;
    v->scale.Z = -((zsize - 0.5f) / 2.0f);
}

/* coords, tranformation , clip code and projection */
/* TODO : handle all cases */
void gl_vertex_transform(GLContext * c, GLVertex * v)
{
    float *m;
    V4 *n;

    if (c->lighting_enabled) {
	/* eye coordinates needed for lighting */

	m = &c->matrix_model_view.m[0][0];
	v->ec.X = (v->coord.X * m[0] + v->coord.Y * m[1] +
		   v->coord.Z * m[2] + m[3]);
	v->ec.Y = (v->coord.X * m[4] + v->coord.Y * m[5] +
		   v->coord.Z * m[6] + m[7]);
	v->ec.Z = (v->coord.X * m[8] + v->coord.Y * m[9] +
		   v->coord.Z * m[10] + m[11]);
	v->ec.W = (v->coord.X * m[12] + v->coord.Y * m[13] +
		   v->coord.Z * m[14] + m[15]);

	/* projection coordinates */
	m = &c->matrix_projection.m[0][0];
	v->pc.X = (v->ec.X * m[0] + v->ec.Y * m[1] +
		   v->ec.Z * m[2] + v->ec.W * m[3]);
	v->pc.Y = (v->ec.X * m[4] + v->ec.Y * m[5] +
		   v->ec.Z * m[6] + v->ec.W * m[7]);
	v->pc.Z = (v->ec.X * m[8] + v->ec.Y * m[9] +
		   v->ec.Z * m[10] + v->ec.W * m[11]);
	v->pc.W = (v->ec.X * m[12] + v->ec.Y * m[13] +
		   v->ec.Z * m[14] + v->ec.W * m[15]);

	m = &c->matrix_model_view_inv.m[0][0];
	n = &c->current_normal;

	v->normal.X = (n->X * m[0] + n->Y * m[1] + n->Z * m[2]);
	v->normal.Y = (n->X * m[4] + n->Y * m[5] + n->Z * m[6]);
	v->normal.Z = (n->X * m[8] + n->Y * m[9] + n->Z * m[10]);

	if (c->normalize_enabled) {
	    gl_V3_Norm(&v->normal);
	}
    } else {
	/* no eye coordinates needed, no normal */
	/* NOTE: W = 1 is assumed */
	m = &c->matrix_model_projection.m[0][0];

	v->pc.X = (v->coord.X * m[0] + v->coord.Y * m[1] +
		   v->coord.Z * m[2] + m[3]);
	v->pc.Y = (v->coord.X * m[4] + v->coord.Y * m[5] +
		   v->coord.Z * m[6] + m[7]);
	v->pc.Z = (v->coord.X * m[8] + v->coord.Y * m[9] +
		   v->coord.Z * m[10] + m[11]);
	if (c->matrix_model_projection_no_w_transform) {
	    v->pc.W = m[15];
	} else {
	    v->pc.W = (v->coord.X * m[12] + v->coord.Y * m[13] +
		       v->coord.Z * m[14] + m[15]);
	}
    }

    v->clip_code = gl_clipcode(v->pc.X, v->pc.Y, v->pc.Z, v->pc.W);
}

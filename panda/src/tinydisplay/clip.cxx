#include "zgl.h"
#include <limits.h>

/* fill triangle profile */
/* #define PROFILE */

#define CLIP_XMIN   (1<<0)
#define CLIP_XMAX   (1<<1)
#define CLIP_YMIN   (1<<2)
#define CLIP_YMAX   (1<<3)
#define CLIP_ZMIN   (1<<4)
#define CLIP_ZMAX   (1<<5)

using std::min;

void gl_transform_to_viewport(GLContext *c,GLVertex *v)
{
  PN_stdfloat winv;

  /* coordinates */
  winv = 1.0f / v->pc.v[3];
  v->zp.x= (int) ( v->pc.v[0] * winv * c->viewport.scale.v[0] 
                   + c->viewport.trans.v[0] );
  v->zp.y= (int) ( v->pc.v[1] * winv * c->viewport.scale.v[1] 
                   + c->viewport.trans.v[1] );
  v->zp.z= (int) ( v->pc.v[2] * winv * c->viewport.scale.v[2] 
                   + c->viewport.trans.v[2] );

  // Add the z-bias, if any.  Be careful not to overflow the int.
  int z = v->zp.z + (c->zbias << (ZB_POINT_Z_FRAC_BITS + 4));
  if (z < v->zp.z && c->zbias > 0) {
    v->zp.z = INT_MAX;
  } else if (z > v->zp.z && c->zbias < 0) {
    v->zp.z = -INT_MAX;
  } else {
    v->zp.z = z;
  }
  if (c->has_zrange) {
    // Rescale the Z value into the specified range.
    static const int z_range = (1 << (ZB_Z_BITS + ZB_POINT_Z_FRAC_BITS)) - 1;
    double z = 1.0 - (double)(v->zp.z) / (double)(z_range);
    z = z * c->zrange + c->zmin;
    v->zp.z = (int)((1.0 - z) * (double)(z_range)) + 1;
  }

  /* color */
  v->zp.r=min((int)(v->color.v[0] * (ZB_POINT_RED_MAX - ZB_POINT_RED_MIN))
                + ZB_POINT_RED_MIN, ZB_POINT_RED_MAX);
  v->zp.g=min((int)(v->color.v[1] * (ZB_POINT_GREEN_MAX - ZB_POINT_GREEN_MIN))
                + ZB_POINT_GREEN_MIN, ZB_POINT_GREEN_MAX);
  v->zp.b=min((int)(v->color.v[2] * (ZB_POINT_BLUE_MAX - ZB_POINT_BLUE_MIN))
                + ZB_POINT_BLUE_MIN, ZB_POINT_BLUE_MAX);
  v->zp.a=min((int)(v->color.v[3] * (ZB_POINT_ALPHA_MAX - ZB_POINT_ALPHA_MIN))
                + ZB_POINT_ALPHA_MIN, ZB_POINT_ALPHA_MAX);

  /* texture */
  if (c->num_textures_enabled >= 1) {
    static const int si = 0;
    v->zp.s = (int)(v->tex_coord[si].v[0] * c->current_textures[si]->s_max); 
    v->zp.t = (int)(v->tex_coord[si].v[1] * c->current_textures[si]->t_max);
  }
  if (c->num_textures_enabled >= 2) {
    static const int si = 1;
    v->zp.sa = (int)(v->tex_coord[si].v[0] * c->current_textures[si]->s_max); 
    v->zp.ta = (int)(v->tex_coord[si].v[1] * c->current_textures[si]->t_max);
  }
  if (c->num_textures_enabled >= 3) {
    static const int si = 2;
    v->zp.sb = (int)(v->tex_coord[si].v[0] * c->current_textures[si]->s_max); 
    v->zp.tb = (int)(v->tex_coord[si].v[1] * c->current_textures[si]->t_max);
  }
}


/* point */

void gl_draw_point(GLContext *c,GLVertex *p0)
{
  if (p0->clip_code == 0) {
    ZB_plot(c->zb,&p0->zp);
  }
}

/* line */

static inline void interpolate(GLVertex *q,GLVertex *p0,GLVertex *p1,PN_stdfloat t)
{
  q->pc.v[0]=p0->pc.v[0]+(p1->pc.v[0]-p0->pc.v[0])*t;
  q->pc.v[1]=p0->pc.v[1]+(p1->pc.v[1]-p0->pc.v[1])*t;
  q->pc.v[2]=p0->pc.v[2]+(p1->pc.v[2]-p0->pc.v[2])*t;
  q->pc.v[3]=p0->pc.v[3]+(p1->pc.v[3]-p0->pc.v[3])*t;

  q->color.v[0]=p0->color.v[0] + (p1->color.v[0]-p0->color.v[0])*t;
  q->color.v[1]=p0->color.v[1] + (p1->color.v[1]-p0->color.v[1])*t;
  q->color.v[2]=p0->color.v[2] + (p1->color.v[2]-p0->color.v[2])*t;
  q->color.v[3]=p0->color.v[3] + (p1->color.v[3]-p0->color.v[3])*t;
}

/*
 * Line Clipping 
 */

/* Line Clipping algorithm from 'Computer Graphics', Principles and
   Practice */
static inline int ClipLine1(PN_stdfloat denom,PN_stdfloat num,PN_stdfloat *tmin,PN_stdfloat *tmax)
{
  PN_stdfloat t;

  if (denom>0) {
    t=num/denom;
    if (t>*tmax) return 0;
    if (t>*tmin) *tmin=t;
  } else if (denom<0) {
    t=num/denom;
    if (t<*tmin) return 0;
    if (t<*tmax) *tmax=t;
  } else if (num>0) return 0;
  return 1;
}

void gl_draw_line(GLContext *c,GLVertex *p1,GLVertex *p2)
{
  PN_stdfloat dx,dy,dz,dw,x1,y1,z1,w1;
  PN_stdfloat tmin,tmax;
  GLVertex q1,q2;
  int cc1,cc2;
  
  cc1=p1->clip_code;
  cc2=p2->clip_code;

  if ( (cc1 | cc2) == 0) {
    if (c->depth_test)
      ZB_line_z(c->zb,&p1->zp,&p2->zp);
    else
      ZB_line(c->zb,&p1->zp,&p2->zp);
  } else if ( (cc1&cc2) != 0 ) {
    return;
  } else {
    dx=p2->pc.v[0]-p1->pc.v[0];
    dy=p2->pc.v[1]-p1->pc.v[1];
    dz=p2->pc.v[2]-p1->pc.v[2];
    dw=p2->pc.v[3]-p1->pc.v[3];
    x1=p1->pc.v[0];
    y1=p1->pc.v[1];
    z1=p1->pc.v[2];
    w1=p1->pc.v[3];
    
    tmin=0;
    tmax=1;
    if (ClipLine1(dx+dw,-x1-w1,&tmin,&tmax) &&
        ClipLine1(-dx+dw,x1-w1,&tmin,&tmax) &&
        ClipLine1(dy+dw,-y1-w1,&tmin,&tmax) &&
        ClipLine1(-dy+dw,y1-w1,&tmin,&tmax) &&
        ClipLine1(dz+dw,-z1-w1,&tmin,&tmax) && 
        ClipLine1(-dz+dw,z1-w1,&tmin,&tmax)) {

      interpolate(&q1,p1,p2,tmin);
      interpolate(&q2,p1,p2,tmax);
      gl_transform_to_viewport(c,&q1);
      gl_transform_to_viewport(c,&q2);

      if (c->depth_test)
          ZB_line_z(c->zb,&q1.zp,&q2.zp);
      else
          ZB_line(c->zb,&q1.zp,&q2.zp);
    }
  }
}


/* triangle */

/*
 * Clipping
 */

/* We clip the segment [a,b] against the 6 planes of the normal volume.
 * We compute the point 'c' of intersection and the value of the parameter 't'
 * of the intersection if x=a+t(b-a). 
 */

#define clip_func(name, sign, dir, dir1, dir2) \
static PN_stdfloat name(V4 *c, V4 *a, V4 *b) \
{\
  PN_stdfloat t,den;\
  PN_stdfloat d[4];\
  d[0] = (b->v[0] - a->v[0]);\
  d[1] = (b->v[1] - a->v[1]);\
  d[2] = (b->v[2] - a->v[2]);\
  d[3] = (b->v[3] - a->v[3]);\
  den = -(sign d[dir]) + d[3];\
  if (den == 0) t=0;\
  else t = ( sign a->v[dir] - a->v[3]) / den;\
  c->v[dir1] = a->v[dir1] + t * d[dir1];\
  c->v[dir2] = a->v[dir2] + t * d[dir2];\
  c->v[3] = a->v[3] + t * d[3];\
  c->v[dir] = sign c->v[3];\
  return t;\
}


clip_func(clip_xmin, -, 0, 1, 2)

clip_func(clip_xmax, +, 0, 1, 2)

clip_func(clip_ymin, -, 1, 0, 2)

clip_func(clip_ymax, +, 1, 0, 2)

clip_func(clip_zmin, -, 2, 0, 1)

clip_func(clip_zmax, +, 2, 0, 1)


PN_stdfloat (*clip_proc[6])(V4 *,V4 *,V4 *)=  {
    clip_xmin,clip_xmax,
    clip_ymin,clip_ymax,
    clip_zmin,clip_zmax
};

static inline void updateTmp(GLContext *c,
                 GLVertex *q,GLVertex *p0,GLVertex *p1,PN_stdfloat t)
{
  if (c->smooth_shade_model) {
    q->color.v[0]=p0->color.v[0] + (p1->color.v[0]-p0->color.v[0])*t;
    q->color.v[1]=p0->color.v[1] + (p1->color.v[1]-p0->color.v[1])*t;
    q->color.v[2]=p0->color.v[2] + (p1->color.v[2]-p0->color.v[2])*t;
    q->color.v[3]=p0->color.v[3] + (p1->color.v[3]-p0->color.v[3])*t;
  } else {
    q->color.v[0]=p0->color.v[0];
    q->color.v[1]=p0->color.v[1];
    q->color.v[2]=p0->color.v[2];
    q->color.v[3]=p0->color.v[3];
  }

  for (int si = 0; si < c->num_textures_enabled; ++si) {
    q->tex_coord[si].v[0]=p0->tex_coord[si].v[0] + (p1->tex_coord[si].v[0]-p0->tex_coord[si].v[0])*t;
    q->tex_coord[si].v[1]=p0->tex_coord[si].v[1] + (p1->tex_coord[si].v[1]-p0->tex_coord[si].v[1])*t;
  }

  q->clip_code=gl_clipcode(q->pc.v[0],q->pc.v[1],q->pc.v[2],q->pc.v[3]);
  if (q->clip_code==0) {
    gl_transform_to_viewport(c,q);
  }
}

static void gl_draw_triangle_clip(GLContext *c,
                                  GLVertex *p0,GLVertex *p1,GLVertex *p2,int clip_bit);

void gl_draw_triangle(GLContext *c,
                      GLVertex *p0,GLVertex *p1,GLVertex *p2)
{
  int co,c_and,cc[3],front;
  PN_stdfloat norm;
  
  cc[0]=p0->clip_code;
  cc[1]=p1->clip_code;
  cc[2]=p2->clip_code;
  
  co=cc[0] | cc[1] | cc[2];

  /* we handle the non clipped case here to go faster */
  if (co==0) {
    
      norm=(PN_stdfloat)(p1->zp.x-p0->zp.x)*(PN_stdfloat)(p2->zp.y-p0->zp.y)-
        (PN_stdfloat)(p2->zp.x-p0->zp.x)*(PN_stdfloat)(p1->zp.y-p0->zp.y);
      
      if (norm == 0) return;

      front = norm < 0.0;
  
      /* back face culling */
      if (c->cull_face_enabled) {
        /* most used case first */
        if (c->cull_clockwise) {
          if (front == 0) return;
          c->draw_triangle_front(c,p0,p1,p2);
        } else {
          if (front != 0) return;
          c->draw_triangle_back(c,p0,p1,p2);
        }
      } else {
        /* no culling */
        if (front) {
          c->draw_triangle_front(c,p0,p1,p2);
        } else {
          c->draw_triangle_back(c,p0,p1,p2);
        }
      }
  } else {
    c_and=cc[0] & cc[1] & cc[2];
    if (c_and==0) {
      gl_draw_triangle_clip(c,p0,p1,p2,0);
    }
  }
}

static void gl_draw_triangle_clip(GLContext *c,
                                  GLVertex *p0,GLVertex *p1,GLVertex *p2,int clip_bit)
{
  int co,c_and,co1,cc[3],edge_flag_tmp,clip_mask;
  GLVertex tmp1,tmp2,*q[3];
  PN_stdfloat tt;

  cc[0]=p0->clip_code;
  cc[1]=p1->clip_code;
  cc[2]=p2->clip_code;
  
  co=cc[0] | cc[1] | cc[2];
  if (co == 0) {
    gl_draw_triangle(c,p0,p1,p2);
  } else {
    c_and=cc[0] & cc[1] & cc[2];
    /* the triangle is completely outside */
    if (c_and!=0) return;

    /* find the next direction to clip */
    while (clip_bit < 6 && (co & (1 << clip_bit)) == 0)  {
      clip_bit++;
    }

    /* this test can be true only in case of rounding errors */
    if (clip_bit == 6) {
#if 0
      printf("Error:\n");
      printf("%f %f %f %f\n",p0->pc.v[0],p0->pc.v[1],p0->pc.v[2],p0->pc.v[3]);
      printf("%f %f %f %f\n",p1->pc.v[0],p1->pc.v[1],p1->pc.v[2],p1->pc.v[3]);
      printf("%f %f %f %f\n",p2->pc.v[0],p2->pc.v[1],p2->pc.v[2],p2->pc.v[3]);
#endif
      return;
    }
  
    clip_mask = 1 << clip_bit;
    co1=(cc[0] ^ cc[1] ^ cc[2]) & clip_mask;
    
    if (co1)  { 
      /* one point outside */

      if (cc[0] & clip_mask) { q[0]=p0; q[1]=p1; q[2]=p2; }
      else if (cc[1] & clip_mask) { q[0]=p1; q[1]=p2; q[2]=p0; }
      else { q[0]=p2; q[1]=p0; q[2]=p1; }
      
      tt=clip_proc[clip_bit](&tmp1.pc,&q[0]->pc,&q[1]->pc);
      updateTmp(c,&tmp1,q[0],q[1],tt);

      tt=clip_proc[clip_bit](&tmp2.pc,&q[0]->pc,&q[2]->pc);
      updateTmp(c,&tmp2,q[0],q[2],tt);

      tmp1.edge_flag=q[0]->edge_flag;
      edge_flag_tmp=q[2]->edge_flag;
      q[2]->edge_flag=0;
      gl_draw_triangle_clip(c,&tmp1,q[1],q[2],clip_bit+1);

      tmp2.edge_flag=1;
      tmp1.edge_flag=0;
      q[2]->edge_flag=edge_flag_tmp;
      gl_draw_triangle_clip(c,&tmp2,&tmp1,q[2],clip_bit+1);
    } else {
      /* two points outside */

      if ((cc[0] & clip_mask)==0) { q[0]=p0; q[1]=p1; q[2]=p2; }
      else if ((cc[1] & clip_mask)==0) { q[0]=p1; q[1]=p2; q[2]=p0; } 
      else { q[0]=p2; q[1]=p0; q[2]=p1; }
      
      tt=clip_proc[clip_bit](&tmp1.pc,&q[0]->pc,&q[1]->pc);
      updateTmp(c,&tmp1,q[0],q[1],tt);

      tt=clip_proc[clip_bit](&tmp2.pc,&q[0]->pc,&q[2]->pc);
      updateTmp(c,&tmp2,q[0],q[2],tt);
      
      tmp1.edge_flag=1;
      tmp2.edge_flag=q[2]->edge_flag;
      gl_draw_triangle_clip(c,q[0],&tmp1,&tmp2,clip_bit+1);
    }
  }
}


#ifdef PROFILE
int count_triangles,count_triangles_textured,count_pixels;
#endif

void gl_draw_triangle_fill(GLContext *c,
                           GLVertex *p0,GLVertex *p1,GLVertex *p2)
{
#ifdef PROFILE
  {
    int norm;
    assert(p0->zp.x >= 0 && p0->zp.x < c->zb->xsize);
    assert(p0->zp.y >= 0 && p0->zp.y < c->zb->ysize);
    assert(p1->zp.x >= 0 && p1->zp.x < c->zb->xsize);
    assert(p1->zp.y >= 0 && p1->zp.y < c->zb->ysize);
    assert(p2->zp.x >= 0 && p2->zp.x < c->zb->xsize);
    assert(p2->zp.y >= 0 && p2->zp.y < c->zb->ysize);
    
    norm=(p1->zp.x-p0->zp.x)*(p2->zp.y-p0->zp.y)-
      (p2->zp.x-p0->zp.x)*(p1->zp.y-p0->zp.y);
    count_pixels+=abs(norm)/2;
    count_triangles++;
  }
#endif
    
#ifdef PROFILE
  if (c->num_textures_enabled != 0) {
    count_triangles_textured++;
  }
#endif

  (*c->zb_fill_tri)(c->zb,&p0->zp,&p1->zp,&p2->zp);
}

/* Render a clipped triangle in line mode */  

void gl_draw_triangle_line(GLContext *c,
                           GLVertex *p0,GLVertex *p1,GLVertex *p2)
{
    if (c->depth_test) {
        if (p0->edge_flag) ZB_line_z(c->zb,&p0->zp,&p1->zp);
        if (p1->edge_flag) ZB_line_z(c->zb,&p1->zp,&p2->zp);
        if (p2->edge_flag) ZB_line_z(c->zb,&p2->zp,&p0->zp);
    } else {
        if (p0->edge_flag) ZB_line(c->zb,&p0->zp,&p1->zp);
        if (p1->edge_flag) ZB_line(c->zb,&p1->zp,&p2->zp);
        if (p2->edge_flag) ZB_line(c->zb,&p2->zp,&p0->zp);
    }
}



/* Render a clipped triangle in point mode */
void gl_draw_triangle_point(GLContext *c,
                            GLVertex *p0,GLVertex *p1,GLVertex *p2)
{
  if (p0->edge_flag) ZB_plot(c->zb,&p0->zp);
  if (p1->edge_flag) ZB_plot(c->zb,&p1->zp);
  if (p2->edge_flag) ZB_plot(c->zb,&p2->zp);
}





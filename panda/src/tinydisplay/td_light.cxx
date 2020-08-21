#include "zgl.h"
#include <math.h>

static inline PN_stdfloat clampf(PN_stdfloat a,PN_stdfloat min,PN_stdfloat max)
{
  if (a<min) return min;
  else if (a>max) return max;
  else return a;
}

/* non optimized lighting model */
void gl_shade_vertex(GLContext *c,GLVertex *v)
{
  PN_stdfloat R,G,B,A;
  GLMaterial *m;
  GLLight *l;
  V3 n,s,d;
  PN_stdfloat dist,tmp,att,dot,dot_spot,dot_spec;
  int twoside = c->light_model_two_side;

  m=&c->materials[0];

  n.v[0]=v->normal.v[0];
  n.v[1]=v->normal.v[1];
  n.v[2]=v->normal.v[2];

  R=m->emission.v[0]+m->ambient.v[0]*c->ambient_light_model.v[0];
  G=m->emission.v[1]+m->ambient.v[1]*c->ambient_light_model.v[1];
  B=m->emission.v[2]+m->ambient.v[2]*c->ambient_light_model.v[2];
  A=clampf(m->diffuse.v[3],0,1);

  for(l=c->first_light;l!=nullptr;l=l->next) {
    PN_stdfloat lR,lB,lG;
    
    /* ambient */
    lR=l->ambient.v[0] * m->ambient.v[0];
    lG=l->ambient.v[1] * m->ambient.v[1];
    lB=l->ambient.v[2] * m->ambient.v[2];

    if (l->position.v[3] == 0) {
      /* light at infinity */
      d.v[0]=l->position.v[0];
      d.v[1]=l->position.v[1];
      d.v[2]=l->position.v[2];
      att=1;
    } else {
      /* distance attenuation */
      d.v[0]=l->position.v[0]-v->ec.v[0];
      d.v[1]=l->position.v[1]-v->ec.v[1];
      d.v[2]=l->position.v[2]-v->ec.v[2];
      dist = sqrtf(d.v[0]*d.v[0]+d.v[1]*d.v[1]+d.v[2]*d.v[2]);
      if (dist>1E-3) {
        tmp=1/dist;
        d.v[0]*=tmp;
        d.v[1]*=tmp;
        d.v[2]*=tmp;
      }
      att=1.0f/(l->attenuation[0]+dist*(l->attenuation[1]+
                                        dist*l->attenuation[2]));
    }
    dot=d.v[0]*n.v[0]+d.v[1]*n.v[1]+d.v[2]*n.v[2];
    if (twoside && dot < 0) dot = -dot;
    if (dot>0) {
      /* diffuse light */
      lR+=dot * l->diffuse.v[0] * m->diffuse.v[0];
      lG+=dot * l->diffuse.v[1] * m->diffuse.v[1];
      lB+=dot * l->diffuse.v[2] * m->diffuse.v[2];

      /* spot light */
      if (l->spot_cutoff != 180) {
        dot_spot=-(d.v[0]*l->norm_spot_direction.v[0]+
                   d.v[1]*l->norm_spot_direction.v[1]+
                   d.v[2]*l->norm_spot_direction.v[2]);
        if (twoside && dot_spot < 0) dot_spot = -dot_spot;
        if (dot_spot < l->cos_spot_cutoff) {
          /* no contribution */
          continue;
        } else {
          /* TODO: optimize */
          if (l->spot_exponent > 0) {
            att=att*pow(dot_spot,l->spot_exponent);
          }
        }
      }

      /* specular light */
      
      if (c->local_light_model) {
        V3 vcoord;
        vcoord.v[0]=v->ec.v[0];
        vcoord.v[1]=v->ec.v[1];
        vcoord.v[2]=v->ec.v[2];
        gl_V3_Norm(&vcoord);
        s.v[0]=d.v[0]-vcoord.v[0];
        s.v[1]=d.v[1]-vcoord.v[0];
        s.v[2]=d.v[2]-vcoord.v[0];
      } else {
        s.v[0]=d.v[0];
        s.v[1]=d.v[1];
        s.v[2]=d.v[2]+1.0f;
      }
      dot_spec=n.v[0]*s.v[0]+n.v[1]*s.v[1]+n.v[2]*s.v[2];
      if (twoside && dot_spec < 0) dot_spec = -dot_spec;
      if (dot_spec>0) {
        GLSpecBuf *specbuf;
        int idx;
        tmp=sqrt(s.v[0]*s.v[0]+s.v[1]*s.v[1]+s.v[2]*s.v[2]);
        if (tmp > 1E-3) {
          dot_spec=dot_spec / tmp;
        }
      
        /* TODO: optimize */
        /* testing specular buffer code */
        /* dot_spec= pow(dot_spec,m->shininess);*/
        specbuf = specbuf_get_buffer(c, m->shininess_i, m->shininess);
        idx = (int)(dot_spec*SPECULAR_BUFFER_SIZE);
        if (idx > SPECULAR_BUFFER_SIZE) idx = SPECULAR_BUFFER_SIZE;
        dot_spec = specbuf->buf[idx];
        lR+=dot_spec * l->specular.v[0] * m->specular.v[0];
        lG+=dot_spec * l->specular.v[1] * m->specular.v[1];
        lB+=dot_spec * l->specular.v[2] * m->specular.v[2];
      }
    }

    R+=att * lR;
    G+=att * lG;
    B+=att * lB;
  }

  v->color.v[0]=clampf(R*v->color.v[0],0,1);
  v->color.v[1]=clampf(G*v->color.v[1],0,1);
  v->color.v[2]=clampf(B*v->color.v[2],0,1);
  v->color.v[3]=clampf(A*v->color.v[3],0,1);
}


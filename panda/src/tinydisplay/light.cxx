#include "zgl.h"
#include "msghandling.h"
#include <math.h>

static inline float clampf(float a,float min,float max)
{
  if (a<min) return min;
  else if (a>max) return max;
  else return a;
}

/* non optimized lighting model */
void gl_shade_vertex(GLContext *c,GLVertex *v)
{
  float R,G,B,A;
  GLMaterial *m;
  GLLight *l;
  V3 n,s,d;
  float dist,tmp,att,dot,dot_spot,dot_spec;
  int twoside = c->light_model_two_side;

  m=&c->materials[0];

  n.X=v->normal.X;
  n.Y=v->normal.Y;
  n.Z=v->normal.Z;

  R=m->emission.v[0]+m->ambient.v[0]*c->ambient_light_model.v[0];
  G=m->emission.v[1]+m->ambient.v[1]*c->ambient_light_model.v[1];
  B=m->emission.v[2]+m->ambient.v[2]*c->ambient_light_model.v[2];
  A=clampf(m->diffuse.v[3],0,1);

  for(l=c->first_light;l!=NULL;l=l->next) {
    float lR,lB,lG;
    
    /* ambient */
    lR=l->ambient.v[0] * m->ambient.v[0];
    lG=l->ambient.v[1] * m->ambient.v[1];
    lB=l->ambient.v[2] * m->ambient.v[2];

    if (l->position.v[3] == 0) {
      /* light at infinity */
      d.X=l->position.v[0];
      d.Y=l->position.v[1];
      d.Z=l->position.v[2];
      att=1;
    } else {
      /* distance attenuation */
      d.X=l->position.v[0]-v->ec.v[0];
      d.Y=l->position.v[1]-v->ec.v[1];
      d.Z=l->position.v[2]-v->ec.v[2];
      dist = sqrtf(d.X*d.X+d.Y*d.Y+d.Z*d.Z);
      if (dist>1E-3) {
        tmp=1/dist;
        d.X*=tmp;
        d.Y*=tmp;
        d.Z*=tmp;
      }
      att=1.0f/(l->attenuation[0]+dist*(l->attenuation[1]+
				     dist*l->attenuation[2]));
    }
    dot=d.X*n.X+d.Y*n.Y+d.Z*n.Z;
    if (twoside && dot < 0) dot = -dot;
    if (dot>0) {
      /* diffuse light */
      lR+=dot * l->diffuse.v[0] * m->diffuse.v[0];
      lG+=dot * l->diffuse.v[1] * m->diffuse.v[1];
      lB+=dot * l->diffuse.v[2] * m->diffuse.v[2];

      /* spot light */
      if (l->spot_cutoff != 180) {
        dot_spot=-(d.X*l->norm_spot_direction.v[0]+
                   d.Y*l->norm_spot_direction.v[1]+
                   d.Z*l->norm_spot_direction.v[2]);
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
        vcoord.X=v->ec.X;
        vcoord.Y=v->ec.Y;
        vcoord.Z=v->ec.Z;
        gl_V3_Norm(&vcoord);
        s.X=d.X-vcoord.X;
        s.Y=d.Y-vcoord.X;
        s.Z=d.Z-vcoord.X;
      } else {
        s.X=d.X;
        s.Y=d.Y;
        s.Z=d.Z+1.0f;
      }
      dot_spec=n.X*s.X+n.Y*s.Y+n.Z*s.Z;
      if (twoside && dot_spec < 0) dot_spec = -dot_spec;
      if (dot_spec>0) {
        GLSpecBuf *specbuf;
        int idx;
        tmp=sqrt(s.X*s.X+s.Y*s.Y+s.Z*s.Z);
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

  v->color.v[0]=clampf(R,0,1);
  v->color.v[1]=clampf(G,0,1);
  v->color.v[2]=clampf(B,0,1);
  v->color.v[3]=A;
}


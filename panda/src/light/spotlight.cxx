// Filename: spotlight.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "spotlight.h"
#include "lightTransition.h"
#include "config_light.h"

#include <renderRelation.h>
#include <perspectiveProjection.h>
#include <texture.h>
#include <geomNode.h>
#include <geomTrifan.h>
#include <depthWriteTransition.h>
#include <transparencyTransition.h>
#include <cullFaceTransition.h>
#include <textureTransition.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Spotlight::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Spotlight::Spotlight(const string& name) : ProjectionNode(name)
{
  set_exponent(1);

  set_color(Colorf(1, 1, 1, 1));
  set_specular(Colorf(1, 1, 1, 1));

  set_constant_attenuation(1);
  set_linear_attenuation(0);
  set_quadratic_attenuation(0);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Spotlight::
output(ostream &out) const {
  NamedNode::output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Spotlight::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2) << "color " << _color << "\n";
  indent(out, indent_level + 2) << "specular " << _specular << "\n";
  indent(out, indent_level + 2)
    << "attenuation " << _constant_attenuation << ", "
    << _linear_attenuation << ", " << _quadratic_attenuation << "\n";
  indent(out, indent_level + 2) << "exponent " << _exponent << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::get_cutoff_angle
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
float Spotlight::get_cutoff_angle(void) const
{
  Projection* proj = ((ProjectionNode *)this)->get_projection();
  Frustumf frustum;
  float cutoff = 0;
  if (proj->get_type() == PerspectiveProjection::get_class_type()) {
    frustum = ((PerspectiveProjection *)proj)->get_frustum();
    cutoff = rad_2_deg(atan(frustum._t / frustum._fnear));
  }
  else
    light_cat.error()
      << "Spotlight::get_cutoff_angle() - spotlight has a non "
      << "perspective projection!" << endl;
  return cutoff;
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::make_image
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool Spotlight::make_image(Texture* texture, float radius)
{
  if (texture == NULL) {
    light_cat.error()
      << "Spotlight::make_image() - NULL texture" << endl;
    return false;
  }
  PixelBuffer* pb = texture->_pbuffer;
  int size = pb->get_xsize();
  if (size == 0) {
    light_cat.error()
      << "Spotlight::make_image() - pixel buffer has size == 0" << endl;
    return false;
  }

  uchar color[3];
  color[0] = (int)(_color[0] * 255.0f);
  color[1] = (int)(_color[1] * 255.0f);
  color[2] = (int)(_color[2] * 255.0f);

  //  float cutoff = get_cutoff_angle();
  //  float dist = 1 / (float)tan(cutoff);
  //  int bufsize = size * size * 3;
  int half_width = (size - 2) / 2;
  float dXY = 1 / (float)half_width;
  float Y = dXY + dXY;
  float X, YY, dist_from_center, intensity;
  uchar C[3];
  int tx, ty, tx2, ty2;

  for (int y = 0; y < half_width; y++, Y += dXY) {
    X = dXY * y + dXY;
    YY = Y * Y;
    ty = y + half_width;

    for (int x = y; x < half_width; x++, X += dXY) {
      dist_from_center = (float)sqrt(X * X + YY);
      float D = dist_from_center;
      if (D <= radius)
        intensity = 1.0f;
          else if (D < 1.0f)
        intensity = pow(cos((D-radius) / 
                (1.0f-radius) * (MathNumbers::pi_f*0.5f)), _exponent);
      else
        intensity = 0;

      C[0] = (uchar)(intensity * color[0]);
      C[1] = (uchar)(intensity * color[1]);
      C[2] = (uchar)(intensity * color[2]);

      tx = x + half_width;

      pb->set_uchar_rgb_texel(C, tx, ty, size);
      pb->set_uchar_rgb_texel(C, tx, size - ty - 1, size);
      pb->set_uchar_rgb_texel(C, size - tx - 1, ty, size);
      pb->set_uchar_rgb_texel(C, size - tx - 1, size - ty - 1, size);

      tx2 = ty; ty2 = tx;

      pb->set_uchar_rgb_texel(C, tx2, ty2, size);
      pb->set_uchar_rgb_texel(C, tx2, size - ty2 - 1, size);
      pb->set_uchar_rgb_texel(C, size - tx2 - 1, ty2, size);
      pb->set_uchar_rgb_texel(C, size - tx2 - 1, size - ty2 - 1, size);
    }
  }
  texture->unprepare();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::make_geometry
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
NamedNode* Spotlight::
make_geometry(float intensity, float length, int num_facets)
{
  Colorf diffuse = _color;
  diffuse[3] = intensity;
  Colorf black(0.0, 0.0, 0.0, intensity);
  float radius = length * (float)tan(deg_2_rad(get_cutoff_angle()));
  float ang_inc = 2.0f*MathNumbers::pi_f / (float)num_facets;
  int num_verts = num_facets + 1;
  int num_indices = num_facets + 2;
  LVector3f offset(0.0, length, 0.0);
  LPoint3f first_last_vert(radius, length, 0.0);

  PTA_Vertexf coords(num_verts);
  PTA_ushort vindex(num_indices);
  PTA_Colorf colors(2);
  PTA_ushort cindex(num_indices);
  PTA_int lengths(1);

  lengths[0] = num_indices;

  float ang = ang_inc;
  LPoint3f origin(0.0, 0.0, 0.0);
  LVector3f x_axis(1.0, 0.0, 0.0);
  LVector3f z_axis(0.0, 0.0, 1.0);
  LPoint3f dx, dz;
  float t;

  coords[0] = origin;
  coords[1] = first_last_vert;
  vindex[0] = 0;
  vindex[1] = 1;
  vindex[num_indices-1] = 1; 

  int i;
  for (i = 2; i < num_indices-1; i++) {
        float sine,cosine;
        csincos(ang,&sine,&cosine);
    t = cosine * radius;
    dx = x_axis * t;
    t = sine * radius;
    dz = z_axis * t;
    coords[i] = dx + dz + offset;
    ang += ang_inc;
    vindex[i] = i;
  }

  colors[0] = diffuse;
  colors[1] = black;
  cindex[0] = 0;
  for (i = 1; i < num_indices; i++)
    cindex[i] = 1;

  GeomTrifan* tfan = new GeomTrifan;
  tfan->set_coords(coords, G_PER_VERTEX, vindex);
  tfan->set_colors(colors, G_PER_VERTEX, cindex);
  tfan->set_num_prims(1);
  tfan->set_lengths(lengths);

  GeomNode* geomnode = new GeomNode("spotlight_frustum_geom");
  geomnode->add_geom(tfan);
  
  NamedNode* root = new NamedNode("spotlight_frustum");
  RenderRelation* root_arc = new RenderRelation(root, geomnode);

  // Disable lighting
  LightTransition *light_trans = 
    new LightTransition(LightTransition::all_off());
  root_arc->set_transition(light_trans);

  // Disable texturing
  TextureTransition *tex_trans = 
    new TextureTransition(TextureTransition::off());
  root_arc->set_transition(tex_trans);

  // Turn off writes to Z
  DepthWriteTransition *depth_trans = 
    new DepthWriteTransition(DepthWriteTransition::off());
  root_arc->set_transition(depth_trans);

  // Enable transparency
  TransparencyTransition *col_trans = 
    new TransparencyTransition(TransparencyProperty::M_alpha);
  root_arc->set_transition(col_trans);

  // Disable culling
  CullFaceTransition *cull_trans = 
    new CullFaceTransition(CullFaceProperty::M_cull_none);
  root_arc->set_transition(cull_trans);

  return root;
}

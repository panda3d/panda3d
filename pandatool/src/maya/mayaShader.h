// Filename: mayaShader.h
// Created by:  drose (01Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MAYASHADER_H
#define MAYASHADER_H

#include <pandatoolbase.h>

#include <luse.h>
#include <lmatrix.h>

#include <string>

class MObject;
class MayaFile;
class EggPrimitive;

class MayaShader {
public:
  MayaShader(MObject engine);

  void set_attributes(EggPrimitive &primitive, MayaFile &file);
  LMatrix3d compute_texture_matrix();

  void output(ostream &out) const;

  string _name;

  bool _has_color;
  Colord _color;
  double _transparency;

  bool _has_texture;
  string _texture;

  LVector2f _coverage;
  LVector2f _translate_frame;
  double _rotate_frame;

  bool _mirror;
  bool _stagger;
  bool _wrap_u;
  bool _wrap_v;

  LVector2f _repeat_uv;
  LVector2f _offset;
  double _rotate_uv;

protected:
  bool read_surface_shader(MObject shader);
  void read_surface_color(MObject color);  
};

inline ostream &operator << (ostream &out, const MayaShader &shader) {
  shader.output(out);
  return out;
}
  
#endif


// Filename: mayaShaders.h
// Created by:  drose (11Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MAYASHADERS_H
#define MAYASHADERS_H

#include <pandatoolbase.h>

#include <map>
#include <string>

class MayaShader;
class MObject;

class MayaShaders {
public:
  MayaShader *find_shader_for_node(MObject node);
  MayaShader *find_shader_for_shading_engine(MObject engine);

protected:

  typedef map<string, MayaShader *> Shaders;
  Shaders _shaders;
};
  
#endif


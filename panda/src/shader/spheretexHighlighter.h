// Filename: spheretexHighlighter.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef SPHERETEXHIGHLIGHTER_H
#define SPHERETEXHIGHLIGHTER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "shader.h"
#include "spheretexShader.h"

#include "pt_Node.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : SpheretexHighlighter
// Description : Creates a highlight texture from a given light and
//               projects this onto the receiving object
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER SpheretexHighlighter : public FrustumShader
{
public:

  SpheretexHighlighter(int size = 128);
  
  virtual void pre_apply(Node *node, const AllAttributesWrapper &init_state,
                         const AllTransitionsWrapper &net_trans,
                         GraphicsStateGuardian *gsg);
  virtual void apply(Node *node, const AllAttributesWrapper &init_state,
                     const AllTransitionsWrapper &net_trans,
                     GraphicsStateGuardian *gsg);
  
  // MPG - should we make this check for powers of 2?
  INLINE void set_size(int size) { _size = size; }
  INLINE int get_size(void) { return _size; }
  //Override base class shader's set_priority to allow
  //us to set the priority of the contained projtex_shader at 
  //the same time
  virtual void set_priority(int priority);
  virtual void set_multipass(bool on);
  
protected:
  
  SpheretexShader               _spheretex_shader;
  int                           _size;
  
public:
  
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FrustumShader::init_type();
    register_type(_type_handle, "SpheretexHighlighter",
                  FrustumShader::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
private:
  
  static TypeHandle _type_handle;
};

#endif

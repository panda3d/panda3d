// Filename: shader.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef SHADER_H
#define SHADER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <configurable.h>
#include <namedNode.h>
#include <projectionNode.h>
#include <referenceCount.h>
#include <graphicsStateGuardian.h>
#include <allAttributesWrapper.h>
#include <allTransitionsWrapper.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Shader
// Description : Renders an effect on a list of receiving objects
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER Shader : public ReferenceCount, public Configurable
{
protected:
 
  Shader(void) : Configurable(), _priority(0) { }

  int _priority;
  bool _multipass_on;

public:
  
  virtual ~Shader(void) { }

  virtual void pre_apply(Node *, const AllAttributesWrapper &, 
			 const AllTransitionsWrapper &, 
			 GraphicsStateGuardian *);
 
  
  virtual void apply(Node *, const AllAttributesWrapper &, 
		     const AllTransitionsWrapper &, 
		     GraphicsStateGuardian *);

  virtual void set_priority(int priority);
  virtual int get_priority(void) const;
  virtual void set_multipass(bool on);

public:
  
  // This is used to visualize partial results, for debugging (and demos)
  class EXPCL_SHADER Visualize {
  public:
    Visualize(void);
    virtual ~Visualize(void);
    virtual void DisplayTexture(PT(Texture)&, Shader*) = 0;
    virtual void DisplayPixelBuffer(PT(PixelBuffer)&, Shader*) = 0;
    virtual void Flush(void) = 0;
  };
  
  static INLINE Visualize* get_viz(void);
  static INLINE void set_viz(Visualize*);
  
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    Configurable::init_type();
    register_type(_type_handle, "Shader",
		  ReferenceCount::get_class_type(),
		  Configurable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
  static Visualize* _viz;
  
private:
  
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : FrustumShader
// Description : Shader that computes effect based on a frustum
//		 (most often a light or a camera) for a list of
//		 receiving objects
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER FrustumShader : public Shader
{
protected:
  
  FrustumShader(void) : Shader() { }
  
public:
  
  virtual ~FrustumShader(void) { }
  
  INLINE int get_num_frusta(void) const { return _frusta.size(); }
  INLINE int add_frustum(ProjectionNode* frust);
  INLINE int remove_frustum(ProjectionNode* frust);
  
  typedef vector<ProjectionNode *> ProjectionVector;
  
protected:
  
  ProjectionVector			_frusta;
  
public:
  
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Shader::init_type();
    register_type(_type_handle, "FrustumShader",
		  Shader::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  
private:
  
  static TypeHandle _type_handle;
};

#include "shader.I"

#endif

// Filename: sgiglutGraphicsPipe.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef SGIGLUTGRAPHICSPIPE_H
#define SGIGLUTGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <string>
#include <sgiGraphicsPipe.h>
#include <glutGraphicsWindow.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : sgiglutGraphicsPipe
// Description : This kind of pipe can create glut windows but
//               supports the functionality of an sgi pipe with
//               hardware channels
////////////////////////////////////////////////////////////////////
class sgiglutGraphicsPipe : public sgiGraphicsPipe
{
public:
  sgiglutGraphicsPipe( const PipeSpecifier& );

  virtual TypeHandle get_window_type() const;

public:
  static GraphicsPipe* make_sgiglutGraphicsPipe(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif

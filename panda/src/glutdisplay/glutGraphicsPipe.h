// Filename: glutGraphicsPipe.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef GLUTGRAPHICSPIPE_H
#define GLUTGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <string>
#include <interactiveGraphicsPipe.h>
#include "glutGraphicsWindow.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : glutGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGLUT glutGraphicsPipe : public InteractiveGraphicsPipe
{
public:
  glutGraphicsPipe( const PipeSpecifier& );

  virtual TypeHandle get_window_type() const;

public:
  static GraphicsPipe* make_glutGraphicsPipe(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

protected:
  glutGraphicsPipe( void );
  glutGraphicsPipe( const glutGraphicsPipe& );
  glutGraphicsPipe& operator=(const glutGraphicsPipe&);
};

#endif

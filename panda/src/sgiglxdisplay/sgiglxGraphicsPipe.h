// Filename: sgiglxGraphicsPipe.h
// Created by:  cary (01Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef __SGIGLXGRAPHICSPIPE_H__
#define __SGIGLXGRAPHICSPIPE_H__

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <sgiGraphicsPipe.h>
#include <glxDisplay.h>

////////////////////////////////////////////////////////////////////
//       Class : SgiGlxGraphicsPipe
// Description : This kind of pipe can create glx windows but
//		 supports the functionality of an sgi pipe with
//		 hardware channels
////////////////////////////////////////////////////////////////////
class SgiGlxGraphicsPipe : public sgiGraphicsPipe, public glxDisplay {
public:
  SgiGlxGraphicsPipe( const PipeSpecifier& );

  virtual TypeHandle get_window_type() const;
  virtual glxDisplay *get_glx_display();

  static GraphicsPipe* make_sgiglxgraphicspipe(const FactoryParams &params);

  static TypeHandle get_class_type( void );
  static void init_type( void );
  virtual TypeHandle get_type( void ) const;
  virtual TypeHandle force_init_type( void );

private:
  static TypeHandle _type_handle;
};

#endif /* __SGIGLXGRAPHICSPIPE_H__ */

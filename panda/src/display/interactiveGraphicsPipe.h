// Filename: interactiveGraphicsPipe.h
// Created by:  cary (10Mar99)
//
////////////////////////////////////////////////////////////////////

#ifndef __INTERACTIVEGRAPHICSPIPE_H__
#define __INTERACTIVEGRAPHICSPIPE_H__

#include <pandabase.h>

#include "graphicsPipe.h"

class EXPCL_PANDA InteractiveGraphicsPipe : public GraphicsPipe {
public:

  InteractiveGraphicsPipe( const PipeSpecifier& );
  virtual ~InteractiveGraphicsPipe(void) = 0;

public:

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

protected:

  InteractiveGraphicsPipe(void);
  InteractiveGraphicsPipe(const InteractiveGraphicsPipe&);
  InteractiveGraphicsPipe& operator=(const InteractiveGraphicsPipe&);
};

#include "interactiveGraphicsPipe.I"

#endif /* __INTERACTIVEGRAPHICSPIPE_H__ */

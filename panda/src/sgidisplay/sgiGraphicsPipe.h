// Filename: sgiGraphicsPipe.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef SGIGRAPHICSPIPE_H
#define SGIGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "sgiHardwareChannel.h"

#include <interactiveGraphicsPipe.h>
#include <string>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : sgiGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class sgiGraphicsPipe : public InteractiveGraphicsPipe {
public:
	
  sgiGraphicsPipe(const PipeSpecifier&);
  virtual ~sgiGraphicsPipe() = 0;

  INLINE void* get_display() const { return _display; }
  INLINE int get_screen() const { return _screen; }

protected:

  typedef map<int,  PT(sgiHardwareChannel) > Channels;
  Channels _hw_chans;

  virtual int get_num_hw_channels();
  virtual HardwareChannel *get_hw_channel(GraphicsWindow* window,
					  int index);

public:

  static TypeHandle get_class_type();
  static void init_type();
  virtual TypeHandle get_type() const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

protected:
	
  void *_display;
  int _screen;
  int _num_channels;

private:

  static TypeHandle _type_handle;

protected:

  sgiGraphicsPipe();
  sgiGraphicsPipe(const sgiGraphicsPipe&);
  sgiGraphicsPipe& operator=(const sgiGraphicsPipe&);
};

#endif

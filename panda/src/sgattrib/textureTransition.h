// Filename: textureTransition.h
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTURETRANSITION_H
#define TEXTURETRANSITION_H

#include <pandabase.h>

#include <onOffTransition.h>
#include <texture.h>

////////////////////////////////////////////////////////////////////
// 	 Class : TextureTransition
// Description : This controls the assignment of the primary texture
//               to a piece of geometry.  If there is no
//               TextureTransition or the transition is off, the
//               geometry is rendered untextured; if there is an 'on'
//               TextureTransition affecting a piece of geometry it
//               will be rendered with the indicated texture.
//
//               At one time this was a MultiTransition because we
//               were thinking of using this interface to support
//               multitexturing.  But on reflection that is a flawed
//               idea because MultiTransitions don't support any
//               ordering, and we don't yet have an interface for
//               specifying multiple layers of UV coordinates anyway.
//               Until this is fully worked out, this will be a simple
//               OnOffTransition: either there is a texture or there
//               isn't.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureTransition : public OnOffTransition {
public:
  INLINE TextureTransition();
  INLINE TextureTransition(Texture *texture);
  INLINE static TextureTransition off();

  INLINE void set_on(Texture *texture);
  INLINE PT(Texture) get_texture() const;
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PT(Texture) _value;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

protected:
  static TypedWriteable *make_TextureTransition(const FactoryParams &params);
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnOffTransition::init_type();
    register_type(_type_handle, "TextureTransition",
		  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class TextureAttribute;
};

#include "textureTransition.I"

#endif

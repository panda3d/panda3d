// Filename: drawable.h
// Created by:  mike (09Jan97)
//
//
#ifndef DDRAWABLE_H
#define DDRAWABLE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <writeableConfigurable.h>
#include <referenceCount.h>
#include <boundedObject.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

class GraphicsStateGuardianBase;
class Datagram;
class DatagramIterator;
class BamReader;
class BamWriter;

////////////////////////////////////////////////////////////////////
//       Class : Drawable
// Description : Object that can be drawn (i.e. issues graphics
//               commands).
//		 NOTE: We had to change the name to dDrawable because
//		 the stupid bastards who wrote X didn't add a prefix
//		 to their variable names
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA dDrawable : public ReferenceCount, public WriteableConfigurable,
		  public BoundedObject {
public:

  dDrawable() : WriteableConfigurable() { }
  virtual ~dDrawable() { }

  virtual void draw(GraphicsStateGuardianBase *) { if (is_dirty()) config(); }

protected:
  virtual void propagate_stale_bound();

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    ReferenceCount::init_type();
    WriteableConfigurable::init_type();
    BoundedObject::init_type();
    register_type(_type_handle, "dDrawable",
		  ReferenceCount::get_class_type(),
		  WriteableConfigurable::get_class_type(),
		  BoundedObject::get_class_type());
  }

PUBLISHED: // Temporary kludge around ffi bug
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

public:
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}


private:
  static TypeHandle _type_handle;
};

#endif


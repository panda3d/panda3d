// Filename: collisionHandlerQueue.h
// Created by:  drose (27Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONHANDLERQUEUE_H
#define COLLISIONHANDLERQUEUE_H

#include <pandabase.h>

#include "collisionHandler.h"
#include "collisionEntry.h"

///////////////////////////////////////////////////////////////////
// 	 Class : CollisionHandlerQueue
// Description : A special kind of CollisionHandler that does nothing
//               except remember the CollisionEntries detected the
//               last pass.  This set of CollisionEntries may then be
//               queried by the calling function.  It's primarily
//               useful when a simple intersection test is being made,
//               e.g. for picking from the window.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionHandlerQueue : public CollisionHandler {
public:
  CollisionHandlerQueue();

  virtual void begin_group();
  virtual void add_entry(CollisionEntry *entry);

  int get_num_entries() const;
  CollisionEntry *get_entry(int n) const;

private:
  typedef vector<PT(CollisionEntry)> Entries;
  Entries _entries;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandler::init_type();
    register_type(_type_handle, "CollisionHandlerQueue",
		  CollisionHandler::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif
  
  


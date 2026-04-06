/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glGeomContext_src.h
 * @author drose
 * @date 2004-03-19
 */

#include "pandabase.h"
#include "geomContext.h"
#include "geomMunger.h"
#include "geomVertexData.h"
#include "pointerTo.h"
#include "pmap.h"
#include "deletedChain.h"

class CLP(GeomMunger);

/**
 *
 */
class EXPCL_GL CLP(GeomContext) : public GeomContext {
public:
  INLINE CLP(GeomContext)(Geom *geom);
  virtual ~CLP(GeomContext)();
  ALLOC_DELETED_CHAIN(CLP(GeomContext));

  bool get_display_list(GLuint &index, const CLP(GeomMunger) *munger,
                        UpdateSeq modified);
  void release_display_lists();

  void remove_munger(CLP(GeomMunger) *munger);

  // The different variants of the display list, for storing the different
  // states the geom might have been rendered in (each using a different
  // munger).
  class DisplayList {
  public:
    INLINE DisplayList();
    GLuint _index;
    UpdateSeq _modified;
  };
  typedef pmap<CLP(GeomMunger) *, DisplayList> DisplayLists;
  DisplayLists _display_lists;

  // The number of vertices encoded in the display list, for stats reporting.
#ifdef DO_PSTATS
  int _num_verts;
#endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "GeomContext",
                  GeomContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glGeomContext_src.I"

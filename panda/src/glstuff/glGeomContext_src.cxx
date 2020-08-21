/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glGeomContext_src.cxx
 * @author drose
 * @date 2004-03-19
 */

TypeHandle CLP(GeomContext)::_type_handle;

/**
 *
 */
CLP(GeomContext)::
~CLP(GeomContext)() {
  nassertv(_display_lists.empty());
}

/**
 * Looks up the display list index associated with the indicated munger, or
 * creates a new one if the munger has not yet been used to render this
 * context.  Fills index with the display list index, and returns true if the
 * display list is current (that is, it has the same modified stamp).
 */
bool CLP(GeomContext)::
get_display_list(GLuint &index, const CLP(GeomMunger) *munger,
                 UpdateSeq modified) {
#if defined(OPENGLES) || !defined(SUPPORT_FIXED_FUNCTION)
  // Display lists not supported by OpenGL ES.
  nassert_raise("OpenGL ES does not support display lists");
  return false;

#else
  DisplayList &dl = _display_lists[(CLP(GeomMunger) *)munger];
  bool list_current = (dl._modified == modified);
  if (dl._index == 0) {
    dl._index = glGenLists(1);
    list_current = false;
    if (munger != (CLP(GeomMunger) *)nullptr) {
      ((CLP(GeomMunger) *)munger)->_geom_contexts.insert(this);
    }
  }

  index = dl._index;
  dl._modified = modified;
  return list_current;
#endif  // OPENGLES
}

/**
 * Called only from the draw thread by
 * GLGraphicsStateGuardian::release_geom(), this should delete all of the
 * queued display lists immediately.
 */
void CLP(GeomContext)::
release_display_lists() {
#if defined(OPENGLES) || !defined(SUPPORT_FIXED_FUNCTION)
  // Display lists not supported by OpenGL ES.
  nassertv(_display_lists.empty());

#else
  DisplayLists::iterator dli;
  for (dli = _display_lists.begin();
       dli != _display_lists.end();
       ++dli) {
    CLP(GeomMunger) *munger = (*dli).first;
    const DisplayList &dl = (*dli).second;
    if (munger != (CLP(GeomMunger) *)nullptr) {
      munger->_geom_contexts.erase(this);
    }

    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "releasing index " << (int)dl._index << "\n";
    }
    glDeleteLists(dl._index, 1);
  }

  _display_lists.clear();
#endif  // OPENGLES
}

/**
 * Called when a glGeomMunger that we are pointing to destructs, this should
 * remove the record of that munger (and mark the display list for deletion).
 */
void CLP(GeomContext)::
remove_munger(CLP(GeomMunger) *munger) {
#if !defined(OPENGLES) && defined(SUPPORT_FIXED_FUNCTION)
  DisplayLists::iterator dli = _display_lists.find(munger);
  nassertv(dli != _display_lists.end());

  GLuint index = (*dli).second._index;
  _display_lists.erase(dli);

  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, munger->get_gsg());

  // We can't delete the display list immediately, because we might be running
  // in any thread.  Instead, enqueue the display list index and let it get
  // deleted at the end of the current or next frame.
  glgsg->record_deleted_display_list(index);
#endif
}

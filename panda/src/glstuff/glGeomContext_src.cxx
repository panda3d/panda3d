// Filename: glGeomContext_src.cxx
// Created by:  drose (19Mar04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

TypeHandle CLP(GeomContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomContext)::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CLP(GeomContext)::
~CLP(GeomContext)() {
  nassertv(_display_lists.empty());
}

///////////////////////////////////////////////////////////////////
//     Function: CLP(GeomContext)::get_display_list
//       Access: Public
//  Description: Looks up the display list index associated with the
//               indicated munger, or creates a new one if the munger
//               has not yet been used to render this context.  Fills
//               index with the display list index, and returns true
//               if the display list is current (that is, it has the
//               same modified stamp).
////////////////////////////////////////////////////////////////////
bool CLP(GeomContext)::
get_display_list(GLuint &index, const CLP(GeomMunger) *munger,
                 UpdateSeq modified) {
  DisplayList &dl = _display_lists[(CLP(GeomMunger) *)munger];
  bool list_current = (dl._modified == modified);
  if (dl._index == 0) {
    dl._index = GLP(GenLists)(1);    
    list_current = false;
    if (munger != (CLP(GeomMunger) *)NULL) {
      ((CLP(GeomMunger) *)munger)->_geom_contexts.insert(this);
    }
  }

  index = dl._index;
  dl._modified = modified;
  return list_current;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomContext)::release_display_lists
//       Access: Public
//  Description: Called only from the draw thread by
//               GLGraphicsStateGuardian::release_geom(), this should
//               delete all of the queued display lists immediately.
////////////////////////////////////////////////////////////////////
void CLP(GeomContext)::
release_display_lists() {
  if (_deprecated_index != 0) {
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "releasing index " << _deprecated_index << "\n";
    }
    GLP(DeleteLists)(_deprecated_index, 1);
    _deprecated_index = 0;
  }

  DisplayLists::iterator dli;
  for (dli = _display_lists.begin();
       dli != _display_lists.end();
       ++dli) {
    CLP(GeomMunger) *munger = (*dli).first;
    const DisplayList &dl = (*dli).second;
    if (munger != (CLP(GeomMunger) *)NULL) {
      munger->_geom_contexts.erase(this);
    }

    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "releasing index " << dl._index << "\n";
    }
    GLP(DeleteLists)(dl._index, 1);
  }

  _display_lists.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomContext)::remove_munger
//       Access: Public
//  Description: Called when a glGeomMunger that we are pointing to
//               destructs, this should remove the record of that
//               munger (and mark the display list for deletion).
////////////////////////////////////////////////////////////////////
void CLP(GeomContext)::
remove_munger(CLP(GeomMunger) *munger) {
  DisplayLists::iterator dli = _display_lists.find(munger);
  nassertv(dli != _display_lists.end());

  GLuint index = (*dli).second._index;
  _display_lists.erase(dli);

  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, munger->get_gsg());

  // We can't delete the display list immediately, because we might be
  // running in any thread.  Instead, enqueue the display list index
  // and let it get deleted at the end of the current or next frame.
  glgsg->record_deleted_display_list(index);
}

// Filename: virtualFileComposite.I
// Created by:  drose (03Aug02)
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


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileComposite::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileComposite::
VirtualFileComposite(VirtualFileSystem *file_system, const Filename &filename) :
  _file_system(file_system),
  _filename(filename)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileComposite::add_component
//       Access: Public
//  Description: Adds one more component to the composite directory.
//                The component should be a directory and the file
//                system and filename should match the composite.
////////////////////////////////////////////////////////////////////
INLINE void VirtualFileComposite::
add_component(VirtualFile *file) {
  nassertv(file->is_directory());
  nassertv(file->get_file_system() == _file_system);
  nassertv(file->get_filename() == _filename);
  
  _components.push_back(file);
}

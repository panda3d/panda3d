// Filename: weakReferenceList.h
// Created by:  drose (27Sep04)
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

#ifndef WEAKREFERENCELIST_H
#define WEAKREFERENCELIST_H

#include "pandabase.h"
#include "pset.h"

class WeakPointerToVoid;

////////////////////////////////////////////////////////////////////
//       Class : WeakReferenceList
// Description : This is a list of WeakPointerTo's that share a
//               reference to a given ReferenceCount object.  It is
//               stored in a separate class since it is assumed that
//               most ReferenceCount objects do not need to store this
//               list at all; this avoids bloating every
//               ReferenceCount object in the world with the size of
//               this object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS WeakReferenceList {
public:
  WeakReferenceList();
  ~WeakReferenceList();

  void add_reference(WeakPointerToVoid *ptv);
  void clear_reference(WeakPointerToVoid *ptv);

private:  
  typedef pset<WeakPointerToVoid *> Pointers;
  Pointers _pointers;
};

#include "weakReferenceList.I"

#endif

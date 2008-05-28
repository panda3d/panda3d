// Filename: weakReferenceList.h
// Created by:  drose (27Sep04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef WEAKREFERENCELIST_H
#define WEAKREFERENCELIST_H

#include "pandabase.h"
#include "pset.h"
#include "mutexImpl.h"

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
  MutexImpl _lock;
};

#include "weakReferenceList.I"

#endif

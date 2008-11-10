// Filename: asyncTaskCollection.h
// Created by:  drose (16Sep08)
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

#ifndef ASYNCTASKCOLLECTION_H
#define ASYNCTASKCOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "asyncTask.h"

////////////////////////////////////////////////////////////////////
//       Class : AsyncTaskCollection
// Description : A list of tasks, for instance as returned by some of
//               the AsyncTaskManager query functions.  This also
//               serves to define an AsyncTaskSequence.
//
//               TODO: None of this is thread-safe yet.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH AsyncTaskCollection {
PUBLISHED:
  AsyncTaskCollection();
  AsyncTaskCollection(const AsyncTaskCollection &copy);
  void operator = (const AsyncTaskCollection &copy);
  INLINE ~AsyncTaskCollection();

  void add_task(AsyncTask *task);
  bool remove_task(AsyncTask *task);
  void add_tasks_from(const AsyncTaskCollection &other);
  void remove_tasks_from(const AsyncTaskCollection &other);
  void remove_duplicate_tasks();
  bool has_task(AsyncTask *task) const;
  void clear();

  AsyncTask *find_task(const string &name) const;

  int get_num_tasks() const;
  AsyncTask *get_task(int index) const;
  MAKE_SEQ(get_tasks, get_num_tasks, get_task);
  void remove_task(int index);
  AsyncTask *operator [] (int index) const;
  int size() const;
  INLINE void operator += (const AsyncTaskCollection &other);
  INLINE AsyncTaskCollection operator + (const AsyncTaskCollection &other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(AsyncTask)) AsyncTasks;
  AsyncTasks _tasks;
};

INLINE ostream &operator << (ostream &out, const AsyncTaskCollection &col) {
  col.output(out);
  return out;
}

#include "asyncTaskCollection.I"

#endif



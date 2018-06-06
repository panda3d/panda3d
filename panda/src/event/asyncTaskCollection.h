/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskCollection.h
 * @author drose
 * @date 2008-09-16
 */

#ifndef ASYNCTASKCOLLECTION_H
#define ASYNCTASKCOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "asyncTask.h"

/**
 * A list of tasks, for instance as returned by some of the AsyncTaskManager
 * query functions.  This also serves to define an AsyncTaskSequence.
 *
 * TODO: None of this is thread-safe yet.
 */
class EXPCL_PANDA_EVENT AsyncTaskCollection {
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

  AsyncTask *find_task(const std::string &name) const;

  size_t get_num_tasks() const;
  AsyncTask *get_task(size_t index) const;
  MAKE_SEQ(get_tasks, get_num_tasks, get_task);
  void remove_task(size_t index);
  AsyncTask *operator [] (size_t index) const;
  size_t size() const;
  INLINE void operator += (const AsyncTaskCollection &other);
  INLINE AsyncTaskCollection operator + (const AsyncTaskCollection &other) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(AsyncTask)) AsyncTasks;
  AsyncTasks _tasks;
};

INLINE std::ostream &operator << (std::ostream &out, const AsyncTaskCollection &col) {
  col.output(out);
  return out;
}

#include "asyncTaskCollection.I"

#endif

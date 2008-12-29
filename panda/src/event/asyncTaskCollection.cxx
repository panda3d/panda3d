// Filename: asyncTaskCollection.cxx
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

#include "asyncTaskCollection.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskCollection::
AsyncTaskCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskCollection::
AsyncTaskCollection(const AsyncTaskCollection &copy) :
  _tasks(copy._tasks)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
operator = (const AsyncTaskCollection &copy) {
  _tasks = copy._tasks;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::add_task
//       Access: Published
//  Description: Adds a new AsyncTask to the collection.
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
add_task(AsyncTask *task) {
  // If the pointer to our internal array is shared by any other
  // AsyncTaskCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren AsyncTaskCollection
  // objects.
  nassertv(task != (AsyncTask *)NULL);

  if (_tasks.get_ref_count() > 1) {
    AsyncTasks old_tasks = _tasks;
    _tasks = AsyncTasks::empty_array(0);
    _tasks.v() = old_tasks.v();
  }

  _tasks.push_back(task);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::remove_task
//       Access: Published
//  Description: Removes the indicated AsyncTask from the collection.
//               Returns true if the task was removed, false if it was
//               not a member of the collection.
////////////////////////////////////////////////////////////////////
bool AsyncTaskCollection::
remove_task(AsyncTask *task) {
  int task_index = -1;
  for (int i = 0; task_index == -1 && i < (int)_tasks.size(); i++) {
    if (_tasks[i] == task) {
      task_index = i;
    }
  }

  if (task_index == -1) {
    // The indicated task was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // AsyncTaskCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren AsyncTaskCollection
  // objects.

  if (_tasks.get_ref_count() > 1) {
    AsyncTasks old_tasks = _tasks;
    _tasks = AsyncTasks::empty_array(0);
    _tasks.v() = old_tasks.v();
  }

  _tasks.erase(_tasks.begin() + task_index);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::add_tasks_from
//       Access: Published
//  Description: Adds all the AsyncTasks indicated in the other
//               collection to this task.  The other tasks are simply
//               appended to the end of the tasks in this list;
//               duplicates are not automatically removed.
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
add_tasks_from(const AsyncTaskCollection &other) {
  int other_num_tasks = other.get_num_tasks();
  for (int i = 0; i < other_num_tasks; i++) {
    add_task(other.get_task(i));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::remove_tasks_from
//       Access: Published
//  Description: Removes from this collection all of the AsyncTasks
//               listed in the other collection.
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
remove_tasks_from(const AsyncTaskCollection &other) {
  AsyncTasks new_tasks;
  int num_tasks = get_num_tasks();
  for (int i = 0; i < num_tasks; i++) {
    PT(AsyncTask) task = get_task(i);
    if (!other.has_task(task)) {
      new_tasks.push_back(task);
    }
  }
  _tasks = new_tasks;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::remove_duplicate_tasks
//       Access: Published
//  Description: Removes any duplicate entries of the same AsyncTasks
//               on this collection.  If a AsyncTask appears multiple
//               times, the first appearance is retained; subsequent
//               appearances are removed.
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
remove_duplicate_tasks() {
  AsyncTasks new_tasks;

  int num_tasks = get_num_tasks();
  for (int i = 0; i < num_tasks; i++) {
    PT(AsyncTask) task = get_task(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (task == get_task(j));
    }

    if (!duplicated) {
      new_tasks.push_back(task);
    }
  }

  _tasks = new_tasks;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::has_task
//       Access: Published
//  Description: Returns true if the indicated AsyncTask appears in
//               this collection, false otherwise.
////////////////////////////////////////////////////////////////////
bool AsyncTaskCollection::
has_task(AsyncTask *task) const {
  for (int i = 0; i < get_num_tasks(); i++) {
    if (task == get_task(i)) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::clear
//       Access: Published
//  Description: Removes all AsyncTasks from the collection.
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
clear() {
  _tasks.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::find_task
//       Access: Published
//  Description: Returns the task in the collection with the
//               indicated name, if any, or NULL if no task has
//               that name.
////////////////////////////////////////////////////////////////////
AsyncTask *AsyncTaskCollection::
find_task(const string &name) const {
  int num_tasks = get_num_tasks();
  for (int i = 0; i < num_tasks; i++) {
    AsyncTask *task = get_task(i);
    if (task->get_name() == name) {
      return task;
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::get_num_tasks
//       Access: Published
//  Description: Returns the number of AsyncTasks in the collection.
////////////////////////////////////////////////////////////////////
int AsyncTaskCollection::
get_num_tasks() const {
  return _tasks.size();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::get_task
//       Access: Published
//  Description: Returns the nth AsyncTask in the collection.
////////////////////////////////////////////////////////////////////
AsyncTask *AsyncTaskCollection::
get_task(int index) const {
  nassertr(index >= 0 && index < (int)_tasks.size(), NULL);

  return _tasks[index];
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::remove_task
//       Access: Published
//  Description: Removes the nth AsyncTask from the collection.
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
remove_task(int index) {
  // If the pointer to our internal array is shared by any other
  // AsyncTaskCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren AsyncTaskCollection
  // objects.

  if (_tasks.get_ref_count() > 1) {
    AsyncTasks old_tasks = _tasks;
    _tasks = AsyncTasks::empty_array(0);
    _tasks.v() = old_tasks.v();
  }

  nassertv(index >= 0 && index < (int)_tasks.size());
  _tasks.erase(_tasks.begin() + index);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::operator []
//       Access: Published
//  Description: Returns the nth AsyncTask in the collection.  This is
//               the same as get_task(), but it may be a more
//               convenient way to access it.
////////////////////////////////////////////////////////////////////
AsyncTask *AsyncTaskCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_tasks.size(), NULL);

  return _tasks[index];
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::size
//       Access: Published
//  Description: Returns the number of tasks in the collection.  This
//               is the same thing as get_num_tasks().
////////////////////////////////////////////////////////////////////
int AsyncTaskCollection::
size() const {
  return _tasks.size();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::output
//       Access: Published
//  Description: Writes a brief one-line description of the
//               AsyncTaskCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
output(ostream &out) const {
  if (get_num_tasks() == 1) {
    out << "1 AsyncTask";
  } else {
    out << get_num_tasks() << " AsyncTasks";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskCollection::write
//       Access: Published
//  Description: Writes a complete multi-line description of the
//               AsyncTaskCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void AsyncTaskCollection::
write(ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_tasks(); i++) {
    indent(out, indent_level) << *get_task(i) << "\n";
  }
}

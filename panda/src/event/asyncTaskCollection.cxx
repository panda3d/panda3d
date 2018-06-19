/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskCollection.cxx
 * @author drose
 * @date 2008-09-16
 */

#include "asyncTaskCollection.h"
#include "indent.h"

/**
 *
 */
AsyncTaskCollection::
AsyncTaskCollection() {
}

/**
 *
 */
AsyncTaskCollection::
AsyncTaskCollection(const AsyncTaskCollection &copy) :
  _tasks(copy._tasks)
{
}

/**
 *
 */
void AsyncTaskCollection::
operator = (const AsyncTaskCollection &copy) {
  _tasks = copy._tasks;
}

/**
 * Adds a new AsyncTask to the collection.
 */
void AsyncTaskCollection::
add_task(AsyncTask *task) {
  // If the pointer to our internal array is shared by any other
  // AsyncTaskCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren AsyncTaskCollection objects.
  nassertv(task != nullptr);

  if (_tasks.get_ref_count() > 1) {
    AsyncTasks old_tasks = _tasks;
    _tasks = AsyncTasks::empty_array(0);
    _tasks.v() = old_tasks.v();
  }

  _tasks.push_back(task);
}

/**
 * Removes the indicated AsyncTask from the collection.  Returns true if the
 * task was removed, false if it was not a member of the collection.
 */
bool AsyncTaskCollection::
remove_task(AsyncTask *task) {
  size_t task_index = (size_t)-1;
  for (size_t i = 0; i < _tasks.size(); ++i) {
    if (_tasks[i] == task) {
      task_index = i;
      break;
    }
  }

  if (task_index == (size_t)-1) {
    // The indicated task was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // AsyncTaskCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren AsyncTaskCollection objects.

  if (_tasks.get_ref_count() > 1) {
    AsyncTasks old_tasks = _tasks;
    _tasks = AsyncTasks::empty_array(0);
    _tasks.v() = old_tasks.v();
  }

  _tasks.erase(_tasks.begin() + task_index);
  return true;
}

/**
 * Adds all the AsyncTasks indicated in the other collection to this task.
 * The other tasks are simply appended to the end of the tasks in this list;
 * duplicates are not automatically removed.
 */
void AsyncTaskCollection::
add_tasks_from(const AsyncTaskCollection &other) {
  int other_num_tasks = other.get_num_tasks();
  for (int i = 0; i < other_num_tasks; i++) {
    add_task(other.get_task(i));
  }
}


/**
 * Removes from this collection all of the AsyncTasks listed in the other
 * collection.
 */
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

/**
 * Removes any duplicate entries of the same AsyncTasks on this collection.
 * If a AsyncTask appears multiple times, the first appearance is retained;
 * subsequent appearances are removed.
 */
void AsyncTaskCollection::
remove_duplicate_tasks() {
  AsyncTasks new_tasks;

  size_t num_tasks = get_num_tasks();
  for (size_t i = 0; i < num_tasks; i++) {
    PT(AsyncTask) task = get_task(i);
    bool duplicated = false;

    for (size_t j = 0; j < i && !duplicated; j++) {
      duplicated = (task == get_task(j));
    }

    if (!duplicated) {
      new_tasks.push_back(task);
    }
  }

  _tasks = new_tasks;
}

/**
 * Returns true if the indicated AsyncTask appears in this collection, false
 * otherwise.
 */
bool AsyncTaskCollection::
has_task(AsyncTask *task) const {
  for (size_t i = 0; i < get_num_tasks(); i++) {
    if (task == get_task(i)) {
      return true;
    }
  }
  return false;
}

/**
 * Removes all AsyncTasks from the collection.
 */
void AsyncTaskCollection::
clear() {
  _tasks.clear();
}

/**
 * Returns the task in the collection with the indicated name, if any, or NULL
 * if no task has that name.
 */
AsyncTask *AsyncTaskCollection::
find_task(const std::string &name) const {
  size_t num_tasks = get_num_tasks();
  for (size_t i = 0; i < num_tasks; ++i) {
    AsyncTask *task = get_task(i);
    if (task->get_name() == name) {
      return task;
    }
  }
  return nullptr;
}

/**
 * Returns the number of AsyncTasks in the collection.
 */
size_t AsyncTaskCollection::
get_num_tasks() const {
  return _tasks.size();
}

/**
 * Returns the nth AsyncTask in the collection.
 */
AsyncTask *AsyncTaskCollection::
get_task(size_t index) const {
  nassertr(index < _tasks.size(), nullptr);

  return _tasks[index];
}

/**
 * Removes the nth AsyncTask from the collection.
 */
void AsyncTaskCollection::
remove_task(size_t index) {
  // If the pointer to our internal array is shared by any other
  // AsyncTaskCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren AsyncTaskCollection objects.

  if (_tasks.get_ref_count() > 1) {
    AsyncTasks old_tasks = _tasks;
    _tasks = AsyncTasks::empty_array(0);
    _tasks.v() = old_tasks.v();
  }

  nassertv(index < _tasks.size());
  _tasks.erase(_tasks.begin() + index);
}

/**
 * Returns the nth AsyncTask in the collection.  This is the same as
 * get_task(), but it may be a more convenient way to access it.
 */
AsyncTask *AsyncTaskCollection::
operator [] (size_t index) const {
  nassertr(index < _tasks.size(), nullptr);
  return _tasks[index];
}

/**
 * Returns the number of tasks in the collection.  This is the same thing as
 * get_num_tasks().
 */
size_t AsyncTaskCollection::
size() const {
  return _tasks.size();
}

/**
 * Writes a brief one-line description of the AsyncTaskCollection to the
 * indicated output stream.
 */
void AsyncTaskCollection::
output(std::ostream &out) const {
  if (get_num_tasks() == 1) {
    out << "1 AsyncTask";
  } else {
    out << get_num_tasks() << " AsyncTasks";
  }
}

/**
 * Writes a complete multi-line description of the AsyncTaskCollection to the
 * indicated output stream.
 */
void AsyncTaskCollection::
write(std::ostream &out, int indent_level) const {
  for (size_t i = 0; i < get_num_tasks(); i++) {
    indent(out, indent_level) << *get_task(i) << "\n";
  }
}

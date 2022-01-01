/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionAsyncTask.I
 * @author rdb
 * @date 2021-11-29
 */

/**
 *
 */
INLINE FunctionAsyncTask::
FunctionAsyncTask(const std::string &name) :
  AsyncTask(name)
{
}

/**
 *
 */
INLINE FunctionAsyncTask::
FunctionAsyncTask(const std::string &name, FunctionAsyncTask::TaskFunction function) :
  AsyncTask(name),
  _function(std::move(function))
{
}

/**
 * Replaces the function that is called when the task runs.
 */
INLINE void FunctionAsyncTask::
set_function(TaskFunction function) {
  _function = std::move(function);
}

/**
 * Returns the function that is called when the task runs.
 */
INLINE const FunctionAsyncTask::TaskFunction &FunctionAsyncTask::
get_function() const {
  return _function;
}

/**
 * Replaces the function that is called when the task begins.  This is an
 * optional function.
 */
INLINE void FunctionAsyncTask::
set_upon_birth(BirthFunction upon_birth) {
  _upon_birth = std::move(upon_birth);
}

/**
 * Returns the function that is called when the task begins, or NULL if the
 * function is not defined.
 */
INLINE const FunctionAsyncTask::BirthFunction &FunctionAsyncTask::
get_upon_birth() const {
  return _upon_birth;
}

/**
 * Replaces the function that is called when the task ends.  This is an
 * optional function.
 */
INLINE void FunctionAsyncTask::
set_upon_death(FunctionAsyncTask::DeathFunction upon_death) {
  _upon_death = upon_death;
}

/**
 * Returns the function that is called when the task ends, or NULL if the
 * function is not defined.
 */
INLINE const FunctionAsyncTask::DeathFunction &FunctionAsyncTask::
get_upon_death() const {
  return _upon_death;
}

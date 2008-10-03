// Filename: genericAsyncTask.I
// Created by:  drose (03Oct08)
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


////////////////////////////////////////////////////////////////////
//     Function: GenericAsyncTask::set_function
//       Access: Published
//  Description: Replaces the function that is called when the task
//               runs.
////////////////////////////////////////////////////////////////////
INLINE void GenericAsyncTask::
set_function(GenericAsyncTask::TaskFunc *function) {
  _function = function;
}

////////////////////////////////////////////////////////////////////
//     Function: GenericAsyncTask::get_function
//       Access: Published
//  Description: Returns the function that is called when the task
//               runs.
////////////////////////////////////////////////////////////////////
INLINE GenericAsyncTask::TaskFunc *GenericAsyncTask::
get_function() const {
  return _function;
}

////////////////////////////////////////////////////////////////////
//     Function: GenericAsyncTask::set_upon_birth
//       Access: Published
//  Description: Replaces the function that is called when the task
//               begins.  This is an optional function.
////////////////////////////////////////////////////////////////////
INLINE void GenericAsyncTask::
set_upon_birth(GenericAsyncTask::BirthFunc *upon_birth) {
  _upon_birth = upon_birth;
}

////////////////////////////////////////////////////////////////////
//     Function: GenericAsyncTask::get_upon_birth
//       Access: Published
//  Description: Returns the function that is called when the task
//               begins, or NULL if the function is not defined.
////////////////////////////////////////////////////////////////////
INLINE GenericAsyncTask::BirthFunc *GenericAsyncTask::
get_upon_birth() const {
  return _upon_birth;
}

////////////////////////////////////////////////////////////////////
//     Function: GenericAsyncTask::set_upon_death
//       Access: Published
//  Description: Replaces the function that is called when the task
//               ends.  This is an optional function.
////////////////////////////////////////////////////////////////////
INLINE void GenericAsyncTask::
set_upon_death(GenericAsyncTask::DeathFunc *upon_death) {
  _upon_death = upon_death;
}

////////////////////////////////////////////////////////////////////
//     Function: GenericAsyncTask::get_upon_death
//       Access: Published
//  Description: Returns the function that is called when the task
//               ends, or NULL if the function is not defined.
////////////////////////////////////////////////////////////////////
INLINE GenericAsyncTask::DeathFunc *GenericAsyncTask::
get_upon_death() const {
  return _upon_death;
}

////////////////////////////////////////////////////////////////////
//     Function: GenericAsyncTask::set_user_data
//       Access: Published
//  Description: Replaces the void pointer that is passed to the task
//               function.  This is any arbitrary pointer; the task
//               object does no processing on it.
////////////////////////////////////////////////////////////////////
INLINE void GenericAsyncTask::
set_user_data(void *user_data) {
  _user_data = user_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GenericAsyncTask::get_user_data
//       Access: Published
//  Description: Returns the void pointer that is passed to the task
//               function.
////////////////////////////////////////////////////////////////////
INLINE void *GenericAsyncTask::
get_user_data() const {
  return _user_data;
}

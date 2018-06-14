/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file genericThread.cxx
 * @author drose
 * @date 2011-11-09
 */

#include "genericThread.h"
#include "pnotify.h"

TypeHandle GenericThread::_type_handle;

/**
 *
 */
GenericThread::
GenericThread(const std::string &name, const std::string &sync_name) :
  Thread(name, sync_name)
{
  _function = nullptr;
  _user_data = nullptr;
}

/**
 *
 */
GenericThread::
GenericThread(const std::string &name, const std::string &sync_name, GenericThread::ThreadFunc *function, void *user_data) :
  Thread(name, sync_name),
  _function(function),
  _user_data(user_data)
{
}

/**
 * This is the thread's main execution function.
 */
void GenericThread::
thread_main() {
  nassertv(_function != nullptr);
  (*_function)(_user_data);
}

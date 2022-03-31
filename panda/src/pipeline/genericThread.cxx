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

#ifndef CPPPARSER

TypeHandle GenericThread::_type_handle;

/**
 *
 */
GenericThread::
GenericThread(const std::string &name, const std::string &sync_name) :
  Thread(name, sync_name)
{
}

/**
 *
 */
GenericThread::
GenericThread(const std::string &name, const std::string &sync_name, GenericThread::ThreadFunc *function, void *user_data) :
  Thread(name, sync_name),
  _function(std::bind(function, user_data))
{
}

/**
 *
 */
GenericThread::
GenericThread(const std::string &name, const std::string &sync_name, std::function<void()> function) :
  Thread(name, sync_name),
  _function(std::move(function))
{
}

/**
 * This is the thread's main execution function.
 */
void GenericThread::
thread_main() {
  nassertv(_function);
  _function();
}

#endif

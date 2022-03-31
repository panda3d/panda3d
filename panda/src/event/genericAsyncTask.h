/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file genericAsyncTask.h
 * @author drose
 * @date 2008-09-16
 */

#ifndef GENERICASYNCTASK_H
#define GENERICASYNCTASK_H

#include "pandabase.h"

#include "asyncTask.h"

/**
 * Associates a generic C-style function pointer with an AsyncTask object.
 * You can use this when you want to create an AsyncTask without having to
 * subclass.
 */
class EXPCL_PANDA_EVENT GenericAsyncTask : public AsyncTask {
public:
  typedef DoneStatus TaskFunc(GenericAsyncTask *task, void *user_data);
  typedef void BirthFunc(GenericAsyncTask *task, void *user_data);
  typedef void DeathFunc(GenericAsyncTask *task, bool clean_exit, void *user_data);

  GenericAsyncTask(const std::string &name = std::string());
  GenericAsyncTask(const std::string &name, TaskFunc *function, void *user_data);
  ALLOC_DELETED_CHAIN(GenericAsyncTask);

  INLINE void set_function(TaskFunc *function);
  INLINE TaskFunc *get_function() const;

  INLINE void set_upon_birth(BirthFunc *function);
  INLINE BirthFunc *get_upon_birth() const;

  INLINE void set_upon_death(DeathFunc *function);
  INLINE DeathFunc *get_upon_death() const;

  INLINE void set_user_data(void *user_data);
  INLINE void *get_user_data() const;

protected:
  virtual bool is_runnable();
  virtual DoneStatus do_task();
  virtual void upon_birth(AsyncTaskManager *manager);
  virtual void upon_death(AsyncTaskManager *manager, bool clean_exit);

private:
  TaskFunc *_function;
  BirthFunc *_upon_birth;
  DeathFunc *_upon_death;
  void *_user_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "GenericAsyncTask",
                  AsyncTask::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "genericAsyncTask.I"

#endif

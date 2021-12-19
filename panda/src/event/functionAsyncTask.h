/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionAsyncTask.h
 * @author rdb
 * @date 2021-11-29
 */

#ifndef FUNCTIONASYNCTASK_H
#define FUNCTIONASYNCTASK_H

#include "pandabase.h"

#include "asyncTask.h"

#ifndef CPPPARSER
#include <functional>

/**
 * Associates a generic std::function (eg. a lambda) with an AsyncTask object.
 * You can use this when you want to create an AsyncTask without having to
 * subclass.
 *
 * @since 1.11.0
 */
class EXPCL_PANDA_EVENT FunctionAsyncTask final : public AsyncTask {
public:
  typedef std::function<DoneStatus (FunctionAsyncTask *task)> TaskFunction;
  typedef std::function<void (FunctionAsyncTask *task)> BirthFunction;
  typedef std::function<void (FunctionAsyncTask *task, bool clean_exit)> DeathFunction;

  INLINE FunctionAsyncTask(const std::string &name = std::string());
  INLINE FunctionAsyncTask(const std::string &name, TaskFunction function);
  ALLOC_DELETED_CHAIN(FunctionAsyncTask);

  INLINE void set_function(TaskFunction function);
  INLINE const TaskFunction &get_function() const;

  INLINE void set_upon_birth(BirthFunction function);
  INLINE const BirthFunction &get_upon_birth() const;

  INLINE void set_upon_death(DeathFunction function);
  INLINE const DeathFunction &get_upon_death() const;

protected:
  virtual bool is_runnable() override;
  virtual DoneStatus do_task() override;
  virtual void upon_birth(AsyncTaskManager *manager) override;
  virtual void upon_death(AsyncTaskManager *manager, bool clean_exit) override;

private:
  TaskFunction _function;
  BirthFunction _upon_birth;
  DeathFunction _upon_death;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "FunctionAsyncTask",
                  AsyncTask::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "functionAsyncTask.I"

#endif  // CPPPARSER

#endif

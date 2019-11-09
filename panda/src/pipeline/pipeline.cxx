/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipeline.cxx
 * @author drose
 * @date 2002-02-21
 */

#include "pipeline.h"
#include "pipelineCyclerTrueImpl.h"
#include "configVariableInt.h"
#include "config_pipeline.h"

Pipeline *Pipeline::_render_pipeline = nullptr;

/**
 *
 */
Pipeline::
Pipeline(const std::string &name, int num_stages) :
  Namable(name),
#ifdef THREADED_PIPELINE
  _num_stages(num_stages),
  _cycle_lock("Pipeline cycle"),
  _lock("Pipeline"),
  _next_cycle_seq(1)
#else
  _num_stages(1)
#endif
{
#ifdef THREADED_PIPELINE
  // We maintain all of the cyclers in the world on one of two linked
  // lists.  Cyclers that are "clean", which is to say, they have the
  // same value across all pipeline stages, are stored on the _clean
  // list.  Cyclers that are "dirty", which have different values
  // across some pipeline stages, are stored instead on the _dirty
  // list.  Cyclers can move themselves from clean to dirty by calling
  // add_dirty_cycler(), and cyclers get moved from dirty to clean
  // during cycle().

  // To visit each cycler once requires traversing both lists.
  _clean.make_head();
  _dirty.make_head();

  // We also store the total count of all cyclers, clean and dirty, in
  // _num_cyclers; and the count of only dirty cyclers in _num_dirty_cyclers.
  _num_cyclers = 0;
  _num_dirty_cyclers = 0;

  // This flag is true only during the call to cycle().
  _cycling = false;

#else
  if (num_stages != 1) {
    pipeline_cat.warning()
      << "Requested " << num_stages
      << " pipeline stages but multithreaded render pipelines not enabled in build.\n";
  }
#endif  // THREADED_PIPELINE

  nassertv(num_stages >= 1);
}

/**
 *
 */
Pipeline::
~Pipeline() {
#ifdef THREADED_PIPELINE
  nassertv(_num_cyclers == 0);
  nassertv(_num_dirty_cyclers == 0);
  _clean.clear_head();
  _dirty.clear_head();
  nassertv(!_cycling);
#endif  // THREADED_PIPELINE
}

/**
 * Flows all the pipeline data down to the next stage.
 */
void Pipeline::
cycle() {
#ifdef THREADED_PIPELINE
  if (pipeline_cat.is_spam()) {
    pipeline_cat.spam()
      << "Beginning the pipeline cycle\n";
  }

  pvector< PT(CycleData) > saved_cdatas;
  {
    ReMutexHolder cycle_holder(_cycle_lock);
    unsigned int prev_seq, next_seq;
    PipelineCyclerLinks prev_dirty;
    {
      // We can't hold the lock protecting the linked lists during the cycling
      // itself, since it could cause a deadlock.
      MutexHolder holder(_lock);
      if (_num_stages == 1) {
        // No need to cycle if there's only one stage.
        nassertv(_dirty._next == &_dirty);
        return;
      }

      nassertv(!_cycling);
      _cycling = true;

      // Increment the cycle sequence number, which is used by this method to
      // communicate with remove_cycler() about the status of dirty cyclers.
      prev_seq = next_seq = _next_cycle_seq;
      if (++next_seq == 0) {
        // Skip 0, which is a reserved number used to indicate a clean cycler.
        ++next_seq;
      }
      _next_cycle_seq = next_seq;

      // Move the dirty list to prev_dirty, for processing.
      prev_dirty.make_head();
      prev_dirty.take_list(_dirty);

      saved_cdatas.reserve(_num_dirty_cyclers);
      _num_dirty_cyclers = 0;
    }

    // This is duplicated for different number of stages, as an optimization.
    switch (_num_stages) {
    case 2:
      while (prev_dirty._next != &prev_dirty) {
        PipelineCyclerLinks *link = prev_dirty._next;
        while (link != &prev_dirty) {
          PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)link;

          if (!cycler->_lock.try_lock()) {
            // No big deal, just move on to the next one for now, and we'll
            // come back around to it.  It's important not to block here in
            // order to prevent one cycler from deadlocking another.
            if (link->_prev != &prev_dirty || link->_next != &prev_dirty) {
              link = cycler->_next;
              continue;
            } else {
              // Well, we are the last cycler left, so we might as well wait.
              // This is necessary to trigger the deadlock detection code.
              cycler->_lock.lock();
            }
          }

          MutexHolder holder(_lock);
          cycler->remove_from_list();

          // We save the result of cycle(), so that we can defer the side-
          // effects that might occur when CycleDatas destruct, at least until
          // the end of this loop.
          saved_cdatas.push_back(cycler->cycle_2());

          // cycle_2() won't leave a cycler dirty.  Add it to the clean list.
          nassertd(!cycler->_dirty) break;
          cycler->insert_before(&_clean);
#ifdef DEBUG_THREADS
          inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
          cycler->_lock.unlock();
          break;
        }
      }
      break;

    case 3:
      while (prev_dirty._next != &prev_dirty) {
        PipelineCyclerLinks *link = prev_dirty._next;
        while (link != &prev_dirty) {
          PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)link;

          if (!cycler->_lock.try_lock()) {
            // No big deal, just move on to the next one for now, and we'll
            // come back around to it.  It's important not to block here in
            // order to prevent one cycler from deadlocking another.
            if (link->_prev != &prev_dirty || link->_next != &prev_dirty) {
              link = cycler->_next;
              continue;
            } else {
              // Well, we are the last cycler left, so we might as well wait.
              // This is necessary to trigger the deadlock detection code.
              cycler->_lock.lock();
            }
          }

          MutexHolder holder(_lock);
          cycler->remove_from_list();

          saved_cdatas.push_back(cycler->cycle_3());

          if (cycler->_dirty) {
            // The cycler is still dirty.  Add it back to the dirty list.
            nassertd(cycler->_dirty == prev_seq) break;
            cycler->insert_before(&_dirty);
            cycler->_dirty = next_seq;
            ++_num_dirty_cyclers;
          } else {
            // The cycler is now clean.  Add it back to the clean list.
            cycler->insert_before(&_clean);
#ifdef DEBUG_THREADS
            inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
          }
          cycler->_lock.unlock();
          break;
        }
      }
      break;

    default:
      while (prev_dirty._next != &prev_dirty) {
        PipelineCyclerLinks *link = prev_dirty._next;
        while (link != &prev_dirty) {
          PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)link;

          if (!cycler->_lock.try_lock()) {
            // No big deal, just move on to the next one for now, and we'll
            // come back around to it.  It's important not to block here in
            // order to prevent one cycler from deadlocking another.
            if (link->_prev != &prev_dirty || link->_next != &prev_dirty) {
              link = cycler->_next;
              continue;
            } else {
              // Well, we are the last cycler left, so we might as well wait.
              // This is necessary to trigger the deadlock detection code.
              cycler->_lock.lock();
            }
          }

          MutexHolder holder(_lock);
          cycler->remove_from_list();

          saved_cdatas.push_back(cycler->cycle());

          if (cycler->_dirty) {
            // The cycler is still dirty.  Add it back to the dirty list.
            nassertd(cycler->_dirty == prev_seq) break;
            cycler->insert_before(&_dirty);
            cycler->_dirty = next_seq;
            ++_num_dirty_cyclers;
          } else {
            // The cycler is now clean.  Add it back to the clean list.
            cycler->insert_before(&_clean);
#ifdef DEBUG_THREADS
            inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
          }
          cycler->_lock.unlock();
          break;
        }
      }
      break;
    }

    // Now we're ready for the next frame.
    prev_dirty.clear_head();
    _cycling = false;
  }

  // And now it's safe to let the CycleData pointers in saved_cdatas destruct,
  // which may cause cascading deletes, and which will in turn cause
  // PipelineCyclers to remove themselves from (or add themselves to) the
  // _dirty list.
  saved_cdatas.clear();

  if (pipeline_cat.is_debug()) {
    pipeline_cat.debug()
      << "Finished the pipeline cycle\n";
  }

#endif  // THREADED_PIPELINE
}

/**
 * Specifies the number of stages required for the pipeline.
 */
void Pipeline::
set_num_stages(int num_stages) {
  nassertv(num_stages >= 1);
#ifdef THREADED_PIPELINE
  // Make sure it's not currently cycling.
  ReMutexHolder cycle_holder(_cycle_lock);
  MutexHolder holder(_lock);
  if (num_stages != _num_stages) {

    // We need to lock every PipelineCycler object attached to this pipeline
    // before we can adjust the number of stages.
    PipelineCyclerLinks *links;
    for (links = _clean._next; links != &_clean; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.lock();
    }
    for (links = _dirty._next; links != &_dirty; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.lock();
    }

    _num_stages = num_stages;

    for (links = _clean._next; links != &_clean; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->set_num_stages(num_stages);
    }
    for (links = _dirty._next; links != &_dirty; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->set_num_stages(num_stages);
    }

    // Now release them all.
    int count = 0;
    for (links = _clean._next; links != &_clean; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.unlock();
      ++count;
    }
    for (links = _dirty._next; links != &_dirty; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.unlock();
      ++count;
    }
    nassertv(count == _num_cyclers);
  }

#else  // THREADED_PIPELINE
  if (num_stages != 1) {
    pipeline_cat.warning()
      << "Requested " << num_stages
      << " pipeline stages but multithreaded render pipelines not enabled in build.\n";
  }
  _num_stages = 1;
#endif  // THREADED_PIPELINE
}

#ifdef THREADED_PIPELINE
/**
 * Adds the indicated cycler to the list of cyclers associated with the
 * pipeline.  This method only exists when true pipelining is configured on.
 */
void Pipeline::
add_cycler(PipelineCyclerTrueImpl *cycler) {
  // It's safe to add it to the list while cycling, since the _clean list is
  // not touched during the cycle loop.
  MutexHolder holder(_lock);
  nassertv(!cycler->_dirty);

  cycler->insert_before(&_clean);
  ++_num_cyclers;

#ifdef DEBUG_THREADS
  inc_cycler_type(_all_cycler_types, cycler->get_parent_type(), 1);
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 * Marks the indicated cycler as "dirty", meaning it will need to be cycled
 * next frame.  This both adds it to the "dirty" set and also sets the "dirty"
 * flag within the cycler.  This method only exists when true pipelining is
 * configured on.
 */
void Pipeline::
add_dirty_cycler(PipelineCyclerTrueImpl *cycler) {
  nassertv(cycler->_lock.debug_is_locked());

  // It's safe to add it to the list while cycling, since it's not currently
  // on the dirty list.
  MutexHolder holder(_lock);
  nassertv(!cycler->_dirty);
  nassertv(_num_stages != 1);

  // Remove it from the "clean" list and add it to the "dirty" list.
  cycler->remove_from_list();
  cycler->insert_before(&_dirty);
  cycler->_dirty = _next_cycle_seq;
  ++_num_dirty_cyclers;

#ifdef DEBUG_THREADS
  inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), 1);
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 * Removes the indicated cycler from the list of cyclers associated with the
 * pipeline.  This method only exists when true pipelining is configured on.
 */
void Pipeline::
remove_cycler(PipelineCyclerTrueImpl *cycler) {
  nassertv(cycler->_lock.debug_is_locked());

  MutexHolder holder(_lock);

  // If it's dirty, it may currently be processed by cycle(), so we need to be
  // careful not to cause a race condition.  It's safe for us to remove it
  // during cycle only if it's 0 (clean) or _next_cycle_seq (scheduled for the
  // next cycle, so not owned by the current one).
  while (cycler->_dirty != 0 && cycler->_dirty != _next_cycle_seq) {
    if (_cycle_lock.try_lock()) {
      // OK, great, we got the lock, so it finished cycling already.
      nassertv(!_cycling);

      --_num_cyclers;
      cycler->remove_from_list();

      cycler->_dirty = false;
      --_num_dirty_cyclers;

  #ifdef DEBUG_THREADS
      inc_cycler_type(_all_cycler_types, cycler->get_parent_type(), -1);
      inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
  #endif

      _cycle_lock.unlock();
      return;
    } else {
      // It's possibly currently being cycled.  We will wait for the cycler
      // to be done with it, so that we can safely remove it.
      _lock.unlock();
      cycler->_lock.unlock();
      Thread::force_yield();
      cycler->_lock.lock();
      _lock.lock();
    }
  }

  // It's not being owned by a cycle operation, so it's fair game.
  --_num_cyclers;
  cycler->remove_from_list();

#ifdef DEBUG_THREADS
  inc_cycler_type(_all_cycler_types, cycler->get_parent_type(), -1);
#endif

  if (cycler->_dirty) {
    cycler->_dirty = 0;
    --_num_dirty_cyclers;
#ifdef DEBUG_THREADS
    inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
  }
}
#endif  // THREADED_PIPELINE

#if defined(THREADED_PIPELINE) && defined(DEBUG_THREADS)
/**
 * Walks through the list of all the different PipelineCycler types in the
 * universe.  For each one, calls the indicated callback function with the
 * TypeHandle of the respective type (actually, the result of
 * cycler::get_parent_type()) and the count of pipeline cyclers of that type.
 * Mainly used for PStats reporting.
 */
void Pipeline::
iterate_all_cycler_types(CallbackFunc *func, void *data) const {
  // Make sure it's not currently cycling.
  ReMutexHolder cycle_holder(_cycle_lock);
  MutexHolder holder(_lock);
  TypeCount::const_iterator ci;
  for (ci = _all_cycler_types.begin(); ci != _all_cycler_types.end(); ++ci) {
    func((*ci).first, (*ci).second, data);
  }
}
#endif  // THREADED_PIPELINE && DEBUG_THREADS

#if defined(THREADED_PIPELINE) && defined(DEBUG_THREADS)
/**
 * Walks through the list of all the different PipelineCycler types, for only
 * the dirty PipelineCyclers.  See also iterate_all_cycler_types().
 */
void Pipeline::
iterate_dirty_cycler_types(CallbackFunc *func, void *data) const {
  // Make sure it's not currently cycling.
  ReMutexHolder cycle_holder(_cycle_lock);
  MutexHolder holder(_lock);
  TypeCount::const_iterator ci;
  for (ci = _dirty_cycler_types.begin(); ci != _dirty_cycler_types.end(); ++ci) {
    func((*ci).first, (*ci).second, data);
  }
}
#endif  // THREADED_PIPELINE && DEBUG_THREADS

/**
 *
 */
void Pipeline::
make_render_pipeline() {
  ConfigVariableInt pipeline_stages
    ("pipeline-stages", 1,
     PRC_DESC("The initial number of stages in the render pipeline.  This is "
              "only meaningful if threaded pipelining is compiled into "
              "Panda.  In most cases, you should not set this at all anyway, "
              "since the pipeline can automatically grow stages as needed, "
              "but it will not remove stages automatically, and having more "
              "pipeline stages than your application requires will incur "
              "additional runtime overhead."));

  nassertv(_render_pipeline == nullptr);
  _render_pipeline = new Pipeline("render", pipeline_stages);
}

#if defined(THREADED_PIPELINE) && defined(DEBUG_THREADS)
/**
 * Increments (or decrements, according to added) the value for TypeHandle in
 * the indicated TypeCount map.  This is used in DEBUG_THREADS mode to track
 * the types of PipelineCyclers that are coming and going, mainly for PStats
 * reporting.
 *
 * It is assumed the lock is held during this call.
 */
void Pipeline::
inc_cycler_type(TypeCount &count, TypeHandle type, int addend) {
  TypeCount::iterator ci = count.find(type);
  if (ci == count.end()) {
    ci = count.insert(TypeCount::value_type(type, 0)).first;
  }
  (*ci).second += addend;
  nassertv((*ci).second >= 0);
}
#endif  // THREADED_PIPELINE && DEBUG_THREADS

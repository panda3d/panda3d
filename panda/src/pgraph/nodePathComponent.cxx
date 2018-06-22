/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePathComponent.cxx
 * @author drose
 * @date 2002-02-25
 */

#include "nodePathComponent.h"
#include "lightMutexHolder.h"

// We start the key counters off at 1, since 0 is reserved for an empty
// NodePath (and also for an unassigned key).
int NodePathComponent::_next_key = 1;
LightMutex NodePathComponent::_key_lock("NodePathComponent::_key_lock");
TypeHandle NodePathComponent::_type_handle;
TypeHandle NodePathComponent::CData::_type_handle;


/**
 *
 */
CycleData *NodePathComponent::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Constructs a new NodePathComponent from the indicated node.  Don't try to
 * call this directly; ask the PandaNode to do it for you.
 */
NodePathComponent::
NodePathComponent(PandaNode *node, NodePathComponent *next,
                  int pipeline_stage, Thread *current_thread) :
  _node(node),
  _key(0)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif

  for (int pipeline_stage_i = pipeline_stage;
       pipeline_stage_i >= 0;
       --pipeline_stage_i) {
    CDStageWriter cdata(_cycler, pipeline_stage_i, current_thread);
    cdata->_next = next;

    if (next != nullptr) {
      cdata->_length = next->get_length(pipeline_stage_i, current_thread) + 1;
    }
  }
}

/**
 * Returns an index number that is guaranteed to be unique for this particular
 * NodePathComponent, and not to be reused for the lifetime of the application
 * (barring integer overflow).
 */
int NodePathComponent::
get_key() const {
  LightMutexHolder holder(_key_lock);
  if (_key == 0) {
    // The first time someone asks for a particular component's key, we make
    // it up on the spot.  This helps keep us from wasting index numbers
    // generating a unique number for *every* component in the world (we only
    // have 4.2 billion 32-bit integers, after all)
    ((NodePathComponent *)this)->_key = _next_key++;
  }
  return _key;
}

/**
 * Returns true if this component represents the top node in the path.
 */
bool NodePathComponent::
is_top_node(int pipeline_stage, Thread *current_thread) const {
  CDStageReader cdata(_cycler, pipeline_stage, current_thread);
  return (cdata->_next == nullptr);
}

/**
 * Returns the length of the path to this node.
 */
int NodePathComponent::
get_length(int pipeline_stage, Thread *current_thread) const {
  CDStageReader cdata(_cycler, pipeline_stage, current_thread);
  return cdata->_length;
}

/**
 * Checks that the length indicated by the component is one more than the
 * length of its predecessor.  If this is broken, fixes it and returns true
 * indicating the component has been changed; otherwise, returns false.
 */
bool NodePathComponent::
fix_length(int pipeline_stage, Thread *current_thread) {
  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);

  int length_should_be = 1;
  if (cdata->_next != nullptr) {
    length_should_be = cdata->_next->get_length(pipeline_stage, current_thread) + 1;
  }

  if (cdata->_length == length_should_be) {
    return false;
  }

  CDStageWriter cdataw(_cycler, pipeline_stage, cdata);
  cdataw->_length = length_should_be;
  return true;
}

/**
 * The recursive implementation of NodePath::output(), this writes the names
 * of each node component in order from beginning to end, by first walking to
 * the end of the linked list and then outputting from there.
 */
void NodePathComponent::
output(std::ostream &out) const {
  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();

  PandaNode *node = get_node();
  NodePathComponent *next = get_next(pipeline_stage, current_thread);
  if (next != nullptr) {
    // This is not the head of the list; keep going up.
    next->output(out);
    out << "/";

    PandaNode *parent_node = next->get_node();
    if (parent_node->find_stashed(node) >= 0) {
      // The node is stashed.
      out << "@@";

    } else if (node->find_parent(parent_node) < 0) {
      // Oops, there's an error.  This shouldn't happen.
      out << ".../";
    }
  }

  // Now output this component.
  if (node->has_name()) {
    out << node->get_name();
  } else {
    out << "-" << node->get_type();
  }
  // out << "[" << this->get_length() << "]";
}

/**
 * Sets the next pointer in the path.
 */
void NodePathComponent::
set_next(NodePathComponent *next, int pipeline_stage, Thread *current_thread) {
  nassertv(next != nullptr);
  CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
  cdata->_next = next;
}

/**
 * Severs any connection to the next pointer in the path and makes this
 * component a top node.
 */
void NodePathComponent::
set_top_node(int pipeline_stage, Thread *current_thread) {
  CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
  cdata->_next = nullptr;
}

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file attribNodeRegistry.cxx
 * @author drose
 * @date 2007-07-07
 */

#include "attribNodeRegistry.h"
#include "lightMutexHolder.h"

AttribNodeRegistry * TVOLATILE AttribNodeRegistry::_global_ptr;

/**
 *
 */
AttribNodeRegistry::
AttribNodeRegistry() {
}

/**
 * Adds the indicated NodePath to the registry.  The name and type of the node
 * are noted at the time of this call; if the name changes later, it will not
 * update the registry index.
 *
 * The NodePath must reference some kind of an attribute node, such as a
 * LightNode or a PlaneNode.  When bam files that reference an attribute node
 * of the same type and the same name are loaded, they will quietly be
 * redirected to reference this NodePath.
 *
 * If there is already a node matching the indicated name and type, it will be
 * replaced.
 */
void AttribNodeRegistry::
add_node(const NodePath &attrib_node) {
  nassertv(!attrib_node.is_empty());
  LightMutexHolder holder(_lock);

  std::pair<Entries::iterator, bool> result = _entries.insert(Entry(attrib_node));
  if (!result.second) {
    // Replace an existing node.
    (*result.first)._node = attrib_node;
  }
}

/**
 * Removes the indicated NodePath from the registry.  The name of the node
 * must not have changed since the matching call to add_node(), or it will not
 * be successfully removed.
 *
 * Returns true if the NodePath is found and removed, false if it is not found
 * (for instance, because the name has changed).
 */
bool AttribNodeRegistry::
remove_node(const NodePath &attrib_node) {
  nassertr(!attrib_node.is_empty(), false);
  LightMutexHolder holder(_lock);
  Entries::iterator ei = _entries.find(Entry(attrib_node));
  if (ei != _entries.end()) {
    _entries.erase(ei);
    return true;
  }
  return false;
}

/**
 * Looks up the indicated NodePath in the registry.  If there is a node
 * already in the registry with the matching name and type, returns that
 * NodePath instead; otherwise, returns the original NodePath.
 */
NodePath AttribNodeRegistry::
lookup_node(const NodePath &orig_node) const {
  nassertr(!orig_node.is_empty(), orig_node);

  LightMutexHolder holder(_lock);
  Entries::const_iterator ei = _entries.find(Entry(orig_node));
  if (ei != _entries.end()) {
    return (*ei)._node;
  }
  return orig_node;
}

/**
 * Returns the total number of nodes in the registry.
 */
int AttribNodeRegistry::
get_num_nodes() const {
  LightMutexHolder holder(_lock);
  return _entries.size();
}

/**
 * Returns the nth NodePath recorded in the registry.
 */
NodePath AttribNodeRegistry::
get_node(int n) const {
  LightMutexHolder holder(_lock);
  nassertr(n >= 0 && n < (int)_entries.size(), NodePath());
  return _entries[n]._node;
}

/**
 * Returns the type of the nth node, as recorded in the registry.
 */
TypeHandle AttribNodeRegistry::
get_node_type(int n) const {
  LightMutexHolder holder(_lock);
  nassertr(n >= 0 && n < (int)_entries.size(), TypeHandle::none());
  return _entries[n]._type;
}

/**
 * Returns the name of the nth node, as recorded in the registry.  This will
 * be the node name as it was at the time the node was recorded; if the node
 * has changed names since then, this will still return the original name.
 */
std::string AttribNodeRegistry::
get_node_name(int n) const {
  LightMutexHolder holder(_lock);
  nassertr(n >= 0 && n < (int)_entries.size(), std::string());
  return _entries[n]._name;
}

/**
 * Returns the index number of the indicated NodePath in the registry
 * (assuming its name hasn't changed since it was recorded in the registry),
 * or -1 if the NodePath cannot be found (for instance, because its name has
 * changed).
 */
int AttribNodeRegistry::
find_node(const NodePath &attrib_node) const {
  nassertr(!attrib_node.is_empty(), -1);
  LightMutexHolder holder(_lock);
  Entries::const_iterator ei = _entries.find(Entry(attrib_node));
  if (ei != _entries.end()) {
    return ei - _entries.begin();
  }
  return -1;
}

/**
 * Returns the index number of the node with the indicated type and name in
 * the registry, or -1 if there is no such node in the registry.
 */
int AttribNodeRegistry::
find_node(TypeHandle type, const std::string &name) const {
  LightMutexHolder holder(_lock);
  Entries::const_iterator ei = _entries.find(Entry(type, name));
  if (ei != _entries.end()) {
    return ei - _entries.begin();
  }
  return -1;
}

/**
 * Removes the nth node from the registry.
 */
void AttribNodeRegistry::
remove_node(int n) {
  LightMutexHolder holder(_lock);
  nassertv(n >= 0 && n < (int)_entries.size());
  _entries.erase(_entries.begin() + n);
}

/**
 * Removes all nodes from the registry.
 */
void AttribNodeRegistry::
clear() {
  LightMutexHolder holder(_lock);
  _entries.clear();
}

/**
 *
 */
void AttribNodeRegistry::
output(std::ostream &out) const {
  LightMutexHolder holder(_lock);

  typedef pmap<TypeHandle, int> Counts;
  Counts counts;

  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    TypeHandle type = (*ei)._type;
    Counts::iterator ci = counts.insert(Counts::value_type(type, 0)).first;
    ++((*ci).second);
  }

  out << _entries.size() << " entries";

  if (!counts.empty()) {
    Counts::iterator ci = counts.begin();
    out << " (" << (*ci).first << ":" << (*ci).second;
    ++ci;
    while (ci != counts.end()) {
      out << ", " << (*ci).first << ":" << (*ci).second;
      ++ci;
    }
    out << ")";
  }
}

/**
 *
 */
void AttribNodeRegistry::
write(std::ostream &out) const {
  LightMutexHolder holder(_lock);

  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    const Entry &entry = (*ei);
    out << entry._type << ", \"" << entry._name << "\": " << entry._node
        << "\n";
  }
}

/**
 *
 */
void AttribNodeRegistry::
make_global_ptr() {
  AttribNodeRegistry *ptr = new AttribNodeRegistry;
  void *result = AtomicAdjust::compare_and_exchange_ptr
    ((void * TVOLATILE &)_global_ptr, nullptr, (void *)ptr);
  if (result != nullptr) {
    // Someone else got there first.
    delete ptr;
  }
  assert(_global_ptr != nullptr);
}

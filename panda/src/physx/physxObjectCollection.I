/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxObjectCollection.I
 * @author enn0x
 * @date 2009-11-08
 */

/**
 *
 */
template <class T>
INLINE unsigned int PhysxObjectCollection<T>::
size() const {

  return _objects.size();
}

/**
 *
 */
template <class T>
INLINE void PhysxObjectCollection<T>::
add(PT(T) object) {

  _objects.push_back(object);
}

/**
 *
 */
template <class T>
INLINE void PhysxObjectCollection<T>::
remove(PT(T) object) {

  typename pvector<PT(T)>::iterator it;

  it = find(_objects.begin(), _objects.end(), object);
  if (it != _objects.end()) {
    _objects.erase(it);
  }
  else
  {
    physx_cat.warning() << "object not found in collection" << std::endl;
  }
}

/**
 * Returns the n-th PhysxObject in the collection.  The operator [] is maybe a
 * more convenient way to access objects from the collection.
 */
template <class T>
INLINE T *PhysxObjectCollection<T>::
get(unsigned int index) const {

  nassertr(index < _objects.size(), nullptr);
  return _objects[index];
}

/**
 * Returns the n-th PhysxObject in the collection.  This is the same as the
 * get() method.
 */
template <class T>
INLINE T *PhysxObjectCollection<T>::
operator [] (unsigned int index) const {

  nassertr(index < _objects.size(), nullptr);
  return _objects[index];
}

/**
 *
 */
template <class T>
INLINE void PhysxObjectCollection<T>::
ls() const {

  ls(nout);
}

/**
 *
 */
template <class T>
INLINE void PhysxObjectCollection<T>::
ls(std::ostream &out, int indent_level) const {

  for (unsigned int i=0; i < size(); i++) {
    get(i)->ls(out, indent_level + 2);
  }
}

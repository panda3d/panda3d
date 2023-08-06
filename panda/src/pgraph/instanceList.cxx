/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file instanceList.cxx
 * @author rdb
 * @date 2019-03-10
 */

#include "instanceList.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "bitArray.h"
#include "geomVertexWriter.h"

TypeHandle InstanceList::_type_handle;

/**
 * Required to implement CopyOnWriteObject.
 */
PT(CopyOnWriteObject) InstanceList::
make_cow_copy() {
  return new InstanceList(*this);
}

/**
 *
 */
InstanceList::
InstanceList() {
}

/**
 *
 */
InstanceList::
InstanceList(const InstanceList &copy) :
  _instances(copy._instances)
{
}

/**
 *
 */
InstanceList::
~InstanceList() {
}

/**
 * Transforms all of the instances in the list by the indicated matrix.
 */
void InstanceList::
xform(const LMatrix4 &mat) {

}

/**
 * Returns an immutable copy without the bits turned on in the indicated mask.
 */
CPT(InstanceList) InstanceList::
without(const BitArray &mask) const {
  size_t num_instances = size();
  size_t num_culled = (size_t)mask.get_num_on_bits();
  if (num_culled == 0) {
    return this;
  }
  else if (num_culled >= num_instances) {
    static CPT(InstanceList) empty_list;
    if (empty_list == nullptr) {
      empty_list = new InstanceList;
    }

    nassertr(num_culled <= num_instances, empty_list);
    return empty_list;
  }

  InstanceList *new_list = new InstanceList;
  new_list->_instances.reserve(num_instances - num_culled);

  for (size_t i = (size_t)mask.get_lowest_off_bit(); i < num_instances; ++i) {
    if (!mask.get_bit(i)) {
      new_list->_instances.push_back(_instances[i]);
    }
  }

  return new_list;
}

/**
 * Returns a GeomVertexArrayData containing the matrices.
 */
CPT(GeomVertexArrayData) InstanceList::
get_array_data(const GeomVertexArrayFormat *format) const {
  CPT(GeomVertexArrayData) array_data = _cached_array;
  if (array_data != nullptr) {
    if (array_data->get_array_format() == format) {
      return array_data;
    }
  }

  nassertr(format != nullptr, nullptr);

  size_t num_instances = size();
  PT(GeomVertexArrayData) new_array = new GeomVertexArrayData(format, GeomEnums::UH_stream);
  new_array->unclean_set_num_rows(num_instances);

  {
    GeomVertexWriter writer(new_array, Thread::get_current_thread());
    writer.set_column(InternalName::get_instance_matrix());
    for (size_t i = 0; i < num_instances; ++i) {
      writer.set_matrix4(_instances[i].get_mat());
    }
  }

  _cached_array = new_array;
  return new_array;
}

/**
 *
 */
void InstanceList::
output(std::ostream &out) const {
  out << "InstanceList[" << size() << "]";
}

/**
 *
 */
void InstanceList::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << "InstanceList[" << size() << "]:\n";
  for (const Instance &instance : *this) {
    indent(out, indent_level + 2) << *instance.get_transform() << "\n";
  }
}

/**
 * Tells the BamReader how to create objects of type InstanceList.
 */
void InstanceList::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void InstanceList::
write_datagram(BamWriter *manager, Datagram &dg) {
  CopyOnWriteObject::write_datagram(manager, dg);

  for (const Instance &instance : *(const InstanceList *)this) {
    manager->write_pointer(dg, instance.get_transform());
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int InstanceList::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CopyOnWriteObject::complete_pointers(p_list, manager);

  for (Instance &instance : *this) {
    instance = Instance(DCAST(TransformState, p_list[pi++]));
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type InstanceList is encountered in the Bam file.  It should create
 * the InstanceList and extract its information from the file.
 */
TypedWritable *InstanceList::
make_from_bam(const FactoryParams &params) {
  InstanceList *object = new InstanceList;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new InstanceList.
 */
void InstanceList::
fillin(DatagramIterator &scan, BamReader *manager) {
  CopyOnWriteObject::fillin(scan, manager);

  size_t num_instances = scan.get_uint16();
  _instances.clear();
  _instances.resize(num_instances);

  for (size_t i = 0; i < num_instances; ++i) {
    manager->read_pointer(scan);
  }

  _cached_array.clear();
}

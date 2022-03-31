/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_bam.cxx
 * @author jason
 * @date 2000-06-13
 */

#include "pandabase.h"
#include "pnotify.h"
#include "dconfig.h"

#include "test_bam.h"

using std::endl;


TypeHandle Person::_type_handle;
TypeHandle Parent::_type_handle;
TypeHandle Child::_type_handle;

Configure(config_test_bam);

ConfigureFn(config_test_bam)
{
  Person::init_type();
  Parent::init_type();
  Child::init_type();

  BamReader::get_factory()->register_factory(Person::get_class_type(), Person::make_person);
  BamReader::get_factory()->register_factory(Parent::get_class_type(), Parent::make_parent);
  BamReader::get_factory()->register_factory(Child::get_class_type(), Child::make_child);
}

void Person::
write_datagram(BamWriter* manager, Datagram &me)
{
  // Write out name
  me.add_string(_name);
  // Write out gender
  me.add_uint8(myGender);
  manager->write_pointer(me, _bro);
  manager->write_pointer(me, _sis);
}

TypedWritable* Person::
make_person(const FactoryParams &params)
{
  Person *me = new Person;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(me, scan, manager);
  return me;
}

void Person::
fillin(Person* me, DatagramIterator& scan, BamReader* manager)
{
  _name = scan.get_string();
  myGender = (Person::sex)scan.get_uint8();
  manager->read_pointer(scan);
  manager->read_pointer(scan);
}

int Person::
complete_pointers(TypedWritable **p_list, BamReader *manager)
{
  int pi = TypedWritable::complete_pointers(p_list, manager);
  _bro = DCAST(Person, p_list[pi++]);
  _sis = DCAST(Person, p_list[pi++]);
  return pi;
}

void Person::
print_relationships(){
  nout << "My name is " << _name << endl;
  if (_bro != nullptr)
    nout << "My brother is " << _bro->name() << endl;
  if (_sis != nullptr)
    nout << "My sister is " << _sis->name() << endl;
}

void Parent::
write_datagram(BamWriter* manager, Datagram &me)
{
  Person::write_datagram(manager, me);
  manager->write_pointer(me, _son);
  manager->write_pointer(me, _daughter);
}

TypedWritable* Parent::
make_parent(const FactoryParams &params)
{
  Parent *me = new Parent;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(me, scan, manager);
  return me;
}

void Parent::
fillin(Parent* me, DatagramIterator& scan, BamReader* manager)
{
  Person::fillin(me, scan, manager);
  manager->read_pointer(scan);
  manager->read_pointer(scan);
}

int Parent::
complete_pointers(TypedWritable **p_list, BamReader *manager)
{
  int pi = Person::complete_pointers(p_list, manager);
  _son = DCAST(Child, p_list[pi++]);
  _daughter = DCAST(Child, p_list[pi++]);
  return pi;
}

void Parent::
setSon(Child* son)
{
  if (son->isMale()) _son = son;
}
void Parent::
setDaughter(Child* daughter)
{
  if (!daughter->isMale()) _daughter = daughter;
}


void Parent::
print_relationships(){
  Person::print_relationships();
  if (_son != nullptr)
    nout << "My son is " << _son->name() << endl;
  if (_daughter != nullptr)
    nout << "My daughter is " << _daughter->name() << endl;
}

void Child::
write_datagram(BamWriter* manager, Datagram &me)
{
  Person::write_datagram(manager, me);
  manager->write_pointer(me, _dad);
  manager->write_pointer(me, _mom);
}

TypedWritable* Child::
make_child(const FactoryParams &params)
{
  Child *me = new Child;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(me, scan, manager);

  return me;
}

void Child::
fillin(Child* me, DatagramIterator& scan, BamReader* manager)
{
  Person::fillin(me, scan, manager);
  manager->read_pointer(scan);
  manager->read_pointer(scan);
}

int Child::
complete_pointers(TypedWritable ** p_list, BamReader *manager)
{
  int pi = Person::complete_pointers(p_list, manager);
  _dad = DCAST(Parent, p_list[pi++]);
  _mom = DCAST(Parent, p_list[pi++]);
  return pi;
}


void Child::
setFather(Parent* dad)
{
  if (dad->isMale()) _dad = dad;
}

void Child::
setMother(Parent* mom)
{
  if (!mom->isMale()) _mom = mom;
}

void Child::
print_relationships(){
  Person::print_relationships();
  if (_dad != nullptr)
      nout << "My dad is " << _dad->name() << endl;
  if (_mom != nullptr)
    nout << "My mom is " << _mom->name() << endl;
}

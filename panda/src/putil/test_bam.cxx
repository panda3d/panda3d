// Filename: test_bam.cxx
// Created by:  jason (13Jun00)
//

#include <pandabase.h>
#include <notify.h>
#include <dconfig.h>

#include "test_bam.h"


TypeHandle Person::_type_handle;
TypeHandle Parent::_type_handle;
TypeHandle Child::_type_handle;

ConfigureFn(config_util)
{
  TypedObject::init_type();
  ReferenceCount::init_type();
  TypedReferenceCount::init_type();
  init_system_type_handles();
  FactoryParam::init_type();
  Datagram::init_type();
  TypedWriteable::init_type();
  WriteableParam::init_type();
  BamReaderParam::init_type();
  TypedWriteableReferenceCount::init_type();

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
  //Write out name
  me.add_string(_name);
  //Write out gender
  me.add_uint8(myGender);
  manager->write_pointer(me, _bro);
  manager->write_pointer(me, _sis);
}

TypedWriteable* Person::
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
  myGender = scan.get_uint8();
  manager->read_pointer(scan, this);
  manager->read_pointer(scan, this);
}

int Person::
complete_pointers(vector_typedWriteable& plist, BamReader *)
{
  _bro = (plist[0] == TypedWriteable::Null) ? (Person*)NULL : DCAST(Person, plist[0]);
  _sis = (plist[1] == TypedWriteable::Null) ? (Person*)NULL : DCAST(Person, plist[1]);
  return 2;
}

void Person::
print_relationships(void){
  nout << "My name is " << _name << endl;
  if (_bro != NULL)
    nout << "My brother is " << _bro->name() << endl;
  if (_sis != NULL)
    nout << "My sister is " << _sis->name() << endl;
}

void Parent::
write_datagram(BamWriter* manager, Datagram &me)
{
  Person::write_datagram(manager, me);
  manager->write_pointer(me, _son);
  manager->write_pointer(me, _daughter);
}

TypedWriteable* Parent::
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
  manager->read_pointer(scan, this);
  manager->read_pointer(scan, this);
}

int Parent::
complete_pointers(vector_typedWriteable& plist, BamReader *manager)
{
  int start = Person::complete_pointers(plist, manager);
  _son = (plist[start] == TypedWriteable::Null) ? (Child*)NULL : DCAST(Child, plist[2]);
  _daughter = (plist[start+1] == TypedWriteable::Null) ? (Child*)NULL : DCAST(Child, plist[3]);
  return start+2;
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
print_relationships(void){
  Person::print_relationships();
  if (_son != NULL)
    nout << "My son is " << _son->name() << endl;
  if (_daughter != NULL)
    nout << "My daughter is " << _daughter->name() << endl;
}

void Child::
write_datagram(BamWriter* manager, Datagram &me)
{
  Person::write_datagram(manager, me);
  manager->write_pointer(me, _dad);
  manager->write_pointer(me, _mom);
}

TypedWriteable* Child::
make_child(const FactoryParams &params)
{
  Child *me = new Child;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(me, scan, manager);
  me->fillin(me, scan, manager);

  return me;
}

void Child::
fillin(Child* me, DatagramIterator& scan, BamReader* manager)
{
  Person::fillin(me, scan, manager);
  manager->read_pointer(scan, this);
  manager->read_pointer(scan, this);
}

int Child::
complete_pointers(vector_typedWriteable& plist, BamReader *manager)
{
  int start = Person::complete_pointers(plist, manager);
  _dad = (plist[start] == TypedWriteable::Null) ? (Parent*)NULL : DCAST(Parent, plist[2]);
  _mom = (plist[start+1] == TypedWriteable::Null) ? (Parent*)NULL : DCAST(Parent, plist[3]);
  return start+2;
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
print_relationships(void){
  Person::print_relationships();
  if (_dad != NULL)
      nout << "My dad is " << _dad->name() << endl;
  if (_mom != NULL)
    nout << "My mom is " << _mom->name() << endl;
}

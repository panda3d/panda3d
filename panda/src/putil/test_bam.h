// Filename: test_bam.h
// Created by:  jason (12Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "pandabase.h"
#include "notify.h"

#include "factory.h"
#include "writableParam.h"
#include "factoryParams.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "pointerTo.h"

#include "typedWritableReferenceCount.h"

#include <ipc_file.h>

class Child;

class Person : public TypedWritableReferenceCount {
public:
  void write_datagram(BamWriter*, Datagram&);

  static TypedWritable *make_person(const FactoryParams &params);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);
protected:
  void fillin(Person*,DatagramIterator&,BamReader*);
public:
  enum sex {
    MALE,
    FEMALE
  };

  void setBrother(Person* bro) {if (bro->isMale()) _bro = bro;}
  void setSister(Person* sis) {if (!sis->isMale()) _sis = sis;}
  bool isMale(void) {return myGender == MALE;}

  void print_relationships(void);
  string name(void) {return _name;}
private:
  Person *_bro, *_sis;
  sex myGender;
  string _name;

public:
  Person(void) {}
  Person(const string &name, const sex Gender) :
     _name(name), myGender(Gender), _bro((Person*)NULL), _sis((Person*)NULL) {

  }
  virtual ~Person() {

  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Person",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

class Parent : public Person {
public:
  void write_datagram(BamWriter*, Datagram&);

  static TypedWritable *make_parent(const FactoryParams &params);
  virtual int complete_pointers(TypedWritable *p_list,
                                BamReader *manager);
protected:
  void fillin(Parent*,DatagramIterator&,BamReader*);
public:
  void setSon(Child*);
  void setDaughter(Child*);

  void print_relationships(void);

private:
  Child *_son, *_daughter;

public:
  Parent(void) {}
  Parent(const string &name, const sex Gender) : Person(name, Gender) {

  }
  virtual ~Parent() {

  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Person::init_type();
    register_type(_type_handle, "Parent",
                  Person::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

class Child : public Person {
public:
  void write_datagram(BamWriter*, Datagram&);

  static TypedWritable *make_child(const FactoryParams &params);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);
protected:
  void fillin(Child*,DatagramIterator&,BamReader*);
public:
  void setFather(Parent*);
  void setMother(Parent*);

  void print_relationships(void);

private:
  Parent *_dad, *_mom;

public:
  Child(void) {}
  Child(const string &name, const sex Gender) : Person(name, Gender) {

  }
  virtual ~Child() {

  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Person::init_type();
    register_type(_type_handle, "Child", Person::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};




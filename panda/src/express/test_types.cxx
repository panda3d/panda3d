/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_types.cxx
 * @author drose
 * @date 1998-10-23
 */

#include "typedObject.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "referenceCount.h"
#include "dcast.h"

#include "pnotify.h"

using std::cerr;
using std::string;

class ThatThingie : public TypedObject, public ReferenceCount {
public:
  ThatThingie(const string &name) : _name(name) {
    nout << "Constructing ThatThingie " << _name << "\n";
  }
  virtual ~ThatThingie() {
    nout << "Destructing ThatThingie " << _name << "\n";
  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "ThatThingie",
                  TypedObject::get_class_type(),
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

  string _name;

private:
  static TypeHandle _type_handle;
};

class ThisThingie : public ThatThingie {
public:
  ThisThingie(const string &name) : ThatThingie(name) {
    nout << "Constructing ThisThingie " << _name << "\n";
  }
  virtual ~ThisThingie() {
    nout << "Destructing ThisThingie " << _name << "\n";
  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ThatThingie::init_type();
    register_type(_type_handle, "ThisThingie", ThatThingie::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

  void const_hello() const {
    nout << "Const hello from " << _name << "\n";
  }

private:
  static TypeHandle _type_handle;
};

class TheOtherThingie {
public:
  TheOtherThingie(const string &name) : _name(name) {
    nout << "Constructing TheOtherThingie " << _name << "\n";
  }
  virtual ~TheOtherThingie() {
    nout << "Destructing TheOtherThingie " << _name << "\n";
  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "TheOtherThingie");
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

  void hello() {
    nout << "Hello from " << _name << "\n";
  }

  string _name;

private:
  static TypeHandle _type_handle;
};

class WhatAThingie : public ThatThingie, public TheOtherThingie {
public:
  WhatAThingie(const string &name) : ThatThingie(name), TheOtherThingie(name) {
    nout << "Constructing WhatAThingie " << ThatThingie::_name << "\n";
  }
  virtual ~WhatAThingie() {
    nout << "Destructing WhatAThingie " << ThatThingie::_name << "\n";
  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ThatThingie::init_type();
    TheOtherThingie::init_type();
    register_type(_type_handle, "WhatAThingie",
                  ThatThingie::get_class_type(),
                  TheOtherThingie::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

TypeHandle ThatThingie::_type_handle;
TypeHandle ThisThingie::_type_handle;
TypeHandle TheOtherThingie::_type_handle;
TypeHandle WhatAThingie::_type_handle;

void show_derivation(TypeHandle type) {
  nout << type << " derives from:";
  for (int i = 0; i < type.get_num_parent_classes(); i++) {
    nout << " " << type.get_parent_class(i);
  }
  nout << "\n";
}

int
main() {
  // This initialization would normally be done by a ConfigureFn block.
  ThatThingie::init_type();
  ThisThingie::init_type();
  TheOtherThingie::init_type();
  WhatAThingie::init_type();

  PointerTo<ThatThingie> thing_1 = new ThisThingie("thing_1");
  ConstPointerTo<ThatThingie> thing_2 = new ThatThingie("thing_2");
  PointerTo<ThatThingie> thing_3 = new WhatAThingie("thing_3");

  nout << "\nthing_1 of type " << thing_1->get_type()
       << "\nthing_2 of type " << thing_2->get_type()
       << "\nthing_3 of type " << thing_3->get_type()

       << "\n\nthing_1 is ThisThingie : "
       << thing_1->is_of_type(ThisThingie::get_class_type())
       << "\nthing_1 is ThatThingie : "
       << thing_1->is_of_type(ThatThingie::get_class_type())
       << "\nthing_1 is TheOtherThingie : "
       << thing_1->is_of_type(TheOtherThingie::get_class_type())
       << "\nthing_1 is WhatAThingie : "
       << thing_1->is_of_type(WhatAThingie::get_class_type())

       << "\n\nthing_2 is ThisThingie : "
       << thing_2->is_of_type(ThisThingie::get_class_type())
       << "\nthing_2 is ThatThingie : "
       << thing_2->is_of_type(ThatThingie::get_class_type())
       << "\nthing_2 is TheOtherThingie : "
       << thing_2->is_of_type(TheOtherThingie::get_class_type())
       << "\nthing_2 is WhatAThingie : "
       << thing_2->is_of_type(WhatAThingie::get_class_type())

       << "\n\nthing_3 is ThisThingie : "
       << thing_3->is_of_type(ThisThingie::get_class_type())
       << "\nthing_3 is ThatThingie : "
       << thing_3->is_of_type(ThatThingie::get_class_type())
       << "\nthing_3 is TheOtherThingie : "
       << thing_3->is_of_type(TheOtherThingie::get_class_type())
       << "\nthing_3 is WhatAThingie : "
       << thing_3->is_of_type(WhatAThingie::get_class_type())
       << "\n\n";

  show_derivation(ThatThingie::get_class_type());
  show_derivation(ThisThingie::get_class_type());
  show_derivation(TheOtherThingie::get_class_type());
  show_derivation(WhatAThingie::get_class_type());

  nout << "WhatAThingie::get_parent_towards(ReferenceCount) = "
       << WhatAThingie::get_class_type().get_parent_towards
    (ReferenceCount::get_class_type()) << "\n"
       << "WhatAThingie::get_parent_towards(ThatThingie) = "
       << WhatAThingie::get_class_type().get_parent_towards
    (ThatThingie::get_class_type()) << "\n"
       << "WhatAThingie::get_parent_towards(ThisThingie) = "
       << WhatAThingie::get_class_type().get_parent_towards
    (ThisThingie::get_class_type()) << "\n"
       << "WhatAThingie::get_parent_towards(TheOtherThingie) = "
       << WhatAThingie::get_class_type().get_parent_towards
    (TheOtherThingie::get_class_type()) << "\n"
       << "WhatAThingie::get_parent_towards(WhatAThingie) = "
       << WhatAThingie::get_class_type().get_parent_towards
    (WhatAThingie::get_class_type()) << "\n"
       << "\n";

  {
    ConstPointerTo<ThatThingie> dup_thing_1 = thing_1;
    nout << "thing_1 is " << thing_1 << " and dup_thing_1 is "
         << dup_thing_1 << "\n";
  }
  nout << "thing_1 is now " << thing_1 << "\n";

  nout << "\n";

  {
    ConstPointerTo<ThatThingie> new_thing = new ThisThingie("new_thing");
    ((const ThisThingie *)new_thing.p())->const_hello();
  }

  nout << "\n";

  ((ThisThingie *)thing_1.p())->const_hello();
  ((WhatAThingie *)thing_3.p())->hello();

  nout << "\n";

  // Testing RefCountProxy.

  RefCountProxy<int> x = 10;
  x++;
  RefCountProxy<int> y;
  y = x;
  nout << "y is " << y << ", y's type is " << y.get_class_type() << "\n";


  // Testing PointerToArray.

  PointerToArray<int> iarray = PointerToArray<int>::empty_array(10);
  memset(iarray, 0, sizeof(int)*10);

  for (int i = 0; i < 10; i++) {
    iarray[i] = i;
  }

  ConstPointerToArray<int> jarray = iarray;

  // jarray[4] = jarray[6];

  nout << "jarray[4] is " << jarray[4] << "\n";

  cerr << "dcast thing_1: " << (void *)thing_1 << "\n";
  ThisThingie *tt1 = DCAST(ThisThingie, thing_1);
  cerr << "gives " << (void *)tt1 << "\n";

  cerr << "dcast thing_2: " << (const void *)thing_2 << "\n";
  const ThisThingie *tt2 = DCAST(ThisThingie, thing_2);
  cerr << "gives " << (const void *)tt2 << "\n";

  return 0;
}

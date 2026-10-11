/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_typehandle.cxx
 * @author drose
 * @date 1998-10-23
 */

#include "typedObject.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "dcast.h"
#include "config_express.h"

#include "catch_amalgamated.hpp"

namespace {
  class ThatThingie : public TypedObject, public ReferenceCount {
  public:
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

  private:
    static TypeHandle _type_handle;
  };

  class ThisThingie : public ThatThingie {
  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      ThatThingie::init_type();
      register_type(_type_handle, "ThisThingie",
                    ThatThingie::get_class_type());
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

  class TheOtherThingie {
  public:
    // A non-empty base so its address genuinely differs from a sibling base,
    // making the multiple-inheritance pointer adjustment observable.
    int _padding = 0;

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
    virtual ~TheOtherThingie() {}

  private:
    static TypeHandle _type_handle;
  };

  class WhatAThingie : public ThatThingie, public TheOtherThingie {
  public:
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

  // register_type() is idempotent, so it is safe to (re)initialize on entry
  // to every test case regardless of ordering.
  void init_types() {
    ThatThingie::init_type();
    ThisThingie::init_type();
    TheOtherThingie::init_type();
    WhatAThingie::init_type();
  }

  // A failed DCAST() deliberately logs an error to the express category.
  // When a test intentionally provokes such a failure, silence that category
  // so the log output does not masquerade as a test failure.
  class QuietExpress {
  public:
    QuietExpress() : _saved(express_cat->get_severity()) {
      express_cat->set_severity(NS_fatal);
    }
    ~QuietExpress() {
      express_cat->set_severity(_saved);
    }
  private:
    NotifySeverity _saved;
  };
}

TEST_CASE("is_of_type reflects the actual runtime type", "[express]") {
  init_types();

  PT(ThatThingie) thing = new ThisThingie;

  // A ThisThingie is-a ThisThingie and is-a ThatThingie, but not the others.
  CHECK(thing->is_of_type(ThisThingie::get_class_type()));
  CHECK(thing->is_of_type(ThatThingie::get_class_type()));
  CHECK_FALSE(thing->is_of_type(TheOtherThingie::get_class_type()));
  CHECK_FALSE(thing->is_of_type(WhatAThingie::get_class_type()));

  // ...and it inherits the registered base types.
  CHECK(thing->is_of_type(ReferenceCount::get_class_type()));
  CHECK(thing->is_of_type(TypedObject::get_class_type()));
}

TEST_CASE("multiple inheritance is reflected in the type graph", "[express]") {
  init_types();

  PT(ThatThingie) thing = new WhatAThingie;

  CHECK(thing->is_of_type(WhatAThingie::get_class_type()));
  CHECK(thing->is_of_type(ThatThingie::get_class_type()));
  CHECK(thing->is_of_type(TheOtherThingie::get_class_type()));
  CHECK(thing->is_of_type(ReferenceCount::get_class_type()));
  // A WhatAThingie is not a ThisThingie: they are siblings under ThatThingie.
  CHECK_FALSE(thing->is_of_type(ThisThingie::get_class_type()));

  TypeHandle what = WhatAThingie::get_class_type();
  REQUIRE(what.get_num_parent_classes() == 2);
  CHECK(what.get_parent_class(0) == ThatThingie::get_class_type());
  CHECK(what.get_parent_class(1) == TheOtherThingie::get_class_type());
}

TEST_CASE("get_parent_towards walks one step toward an ancestor", "[express]") {
  init_types();

  TypeHandle what = WhatAThingie::get_class_type();

  // ReferenceCount is reached through the ThatThingie base.
  CHECK(what.get_parent_towards(ReferenceCount::get_class_type())
        == ThatThingie::get_class_type());
  // ThatThingie is a direct parent, so the step toward it is itself.
  CHECK(what.get_parent_towards(ThatThingie::get_class_type())
        == ThatThingie::get_class_type());
  CHECK(what.get_parent_towards(TheOtherThingie::get_class_type())
        == TheOtherThingie::get_class_type());
}

TEST_CASE("DCAST succeeds down to the real type and fails otherwise", "[express]") {
  init_types();

  PT(ThatThingie) this_thing = new ThisThingie;
  PT(ThatThingie) what_thing = new WhatAThingie;

  // Downcast to the correct dynamic type yields a non-null pointer.
  CHECK(DCAST(ThisThingie, this_thing) != nullptr);
  CHECK(DCAST(WhatAThingie, what_thing) != nullptr);
  // Downcast to an unrelated/sibling type yields null (and logs an error we
  // deliberately silence here).  This check is only meaningful when DCAST
  // verification is compiled in; a release build (DO_DCAST off) performs an
  // unchecked cast that returns a non-null pointer.
#ifdef DO_DCAST
  {
    QuietExpress quiet;
    CHECK(DCAST(WhatAThingie, this_thing) == nullptr);
    CHECK(DCAST(ThisThingie, what_thing) == nullptr);
  }
#endif
}

TEST_CASE("cross-cast across multiple inheritance adjusts the pointer", "[express]") {
  init_types();

  WhatAThingie *w = new WhatAThingie;

  // Upcasting to each base is a compile-time pointer adjustment; because
  // TheOtherThingie is not the first base, its subobject sits at a different
  // address than the ThatThingie subobject.
  ThatThingie *as_that = w;
  TheOtherThingie *as_other = w;
  CHECK((void *)as_that != (void *)as_other);

  // DCAST back to the derived type recovers the original object from a base
  // pointer regardless of which base we came through.
  ThatThingie *recovered = DCAST(WhatAThingie, as_that);
  CHECK(recovered == as_that);
  CHECK(DCAST(WhatAThingie, as_that) == w);

  delete w;
}

TEST_CASE("RefCountProxy behaves like its wrapped value", "[express]") {
  RefCountProxy<int> x = 10;
  ++x;
  CHECK((int)x == 11);

  RefCountProxy<int> y;
  y = x;
  CHECK((int)y == 11);
}

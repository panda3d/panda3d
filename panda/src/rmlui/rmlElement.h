/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlElement.h
 * @author rdb
 * @date 2011-11-30
 */

#ifndef RML_ELEMENT_H
#define RML_ELEMENT_H

#include "config_rmlui.h"
#include "referenceCount.h"
#include "callbackObject.h"
#include "luse.h"  // LVecBase2f, used in the PUBLISHED interface (interrogate-visible)

#ifndef CPPPARSER
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/EventListener.h>
#include <string>
#endif

/**
 * Thin Python-accessible wrapper around Rml::Element.
 *
 * Obtained via RmlDocument::get_element_by_id().
 */
class EXPCL_PANDARMLUI RmlElement : public ReferenceCount {
PUBLISHED:
  std::string get_id() const;
  void set_inner_rml(const std::string &rml);
  std::string get_inner_rml() const;
  void set_attribute(const std::string &name, const std::string &value);
  std::string get_attribute(const std::string &name, const std::string &default_value = std::string()) const;
  void set_class(const std::string &class_name, bool activate);
  bool is_class_set(const std::string &class_name) const;
  void click();
  void focus();

  // value property for form controls (<input>, <select>, <textarea>).
  // Returns an empty string if the element is not a form control.
  std::string get_value() const;
  void set_value(const std::string &value);

  MAKE_PROPERTY(id, get_id);
  MAKE_PROPERTY(inner_rml, get_inner_rml, set_inner_rml);
  MAKE_PROPERTY(value, get_value, set_value);

  // Geometry queries.
  LVecBase2f get_relative_offset() const;
  LVecBase2f get_absolute_offset() const;
  float get_offset_width() const;
  float get_offset_height() const;

  MAKE_PROPERTY(offset_width,  get_offset_width);
  MAKE_PROPERTY(offset_height, get_offset_height);

  // Attribute management.
  bool has_attribute(const std::string &name) const;
  void remove_attribute(const std::string &name);

  // Inline style by property name.
  bool set_property(const std::string &name, const std::string &value);
  void remove_property(const std::string &name);

  // Pseudo-class query.
  bool is_pseudo_class_set(const std::string &pseudo_class) const;

  // Scroll control.
  void scroll_into_view(bool align_with_top = true);

  // CSS selector query.
  PT(RmlElement) query_selector(const std::string &selector);

  // DOM tree traversal.
  PT(RmlElement) get_parent_node() const;
  PT(RmlElement) get_child(int index) const;
  int get_num_children() const;

  MAKE_PROPERTY(num_children, get_num_children);

  // Programmatic DOM manipulation.
  PT(RmlElement) append_child(RmlElement *child);
  void remove_child(RmlElement *child);

  // Attach a callback to a DOM event on this element.  When the event fires,
  // the callback is invoked with an RmlEvent (a CallbackData specialization)
  // exposing the event type and its parameters.  From Python a plain callable
  // may be passed directly; interrogate wraps it in a PythonCallbackObject.
  //
  // The listener is owned by the underlying RmlUi element and survives for as
  // long as the element exists, independent of this wrapper's lifetime.
  // Multiple listeners may be attached by calling this method repeatedly.
  void add_event_listener(const std::string &dom_event, CallbackObject *callback);

public:
  RmlElement() = default;
#ifndef CPPPARSER
  explicit RmlElement(Rml::Element *el)
    : _el(el), _observer(el ? el->GetObserverPtr() : Rml::ObserverPtr<Rml::Element>()) {}
  // A copy is a non-owning alias of the same element: it shares _el and the
  // liveness observer, but never _owned (Rml::ElementPtr is move-only and DOM
  // ownership cannot be shared, so the copy never owns the element).  The
  // interrogate-generated Python bindings require a callable copy constructor.
  RmlElement(const RmlElement &other)
    : _el(other._el), _observer(other._observer) {}
  RmlElement &operator=(const RmlElement &) = delete;

  ~RmlElement();

protected:
  friend class RmlDocument;

  // Returns the underlying element, or nullptr if it has been destroyed (e.g.
  // the document was closed/unloaded or the owning region torn down).  The
  // ObserverPtr auto-nulls when RmlUi destroys the element, so this makes the
  // wrapper's nassert guards detect a dead element instead of dereferencing
  // freed memory.
  Rml::Element *element() const {
    return _observer.get();
  }

  Rml::Element *_el = nullptr;
  // Tracks _el's liveness; expires automatically when RmlUi destroys _el.
  Rml::ObserverPtr<Rml::Element> _observer;
  // Non-null only for freshly-created (not yet DOM-inserted) elements.
  // append_child() moves this into the DOM and clears it.
  Rml::ElementPtr _owned;

  // Bridges an Rml::Event to a Panda CallbackObject.  The listener is owned by
  // the RmlUi element it is attached to (RmlUi never deletes listeners itself;
  // it calls OnDetach when the element is destroyed, at which point the
  // listener deletes itself).  It holds a strong reference to the callback, so
  // its lifetime is fully decoupled from any RmlElement Python wrapper.
  class RmlEventListener : public Rml::EventListener {
  public:
    explicit RmlEventListener(CallbackObject *callback) : _callback(callback) {}
    void ProcessEvent(Rml::Event &event) override;
    void OnDetach(Rml::Element *) override { delete this; }

  private:
    PT(CallbackObject) _callback;
  };
#endif  // !CPPPARSER
};

#endif

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlElement.cxx
 * @author rdb
 * @date 2011-11-30
 */

#include "rmlElement.h"
#include "rmlEvent.h"
#include "pointerTo.h"
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/Event.h>

/**
 * Returns the element's id attribute.
 */
std::string RmlElement::
get_id() const {
  Rml::Element *e = element();
  if (e == nullptr) return std::string();
  return e->GetId();
}

/**
 * Replaces the element's inner content with the given RML markup string.
 */
void RmlElement::
set_inner_rml(const std::string &rml) {
  if (element() == nullptr) return;
  element()->SetInnerRML(rml);
}

/**
 * Returns the element's inner content as an RML markup string.
 */
std::string RmlElement::
get_inner_rml() const {
  if (element() == nullptr) return std::string();
  return element()->GetInnerRML();
}

/**
 * Sets an attribute on the element.
 */
void RmlElement::
set_attribute(const std::string &name, const std::string &value) {
  if (element() == nullptr) return;
  element()->SetAttribute(name, value);
}

/**
 * Returns the value of the named attribute, or default_value if the attribute
 * is not present.
 */
std::string RmlElement::
get_attribute(const std::string &name, const std::string &default_value) const {
  if (element() == nullptr) return default_value;
  return element()->GetAttribute<Rml::String>(name, default_value);
}

/**
 * Adds or removes a CSS class from the element.
 */
void RmlElement::
set_class(const std::string &class_name, bool activate) {
  if (element() == nullptr) return;
  element()->SetClass(class_name, activate);
}

/**
 * Returns true if the element currently has the given CSS class.
 */
bool RmlElement::
is_class_set(const std::string &class_name) const {
  if (element() == nullptr) return false;
  return element()->IsClassSet(class_name);
}

/**
 * Simulates a mouse click on the element.
 */
void RmlElement::
click() {
  if (element() == nullptr) return;
  element()->Click();
}

/**
 * Moves keyboard focus to this element.
 */
void RmlElement::
focus() {
  if (element() == nullptr) return;
  element()->Focus();
}

/**
 * Returns the current value of a form control element (<input>, <select>,
 * <textarea>).  Returns an empty string if the element is not a form control.
 */
std::string RmlElement::
get_value() const {
  if (element() == nullptr) return std::string();
  Rml::ElementFormControl *fc =
    rmlui_dynamic_cast<Rml::ElementFormControl *>(element());
  return fc ? fc->GetValue() : std::string();
}

/**
 * Sets the value of a form control element.  Has no effect if the element is
 * not a form control.
 */
void RmlElement::
set_value(const std::string &value) {
  if (element() == nullptr) return;
  Rml::ElementFormControl *fc =
    rmlui_dynamic_cast<Rml::ElementFormControl *>(element());
  if (fc) fc->SetValue(value);
}

/**
 * Returns the element's offset position relative to its offset parent.
 */
LVecBase2f RmlElement::
get_relative_offset() const {
  if (element() == nullptr) return LVecBase2f(0, 0);
  Rml::Vector2f v = element()->GetRelativeOffset();
  return LVecBase2f(v.x, v.y);
}

/**
 * Returns the element's offset position in document (screen) space.
 */
LVecBase2f RmlElement::
get_absolute_offset() const {
  if (element() == nullptr) return LVecBase2f(0, 0);
  Rml::Vector2f v = element()->GetAbsoluteOffset();
  return LVecBase2f(v.x, v.y);
}

/**
 * Returns the rendered box width of the element in dp.
 */
float RmlElement::
get_offset_width() const {
  if (element() == nullptr) return 0.0f;
  return element()->GetOffsetWidth();
}

/**
 * Returns the rendered box height of the element in dp.
 */
float RmlElement::
get_offset_height() const {
  if (element() == nullptr) return 0.0f;
  return element()->GetOffsetHeight();
}

/**
 * Returns true if the element has the named attribute.
 */
bool RmlElement::
has_attribute(const std::string &name) const {
  if (element() == nullptr) return false;
  return element()->HasAttribute(name);
}

/**
 * Removes the named attribute from the element.
 */
void RmlElement::
remove_attribute(const std::string &name) {
  if (element() == nullptr) return;
  element()->RemoveAttribute(name);
}

/**
 * Sets an inline CSS property on the element.  Returns false on parse error.
 */
bool RmlElement::
set_property(const std::string &name, const std::string &value) {
  if (element() == nullptr) return false;
  return element()->SetProperty(name, value);
}

/**
 * Removes an inline CSS property from the element.
 */
void RmlElement::
remove_property(const std::string &name) {
  if (element() == nullptr) return;
  element()->RemoveProperty(name);
}

/**
 * Returns true if the given pseudo-class (e.g. "hover", "active") is set.
 */
bool RmlElement::
is_pseudo_class_set(const std::string &pseudo_class) const {
  if (element() == nullptr) return false;
  return element()->IsPseudoClassSet(pseudo_class);
}

/**
 * Scrolls the element into the viewport.  If align_with_top is true the
 * element is aligned with the top of the scroll container.
 */
void RmlElement::
scroll_into_view(bool align_with_top) {
  if (element() == nullptr) return;
  element()->ScrollIntoView(align_with_top);
}

/**
 * Returns the first element matching the CSS selector, or nullptr.
 */
PT(RmlElement) RmlElement::
query_selector(const std::string &selector) {
  if (element() == nullptr) return nullptr;
  Rml::Element *result = element()->QuerySelector(selector);
  return result ? new RmlElement(result) : nullptr;
}

/**
 * Returns the parent element, or nullptr if this is the root.
 */
PT(RmlElement) RmlElement::
get_parent_node() const {
  if (element() == nullptr) return nullptr;
  Rml::Element *p = element()->GetParentNode();
  return p ? new RmlElement(p) : nullptr;
}

/**
 * Returns the child element at index, or nullptr if out of range.
 */
PT(RmlElement) RmlElement::
get_child(int index) const {
  if (element() == nullptr) return nullptr;
  Rml::Element *c = element()->GetChild(index);
  return c ? new RmlElement(c) : nullptr;
}

/**
 * Returns the number of direct child elements.
 */
int RmlElement::
get_num_children() const {
  if (element() == nullptr) return 0;
  return element()->GetNumChildren();
}

/**
 * Appends child to this element's DOM subtree.  child must be a freshly-
 * created element (via RmlDocument.create_element) whose _owned pointer is
 * still set; calling this on an already-inserted element is unsupported.
 * Returns a non-owning wrapper to the appended element (now DOM-owned).
 */
PT(RmlElement) RmlElement::
append_child(RmlElement *child) {
  if (element() == nullptr) return nullptr;
  nassertr(child != nullptr && child->element() != nullptr, nullptr);
  nassertr(child->_owned != nullptr, nullptr);
  Rml::Element *appended = element()->AppendChild(std::move(child->_owned));
  child->_owned = nullptr;
  return appended ? new RmlElement(appended) : nullptr;
}

/**
 * Removes child from this element's DOM subtree and frees it.
 * The child wrapper must not be used after this call.
 */
void RmlElement::
remove_child(RmlElement *child) {
  if (element() == nullptr) return;
  nassertv(child != nullptr && child->element() != nullptr);
  // RmlUi fires OnDetach on any attached listeners as the element is destroyed,
  // which deletes them; we do not track listeners on the wrapper.
  element()->RemoveChild(child->element());
  child->_el = nullptr;
  child->_observer.reset();
}

/**
 * Attaches a callback to a DOM event on this element.  See the header for the
 * lifetime contract: the listener is owned by the RmlUi element, not by this
 * wrapper.
 */
void RmlElement::
add_event_listener(const std::string &dom_event, CallbackObject *callback) {
  if (element() == nullptr) return;
  nassertv(callback != nullptr);

  // RmlUi takes a non-owning EventListener pointer and calls OnDetach (which
  // deletes the listener) when the element is destroyed.  The listener holds a
  // strong reference to the callback.
  RmlEventListener *listener = new RmlEventListener(callback);
  element()->AddEventListener(dom_event, listener);
}

/**
 *
 */
RmlElement::
~RmlElement() {
  _el = nullptr;
}

/**
 * Wraps the Rml::Event in an RmlEvent and dispatches it to the CallbackObject.
 * The RmlEvent (like every Panda CallbackData) is a transient stack object only
 * valid for the duration of the callback; see the header.
 */
void RmlElement::RmlEventListener::
ProcessEvent(Rml::Event &event) {
  if (_callback != nullptr) {
    RmlEvent data(&event);
    _callback->do_callback(&data);
  }
}

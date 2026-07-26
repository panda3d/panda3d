/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlDocument.cxx
 * @author rdb
 * @date 2011-11-30
 */

#include "rmlDocument.h"
#include "rmlElement.h"
#include <RmlUi/Core/ElementDocument.h>

/**
 * Makes the document visible and gives it focus.
 */
void RmlDocument::
show() {
  if (get_raw() == nullptr) return;
  get_raw()->Show();
}

/**
 * Hides the document.
 */
void RmlDocument::
hide() {
  if (get_raw() == nullptr) return;
  get_raw()->Hide();
}

/**
 * Closes and destroys the document.  The underlying document is freed on
 * the next Context::Update(); this wrapper nulls _doc immediately so that
 * any subsequent method call on the wrapper safely returns without crashing.
 */
void RmlDocument::
close() {
  if (get_raw() == nullptr) {
    return;
  }
  // NOTE: Any RmlElement wrappers obtained from this document become invalid
  // after the next RmlContext::update() call.  Do not use stored RmlElement
  // references after calling close().
  get_raw()->Close();
  _doc = nullptr;
  _observer.reset();
}

/**
 * Returns the element with the given id attribute, or nullptr if not found.
 */
PT(RmlElement) RmlDocument::
get_element_by_id(const std::string &id) {
  if (get_raw() == nullptr) return nullptr;
  Rml::Element *el = get_raw()->GetElementById(id);
  if (el == nullptr) {
    return nullptr;
  }
  return new RmlElement(el);
}

/**
 * Returns the document's title string (the contents of the <title> element).
 */
std::string RmlDocument::
get_title() const {
  if (get_raw() == nullptr) return std::string();
  return get_raw()->GetTitle();
}

/**
 * Sets the document's title string.
 */
void RmlDocument::
set_title(const std::string &title) {
  if (get_raw() == nullptr) return;
  get_raw()->SetTitle(title);
}

/**
 * Returns the URL the document was loaded from, or "[from memory]" for
 * in-memory documents.
 */
std::string RmlDocument::
get_source_url() const {
  if (get_raw() == nullptr) return std::string();
  return get_raw()->GetSourceURL();
}

/**
 * Returns true if the document is currently shown as a modal dialog.
 */
bool RmlDocument::
is_modal() const {
  if (get_raw() == nullptr) return false;
  return get_raw()->IsModal();
}

/**
 * Pulls the document to the front of the context's document stack.
 */
void RmlDocument::
pull_to_front() {
  if (get_raw() == nullptr) return;
  get_raw()->PullToFront();
}

/**
 * Pushes the document to the back of the context's document stack.
 */
void RmlDocument::
push_to_back() {
  if (get_raw() == nullptr) return;
  get_raw()->PushToBack();
}

/**
 * Reloads all style sheets referenced by this document.
 */
void RmlDocument::
reload_style_sheet() {
  if (get_raw() == nullptr) return;
  get_raw()->ReloadStyleSheet();
}

/**
 * Creates a new element with the given tag name.  The element is not yet
 * part of the DOM; attach it with RmlElement.append_child().
 */
PT(RmlElement) RmlDocument::
create_element(const std::string &tag) {
  if (get_raw() == nullptr) return nullptr;
  Rml::ElementPtr ptr = get_raw()->CreateElement(tag);
  if (!ptr) {
    return nullptr;
  }
  Rml::Element *raw = ptr.get();
  PT(RmlElement) wrapper = new RmlElement(raw);
  wrapper->_owned = std::move(ptr);
  return wrapper;
}

/**
 * Creates a new text node with the given content.  Attach it with
 * RmlElement.append_child().
 */
PT(RmlElement) RmlDocument::
create_text_node(const std::string &text) {
  if (get_raw() == nullptr) return nullptr;
  Rml::ElementPtr ptr = get_raw()->CreateTextNode(text);
  if (!ptr) {
    return nullptr;
  }
  Rml::Element *raw = ptr.get();
  PT(RmlElement) wrapper = new RmlElement(raw);
  wrapper->_owned = std::move(ptr);
  return wrapper;
}

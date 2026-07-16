/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlDocument.h
 * @author rdb
 * @date 2011-11-30
 */

#ifndef RML_DOCUMENT_H
#define RML_DOCUMENT_H

#include "config_rmlui.h"
#include "referenceCount.h"
#include "pointerTo.h"

class RmlElement;

#ifndef CPPPARSER
#include <RmlUi/Core/ElementDocument.h>
#endif

/**
 * Thin Python-accessible wrapper around Rml::ElementDocument.
 *
 * Obtained via RmlContext::load_document().  The document is owned by
 * RmlUi; this wrapper holds a non-owning pointer.
 */
class EXPCL_PANDARMLUI RmlDocument : public ReferenceCount {
PUBLISHED:
  void show();
  void hide();
  void close();

  PT(RmlElement) get_element_by_id(const std::string &id);

  std::string get_title() const;
  void set_title(const std::string &title);
  MAKE_PROPERTY(title, get_title, set_title);

  // Additional metadata.
  std::string get_source_url() const;
  bool is_modal() const;

  MAKE_PROPERTY(source_url, get_source_url);
  MAKE_PROPERTY(modal,      is_modal);

  // Z-ordering.
  void pull_to_front();
  void push_to_back();

  // Style hot-reload.
  void reload_style_sheet();

  // Programmatic element creation.
  PT(RmlElement) create_element(const std::string &tag);
  PT(RmlElement) create_text_node(const std::string &text);

public:
  RmlDocument() = default;
#ifndef CPPPARSER
  explicit RmlDocument(Rml::ElementDocument *doc)
    : _doc(doc),
      _observer(doc ? doc->GetObserverPtr() : Rml::ObserverPtr<Rml::Element>()) {}
  // Returns the live document, or nullptr if it has been destroyed (closed,
  // unloaded, or the owning region torn down) — the ObserverPtr auto-nulls when
  // RmlUi destroys the document, so callers never dereference freed memory.
  Rml::ElementDocument *get_raw() const { return _observer ? _doc : nullptr; }
private:
  Rml::ElementDocument *_doc = nullptr;
  // Tracks _doc's liveness; expires automatically when RmlUi destroys it.
  Rml::ObserverPtr<Rml::Element> _observer;
#endif
};

#endif

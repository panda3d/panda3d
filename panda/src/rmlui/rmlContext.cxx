/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlContext.cxx
 * @author rdb
 * @date 2011-11-30
 */

#include "rmlContext.h"
#include "rmlDocument.h"
#include "rmlElement.h"
#include "rmlDataModel.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>

/**
 * Loads an RML document from the given path and returns a Python-accessible
 * wrapper.  Returns nullptr on failure.
 */
PT(RmlDocument) RmlContext::
load_document(const std::string &path) {
  if (_ctx == nullptr) return nullptr;
  Rml::ElementDocument *doc = _ctx->LoadDocument(path);
  if (doc == nullptr) {
    return nullptr;
  }
  return new RmlDocument(doc);
}

/**
 * Loads a font face from the given file path.  If fallback is true, the face
 * will be used as a fallback for characters not found in other fonts.
 */
bool RmlContext::
load_font_face(const std::string &path, bool fallback) {
  if (_ctx == nullptr) return false;
  return Rml::LoadFontFace(path, fallback);
}

/**
 * Updates the RmlUi context, processing animations and layout.  This is
 * called automatically by RmlRegion::do_cull; only call manually when not
 * using RmlRegion.
 */
void RmlContext::
update() {
  if (_ctx == nullptr) return;
  _ctx->Update();
}

/**
 * Returns the width of the context in pixels.
 */
int RmlContext::
get_width() const {
  if (_ctx == nullptr) return 0;
  return _ctx->GetDimensions().x;
}

/**
 * Returns the height of the context in pixels.
 */
int RmlContext::
get_height() const {
  if (_ctx == nullptr) return 0;
  return _ctx->GetDimensions().y;
}

/**
 * Returns the name this context was created with.
 */
std::string RmlContext::
get_name() const {
  if (_ctx == nullptr) return std::string();
  return _ctx->GetName();
}

/**
 * Creates a new named data model and returns a handle that can be used to
 * bind Python variables to it.  Returns nullptr if a model with that name
 * already exists.
 *
 * Elements reference the model with the attribute data-model="name".
 */
PT(RmlDataModel) RmlContext::
create_data_model(const std::string &name) {
  if (_ctx == nullptr) return nullptr;
  Rml::DataModelConstructor constructor = _ctx->CreateDataModel(name);
  if (!constructor) {
    return nullptr;
  }
  Rml::DataModelHandle handle = constructor.GetModelHandle();
  PT(RmlDataModel) model = new RmlDataModel(handle, constructor);
  // Retain the wrapper for the context's lifetime: bind_list stores its
  // VariableDefinition inside the wrapper while RmlUi keeps only a raw pointer.
  _data_models[name] = model;
  return model;
}

/**
 * Returns a handle to an existing data model, or nullptr if not found.
 * The returned handle can be used to bind additional variables or to mark
 * variables dirty.
 */
PT(RmlDataModel) RmlContext::
get_data_model(const std::string &name) {
  if (_ctx == nullptr) return nullptr;
  // Return the cached wrapper if we vended one, so that bindings made through
  // it (and the VariableDefinitions they own) stay alive and a caller that
  // re-fetches the model sees the same handle.
  auto it = _data_models.find(name);
  if (it != _data_models.end()) {
    return it->second;
  }
  Rml::DataModelConstructor constructor = _ctx->GetDataModel(name);
  if (!constructor) {
    return nullptr;
  }
  Rml::DataModelHandle handle = constructor.GetModelHandle();
  PT(RmlDataModel) model = new RmlDataModel(handle, constructor);
  _data_models[name] = model;
  return model;
}

/**
 * Invalidates and drops every cached RmlDataModel wrapper.  See the header.
 */
void RmlContext::
_invalidate_data_models() {
  for (auto &entry : _data_models) {
    if (entry.second != nullptr) {
      entry.second->_invalidate();
    }
  }
  _data_models.clear();
}

/**
 * Removes the named data model.  All data views, controllers, and bindings
 * it contains are also removed.  Invalidates any existing RmlDataModel
 * handles pointing to the model.
 *
 * Returns true if the model was found and removed, false otherwise.
 */
bool RmlContext::
remove_data_model(const std::string &name) {
  if (_ctx == nullptr) return false;
  bool removed = _ctx->RemoveDataModel(name);
  if (removed) {
    // Invalidate and drop the cached wrapper: the RmlUi model and its bindings
    // are gone, so any retained RmlDataModel handle must stop being used.
    auto it = _data_models.find(name);
    if (it != _data_models.end()) {
      it->second->_invalidate();
      _data_models.erase(it);
    }
  }
  return removed;
}

/**
 * Returns true if the mouse cursor is currently over or interacting with any
 * element in this context.  Use to suppress game input while the UI is active.
 */
bool RmlContext::
is_mouse_interacting() const {
  if (_ctx == nullptr) return false;
  return _ctx->IsMouseInteracting();
}

/**
 * Returns the topmost element under the given screen-space point, or nullptr.
 * NOTE: The returned wrapper is non-owning.  Do not store it across a
 * ctx.update() call that may unload documents.
 */
PT(RmlElement) RmlContext::
get_element_at_point(float x, float y) const {
  if (_ctx == nullptr) return nullptr;
  Rml::Element *el = _ctx->GetElementAtPoint({x, y});
  return el ? new RmlElement(el) : nullptr;
}

/**
 * Returns the element currently under the mouse cursor, or nullptr.
 * NOTE: The returned wrapper is non-owning.  Do not store it across a
 * ctx.update() call that may unload documents.
 */
PT(RmlElement) RmlContext::
get_hover_element() const {
  if (_ctx == nullptr) return nullptr;
  Rml::Element *el = _ctx->GetHoverElement();
  return el ? new RmlElement(el) : nullptr;
}

/**
 * Returns the element that currently holds keyboard focus, or nullptr.
 * NOTE: The returned wrapper is non-owning.  Do not store it across a
 * ctx.update() call that may unload documents.
 */
PT(RmlElement) RmlContext::
get_focus_element() const {
  if (_ctx == nullptr) return nullptr;
  Rml::Element *el = _ctx->GetFocusElement();
  return el ? new RmlElement(el) : nullptr;
}

/**
 * Loads an RML document from an in-memory string.  source_url is used for
 * error reporting only; defaults to "[from memory]" if empty.
 */
PT(RmlDocument) RmlContext::
load_document_from_memory(const std::string &rml, const std::string &source_url) {
  if (_ctx == nullptr) return nullptr;
  Rml::ElementDocument *doc = _ctx->LoadDocumentFromMemory(
      rml, source_url.empty() ? "[from memory]" : source_url);
  return doc ? new RmlDocument(doc) : nullptr;
}

/**
 * Unloads a single document from this context.
 */
void RmlContext::
unload_document(RmlDocument *doc) {
  if (_ctx == nullptr || doc == nullptr) return;
  Rml::ElementDocument *raw = doc->get_raw();
  if (raw == nullptr) return;  // already destroyed
  _ctx->UnloadDocument(raw);
}

/**
 * Unloads all documents from this context.
 */
void RmlContext::
unload_all_documents() {
  if (_ctx == nullptr) return;
  _ctx->UnloadAllDocuments();
}

/**
 * Returns the number of documents currently loaded in this context.
 */
int RmlContext::
get_num_documents() const {
  if (_ctx == nullptr) return 0;
  return _ctx->GetNumDocuments();
}

/**
 * Sets the density-independent pixel ratio for HiDPI scaling.
 */
void RmlContext::
set_density_independent_pixel_ratio(float ratio) {
  if (_ctx == nullptr) return;
  _ctx->SetDensityIndependentPixelRatio(ratio);
}

/**
 * Enables or disables the built-in RmlUi software mouse cursor.
 */
void RmlContext::
enable_mouse_cursor(bool enable) {
  if (_ctx == nullptr) return;
  _ctx->EnableMouseCursor(enable);
}

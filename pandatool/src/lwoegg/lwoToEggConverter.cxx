/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoToEggConverter.cxx
 * @author drose
 * @date 2001-04-25
 */

#include "lwoToEggConverter.h"
#include "cLwoLayer.h"
#include "cLwoClip.h"
#include "cLwoPoints.h"
#include "cLwoPolygons.h"
#include "cLwoSurface.h"

#include "eggData.h"
#include "lwoHeader.h"
#include "lwoLayer.h"
#include "lwoClip.h"
#include "lwoPoints.h"
#include "lwoPolygons.h"
#include "lwoVertexMap.h"
#include "lwoDiscontinuousVertexMap.h"
#include "lwoTags.h"
#include "lwoPolygonTags.h"
#include "lwoInputFile.h"
#include "dcast.h"


/**
 *
 */
LwoToEggConverter::
LwoToEggConverter() {
  _generic_layer = nullptr;
  _make_materials = true;
}

/**
 *
 */
LwoToEggConverter::
LwoToEggConverter(const LwoToEggConverter &copy) :
  SomethingToEggConverter(copy)
{
}

/**
 *
 */
LwoToEggConverter::
~LwoToEggConverter() {
  cleanup();
}

/**
 * Allocates and returns a new copy of the converter.
 */
SomethingToEggConverter *LwoToEggConverter::
make_copy() {
  return new LwoToEggConverter(*this);
}

/**
 * Returns the English name of the file type this converter supports.
 */
std::string LwoToEggConverter::
get_name() const {
  return "Lightwave";
}

/**
 * Returns the common extension of the file type this converter supports.
 */
std::string LwoToEggConverter::
get_extension() const {
  return "lwo";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz extension), false otherwise.
 */
bool LwoToEggConverter::
supports_compressed() const {
  return true;
}

/**
 * Handles the reading of the input file and converting it to egg.  Returns
 * true if successful, false otherwise.
 *
 * This is designed to be as generic as possible, generally in support of run-
 * time loading.  Command-line converters may choose to use convert_lwo()
 * instead, as it provides more control.
 */
bool LwoToEggConverter::
convert_file(const Filename &filename) {
  LwoInputFile in;

  nout << "Reading " << filename << "\n";
  if (!in.open_read(filename)) {
    nout << "Unable to open " << filename << "\n";
    return false;
  }

  PT(IffChunk) chunk = in.get_chunk();
  if (chunk == nullptr) {
    nout << "Unable to read " << filename << "\n";
    return false;
  }

  if (!chunk->is_of_type(LwoHeader::get_class_type())) {
    nout << "File " << filename << " is not a Lightwave Object file.\n";
    return false;
  }

  LwoHeader *header = DCAST(LwoHeader, chunk);
  if (!header->is_valid()) {
    nout << "File " << filename
         << " is not recognized as a Lightwave Object file.  "
         << "Perhaps the version is too recent.\n";
    return false;
  }

  return convert_lwo(header);
}

/**
 * Fills up the egg_data structure according to the indicated lwo structure.
 */
bool LwoToEggConverter::
convert_lwo(const LwoHeader *lwo_header) {
  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_yup_left);
  }

  _error = false;
  _lwo_header = lwo_header;

  collect_lwo();
  make_egg();
  connect_egg();

  _egg_data->remove_unused_vertices(true);
  cleanup();

  return !had_error();
}

/**
 * Returns a pointer to the layer with the given index number, or NULL if
 * there is no such layer.
 */
CLwoLayer *LwoToEggConverter::
get_layer(int number) const {
  if (number >= 0 && number < (int)_layers.size()) {
    return _layers[number];
  }
  return nullptr;
}

/**
 * Returns a pointer to the clip with the given index number, or NULL if there
 * is no such clip.
 */
CLwoClip *LwoToEggConverter::
get_clip(int number) const {
  if (number >= 0 && number < (int)_clips.size()) {
    return _clips[number];
  }
  return nullptr;
}

/**
 * Returns a pointer to the surface definition with the given name, or NULL if
 * there is no such surface.
 */
CLwoSurface *LwoToEggConverter::
get_surface(const std::string &name) const {
  Surfaces::const_iterator si;
  si = _surfaces.find(name);
  if (si != _surfaces.end()) {
    return (*si).second;
  }
  return nullptr;
}

/**
 * Frees all the internal data structures after we're done converting, and
 * resets the converter to its initial state.
 */
void LwoToEggConverter::
cleanup() {
  _lwo_header.clear();

  if (_generic_layer != nullptr) {
    delete _generic_layer;
    _generic_layer = nullptr;
  }

  Layers::iterator li;
  for (li = _layers.begin(); li != _layers.end(); ++li) {
    CLwoLayer *layer = (*li);
    if (layer != nullptr) {
      delete layer;
    }
  }
  _layers.clear();

  Clips::iterator ci;
  for (ci = _clips.begin(); ci != _clips.end(); ++ci) {
    CLwoClip *clip = (*ci);
    if (clip != nullptr) {
      delete clip;
    }
  }
  _clips.clear();

  Points::iterator pi;
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    CLwoPoints *points = (*pi);
    delete points;
  }
  _points.clear();

  Polygons::iterator gi;
  for (gi = _polygons.begin(); gi != _polygons.end(); ++gi) {
    CLwoPolygons *polygons = (*gi);
    delete polygons;
  }
  _polygons.clear();

  Surfaces::iterator si;
  for (si = _surfaces.begin(); si != _surfaces.end(); ++si) {
    CLwoSurface *surface = (*si).second;
    delete surface;
  }
  _surfaces.clear();
}

/**
 * Walks through the chunks in the Lightwave data and creates wrapper objects
 * for each relevant piece.
 */
void LwoToEggConverter::
collect_lwo() {
  CLwoLayer *last_layer = nullptr;
  CLwoPoints *last_points = nullptr;
  CLwoPolygons *last_polygons = nullptr;

  const LwoTags *tags = nullptr;

  int num_chunks = _lwo_header->get_num_chunks();
  for (int i = 0; i < num_chunks; i++) {
    const IffChunk *chunk = _lwo_header->get_chunk(i);

    if (chunk->is_of_type(LwoLayer::get_class_type())) {
      const LwoLayer *lwo_layer = DCAST(LwoLayer, chunk);
      CLwoLayer *layer = new CLwoLayer(this, lwo_layer);
      int number = layer->get_number();
      slot_layer(number);

      if (_layers[number] != nullptr) {
        nout << "Warning: multiple layers with number " << number << "\n";
      }
      _layers[number] = layer;
      last_layer = layer;
      last_points = nullptr;
      last_polygons = nullptr;

    } else if (chunk->is_of_type(LwoClip::get_class_type())) {
      const LwoClip *lwo_clip = DCAST(LwoClip, chunk);
      CLwoClip *clip = new CLwoClip(this, lwo_clip);

      int index = clip->get_index();
      slot_clip(index);

      if (_clips[index] != nullptr) {
        nout << "Warning: multiple clips with index " << index << "\n";
      }
      _clips[index] = clip;

    } else if (chunk->is_of_type(LwoPoints::get_class_type())) {
      if (last_layer == nullptr) {
        last_layer = make_generic_layer();
      }

      const LwoPoints *lwo_points = DCAST(LwoPoints, chunk);
      CLwoPoints *points = new CLwoPoints(this, lwo_points, last_layer);
      _points.push_back(points);
      last_points = points;

    } else if (chunk->is_of_type(LwoVertexMap::get_class_type())) {
      if (last_points == nullptr) {
        nout << "Vertex map chunk encountered without a preceding points chunk.\n";
      } else {
        const LwoVertexMap *lwo_vmap = DCAST(LwoVertexMap, chunk);
        last_points->add_vmap(lwo_vmap);
      }

    } else if (chunk->is_of_type(LwoDiscontinuousVertexMap::get_class_type())) {
      if (last_polygons == nullptr) {
        nout << "Discontinous vertex map chunk encountered without a preceding polygons chunk.\n";
      } else {
        const LwoDiscontinuousVertexMap *lwo_vmad = DCAST(LwoDiscontinuousVertexMap, chunk);
        last_polygons->add_vmad(lwo_vmad);
      }

    } else if (chunk->is_of_type(LwoTags::get_class_type())) {
      tags = DCAST(LwoTags, chunk);

    } else if (chunk->is_of_type(LwoPolygons::get_class_type())) {
      if (last_points == nullptr) {
        nout << "Polygon chunk encountered without a preceding points chunk.\n";
      } else {
        const LwoPolygons *lwo_polygons = DCAST(LwoPolygons, chunk);
        CLwoPolygons *polygons =
          new CLwoPolygons(this, lwo_polygons, last_points);
        _polygons.push_back(polygons);
        last_polygons = polygons;
      }

    } else if (chunk->is_of_type(LwoPolygonTags::get_class_type())) {
      if (last_polygons == nullptr) {
        nout << "Polygon tags chunk encountered without a preceding polygons chunk.\n";
      } else if (tags == nullptr) {
        nout << "Polygon tags chunk encountered without a preceding tags chunk.\n";
      } else {
        const LwoPolygonTags *lwo_ptags = DCAST(LwoPolygonTags, chunk);
        last_polygons->add_ptags(lwo_ptags, tags);
      }

    } else if (chunk->is_of_type(LwoSurface::get_class_type())) {
      if (last_layer == nullptr) {
        last_layer = make_generic_layer();
      }

      const LwoSurface *lwo_surface = DCAST(LwoSurface, chunk);
      CLwoSurface *surface = new CLwoSurface(this, lwo_surface);

      bool inserted = _surfaces.insert(Surfaces::value_type(surface->get_name(), surface)).second;
      if (!inserted) {
        nout << "Multiple surface definitions named " << surface->get_name() << "\n";
        delete surface;
      }
    }
  }
}

/**
 * Makes egg structures for all of the conversion wrapper objects.
 */
void LwoToEggConverter::
make_egg() {
  if (_generic_layer != nullptr) {
    _generic_layer->make_egg();
  }

  Layers::iterator li;
  for (li = _layers.begin(); li != _layers.end(); ++li) {
    CLwoLayer *layer = (*li);
    if (layer != nullptr) {
      layer->make_egg();
    }
  }

  Points::iterator pi;
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    CLwoPoints *points = (*pi);
    points->make_egg();
  }

  Polygons::iterator gi;
  for (gi = _polygons.begin(); gi != _polygons.end(); ++gi) {
    CLwoPolygons *polygons = (*gi);
    polygons->make_egg();
  }
}

/**
 * Connects together all of the egg structures.
 */
void LwoToEggConverter::
connect_egg() {
  if (_generic_layer != nullptr) {
    _generic_layer->connect_egg();
  }

  Layers::iterator li;
  for (li = _layers.begin(); li != _layers.end(); ++li) {
    CLwoLayer *layer = (*li);
    if (layer != nullptr) {
      layer->connect_egg();
    }
  }

  Points::iterator pi;
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    CLwoPoints *points = (*pi);
    points->connect_egg();
  }

  Polygons::iterator gi;
  for (gi = _polygons.begin(); gi != _polygons.end(); ++gi) {
    CLwoPolygons *polygons = (*gi);
    polygons->connect_egg();
  }
}

/**
 * Ensures that there is space in the _layers array to store an element at
 * position number.
 */
void LwoToEggConverter::
slot_layer(int number) {
  nassertv(number - (int)_layers.size() < 1000);
  while (number >= (int)_layers.size()) {
    _layers.push_back(nullptr);
  }
  nassertv(number >= 0 && number < (int)_layers.size());
}

/**
 * Ensures that there is space in the _clips array to store an element at
 * position number.
 */
void LwoToEggConverter::
slot_clip(int number) {
  nassertv(number - (int)_clips.size() < 1000);
  while (number >= (int)_clips.size()) {
    _clips.push_back(nullptr);
  }
  nassertv(number >= 0 && number < (int)_clips.size());
}

/**
 * If a geometry definition is encountered in the Lightwave file before a
 * layer definition, we should make a generic layer to hold the geometry.
 * This makes and returns a single layer for this purpose.  It should not be
 * called twice.
 */
CLwoLayer *LwoToEggConverter::
make_generic_layer() {
  nassertr(_generic_layer == nullptr, _generic_layer);

  PT(LwoLayer) layer = new LwoLayer;
  layer->make_generic();

  _generic_layer = new CLwoLayer(this, layer);
  return _generic_layer;
}

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinManager.cxx
 * @author drose
 * @date 2002-02-28
 */

#include "cullBinManager.h"
#include "renderState.h"
#include "cullResult.h"
#include "config_pgraph.h"
#include "string_utils.h"
#include "configVariableColor.h"

using std::string;

CullBinManager *CullBinManager::_global_ptr = nullptr;

/**
 * The constructor is not intended to be called directly; there is only one
 * CullBinManager and it constructs itself.  This could have been a private
 * constructor, but gcc issues a spurious warning if the constructor is
 * private and the class has no friends.
 */
CullBinManager::
CullBinManager() {
  _bins_are_sorted = true;
  _unused_bin_index = false;

  setup_initial_bins();
}

/**
 * Don't call the destructor.
 */
CullBinManager::
~CullBinManager() {
}

/**
 * Defines a new bin with the indicated name, and returns the new bin_index.
 * If there is already a bin with the same name returns its bin_index if it
 * had the same properties; otherwise, reports an error and returns -1.
 */
int CullBinManager::
add_bin(const string &name, BinType type, int sort) {
  BinsByName::const_iterator bni = _bins_by_name.find(name);
  if (bni != _bins_by_name.end()) {
    // We already have such a bin.  This is not a problem if the bin has the
    // same properties.
    int bin_index = (*bni).second;
    nassertr(bin_index >= 0 && bin_index < (int)_bin_definitions.size(), -1);
    const BinDefinition &def = _bin_definitions[bin_index];
    nassertr(def._in_use, -1);
    if (def._type == type && def._sort == sort) {
      return bin_index;
    }

    // Nope, the old bin had different properties.  Too bad.
    pgraph_cat.warning()
      << "Cannot create a bin named " << name
      << "; already have a bin by that name.\n";
    return -1;
  }

  // No bin by that name already; choose a bin_index to assign to the newly
  // created bin.
  int new_bin_index = -1;
  if (_unused_bin_index) {
    // If there is some bin index that's not being used, we can claim it.
    int i = 0;
    for (i = 0; i < (int)_bin_definitions.size() && new_bin_index == -1; i++) {
      if (!_bin_definitions[i]._in_use) {
        new_bin_index = i;
      }
    }

    if (new_bin_index == -1) {
      // Oops, maybe we've used up all the unused indices already.
      _unused_bin_index = false;
    }
  }

  if (new_bin_index == -1) {
    // Slot a new index on the end of the vector.
    new_bin_index = _bin_definitions.size();
    _bin_definitions.push_back(BinDefinition());
  }

  BinDefinition &def = _bin_definitions[new_bin_index];
  def._in_use = true;
  def._name = name;
  def._type = type;
  def._sort = sort;
  def._active = true;

#ifndef NDEBUG
  // Check if there was a flash color configured for this bin name.
  ConfigVariableColor flash_bin
    ("flash-bin-" + name, "", "", ConfigVariable::F_dynamic);
  if (flash_bin.get_num_words() == 0) {
    def._flash_active = false;
  } else {
    def._flash_active = true;
    def._flash_color = flash_bin.get_value();
  }
#endif

  _bins_by_name.insert(BinsByName::value_type(name, new_bin_index));
  _sorted_bins.push_back(new_bin_index);
  _bins_are_sorted = false;

  return new_bin_index;
}

/**
 * Permanently removes the indicated bin.  This operation is not protected
 * from the pipeline and will disturb whatever is currently rendering in draw.
 * You should not call this during the normal course of rendering a frame; it
 * is intended only as an aid to development, to allow the developer to
 * interactively fiddle with the set of bins.
 */
void CullBinManager::
remove_bin(int bin_index) {
  nassertv(bin_index >= 0 && bin_index < (int)_bin_definitions.size());
  nassertv(_bin_definitions[bin_index]._in_use);

  _bin_definitions[bin_index]._in_use = false;
  SortedBins::iterator si =
    find(_sorted_bins.begin(), _sorted_bins.end(), bin_index);
  nassertv(si != _sorted_bins.end());
  _sorted_bins.erase(si);
  _bins_by_name.erase(_bin_definitions[bin_index]._name);

  // Now we have to make sure all of the data objects in the world that had
  // cached this bin index or have a bin object are correctly updated.

  // First, tell all the RenderStates in the world to reset their bin index
  // cache.
  RenderState::bin_removed(bin_index);

  // Now tell all the CullResults to clear themselves up too.
  CullResult::bin_removed(bin_index);
}

/**
 * Returns the bin_index associated with the bin of the given name, or -1 if
 * no bin has that name.
 */
int CullBinManager::
find_bin(const string &name) const {
  BinsByName::const_iterator bni;
  bni = _bins_by_name.find(name);
  if (bni != _bins_by_name.end()) {
    return (*bni).second;
  }
  return -1;
}

/**
 *
 */
void CullBinManager::
write(std::ostream &out) const {
  if (!_bins_are_sorted) {
    ((CullBinManager *)this)->do_sort_bins();
  }
  SortedBins::const_iterator sbi;
  for (sbi = _sorted_bins.begin(); sbi != _sorted_bins.end(); ++sbi) {
    int bin_index = (*sbi);
    out << get_bin_name(bin_index) << ", " << get_bin_type(bin_index)
        << ", " << get_bin_sort(bin_index) << "\n";
  }
}

/**
 * Intended to be called by CullResult when a new CullBin pointer
 * corresponding to the indicated bin_index is required.  It allocates and
 * returns a brand new CullBin object of the appropriate type.
 */
PT(CullBin) CullBinManager::
make_new_bin(int bin_index, GraphicsStateGuardianBase *gsg,
             const PStatCollector &draw_region_pcollector) {
  nassertr(bin_index >= 0 && bin_index < (int)_bin_definitions.size(), nullptr);
  nassertr(_bin_definitions[bin_index]._in_use, nullptr);
  string name = get_bin_name(bin_index);

  BinType type = _bin_definitions[bin_index]._type;
  BinConstructors::const_iterator ci = _bin_constructors.find(type);
  if (ci != _bin_constructors.end()) {
    BinConstructor *constructor = (*ci).second;
    return constructor(name, gsg, draw_region_pcollector);
  }

  // Hmm, unknown (or unregistered) bin type.
  nassert_raise("unknown bin type");
  return nullptr;
}

/**
 * Intended to be called at startup type by each CullBin type, to register the
 * constructor for each type.
 */
void CullBinManager::
register_bin_type(BinType type, CullBinManager::BinConstructor *constructor) {
  bool inserted = _bin_constructors.insert(BinConstructors::value_type(type, constructor)).second;
  nassertv(inserted);
}

/**
 * Puts the _sorted_bins vector in proper rendering order.
 */
void CullBinManager::
do_sort_bins() {
  sort(_sorted_bins.begin(), _sorted_bins.end(), SortBins(this));
  _bins_are_sorted = true;
}

/**
 * Called only at construction time to create the default bins and the bins
 * specified in the Configrc file.
 */
void CullBinManager::
setup_initial_bins() {
  ConfigVariableList cull_bin
    ("cull-bin",
     PRC_DESC("Creates a new cull bin by name, with the specified properties.  "
              "This is a string in three tokens, separated by whitespace: "
              "'bin_name sort type'."));

  // First, add all of the bins specified in the Configrc file.
  int num_bins = cull_bin.get_num_unique_values();

  for (int bi = 0; bi < num_bins; bi++) {
    string def = cull_bin.get_unique_value(bi);

    // This is a string in three tokens, separated by whitespace: bin_name
    // sort type

    vector_string words;
    extract_words(def, words);

    if (words.size() != 3) {
      pgraph_cat.error()
        << "Invalid cull-bin definition: " << def << "\n"
        << "Definition should be three words: bin_name sort type\n";
    } else {
      int sort;
      if (!string_to_int(words[1], sort)) {
        pgraph_cat.error()
          << "Invalid cull-bin definition: " << def << "\n"
          << "Sort token " << words[1] << " is not an integer.\n";

      } else {
        BinType type = parse_bin_type(words[2]);
        if (type == BT_invalid) {
          pgraph_cat.error()
            << "Invalid cull-bin definition: " << def << "\n"
            << "Bin type " << words[2] << " is not known.\n";
        } else {
          add_bin(words[0], type, sort);
        }
      }
    }
  }

  // Now add the default bins, unless the names have already been specified
  // explicitly in the Config file, above.
  if (find_bin("background") == -1) {
    add_bin("background", BT_fixed, 10);
  }
  if (find_bin("opaque") == -1) {
    add_bin("opaque", BT_state_sorted, 20);
  }
  if (find_bin("transparent") == -1) {
    add_bin("transparent", BT_back_to_front, 30);
  }
  if (find_bin("fixed") == -1) {
    add_bin("fixed", BT_fixed, 40);
  }
  if (find_bin("unsorted") == -1) {
    add_bin("unsorted", BT_unsorted, 50);
  }
}

/**
 * Given the name of a bin type, returns the corresponding BinType value, or
 * BT_invalid if it is an unknown type.
 */
CullBinManager::BinType CullBinManager::
parse_bin_type(const string &bin_type) {
  if (cmp_nocase_uh(bin_type, "unsorted") == 0) {
    return BT_unsorted;

  } else if (cmp_nocase_uh(bin_type, "state_sorted") == 0) {
    return BT_state_sorted;

  } else if (cmp_nocase_uh(bin_type, "fixed") == 0) {
    return BT_fixed;

  } else if (cmp_nocase_uh(bin_type, "back_to_front") == 0) {
    return BT_back_to_front;

  } else if (cmp_nocase_uh(bin_type, "front_to_back") == 0) {
    return BT_front_to_back;

  } else {
    return BT_invalid;
  }
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, CullBinManager::BinType bin_type) {
  switch (bin_type) {
  case CullBinManager::BT_invalid:
    return out << "invalid";

  case CullBinManager::BT_unsorted:
    return out << "unsorted";

  case CullBinManager::BT_state_sorted:
    return out << "state_sorted";

  case CullBinManager::BT_back_to_front:
    return out << "back_to_front";

  case CullBinManager::BT_front_to_back:
    return out << "front_to_back";

  case CullBinManager::BT_fixed:
    return out << "fixed";
  }

  return out << "**invalid BinType(" << (int)bin_type << ")**";
}

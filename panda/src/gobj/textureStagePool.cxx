/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureStagePool.cxx
 * @author drose
 * @date 2010-05-03
 */

#include "textureStagePool.h"
#include "config_gobj.h"
#include "mutexHolder.h"
#include "configVariableEnum.h"
#include "string_utils.h"

using std::istream;
using std::ostream;
using std::string;

TextureStagePool *TextureStagePool::_global_ptr = nullptr;


/**
 * Lists the contents of the TextureStage pool to the indicated output stream.
 */
void TextureStagePool::
write(ostream &out) {
  get_global_ptr()->ns_list_contents(out);
}

/**
 * The constructor is not intended to be called directly; there's only
 * supposed to be one TextureStagePool in the universe and it constructs
 * itself.
 */
TextureStagePool::
TextureStagePool() {
  ConfigVariableEnum<Mode> texture_stage_pool_mode
    ("texture-stage-pool-mode", M_none,
     PRC_DESC("Defines the initial value of TextureStagePool::set_mode().  "
              "Set this to 'none' to disable the use of the TextureStagePool, "
              "to 'name' to group TextureStages by name only, or 'unique' "
              "to group together only identical TextureStages."));
  _mode = texture_stage_pool_mode.get_value();
}

/**
 * The nonstatic implementation of get_stage().
 */
TextureStage *TextureStagePool::
ns_get_stage(TextureStage *temp) {
  MutexHolder holder(_lock);

  switch (_mode) {
  case M_none:
    return temp;

  case M_name:
    {
      StagesByName::iterator ni = _stages_by_name.find(temp->get_name());
      if (ni == _stages_by_name.end()) {
        ni = _stages_by_name.insert(StagesByName::value_type(temp->get_name(), temp)).first;
      } else {
        if ((*ni).first != (*ni).second->get_name()) {
          // The pointer no longer matches the original name.  Save a new one.
          (*ni).second = temp;
        }
      }
      return (*ni).second;
    }

  case M_unique:
    {
      CPT(TextureStage) cpttemp = temp;
      StagesByProperties::iterator si = _stages_by_properties.find(cpttemp);
      if (si == _stages_by_properties.end()) {
        si = _stages_by_properties.insert(StagesByProperties::value_type(new TextureStage(*temp), temp)).first;
      } else {
        if (*(*si).first != *(*si).second) {
          // The pointer no longer matches its original value.  Save a new
          // one.
          (*si).second = temp;
        }
      }
      return (*si).second;
    }
  }

  return nullptr;
}

/**
 * The nonstatic implementation of release_stage().
 */
void TextureStagePool::
ns_release_stage(TextureStage *temp) {
  MutexHolder holder(_lock);

  switch (_mode) {
  case M_none:
    break;

  case M_name:
    _stages_by_name.erase(temp->get_name());
    break;

  case M_unique:
    {
      CPT(TextureStage) cpttemp = temp;
      _stages_by_properties.erase(cpttemp);
    }
    break;
  }
}

/**
 * The nonstatic implementation of release_all_stages().
 */
void TextureStagePool::
ns_release_all_stages() {
  MutexHolder holder(_lock);

  _stages_by_name.clear();
  _stages_by_properties.clear();
}

/**
 *
 */
void TextureStagePool::
ns_set_mode(TextureStagePool::Mode mode) {
  MutexHolder holder(_lock);

  if (_mode != mode) {
    _stages_by_name.clear();
    _stages_by_properties.clear();
    _mode = mode;
  }
}

/**
 *
 */
TextureStagePool::Mode TextureStagePool::
ns_get_mode() {
  MutexHolder holder(_lock);

  return _mode;
}

/**
 * The nonstatic implementation of garbage_collect().
 */
int TextureStagePool::
ns_garbage_collect() {
  MutexHolder holder(_lock);

  switch (_mode) {
  case M_none:
    return 0;

  case M_name:
    {
      int num_released = 0;
      StagesByName new_set;

      StagesByName::iterator ni;
      for (ni = _stages_by_name.begin(); ni != _stages_by_name.end(); ++ni) {
        const string &name = (*ni).first;
        TextureStage *ts2 = (*ni).second;
        if (name != ts2->get_name() || ts2->get_ref_count() == 1) {
          if (gobj_cat.is_debug()) {
            gobj_cat.debug()
              << "Releasing " << name << "\n";
          }
          ++num_released;
        } else {
          new_set.insert(new_set.end(), *ni);
        }
      }

      _stages_by_name.swap(new_set);
      return num_released;
    }

  case M_unique:
    {
      int num_released = 0;
      StagesByProperties new_set;

      StagesByProperties::iterator si;
      for (si = _stages_by_properties.begin(); si != _stages_by_properties.end(); ++si) {
        const TextureStage *ts1 = (*si).first;
        TextureStage *ts2 = (*si).second;
        if ((*ts1) != (*ts2) || ts2->get_ref_count() == 1) {
          if (gobj_cat.is_debug()) {
            gobj_cat.debug()
              << "Releasing " << *ts1 << "\n";
          }
          ++num_released;
        } else {
          new_set.insert(new_set.end(), *si);
        }
      }

      _stages_by_properties.swap(new_set);
      return num_released;
    }
  }

  return 0;
}

/**
 * The nonstatic implementation of list_contents().
 */
void TextureStagePool::
ns_list_contents(ostream &out) const {
  MutexHolder holder(_lock);

  out << "TextureStagePool in mode " << _mode << "\n";

  switch (_mode) {
  case M_none:
    break;

  case M_name:
    {
      out << _stages_by_name.size() << " TextureStages:\n";
      StagesByName::const_iterator ni;
      for (ni = _stages_by_name.begin(); ni != _stages_by_name.end(); ++ni) {
        const string &name = (*ni).first;
        TextureStage *ts2 = (*ni).second;
        out << "  " << name
            << " (count = " << ts2->get_ref_count() << ")\n";
      }
    }
    break;

  case M_unique:
    {
      out << _stages_by_properties.size() << " TextureStages:\n";
      StagesByProperties::const_iterator si;
      for (si = _stages_by_properties.begin(); si != _stages_by_properties.end(); ++si) {
        const TextureStage *ts1 = (*si).first;
        TextureStage *ts2 = (*si).second;
        out << "  " << *ts1
            << " (count = " << ts2->get_ref_count() << ")\n";
      }
    }
    break;
  }
}

/**
 * Initializes and/or returns the global pointer to the one TextureStagePool
 * object in the system.
 */
TextureStagePool *TextureStagePool::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new TextureStagePool;
  }
  return _global_ptr;
}

/**
 *
 */
ostream &
operator << (ostream &out, TextureStagePool::Mode mode) {
  switch (mode) {
  case TextureStagePool::M_none:
    return out << "none";

  case TextureStagePool::M_name:
    return out << "name";

  case TextureStagePool::M_unique:
    return out << "unique";
  }

  return out << "**invalid mode (" << (int)mode << ")**";
}

/**
 *
 */
istream &
operator >> (istream &in, TextureStagePool::Mode &mode) {
  string word;
  in >> word;

  if (cmp_nocase(word, "none") == 0) {
    mode = TextureStagePool::M_none;
  } else if (cmp_nocase(word, "name") == 0) {
    mode = TextureStagePool::M_name;
  } else if (cmp_nocase(word, "unique") == 0) {
    mode = TextureStagePool::M_unique;

  } else {
    gobj_cat->error() << "Invalid TextureStagePool mode value: " << word << "\n";
    mode = TextureStagePool::M_none;
  }

  return in;
}

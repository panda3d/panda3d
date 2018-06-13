/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_map.cxx
 * @author drose
 * @date 2004-09-29
 */

#include "pandabase.h"
#include "pmap.h"
#include "memoryUsage.h"
#include "clockObject.h"

using std::cerr;
using std::cout;
using std::string;

class Alpha {
public:
  Alpha(const string &str) : _str(str) { }

  size_t get_hash() const {
    size_t hash = 0;
    string::const_iterator si;
    for (si = _str.begin(); si != _str.end(); ++si) {
      hash = (hash * 31) + (size_t)(*si);
    }
    return hash;
  }

  bool operator < (const Alpha &other) const {
    return strcmp(_str.c_str(), other._str.c_str()) < 0;
  }

  string _str;
};

std::ostream &operator << (std::ostream &out, const Alpha &alpha) {
  return out << alpha._str;
}

class Beta : public Alpha {
public:
  Beta(const string &str) : Alpha(str) { }

  size_t get_hash() const {
    return ~_str[0];
  }
};


static const char * const sample_strings[] = {
  "apple",
  "apricot",
  "banana",
  "blueberry",
  "grape",
  "grapefruit",
  "guava",
  "honeydew",
  "lemon",
  "lime",
  "mango",
  "orange",
  "peach",
  "pear",
  "pineapple",
  "plum",
  "raspberry",
  "strawberry",
  "watermelon",
};
static const size_t num_sample_strings = sizeof(sample_strings) / sizeof(const char *);

void
insert_fruit() {
  // typedef pmap<Alpha, string> MapType; typedef pmap<Beta, string> MapType;
  // typedef phash_map<Alpha, string> MapType;
  typedef phash_map<Beta, string> MapType;

  MapType m;

  for (size_t i = 0; i < num_sample_strings; i++) {
    string str = sample_strings[i];
    bool inserted = m.insert(MapType::value_type(str, str)).second;
    if (!inserted) {
      cerr << "Could not insert " << str << "\n";
    }
  }

  cout << "Map contains:\n";
  MapType::iterator mi;
  for (mi = m.begin(); mi != m.end(); ++mi) {
    cout << "  " << (*mi).first << "\n";
  }
}

void
test_performance() {
  typedef Alpha KeyType;

  typedef phash_map<KeyType, int> MapType;
  // typedef pmap<KeyType, int> MapType;

  MemoryUsage::is_tracking();
  ClockObject *clock = ClockObject::get_global_clock();

  static const int key_len = 10;
  static const int sample_mask = 0xffff;
  static const int sample_size = sample_mask + 1;
  static const int initial_population = 1000;
  static const int num_cycles = 10000;
  static const int num_reps = 3;

  std::vector<KeyType> samples;
  samples.reserve(sample_size);
  for (int s = 0; s < sample_size; s++) {
    string key;
    for (int k = 0; k < key_len; k++) {
      key += string(1, (char)((rand() & 0x3f) + '@'));
    }
    samples.push_back(key);
  }

  MemoryUsage::freeze();
  MapType *m = new MapType;
  cerr << "Empty map uses " << MemoryUsage::get_current_cpp_size()
       << " bytes.\n";

  for (int p = 0; p < initial_population; p++) {
    m->insert(MapType::value_type(samples[rand() & sample_mask], 0));
  }
  cerr << "map with " << m->size()
       << " elements uses " << MemoryUsage::get_current_cpp_size()
       << " bytes.\n";

  for (int r = 0; r < num_reps; r++) {
    double now = clock->get_real_time();
    for (int c = 0; c < num_cycles; c++) {
      const KeyType &element = samples[rand() & sample_mask];
      bool inserted = m->insert(MapType::value_type(element, 0)).second;
      if (inserted) {
        m->erase(element);
      }
    }
    double elapsed = clock->get_real_time() - now;
    cerr << "After " << num_cycles << " cycles in " << elapsed * 1000.0
         << " ms, map has " << m->size()
         << " elements and uses " << MemoryUsage::get_current_cpp_size()
         << " bytes.\n";
  }
}

int
main(int argc, char *argv[]) {
  // insert_fruit();
  test_performance();

  return 0;
}

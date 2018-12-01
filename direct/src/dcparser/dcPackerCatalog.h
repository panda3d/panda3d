/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPackerCatalog.h
 * @author drose
 * @date 2004-06-21
 */

#ifndef DCPACKERCATALOG_H
#define DCPACKERCATALOG_H

#include "dcbase.h"

class DCPackerInterface;
class DCPacker;
class DCSwitchParameter;

/**
 * This object contains the names of all of the nested fields available within
 * a particular field.  It is created on demand when a catalog is first
 * requested from a particular field; its ownership is retained by the field
 * so it must not be deleted.
 */
class EXPCL_DIRECT_DCPARSER DCPackerCatalog {
private:
  DCPackerCatalog(const DCPackerInterface *root);
  DCPackerCatalog(const DCPackerCatalog &copy);
  ~DCPackerCatalog();

public:
  // The Entry class records the static catalog data: the name of each field
  // and its relationship to its parent.
  class Entry {
  public:
    std::string _name;
    const DCPackerInterface *_field;
    const DCPackerInterface *_parent;
    int _field_index;
  };

  // The LiveCatalog class adds the dynamic catalog data: the actual location
  // of each field within the data record.  This might be different for
  // different data records (since some data fields have a dynamic length).
  class LiveCatalogEntry {
  public:
    size_t _begin;
    size_t _end;
  };
  class LiveCatalog {
  public:
    INLINE size_t get_begin(int n) const;
    INLINE size_t get_end(int n) const;

    INLINE int get_num_entries() const;
    INLINE const Entry &get_entry(int n) const;
    INLINE int find_entry_by_name(const std::string &name) const;
    INLINE int find_entry_by_field(const DCPackerInterface *field) const;

  private:
    typedef pvector<LiveCatalogEntry> LiveEntries;
    LiveEntries _live_entries;

    const DCPackerCatalog *_catalog;
    friend class DCPackerCatalog;
  };

  INLINE int get_num_entries() const;
  INLINE const Entry &get_entry(int n) const;
  int find_entry_by_name(const std::string &name) const;
  int find_entry_by_field(const DCPackerInterface *field) const;

  const LiveCatalog *get_live_catalog(const char *data, size_t length) const;
  void release_live_catalog(const LiveCatalog *live_catalog) const;

private:
  void add_entry(const std::string &name, const DCPackerInterface *field,
                 const DCPackerInterface *parent, int field_index);

  void r_fill_catalog(const std::string &name_prefix, const DCPackerInterface *field,
                      const DCPackerInterface *parent, int field_index);
  void r_fill_live_catalog(LiveCatalog *live_catalog, DCPacker &packer,
                           const DCSwitchParameter *&last_switch) const;

  const DCPackerCatalog *update_switch_fields(const DCSwitchParameter *dswitch,
                                              const DCPackerInterface *switch_case) const;


  const DCPackerInterface *_root;
  LiveCatalog *_live_catalog;

  typedef pvector<Entry> Entries;
  Entries _entries;

  typedef pmap<std::string, int> EntriesByName;
  EntriesByName _entries_by_name;

  typedef pmap<const DCPackerInterface *, int> EntriesByField;
  EntriesByField _entries_by_field;

  typedef pmap<const DCPackerInterface *, DCPackerCatalog *> SwitchCatalogs;
  SwitchCatalogs _switch_catalogs;

  typedef pmap<const DCSwitchParameter *, std::string> SwitchPrefixes;
  SwitchPrefixes _switch_prefixes;

  friend class DCPackerInterface;
};

#include "dcPackerCatalog.I"

#endif

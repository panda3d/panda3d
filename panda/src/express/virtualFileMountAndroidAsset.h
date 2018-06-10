/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountAndroidAsset.h
 * @author rdb
 * @date 2013-01-21
 */

#ifndef VIRTUALFILEMOUNTANDROIDASSET_H
#define VIRTUALFILEMOUNTANDROIDASSET_H

#ifdef ANDROID

#include "pandabase.h"

#include "virtualFileMount.h"
#include "multifile.h"
#include "pointerTo.h"

#include <android/asset_manager.h>

/**
 * Maps a Multifile's contents into the VirtualFileSystem.
 */
class EXPCL_PANDA_EXPRESS VirtualFileMountAndroidAsset : public VirtualFileMount {
PUBLISHED:
  INLINE VirtualFileMountAndroidAsset(AAssetManager *mgr, const std::string &apk_path);
  virtual ~VirtualFileMountAndroidAsset();

public:
  int get_fd(const Filename &file, off_t *start, off_t *end) const;

  virtual bool has_file(const Filename &file) const;
  virtual bool is_directory(const Filename &file) const;
  virtual bool is_regular_file(const Filename &file) const;

  virtual bool read_file(const Filename &file, bool do_uncompress,
                         vector_uchar &result) const;

  virtual std::istream *open_read_file(const Filename &file) const;
  virtual std::streamsize get_file_size(const Filename &file, std::istream *stream) const;
  virtual std::streamsize get_file_size(const Filename &file) const;
  virtual time_t get_timestamp(const Filename &file) const;
  virtual bool get_system_info(const Filename &file, SubfileInfo &info);

  virtual bool scan_directory(vector_string &contents,
                              const Filename &dir) const;

private:
  AAssetManager *_asset_mgr;
  std::string _apk_path;

  class AssetStream : public std::istream {
  public:
    INLINE AssetStream(AAsset *asset);
    virtual ~AssetStream();
  };

  class AssetStreamBuf : public std::streambuf {
  public:
    AssetStreamBuf(AAsset *asset);
    virtual ~AssetStreamBuf();

    virtual std::streampos seekoff(std::streamoff off, ios_seekdir dir, ios_openmode which);
    virtual std::streampos seekpos(std::streampos pos, ios_openmode which);

  protected:
    virtual int underflow();

  private:
    AAsset *_asset;
    off_t _offset;

    friend class VirtualFileMountAndroidAsset;
  };

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VirtualFileMount::init_type();
    register_type(_type_handle, "VirtualFileMountAndroidAsset",
                  VirtualFileMount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "virtualFileMountAndroidAsset.I"

#endif  // ANDROID

#endif

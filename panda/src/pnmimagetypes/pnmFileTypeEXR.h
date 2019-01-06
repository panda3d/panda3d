/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeEXR.h
 * @author drose
 * @date 2000-06-17
 */

#ifndef PNMFILETYPEEXR_H
#define PNMFILETYPEEXR_H

#include "pandabase.h"

#ifdef HAVE_OPENEXR

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

#include <ImfInputFile.h>
#include <OpenEXRConfig.h>

#ifdef OPENEXR_IMF_NAMESPACE
namespace IMF = OPENEXR_IMF_NAMESPACE;
#else
namespace IMF = Imf;
#endif

class ImfStdIstream;

/**
 * For reading and writing EXR floating-point or integer files.
 */
class EXPCL_PANDA_PNMIMAGETYPES PNMFileTypeEXR : public PNMFileType {
public:
  PNMFileTypeEXR();

  virtual std::string get_name() const;

  virtual int get_num_extensions() const;
  virtual std::string get_extension(int n) const;
  virtual std::string get_suggested_extension() const;

  virtual bool has_magic_number() const;
  virtual bool matches_magic_number(const std::string &magic_number) const;

  virtual PNMReader *make_reader(std::istream *file, bool owns_file = true,
                                 const std::string &magic_number = std::string());
  virtual PNMWriter *make_writer(std::ostream *file, bool owns_file = true);

public:
  class Reader : public PNMReader {
  public:
    Reader(PNMFileType *type, std::istream *file, bool owns_file, std::string magic_number);
    virtual ~Reader();

    virtual bool is_floating_point();
    virtual bool read_pfm(PfmFile &pfm);
    virtual int read_data(xel *array, xelval *alpha);

  private:
    class ImfStdIstream *_strm;
    IMF::InputFile _imf_file;

    typedef std::vector<std::string> ChannelNames;
    ChannelNames _channel_names;
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, std::ostream *file, bool owns_file);

    virtual bool supports_floating_point();
    virtual bool supports_integer();
    virtual bool write_pfm(const PfmFile &pfm);
    virtual int write_data(xel *array, xelval *alpha);
  };

private:

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypeEXR(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeEXR",
                  PNMFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_OPENEXR

#endif

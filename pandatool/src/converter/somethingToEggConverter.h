// Filename: somethingToEggConverter.h
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOMETHINGTOEGGCONVERTER_H
#define SOMETHINGTOEGGCONVERTER_H

#include <pandatoolbase.h>

#include <filename.h>
#include <config_util.h>  // for get_texture_path() and get_model_path()

class EggData;
class EggGroupNode;

////////////////////////////////////////////////////////////////////
// 	 Class : SomethingToEggConverter
// Description : This is a base class for a family of converter
//               classes that manage a conversion from some file type
//               to egg format.
//
//               Classes of this type can be used to implement xxx2egg
//               converter programs, as well as LoaderFileTypeXXX
//               run-time loaders.
////////////////////////////////////////////////////////////////////
class SomethingToEggConverter {
public:
  SomethingToEggConverter();
  SomethingToEggConverter(const SomethingToEggConverter &copy);
  virtual ~SomethingToEggConverter();

  virtual SomethingToEggConverter *make_copy()=0;

  enum PathConvert {
    PC_relative,
    PC_absolute,
    PC_rel_abs,
    PC_strip,
    PC_unchanged
  };
  INLINE void set_texture_path_convert(PathConvert tpc,
				       const Filename &tpc_directory = Filename());
  INLINE void set_model_path_convert(PathConvert mpc,
				     const Filename &mpc_directory = Filename());

  INLINE void set_merge_externals(bool merge_externals);
  INLINE bool get_merge_externals() const;

  void set_egg_data(EggData *egg_data, bool owns_egg_data);
  INLINE void clear_egg_data();
  INLINE EggData &get_egg_data();

  virtual string get_name() const=0;
  virtual string get_extension() const=0;

  virtual bool convert_file(const Filename &filename)=0;

  bool handle_external_reference(EggGroupNode *egg_parent,
				 const Filename &orig_filename,
				 const DSearchPath &searchpath);
  INLINE bool handle_external_reference(EggGroupNode *egg_parent,
					const Filename &orig_filename);


  INLINE Filename convert_texture_path(const Filename &orig_filename);
  INLINE Filename convert_texture_path(const Filename &orig_filename,
				       const DSearchPath &searchpath);
  INLINE Filename convert_model_path(const Filename &orig_filename);
  INLINE Filename convert_model_path(const Filename &orig_filename,
				     const DSearchPath &searchpath);

protected:
  static Filename convert_path(const Filename &orig_filename,
			       const DSearchPath &searchpath,
			       const Filename &rel_dir,
			       PathConvert path_convert);


protected:
  PathConvert _tpc;
  Filename _tpc_directory;
  PathConvert _mpc;
  Filename _mpc_directory;

  bool _merge_externals;

  EggData *_egg_data;
  bool _owns_egg_data;

  bool _error;
};

#include "somethingToEggConverter.I"

#endif



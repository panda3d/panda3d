// Filename: loaderFileTypePandatool.h
// Created by:  drose (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LOADERFILETYPEPANDATOOL_H
#define LOADERFILETYPEPANDATOOL_H

#include <pandatoolbase.h>

#include <loaderFileType.h>

class SomethingToEggConverter;

////////////////////////////////////////////////////////////////////
//       Class : LoaderFileTypePandatool
// Description : This defines the Loader interface to files whose
//               converters are defined within the Pandatool package
//               and inherit from SomethingToEggConverter, like
//               FltToEggConverter and LwoToEggConverter.
////////////////////////////////////////////////////////////////////
class EXPCL_PTLOADER LoaderFileTypePandatool : public LoaderFileType {
public:
  LoaderFileTypePandatool(SomethingToEggConverter *converter);
  virtual ~LoaderFileTypePandatool();

  virtual string get_name() const;
  virtual string get_extension() const;

  virtual void resolve_filename(Filename &path) const;
  virtual PT_Node load_file(const Filename &path, bool report_errors) const;

private:
  SomethingToEggConverter *_converter;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LoaderFileType::init_type();
    register_type(_type_handle, "LoaderFileTypePandatool",
                  LoaderFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif


// Filename: texture.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include "texture.h"
#include "config_gobj.h"

#include <stddef.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

//Should this be here?
#include "texturePool.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Texture::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
Texture() : ImageBuffer() {
  _level = 0; // Mipmap level
  _magfilter = FT_nearest;
  _minfilter = FT_nearest;
  _wrapu = WM_repeat;
  _wrapv = WM_repeat;
  _anisotropic_degree = 1;
  _pbuffer = new PixelBuffer;
  _has_requested_size = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Destructor 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Texture::
~Texture() {
  unprepare();
}

////////////////////////////////////////////////////////////////////
//     Function: read
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::read(const string& name)
{
  PNMImage pnmimage;

  if (!pnmimage.read(name)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << name << endl;
    return false;
  }

  if (max_texture_dimension > 0 &&
      (pnmimage.get_x_size() > max_texture_dimension ||
       pnmimage.get_y_size() > max_texture_dimension)) {
    int new_xsize = min(pnmimage.get_x_size(), max_texture_dimension);
    int new_ysize = min(pnmimage.get_y_size(), max_texture_dimension);
    gobj_cat.info()
      << "Automatically rescaling " << name << " from " 
      << pnmimage.get_x_size() << " by " << pnmimage.get_y_size() << " to "
      << new_xsize << " by " << new_ysize << "\n";

    PNMImage scaled(new_xsize, new_ysize, pnmimage.get_num_channels(),
		    pnmimage.get_maxval(), pnmimage.get_type());
    scaled.gaussian_filter_from(0.5, pnmimage);
    pnmimage = scaled;
  }

  set_name(name);
  clear_alpha_name();
  return load(pnmimage);
}

////////////////////////////////////////////////////////////////////
//     Function: read
//       Access:
//  Description: Combine a 3-component image with a grayscale image
//		 to get a 4-component image
////////////////////////////////////////////////////////////////////
bool Texture::read(const string &name, const string &gray) {
  PNMImage pnmimage;
  if (!pnmimage.read(name)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << name << endl;
    return false;
  }

  PNMImage grayimage;
  if (!grayimage.read(gray)) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << gray << endl;
    return false;
  }

  if (max_texture_dimension > 0 &&
      (pnmimage.get_x_size() > max_texture_dimension ||
       pnmimage.get_y_size() > max_texture_dimension)) {
    int new_xsize = min(pnmimage.get_x_size(), max_texture_dimension);
    int new_ysize = min(pnmimage.get_y_size(), max_texture_dimension);
    gobj_cat.info()
      << "Automatically rescaling " << name << " from " 
      << pnmimage.get_x_size() << " by " << pnmimage.get_y_size() << " to "
      << new_xsize << " by " << new_ysize << "\n";

    PNMImage scaled(new_xsize, new_ysize, pnmimage.get_num_channels(),
		    pnmimage.get_maxval(), pnmimage.get_type());
    scaled.gaussian_filter_from(0.5, pnmimage);
    pnmimage = scaled;
  }

  // Now do the same for the grayscale image
  if (max_texture_dimension > 0 &&
      (grayimage.get_x_size() > max_texture_dimension ||
       grayimage.get_y_size() > max_texture_dimension)) {
    int new_xsize = min(grayimage.get_x_size(), max_texture_dimension);
    int new_ysize = min(grayimage.get_y_size(), max_texture_dimension);
    gobj_cat.info()
      << "Automatically rescaling " << gray << " from " 
      << grayimage.get_x_size() << " by " << grayimage.get_y_size() << " to "
      << new_xsize << " by " << new_ysize << "\n";

    PNMImage scaled(new_xsize, new_ysize, pnmimage.get_num_channels(),
		    pnmimage.get_maxval(), pnmimage.get_type());
    scaled.gaussian_filter_from(0.5, pnmimage);
    pnmimage = scaled;
  }

  // Make sure the 2 images are the same size
  int xsize = pnmimage.get_x_size();
  int ysize = pnmimage.get_y_size();
  int gxsize = grayimage.get_x_size();
  int gysize = grayimage.get_y_size();
  if ((xsize != gxsize) || (ysize != gysize)) {
    gobj_cat.error()
      << "Texture::read() - grayscale image not the same size as original - "
      << "orig = " << xsize << "x" << ysize << " - gray = " << gxsize
      << "x" << gysize << endl;
    return false;
  }
  
  // Make the original image a 4-component image
  pnmimage.add_alpha();
  for (int x = 0; x < xsize; x++) {
    for (int y = 0; y < ysize; y++) { 
      pnmimage.set_alpha(x, y, grayimage.get_gray(x, y));
    }
  }

  set_name(name);
  set_alpha_name(gray);
  return load(pnmimage);
}

////////////////////////////////////////////////////////////////////
//     Function: write 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::write(const string& name) const
{
  return _pbuffer->write(name);
}

////////////////////////////////////////////////////////////////////
//     Function: load
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::load(const PNMImage& pnmimage)
{
  if (_pbuffer->load( pnmimage ) == false)
    return false;

  unprepare();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: store
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool Texture::store(PNMImage& pnmimage) const
{
  return _pbuffer->store( pnmimage );
}

////////////////////////////////////////////////////////////////////
//     Function: prepare 
//       Access: Public
//  Description: Creates a context for the texture on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) TextureContext.
////////////////////////////////////////////////////////////////////
TextureContext *Texture::
prepare(GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(gsg);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  TextureContext *tc = gsg->prepare_texture(this);
  _contexts[gsg] = tc;
  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::unprepare
//       Access: Public
//  Description: Frees the context allocated on all GSG's for which
//               the texture has been declared.
////////////////////////////////////////////////////////////////////
void Texture::
unprepare() {
  // We have to traverse a copy of the _contexts list, because the GSG
  // will call clear_gsg() in response to each release_texture(), and
  // we don't want to be modifying the _contexts list while we're
  // traversing it.
  Contexts temp = _contexts;

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    GraphicsStateGuardianBase *gsg = (*ci).first;
    TextureContext *tc = (*ci).second;
    gsg->release_texture(tc);
  }

  // Now that we've called release_texture() on every known GSG, the
  // _contexts list should have completely emptied itself.
  nassertv(_contexts.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::unprepare
//       Access: Public
//  Description: Frees the texture context only on the indicated GSG,
//               if it exists there.
////////////////////////////////////////////////////////////////////
void Texture::
unprepare(GraphicsStateGuardianBase *gsg) {
  Contexts::iterator ci;
  ci = _contexts.find(gsg);
  if (ci != _contexts.end()) {
    TextureContext *tc = (*ci).second;
    gsg->release_texture(tc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::clear_gsg
//       Access: Public
//  Description: Removes the indicated GSG from the Texture's known
//               GSG's, without actually releasing the texture on that
//               GSG.  This is intended to be called only from
//               GSG::release_texture(); it should never be called by
//               user code.
////////////////////////////////////////////////////////////////////
void Texture::
clear_gsg(GraphicsStateGuardianBase *gsg) {
  Contexts::iterator ci;
  ci = _contexts.find(gsg);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_gsg() was called on a GSG which
    // the texture didn't know about.
    nassertv(false);
  }
}

void Texture::copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr) {
  gsg->copy_texture(prepare(gsg), dr);
}

void Texture::copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
			const RenderBuffer &rb) {
  gsg->copy_texture(prepare(gsg), dr, rb);
}

void Texture::draw(GraphicsStateGuardianBase *) {
  gobj_cat.error()
    << "DisplayRegion required to draw texture.\n";
}

void Texture::draw(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr) {
  gsg->draw_texture(prepare(gsg), dr);
}

void Texture::draw(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
			const RenderBuffer &rb) {
  gsg->draw_texture(prepare(gsg), dr, rb);
}

////////////////////////////////////////////////////////////////////
//     Function: set_wrapu 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::set_wrapu(WrapMode wrap)
{
  if (_wrapu != wrap) {
    unprepare();
    _wrapu = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_wrapv
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::set_wrapv(WrapMode wrap)
{
  if (_wrapv != wrap) {
    unprepare();
    _wrapv = wrap;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_minfilter
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::set_minfilter(FilterType filter)
{
  if (_minfilter != filter) {
    unprepare();
    _minfilter = filter;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_magfilter
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void Texture::set_magfilter(FilterType filter)
{
  if (_magfilter != filter) {
    unprepare();
    _magfilter = filter;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_anisotropic_degree
//       Access: Public
//  Description: Specifies the level of anisotropic filtering to apply
//               to the texture.  Normally, this is 1, to indicate
//               anisotropic filtering is disabled.  This may be set
//               to a number higher than one to enable anisotropic
//               filtering, if the rendering backend supports this.
////////////////////////////////////////////////////////////////////
void Texture::
set_anisotropic_degree(int anisotropic_degree) {
  if (_anisotropic_degree != anisotropic_degree) {
    unprepare();
    _anisotropic_degree = anisotropic_degree;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Texture::
write_datagram(BamWriter *manager, Datagram &me)
{
  ImageBuffer::write_datagram(manager, me);
  me.add_uint32(_level);
  me.add_uint8(_wrapu);
  me.add_uint8(_wrapv);
  me.add_uint8(_minfilter);
  me.add_uint8(_magfilter);
  me.add_uint8(_magfiltercolor);
  me.add_uint8(_magfilteralpha);
  me.add_int16(_anisotropic_degree);
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void Texture::
fillin(DatagramIterator &scan, BamReader *manager) {
  //We don't want to call ImageBuffer::fillin, like we 
  //would normally, since due to needing to know the name
  //of the Texture before creating it, we have already read
  //that name in.  This is something of a problem as it forces 
  //Texture to know how the parent write_datagram works.  And
  //makes the assumption that the only data being written is
  //the name
  _level = scan.get_uint32();
  _wrapu = (enum WrapMode) scan.get_uint8();
  _wrapv = (enum WrapMode) scan.get_uint8();
  _minfilter = (enum FilterType) scan.get_uint8();
  _magfilter = (enum FilterType) scan.get_uint8();
  _magfiltercolor = (enum FilterType) scan.get_uint8();
  _magfilteralpha = (enum FilterType) scan.get_uint8();

  if (manager->get_file_minor_ver() >= 4) {
    _anisotropic_degree = scan.get_int16();
  } else {
    _anisotropic_degree = 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::make_Texture
//       Access: Protected
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
TypedWriteable* Texture::
make_Texture(const FactoryParams &params)
{
  //The process of making a texture is slightly
  //different than making other Writeable objects.
  //That is because all creation of Textures should
  //be done through calls to TexturePool, which ensures
  //that any loads of the same Texture, refer to the
  //same memory
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  string name = scan.get_string();
  string alpha_name;
  if (manager->get_file_minor_ver() >= 3) {
    alpha_name = scan.get_string();
  }

  PT(Texture) me;

  if (alpha_name.empty()) {
    me = TexturePool::load_texture(name);
  } else {
    me = TexturePool::load_texture(name, alpha_name);
  }

  if (me == (Texture *)NULL) {
    // Oops, we couldn't load the texture; we'll just return NULL.
    // But we do need a dummy texture to read in and ignore all of the
    // attributes.
    PT(Texture) dummy = new Texture;
    dummy->fillin(scan, manager);

  } else {
    if (gobj_cat->is_debug()) {
      gobj_cat->debug() << "Created texture " << me->get_name() << endl;
    }
    me->fillin(scan, manager);
  }
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a Texture object
////////////////////////////////////////////////////////////////////
void Texture::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_Texture);
}




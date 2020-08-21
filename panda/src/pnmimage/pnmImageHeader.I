/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmImageHeader.I
 * @author drose
 * @date 2000-06-15
 */

/**
 *
 */
INLINE PNMImageHeader::
PNMImageHeader() {
  _x_size = 0;
  _y_size = 0;
  _num_channels = 0;
  _maxval = 255;
  _color_space = CS_unspecified;
  _type = nullptr;
}

/**
 *
 */
INLINE PNMImageHeader::
PNMImageHeader(const PNMImageHeader &copy) :
  _x_size(copy._x_size),
  _y_size(copy._y_size),
  _num_channels(copy._num_channels),
  _maxval(copy._maxval),
  _color_space(copy._color_space),
  _type(copy._type)
{
}

/**
 *
 */
INLINE void PNMImageHeader::
operator = (const PNMImageHeader &copy) {
  _x_size = copy._x_size;
  _y_size = copy._y_size;
  _num_channels = copy._num_channels;
  _maxval = copy._maxval;
  _color_space = copy._color_space;
  _comment = copy._comment;
  _type = copy._type;
}

/**
 *
 */
INLINE PNMImageHeader::
~PNMImageHeader() {
}

/**
 * Returns the image type of the image, as an enumerated value.  This is
 * really just the number of channels cast to the enumerated type.
 */
INLINE PNMImageHeader::ColorType PNMImageHeader::
get_color_type() const {
  nassertr(_num_channels >= 1 && _num_channels <= 4, CT_invalid);
  return (ColorType)_num_channels;
}

/**
 * Returns the number of channels in the image.
 */
INLINE int PNMImageHeader::
get_num_channels() const {
  nassertr(_num_channels >= 1 && _num_channels <= 4, 0);
  return _num_channels;
}

/**
 * This static variant of is_grayscale() returns true if the indicated image
 * type represents a grayscale image, false otherwise.
 */
INLINE bool PNMImageHeader::
is_grayscale(PNMImageHeader::ColorType color_type) {
  return (color_type == CT_grayscale || color_type == CT_two_channel);
}

/**
 * Returns false if the image is a full-color image, and has red, green, and
 * blue components; true if it is a grayscale image and has only a gray
 * component.  (The gray color is actually stored in the blue channel, and the
 * red and green channels are ignored.)
 */
INLINE bool PNMImageHeader::
is_grayscale() const {
  return is_grayscale(get_color_type());
}

/**
 * This static variant of has_alpha() returns true if the indicated image type
 * includes an alpha channel, false otherwise.
 */
INLINE bool PNMImageHeader::
has_alpha(PNMImageHeader::ColorType color_type) {
  return (color_type == CT_two_channel || color_type == CT_four_channel);
}

/**
 * Returns true if the image includes an alpha channel, false otherwise.
 * Unlike is_grayscale(), if this returns false it is an error to call any of
 * the functions accessing the alpha channel.
 */
INLINE bool PNMImageHeader::
has_alpha() const {
  return has_alpha(get_color_type());
}

/**
 * Returns the maximum channel value allowable for any pixel in this image;
 * for instance, 255 for a typical 8-bit-per-channel image.  A pixel with this
 * value is full on.
 */
INLINE xelval PNMImageHeader::
get_maxval() const {
  return _maxval;
}

/**
 * Returns the color space that the image is encoded in, or CS_unspecified if
 * unknown.
 */
INLINE ColorSpace PNMImageHeader::
get_color_space() const {
  return _color_space;
}

/**
 * Returns the number of pixels in the X direction.  This is one more than the
 * largest allowable X coordinate.
 */
INLINE int PNMImageHeader::
get_x_size() const {
  return _x_size;
}

/**
 * Returns the number of pixels in the Y direction.  This is one more than the
 * largest allowable Y coordinate.
 */
INLINE int PNMImageHeader::
get_y_size() const {
  return _y_size;
}

/**
 * Returns the number of pixels in each direction.  This is one more than the
 * largest allowable coordinates.
 */
INLINE LVecBase2i PNMImageHeader::
get_size() const {
  return LVecBase2i(_x_size, _y_size);
}

/**
 * Gets the user comment from the file.
 */
INLINE std::string PNMImageHeader::
get_comment() const {
  return _comment;
}

/**
 * Writes a user comment string to the image (header).
 */
INLINE void PNMImageHeader::
set_comment(const std::string& comment) {
  _comment = comment;
}

/**
 * Returns true if the PNMImageHeader knows what type it is, false otherwise.
 */
INLINE bool PNMImageHeader::
has_type() const {
  return _type != nullptr;
}

/**
 * If the file type is known (e.g.  has_type() returns true), returns its
 * PNMFileType pointer; otherwise, returns NULL.
 */
INLINE PNMFileType *PNMImageHeader::
get_type() const {
  return _type;
}

/**
 * Sets the file type of this PNMImage.  This will be the default type used
 * when an image is read, if the type cannot be determined by magic number or
 * inferred by extension, or the type used when the image is written, if the
 * type cannot be inferred from the filename extension.
 */
INLINE void PNMImageHeader::
set_type(PNMFileType *type) {
  _type = type;
}

/**
 * Records the indicated color in the histogram.
 */
INLINE void PNMImageHeader::
record_color(PNMImageHeader::HistMap &hist,
             const PNMImageHeader::PixelSpec &color) {
  // First, try to add the color with a count of 0, in case it does not
  // already exist in the table.
  HistMap::iterator hi = hist.insert(HistMap::value_type(color, 0)).first;

  // Now that either succeeded or failed, but either way hi is now the
  // iterator to the count value in the table associated with the given color.
  // Increment that count.
  (*hi).second++;
}

/**
 *
 */
INLINE PNMImageHeader::PixelSpec::
PixelSpec(xelval gray_value) :
  _red(gray_value),
  _green(gray_value),
  _blue(gray_value),
  _alpha(0)
{
}

/**
 *
 */
INLINE PNMImageHeader::PixelSpec::
PixelSpec(xelval gray_value, xelval alpha) :
  _red(gray_value),
  _green(gray_value),
  _blue(gray_value),
  _alpha(alpha)
{
}

/**
 *
 */
INLINE PNMImageHeader::PixelSpec::
PixelSpec(xelval red, xelval green, xelval blue) :
  _red(red),
  _green(green),
  _blue(blue),
  _alpha(0)
{
}

/**
 *
 */
INLINE PNMImageHeader::PixelSpec::
PixelSpec(xelval red, xelval green, xelval blue, xelval alpha) :
  _red(red),
  _green(green),
  _blue(blue),
  _alpha(alpha)
{
}

/**
 *
 */
INLINE PNMImageHeader::PixelSpec::
PixelSpec(const xel &rgb) :
  _red(PPM_GETR(rgb)),
  _green(PPM_GETG(rgb)),
  _blue(PPM_GETB(rgb)),
  _alpha(0)
{
}

/**
 *
 */
INLINE PNMImageHeader::PixelSpec::
PixelSpec(const xel &rgb, xelval alpha) :
  _red(PPM_GETR(rgb)),
  _green(PPM_GETG(rgb)),
  _blue(PPM_GETB(rgb)),
  _alpha(alpha)
{
}

/**
 *
 */
INLINE bool PNMImageHeader::PixelSpec::
operator < (const PixelSpec &other) const {
  return compare_to(other) < 0;
}

/**
 *
 */
INLINE bool PNMImageHeader::PixelSpec::
operator == (const PixelSpec &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool PNMImageHeader::PixelSpec::
operator != (const PixelSpec &other) const {
  return compare_to(other) != 0;
}

/**
 *
 */
INLINE int PNMImageHeader::PixelSpec::
compare_to(const PixelSpec &other) const {
  if (_red != other._red) {
    return _red < other._red ? -1 : 1;
  }
  if (_green != other._green) {
    return _green < other._green ? -1 : 1;
  }
  if (_blue != other._blue) {
    return _blue < other._blue ? -1 : 1;
  }
  if (_alpha != other._alpha) {
    return _alpha < other._alpha ? -1 : 1;
  }
  return 0;
}

/**
 *
 */
INLINE xelval PNMImageHeader::PixelSpec::
get_red() const {
  return _red;
}

/**
 *
 */
INLINE xelval PNMImageHeader::PixelSpec::
get_green() const {
  return _green;
}

/**
 *
 */
INLINE xelval PNMImageHeader::PixelSpec::
get_blue() const {
  return _blue;
}

/**
 *
 */
INLINE xelval PNMImageHeader::PixelSpec::
get_alpha() const {
  return _alpha;
}

/**
 *
 */
INLINE void PNMImageHeader::PixelSpec::
set_red(xelval red) {
  _red = red;
}

/**
 *
 */
INLINE void PNMImageHeader::PixelSpec::
set_green(xelval green) {
  _green = green;
}

/**
 *
 */
INLINE void PNMImageHeader::PixelSpec::
set_blue(xelval blue) {
  _blue = blue;
}

/**
 *
 */
INLINE void PNMImageHeader::PixelSpec::
set_alpha(xelval alpha) {
  _alpha = alpha;
}

/**
 * Indexes numerically into the components, in the order R, G, B, A.  This
 * also makes the PixelSpec work like a tuple in Python.
 */
INLINE xelval PNMImageHeader::PixelSpec::
operator [](int n) const {
  nassertr(n >= 0 && n < size(), 0);
  return (&_red)[n];
}

/**
 * Specifies the number of components in the PixelSpec; this is always 4,
 * regardless of the type of image it was taken from.
 */
INLINE int PNMImageHeader::PixelSpec::
size() {
  return 4;
}

// Interrogate seems to have some problem with the syntax of this method.
// Whatever, we don't need it.
#ifndef CPPPARSER
/**
 *
 */
INLINE PNMImageHeader::PixelSpecCount::
PixelSpecCount(const PNMImageHeader::PixelSpec &pixel, int count) :
  _pixel(pixel),
  _count(count)
{
}
#endif  // CPPPARSER

/**
 * Used to sort the pixels in order from most common to least common.
 */
INLINE bool PNMImageHeader::PixelSpecCount::
operator < (const PNMImageHeader::PixelSpecCount &other) const {
  return _count > other._count;
}

/**
 *
 */
INLINE PNMImageHeader::Histogram::
Histogram() {
}

/**
 * Returns the number of unique pixel colors in the histogram.
 */
INLINE int PNMImageHeader::Histogram::
get_num_pixels() const {
  return _pixels.size();
}

/**
 * Returns the nth unique pixel color in the histogram.  These are ordered by
 * default from most common to least common.
 */
INLINE const PNMImageHeader::PixelSpec &PNMImageHeader::Histogram::
get_pixel(int n) const {
  nassertr(n >= 0 && n < (int)_pixels.size(), _pixels[0]._pixel);
  return _pixels[n]._pixel;
}

/**
 * Returns the number of occurrences in the image of the nth unique pixel
 * color in the histogram.
 */
INLINE int PNMImageHeader::Histogram::
get_count(int n) const {
  nassertr(n >= 0 && n < (int)_pixels.size(), 0);
  return _pixels[n]._count;
}

/**
 * Returns the number of occurrences in the image of the indicated pixel
 * color.
 */
INLINE int PNMImageHeader::Histogram::
get_count(const PNMImageHeader::PixelSpec &pixel) const {
  HistMap::const_iterator hi;
  hi = _hist_map.find(pixel);
  if (hi == _hist_map.end()) {
    return 0;
  }
  return (*hi).second;
}

/**
 * Swaps the data in the Histogram with the indicated data.  This is normally
 * used to load the Histogram data initially in PNMImage::make_histogram().
 */
INLINE void PNMImageHeader::Histogram::
swap(PixelCount &pixels, HistMap &hist_map) {
  _pixels.swap(pixels);
  _hist_map.swap(hist_map);
}

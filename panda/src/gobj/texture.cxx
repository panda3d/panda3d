/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texture.cxx
 * @author mike
 * @date 1997-01-09
 * @author fperazzi, PandaSE
 * @date 2010-04-29
 */

#include "pandabase.h"
#include "texture.h"
#include "config_gobj.h"
#include "config_putil.h"
#include "texturePool.h"
#include "textureContext.h"
#include "bamCache.h"
#include "bamCacheRecord.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "string_utils.h"
#include "preparedGraphicsObjects.h"
#include "pnmImage.h"
#include "pnmReader.h"
#include "pfmFile.h"
#include "virtualFileSystem.h"
#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "bam.h"
#include "zStream.h"
#include "indent.h"
#include "cmath.h"
#include "pStatTimer.h"
#include "pbitops.h"
#include "streamReader.h"
#include "texturePeeker.h"
#include "convert_srgb.h"

#ifdef HAVE_SQUISH
#include <squish.h>
#endif  // HAVE_SQUISH

#include <stddef.h>

using std::endl;
using std::istream;
using std::max;
using std::min;
using std::ostream;
using std::string;
using std::swap;

ConfigVariableEnum<Texture::QualityLevel> texture_quality_level
("texture-quality-level", Texture::QL_normal,
 PRC_DESC("This specifies a global quality level for all textures.  You "
          "may specify either fastest, normal, or best.  This actually "
          "affects the meaning of Texture::set_quality_level(QL_default), "
          "so it may be overridden on a per-texture basis.  This generally "
          "only has an effect when using the tinydisplay software renderer; "
          "it has little or no effect on normal, hardware-accelerated "
          "renderers.  See Texture::set_quality_level()."));

PStatCollector Texture::_texture_read_pcollector("*:Texture:Read");
TypeHandle Texture::_type_handle;
TypeHandle Texture::CData::_type_handle;
AutoTextureScale Texture::_textures_power_2 = ATS_unspecified;

// Stuff to read and write DDS files.

// little-endian, of course
#define DDS_MAGIC 0x20534444


// DDS_header.dwFlags
#define DDSD_CAPS                   0x00000001
#define DDSD_HEIGHT                 0x00000002
#define DDSD_WIDTH                  0x00000004
#define DDSD_PITCH                  0x00000008
#define DDSD_PIXELFORMAT            0x00001000
#define DDSD_MIPMAPCOUNT            0x00020000
#define DDSD_LINEARSIZE             0x00080000
#define DDSD_DEPTH                  0x00800000

// DDS_header.sPixelFormat.dwFlags
#define DDPF_ALPHAPIXELS            0x00000001
#define DDPF_FOURCC                 0x00000004
#define DDPF_INDEXED                0x00000020
#define DDPF_RGB                    0x00000040

// DDS_header.sCaps.dwCaps1
#define DDSCAPS_COMPLEX             0x00000008
#define DDSCAPS_TEXTURE             0x00001000
#define DDSCAPS_MIPMAP              0x00400000

// DDS_header.sCaps.dwCaps2
#define DDSCAPS2_CUBEMAP            0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000
#define DDSCAPS2_VOLUME             0x00200000

struct DDSPixelFormat {
  unsigned int pf_size;
  unsigned int pf_flags;
  unsigned int four_cc;
  unsigned int rgb_bitcount;
  unsigned int r_mask;
  unsigned int g_mask;
  unsigned int b_mask;
  unsigned int a_mask;
};

struct DDSCaps2 {
  unsigned int caps1;
  unsigned int caps2;
  unsigned int ddsx;
};

struct DDSHeader {
  unsigned int dds_magic;
  unsigned int dds_size;
  unsigned int dds_flags;
  unsigned int height;
  unsigned int width;
  unsigned int pitch;
  unsigned int depth;
  unsigned int num_levels;

  DDSPixelFormat pf;
  DDSCaps2 caps;
};

// Stuff to read KTX files.
enum KTXType {
  KTX_BYTE = 0x1400,
  KTX_UNSIGNED_BYTE = 0x1401,
  KTX_SHORT = 0x1402,
  KTX_UNSIGNED_SHORT = 0x1403,
  KTX_INT = 0x1404,
  KTX_UNSIGNED_INT = 0x1405,
  KTX_FLOAT = 0x1406,
  KTX_HALF_FLOAT = 0x140B,
  KTX_UNSIGNED_BYTE_3_3_2 = 0x8032,
  KTX_UNSIGNED_SHORT_4_4_4_4 = 0x8033,
  KTX_UNSIGNED_SHORT_5_5_5_1 = 0x8034,
  KTX_UNSIGNED_INT_8_8_8_8 = 0x8035,
  KTX_UNSIGNED_INT_10_10_10_2 = 0x8036,
  KTX_UNSIGNED_BYTE_2_3_3_REV = 0x8362,
  KTX_UNSIGNED_SHORT_5_6_5 = 0x8363,
  KTX_UNSIGNED_SHORT_5_6_5_REV = 0x8364,
  KTX_UNSIGNED_SHORT_4_4_4_4_REV = 0x8365,
  KTX_UNSIGNED_SHORT_1_5_5_5_REV = 0x8366,
  KTX_UNSIGNED_INT_8_8_8_8_REV = 0x8367,
  KTX_UNSIGNED_INT_2_10_10_10_REV = 0x8368,
  KTX_UNSIGNED_INT_24_8 = 0x84FA,
  KTX_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B,
  KTX_UNSIGNED_INT_5_9_9_9_REV = 0x8C3E,
  KTX_FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD,
};

enum KTXFormat {
  KTX_ALPHA = 0x1906,
  KTX_ALPHA12 = 0x803D,
  KTX_ALPHA16 = 0x803E,
  KTX_ALPHA16_SNORM = 0x9018,
  KTX_ALPHA4 = 0x803B,
  KTX_ALPHA8 = 0x803C,
  KTX_ALPHA8_SNORM = 0x9014,
  KTX_ALPHA_SNORM = 0x9010,
  KTX_BGR = 0x80E0,
  KTX_BGR_INTEGER = 0x8D9A,
  KTX_BGRA = 0x80E1,
  KTX_BGRA_INTEGER = 0x8D9B,
  KTX_BLUE = 0x1905,
  KTX_BLUE_INTEGER = 0x8D96,
  KTX_COLOR_INDEX = 0x1900,
  KTX_DEPTH24_STENCIL8 = 0x88F0,
  KTX_DEPTH32F_STENCIL8 = 0x8CAD,
  KTX_DEPTH_COMPONENT = 0x1902,
  KTX_DEPTH_COMPONENT16 = 0x81A5,
  KTX_DEPTH_COMPONENT24 = 0x81A6,
  KTX_DEPTH_COMPONENT32 = 0x81A7,
  KTX_DEPTH_COMPONENT32F = 0x8CAC,
  KTX_DEPTH_STENCIL = 0x84F9,
  KTX_GREEN = 0x1904,
  KTX_GREEN_INTEGER = 0x8D95,
  KTX_INTENSITY = 0x8049,
  KTX_INTENSITY12 = 0x804C,
  KTX_INTENSITY16 = 0x804D,
  KTX_INTENSITY16_SNORM = 0x901B,
  KTX_INTENSITY4 = 0x804A,
  KTX_INTENSITY8 = 0x804B,
  KTX_INTENSITY8_SNORM = 0x9017,
  KTX_INTENSITY_SNORM = 0x9013,
  KTX_LUMINANCE = 0x1909,
  KTX_LUMINANCE12 = 0x8041,
  KTX_LUMINANCE12_ALPHA12 = 0x8047,
  KTX_LUMINANCE12_ALPHA4 = 0x8046,
  KTX_LUMINANCE16 = 0x8042,
  KTX_LUMINANCE16_ALPHA16 = 0x8048,
  KTX_LUMINANCE16_ALPHA16_SNORM = 0x901A,
  KTX_LUMINANCE16_SNORM = 0x9019,
  KTX_LUMINANCE4 = 0x803F,
  KTX_LUMINANCE4_ALPHA4 = 0x8043,
  KTX_LUMINANCE6_ALPHA2 = 0x8044,
  KTX_LUMINANCE8 = 0x8040,
  KTX_LUMINANCE8_ALPHA8 = 0x8045,
  KTX_LUMINANCE8_ALPHA8_SNORM = 0x9016,
  KTX_LUMINANCE8_SNORM = 0x9015,
  KTX_LUMINANCE_ALPHA = 0x190A,
  KTX_LUMINANCE_ALPHA_SNORM = 0x9012,
  KTX_LUMINANCE_SNORM = 0x9011,
  KTX_R11F_G11F_B10F = 0x8C3A,
  KTX_R16 = 0x822A,
  KTX_R16_SNORM = 0x8F98,
  KTX_R16F = 0x822D,
  KTX_R16I = 0x8233,
  KTX_R16UI = 0x8234,
  KTX_R32F = 0x822E,
  KTX_R32I = 0x8235,
  KTX_R32UI = 0x8236,
  KTX_R3_G3_B2 = 0x2A10,
  KTX_R8 = 0x8229,
  KTX_R8_SNORM = 0x8F94,
  KTX_R8I = 0x8231,
  KTX_R8UI = 0x8232,
  KTX_RED = 0x1903,
  KTX_RED_INTEGER = 0x8D94,
  KTX_RED_SNORM = 0x8F90,
  KTX_RG = 0x8227,
  KTX_RG16 = 0x822C,
  KTX_RG16_SNORM = 0x8F99,
  KTX_RG16F = 0x822F,
  KTX_RG16I = 0x8239,
  KTX_RG16UI = 0x823A,
  KTX_RG32F = 0x8230,
  KTX_RG32I = 0x823B,
  KTX_RG32UI = 0x823C,
  KTX_RG8 = 0x822B,
  KTX_RG8_SNORM = 0x8F95,
  KTX_RG8I = 0x8237,
  KTX_RG8UI = 0x8238,
  KTX_RG_INTEGER = 0x8228,
  KTX_RG_SNORM = 0x8F91,
  KTX_RGB = 0x1907,
  KTX_RGB10 = 0x8052,
  KTX_RGB10_A2 = 0x8059,
  KTX_RGB12 = 0x8053,
  KTX_RGB16 = 0x8054,
  KTX_RGB16_SNORM = 0x8F9A,
  KTX_RGB16F = 0x881B,
  KTX_RGB16I = 0x8D89,
  KTX_RGB16UI = 0x8D77,
  KTX_RGB2 = 0x804E,
  KTX_RGB32F = 0x8815,
  KTX_RGB32I = 0x8D83,
  KTX_RGB32UI = 0x8D71,
  KTX_RGB4 = 0x804F,
  KTX_RGB5 = 0x8050,
  KTX_RGB5_A1 = 0x8057,
  KTX_RGB8 = 0x8051,
  KTX_RGB8_SNORM = 0x8F96,
  KTX_RGB8I = 0x8D8F,
  KTX_RGB8UI = 0x8D7D,
  KTX_RGB9_E5 = 0x8C3D,
  KTX_RGB_INTEGER = 0x8D98,
  KTX_RGB_SNORM = 0x8F92,
  KTX_RGBA = 0x1908,
  KTX_RGBA12 = 0x805A,
  KTX_RGBA16 = 0x805B,
  KTX_RGBA16_SNORM = 0x8F9B,
  KTX_RGBA16F = 0x881A,
  KTX_RGBA16I = 0x8D88,
  KTX_RGBA16UI = 0x8D76,
  KTX_RGBA2 = 0x8055,
  KTX_RGBA32F = 0x8814,
  KTX_RGBA32I = 0x8D82,
  KTX_RGBA32UI = 0x8D70,
  KTX_RGBA4 = 0x8056,
  KTX_RGBA8 = 0x8058,
  KTX_RGBA8_SNORM = 0x8F97,
  KTX_RGBA8I = 0x8D8E,
  KTX_RGBA8UI = 0x8D7C,
  KTX_RGBA_INTEGER = 0x8D99,
  KTX_RGBA_SNORM = 0x8F93,
  KTX_SLUMINANCE = 0x8C46,
  KTX_SLUMINANCE8 = 0x8C47,
  KTX_SLUMINANCE8_ALPHA8 = 0x8C45,
  KTX_SLUMINANCE_ALPHA = 0x8C44,
  KTX_SRGB = 0x8C40,
  KTX_SRGB8 = 0x8C41,
  KTX_SRGB8_ALPHA8 = 0x8C43,
  KTX_SRGB_ALPHA = 0x8C42,
  KTX_STENCIL_INDEX = 0x1901,
  KTX_STENCIL_INDEX1 = 0x8D46,
  KTX_STENCIL_INDEX16 = 0x8D49,
  KTX_STENCIL_INDEX4 = 0x8D47,
  KTX_STENCIL_INDEX8 = 0x8D48,
};

enum KTXCompressedFormat {
  KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2 = 0x8C72,
  KTX_COMPRESSED_LUMINANCE_LATC1 = 0x8C70,
  KTX_COMPRESSED_R11_EAC = 0x9270,
  KTX_COMPRESSED_RED = 0x8225,
  KTX_COMPRESSED_RED_RGTC1 = 0x8DBB,
  KTX_COMPRESSED_RG = 0x8226,
  KTX_COMPRESSED_RG11_EAC = 0x9272,
  KTX_COMPRESSED_RG_RGTC2 = 0x8DBD,
  KTX_COMPRESSED_RGB = 0x84ED,
  KTX_COMPRESSED_RGB8_ETC2 = 0x9274,
  KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276,
  KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT = 0x8E8E,
  KTX_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F,
  KTX_COMPRESSED_RGB_FXT1_3DFX = 0x86B0,
  KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG = 0x8C01,
  KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG = 0x8C00,
  KTX_COMPRESSED_RGB_S3TC_DXT1 = 0x83F0,
  KTX_COMPRESSED_RGBA = 0x84EE,
  KTX_COMPRESSED_RGBA8_ETC2_EAC = 0x9278,
  KTX_COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C,
  KTX_COMPRESSED_RGBA_FXT1_3DFX = 0x86B1,
  KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG = 0x8C03,
  KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG = 0x9137,
  KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG = 0x8C02,
  KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG = 0x9138,
  KTX_COMPRESSED_RGBA_S3TC_DXT1 = 0x83F1,
  KTX_COMPRESSED_RGBA_S3TC_DXT3 = 0x83F2,
  KTX_COMPRESSED_RGBA_S3TC_DXT5 = 0x83F3,
  KTX_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2 = 0x8C73,
  KTX_COMPRESSED_SIGNED_LUMINANCE_LATC1 = 0x8C71,
  KTX_COMPRESSED_SIGNED_R11_EAC = 0x9271,
  KTX_COMPRESSED_SIGNED_RED_RGTC1 = 0x8DBC,
  KTX_COMPRESSED_SIGNED_RG11_EAC = 0x9273,
  KTX_COMPRESSED_SIGNED_RG_RGTC2 = 0x8DBE,
  KTX_COMPRESSED_SLUMINANCE = 0x8C4A,
  KTX_COMPRESSED_SLUMINANCE_ALPHA = 0x8C4B,
  KTX_COMPRESSED_SRGB = 0x8C48,
  KTX_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279,
  KTX_COMPRESSED_SRGB8_ETC2 = 0x9275,
  KTX_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277,
  KTX_COMPRESSED_SRGB_ALPHA = 0x8C49,
  KTX_COMPRESSED_SRGB_ALPHA_BPTC_UNORM = 0x8E8D,
  KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1 = 0x8A56,
  KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2 = 0x93F0,
  KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1 = 0x8A57,
  KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2 = 0x93F1,
  KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1 = 0x8C4D,
  KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3 = 0x8C4E,
  KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5 = 0x8C4F,
  KTX_COMPRESSED_SRGB_PVRTC_2BPPV1 = 0x8A54,
  KTX_COMPRESSED_SRGB_PVRTC_4BPPV1 = 0x8A55,
  KTX_COMPRESSED_SRGB_S3TC_DXT1 = 0x8C4C,
  KTX_ETC1_RGB8 = 0x8D64,
  KTX_ETC1_SRGB8 = 0x88EE,
};

/**
 * Constructs an empty texture.  The default is to set up the texture as an
 * empty 2-d texture; follow up with one of the variants of setup_texture() if
 * this is not what you want.
 */
Texture::
Texture(const string &name) :
  Namable(name),
  _lock(name),
  _cvar(_lock)
{
  _reloading = false;

  CDWriter cdata(_cycler, true);
  do_set_format(cdata, F_rgb);
  do_set_component_type(cdata, T_unsigned_byte);
}

/**
 * Use Texture::make_copy() to make a duplicate copy of an existing Texture.
 */
Texture::
Texture(const Texture &copy) :
  Namable(copy),
  _cycler(copy._cycler),
  _lock(copy.get_name()),
  _cvar(_lock)
{
  _reloading = false;
}

/**
 * Use Texture::make_copy() to make a duplicate copy of an existing Texture.
 */
void Texture::
operator = (const Texture &copy) {
  Namable::operator = (copy);
  _cycler = copy._cycler;
}

/**
 *
 */
Texture::
~Texture() {
  release_all();
  nassertv(!_reloading);
}

/**
 * Generates a special cube map image in the texture that can be used to apply
 * bump mapping effects: for each texel in the cube map that is indexed by the
 * 3-d texture coordinates (x, y, z), the resulting value is the normalized
 * vector (x, y, z) (compressed from -1..1 into 0..1).
 */
void Texture::
generate_normalization_cube_map(int size) {
  CDWriter cdata(_cycler, true);
  do_setup_texture(cdata, TT_cube_map, size, size, 6, T_unsigned_byte, F_rgb);
  PTA_uchar image = do_make_ram_image(cdata);
  cdata->_keep_ram_image = true;

  cdata->inc_image_modified();
  cdata->inc_properties_modified();

  PN_stdfloat half_size = (PN_stdfloat)size * 0.5f;
  PN_stdfloat center = half_size - 0.5f;

  LMatrix4 scale
    (127.5f, 0.0f, 0.0f, 0.0f,
     0.0f, 127.5f, 0.0f, 0.0f,
     0.0f, 0.0f, 127.5f, 0.0f,
     127.5f, 127.5f, 127.5f, 1.0f);

  unsigned char *p = image;
  int xi, yi;

  // Page 0: positive X.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3 vec(half_size, center - yi, center - xi);
      vec.normalize();
      vec = scale.xform_point(vec);

      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 1: negative X.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3 vec(-half_size, center - yi, xi - center);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 2: positive Y.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3 vec(xi - center, half_size, yi - center);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 3: negative Y.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3 vec(xi - center, -half_size, center - yi);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 4: positive Z.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3 vec(xi - center, center - yi, half_size);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }

  // Page 5: negative Z.
  for (yi = 0; yi < size; ++yi) {
    for (xi = 0; xi < size; ++xi) {
      LVector3 vec(center - xi, center - yi, -half_size);
      vec.normalize();
      vec = scale.xform_point(vec);
      *p++ = (unsigned char)vec[2];
      *p++ = (unsigned char)vec[1];
      *p++ = (unsigned char)vec[0];
    }
  }
}

/**
 * Generates a special 256x1 1-d texture that can be used to apply an
 * arbitrary alpha scale to objects by judicious use of texture matrix.  The
 * texture is a gradient, with an alpha of 0 on the left (U = 0), and 255 on
 * the right (U = 1).
 */
void Texture::
generate_alpha_scale_map() {
  CDWriter cdata(_cycler, true);
  do_setup_texture(cdata, TT_1d_texture, 256, 1, 1, T_unsigned_byte, F_alpha);
  cdata->_default_sampler.set_wrap_u(SamplerState::WM_clamp);
  cdata->_default_sampler.set_minfilter(SamplerState::FT_nearest);
  cdata->_default_sampler.set_magfilter(SamplerState::FT_nearest);

  cdata->_compression = CM_off;

  cdata->inc_image_modified();
  cdata->inc_properties_modified();

  PTA_uchar image = do_make_ram_image(cdata);
  cdata->_keep_ram_image = true;

  unsigned char *p = image;
  for (int xi = 0; xi < 256; ++xi) {
    *p++ = xi;
  }
}

/**
 * Reads the named filename into the texture.
 */
bool Texture::
read(const Filename &fullpath, const LoaderOptions &options) {
  CDWriter cdata(_cycler, true);
  do_clear(cdata);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  return do_read(cdata, fullpath, Filename(), 0, 0, 0, 0, false, false,
                 options, nullptr);
}

/**
 * Combine a 3-component image with a grayscale image to get a 4-component
 * image.
 *
 * See the description of the full-parameter read() method for the meaning of
 * the primary_file_num_channels and alpha_file_channel parameters.
 */
bool Texture::
read(const Filename &fullpath, const Filename &alpha_fullpath,
     int primary_file_num_channels, int alpha_file_channel,
     const LoaderOptions &options) {
  CDWriter cdata(_cycler, true);
  do_clear(cdata);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  return do_read(cdata, fullpath, alpha_fullpath, primary_file_num_channels,
                 alpha_file_channel, 0, 0, false, false,
                 options, nullptr);
}

/**
 * Reads a single file into a single page or mipmap level, or automatically
 * reads a series of files into a series of pages and/or mipmap levels.
 *
 * See the description of the full-parameter read() method for the meaning of
 * the various parameters.
 */
bool Texture::
read(const Filename &fullpath, int z, int n,
     bool read_pages, bool read_mipmaps,
     const LoaderOptions &options) {
  CDWriter cdata(_cycler, true);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  return do_read(cdata, fullpath, Filename(), 0, 0, z, n, read_pages, read_mipmaps,
                 options, nullptr);
}

/**
 * Reads the texture from the indicated filename.  If
 * primary_file_num_channels is not 0, it specifies the number of components
 * to downgrade the image to if it is greater than this number.
 *
 * If the filename has the extension .txo, this implicitly reads a texture
 * object instead of a filename (which replaces all of the texture
 * properties).  In this case, all the rest of the parameters are ignored, and
 * the filename should not contain any hash marks; just the one named file
 * will be read, since a single .txo file can contain all pages and mipmaps
 * necessary to define a texture.
 *
 * If alpha_fullpath is not empty, it specifies the name of a file from which
 * to retrieve the alpha.  In this case, alpha_file_channel represents the
 * numeric channel of this image file to use as the resulting texture's alpha
 * channel; usually, this is 0 to indicate the grayscale combination of r, g,
 * b; or it may be a one-based channel number, e.g.  1 for the red channel, 2
 * for the green channel, and so on.
 *
 * If read pages is false, then z indicates the page number into which this
 * image will be assigned.  Normally this is 0 for the first (or only) page of
 * the texture.  3-D textures have one page for each level of depth, and cube
 * map textures always have six pages.
 *
 * If read_pages is true, multiple images will be read at once, one for each
 * page of a cube map or a 3-D texture.  In this case, the filename should
 * contain a sequence of one or more hash marks ("#") which will be filled in
 * with the z value of each page, zero-based.  In this case, the z parameter
 * indicates the maximum z value that will be loaded, or 0 to load all
 * filenames that exist.
 *
 * If read_mipmaps is false, then n indicates the mipmap level to which this
 * image will be assigned.  Normally this is 0 for the base texture image, but
 * it is possible to load custom mipmap levels into the later images.  After
 * the base texture image is loaded (thus defining the size of the texture),
 * you can call get_expected_num_mipmap_levels() to determine the maximum
 * sensible value for n.
 *
 * If read_mipmaps is true, multiple images will be read as above, but this
 * time the images represent the different mipmap levels of the texture image.
 * In this case, the n parameter indicates the maximum n value that will be
 * loaded, or 0 to load all filenames that exist (up to the expected number of
 * mipmap levels).
 *
 * If both read_pages and read_mipmaps is true, then both sequences will be
 * read; the filename should contain two sequences of hash marks, separated by
 * some character such as a hyphen, underscore, or dot.  The first hash mark
 * sequence will be filled in with the mipmap level, while the second hash
 * mark sequence will be the page index.
 *
 * This method implicitly sets keep_ram_image to false.
 */
bool Texture::
read(const Filename &fullpath, const Filename &alpha_fullpath,
     int primary_file_num_channels, int alpha_file_channel,
     int z, int n, bool read_pages, bool read_mipmaps,
     BamCacheRecord *record,
     const LoaderOptions &options) {
  CDWriter cdata(_cycler, true);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  return do_read(cdata, fullpath, alpha_fullpath, primary_file_num_channels,
                 alpha_file_channel, z, n, read_pages, read_mipmaps,
                 options, record);
}

/**
 * Estimates the amount of texture memory that will be consumed by loading
 * this texture.  This returns a value that is not specific to any particular
 * graphics card or driver; it tries to make a reasonable assumption about how
 * a driver will load the texture.  It does not account for texture
 * compression or anything fancy.  This is mainly useful for debugging and
 * reporting purposes.
 *
 * Returns a value in bytes.
 */
size_t Texture::
estimate_texture_memory() const {
  CDReader cdata(_cycler);
  size_t pixels = cdata->_x_size * cdata->_y_size * cdata->_z_size;

  size_t bpp = 4;
  switch (cdata->_format) {
  case Texture::F_rgb332:
    bpp = 1;
    break;

  case Texture::F_alpha:
  case Texture::F_red:
  case Texture::F_green:
  case Texture::F_blue:
  case Texture::F_luminance:
  case Texture::F_sluminance:
  case Texture::F_r8i:
    bpp = 1;
    break;

  case Texture::F_luminance_alpha:
  case Texture::F_luminance_alphamask:
  case Texture::F_sluminance_alpha:
  case Texture::F_rgba4:
  case Texture::F_rgb5:
  case Texture::F_rgba5:
  case Texture::F_rg:
    bpp = 2;
    break;

  case Texture::F_rgba:
  case Texture::F_rgbm:
  case Texture::F_rgb:
  case Texture::F_srgb:
    // Most of the above formats have only 3 bytes, but they are most likely
    // to get padded by the driver
    bpp = 4;
    break;

  case Texture::F_color_index:
  case Texture::F_rgb8:
  case Texture::F_rgba8:
  case Texture::F_srgb_alpha:
  case Texture::F_rgb8i:
  case Texture::F_rgba8i:
    bpp = 4;
    break;

  case Texture::F_depth_stencil:
    bpp = 4;
    break;

  case Texture::F_depth_component:
  case Texture::F_depth_component16:
    bpp = 2;
    break;

  case Texture::F_depth_component24: // Gets padded
  case Texture::F_depth_component32:
    bpp = 4;
    break;

  case Texture::F_rgba12:
  case Texture::F_rgb12:
    bpp = 8;
    break;

  case Texture::F_rgba16:
    bpp = 8;
    break;
  case Texture::F_rgba32:
    bpp = 16;
    break;

  case Texture::F_r16:
  case Texture::F_r16i:
  case Texture::F_rg8i:
    bpp = 2;
    break;
  case Texture::F_rg16:
    bpp = 4;
    break;
  case Texture::F_rgb16:
    bpp = 8;
    break;

  case Texture::F_r32i:
  case Texture::F_r32:
    bpp = 4;
    break;

  case Texture::F_rg32:
    bpp = 8;
    break;

  case Texture::F_rgb32:
    bpp = 16;
    break;

  case Texture::F_r11_g11_b10:
  case Texture::F_rgb9_e5:
  case Texture::F_rgb10_a2:
    bpp = 4;
    break;

  default:
    gobj_cat.warning() << "Unhandled format in estimate_texture_memory(): "
                       << cdata->_format << "\n";
    break;
  }

  size_t bytes = pixels * bpp;
  if (uses_mipmaps()) {
    bytes = (bytes * 4) / 3;
  }

  return bytes;
}

/**
 * Records an arbitrary object in the Texture, associated with a specified
 * key.  The object may later be retrieved by calling get_aux_data() with the
 * same key.
 *
 * These data objects are not recorded to a bam or txo file.
 */
void Texture::
set_aux_data(const string &key, TypedReferenceCount *aux_data) {
  MutexHolder holder(_lock);
  _aux_data[key] = aux_data;
}

/**
 * Removes a record previously recorded via set_aux_data().
 */
void Texture::
clear_aux_data(const string &key) {
  MutexHolder holder(_lock);
  _aux_data.erase(key);
}

/**
 * Returns a record previously recorded via set_aux_data().  Returns NULL if
 * there was no record associated with the indicated key.
 */
TypedReferenceCount *Texture::
get_aux_data(const string &key) const {
  MutexHolder holder(_lock);
  AuxData::const_iterator di;
  di = _aux_data.find(key);
  if (di != _aux_data.end()) {
    return (*di).second;
  }
  return nullptr;
}

/**
 * Reads the texture from a Panda texture object.  This defines the complete
 * Texture specification, including the image data as well as all texture
 * properties.  This only works if the txo file contains a static Texture
 * image, as opposed to a subclass of Texture such as a movie texture.
 *
 * Pass a real filename if it is available, or empty string if it is not.
 */
bool Texture::
read_txo(istream &in, const string &filename) {
  CDWriter cdata(_cycler, true);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  return do_read_txo(cdata, in, filename);
}

/**
 * Constructs a new Texture object from the txo file.  This is similar to
 * Texture::read_txo(), but it constructs and returns a new object, which
 * allows it to return a subclass of Texture (for instance, a movie texture).
 *
 * Pass a real filename if it is available, or empty string if it is not.
 */
PT(Texture) Texture::
make_from_txo(istream &in, const string &filename) {
  DatagramInputFile din;

  if (!din.open(in, filename)) {
    gobj_cat.error()
      << "Could not read texture object: " << filename << "\n";
    return nullptr;
  }

  string head;
  if (!din.read_header(head, _bam_header.size())) {
    gobj_cat.error()
      << filename << " is not a texture object file.\n";
    return nullptr;
  }

  if (head != _bam_header) {
    gobj_cat.error()
      << filename << " is not a texture object file.\n";
    return nullptr;
  }

  BamReader reader(&din);
  if (!reader.init()) {
    return nullptr;
  }

  TypedWritable *object = reader.read_object();

  if (object != nullptr &&
      object->is_exact_type(BamCacheRecord::get_class_type())) {
    // Here's a special case: if the first object in the file is a
    // BamCacheRecord, it's really a cache data file and not a true txo file;
    // but skip over the cache data record and let the user treat it like an
    // ordinary txo file.
    object = reader.read_object();
  }

  if (object == nullptr) {
    gobj_cat.error()
      << "Texture object " << filename << " is empty.\n";
    return nullptr;

  } else if (!object->is_of_type(Texture::get_class_type())) {
    gobj_cat.error()
      << "Texture object " << filename << " contains a "
      << object->get_type() << ", not a Texture.\n";
    return nullptr;
  }

  PT(Texture) other = DCAST(Texture, object);
  if (!reader.resolve()) {
    gobj_cat.error()
      << "Unable to fully resolve texture object file.\n";
    return nullptr;
  }

  return other;
}

/**
 * Writes the texture to a Panda texture object.  This defines the complete
 * Texture specification, including the image data as well as all texture
 * properties.
 *
 * The filename is just for reference.
 */
bool Texture::
write_txo(ostream &out, const string &filename) const {
  CDReader cdata(_cycler);
  return do_write_txo(cdata, out, filename);
}

/**
 * Reads the texture from a DDS file object.  This is a Microsoft-defined file
 * format; it is similar in principle to a txo object, in that it is designed
 * to contain the texture image in a form as similar as possible to its
 * runtime image, and it can contain mipmaps, pre-compressed textures, and so
 * on.
 *
 * As with read_txo, the filename is just for reference.
 */
bool Texture::
read_dds(istream &in, const string &filename, bool header_only) {
  CDWriter cdata(_cycler, true);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  return do_read_dds(cdata, in, filename, header_only);
}

/**
 * Reads the texture from a KTX file object.  This is a Khronos-defined file
 * format; it is similar in principle to a dds object, in that it is designed
 * to contain the texture image in a form as similar as possible to its
 * runtime image, and it can contain mipmaps, pre-compressed textures, and so
 * on.
 *
 * As with read_dds, the filename is just for reference.
 */
bool Texture::
read_ktx(istream &in, const string &filename, bool header_only) {
  CDWriter cdata(_cycler, true);
  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  return do_read_ktx(cdata, in, filename, header_only);
}

/**
 * Loads a texture whose filename is derived by concatenating a suffix to the
 * filename of this texture.  May return NULL, for example, if this texture
 * doesn't have a filename.
 */
Texture *Texture::
load_related(const InternalName *suffix) const {
  MutexHolder holder(_lock);
  CDReader cdata(_cycler);

  RelatedTextures::const_iterator ti;
  ti = _related_textures.find(suffix);
  if (ti != _related_textures.end()) {
    return (*ti).second;
  }
  if (cdata->_fullpath.empty()) {
    return nullptr;
  }
  Filename main = cdata->_fullpath;
  main.set_basename_wo_extension(main.get_basename_wo_extension() +
                                 suffix->get_name());
  PT(Texture) res;
  if (!cdata->_alpha_fullpath.empty()) {
    Filename alph = cdata->_alpha_fullpath;
    alph.set_basename_wo_extension(alph.get_basename_wo_extension() +
                                   suffix->get_name());
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    if (vfs->exists(alph)) {
      // The alpha variant of the filename, with the suffix, exists.  Use it
      // to load the texture.
      res = TexturePool::load_texture(main, alph,
                                      cdata->_primary_file_num_channels,
                                      cdata->_alpha_file_channel, false);
    } else {
      // If the alpha variant of the filename doesn't exist, just go ahead and
      // load the related texture without alpha.
      res = TexturePool::load_texture(main);
    }

  } else {
    // No alpha filename--just load the single file.  It doesn't necessarily
    // have the same number of channels as this one.
    res = TexturePool::load_texture(main);
  }

  // I'm casting away the const-ness of 'this' because this field is only a
  // cache.
  ((Texture *)this)->_related_textures.insert(RelatedTextures::value_type(suffix, res));
  return res;
}

/**
 * Replaces the current system-RAM image with the new data, converting it
 * first if necessary from the indicated component-order format.  See
 * get_ram_image_as() for specifications about the format.  This method cannot
 * support compressed image data or sub-pages; use set_ram_image() for that.
 */
void Texture::
set_ram_image_as(CPTA_uchar image, const string &supplied_format) {
  CDWriter cdata(_cycler, true);

  string format = upcase(supplied_format);

  // Make sure we can grab something that's uncompressed.
  int imgsize = cdata->_x_size * cdata->_y_size;
  nassertv(image.size() == (size_t)(cdata->_component_width * format.size() * imgsize));

  // Check if the format is already what we have internally.
  if ((cdata->_num_components == 1 && format.size() == 1) ||
      (cdata->_num_components == 2 && format.size() == 2 && format.at(1) == 'A' && format.at(0) != 'A') ||
      (cdata->_num_components == 3 && format == "BGR") ||
      (cdata->_num_components == 4 && format == "BGRA")) {
    // The format string is already our format, so we just need to copy it.
    do_set_ram_image(cdata, image);
    return;
  }

  // Create a new empty array that can hold our image.
  PTA_uchar newdata = PTA_uchar::empty_array(imgsize * cdata->_num_components * cdata->_component_width, get_class_type());

  // These ifs are for optimization of commonly used image types.
  if (cdata->_component_width == 1) {
    if (format == "RGBA" && cdata->_num_components == 4) {
      imgsize *= 4;
      for (int p = 0; p < imgsize; p += 4) {
        newdata[p + 2] = image[p    ];
        newdata[p + 1] = image[p + 1];
        newdata[p    ] = image[p + 2];
        newdata[p + 3] = image[p + 3];
      }
      do_set_ram_image(cdata, newdata);
      return;
    }
    if (format == "RGB" && cdata->_num_components == 3) {
      imgsize *= 3;
      for (int p = 0; p < imgsize; p += 3) {
        newdata[p + 2] = image[p    ];
        newdata[p + 1] = image[p + 1];
        newdata[p    ] = image[p + 2];
      }
      do_set_ram_image(cdata, newdata);
      return;
    }
    if (format == "A" && cdata->_num_components != 3) {
      // We can generally rely on alpha to be the last component.
      int component = cdata->_num_components - 1;
      for (int p = 0; p < imgsize; ++p) {
        newdata[component] = image[p];
      }
      do_set_ram_image(cdata, newdata);
      return;
    }
    for (int p = 0; p < imgsize; ++p) {
      for (uchar s = 0; s < format.size(); ++s) {
        signed char component = -1;
        if (format.at(s) == 'B' || (cdata->_num_components <= 2 && format.at(s) != 'A')) {
          component = 0;
        } else if (format.at(s) == 'G') {
          component = 1;
        } else if (format.at(s) == 'R') {
          component = 2;
        } else if (format.at(s) == 'A') {
          if (cdata->_num_components != 3) {
            component = cdata->_num_components - 1;
          } else {
            // Ignore.
          }
        } else if (format.at(s) == '0') {
          // Ignore.
        } else if (format.at(s) == '1') {
          // Ignore.
        } else {
          gobj_cat.error() << "Unexpected component character '"
            << format.at(s) << "', expected one of RGBA!\n";
          return;
        }
        if (component >= 0) {
          newdata[p * cdata->_num_components + component] = image[p * format.size() + s];
        }
      }
    }
    do_set_ram_image(cdata, newdata);
    return;
  }
  for (int p = 0; p < imgsize; ++p) {
    for (uchar s = 0; s < format.size(); ++s) {
      signed char component = -1;
      if (format.at(s) == 'B' || (cdata->_num_components <= 2 && format.at(s) != 'A')) {
        component = 0;
      } else if (format.at(s) == 'G') {
        component = 1;
      } else if (format.at(s) == 'R') {
        component = 2;
      } else if (format.at(s) == 'A') {
        if (cdata->_num_components != 3) {
          component = cdata->_num_components - 1;
        } else {
          // Ignore.
        }
      } else if (format.at(s) == '0') {
        // Ignore.
      } else if (format.at(s) == '1') {
        // Ignore.
      } else {
        gobj_cat.error() << "Unexpected component character '"
          << format.at(s) << "', expected one of RGBA!\n";
        return;
      }
      if (component >= 0) {
        memcpy((void*)(newdata + (p * cdata->_num_components + component) * cdata->_component_width),
               (void*)(image + (p * format.size() + s) * cdata->_component_width),
               cdata->_component_width);
      }
    }
  }
  do_set_ram_image(cdata, newdata);
  return;
}

/**
 * Returns the flag that indicates whether this Texture is eligible to have
 * its main RAM copy of the texture memory dumped when the texture is prepared
 * for rendering.  See set_keep_ram_image().
 */
bool Texture::
get_keep_ram_image() const {
  CDReader cdata(_cycler);
  return cdata->_keep_ram_image;
}

/**
 * Returns true if there is enough information in this Texture object to write
 * it to the bam cache successfully, false otherwise.  For most textures, this
 * is the same as has_ram_image().
 */
bool Texture::
is_cacheable() const {
  CDReader cdata(_cycler);
  return do_has_bam_rawdata(cdata);
}

/**
 * Returns the number of contiguous mipmap levels that exist in RAM, up until
 * the first gap in the sequence.  It is guaranteed that at least mipmap
 * levels [0, get_num_ram_mipmap_images()) exist.
 *
 * The number returned will never exceed the number of required mipmap images
 * based on the size of the texture and its filter mode.
 *
 * This method is different from get_num_ram_mipmap_images() in that it
 * returns only the number of mipmap levels that can actually be usefully
 * loaded, regardless of the actual number that may be stored.
 */
int Texture::
get_num_loadable_ram_mipmap_images() const {
  CDReader cdata(_cycler);
  if (cdata->_ram_images.empty() || cdata->_ram_images[0]._image.empty()) {
    // If we don't even have a base image, the answer is none.
    return 0;
  }
  if (!uses_mipmaps()) {
    // If we have a base image and don't require mipmapping, the answer is 1.
    return 1;
  }

  // Check that we have enough mipmap levels to meet the size requirements.
  int size = max(cdata->_x_size, max(cdata->_y_size, cdata->_z_size));
  int n = 0;
  int x = 1;
  while (x < size) {
    x = (x << 1);
    ++n;
    if (n >= (int)cdata->_ram_images.size() || cdata->_ram_images[n]._image.empty()) {
      return n;
    }
  }

  ++n;
  return n;
}

/**
 * Returns the system-RAM image data associated with the nth mipmap level, if
 * present.  Returns NULL if the nth mipmap level is not present.
 */
CPTA_uchar Texture::
get_ram_mipmap_image(int n) const {
  CDReader cdata(_cycler);
  if (n < (int)cdata->_ram_images.size() && !cdata->_ram_images[n]._image.empty()) {
    return cdata->_ram_images[n]._image;
  }
  return CPTA_uchar(get_class_type());
}

/**
 * Similiar to get_ram_mipmap_image(), however, in this case the void pointer
 * for the given ram image is returned.  This will be NULL unless it has been
 * explicitly set.
 */
void *Texture::
get_ram_mipmap_pointer(int n) const {
  CDReader cdata(_cycler);
  if (n < (int)cdata->_ram_images.size()) {
    return cdata->_ram_images[n]._pointer_image;
  }
  return nullptr;
}

/**
 * Sets an explicit void pointer as the texture's mipmap image for the
 * indicated level.  This is a special call to direct a texture to reference
 * some external image location, for instance from a webcam input.
 *
 * The texture will henceforth reference this pointer directly, instead of its
 * own internal storage; the user is responsible for ensuring the data at this
 * address remains allocated and valid, and in the correct format, during the
 * lifetime of the texture.
 */
void Texture::
set_ram_mipmap_pointer(int n, void *image, size_t page_size) {
  CDWriter cdata(_cycler, true);
  nassertv(cdata->_ram_image_compression != CM_off || do_get_expected_ram_mipmap_image_size(cdata, n));

  while (n >= (int)cdata->_ram_images.size()) {
    cdata->_ram_images.push_back(RamImage());
  }

  cdata->_ram_images[n]._page_size = page_size;
  // _ram_images[n]._image.clear(); wtf is going on?!
  cdata->_ram_images[n]._pointer_image = image;
  cdata->inc_image_modified();
}

/**
 * Accepts a raw pointer cast as an int, which is then passed to
 * set_ram_mipmap_pointer(); see the documentation for that method.
 *
 * This variant is particularly useful to set an external pointer from a
 * language like Python, which doesn't support void pointers directly.
 */
void Texture::
set_ram_mipmap_pointer_from_int(long long pointer, int n, int page_size) {
  set_ram_mipmap_pointer(n, (void*)pointer, (size_t)page_size);
}

/**
 * Discards the current system-RAM image for the nth mipmap level.
 */
void Texture::
clear_ram_mipmap_image(int n) {
  CDWriter cdata(_cycler, true);
  if (n >= (int)cdata->_ram_images.size()) {
    return;
  }
  cdata->_ram_images[n]._page_size = 0;
  cdata->_ram_images[n]._image.clear();
  cdata->_ram_images[n]._pointer_image = nullptr;
}

/**
 * Returns a modifiable pointer to the internal "simple" texture image.  See
 * set_simple_ram_image().
 */
PTA_uchar Texture::
modify_simple_ram_image() {
  CDWriter cdata(_cycler, true);
  cdata->_simple_image_date_generated = (int32_t)time(nullptr);
  return cdata->_simple_ram_image._image;
}

/**
 * Creates an empty array for the simple ram image of the indicated size, and
 * returns a modifiable pointer to the new array.  See set_simple_ram_image().
 */
PTA_uchar Texture::
new_simple_ram_image(int x_size, int y_size) {
  CDWriter cdata(_cycler, true);
  nassertr(cdata->_texture_type == TT_2d_texture, PTA_uchar());
  size_t expected_page_size = (size_t)(x_size * y_size * 4);

  cdata->_simple_x_size = x_size;
  cdata->_simple_y_size = y_size;
  cdata->_simple_ram_image._image = PTA_uchar::empty_array(expected_page_size);
  cdata->_simple_ram_image._page_size = expected_page_size;
  cdata->_simple_image_date_generated = (int32_t)time(nullptr);
  cdata->inc_simple_image_modified();

  return cdata->_simple_ram_image._image;
}

/**
 * Computes the "simple" ram image by loading the main RAM image, if it is not
 * already available, and reducing it to 16x16 or smaller.  This may be an
 * expensive operation.
 */
void Texture::
generate_simple_ram_image() {
  CDWriter cdata(_cycler, true);

  if (cdata->_texture_type != TT_2d_texture ||
      cdata->_ram_image_compression != CM_off) {
    return;
  }

  PNMImage pnmimage;
  if (!do_store_one(cdata, pnmimage, 0, 0)) {
    return;
  }

  // Start at the suggested size from the config file.
  int x_size = simple_image_size.get_word(0);
  int y_size = simple_image_size.get_word(1);

  // Limit it to no larger than the source image, and also make it a power of
  // two.
  x_size = down_to_power_2(min(x_size, cdata->_x_size));
  y_size = down_to_power_2(min(y_size, cdata->_y_size));

  // Generate a reduced image of that size.
  PNMImage scaled(x_size, y_size, pnmimage.get_num_channels());
  scaled.quick_filter_from(pnmimage);

  // Make sure the reduced image has 4 components, by convention.
  if (!scaled.has_alpha()) {
    scaled.add_alpha();
    scaled.alpha_fill(1.0);
  }
  scaled.set_num_channels(4);

  // Now see if we can go even smaller.
  bool did_anything;
  do {
    did_anything = false;

    // Try to reduce X.
    if (x_size > 1) {
      int new_x_size = (x_size >> 1);
      PNMImage smaller(new_x_size, y_size, 4);
      smaller.quick_filter_from(scaled);
      PNMImage bigger(x_size, y_size, 4);
      bigger.quick_filter_from(smaller);

      if (compare_images(scaled, bigger)) {
        scaled.take_from(smaller);
        x_size = new_x_size;
        did_anything = true;
      }
    }

    // Try to reduce Y.
    if (y_size > 1) {
      int new_y_size = (y_size >> 1);
      PNMImage smaller(x_size, new_y_size, 4);
      smaller.quick_filter_from(scaled);
      PNMImage bigger(x_size, y_size, 4);
      bigger.quick_filter_from(smaller);

      if (compare_images(scaled, bigger)) {
        scaled.take_from(smaller);
        y_size = new_y_size;
        did_anything = true;
      }
    }
  } while (did_anything);

  size_t expected_page_size = (size_t)(x_size * y_size * 4);
  PTA_uchar image = PTA_uchar::empty_array(expected_page_size, get_class_type());
  convert_from_pnmimage(image, expected_page_size, x_size, 0, 0, 0, scaled, 4, 1);

  do_set_simple_ram_image(cdata, image, x_size, y_size);
  cdata->_simple_image_date_generated = (int32_t)time(nullptr);
}

/**
 * Returns a TexturePeeker object that can be used to examine the individual
 * texels stored within this Texture by (u, v) coordinate.
 *
 * If the texture has a ram image resident, that image is used.  If it does
 * not have a full ram image but does have a simple_ram_image resident, that
 * image is used instead.  If neither image is resident the full image is
 * reloaded.
 *
 * Returns NULL if the texture cannot find an image to load, or the texture
 * format is incompatible.
 */
PT(TexturePeeker) Texture::
peek() {
  CDWriter cdata(_cycler, unlocked_ensure_ram_image(true));

  PT(TexturePeeker) peeker = new TexturePeeker(this, cdata);
  if (peeker->is_valid()) {
    return peeker;
  }

  return nullptr;
}

/**
 * Indicates that the texture should be enqueued to be prepared in the
 * indicated prepared_objects at the beginning of the next frame.  This will
 * ensure the texture is already loaded into texture memory if it is expected
 * to be rendered soon.
 *
 * Use this function instead of prepare_now() to preload textures from a user
 * interface standpoint.
 */
PT(AsyncFuture) Texture::
prepare(PreparedGraphicsObjects *prepared_objects) {
  return prepared_objects->enqueue_texture_future(this);
}

/**
 * Returns true if the texture has already been prepared or enqueued for
 * preparation on the indicated GSG, false otherwise.
 */
bool Texture::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  PreparedViews::const_iterator pvi;
  pvi = _prepared_views.find(prepared_objects);
  if (pvi != _prepared_views.end()) {
    return true;
  }
  return prepared_objects->is_texture_queued(this);
}

/**
 * Returns true if the texture needs to be re-loaded onto the indicated GSG,
 * either because its image data is out-of-date, or because it's not fully
 * prepared now.
 */
bool Texture::
was_image_modified(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  CDReader cdata(_cycler);

  PreparedViews::const_iterator pvi;
  pvi = _prepared_views.find(prepared_objects);
  if (pvi != _prepared_views.end()) {
    const Contexts &contexts = (*pvi).second;
    for (int view = 0; view < cdata->_num_views; ++view) {
      Contexts::const_iterator ci;
      ci = contexts.find(view);
      if (ci == contexts.end()) {
        return true;
      }
      TextureContext *tc = (*ci).second;
      if (tc->was_image_modified()) {
        return true;
      }
    }
    return false;
  }
  return true;
}

/**
 * Returns the number of bytes which the texture is reported to consume within
 * graphics memory, for the indicated GSG.  This may return a nonzero value
 * even if the texture is not currently resident; you should also check
 * get_resident() if you want to know how much space the texture is actually
 * consuming right now.
 */
size_t Texture::
get_data_size_bytes(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  CDReader cdata(_cycler);

  PreparedViews::const_iterator pvi;
  size_t total_size = 0;
  pvi = _prepared_views.find(prepared_objects);
  if (pvi != _prepared_views.end()) {
    const Contexts &contexts = (*pvi).second;
    for (int view = 0; view < cdata->_num_views; ++view) {
      Contexts::const_iterator ci;
      ci = contexts.find(view);
      if (ci != contexts.end()) {
        TextureContext *tc = (*ci).second;
        total_size += tc->get_data_size_bytes();
      }
    }
  }

  return total_size;
}

/**
 * Returns true if this Texture was rendered in the most recent frame within
 * the indicated GSG.
 */
bool Texture::
get_active(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  CDReader cdata(_cycler);

  PreparedViews::const_iterator pvi;
  pvi = _prepared_views.find(prepared_objects);
  if (pvi != _prepared_views.end()) {
    const Contexts &contexts = (*pvi).second;
    for (int view = 0; view < cdata->_num_views; ++view) {
      Contexts::const_iterator ci;
      ci = contexts.find(view);
      if (ci != contexts.end()) {
        TextureContext *tc = (*ci).second;
        if (tc->get_active()) {
          return true;
        }
      }
    }
  }
  return false;
}

/**
 * Returns true if this Texture is reported to be resident within graphics
 * memory for the indicated GSG.
 */
bool Texture::
get_resident(PreparedGraphicsObjects *prepared_objects) const {
  MutexHolder holder(_lock);
  CDReader cdata(_cycler);

  PreparedViews::const_iterator pvi;
  pvi = _prepared_views.find(prepared_objects);
  if (pvi != _prepared_views.end()) {
    const Contexts &contexts = (*pvi).second;
    for (int view = 0; view < cdata->_num_views; ++view) {
      Contexts::const_iterator ci;
      ci = contexts.find(view);
      if (ci != contexts.end()) {
        TextureContext *tc = (*ci).second;
        if (tc->get_resident()) {
          return true;
        }
      }
    }
  }
  return false;
}

/**
 * Frees the texture context only on the indicated object, if it exists there.
 * Returns true if it was released, false if it had not been prepared.
 */
bool Texture::
release(PreparedGraphicsObjects *prepared_objects) {
  MutexHolder holder(_lock);
  PreparedViews::iterator pvi;
  pvi = _prepared_views.find(prepared_objects);
  if (pvi != _prepared_views.end()) {
    Contexts temp;
    temp.swap((*pvi).second);
    Contexts::iterator ci;
    for (ci = temp.begin(); ci != temp.end(); ++ci) {
      TextureContext *tc = (*ci).second;
      if (tc != nullptr) {
        prepared_objects->release_texture(tc);
      }
    }
    _prepared_views.erase(pvi);
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_texture(this);
}

/**
 * Frees the context allocated on all objects for which the texture has been
 * declared.  Returns the number of contexts which have been freed.
 */
int Texture::
release_all() {
  MutexHolder holder(_lock);

  // We have to traverse a copy of the _prepared_views list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response to
  // each release_texture(), and we don't want to be modifying the
  // _prepared_views list while we're traversing it.
  PreparedViews temp;
  temp.swap(_prepared_views);
  int num_freed = (int)temp.size();

  PreparedViews::iterator pvi;
  for (pvi = temp.begin(); pvi != temp.end(); ++pvi) {
    PreparedGraphicsObjects *prepared_objects = (*pvi).first;
    Contexts temp;
    temp.swap((*pvi).second);
    Contexts::iterator ci;
    for (ci = temp.begin(); ci != temp.end(); ++ci) {
      TextureContext *tc = (*ci).second;
      if (tc != nullptr) {
        prepared_objects->release_texture(tc);
      }
    }
  }

  return num_freed;
}

/**
 * Not to be confused with write(Filename), this method simply describes the
 * texture properties.
 */
void Texture::
write(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);
  indent(out, indent_level)
    << cdata->_texture_type << " " << get_name();
  if (!cdata->_filename.empty()) {
    out << " (from " << cdata->_filename << ")";
  }
  out << "\n";

  indent(out, indent_level + 2);

  switch (cdata->_texture_type) {
  case TT_1d_texture:
    out << "1-d, " << cdata->_x_size;
    break;

  case TT_2d_texture:
    out << "2-d, " << cdata->_x_size << " x " << cdata->_y_size;
    break;

  case TT_3d_texture:
    out << "3-d, " << cdata->_x_size << " x " << cdata->_y_size << " x " << cdata->_z_size;
    break;

  case TT_2d_texture_array:
    out << "2-d array, " << cdata->_x_size << " x " << cdata->_y_size << " x " << cdata->_z_size;
    break;

  case TT_cube_map:
    out << "cube map, " << cdata->_x_size << " x " << cdata->_y_size;
    break;

  case TT_cube_map_array:
    out << "cube map array, " << cdata->_x_size << " x " << cdata->_y_size << " x " << cdata->_z_size;
    break;

  case TT_buffer_texture:
    out << "buffer, " << cdata->_x_size;
    break;

  case TT_1d_texture_array:
    out << "1-d array, " << cdata->_x_size << " x " << cdata->_y_size;
    break;
  }

  if (cdata->_num_views > 1) {
    out << " (x " << cdata->_num_views << " views)";
  }

  out << " pixels, each " << cdata->_num_components;

  switch (cdata->_component_type) {
  case T_unsigned_byte:
  case T_byte:
    out << " bytes";
    break;

  case T_unsigned_short:
  case T_short:
    out << " shorts";
    break;

  case T_half_float:
    out << " half";
  case T_float:
    out << " floats";
    break;

  case T_unsigned_int_24_8:
  case T_int:
  case T_unsigned_int:
    out << " ints";
    break;

  default:
    break;
  }

  out << ", ";
  switch (cdata->_format) {
  case F_color_index:
    out << "color_index";
    break;
  case F_depth_stencil:
    out << "depth_stencil";
    break;
  case F_depth_component:
    out << "depth_component";
    break;
  case F_depth_component16:
    out << "depth_component16";
    break;
  case F_depth_component24:
    out << "depth_component24";
    break;
  case F_depth_component32:
    out << "depth_component32";
    break;

  case F_rgba:
    out << "rgba";
    break;
  case F_rgbm:
    out << "rgbm";
    break;
  case F_rgba32:
    out << "rgba32";
    break;
  case F_rgba16:
    out << "rgba16";
    break;
  case F_rgba12:
    out << "rgba12";
    break;
  case F_rgba8:
    out << "rgba8";
    break;
  case F_rgba4:
    out << "rgba4";
    break;

  case F_rgb:
    out << "rgb";
    break;
  case F_rgb12:
    out << "rgb12";
    break;
  case F_rgb8:
    out << "rgb8";
    break;
  case F_rgb5:
    out << "rgb5";
    break;
  case F_rgba5:
    out << "rgba5";
    break;
  case F_rgb332:
    out << "rgb332";
    break;

  case F_red:
    out << "red";
    break;
  case F_green:
    out << "green";
    break;
  case F_blue:
    out << "blue";
    break;
  case F_alpha:
    out << "alpha";
    break;
  case F_luminance:
    out << "luminance";
    break;
  case F_luminance_alpha:
    out << "luminance_alpha";
    break;
  case F_luminance_alphamask:
    out << "luminance_alphamask";
    break;

  case F_r16:
    out << "r16";
    break;
  case F_r16i:
    out << "r16i";
    break;

  case F_rg16:
    out << "rg16";
    break;
  case F_rgb16:
    out << "rgb16";
    break;

  case F_srgb:
    out << "srgb";
    break;
  case F_srgb_alpha:
    out << "srgb_alpha";
    break;
  case F_sluminance:
    out << "sluminance";
    break;
  case F_sluminance_alpha:
    out << "sluminance_alpha";
    break;

  case F_r32i:
    out << "r32i";
    break;

  case F_r32:
    out << "r32";
    break;
  case F_rg32:
    out << "rg32";
    break;
  case F_rgb32:
    out << "rgb32";
    break;

  case F_r8i:
    out << "r8i";
    break;
  case F_rg8i:
    out << "rg8i";
    break;
  case F_rgb8i:
    out << "rgb8i";
    break;
  case F_rgba8i:
    out << "rgba8i";
    break;
  case F_r11_g11_b10:
    out << "r11_g11_b10";
    break;
  case F_rgb9_e5:
    out << "rgb9_e5";
    break;
  case F_rgb10_a2:
    out << "rgb10_a2";
    break;

  case F_rg:
    out << "rg";
    break;
  }

  if (cdata->_compression != CM_default) {
    out << ", compression " << cdata->_compression;
  }
  out << "\n";

  indent(out, indent_level + 2);

  cdata->_default_sampler.output(out);

  if (do_has_ram_image(cdata)) {
    indent(out, indent_level + 2)
      << do_get_ram_image_size(cdata) << " bytes in ram, compression "
      << cdata->_ram_image_compression << "\n";

    if (cdata->_ram_images.size() > 1) {
      int count = 0;
      size_t total_size = 0;
      for (size_t n = 1; n < cdata->_ram_images.size(); ++n) {
        if (!cdata->_ram_images[n]._image.empty()) {
          ++count;
          total_size += cdata->_ram_images[n]._image.size();
        } else {
          // Stop at the first gap.
          break;
        }
      }
      indent(out, indent_level + 2)
        << count
        << " mipmap levels also present in ram (" << total_size
        << " bytes).\n";
    }

  } else {
    indent(out, indent_level + 2)
      << "no ram image\n";
  }

  if (!cdata->_simple_ram_image._image.empty()) {
    indent(out, indent_level + 2)
      << "simple image: " << cdata->_simple_x_size << " x "
      << cdata->_simple_y_size << ", "
      << cdata->_simple_ram_image._image.size() << " bytes\n";
  }
}


/**
 * Changes the size of the texture, padding if necessary, and setting the pad
 * region as well.
 */
void Texture::
set_size_padded(int x, int y, int z) {
  CDWriter cdata(_cycler, true);
  if (do_get_auto_texture_scale(cdata) != ATS_none) {
    do_set_x_size(cdata, up_to_power_2(x));
    do_set_y_size(cdata, up_to_power_2(y));

    if (cdata->_texture_type == TT_3d_texture) {
      // Only pad 3D textures.  It does not make sense to do so for cube maps
      // or 2D texture arrays.
      do_set_z_size(cdata, up_to_power_2(z));
    } else {
      do_set_z_size(cdata, z);
    }
  } else {
    do_set_x_size(cdata, x);
    do_set_y_size(cdata, y);
    do_set_z_size(cdata, z);
  }
  do_set_pad_size(cdata,
                  cdata->_x_size - x,
                  cdata->_y_size - y,
                  cdata->_z_size - z);
}

/**
 * Specifies the size of the texture as it exists in its original disk file,
 * before any Panda scaling.
 */
void Texture::
set_orig_file_size(int x, int y, int z) {
  CDWriter cdata(_cycler, true);
  cdata->_orig_file_x_size = x;
  cdata->_orig_file_y_size = y;

  nassertv(z == cdata->_z_size);
}

/**
 * Creates a context for the texture on the particular GSG, if it does not
 * already exist.  Returns the new (or old) TextureContext.  This assumes that
 * the GraphicsStateGuardian is the currently active rendering context and
 * that it is ready to accept new textures.  If this is not necessarily the
 * case, you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a texture does not need to be explicitly prepared by the user before it may
 * be rendered.
 */
TextureContext *Texture::
prepare_now(int view,
            PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);
  CDReader cdata(_cycler);

  // Don't exceed the actual number of views.
  view = max(min(view, cdata->_num_views - 1), 0);

  // Get the list of PreparedGraphicsObjects for this view.
  Contexts &contexts = _prepared_views[prepared_objects];
  Contexts::const_iterator pvi;
  pvi = contexts.find(view);
  if (pvi != contexts.end()) {
    return (*pvi).second;
  }

  TextureContext *tc = prepared_objects->prepare_texture_now(this, view, gsg);
  contexts[view] = tc;

  return tc;
}

/**
 * Returns the smallest power of 2 greater than or equal to value.
 */
int Texture::
up_to_power_2(int value) {
  if (value <= 1) {
    return 1;
  }
  int bit = get_next_higher_bit(((unsigned int)value) - 1);
  return (1 << bit);
}

/**
 * Returns the largest power of 2 less than or equal to value.
 */
int Texture::
down_to_power_2(int value) {
  if (value <= 1) {
    return 1;
  }
  int bit = get_next_higher_bit(((unsigned int)value) >> 1);
  return (1 << bit);
}

/**
 * Asks the PNMImage to change its scale when it reads the image, according to
 * the whims of the Config.prc file.
 *
 * For most efficient results, this method should be called after
 * pnmimage.read_header() has been called, but before pnmimage.read().  This
 * method may also be called after pnmimage.read(), i.e.  when the pnmimage is
 * already loaded; in this case it will rescale the image on the spot.  Also
 * see rescale_texture().
 */
void Texture::
consider_rescale(PNMImage &pnmimage) {
  consider_rescale(pnmimage, get_name(), get_auto_texture_scale());
}

/**
 * Asks the PNMImage to change its scale when it reads the image, according to
 * the whims of the Config.prc file.
 *
 * For most efficient results, this method should be called after
 * pnmimage.read_header() has been called, but before pnmimage.read().  This
 * method may also be called after pnmimage.read(), i.e.  when the pnmimage is
 * already loaded; in this case it will rescale the image on the spot.  Also
 * see rescale_texture().
 */
void Texture::
consider_rescale(PNMImage &pnmimage, const string &name, AutoTextureScale auto_texture_scale) {
  int new_x_size = pnmimage.get_x_size();
  int new_y_size = pnmimage.get_y_size();
  if (adjust_size(new_x_size, new_y_size, name, false, auto_texture_scale)) {
    if (pnmimage.is_valid()) {
      // The image is already loaded.  Rescale on the spot.
      PNMImage new_image(new_x_size, new_y_size, pnmimage.get_num_channels(),
                         pnmimage.get_maxval(), pnmimage.get_type(),
                         pnmimage.get_color_space());
      new_image.quick_filter_from(pnmimage);
      pnmimage.take_from(new_image);
    } else {
      // Rescale while reading.  Some image types (e.g.  jpeg) can take
      // advantage of this.
      pnmimage.set_read_size(new_x_size, new_y_size);
    }
  }
}

/**
 * Returns the indicated TextureType converted to a string word.
 */
string Texture::
format_texture_type(TextureType tt) {
  switch (tt) {
  case TT_1d_texture:
    return "1d_texture";
  case TT_2d_texture:
    return "2d_texture";
  case TT_3d_texture:
    return "3d_texture";
  case TT_2d_texture_array:
    return "2d_texture_array";
  case TT_cube_map:
    return "cube_map";
  case TT_cube_map_array:
    return "cube_map_array";
  case TT_buffer_texture:
    return "buffer_texture";
  case TT_1d_texture_array:
    return "1d_texture_array";
  }
  return "**invalid**";
}

/**
 * Returns the TextureType corresponding to the indicated string word.
 */
Texture::TextureType Texture::
string_texture_type(const string &str) {
  if (cmp_nocase(str, "1d_texture") == 0) {
    return TT_1d_texture;
  } else if (cmp_nocase(str, "2d_texture") == 0) {
    return TT_2d_texture;
  } else if (cmp_nocase(str, "3d_texture") == 0) {
    return TT_3d_texture;
  } else if (cmp_nocase(str, "2d_texture_array") == 0) {
    return TT_2d_texture_array;
  } else if (cmp_nocase(str, "cube_map") == 0) {
    return TT_cube_map;
  } else if (cmp_nocase(str, "cube_map_array") == 0) {
    return TT_cube_map_array;
  } else if (cmp_nocase(str, "buffer_texture") == 0) {
    return TT_buffer_texture;
  }

  gobj_cat->error()
    << "Invalid Texture::TextureType value: " << str << "\n";
  return TT_2d_texture;
}

/**
 * Returns the indicated ComponentType converted to a string word.
 */
string Texture::
format_component_type(ComponentType ct) {
  switch (ct) {
  case T_unsigned_byte:
    return "unsigned_byte";
  case T_unsigned_short:
    return "unsigned_short";
  case T_float:
    return "float";
  case T_unsigned_int_24_8:
    return "unsigned_int_24_8";
  case T_int:
    return "int";
  case T_byte:
    return "unsigned_byte";
  case T_short:
    return "short";
  case T_half_float:
    return "half_float";
  case T_unsigned_int:
    return "unsigned_int";
  }

  return "**invalid**";
}

/**
 * Returns the ComponentType corresponding to the indicated string word.
 */
Texture::ComponentType Texture::
string_component_type(const string &str) {
  if (cmp_nocase(str, "unsigned_byte") == 0) {
    return T_unsigned_byte;
  } else if (cmp_nocase(str, "unsigned_short") == 0) {
    return T_unsigned_short;
  } else if (cmp_nocase(str, "float") == 0) {
    return T_float;
  } else if (cmp_nocase(str, "unsigned_int_24_8") == 0) {
    return T_unsigned_int_24_8;
  } else if (cmp_nocase(str, "int") == 0) {
    return T_int;
  } else if (cmp_nocase(str, "byte") == 0) {
    return T_byte;
  } else if (cmp_nocase(str, "short") == 0) {
    return T_short;
  } else if (cmp_nocase(str, "half_float") == 0) {
    return T_half_float;
  } else if (cmp_nocase(str, "unsigned_int") == 0) {
    return T_unsigned_int;
  }

  gobj_cat->error()
    << "Invalid Texture::ComponentType value: " << str << "\n";
  return T_unsigned_byte;
}

/**
 * Returns the indicated Format converted to a string word.
 */
string Texture::
format_format(Format format) {
  switch (format) {
  case F_depth_stencil:
    return "depth_stencil";
  case F_depth_component:
    return "depth_component";
  case F_depth_component16:
    return "depth_component16";
  case F_depth_component24:
    return "depth_component24";
  case F_depth_component32:
    return "depth_component32";
  case F_color_index:
    return "color_index";
  case F_red:
    return "red";
  case F_green:
    return "green";
  case F_blue:
    return "blue";
  case F_alpha:
    return "alpha";
  case F_rgb:
    return "rgb";
  case F_rgb5:
    return "rgb5";
  case F_rgb8:
    return "rgb8";
  case F_rgb12:
    return "rgb12";
  case F_rgb332:
    return "rgb332";
  case F_rgba:
    return "rgba";
  case F_rgbm:
    return "rgbm";
  case F_rgba4:
    return "rgba4";
  case F_rgba5:
    return "rgba5";
  case F_rgba8:
    return "rgba8";
  case F_rgba12:
    return "rgba12";
  case F_luminance:
    return "luminance";
  case F_luminance_alpha:
    return "luminance_alpha";
  case F_luminance_alphamask:
    return "luminance_alphamask";
  case F_rgba16:
    return "rgba16";
  case F_rgba32:
    return "rgba32";
  case F_r16:
    return "r16";
  case F_r16i:
    return "r16i";
  case F_rg16:
    return "rg16";
  case F_rgb16:
    return "rgb16";
  case F_srgb:
    return "srgb";
  case F_srgb_alpha:
    return "srgb_alpha";
  case F_sluminance:
    return "sluminance";
  case F_sluminance_alpha:
    return "sluminance_alpha";
  case F_r32i:
    return "r32i";
  case F_r32:
    return "r32";
  case F_rg32:
    return "rg32";
  case F_rgb32:
    return "rgb32";
  case F_r8i:
    return "r8i";
  case F_rg8i:
    return "rg8i";
  case F_rgb8i:
    return "rgb8i";
  case F_rgba8i:
    return "rgba8i";
  case F_r11_g11_b10:
    return "r11g11b10";
  case F_rgb9_e5:
    return "rgb9_e5";
  case F_rgb10_a2:
    return "rgb10_a2";
  case F_rg:
    return "rg";
  }
  return "**invalid**";
}

/**
 * Returns the Format corresponding to the indicated string word.
 */
Texture::Format Texture::
string_format(const string &str) {
  if (cmp_nocase(str, "depth_stencil") == 0) {
    return F_depth_stencil;
  } else if (cmp_nocase(str, "depth_component") == 0) {
    return F_depth_component;
  } else if (cmp_nocase(str, "depth_component16") == 0 || cmp_nocase(str, "d16") == 0) {
    return F_depth_component16;
  } else if (cmp_nocase(str, "depth_component24") == 0 || cmp_nocase(str, "d24") == 0) {
    return F_depth_component24;
  } else if (cmp_nocase(str, "depth_component32") == 0 || cmp_nocase(str, "d32") == 0) {
    return F_depth_component32;
  } else if (cmp_nocase(str, "color_index") == 0) {
    return F_color_index;
  } else if (cmp_nocase(str, "red") == 0) {
    return F_red;
  } else if (cmp_nocase(str, "green") == 0) {
    return F_green;
  } else if (cmp_nocase(str, "blue") == 0) {
    return F_blue;
  } else if (cmp_nocase(str, "alpha") == 0) {
    return F_alpha;
  } else if (cmp_nocase(str, "rgb") == 0) {
    return F_rgb;
  } else if (cmp_nocase(str, "rgb5") == 0) {
    return F_rgb5;
  } else if (cmp_nocase(str, "rgb8") == 0 || cmp_nocase(str, "r8g8b8") == 0) {
    return F_rgb8;
  } else if (cmp_nocase(str, "rgb12") == 0) {
    return F_rgb12;
  } else if (cmp_nocase(str, "rgb332") == 0 || cmp_nocase(str, "r3g3b2") == 0) {
    return F_rgb332;
  } else if (cmp_nocase(str, "rgba") == 0) {
    return F_rgba;
  } else if (cmp_nocase(str, "rgbm") == 0) {
    return F_rgbm;
  } else if (cmp_nocase(str, "rgba4") == 0) {
    return F_rgba4;
  } else if (cmp_nocase(str, "rgba5") == 0) {
    return F_rgba5;
  } else if (cmp_nocase(str, "rgba8") == 0 || cmp_nocase(str, "r8g8b8a8") == 0) {
    return F_rgba8;
  } else if (cmp_nocase(str, "rgba12") == 0) {
    return F_rgba12;
  } else if (cmp_nocase(str, "luminance") == 0) {
    return F_luminance;
  } else if (cmp_nocase(str, "luminance_alpha") == 0) {
    return F_luminance_alpha;
  } else if (cmp_nocase(str, "luminance_alphamask") == 0) {
    return F_luminance_alphamask;
  } else if (cmp_nocase(str, "rgba16") == 0 || cmp_nocase(str, "r16g16b16a16") == 0) {
    return F_rgba16;
  } else if (cmp_nocase(str, "rgba32") == 0 || cmp_nocase(str, "r32g32b32a32") == 0) {
    return F_rgba32;
  } else if (cmp_nocase(str, "r16") == 0 || cmp_nocase(str, "red16") == 0) {
    return F_r16;
  } else if (cmp_nocase(str, "r16i") == 0) {
    return F_r16i;
  } else if (cmp_nocase(str, "rg16") == 0 || cmp_nocase(str, "r16g16") == 0) {
    return F_rg16;
  } else if (cmp_nocase(str, "rgb16") == 0 || cmp_nocase(str, "r16g16b16") == 0) {
    return F_rgb16;
  } else if (cmp_nocase(str, "srgb") == 0) {
    return F_srgb;
  } else if (cmp_nocase(str, "srgb_alpha") == 0) {
    return F_srgb_alpha;
  } else if (cmp_nocase(str, "sluminance") == 0) {
    return F_sluminance;
  } else if (cmp_nocase(str, "sluminance_alpha") == 0) {
    return F_sluminance_alpha;
  } else if (cmp_nocase(str, "r32i") == 0) {
    return F_r32i;
  } else if (cmp_nocase(str, "r32") == 0 || cmp_nocase(str, "red32") == 0) {
    return F_r32;
  } else if (cmp_nocase(str, "rg32") == 0 || cmp_nocase(str, "r32g32") == 0) {
    return F_rg32;
  } else if (cmp_nocase(str, "rgb32") == 0 || cmp_nocase(str, "r32g32b32") == 0) {
    return F_rgb32;
  } else if (cmp_nocase(str, "r11g11b10") == 0) {
    return F_r11_g11_b10;
  } else if (cmp_nocase(str, "rgb9_e5") == 0) {
    return F_rgb9_e5;
  } else if (cmp_nocase_uh(str, "rgb10_a2") == 0 || cmp_nocase(str, "r10g10b10a2") == 0) {
    return F_rgb10_a2;
  } else if (cmp_nocase_uh(str, "rg") == 0) {
    return F_rg;
  }

  gobj_cat->error()
    << "Invalid Texture::Format value: " << str << "\n";
  return F_rgba;
}

/**
 * Returns the indicated CompressionMode converted to a string word.
 */
string Texture::
format_compression_mode(CompressionMode cm) {
  switch (cm) {
  case CM_default:
    return "default";
  case CM_off:
    return "off";
  case CM_on:
    return "on";
  case CM_fxt1:
    return "fxt1";
  case CM_dxt1:
    return "dxt1";
  case CM_dxt2:
    return "dxt2";
  case CM_dxt3:
    return "dxt3";
  case CM_dxt4:
    return "dxt4";
  case CM_dxt5:
    return "dxt5";
  case CM_pvr1_2bpp:
    return "pvr1_2bpp";
  case CM_pvr1_4bpp:
    return "pvr1_4bpp";
  case CM_rgtc:
    return "rgtc";
  case CM_etc1:
    return "etc1";
  case CM_etc2:
    return "etc2";
  case CM_eac:
    return "eac";
  }

  return "**invalid**";
}

/**
 * Returns the CompressionMode value associated with the given string
 * representation.
 */
Texture::CompressionMode Texture::
string_compression_mode(const string &str) {
  if (cmp_nocase_uh(str, "default") == 0) {
    return CM_default;
  } else if (cmp_nocase_uh(str, "off") == 0) {
    return CM_off;
  } else if (cmp_nocase_uh(str, "on") == 0) {
    return CM_on;
  } else if (cmp_nocase_uh(str, "fxt1") == 0) {
    return CM_fxt1;
  } else if (cmp_nocase_uh(str, "dxt1") == 0) {
    return CM_dxt1;
  } else if (cmp_nocase_uh(str, "dxt2") == 0) {
    return CM_dxt2;
  } else if (cmp_nocase_uh(str, "dxt3") == 0) {
    return CM_dxt3;
  } else if (cmp_nocase_uh(str, "dxt4") == 0) {
    return CM_dxt4;
  } else if (cmp_nocase_uh(str, "dxt5") == 0) {
    return CM_dxt5;
  } else if (cmp_nocase_uh(str, "pvr1_2bpp") == 0) {
    return CM_pvr1_2bpp;
  } else if (cmp_nocase_uh(str, "pvr1_4bpp") == 0) {
    return CM_pvr1_4bpp;
  } else if (cmp_nocase_uh(str, "rgtc") == 0) {
    return CM_rgtc;
  } else if (cmp_nocase_uh(str, "etc1") == 0) {
    return CM_etc1;
  } else if (cmp_nocase_uh(str, "etc2") == 0) {
    return CM_etc2;
  } else if (cmp_nocase_uh(str, "eac") == 0) {
    return CM_eac;
  }

  gobj_cat->error()
    << "Invalid Texture::CompressionMode value: " << str << "\n";
  return CM_default;
}


/**
 * Returns the indicated QualityLevel converted to a string word.
 */
string Texture::
format_quality_level(QualityLevel ql) {
  switch (ql) {
  case QL_default:
    return "default";
  case QL_fastest:
    return "fastest";
  case QL_normal:
    return "normal";
  case QL_best:
    return "best";
  }

  return "**invalid**";
}

/**
 * Returns the QualityLevel value associated with the given string
 * representation.
 */
Texture::QualityLevel Texture::
string_quality_level(const string &str) {
  if (cmp_nocase(str, "default") == 0) {
    return QL_default;
  } else if (cmp_nocase(str, "fastest") == 0) {
    return QL_fastest;
  } else if (cmp_nocase(str, "normal") == 0) {
    return QL_normal;
  } else if (cmp_nocase(str, "best") == 0) {
    return QL_best;
  }

  gobj_cat->error()
    << "Invalid Texture::QualityLevel value: " << str << "\n";
  return QL_default;
}

/**
 * This method is called by the GraphicsEngine at the beginning of the frame
 * *after* a texture has been successfully uploaded to graphics memory.  It is
 * intended as a callback so the texture can release its RAM image, if
 * _keep_ram_image is false.
 *
 * This is called indirectly when the GSG calls
 * GraphicsEngine::texture_uploaded().
 */
void Texture::
texture_uploaded() {
  CDLockedReader cdata(_cycler);

  if (!keep_texture_ram && !cdata->_keep_ram_image) {
    // Once we have prepared the texture, we can generally safely remove the
    // pixels from main RAM.  The GSG is now responsible for remembering what
    // it looks like.

    CDWriter cdataw(_cycler, cdata, false);
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Dumping RAM for texture " << get_name() << "\n";
    }
    do_clear_ram_image(cdataw);
  }
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this node during the cull traversal.
 */
bool Texture::
has_cull_callback() const {
  return false;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.
 *
 * This is called each time the Texture is discovered applied to a Geom in the
 * traversal.  It should return true if the Geom is visible, false if it
 * should be omitted.
 */
bool Texture::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  return true;
}

/**
 * A factory function to make a new Texture, used to pass to the TexturePool.
 */
PT(Texture) Texture::
make_texture() {
  return new Texture;
}

/**
 * Returns true if the indicated component type is unsigned, false otherwise.
 */
bool Texture::
is_unsigned(Texture::ComponentType ctype) {
  return (ctype == T_unsigned_byte ||
          ctype == T_unsigned_short ||
          ctype == T_unsigned_int_24_8 ||
          ctype == T_unsigned_int);
}

/**
 * Returns true if the indicated compression mode is one of the specific
 * compression types, false otherwise.
 */
bool Texture::
is_specific(Texture::CompressionMode compression) {
  switch (compression) {
  case CM_default:
  case CM_off:
  case CM_on:
    return false;

  default:
    return true;
  }
}

/**
 * Returns true if the indicated format includes alpha, false otherwise.
 */
bool Texture::
has_alpha(Format format) {
  switch (format) {
  case F_alpha:
  case F_rgba:
  case F_rgbm:
  case F_rgba4:
  case F_rgba5:
  case F_rgba8:
  case F_rgba12:
  case F_rgba16:
  case F_rgba32:
  case F_luminance_alpha:
  case F_luminance_alphamask:
  case F_srgb_alpha:
  case F_sluminance_alpha:
  case F_rgba8i:
  case F_rgb10_a2:
    return true;

  default:
    return false;
  }
}

/**
 * Returns true if the indicated format includes a binary alpha only, false
 * otherwise.
 */
bool Texture::
has_binary_alpha(Format format) {
  switch (format) {
  case F_rgbm:
    return true;

  default:
    return false;
  }
}

/**
 * Returns true if the indicated format is in the sRGB color space, false
 * otherwise.
 */
bool Texture::
is_srgb(Format format) {
  switch (format) {
  case F_srgb:
  case F_srgb_alpha:
  case F_sluminance:
  case F_sluminance_alpha:
    return true;

  default:
    return false;
  }
}

/**
 * Computes the proper size of the texture, based on the original size, the
 * filename, and the resizing whims of the config file.
 *
 * x_size and y_size should be loaded with the texture image's original size
 * on disk.  On return, they will be loaded with the texture's in-memory
 * target size.  The return value is true if the size has been adjusted, or
 * false if it is the same.
 */
bool Texture::
adjust_size(int &x_size, int &y_size, const string &name,
            bool for_padding, AutoTextureScale auto_texture_scale) {
  bool exclude = false;
  int num_excludes = exclude_texture_scale.get_num_unique_values();
  for (int i = 0; i < num_excludes && !exclude; ++i) {
    GlobPattern pat(exclude_texture_scale.get_unique_value(i));
    if (pat.matches(name)) {
      exclude = true;
    }
  }

  int new_x_size = x_size;
  int new_y_size = y_size;

  if (!exclude) {
    new_x_size = (int)cfloor(new_x_size * texture_scale + 0.5);
    new_y_size = (int)cfloor(new_y_size * texture_scale + 0.5);

    // Don't auto-scale below 4 in either dimension.  This causes problems for
    // DirectX and texture compression.
    new_x_size = min(max(new_x_size, (int)texture_scale_limit), x_size);
    new_y_size = min(max(new_y_size, (int)texture_scale_limit), y_size);
  }

  AutoTextureScale ats = auto_texture_scale;
  if (ats == ATS_unspecified) {
    ats = get_textures_power_2();
  }
  if (!for_padding && ats == ATS_pad) {
    // If we're not calculating the padding size--that is, we're calculating
    // the initial scaling size instead--then ignore ATS_pad, and treat it the
    // same as ATS_none.
    ats = ATS_none;
  }

  switch (ats) {
  case ATS_down:
    new_x_size = down_to_power_2(new_x_size);
    new_y_size = down_to_power_2(new_y_size);
    break;

  case ATS_up:
  case ATS_pad:
    new_x_size = up_to_power_2(new_x_size);
    new_y_size = up_to_power_2(new_y_size);
    break;

  case ATS_none:
  case ATS_unspecified:
    break;
  }

  ats = textures_square.get_value();
  if (!for_padding && ats == ATS_pad) {
    ats = ATS_none;
  }
  switch (ats) {
  case ATS_down:
    new_x_size = new_y_size = min(new_x_size, new_y_size);
    break;

  case ATS_up:
  case ATS_pad:
    new_x_size = new_y_size = max(new_x_size, new_y_size);
    break;

  case ATS_none:
  case ATS_unspecified:
    break;
  }

  if (!exclude) {
    int max_dimension = max_texture_dimension;

    if (max_dimension < 0) {
      GraphicsStateGuardianBase *gsg = GraphicsStateGuardianBase::get_default_gsg();
      if (gsg != nullptr) {
        max_dimension = gsg->get_max_texture_dimension();
      }
    }

    if (max_dimension > 0) {
      new_x_size = min(new_x_size, (int)max_dimension);
      new_y_size = min(new_y_size, (int)max_dimension);
    }
  }

  if (x_size != new_x_size || y_size != new_y_size) {
    x_size = new_x_size;
    y_size = new_y_size;
    return true;
  }

  return false;
}

/**
 * May be called prior to calling read_txo() or any bam-related Texture-
 * creating callback, to ensure that the proper dynamic libraries for a
 * Texture of the current class type, and the indicated filename, have been
 * already loaded.
 *
 * This is a low-level function that should not normally need to be called
 * directly by the user.
 *
 * Note that for best results you must first create a Texture object of the
 * appropriate class type for your filename, for instance with
 * TexturePool::make_texture().
 */
void Texture::
ensure_loader_type(const Filename &filename) {
  // For a plain Texture type, this doesn't need to do anything.
}

/**
 * Called by TextureContext to give the Texture a chance to mark itself dirty
 * before rendering, if necessary.
 */
void Texture::
reconsider_dirty() {
}

/**
 * Works like adjust_size, but also considers the texture class.  Movie
 * textures, for instance, always pad outwards, regardless of textures-
 * power-2.
 */
bool Texture::
do_adjust_this_size(const CData *cdata, int &x_size, int &y_size, const string &name,
                    bool for_padding) const {
  return adjust_size(x_size, y_size, name, for_padding, cdata->_auto_texture_scale);
}

/**
 * The internal implementation of the various read() methods.
 */
bool Texture::
do_read(CData *cdata, const Filename &fullpath, const Filename &alpha_fullpath,
        int primary_file_num_channels, int alpha_file_channel,
        int z, int n, bool read_pages, bool read_mipmaps,
        const LoaderOptions &options, BamCacheRecord *record) {
  PStatTimer timer(_texture_read_pcollector);

  if (options.get_auto_texture_scale() != ATS_unspecified) {
    cdata->_auto_texture_scale = options.get_auto_texture_scale();
  }

  bool header_only = ((options.get_texture_flags() & (LoaderOptions::TF_preload | LoaderOptions::TF_preload_simple)) == 0);
  if (record != nullptr) {
    header_only = false;
  }

  if ((z == 0 || read_pages) && (n == 0 || read_mipmaps)) {
    // When we re-read the page 0 of the base image, we clear everything and
    // start over.
    do_clear_ram_image(cdata);
  }

  if (is_txo_filename(fullpath)) {
    if (record != nullptr) {
      record->add_dependent_file(fullpath);
    }
    return do_read_txo_file(cdata, fullpath);
  }

  if (is_dds_filename(fullpath)) {
    if (record != nullptr) {
      record->add_dependent_file(fullpath);
    }
    return do_read_dds_file(cdata, fullpath, header_only);
  }

  if (is_ktx_filename(fullpath)) {
    if (record != nullptr) {
      record->add_dependent_file(fullpath);
    }
    return do_read_ktx_file(cdata, fullpath, header_only);
  }

  // If read_pages or read_mipmaps is specified, then z and n actually
  // indicate z_size and n_size, respectively--the numerical limits on which
  // to search for filenames.
  int z_size = z;
  int n_size = n;

  // Certain texture types have an implicit z_size.  If z_size is omitted,
  // choose an appropriate default based on the texture type.
  if (z_size == 0) {
    switch (cdata->_texture_type) {
    case TT_1d_texture:
    case TT_2d_texture:
    case TT_buffer_texture:
      z_size = 1;
      break;

    case TT_cube_map:
      z_size = 6;
      break;

    default:
      break;
    }
  }

  int num_views = 0;
  if (options.get_texture_flags() & LoaderOptions::TF_multiview) {
    // We'll be loading a multiview texture.
    read_pages = true;
    if (options.get_texture_num_views() != 0) {
      num_views = options.get_texture_num_views();
      do_set_num_views(cdata, num_views);
    }
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  if (read_pages && read_mipmaps) {
    // Read a sequence of pages * mipmap levels.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    Filename alpha_fullpath_pattern = Filename::pattern_filename(alpha_fullpath);
    do_set_z_size(cdata, z_size);

    n = 0;
    while (true) {
      // For mipmap level 0, the total number of pages might be determined by
      // the number of files we find.  After mipmap level 0, though, the
      // number of pages is predetermined.
      if (n != 0) {
        z_size = do_get_expected_mipmap_z_size(cdata, n);
      }

      z = 0;

      Filename n_pattern = Filename::pattern_filename(fullpath_pattern.get_filename_index(z));
      Filename alpha_n_pattern = Filename::pattern_filename(alpha_fullpath_pattern.get_filename_index(z));

      if (!n_pattern.has_hash()) {
        gobj_cat.error()
          << "Filename requires two different hash sequences: " << fullpath
          << "\n";
        return false;
      }

      Filename file = n_pattern.get_filename_index(n);
      Filename alpha_file = alpha_n_pattern.get_filename_index(n);

      if ((n_size == 0 && (vfs->exists(file) || n == 0)) ||
          (n_size != 0 && n < n_size)) {
        // Continue through the loop.
      } else {
        // We've reached the end of the mipmap sequence.
        break;
      }

      int num_pages = z_size * num_views;
      while ((num_pages == 0 && (vfs->exists(file) || z == 0)) ||
             (num_pages != 0 && z < num_pages)) {
        if (!do_read_one(cdata, file, alpha_file, z, n, primary_file_num_channels,
                         alpha_file_channel, options, header_only, record)) {
          return false;
        }
        ++z;

        n_pattern = Filename::pattern_filename(fullpath_pattern.get_filename_index(z));
        file = n_pattern.get_filename_index(n);
        alpha_file = alpha_n_pattern.get_filename_index(n);
      }

      if (n == 0 && n_size == 0) {
        // If n_size is not specified, it gets implicitly set after we read
        // the base texture image (which determines the size of the texture).
        n_size = do_get_expected_num_mipmap_levels(cdata);
      }
      ++n;
    }
    cdata->_fullpath = fullpath_pattern;
    cdata->_alpha_fullpath = alpha_fullpath_pattern;

  } else if (read_pages) {
    // Read a sequence of cube map or 3-D texture pages.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    Filename alpha_fullpath_pattern = Filename::pattern_filename(alpha_fullpath);
    if (!fullpath_pattern.has_hash()) {
      gobj_cat.error()
        << "Filename requires a hash mark: " << fullpath
        << "\n";
      return false;
    }

    do_set_z_size(cdata, z_size);
    z = 0;
    Filename file = fullpath_pattern.get_filename_index(z);
    Filename alpha_file = alpha_fullpath_pattern.get_filename_index(z);

    int num_pages = z_size * num_views;
    while ((num_pages == 0 && (vfs->exists(file) || z == 0)) ||
           (num_pages != 0 && z < num_pages)) {
      if (!do_read_one(cdata, file, alpha_file, z, 0, primary_file_num_channels,
                       alpha_file_channel, options, header_only, record)) {
        return false;
      }
      ++z;

      file = fullpath_pattern.get_filename_index(z);
      alpha_file = alpha_fullpath_pattern.get_filename_index(z);
    }
    cdata->_fullpath = fullpath_pattern;
    cdata->_alpha_fullpath = alpha_fullpath_pattern;

  } else if (read_mipmaps) {
    // Read a sequence of mipmap levels.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    Filename alpha_fullpath_pattern = Filename::pattern_filename(alpha_fullpath);
    if (!fullpath_pattern.has_hash()) {
      gobj_cat.error()
        << "Filename requires a hash mark: " << fullpath
        << "\n";
      return false;
    }

    n = 0;
    Filename file = fullpath_pattern.get_filename_index(n);
    Filename alpha_file = alpha_fullpath_pattern.get_filename_index(n);

    while ((n_size == 0 && (vfs->exists(file) || n == 0)) ||
           (n_size != 0 && n < n_size)) {
      if (!do_read_one(cdata, file, alpha_file, z, n,
                       primary_file_num_channels, alpha_file_channel,
                       options, header_only, record)) {
        return false;
      }
      ++n;

      if (n_size == 0 && n >= do_get_expected_num_mipmap_levels(cdata)) {
        // Don't try to read more than the requisite number of mipmap levels
        // (unless the user insisted on it for some reason).
        break;
      }

      file = fullpath_pattern.get_filename_index(n);
      alpha_file = alpha_fullpath_pattern.get_filename_index(n);
    }
    cdata->_fullpath = fullpath_pattern;
    cdata->_alpha_fullpath = alpha_fullpath_pattern;

  } else {
    // Just an ordinary read of one file.
    if (!do_read_one(cdata, fullpath, alpha_fullpath, z, n,
                     primary_file_num_channels, alpha_file_channel,
                     options, header_only, record)) {
      return false;
    }
  }

  cdata->_has_read_pages = read_pages;
  cdata->_has_read_mipmaps = read_mipmaps;
  cdata->_num_mipmap_levels_read = cdata->_ram_images.size();

  if (header_only) {
    // If we were only supposed to be checking the image header information,
    // don't let the Texture think that it's got the image now.
    do_clear_ram_image(cdata);
  } else {
    if ((options.get_texture_flags() & LoaderOptions::TF_preload) != 0) {
      // If we intend to keep the ram image around, consider compressing it
      // etc.
      bool generate_mipmaps = ((options.get_texture_flags() & LoaderOptions::TF_generate_mipmaps) != 0);
      bool allow_compression = ((options.get_texture_flags() & LoaderOptions::TF_allow_compression) != 0);
      do_consider_auto_process_ram_image(cdata, generate_mipmaps || uses_mipmaps(), allow_compression);
    }
  }

  return true;
}

/**
 * Called only from do_read(), this method reads a single image file, either
 * one page or one mipmap level.
 */
bool Texture::
do_read_one(CData *cdata, const Filename &fullpath, const Filename &alpha_fullpath,
            int z, int n, int primary_file_num_channels, int alpha_file_channel,
            const LoaderOptions &options, bool header_only, BamCacheRecord *record) {
  if (record != nullptr) {
    nassertr(!header_only, false);
    record->add_dependent_file(fullpath);
  }

  PNMImage image;
  PfmFile pfm;
  PNMReader *image_reader = image.make_reader(fullpath, nullptr, false);
  if (image_reader == nullptr) {
    gobj_cat.error()
      << "Texture::read() - couldn't read: " << fullpath << endl;
    return false;
  }
  image.copy_header_from(*image_reader);

  AutoTextureScale auto_texture_scale = do_get_auto_texture_scale(cdata);

  // If it's a floating-point image file, read it by default into a floating-
  // point texture.
  bool read_floating_point;
  int texture_load_type = (options.get_texture_flags() & (LoaderOptions::TF_integer | LoaderOptions::TF_float));
  switch (texture_load_type) {
  case LoaderOptions::TF_integer:
    read_floating_point = false;
    break;

  case LoaderOptions::TF_float:
    read_floating_point = true;
    break;

  default:
    // Neither TF_integer nor TF_float was specified; determine which way the
    // texture wants to be loaded.
    read_floating_point = (image_reader->is_floating_point());
    if (!alpha_fullpath.empty()) {
      read_floating_point = false;
    }
  }

  if (header_only || textures_header_only) {
    int x_size = image.get_x_size();
    int y_size = image.get_y_size();
    if (z == 0 && n == 0) {
      cdata->_orig_file_x_size = x_size;
      cdata->_orig_file_y_size = y_size;
    }

    if (textures_header_only) {
      // In this mode, we never intend to load the actual texture image
      // anyway, so we don't even need to make the size right.
      x_size = 1;
      y_size = 1;

    } else {
      adjust_size(x_size, y_size, fullpath.get_basename(), false, auto_texture_scale);
    }

    if (read_floating_point) {
      pfm.clear(x_size, y_size, image.get_num_channels());
    } else {
      image = PNMImage(x_size, y_size, image.get_num_channels(),
                       image.get_maxval(), image.get_type(),
                       image.get_color_space());
      image.fill(0.2, 0.3, 1.0);
      if (image.has_alpha()) {
        image.alpha_fill(1.0);
      }
    }
    delete image_reader;

  } else {
    if (z == 0 && n == 0) {
      int x_size = image.get_x_size();
      int y_size = image.get_y_size();

      cdata->_orig_file_x_size = x_size;
      cdata->_orig_file_y_size = y_size;

      if (adjust_size(x_size, y_size, fullpath.get_basename(), false, auto_texture_scale)) {
        image.set_read_size(x_size, y_size);
      }
    } else {
      image.set_read_size(do_get_expected_mipmap_x_size(cdata, n),
                          do_get_expected_mipmap_y_size(cdata, n));
    }

    if (image.get_x_size() != image.get_read_x_size() ||
        image.get_y_size() != image.get_read_y_size()) {
      gobj_cat.info()
        << "Implicitly rescaling " << fullpath.get_basename() << " from "
        << image.get_x_size() << " by " << image.get_y_size() << " to "
        << image.get_read_x_size() << " by " << image.get_read_y_size()
        << "\n";
    }

    bool success;
    if (read_floating_point) {
      success = pfm.read(image_reader);
    } else {
      success = image.read(image_reader);
    }

    if (!success) {
      gobj_cat.error()
        << "Texture::read() - couldn't read: " << fullpath << endl;
      return false;
    }
    Thread::consider_yield();
  }

  PNMImage alpha_image;
  if (!alpha_fullpath.empty()) {
    PNMReader *alpha_image_reader = alpha_image.make_reader(alpha_fullpath, nullptr, false);
    if (alpha_image_reader == nullptr) {
      gobj_cat.error()
        << "Texture::read() - couldn't read: " << alpha_fullpath << endl;
      return false;
    }
    alpha_image.copy_header_from(*alpha_image_reader);

    if (record != nullptr) {
      record->add_dependent_file(alpha_fullpath);
    }

    if (header_only || textures_header_only) {
      int x_size = image.get_x_size();
      int y_size = image.get_y_size();
      alpha_image = PNMImage(x_size, y_size, alpha_image.get_num_channels(),
                             alpha_image.get_maxval(), alpha_image.get_type(),
                             alpha_image.get_color_space());
      alpha_image.fill(1.0);
      if (alpha_image.has_alpha()) {
        alpha_image.alpha_fill(1.0);
      }
      delete alpha_image_reader;

    } else {
      if (image.get_x_size() != alpha_image.get_x_size() ||
          image.get_y_size() != alpha_image.get_y_size()) {
        gobj_cat.info()
          << "Implicitly rescaling " << alpha_fullpath.get_basename()
          << " from " << alpha_image.get_x_size() << " by "
          << alpha_image.get_y_size() << " to " << image.get_x_size()
          << " by " << image.get_y_size() << "\n";
        alpha_image.set_read_size(image.get_x_size(), image.get_y_size());
      }

      if (!alpha_image.read(alpha_image_reader)) {
        gobj_cat.error()
          << "Texture::read() - couldn't read (alpha): " << alpha_fullpath << endl;
        return false;
      }
      Thread::consider_yield();
    }
  }

  if (z == 0 && n == 0) {
    if (!has_name()) {
      set_name(fullpath.get_basename_wo_extension());
    }
    if (cdata->_filename.empty()) {
      cdata->_filename = fullpath;
      cdata->_alpha_filename = alpha_fullpath;

      // The first time we set the filename via a read() operation, we clear
      // keep_ram_image.  The user can always set it again later if he needs
      // to.
      cdata->_keep_ram_image = false;
    }

    cdata->_fullpath = fullpath;
    cdata->_alpha_fullpath = alpha_fullpath;
  }

  if (!alpha_fullpath.empty()) {
    // The grayscale (alpha channel) image must be the same size as the main
    // image.  This should really have been already guaranteed by the above.
    if (image.get_x_size() != alpha_image.get_x_size() ||
        image.get_y_size() != alpha_image.get_y_size()) {
      gobj_cat.info()
        << "Automatically rescaling " << alpha_fullpath.get_basename()
        << " from " << alpha_image.get_x_size() << " by "
        << alpha_image.get_y_size() << " to " << image.get_x_size()
        << " by " << image.get_y_size() << "\n";

      PNMImage scaled(image.get_x_size(), image.get_y_size(),
                      alpha_image.get_num_channels(),
                      alpha_image.get_maxval(), alpha_image.get_type(),
                      alpha_image.get_color_space());
      scaled.quick_filter_from(alpha_image);
      Thread::consider_yield();
      alpha_image = scaled;
    }
  }

  if (n == 0) {
    consider_downgrade(image, primary_file_num_channels, get_name());
    cdata->_primary_file_num_channels = image.get_num_channels();
    cdata->_alpha_file_channel = 0;
  }

  if (!alpha_fullpath.empty()) {
    // Make the original image a 4-component image by taking the grayscale
    // value from the second image.
    image.add_alpha();

    if (alpha_file_channel == 4 ||
        (alpha_file_channel == 2 && alpha_image.get_num_channels() == 2)) {

      if (!alpha_image.has_alpha()) {
        gobj_cat.error()
          << alpha_fullpath.get_basename() << " has no channel " << alpha_file_channel << ".\n";
      } else {
        // Use the alpha channel.
        for (int x = 0; x < image.get_x_size(); x++) {
          for (int y = 0; y < image.get_y_size(); y++) {
            image.set_alpha(x, y, alpha_image.get_alpha(x, y));
          }
        }
      }
      cdata->_alpha_file_channel = alpha_image.get_num_channels();

    } else if (alpha_file_channel >= 1 && alpha_file_channel <= 3 &&
               alpha_image.get_num_channels() >= 3) {
      // Use the appropriate red, green, or blue channel.
      for (int x = 0; x < image.get_x_size(); x++) {
        for (int y = 0; y < image.get_y_size(); y++) {
          image.set_alpha(x, y, alpha_image.get_channel_val(x, y, alpha_file_channel - 1));
        }
      }
      cdata->_alpha_file_channel = alpha_file_channel;

    } else {
      // Use the grayscale channel.
      for (int x = 0; x < image.get_x_size(); x++) {
        for (int y = 0; y < image.get_y_size(); y++) {
          image.set_alpha(x, y, alpha_image.get_gray(x, y));
        }
      }
      cdata->_alpha_file_channel = 0;
    }
  }

  if (read_floating_point) {
    if (!do_load_one(cdata, pfm, fullpath.get_basename(), z, n, options)) {
      return false;
    }
  } else {
    // Now see if we want to pad the image within a larger power-of-2 image.
    int pad_x_size = 0;
    int pad_y_size = 0;
    if (do_get_auto_texture_scale(cdata) == ATS_pad) {
      int new_x_size = image.get_x_size();
      int new_y_size = image.get_y_size();
      if (do_adjust_this_size(cdata, new_x_size, new_y_size, fullpath.get_basename(), true)) {
        pad_x_size = new_x_size - image.get_x_size();
        pad_y_size = new_y_size - image.get_y_size();
        PNMImage new_image(new_x_size, new_y_size, image.get_num_channels(),
                           image.get_maxval(), image.get_type(),
                           image.get_color_space());
        new_image.copy_sub_image(image, 0, new_y_size - image.get_y_size());
        image.take_from(new_image);
      }
    }

    if (!do_load_one(cdata, image, fullpath.get_basename(), z, n, options)) {
      return false;
    }

    do_set_pad_size(cdata, pad_x_size, pad_y_size, 0);
  }
  return true;
}

/**
 * Internal method to load a single page or mipmap level.
 */
bool Texture::
do_load_one(CData *cdata, const PNMImage &pnmimage, const string &name, int z, int n,
            const LoaderOptions &options) {
  if (cdata->_ram_images.size() <= 1 && n == 0) {
    // A special case for mipmap level 0.  When we load mipmap level 0, unless
    // we already have mipmap levels, it determines the image properties like
    // size and number of components.
    if (!do_reconsider_z_size(cdata, z, options)) {
      return false;
    }
    nassertr(z >= 0 && z < cdata->_z_size * cdata->_num_views, false);

    if (z == 0) {
      ComponentType component_type = T_unsigned_byte;
      xelval maxval = pnmimage.get_maxval();
      if (maxval > 255) {
        component_type = T_unsigned_short;
      }

      if (!do_reconsider_image_properties(cdata, pnmimage.get_x_size(), pnmimage.get_y_size(),
                                          pnmimage.get_num_channels(), component_type,
                                          z, options)) {
        return false;
      }
    }

    do_modify_ram_image(cdata);
    cdata->_loaded_from_image = true;
  }

  do_modify_ram_mipmap_image(cdata, n);

  // Ensure the PNMImage is an appropriate size.
  int x_size = do_get_expected_mipmap_x_size(cdata, n);
  int y_size = do_get_expected_mipmap_y_size(cdata, n);
  if (pnmimage.get_x_size() != x_size ||
      pnmimage.get_y_size() != y_size) {
    gobj_cat.info()
      << "Automatically rescaling " << name;
    if (n != 0) {
      gobj_cat.info(false)
        << " mipmap level " << n;
    }
    gobj_cat.info(false)
      << " from " << pnmimage.get_x_size() << " by "
      << pnmimage.get_y_size() << " to " << x_size << " by "
      << y_size << "\n";

    PNMImage scaled(x_size, y_size, pnmimage.get_num_channels(),
                    pnmimage.get_maxval(), pnmimage.get_type(),
                    pnmimage.get_color_space());
    scaled.quick_filter_from(pnmimage);
    Thread::consider_yield();

    convert_from_pnmimage(cdata->_ram_images[n]._image,
                          do_get_expected_ram_mipmap_page_size(cdata, n),
                          x_size, 0, 0, z, scaled,
                          cdata->_num_components, cdata->_component_width);
  } else {
    // Now copy the pixel data from the PNMImage into our internal
    // cdata->_image component.
    convert_from_pnmimage(cdata->_ram_images[n]._image,
                          do_get_expected_ram_mipmap_page_size(cdata, n),
                          x_size, 0, 0, z, pnmimage,
                          cdata->_num_components, cdata->_component_width);
  }
  Thread::consider_yield();

  return true;
}

/**
 * Internal method to load a single page or mipmap level.
 */
bool Texture::
do_load_one(CData *cdata, const PfmFile &pfm, const string &name, int z, int n,
            const LoaderOptions &options) {
  if (cdata->_ram_images.size() <= 1 && n == 0) {
    // A special case for mipmap level 0.  When we load mipmap level 0, unless
    // we already have mipmap levels, it determines the image properties like
    // size and number of components.
    if (!do_reconsider_z_size(cdata, z, options)) {
      return false;
    }
    nassertr(z >= 0 && z < cdata->_z_size * cdata->_num_views, false);

    if (z == 0) {
      ComponentType component_type = T_float;
      if (!do_reconsider_image_properties(cdata, pfm.get_x_size(), pfm.get_y_size(),
                                          pfm.get_num_channels(), component_type,
                                          z, options)) {
        return false;
      }
    }

    do_modify_ram_image(cdata);
    cdata->_loaded_from_image = true;
  }

  do_modify_ram_mipmap_image(cdata, n);

  // Ensure the PfmFile is an appropriate size.
  int x_size = do_get_expected_mipmap_x_size(cdata, n);
  int y_size = do_get_expected_mipmap_y_size(cdata, n);
  if (pfm.get_x_size() != x_size ||
      pfm.get_y_size() != y_size) {
    gobj_cat.info()
      << "Automatically rescaling " << name;
    if (n != 0) {
      gobj_cat.info(false)
        << " mipmap level " << n;
    }
    gobj_cat.info(false)
      << " from " << pfm.get_x_size() << " by "
      << pfm.get_y_size() << " to " << x_size << " by "
      << y_size << "\n";

    PfmFile scaled(pfm);
    scaled.resize(x_size, y_size);
    Thread::consider_yield();

    convert_from_pfm(cdata->_ram_images[n]._image,
                     do_get_expected_ram_mipmap_page_size(cdata, n), z,
                     scaled, cdata->_num_components, cdata->_component_width);
  } else {
    // Now copy the pixel data from the PfmFile into our internal
    // cdata->_image component.
    convert_from_pfm(cdata->_ram_images[n]._image,
                     do_get_expected_ram_mipmap_page_size(cdata, n), z,
                     pfm, cdata->_num_components, cdata->_component_width);
  }
  Thread::consider_yield();

  return true;
}

/**
 * Internal method to load an image into a section of a texture page or mipmap
 * level.
 */
bool Texture::
do_load_sub_image(CData *cdata, const PNMImage &image, int x, int y, int z, int n) {
  nassertr(n >= 0 && (size_t)n < cdata->_ram_images.size(), false);

  int tex_x_size = do_get_expected_mipmap_x_size(cdata, n);
  int tex_y_size = do_get_expected_mipmap_y_size(cdata, n);
  int tex_z_size = do_get_expected_mipmap_z_size(cdata, n);

  nassertr(x >= 0 && x < tex_x_size, false);
  nassertr(y >= 0 && y < tex_y_size, false);
  nassertr(z >= 0 && z < tex_z_size, false);

  nassertr(image.get_x_size() + x <= tex_x_size, false);
  nassertr(image.get_y_size() + y <= tex_y_size, false);

  // Flip y
  y = cdata->_y_size - (image.get_y_size() + y);

  cdata->inc_image_modified();
  do_modify_ram_mipmap_image(cdata, n);
  convert_from_pnmimage(cdata->_ram_images[n]._image,
                        do_get_expected_ram_mipmap_page_size(cdata, n),
                        tex_x_size, x, y, z, image,
                        cdata->_num_components, cdata->_component_width);

  return true;
}

/**
 * Called internally when read() detects a txo file.  Assumes the lock is
 * already held.
 */
bool Texture::
do_read_txo_file(CData *cdata, const Filename &fullpath) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename = Filename::binary_filename(fullpath);
  PT(VirtualFile) file = vfs->get_file(filename);
  if (file == nullptr) {
    // No such file.
    gobj_cat.error()
      << "Could not find " << fullpath << "\n";
    return false;
  }

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Reading texture object " << filename << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = do_read_txo(cdata, *in, fullpath);
  vfs->close_read_file(in);

  cdata->_fullpath = fullpath;
  cdata->_alpha_fullpath = Filename();
  cdata->_keep_ram_image = false;

  return success;
}

/**
 *
 */
bool Texture::
do_read_txo(CData *cdata, istream &in, const string &filename) {
  PT(Texture) other = make_from_txo(in, filename);
  if (other == nullptr) {
    return false;
  }

  CDReader cdata_other(other->_cycler);
  Namable::operator = (*other);
  do_assign(cdata, other, cdata_other);

  cdata->_loaded_from_image = true;
  cdata->_loaded_from_txo = true;
  cdata->_has_read_pages = false;
  cdata->_has_read_mipmaps = false;
  cdata->_num_mipmap_levels_read = 0;
  return true;
}

/**
 * Called internally when read() detects a DDS file.  Assumes the lock is
 * already held.
 */
bool Texture::
do_read_dds_file(CData *cdata, const Filename &fullpath, bool header_only) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename = Filename::binary_filename(fullpath);
  PT(VirtualFile) file = vfs->get_file(filename);
  if (file == nullptr) {
    // No such file.
    gobj_cat.error()
      << "Could not find " << fullpath << "\n";
    return false;
  }

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Reading DDS file " << filename << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = do_read_dds(cdata, *in, fullpath, header_only);
  vfs->close_read_file(in);

  if (!has_name()) {
    set_name(fullpath.get_basename_wo_extension());
  }

  cdata->_fullpath = fullpath;
  cdata->_alpha_fullpath = Filename();
  cdata->_keep_ram_image = false;

  return success;
}

/**
 *
 */
bool Texture::
do_read_dds(CData *cdata, istream &in, const string &filename, bool header_only) {
  StreamReader dds(in);

  // DDS header (19 words)
  DDSHeader header;
  header.dds_magic = dds.get_uint32();
  header.dds_size = dds.get_uint32();
  header.dds_flags = dds.get_uint32();
  header.height = dds.get_uint32();
  header.width = dds.get_uint32();
  header.pitch = dds.get_uint32();
  header.depth = dds.get_uint32();
  header.num_levels = dds.get_uint32();
  dds.skip_bytes(44);

  // Pixelformat (8 words)
  header.pf.pf_size = dds.get_uint32();
  header.pf.pf_flags = dds.get_uint32();
  header.pf.four_cc = dds.get_uint32();
  header.pf.rgb_bitcount = dds.get_uint32();
  header.pf.r_mask = dds.get_uint32();
  header.pf.g_mask = dds.get_uint32();
  header.pf.b_mask = dds.get_uint32();
  header.pf.a_mask = dds.get_uint32();

  // Caps (4 words)
  header.caps.caps1 = dds.get_uint32();
  header.caps.caps2 = dds.get_uint32();
  header.caps.ddsx = dds.get_uint32();
  dds.skip_bytes(4);

  // Pad out to 32 words
  dds.skip_bytes(4);

  if (header.dds_magic != DDS_MAGIC || (in.fail() || in.eof())) {
    gobj_cat.error()
      << filename << " is not a DDS file.\n";
    return false;
  }

  if ((header.dds_flags & DDSD_MIPMAPCOUNT) == 0) {
    // No bit set means only the base mipmap level.
    header.num_levels = 1;

  } else if (header.num_levels == 0) {
    // Some files seem to have this set to 0 for some reason--existing readers
    // assume 0 means 1.
    header.num_levels = 1;
  }

  TextureType texture_type;
  if (header.caps.caps2 & DDSCAPS2_CUBEMAP) {
    static const unsigned int all_faces =
      (DDSCAPS2_CUBEMAP_POSITIVEX |
       DDSCAPS2_CUBEMAP_POSITIVEY |
       DDSCAPS2_CUBEMAP_POSITIVEZ |
       DDSCAPS2_CUBEMAP_NEGATIVEX |
       DDSCAPS2_CUBEMAP_NEGATIVEY |
       DDSCAPS2_CUBEMAP_NEGATIVEZ);
    if ((header.caps.caps2 & all_faces) != all_faces) {
      gobj_cat.error()
        << filename << " is missing some cube map faces; cannot load.\n";
      return false;
    }
    header.depth = 6;
    texture_type = TT_cube_map;

  } else if (header.caps.caps2 & DDSCAPS2_VOLUME) {
    texture_type = TT_3d_texture;

  } else {
    texture_type = TT_2d_texture;
    header.depth = 1;
  }

  // Determine the function to use to read the DDS image.
  typedef PTA_uchar (*ReadDDSLevelFunc)(Texture *tex, Texture::CData *cdata,
                                        const DDSHeader &header, int n, istream &in);
  ReadDDSLevelFunc func = nullptr;

  Format format = F_rgb;
  ComponentType component_type = T_unsigned_byte;

  do_clear_ram_image(cdata);
  CompressionMode compression = CM_off;

  if ((header.pf.pf_flags & DDPF_FOURCC) != 0 &&
      header.pf.four_cc == 0x30315844) {   // 'DX10'
    // A DirectX 10 style texture, which has an additional header.
    func = read_dds_level_generic_uncompressed;
    unsigned int dxgi_format = dds.get_uint32();
    unsigned int dimension = dds.get_uint32();
    unsigned int misc_flag = dds.get_uint32();
    unsigned int array_size = dds.get_uint32();
    /*unsigned int alpha_mode = */dds.get_uint32();

    switch (dxgi_format) {
    case 2:    // DXGI_FORMAT_R32G32B32A32_FLOAT
      format = F_rgba32;
      component_type = T_float;
      func = read_dds_level_abgr32;
      break;
    case 10:   // DXGI_FORMAT_R16G16B16A16_FLOAT
      format = F_rgba16;
      component_type = T_half_float;
      func = read_dds_level_abgr16;
      break;
    case 11:   // DXGI_FORMAT_R16G16B16A16_UNORM
      format = F_rgba16;
      component_type = T_unsigned_short;
      func = read_dds_level_abgr16;
      break;
    case 16:   // DXGI_FORMAT_R32G32_FLOAT
      format = F_rg32;
      component_type = T_float;
      func = read_dds_level_raw;
      break;
    case 27:   // DXGI_FORMAT_R8G8B8A8_TYPELESS
    case 28:   // DXGI_FORMAT_R8G8B8A8_UNORM
      format = F_rgba8;
      func = read_dds_level_abgr8;
      break;
    case 29:   // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
      format = F_srgb_alpha;
      func = read_dds_level_abgr8;
      break;
    case 30:   // DXGI_FORMAT_R8G8B8A8_UINT
      format = F_rgba8i;
      func = read_dds_level_abgr8;
      break;
    case 31:   // DXGI_FORMAT_R8G8B8A8_SNORM
      format = F_rgba8;
      component_type = T_byte;
      func = read_dds_level_abgr8;
      break;
    case 32:   // DXGI_FORMAT_R8G8B8A8_SINT
      format = F_rgba8i;
      component_type = T_byte;
      func = read_dds_level_abgr8;
      break;
    case 34:   // DXGI_FORMAT_R16G16_FLOAT:
      format = F_rg16;
      component_type = T_half_float;
      func = read_dds_level_raw;
      break;
    case 35:   // DXGI_FORMAT_R16G16_UNORM:
      format = F_rg16;
      component_type = T_unsigned_short;
      func = read_dds_level_raw;
      break;
    case 37:   // DXGI_FORMAT_R16G16_SNORM:
      format = F_rg16;
      component_type = T_short;
      func = read_dds_level_raw;
      break;
    case 40:   // DXGI_FORMAT_D32_FLOAT
      format = F_depth_component32;
      component_type = T_float;
      func = read_dds_level_raw;
      break;
    case 41:   // DXGI_FORMAT_R32_FLOAT
      format = F_r32;
      component_type = T_float;
      func = read_dds_level_raw;
      break;
    case 42:   // DXGI_FORMAT_R32_UINT
      format = F_r32i;
      component_type = T_unsigned_int;
      func = read_dds_level_raw;
      break;
    case 43:   // DXGI_FORMAT_R32_SINT
      format = F_r32i;
      component_type = T_int;
      func = read_dds_level_raw;
      break;
    case 48:   // DXGI_FORMAT_R8G8_TYPELESS
    case 49:   // DXGI_FORMAT_R8G8_UNORM
      format = F_rg;
      break;
    case 50:   // DXGI_FORMAT_R8G8_UINT
      format = F_rg8i;
      break;
    case 51:   // DXGI_FORMAT_R8G8_SNORM
      format = F_rg;
      component_type = T_byte;
      break;
    case 52:   // DXGI_FORMAT_R8G8_SINT
      format = F_rg8i;
      component_type = T_byte;
      break;
    case 54:   // DXGI_FORMAT_R16_FLOAT:
      format = F_r16;
      component_type = T_half_float;
      func = read_dds_level_raw;
      break;
    case 55:   // DXGI_FORMAT_D16_UNORM:
      format = F_depth_component16;
      component_type = T_unsigned_short;
      func = read_dds_level_raw;
      break;
    case 56:   // DXGI_FORMAT_R16_UNORM:
      format = F_r16;
      component_type = T_unsigned_short;
      func = read_dds_level_raw;
      break;
    case 57:   // DXGI_FORMAT_R16_UINT:
      format = F_r16i;
      component_type = T_unsigned_short;
      func = read_dds_level_raw;
      break;
    case 58:   // DXGI_FORMAT_R16_SNORM:
      format = F_r16;
      component_type = T_short;
      func = read_dds_level_raw;
      break;
    case 59:   // DXGI_FORMAT_R16_SINT:
      format = F_r16i;
      component_type = T_short;
      func = read_dds_level_raw;
      break;
    case 60:   // DXGI_FORMAT_R8_TYPELESS
    case 61:   // DXGI_FORMAT_R8_UNORM
      format = F_red;
      break;
    case 62:   // DXGI_FORMAT_R8_UINT
      format = F_r8i;
      break;
    case 63:   // DXGI_FORMAT_R8_SNORM
      format = F_red;
      component_type = T_byte;
      break;
    case 64:   // DXGI_FORMAT_R8_SINT
      format = F_r8i;
      component_type = T_byte;
      break;
    case 65:   // DXGI_FORMAT_A8_UNORM
      format = F_alpha;
      break;
    case 70:   // DXGI_FORMAT_BC1_TYPELESS
    case 71:   // DXGI_FORMAT_BC1_UNORM
      format = F_rgb;
      compression = CM_dxt1;
      func = read_dds_level_bc1;
      break;
    case 72:   // DXGI_FORMAT_BC1_UNORM_SRGB
      format = F_srgb;
      compression = CM_dxt1;
      func = read_dds_level_bc1;
      break;
    case 73:   // DXGI_FORMAT_BC2_TYPELESS
    case 74:   // DXGI_FORMAT_BC2_UNORM
      format = F_rgba;
      compression = CM_dxt3;
      func = read_dds_level_bc2;
      break;
    case 75:   // DXGI_FORMAT_BC2_UNORM_SRGB
      format = F_srgb_alpha;
      compression = CM_dxt3;
      func = read_dds_level_bc2;
      break;
    case 76:   // DXGI_FORMAT_BC3_TYPELESS
    case 77:   // DXGI_FORMAT_BC3_UNORM
      format = F_rgba;
      compression = CM_dxt5;
      func = read_dds_level_bc3;
      break;
    case 78:   // DXGI_FORMAT_BC3_UNORM_SRGB
      format = F_srgb_alpha;
      compression = CM_dxt5;
      func = read_dds_level_bc3;
      break;
    case 79:   // DXGI_FORMAT_BC4_TYPELESS
    case 80:   // DXGI_FORMAT_BC4_UNORM
      format = F_red;
      compression = CM_rgtc;
      func = read_dds_level_bc4;
      break;
    case 82:   // DXGI_FORMAT_BC5_TYPELESS
    case 83:   // DXGI_FORMAT_BC5_UNORM
      format = F_rg;
      compression = CM_rgtc;
      func = read_dds_level_bc5;
      break;
    case 87:   // DXGI_FORMAT_B8G8R8A8_UNORM
    case 90:   // DXGI_FORMAT_B8G8R8A8_TYPELESS
      format = F_rgba8;
      break;
    case 88:   // DXGI_FORMAT_B8G8R8X8_UNORM
    case 92:   // DXGI_FORMAT_B8G8R8X8_TYPELESS
      format = F_rgb8;
      break;
    case 91:   // DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
      format = F_srgb_alpha;
      break;
    case 93:   // DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
      format = F_srgb;
      break;
    case 115:  // DXGI_FORMAT_B4G4R4A4_UNORM
      format = F_rgba4;
      break;
    default:
      gobj_cat.error()
        << filename << ": unsupported DXGI format " << dxgi_format << ".\n";
      return false;
    }

    switch (dimension) {
    case 2:  // DDS_DIMENSION_TEXTURE1D
      texture_type = TT_1d_texture;
      header.depth = 1;
      break;
    case 3:  // DDS_DIMENSION_TEXTURE2D
      if (misc_flag & 0x4) {  // DDS_RESOURCE_MISC_TEXTURECUBE
        if (array_size > 1) {
          texture_type = TT_cube_map_array;
          header.depth = array_size * 6;
        } else {
          texture_type = TT_cube_map;
          header.depth = 6;
        }
      } else {
        if (array_size > 1) {
          texture_type = TT_2d_texture_array;
          header.depth = array_size;
        } else {
          texture_type = TT_2d_texture;
          header.depth = 1;
        }
      }
      break;
    case 4:  // DDS_DIMENSION_TEXTURE3D
      texture_type = TT_3d_texture;
      break;
    default:
      gobj_cat.error()
        << filename << ": unsupported dimension.\n";
      return false;
    }

  } else if (header.pf.pf_flags & DDPF_FOURCC) {
    // Some compressed texture format.
    if (texture_type == TT_3d_texture) {
      gobj_cat.error()
        << filename << ": unsupported compression on 3-d texture.\n";
      return false;
    }

    // Most of the compressed formats support alpha.
    format = F_rgba;
    switch (header.pf.four_cc) {
    case 0x31545844:   // 'DXT1', little-endian.
      compression = CM_dxt1;
      func = read_dds_level_bc1;
      format = F_rgbm;
      break;
    case 0x32545844:   // 'DXT2'
      compression = CM_dxt2;
      func = read_dds_level_bc2;
      break;
    case 0x33545844:   // 'DXT3'
      compression = CM_dxt3;
      func = read_dds_level_bc2;
      break;
    case 0x34545844:   // 'DXT4'
      compression = CM_dxt4;
      func = read_dds_level_bc3;
      break;
    case 0x35545844:   // 'DXT5'
      compression = CM_dxt5;
      func = read_dds_level_bc3;
      break;
    case 0x31495441:   // 'ATI1'
    case 0x55344342:   // 'BC4U'
      compression = CM_rgtc;
      func = read_dds_level_bc4;
      format = F_red;
      break;
    case 0x32495441:   // 'ATI2'
    case 0x55354342:   // 'BC5U'
      compression = CM_rgtc;
      func = read_dds_level_bc5;
      format = F_rg;
      break;
    case 36:   // D3DFMT_A16B16G16R16
      func = read_dds_level_abgr16;
      format = F_rgba16;
      component_type = T_unsigned_short;
      break;
    case 110:  // D3DFMT_Q16W16V16U16
      func = read_dds_level_abgr16;
      format = F_rgba16;
      component_type = T_short;
      break;
    case 113:  // D3DFMT_A16B16G16R16F
      func = read_dds_level_abgr16;
      format = F_rgba16;
      component_type = T_half_float;
      break;
    case 116:  // D3DFMT_A32B32G32R32F
      func = read_dds_level_abgr32;
      format = F_rgba32;
      component_type = T_float;
      break;
    default:
      gobj_cat.error()
        << filename << ": unsupported texture compression (FourCC: 0x"
        << std::hex << header.pf.four_cc << std::dec << ").\n";
      return false;
    }

  } else {
    // An uncompressed texture format.
    func = read_dds_level_generic_uncompressed;

    if (header.pf.pf_flags & DDPF_ALPHAPIXELS) {
      // An uncompressed format that involves alpha.
      format = F_rgba;
      if (header.pf.rgb_bitcount == 32 &&
          header.pf.r_mask == 0x000000ff &&
          header.pf.g_mask == 0x0000ff00 &&
          header.pf.b_mask == 0x00ff0000 &&
          header.pf.a_mask == 0xff000000U) {
        func = read_dds_level_abgr8;
      } else if (header.pf.rgb_bitcount == 32 &&
          header.pf.r_mask == 0x00ff0000 &&
          header.pf.g_mask == 0x0000ff00 &&
          header.pf.b_mask == 0x000000ff &&
          header.pf.a_mask == 0xff000000U) {
        func = read_dds_level_rgba8;

      } else if (header.pf.r_mask != 0 &&
                 header.pf.g_mask == 0 &&
                 header.pf.b_mask == 0) {
        func = read_dds_level_luminance_uncompressed;
        format = F_luminance_alpha;
      }
    } else {
      // An uncompressed format that doesn't involve alpha.
      if (header.pf.rgb_bitcount == 24 &&
          header.pf.r_mask == 0x00ff0000 &&
          header.pf.g_mask == 0x0000ff00 &&
          header.pf.b_mask == 0x000000ff) {
        func = read_dds_level_bgr8;
      } else if (header.pf.rgb_bitcount == 24 &&
                 header.pf.r_mask == 0x000000ff &&
                 header.pf.g_mask == 0x0000ff00 &&
                 header.pf.b_mask == 0x00ff0000) {
        func = read_dds_level_rgb8;

      } else if (header.pf.r_mask != 0 &&
                 header.pf.g_mask == 0 &&
                 header.pf.b_mask == 0) {
        func = read_dds_level_luminance_uncompressed;
        format = F_luminance;
      }
    }
  }

  do_setup_texture(cdata, texture_type, header.width, header.height, header.depth,
                   component_type, format);

  cdata->_orig_file_x_size = cdata->_x_size;
  cdata->_orig_file_y_size = cdata->_y_size;
  cdata->_compression = compression;
  cdata->_ram_image_compression = compression;

  if (!header_only) {
    switch (texture_type) {
    case TT_3d_texture:
      {
        // 3-d textures store all the depth slices for mipmap level 0, then
        // all the depth slices for mipmap level 1, and so on.
        for (int n = 0; n < (int)header.num_levels; ++n) {
          int z_size = do_get_expected_mipmap_z_size(cdata, n);
          pvector<PTA_uchar> pages;
          size_t page_size = 0;
          int z;
          for (z = 0; z < z_size; ++z) {
            PTA_uchar page = func(this, cdata, header, n, in);
            if (page.is_null()) {
              return false;
            }
            nassertr(page_size == 0 || page_size == page.size(), false);
            page_size = page.size();
            pages.push_back(page);
          }
          // Now reassemble the pages into one big image.  Because this is a
          // Microsoft format, the images are stacked in reverse order; re-
          // reverse them.
          PTA_uchar image = PTA_uchar::empty_array(page_size * z_size);
          unsigned char *imagep = (unsigned char *)image.p();
          for (z = 0; z < z_size; ++z) {
            int fz = z_size - 1 - z;
            memcpy(imagep + z * page_size, pages[fz].p(), page_size);
          }

          do_set_ram_mipmap_image(cdata, n, image, page_size);
        }
      }
      break;

    case TT_cube_map:
      {
        // Cube maps store all the mipmap levels for face 0, then all the
        // mipmap levels for face 1, and so on.
        pvector<pvector<PTA_uchar> > pages;
        pages.reserve(6);
        int z, n;
        for (z = 0; z < 6; ++z) {
          pages.push_back(pvector<PTA_uchar>());
          pvector<PTA_uchar> &levels = pages.back();
          levels.reserve(header.num_levels);

          for (n = 0; n < (int)header.num_levels; ++n) {
            PTA_uchar image = func(this, cdata, header, n, in);
            if (image.is_null()) {
              return false;
            }
            levels.push_back(image);
          }
        }

        // Now, for each level, reassemble the pages into one big image.
        // Because this is a Microsoft format, the levels are arranged in a
        // rotated order.
        static const int level_remap[6] = {
          0, 1, 5, 4, 2, 3
        };
        for (n = 0; n < (int)header.num_levels; ++n) {
          size_t page_size = pages[0][n].size();
          PTA_uchar image = PTA_uchar::empty_array(page_size * 6);
          unsigned char *imagep = (unsigned char *)image.p();
          for (z = 0; z < 6; ++z) {
            int fz = level_remap[z];
            nassertr(pages[fz][n].size() == page_size, false);
            memcpy(imagep + z * page_size, pages[fz][n].p(), page_size);
          }

          do_set_ram_mipmap_image(cdata, n, image, page_size);
        }
      }
      break;

    case TT_2d_texture_array:
    case TT_cube_map_array: //TODO: rearrange cube map array faces?
      {
        // Texture arrays store all the mipmap levels for layer 0, then all
        // the mipmap levels for layer 1, and so on.
        pvector<pvector<PTA_uchar> > pages;
        pages.reserve(header.depth);
        int z, n;
        for (z = 0; z < (int)header.depth; ++z) {
          pages.push_back(pvector<PTA_uchar>());
          pvector<PTA_uchar> &levels = pages.back();
          levels.reserve(header.num_levels);

          for (n = 0; n < (int)header.num_levels; ++n) {
            PTA_uchar image = func(this, cdata, header, n, in);
            if (image.is_null()) {
              return false;
            }
            levels.push_back(image);
          }
        }

        // Now, for each level, reassemble the pages into one big image.
        for (n = 0; n < (int)header.num_levels; ++n) {
          size_t page_size = pages[0][n].size();
          PTA_uchar image = PTA_uchar::empty_array(page_size * header.depth);
          unsigned char *imagep = (unsigned char *)image.p();
          for (z = 0; z < (int)header.depth; ++z) {
            nassertr(pages[z][n].size() == page_size, false);
            memcpy(imagep + z * page_size, pages[z][n].p(), page_size);
          }

          do_set_ram_mipmap_image(cdata, n, image, page_size);
        }
      }
      break;

    default:
      // Normal 2-d textures simply store the mipmap levels.
      {
        for (int n = 0; n < (int)header.num_levels; ++n) {
          PTA_uchar image = func(this, cdata, header, n, in);
          if (image.is_null()) {
            return false;
          }
          do_set_ram_mipmap_image(cdata, n, image, 0);
        }
      }
    }
    cdata->_has_read_pages = true;
    cdata->_has_read_mipmaps = true;
    cdata->_num_mipmap_levels_read = cdata->_ram_images.size();
  }

  if (in.fail()) {
    gobj_cat.error()
      << filename << ": truncated DDS file.\n";
    return false;
  }

  cdata->_loaded_from_image = true;
  cdata->_loaded_from_txo = true;

  return true;
}

/**
 * Called internally when read() detects a KTX file.  Assumes the lock is
 * already held.
 */
bool Texture::
do_read_ktx_file(CData *cdata, const Filename &fullpath, bool header_only) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename = Filename::binary_filename(fullpath);
  PT(VirtualFile) file = vfs->get_file(filename);
  if (file == nullptr) {
    // No such file.
    gobj_cat.error()
      << "Could not find " << fullpath << "\n";
    return false;
  }

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Reading KTX file " << filename << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = do_read_ktx(cdata, *in, fullpath, header_only);
  vfs->close_read_file(in);

  if (!has_name()) {
    set_name(fullpath.get_basename_wo_extension());
  }

  cdata->_fullpath = fullpath;
  cdata->_alpha_fullpath = Filename();
  cdata->_keep_ram_image = false;

  return success;
}

/**
 *
 */
bool Texture::
do_read_ktx(CData *cdata, istream &in, const string &filename, bool header_only) {
  StreamReader ktx(in);

  unsigned char magic[12];
  if (ktx.extract_bytes(magic, 12) != 12 ||
      memcmp(magic, "\xABKTX 11\xBB\r\n\x1A\n", 12) != 0) {
    gobj_cat.error()
      << filename << " is not a KTX file.\n";
    return false;
  }

  // See: https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
  uint32_t gl_type, /*type_size,*/ gl_format, internal_format, gl_base_format,
    width, height, depth, num_array_elements, num_faces, num_mipmap_levels,
    kvdata_size;

  bool big_endian;
  if (ktx.get_uint32() == 0x04030201) {
    big_endian = false;
    gl_type = ktx.get_uint32();
    /*type_size = */ktx.get_uint32();
    gl_format = ktx.get_uint32();
    internal_format = ktx.get_uint32();
    gl_base_format = ktx.get_uint32();
    width = ktx.get_uint32();
    height = ktx.get_uint32();
    depth = ktx.get_uint32();
    num_array_elements = ktx.get_uint32();
    num_faces = ktx.get_uint32();
    num_mipmap_levels = ktx.get_uint32();
    kvdata_size = ktx.get_uint32();
  } else {
    big_endian = true;
    gl_type = ktx.get_be_uint32();
    /*type_size = */ktx.get_be_uint32();
    gl_format = ktx.get_be_uint32();
    internal_format = ktx.get_be_uint32();
    gl_base_format = ktx.get_be_uint32();
    width = ktx.get_be_uint32();
    height = ktx.get_be_uint32();
    depth = ktx.get_be_uint32();
    num_array_elements = ktx.get_be_uint32();
    num_faces = ktx.get_be_uint32();
    num_mipmap_levels = ktx.get_be_uint32();
    kvdata_size = ktx.get_be_uint32();
  }

  // Skip metadata section.
  ktx.skip_bytes(kvdata_size);

  ComponentType type;
  CompressionMode compression;
  Format format;
  bool swap_bgr = false;

  if (gl_type == 0 || gl_format == 0) {
    // Compressed texture.
    if (gl_type > 0 || gl_format > 0) {
      gobj_cat.error()
        << "Compressed textures must have both type and format set to 0.\n";
      return false;
    }
    type = T_unsigned_byte;
    compression = CM_on;

    KTXFormat base_format;
    switch ((KTXCompressedFormat)internal_format) {
    case KTX_COMPRESSED_RED:
      format = F_red;
      base_format = KTX_RED;
      break;
    case KTX_COMPRESSED_RG:
      format = F_rg;
      base_format = KTX_RG;
      break;
    case KTX_COMPRESSED_RGB:
      format = F_rgb;
      base_format = KTX_RGB;
      break;
    case KTX_COMPRESSED_RGBA:
      format = F_rgba;
      base_format = KTX_RGBA;
      break;
    case KTX_COMPRESSED_SRGB:
      format = F_srgb;
      base_format = KTX_SRGB;
      break;
    case KTX_COMPRESSED_SRGB_ALPHA:
      format = F_srgb_alpha;
      base_format = KTX_SRGB_ALPHA;
      break;
    case KTX_COMPRESSED_RGB_FXT1_3DFX:
      format = F_rgb;
      base_format = KTX_RGB;
      compression = CM_fxt1;
      break;
    case KTX_COMPRESSED_RGBA_FXT1_3DFX:
      format = F_rgba;
      base_format = KTX_RGBA;
      compression = CM_fxt1;
      break;
    case KTX_COMPRESSED_RGB_S3TC_DXT1:
      format = F_rgb;
      base_format = KTX_RGB;
      compression = CM_dxt1;
      break;
    case KTX_COMPRESSED_RGBA_S3TC_DXT1:
      format = F_rgbm;
      base_format = KTX_RGB;
      compression = CM_dxt1;
      break;
    case KTX_COMPRESSED_RGBA_S3TC_DXT3:
      format = F_rgba;
      base_format = KTX_RGBA;
      compression = CM_dxt3;
      break;
    case KTX_COMPRESSED_RGBA_S3TC_DXT5:
      format = F_rgba;
      base_format = KTX_RGBA;
      compression = CM_dxt5;
      break;
    case KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1:
      format = F_srgb_alpha;
      base_format = KTX_SRGB_ALPHA;
      compression = CM_dxt1;
      break;
    case KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3:
      format = F_srgb_alpha;
      base_format = KTX_SRGB_ALPHA;
      compression = CM_dxt3;
      break;
    case KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5:
      format = F_srgb_alpha;
      base_format = KTX_SRGB_ALPHA;
      compression = CM_dxt5;
      break;
    case KTX_COMPRESSED_SRGB_S3TC_DXT1:
      format = F_srgb;
      base_format = KTX_SRGB;
      compression = CM_dxt1;
      break;
    case KTX_COMPRESSED_RED_RGTC1:
    case KTX_COMPRESSED_SIGNED_RED_RGTC1:
      format = F_red;
      base_format = KTX_RED;
      compression = CM_rgtc;
      break;
    case KTX_COMPRESSED_RG_RGTC2:
    case KTX_COMPRESSED_SIGNED_RG_RGTC2:
      format = F_rg;
      base_format = KTX_RG;
      compression = CM_rgtc;
      break;
    case KTX_ETC1_RGB8:
      format = F_rgb;
      base_format = KTX_RGB;
      compression = CM_etc1;
      break;
    case KTX_ETC1_SRGB8:
      format = F_srgb;
      base_format = KTX_SRGB;
      compression = CM_etc1;
      break;
    case KTX_COMPRESSED_RGB8_ETC2:
      format = F_rgb;
      base_format = KTX_RGB;
      compression = CM_etc2;
      break;
    case KTX_COMPRESSED_SRGB8_ETC2:
      format = F_srgb;
      base_format = KTX_SRGB;
      compression = CM_etc2;
      break;
    case KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      format = F_rgbm;
      base_format = KTX_RGBA;
      compression = CM_etc2;
      break;
    case KTX_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      format = F_rgbm;
      base_format = KTX_SRGB8_ALPHA8;
      compression = CM_etc2;
      break;
    case KTX_COMPRESSED_RGBA8_ETC2_EAC:
      format = F_rgba;
      base_format = KTX_RGBA;
      compression = CM_etc2;
      break;
    case KTX_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
      format = F_srgb_alpha;
      base_format = KTX_SRGB8_ALPHA8;
      compression = CM_etc2;
      break;
    case KTX_COMPRESSED_R11_EAC:
    case KTX_COMPRESSED_SIGNED_R11_EAC:
      format = F_red;
      base_format = KTX_RED;
      compression = CM_eac;
      break;
    case KTX_COMPRESSED_RG11_EAC:
    case KTX_COMPRESSED_SIGNED_RG11_EAC:
      format = F_rg;
      base_format = KTX_RG;
      compression = CM_eac;
      break;
    case KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1:
      format = F_srgb_alpha;
      base_format = KTX_SRGB_ALPHA;
      compression = CM_pvr1_2bpp;
      break;
    case KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1:
      format = F_srgb_alpha;
      base_format = KTX_SRGB_ALPHA;
      compression = CM_pvr1_4bpp;
      break;
    case KTX_COMPRESSED_RGBA_BPTC_UNORM:
    case KTX_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
    case KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
    case KTX_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
    default:
      gobj_cat.error()
        << filename << " has unsupported compressed internal format " << internal_format << "\n";
      return false;
    }

    if (base_format != gl_base_format) {
      gobj_cat.error()
        << filename << " has internal format that is incompatible with base "
           "format (0x" << std::hex << gl_base_format << ", expected 0x"
        << base_format << std::dec << ")\n";
      return false;
    }

  } else {
    // Uncompressed texture.
    compression = CM_off;
    switch ((KTXType)gl_type) {
    case KTX_BYTE:
      type = T_byte;
      break;
    case KTX_UNSIGNED_BYTE:
      type = T_unsigned_byte;
      break;
    case KTX_SHORT:
      type = T_short;
      break;
    case KTX_UNSIGNED_SHORT:
      type = T_unsigned_short;
      break;
    case KTX_INT:
      type = T_int;
      break;
    case KTX_UNSIGNED_INT:
      type = T_unsigned_int;
      break;
    case KTX_FLOAT:
      type = T_float;
      break;
    case KTX_HALF_FLOAT:
      type = T_half_float;
      break;
    case KTX_UNSIGNED_INT_24_8:
      type = T_unsigned_int_24_8;
      break;
    default:
      gobj_cat.error()
        << filename << " has unsupported component type " << gl_type << "\n";
      return false;
    }

    if (gl_format != gl_base_format) {
      gobj_cat.error()
        << filename << " has mismatched formats: " << gl_format << " != "
        << gl_base_format << "\n";
    }

    switch (gl_format) {
    case KTX_DEPTH_COMPONENT:
      switch (internal_format) {
      case KTX_DEPTH_COMPONENT:
        format = F_depth_component;
        break;
      case KTX_DEPTH_COMPONENT16:
        format = F_depth_component16;
        break;
      case KTX_DEPTH_COMPONENT24:
        format = F_depth_component24;
        break;
      case KTX_DEPTH_COMPONENT32:
      case KTX_DEPTH_COMPONENT32F:
        format = F_depth_component32;
        break;
      default:
        format = F_depth_component;
        gobj_cat.warning()
          << filename << " has unsupported depth component format " << internal_format << "\n";
      }
      break;

    case KTX_DEPTH_STENCIL:
      format = F_depth_stencil;
      if (internal_format != KTX_DEPTH_STENCIL &&
          internal_format != KTX_DEPTH24_STENCIL8) {
        gobj_cat.warning()
          << filename << " has unsupported depth stencil format " << internal_format << "\n";
      }
      break;

    case KTX_RED:
      switch (internal_format) {
      case KTX_RED:
      case KTX_RED_SNORM:
      case KTX_R8:
      case KTX_R8_SNORM:
        format = F_red;
        break;
      case KTX_R16:
      case KTX_R16_SNORM:
      case KTX_R16F:
        format = F_r16;
        break;
      case KTX_R32F:
        format = F_r32;
        break;
      default:
        format = F_red;
        gobj_cat.warning()
          << filename << " has unsupported red format " << internal_format << "\n";
      }
      break;

    case KTX_RED_INTEGER:
      switch (internal_format) {
      case KTX_R8I:
      case KTX_R8UI:
        format = F_r8i;
        break;
      case KTX_R16I:
      case KTX_R16UI:
        format = F_r16i;
        break;
      case KTX_R32I:
      case KTX_R32UI:
        format = F_r32i;
        break;
      default:
        gobj_cat.error()
          << filename << " has unsupported red integer format " << internal_format << "\n";
        return false;
      }
      break;

    case KTX_GREEN:
      format = F_green;
      if (internal_format != KTX_GREEN) {
        gobj_cat.warning()
          << filename << " has unsupported green format " << internal_format << "\n";
      }
      break;

    case KTX_BLUE:
      format = F_blue;
      if (internal_format != KTX_BLUE) {
        gobj_cat.warning()
          << filename << " has unsupported blue format " << internal_format << "\n";
      }
      break;

    case KTX_RG:
      switch (internal_format) {
      case KTX_RG:
      case KTX_RG_SNORM:
      case KTX_RG8:
      case KTX_RG8_SNORM:
        format = F_rg;
        break;
      case KTX_RG16:
      case KTX_RG16_SNORM:
      case KTX_RG16F:
        format = F_rg16;
        break;
      case KTX_RG32F:
        format = F_rg32;
        break;
      default:
        format = F_rg;
        gobj_cat.warning()
          << filename << " has unsupported RG format " << internal_format << "\n";
      }
      break;

    case KTX_RG_INTEGER:
      switch (internal_format) {
      case KTX_RG8I:
      case KTX_RG8UI:
        format = F_rg8i;
        break;
      case KTX_RG16I:
      case KTX_RG16UI:
      case KTX_RG32I:
      case KTX_RG32UI:
      default:
        gobj_cat.error()
          << filename << " has unsupported RG integer format " << internal_format << "\n";
        return false;
      }
      break;

    case KTX_RGB:
      swap_bgr = true;
    case KTX_BGR:
      switch (internal_format) {
      case KTX_RGB:
      case KTX_RGB_SNORM:
        format = F_rgb;
        break;
      case KTX_RGB5:
        format = F_rgb5;
        break;
      case KTX_RGB12:
        format = F_rgb12;
        break;
      case KTX_R3_G3_B2:
        format = F_rgb332;
        break;
      case KTX_RGB9_E5:
        format = F_rgb9_e5;
        break;
      case KTX_R11F_G11F_B10F:
        format = F_r11_g11_b10;
        break;
      case KTX_RGB8:
      case KTX_RGB8_SNORM:
        format = F_rgb8;
        break;
      case KTX_RGB16:
      case KTX_RGB16_SNORM:
      case KTX_RGB16F:
        format = F_rgb16;
        break;
      case KTX_RGB32F:
        format = F_rgb32;
        break;
      case KTX_SRGB:
      case KTX_SRGB8:
        format = F_srgb;
        break;
      default:
        format = F_rgb;
        gobj_cat.warning()
          << filename << " has unsupported RGB format " << internal_format << "\n";
      }
      break;

    case KTX_RGB_INTEGER:
      swap_bgr = true;
    case KTX_BGR_INTEGER:
      switch (internal_format) {
      case KTX_RGB8I:
      case KTX_RGB8UI:
        format = F_rgb8i;
        break;
      case KTX_RGB16I:
      case KTX_RGB16UI:
      case KTX_RGB32I:
      case KTX_RGB32UI:
      default:
        gobj_cat.error()
          << filename << " has unsupported RGB integer format " << internal_format << "\n";
        return false;
      }
      break;

    case KTX_RGBA:
      swap_bgr = true;
    case KTX_BGRA:
      switch (internal_format) {
      case KTX_RGBA:
      case KTX_RGBA_SNORM:
        format = F_rgba;
        break;
      case KTX_RGBA4:
        format = F_rgba4;
        break;
      case KTX_RGB5_A1:
        format = F_rgba5;
        break;
      case KTX_RGBA12:
        format = F_rgba12;
        break;
      case KTX_RGB10_A2:
        format = F_rgb10_a2;
        break;
      case KTX_RGBA8:
      case KTX_RGBA8_SNORM:
        format = F_rgba8;
        break;
      case KTX_RGBA16:
      case KTX_RGBA16_SNORM:
      case KTX_RGBA16F:
        format = F_rgba16;
        break;
      case KTX_RGBA32F:
        format = F_rgba32;
        break;
      case KTX_SRGB_ALPHA:
      case KTX_SRGB8_ALPHA8:
        format = F_srgb_alpha;
        break;
      default:
        format = F_rgba;
        gobj_cat.warning()
          << filename << " has unsupported RGBA format " << internal_format << "\n";
      }
      break;
      break;

    case KTX_RGBA_INTEGER:
      swap_bgr = true;
    case KTX_BGRA_INTEGER:
      switch (internal_format) {
      case KTX_RGBA8I:
      case KTX_RGBA8UI:
        format = F_rgba8i;
        break;
      case KTX_RGBA16I:
      case KTX_RGBA16UI:
      case KTX_RGBA32I:
      case KTX_RGBA32UI:
      default:
        gobj_cat.error()
          << filename << " has unsupported RGBA integer format " << internal_format << "\n";
        return false;
      }
      break;

    case KTX_LUMINANCE:
      format = F_luminance;
      break;

    case KTX_LUMINANCE_ALPHA:
      format = F_luminance_alpha;
      break;

    case KTX_ALPHA:
      format = F_alpha;
      break;

    case KTX_STENCIL_INDEX:
    default:
      gobj_cat.error()
        << filename << " has unsupported format " << gl_format << "\n";
      return false;
    }
  }

  TextureType texture_type;
  if (depth > 0) {
    texture_type = TT_3d_texture;

  } else if (num_faces > 1) {
    if (num_faces != 6) {
      gobj_cat.error()
        << filename << " has " << num_faces << " cube map faces, expected 6\n";
      return false;
    }
    if (width != height) {
      gobj_cat.error()
        << filename << " is cube map, but does not have square dimensions\n";
      return false;
    }
    if (num_array_elements > 0) {
      depth = num_array_elements * 6;
      texture_type = TT_cube_map_array;
    } else {
      depth = 6;
      texture_type = TT_cube_map;
    }

  } else if (height > 0) {
    if (num_array_elements > 0) {
      depth = num_array_elements;
      texture_type = TT_2d_texture_array;
    } else {
      depth = 1;
      texture_type = TT_2d_texture;
    }

  } else if (width > 0) {
    depth = 1;
    if (num_array_elements > 0) {
      height = num_array_elements;
      texture_type = TT_1d_texture_array;
    } else {
      height = 1;
      texture_type = TT_1d_texture;
    }

  } else {
    gobj_cat.error()
      << filename << " has zero size\n";
    return false;
  }

  do_setup_texture(cdata, texture_type, width, height, depth, type, format);

  cdata->_orig_file_x_size = cdata->_x_size;
  cdata->_orig_file_y_size = cdata->_y_size;
  cdata->_compression = compression;
  cdata->_ram_image_compression = compression;

  if (!header_only) {
    bool generate_mipmaps = false;
    if (num_mipmap_levels == 0) {
      generate_mipmaps = true;
      num_mipmap_levels = 1;
    }

    for (uint32_t n = 0; n < num_mipmap_levels; ++n) {
      uint32_t image_size;
      if (big_endian) {
        image_size = ktx.get_be_uint32();
      } else {
        image_size = ktx.get_uint32();
      }
      PTA_uchar image;

      if (compression == CM_off) {
        uint32_t row_size = do_get_expected_mipmap_x_size(cdata, (int)n) * cdata->_num_components * cdata->_component_width;
        uint32_t num_rows = do_get_expected_mipmap_y_size(cdata, (int)n) * do_get_expected_mipmap_z_size(cdata, (int)n);
        uint32_t row_padded = (row_size + 3) & ~3;

        if (image_size == row_size * num_rows) {
          if (row_padded != row_size) {
            // Someone tightly packed the image.  This is invalid, but because
            // we like it tightly packed too, we'll read it anyway.
            gobj_cat.warning()
              << filename << " does not have proper row padding for mipmap "
                             "level " << n << "\n";
          }
          image = PTA_uchar::empty_array(image_size);
          ktx.extract_bytes(image.p(), image_size);

        } else if (image_size != row_padded * num_rows) {
          gobj_cat.error()
            << filename << " has invalid image size " << image_size
            << " for mipmap level " << n << " (expected "
            << row_padded * num_rows << ")\n";
          return false;

        } else {
          // Read it row by row.
          image = PTA_uchar::empty_array(row_size * num_rows);
          uint32_t skip = row_padded - row_size;
          unsigned char *p = image.p();
          for (uint32_t row = 0; row < num_rows; ++row) {
            ktx.extract_bytes(p, row_size);
            ktx.skip_bytes(skip);
            p += row_size;
          }
        }

        // Swap red and blue channels if necessary to match Panda conventions.
        if (swap_bgr) {
          unsigned char *begin = image.p();
          const unsigned char *end = image.p() + image.size();
          size_t skip = cdata->_num_components;
          nassertr(skip == 3 || skip == 4, false);

          switch (cdata->_component_width) {
          case 1:
            for (unsigned char *p = begin; p < end; p += skip) {
              swap(p[0], p[2]);
            }
            break;
          case 2:
            for (short *p = (short *)begin; p < (short *)end; p += skip) {
              swap(p[0], p[2]);
            }
            break;
          case 4:
            for (int *p = (int *)begin; p < (int *)end; p += skip) {
              swap(p[0], p[2]);
            }
            break;
          default:
            nassert_raise("unexpected channel count");
            return false;
          }
        }

        do_set_ram_mipmap_image(cdata, (int)n, std::move(image),
          row_size * do_get_expected_mipmap_y_size(cdata, (int)n));

      } else {
        // Compressed image.  We'll trust that the file has the right size.
        image = PTA_uchar::empty_array(image_size);
        ktx.extract_bytes(image.p(), image_size);
        do_set_ram_mipmap_image(cdata, (int)n, std::move(image), image_size / depth);
      }

      ktx.skip_bytes(3 - ((image_size + 3) & 3));
    }

    cdata->_has_read_pages = true;
    cdata->_has_read_mipmaps = true;
    cdata->_num_mipmap_levels_read = cdata->_ram_images.size();

    if (generate_mipmaps) {
      do_generate_ram_mipmap_images(cdata, false);
    }
  }

  if (in.fail()) {
    gobj_cat.error()
      << filename << ": truncated KTX file.\n";
    return false;
  }

  cdata->_loaded_from_image = true;
  cdata->_loaded_from_txo = true;

  return true;
}

/**
 * Internal method to write a series of pages and/or mipmap levels to disk
 * files.
 */
bool Texture::
do_write(CData *cdata,
         const Filename &fullpath, int z, int n, bool write_pages, bool write_mipmaps) {
  if (is_txo_filename(fullpath)) {
    if (!do_has_bam_rawdata(cdata)) {
      do_get_bam_rawdata(cdata);
    }
    nassertr(do_has_bam_rawdata(cdata), false);
    return do_write_txo_file(cdata, fullpath);
  }

  if (!do_has_uncompressed_ram_image(cdata)) {
    do_get_uncompressed_ram_image(cdata);
  }

  nassertr(do_has_ram_mipmap_image(cdata, n), false);
  nassertr(cdata->_ram_image_compression == CM_off, false);

  if (write_pages && write_mipmaps) {
    // Write a sequence of pages * mipmap levels.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    int num_levels = cdata->_ram_images.size();

    for (int n = 0; n < num_levels; ++n) {
      int num_pages = do_get_expected_mipmap_num_pages(cdata, n);

      for (z = 0; z < num_pages; ++z) {
        Filename n_pattern = Filename::pattern_filename(fullpath_pattern.get_filename_index(z));

        if (!n_pattern.has_hash()) {
          gobj_cat.error()
            << "Filename requires two different hash sequences: " << fullpath
            << "\n";
          return false;
        }

        if (!do_write_one(cdata, n_pattern.get_filename_index(n), z, n)) {
          return false;
        }
      }
    }

  } else if (write_pages) {
    // Write a sequence of pages.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    if (!fullpath_pattern.has_hash()) {
      gobj_cat.error()
        << "Filename requires a hash mark: " << fullpath
        << "\n";
      return false;
    }

    int num_pages = cdata->_z_size * cdata->_num_views;
    for (z = 0; z < num_pages; ++z) {
      if (!do_write_one(cdata, fullpath_pattern.get_filename_index(z), z, n)) {
        return false;
      }
    }

  } else if (write_mipmaps) {
    // Write a sequence of mipmap images.
    Filename fullpath_pattern = Filename::pattern_filename(fullpath);
    if (!fullpath_pattern.has_hash()) {
      gobj_cat.error()
        << "Filename requires a hash mark: " << fullpath
        << "\n";
      return false;
    }

    int num_levels = cdata->_ram_images.size();
    for (int n = 0; n < num_levels; ++n) {
      if (!do_write_one(cdata, fullpath_pattern.get_filename_index(n), z, n)) {
        return false;
      }
    }

  } else {
    // Write a single file.
    if (!do_write_one(cdata, fullpath, z, n)) {
      return false;
    }
  }

  return true;
}

/**
 * Internal method to write the indicated page and mipmap level to a disk
 * image file.
 */
bool Texture::
do_write_one(CData *cdata, const Filename &fullpath, int z, int n) {
  if (!do_has_ram_mipmap_image(cdata, n)) {
    return false;
  }

  nassertr(cdata->_ram_image_compression == CM_off, false);

  bool success;
  if (cdata->_component_type == T_float) {
    // Writing a floating-point texture.
    PfmFile pfm;
    if (!do_store_one(cdata, pfm, z, n)) {
      return false;
    }
    success = pfm.write(fullpath);
  } else {
    // Writing a normal, integer texture.
    PNMImage pnmimage;
    if (!do_store_one(cdata, pnmimage, z, n)) {
      return false;
    }
    success = pnmimage.write(fullpath);
  }

  if (!success) {
    gobj_cat.error()
      << "Texture::write() - couldn't write: " << fullpath << endl;
    return false;
  }

  return true;
}

/**
 * Internal method to copy a page and/or mipmap level to a PNMImage.
 */
bool Texture::
do_store_one(CData *cdata, PNMImage &pnmimage, int z, int n) {
  // First, reload the ram image if necessary.
  do_get_uncompressed_ram_image(cdata);

  if (!do_has_ram_mipmap_image(cdata, n)) {
    return false;
  }

  nassertr(z >= 0 && z < do_get_expected_mipmap_num_pages(cdata, n), false);
  nassertr(cdata->_ram_image_compression == CM_off, false);

  if (cdata->_component_type == T_float) {
    // PNMImage by way of PfmFile.
    PfmFile pfm;
    bool success = convert_to_pfm(pfm,
                                  do_get_expected_mipmap_x_size(cdata, n),
                                  do_get_expected_mipmap_y_size(cdata, n),
                                  cdata->_num_components, cdata->_component_width,
                                  cdata->_ram_images[n]._image,
                                  do_get_ram_mipmap_page_size(cdata, n), z);
    if (!success) {
      return false;
    }
    return pfm.store(pnmimage);
  }

  return convert_to_pnmimage(pnmimage,
                             do_get_expected_mipmap_x_size(cdata, n),
                             do_get_expected_mipmap_y_size(cdata, n),
                             cdata->_num_components, cdata->_component_type,
                             is_srgb(cdata->_format),
                             cdata->_ram_images[n]._image,
                             do_get_ram_mipmap_page_size(cdata, n), z);
}

/**
 * Internal method to copy a page and/or mipmap level to a PfmFile.
 */
bool Texture::
do_store_one(CData *cdata, PfmFile &pfm, int z, int n) {
  // First, reload the ram image if necessary.
  do_get_uncompressed_ram_image(cdata);

  if (!do_has_ram_mipmap_image(cdata, n)) {
    return false;
  }

  nassertr(z >= 0 && z < do_get_expected_mipmap_num_pages(cdata, n), false);
  nassertr(cdata->_ram_image_compression == CM_off, false);

  if (cdata->_component_type != T_float) {
    // PfmFile by way of PNMImage.
    PNMImage pnmimage;
    bool success =
      convert_to_pnmimage(pnmimage,
                          do_get_expected_mipmap_x_size(cdata, n),
                          do_get_expected_mipmap_y_size(cdata, n),
                          cdata->_num_components, cdata->_component_type,
                          is_srgb(cdata->_format),
                          cdata->_ram_images[n]._image,
                          do_get_ram_mipmap_page_size(cdata, n), z);
    if (!success) {
      return false;
    }
    return pfm.load(pnmimage);
  }

  return convert_to_pfm(pfm,
                        do_get_expected_mipmap_x_size(cdata, n),
                        do_get_expected_mipmap_y_size(cdata, n),
                        cdata->_num_components, cdata->_component_width,
                        cdata->_ram_images[n]._image,
                        do_get_ram_mipmap_page_size(cdata, n), z);
}

/**
 * Called internally when write() detects a txo filename.
 */
bool Texture::
do_write_txo_file(const CData *cdata, const Filename &fullpath) const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename filename = Filename::binary_filename(fullpath);
  ostream *out = vfs->open_write_file(filename, true, true);
  if (out == nullptr) {
    gobj_cat.error()
      << "Unable to open " << filename << "\n";
    return false;
  }

  bool success = do_write_txo(cdata, *out, fullpath);
  vfs->close_write_file(out);
  return success;
}

/**
 *
 */
bool Texture::
do_write_txo(const CData *cdata, ostream &out, const string &filename) const {
  DatagramOutputFile dout;

  if (!dout.open(out, filename)) {
    gobj_cat.error()
      << "Could not write texture object: " << filename << "\n";
    return false;
  }

  if (!dout.write_header(_bam_header)) {
    gobj_cat.error()
      << "Unable to write to " << filename << "\n";
    return false;
  }

  BamWriter writer(&dout);
  if (!writer.init()) {
    return false;
  }

  writer.set_file_texture_mode(BamWriter::BTM_rawdata);

  if (!writer.write_object(this)) {
    return false;
  }

  if (!do_has_bam_rawdata(cdata)) {
    gobj_cat.error()
      << get_name() << " does not have ram image\n";
    return false;
  }

  return true;
}

/**
 * If the texture has a ram image already, this acquires the CData write lock
 * and returns it.
 *
 * If the texture lacks a ram image, this performs do_reload_ram_image(), but
 * without holding the lock on this particular Texture object, to avoid
 * holding the lock across what might be a slow operation.  Instead, the
 * reload is performed in a copy of the texture object, and then the lock is
 * acquired and the data is copied in.
 *
 * In any case, the return value is a locked CData object, which must be
 * released with an explicit call to release_write().  The CData object will
 * have a ram image unless for some reason do_reload_ram_image() fails.
 */
Texture::CData *Texture::
unlocked_ensure_ram_image(bool allow_compression) {
  Thread *current_thread = Thread::get_current_thread();

  // First, wait for any other threads that might be simultaneously performing
  // the same operation.
  MutexHolder holder(_lock);
  while (_reloading) {
    _cvar.wait();
  }

  // Then make sure we still need to reload before continuing.
  const CData *cdata = _cycler.read(current_thread);
  bool has_ram_image = do_has_ram_image(cdata);
  if (has_ram_image && !allow_compression && cdata->_ram_image_compression != Texture::CM_off) {
    // If we don't want compression, but the ram image we have is pre-
    // compressed, we don't consider it.
    has_ram_image = false;
  }
  if (has_ram_image || !do_can_reload(cdata)) {
    // We don't need to reload after all, or maybe we can't reload anyway.
    // Return, but elevate the lock first, as we promised.
    return _cycler.elevate_read_upstream(cdata, false, current_thread);
  }

  // We need to reload.
  nassertr(!_reloading, nullptr);
  _reloading = true;

  PT(Texture) tex = do_make_copy(cdata);
  _cycler.release_read(cdata);
  _lock.unlock();

  // Perform the actual reload in a copy of the texture, while our own mutex
  // is left unlocked.
  CDWriter cdata_tex(tex->_cycler, true);
  tex->do_reload_ram_image(cdata_tex, allow_compression);

  _lock.lock();

  CData *cdataw = _cycler.write_upstream(false, current_thread);

  // Rather than calling do_assign(), which would copy *all* of the reloaded
  // texture's properties over, we only copy in the ones which are relevant to
  // the ram image.  This way, if the properties have changed during the
  // reload (for instance, because we reloaded a txo), it won't contaminate
  // the original texture.
  cdataw->_orig_file_x_size = cdata_tex->_orig_file_x_size;
  cdataw->_orig_file_y_size = cdata_tex->_orig_file_y_size;

  // If any of *these* properties have changed, the texture has changed in
  // some fundamental way.  Update it appropriately.
  if (cdata_tex->_x_size != cdataw->_x_size ||
      cdata_tex->_y_size != cdataw->_y_size ||
      cdata_tex->_z_size != cdataw->_z_size ||
      cdata_tex->_num_views != cdataw->_num_views ||
      cdata_tex->_num_components != cdataw->_num_components ||
      cdata_tex->_component_width != cdataw->_component_width ||
      cdata_tex->_texture_type != cdataw->_texture_type ||
      cdata_tex->_component_type != cdataw->_component_type) {

    cdataw->_x_size = cdata_tex->_x_size;
    cdataw->_y_size = cdata_tex->_y_size;
    cdataw->_z_size = cdata_tex->_z_size;
    cdataw->_num_views = cdata_tex->_num_views;

    cdataw->_num_components = cdata_tex->_num_components;
    cdataw->_component_width = cdata_tex->_component_width;
    cdataw->_texture_type = cdata_tex->_texture_type;
    cdataw->_format = cdata_tex->_format;
    cdataw->_component_type = cdata_tex->_component_type;

    cdataw->inc_properties_modified();
    cdataw->inc_image_modified();
  }

  cdataw->_keep_ram_image = cdata_tex->_keep_ram_image;
  cdataw->_ram_image_compression = cdata_tex->_ram_image_compression;
  cdataw->_ram_images = cdata_tex->_ram_images;

  nassertr(_reloading, nullptr);
  _reloading = false;

  // We don't generally increment the cdata->_image_modified semaphore,
  // because this is just a reload, and presumably the image hasn't changed
  // (unless we hit the if condition above).

  _cvar.notify_all();

  // Return the still-locked cdata.
  return cdataw;
}

/**
 * Called when the Texture image is required but the ram image is not
 * available, this will reload it from disk or otherwise do whatever is
 * required to make it available, if possible.
 *
 * Assumes the lock is already held.  The lock will be held during the
 * duration of this operation.
 */
void Texture::
do_reload_ram_image(CData *cdata, bool allow_compression) {
  BamCache *cache = BamCache::get_global_ptr();
  PT(BamCacheRecord) record;

  if (!do_has_compression(cdata)) {
    allow_compression = false;
  }

  if ((cache->get_cache_textures() || (allow_compression && cache->get_cache_compressed_textures())) && !textures_header_only) {
    // See if the texture can be found in the on-disk cache, if it is active.

    record = cache->lookup(cdata->_fullpath, "txo");
    if (record != nullptr &&
        record->has_data()) {
      PT(Texture) tex = DCAST(Texture, record->get_data());

      // But don't use the cache record if the config parameters have changed,
      // and we want a different-sized texture now.
      int x_size = cdata->_orig_file_x_size;
      int y_size = cdata->_orig_file_y_size;
      do_adjust_this_size(cdata, x_size, y_size, cdata->_filename.get_basename(), true);
      if (x_size != tex->get_x_size() || y_size != tex->get_y_size()) {
        if (gobj_cat.is_debug()) {
          gobj_cat.debug()
            << "Cached texture " << *this << " has size "
            << tex->get_x_size() << " x " << tex->get_y_size()
            << " instead of " << x_size << " x " << y_size
            << "; ignoring cache.\n";
        }
      } else {
        // Also don't keep the cached version if it's compressed but we want
        // uncompressed.
        if (!allow_compression && tex->get_ram_image_compression() != Texture::CM_off) {
          if (gobj_cat.is_debug()) {
            gobj_cat.debug()
              << "Cached texture " << *this
              << " is compressed in cache; ignoring cache.\n";
          }
        } else {
          gobj_cat.info()
            << "Texture " << get_name() << " reloaded from disk cache\n";
          // We don't want to replace all the texture parameters--for
          // instance, we don't want to change the filter type or the border
          // color or anything--we just want to get the image and necessary
          // associated parameters.
          CDReader cdata_tex(tex->_cycler);
          cdata->_x_size = cdata_tex->_x_size;
          cdata->_y_size = cdata_tex->_y_size;
          if (cdata->_num_components != cdata_tex->_num_components) {
            cdata->_num_components = cdata_tex->_num_components;
            cdata->_format = cdata_tex->_format;
          }
          cdata->_component_type = cdata_tex->_component_type;
          cdata->_compression = cdata_tex->_compression;
          cdata->_ram_image_compression = cdata_tex->_ram_image_compression;
          cdata->_ram_images = cdata_tex->_ram_images;
          cdata->_loaded_from_image = true;

          bool was_compressed = (cdata->_ram_image_compression != CM_off);
          if (do_consider_auto_process_ram_image(cdata, uses_mipmaps(), allow_compression)) {
            bool is_compressed = (cdata->_ram_image_compression != CM_off);
            if (!was_compressed && is_compressed &&
                cache->get_cache_compressed_textures()) {
              // We've re-compressed the image after loading it from the
              // cache.  To keep the cache current, rewrite it to the cache
              // now, in its newly compressed form.
              record->set_data(this, this);
              cache->store(record);
            }
          }

          return;
        }
      }
    }
  }

  gobj_cat.info()
    << "Reloading texture " << get_name() << "\n";

  int z = 0;
  int n = 0;

  if (cdata->_has_read_pages) {
    z = cdata->_z_size;
  }
  if (cdata->_has_read_mipmaps) {
    n = cdata->_num_mipmap_levels_read;
  }

  cdata->_loaded_from_image = false;
  Format orig_format = cdata->_format;
  int orig_num_components = cdata->_num_components;

  LoaderOptions options;
  if (allow_compression) {
    options.set_texture_flags(LoaderOptions::TF_preload |
                              LoaderOptions::TF_allow_compression);
  } else {
    options.set_texture_flags(LoaderOptions::TF_preload);
  }
  do_read(cdata, cdata->_fullpath, cdata->_alpha_fullpath,
          cdata->_primary_file_num_channels, cdata->_alpha_file_channel,
          z, n, cdata->_has_read_pages, cdata->_has_read_mipmaps, options, nullptr);

  if (orig_num_components == cdata->_num_components) {
    // Restore the original format, in case it was needlessly changed during
    // the reload operation.
    cdata->_format = orig_format;
  }

  if (do_has_ram_image(cdata) && record != nullptr) {
    if (cache->get_cache_textures() || (cdata->_ram_image_compression != CM_off && cache->get_cache_compressed_textures())) {
      // Update the cache.
      if (record != nullptr) {
        record->add_dependent_file(cdata->_fullpath);
      }
      record->set_data(this, this);
      cache->store(record);
    }
  }
}

/**
 * This is called internally to uniquify the ram image pointer without
 * updating cdata->_image_modified.
 */
PTA_uchar Texture::
do_modify_ram_image(CData *cdata) {
  if (cdata->_ram_images.empty() || cdata->_ram_images[0]._image.empty() ||
      cdata->_ram_image_compression != CM_off) {
    do_make_ram_image(cdata);
  } else {
    do_clear_ram_mipmap_images(cdata);
  }
  return cdata->_ram_images[0]._image;
}

/**
 * This is called internally to make a new ram image without updating
 * cdata->_image_modified.
 */
PTA_uchar Texture::
do_make_ram_image(CData *cdata) {
  int image_size = do_get_expected_ram_image_size(cdata);
  cdata->_ram_images.clear();
  cdata->_ram_images.push_back(RamImage());
  cdata->_ram_images[0]._page_size = do_get_expected_ram_page_size(cdata);
  cdata->_ram_images[0]._image = PTA_uchar::empty_array(image_size, get_class_type());
  cdata->_ram_images[0]._pointer_image = nullptr;
  cdata->_ram_image_compression = CM_off;

  if (cdata->_has_clear_color) {
    // Fill the image with the clear color.
    unsigned char pixel[16];
    const int pixel_size = do_get_clear_data(cdata, pixel);
    nassertr(pixel_size > 0, cdata->_ram_images[0]._image);

    unsigned char *image_data = cdata->_ram_images[0]._image;
    for (int i = 0; i < image_size; i += pixel_size) {
      memcpy(image_data + i, pixel, pixel_size);
    }
  }

  return cdata->_ram_images[0]._image;
}

/**
 * Replaces the current system-RAM image with the new data.  If compression is
 * not CM_off, it indicates that the new data is already pre-compressed in the
 * indicated format.
 *
 * This does *not* affect keep_ram_image.
 */
void Texture::
do_set_ram_image(CData *cdata, CPTA_uchar image, Texture::CompressionMode compression,
                 size_t page_size) {
  nassertv(compression != CM_default);
  nassertv(compression != CM_off || image.size() == do_get_expected_ram_image_size(cdata));
  if (cdata->_ram_images.empty()) {
    cdata->_ram_images.push_back(RamImage());
  } else {
    do_clear_ram_mipmap_images(cdata);
  }
  if (page_size == 0) {
    page_size = image.size();
  }
  if (cdata->_ram_images[0]._image != image ||
      cdata->_ram_images[0]._page_size != page_size ||
      cdata->_ram_image_compression != compression) {
    cdata->_ram_images[0]._image = image.cast_non_const();
    cdata->_ram_images[0]._page_size = page_size;
    cdata->_ram_images[0]._pointer_image = nullptr;
    cdata->_ram_image_compression = compression;
    cdata->inc_image_modified();
  }
}

/**
 * This is called internally to uniquify the nth mipmap image pointer without
 * updating cdata->_image_modified.
 */
PTA_uchar Texture::
do_modify_ram_mipmap_image(CData *cdata, int n) {
  nassertr(cdata->_ram_image_compression == CM_off, PTA_uchar());

  if (n >= (int)cdata->_ram_images.size() ||
      cdata->_ram_images[n]._image.empty()) {
    do_make_ram_mipmap_image(cdata, n);
  }
  return cdata->_ram_images[n]._image;
}

/**
 *
 */
PTA_uchar Texture::
do_make_ram_mipmap_image(CData *cdata, int n) {
  nassertr(cdata->_ram_image_compression == CM_off, PTA_uchar(get_class_type()));

  while (n >= (int)cdata->_ram_images.size()) {
    cdata->_ram_images.push_back(RamImage());
  }

  size_t image_size = do_get_expected_ram_mipmap_image_size(cdata, n);
  cdata->_ram_images[n]._image = PTA_uchar::empty_array(image_size, get_class_type());
  cdata->_ram_images[n]._pointer_image = nullptr;
  cdata->_ram_images[n]._page_size = do_get_expected_ram_mipmap_page_size(cdata, n);

  if (cdata->_has_clear_color) {
    // Fill the image with the clear color.
    unsigned char pixel[16];
    const size_t pixel_size = (size_t)do_get_clear_data(cdata, pixel);
    nassertr(pixel_size > 0, cdata->_ram_images[n]._image);

    unsigned char *image_data = cdata->_ram_images[n]._image;
    for (size_t i = 0; i < image_size; i += pixel_size) {
      memcpy(image_data + i, pixel, pixel_size);
    }
  }

  return cdata->_ram_images[n]._image;
}

/**
 *
 */
void Texture::
do_set_ram_mipmap_image(CData *cdata, int n, CPTA_uchar image, size_t page_size) {
  nassertv(cdata->_ram_image_compression != CM_off || image.size() == do_get_expected_ram_mipmap_image_size(cdata, n));

  while (n >= (int)cdata->_ram_images.size()) {
    cdata->_ram_images.push_back(RamImage());
  }
  if (page_size == 0) {
    page_size = image.size();
  }

  if (cdata->_ram_images[n]._image != image ||
      cdata->_ram_images[n]._page_size != page_size) {
    cdata->_ram_images[n]._image = image.cast_non_const();
    cdata->_ram_images[n]._pointer_image = nullptr;
    cdata->_ram_images[n]._page_size = page_size;
    cdata->inc_image_modified();
  }
}

/**
 * Returns a string with a single pixel representing the clear color of the
 * texture in the format of this texture.
 *
 * In other words, to create an uncompressed RAM texture filled with the clear
 * color, it should be initialized with this string repeated for every pixel.
 */
size_t Texture::
do_get_clear_data(const CData *cdata, unsigned char *into) const {
  nassertr(cdata->_has_clear_color, 0);

  int num_components = cdata->_num_components;
  nassertr(num_components > 0, 0);
  nassertr(num_components <= 4, 0);

  LVecBase4 clear_value = cdata->_clear_color;

  // Swap red and blue components.
  if (num_components >= 3) {
    std::swap(clear_value[0], clear_value[2]);
  }

  switch (cdata->_component_type) {
  case T_unsigned_byte:
    if (is_srgb(cdata->_format)) {
      xel color;
      xelval alpha;
      encode_sRGB_uchar(clear_value, color, alpha);
      switch (num_components) {
      case 4: into[3] = (unsigned char)alpha;
      case 3: into[2] = (unsigned char)color.b;
      case 2: into[1] = (unsigned char)color.g;
      case 1: into[0] = (unsigned char)color.r;
      }
    } else {
      LColor scaled = clear_value.fmin(LColor(1)).fmax(LColor::zero());
      scaled *= 255;
      for (int i = 0; i < num_components; ++i) {
        into[i] = (unsigned char)scaled[i];
      }
    }
    break;

  case T_unsigned_short:
    {
      LColor scaled = clear_value.fmin(LColor(1)).fmax(LColor::zero());
      scaled *= 65535;
      for (int i = 0; i < num_components; ++i) {
        ((unsigned short *)into)[i] = (unsigned short)scaled[i];
      }
      break;
    }

  case T_float:
    for (int i = 0; i < num_components; ++i) {
      ((float *)into)[i] = clear_value[i];
    }
    break;

  case T_unsigned_int_24_8:
    nassertr(num_components == 1, 0);
    *((unsigned int *)into) =
      ((unsigned int)(clear_value[0] * 16777215) << 8) +
       (unsigned int)max(min(clear_value[1], (PN_stdfloat)255), (PN_stdfloat)0);
    break;

  case T_int:
    // Note: there are no 32-bit UNORM textures.  Therefore, we don't do any
    // normalization here, either.
    for (int i = 0; i < num_components; ++i) {
      ((int *)into)[i] = (int)clear_value[i];
    }
    break;

  case T_byte:
    {
      LColor scaled = clear_value.fmin(LColor(1)).fmax(LColor(-1));
      scaled *= 127;
      for (int i = 0; i < num_components; ++i) {
        ((signed char *)into)[i] = (signed char)scaled[i];
      }
      break;
    }

  case T_short:
    {
      LColor scaled = clear_value.fmin(LColor(1)).fmax(LColor(-1));
      scaled *= 32767;
      for (int i = 0; i < num_components; ++i) {
        ((short *)into)[i] = (short)scaled[i];
      }
      break;
    }

  case T_half_float:
    for (int i = 0; i < num_components; ++i) {
      union {
        uint32_t ui;
        float uf;
      } v;
      v.uf = clear_value[i];
      uint16_t sign = ((v.ui & 0x80000000u) >> 16u);
      uint32_t mantissa = (v.ui & 0x007fffffu);
      uint16_t exponent = (uint16_t)std::min(std::max((int)((v.ui & 0x7f800000u) >> 23u) - 112, 0), 31);
      mantissa += (mantissa & 0x00001000u) << 1u;
      ((uint16_t *)into)[i] = (uint16_t)(sign | ((exponent << 10u) | (mantissa >> 13u)));
    }
    break;

  case T_unsigned_int:
    // Note: there are no 32-bit UNORM textures.  Therefore, we don't do any
    // normalization here, either.
    for (int i = 0; i < num_components; ++i) {
      ((unsigned int *)into)[i] = (unsigned int)clear_value[i];
    }
  }

  return num_components * cdata->_component_width;
}

/**
 * Should be called after a texture has been loaded into RAM, this considers
 * generating mipmaps and/or compressing the RAM image.
 *
 * Returns true if the image was modified by this operation, false if it
 * wasn't.
 */
bool Texture::
consider_auto_process_ram_image(bool generate_mipmaps, bool allow_compression) {
  CDWriter cdata(_cycler, false);
  return do_consider_auto_process_ram_image(cdata, generate_mipmaps, allow_compression);
}

/**
 * Should be called after a texture has been loaded into RAM, this considers
 * generating mipmaps and/or compressing the RAM image.
 *
 * Returns true if the image was modified by this operation, false if it
 * wasn't.
 */
bool Texture::
do_consider_auto_process_ram_image(CData *cdata, bool generate_mipmaps,
                                   bool allow_compression) {
  bool modified = false;

  if (generate_mipmaps && !driver_generate_mipmaps &&
      cdata->_ram_images.size() == 1) {
    do_generate_ram_mipmap_images(cdata, false);
    modified = true;
  }

  if (allow_compression && !driver_compress_textures) {
    CompressionMode compression = cdata->_compression;
    if (compression == CM_default && compressed_textures) {
      compression = CM_on;
    }
    if (compression != CM_off && cdata->_ram_image_compression == CM_off) {
      GraphicsStateGuardianBase *gsg = GraphicsStateGuardianBase::get_default_gsg();
      if (do_compress_ram_image(cdata, compression, QL_default, gsg)) {
        if (gobj_cat.is_debug()) {
          gobj_cat.debug()
            << "Compressed " << get_name() << " with "
            << cdata->_ram_image_compression << "\n";
        }
        modified = true;
      }
    }
  }

  return modified;
}

/**
 *
 */
bool Texture::
do_compress_ram_image(CData *cdata, Texture::CompressionMode compression,
                      Texture::QualityLevel quality_level,
                      GraphicsStateGuardianBase *gsg) {
  nassertr(compression != CM_off, false);

  if (cdata->_ram_images.empty() || cdata->_ram_image_compression != CM_off) {
    return false;
  }

  if (compression == CM_on) {
    // Select an appropriate compression mode automatically.
    switch (cdata->_format) {
    case Texture::F_rgbm:
    case Texture::F_rgb:
    case Texture::F_rgb5:
    case Texture::F_rgba5:
    case Texture::F_rgb8:
    case Texture::F_rgb12:
    case Texture::F_rgb332:
    case Texture::F_rgb16:
    case Texture::F_rgb32:
    case Texture::F_rgb10_a2:
      if (gsg == nullptr || gsg->get_supports_compressed_texture_format(CM_dxt1)) {
        compression = CM_dxt1;
      } else if (gsg->get_supports_compressed_texture_format(CM_dxt3)) {
        compression = CM_dxt3;
      } else if (gsg->get_supports_compressed_texture_format(CM_dxt5)) {
        compression = CM_dxt5;
      } else if (gsg->get_supports_compressed_texture_format(CM_etc2)) {
        compression = CM_etc2;
      } else if (gsg->get_supports_compressed_texture_format(CM_etc1)) {
        compression = CM_etc1;
      }
      break;

    case Texture::F_rgba4:
      if (gsg == nullptr || gsg->get_supports_compressed_texture_format(CM_dxt3)) {
        compression = CM_dxt3;
      } else if (gsg->get_supports_compressed_texture_format(CM_dxt5)) {
        compression = CM_dxt5;
      } else if (gsg->get_supports_compressed_texture_format(CM_etc2)) {
        compression = CM_etc2;
      }
      break;

    case Texture::F_rgba:
    case Texture::F_rgba8:
    case Texture::F_rgba12:
    case Texture::F_rgba16:
    case Texture::F_rgba32:
      if (gsg == nullptr || gsg->get_supports_compressed_texture_format(CM_dxt5)) {
        compression = CM_dxt5;
      } else if (gsg->get_supports_compressed_texture_format(CM_etc2)) {
        compression = CM_etc2;
      }
      break;

    case Texture::F_red:
    case Texture::F_rg:
      if (gsg == nullptr || gsg->get_supports_compressed_texture_format(CM_rgtc)) {
        compression = CM_rgtc;
      } else if (gsg->get_supports_compressed_texture_format(CM_eac)) {
        compression = CM_eac;
      }
      break;

    default:
      break;
    }
  }

  // Choose an appropriate quality level.
  if (quality_level == Texture::QL_default) {
    quality_level = cdata->_quality_level;
  }
  if (quality_level == Texture::QL_default) {
    quality_level = texture_quality_level;
  }

  if (compression == CM_rgtc) {
    // We should compress RGTC ourselves, as squish does not support it.
    if (cdata->_component_type != T_unsigned_byte) {
      return false;
    }

    if (!do_has_all_ram_mipmap_images(cdata)) {
      // If we're about to compress the RAM image, we should ensure that we
      // have all of the mipmap levels first.
      do_generate_ram_mipmap_images(cdata, false);
    }

    RamImages compressed_ram_images;
    compressed_ram_images.resize(cdata->_ram_images.size());

    for (size_t n = 0; n < cdata->_ram_images.size(); ++n) {
      const RamImage *uncompressed_image = &cdata->_ram_images[n];

      int x_size = do_get_expected_mipmap_x_size(cdata, n);
      int y_size = do_get_expected_mipmap_y_size(cdata, n);
      int num_pages = do_get_expected_mipmap_num_pages(cdata, n);

      // It is important that we handle image sizes that aren't a multiple of
      // the block size, since this method may be used to compress mipmaps,
      // which go all the way to 1x1.  Pad the image if necessary.
      RamImage temp_image;
      if ((x_size | y_size) & 0x3) {
        int virtual_x_size = x_size;
        int virtual_y_size = y_size;
        x_size = (x_size + 3) & ~0x3;
        y_size = (y_size + 3) & ~0x3;

        temp_image._page_size = x_size * y_size * cdata->_num_components;
        temp_image._image = PTA_uchar::empty_array(temp_image._page_size * num_pages);

        for (int z = 0; z < num_pages; ++z) {
          unsigned char *dest = temp_image._image.p() + z * temp_image._page_size;
          unsigned const char *src = uncompressed_image->_image.p() + z * uncompressed_image->_page_size;

          for (int y = 0; y < virtual_y_size; ++y) {
            memcpy(dest, src, virtual_x_size);
            src += virtual_x_size;
            dest += x_size;
          }
        }

        uncompressed_image = &temp_image;
      }

      // Create a new image to hold the compressed texture pages.
      RamImage &compressed_image = compressed_ram_images[n];
      compressed_image._page_size = (x_size * y_size * cdata->_num_components) >> 1;
      compressed_image._image = PTA_uchar::empty_array(compressed_image._page_size * num_pages);

      if (cdata->_num_components == 1) {
        do_compress_ram_image_bc4(*uncompressed_image, compressed_image,
                                  x_size, y_size, num_pages);
      } else if (cdata->_num_components == 2) {
        do_compress_ram_image_bc5(*uncompressed_image, compressed_image,
                                  x_size, y_size, num_pages);
      } else {
        // Invalid.
        return false;
      }
    }

    cdata->_ram_images.swap(compressed_ram_images);
    cdata->_ram_image_compression = CM_rgtc;
    return true;
  }

#ifdef HAVE_SQUISH
  if (cdata->_texture_type != TT_3d_texture &&
      cdata->_texture_type != TT_2d_texture_array &&
      cdata->_component_type == T_unsigned_byte) {
    int squish_flags = 0;
    switch (compression) {
    case CM_dxt1:
      squish_flags |= squish::kDxt1;
      break;

    case CM_dxt3:
      squish_flags |= squish::kDxt3;
      break;

    case CM_dxt5:
      squish_flags |= squish::kDxt5;
      break;

    default:
      break;
    }

    if (squish_flags != 0) {
      // This compression mode is supported by squish; use it.
      switch (quality_level) {
      case QL_fastest:
        squish_flags |= squish::kColourRangeFit;
        break;

      case QL_normal:
        // ColourClusterFit is just too slow for everyday use.
        squish_flags |= squish::kColourRangeFit;
        // squish_flags |= squish::kColourClusterFit;
        break;

      case QL_best:
        squish_flags |= squish::kColourIterativeClusterFit;
        break;

      default:
        break;
      }

      if (do_squish(cdata, compression, squish_flags)) {
        return true;
      }
    }
  }
#endif  // HAVE_SQUISH

  return false;
}

/**
 *
 */
bool Texture::
do_uncompress_ram_image(CData *cdata) {
  nassertr(!cdata->_ram_images.empty(), false);

  if (cdata->_ram_image_compression == CM_rgtc) {
    // We should decompress RGTC ourselves, as squish doesn't support it.
    RamImages uncompressed_ram_images;
    uncompressed_ram_images.resize(cdata->_ram_images.size());

    for (size_t n = 0; n < cdata->_ram_images.size(); ++n) {
      const RamImage &compressed_image = cdata->_ram_images[n];

      int x_size = do_get_expected_mipmap_x_size(cdata, n);
      int y_size = do_get_expected_mipmap_y_size(cdata, n);
      int num_pages = do_get_expected_mipmap_num_pages(cdata, n);

      RamImage &uncompressed_image = uncompressed_ram_images[n];
      uncompressed_image._page_size = do_get_expected_ram_mipmap_page_size(cdata, n);
      uncompressed_image._image = PTA_uchar::empty_array(uncompressed_image._page_size * num_pages);

      if (cdata->_num_components == 1) {
        do_uncompress_ram_image_bc4(compressed_image, uncompressed_image,
                                    x_size, y_size, num_pages);
      } else if (cdata->_num_components == 2) {
        do_uncompress_ram_image_bc5(compressed_image, uncompressed_image,
                                    x_size, y_size, num_pages);
      } else {
        // Invalid.
        return false;
      }
    }
    cdata->_ram_images.swap(uncompressed_ram_images);
    cdata->_ram_image_compression = CM_off;
    return true;
  }

#ifdef HAVE_SQUISH
  if (cdata->_texture_type != TT_3d_texture &&
      cdata->_texture_type != TT_2d_texture_array &&
      cdata->_component_type == T_unsigned_byte) {
    int squish_flags = 0;
    switch (cdata->_ram_image_compression) {
    case CM_dxt1:
      squish_flags |= squish::kDxt1;
      break;

    case CM_dxt3:
      squish_flags |= squish::kDxt3;
      break;

    case CM_dxt5:
      squish_flags |= squish::kDxt5;
      break;

    default:
      break;
    }

    if (squish_flags != 0) {
      // This compression mode is supported by squish; use it.
      if (do_unsquish(cdata, squish_flags)) {
        return true;
      }
    }
  }
#endif  // HAVE_SQUISH
  return false;
}

/**
 * Compresses a RAM image using BC4 compression.
 */
void Texture::
do_compress_ram_image_bc4(const RamImage &uncompressed_image,
                          RamImage &compressed_image,
                          int x_size, int y_size, int num_pages) {
  int x_blocks = (x_size >> 2);
  int y_blocks = (y_size >> 2);

  // NB. This algorithm isn't fully optimal, since it doesn't try to make use
  // of the secondary interpolation mode supported by BC4.  This is not
  // important for most textures, but it may be added in the future.

  nassertv((size_t)x_blocks * (size_t)y_blocks * 4 * 4 <= uncompressed_image._page_size);
  nassertv((size_t)x_size * (size_t)y_size == uncompressed_image._page_size);

  static const int remap[] = {1, 7, 6, 5, 4, 3, 2, 0};

  for (int z = 0; z < num_pages; ++z) {
    unsigned char *dest = compressed_image._image.p() + z * compressed_image._page_size;
    unsigned const char *src = uncompressed_image._image.p() + z * uncompressed_image._page_size;

    // Convert one 4 x 4 block at a time.
    for (int y = 0; y < y_blocks; ++y) {
      for (int x = 0; x < x_blocks; ++x) {
        int a, b, c, d;
        float fac, add;
        unsigned char minv, maxv;
        unsigned const char *blk = src;

        // Find the minimum and maximum value in the block.
        minv = blk[0];
        maxv = blk[0];
        minv = min(blk[1], minv); maxv = max(blk[1], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[3], minv); maxv = max(blk[3], maxv);
        blk += x_size;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[1], minv); maxv = max(blk[1], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[3], minv); maxv = max(blk[3], maxv);
        blk += x_size;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[1], minv); maxv = max(blk[1], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[3], minv); maxv = max(blk[3], maxv);
        blk += x_size;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[1], minv); maxv = max(blk[1], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[3], minv); maxv = max(blk[3], maxv);

        // Now calculate the index for each pixel.
        blk = src;
        if (maxv > minv) {
          fac = 7.5f / (maxv - minv);
        } else {
          fac = 0;
        }
        add = -minv * fac;
        a = (remap[(int)(blk[0] * fac + add)])
          | (remap[(int)(blk[1] * fac + add)] << 3)
          | (remap[(int)(blk[2] * fac + add)] << 6)
          | (remap[(int)(blk[3] * fac + add)] << 9);
        blk += x_size;
        b = (remap[(int)(blk[0] * fac + add)] << 4)
          | (remap[(int)(blk[1] * fac + add)] << 7)
          | (remap[(int)(blk[2] * fac + add)] << 10)
          | (remap[(int)(blk[3] * fac + add)] << 13);
        blk += x_size;
        c = (remap[(int)(blk[0] * fac + add)])
          | (remap[(int)(blk[1] * fac + add)] << 3)
          | (remap[(int)(blk[2] * fac + add)] << 6)
          | (remap[(int)(blk[3] * fac + add)] << 9);
        blk += x_size;
        d = (remap[(int)(blk[0] * fac + add)] << 4)
          | (remap[(int)(blk[1] * fac + add)] << 7)
          | (remap[(int)(blk[2] * fac + add)] << 10)
          | (remap[(int)(blk[3] * fac + add)] << 13);

        *(dest++) = maxv;
        *(dest++) = minv;
        *(dest++) = a & 0xff;
        *(dest++) = (a >> 8) | (b & 0xf0);
        *(dest++) = b >> 8;
        *(dest++) = c & 0xff;
        *(dest++) = (c >> 8) | (d & 0xf0);
        *(dest++) = d >> 8;

        // Advance to the beginning of the next 4x4 block.
        src += 4;
      }
      src += x_size * 3;
    }
    Thread::consider_yield();
  }
}

/**
 * Compresses a RAM image using BC5 compression.
 */
void Texture::
do_compress_ram_image_bc5(const RamImage &uncompressed_image,
                          RamImage &compressed_image,
                          int x_size, int y_size, int num_pages) {
  int x_blocks = (x_size >> 2);
  int y_blocks = (y_size >> 2);
  int stride = x_size * 2;

  // BC5 uses the same compression algorithm as BC4, except repeated for two
  // channels.

  nassertv((size_t)x_blocks * (size_t)y_blocks * 4 * 4 * 2 <= uncompressed_image._page_size);
  nassertv((size_t)stride * (size_t)y_size == uncompressed_image._page_size);

  static const int remap[] = {1, 7, 6, 5, 4, 3, 2, 0};

  for (int z = 0; z < num_pages; ++z) {
    unsigned char *dest = compressed_image._image.p() + z * compressed_image._page_size;
    unsigned const char *src = uncompressed_image._image.p() + z * uncompressed_image._page_size;

    // Convert one 4 x 4 block at a time.
    for (int y = 0; y < y_blocks; ++y) {
      for (int x = 0; x < x_blocks; ++x) {
        int a, b, c, d;
        float fac, add;
        unsigned char minv, maxv;
        unsigned const char *blk = src;

        // Find the minimum and maximum red value in the block.
        minv = blk[0];
        maxv = blk[0];
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[4], minv); maxv = max(blk[4], maxv);
        minv = min(blk[6], minv); maxv = max(blk[6], maxv);
        blk += stride;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[4], minv); maxv = max(blk[4], maxv);
        minv = min(blk[6], minv); maxv = max(blk[6], maxv);
        blk += stride;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[4], minv); maxv = max(blk[4], maxv);
        minv = min(blk[6], minv); maxv = max(blk[6], maxv);
        blk += stride;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[4], minv); maxv = max(blk[4], maxv);
        minv = min(blk[6], minv); maxv = max(blk[6], maxv);

        // Now calculate the index for each pixel.
        if (maxv > minv) {
          fac = 7.5f / (maxv - minv);
        } else {
          fac = 0;
        }
        add = -minv * fac;
        blk = src;
        a = (remap[(int)(blk[0] * fac + add)])
          | (remap[(int)(blk[2] * fac + add)] << 3)
          | (remap[(int)(blk[4] * fac + add)] << 6)
          | (remap[(int)(blk[6] * fac + add)] << 9);
        blk += stride;
        b = (remap[(int)(blk[0] * fac + add)] << 4)
          | (remap[(int)(blk[2] * fac + add)] << 7)
          | (remap[(int)(blk[4] * fac + add)] << 10)
          | (remap[(int)(blk[6] * fac + add)] << 13);
        blk += stride;
        c = (remap[(int)(blk[0] * fac + add)])
          | (remap[(int)(blk[2] * fac + add)] << 3)
          | (remap[(int)(blk[4] * fac + add)] << 6)
          | (remap[(int)(blk[6] * fac + add)] << 9);
        blk += stride;
        d = (remap[(int)(blk[0] * fac + add)] << 4)
          | (remap[(int)(blk[2] * fac + add)] << 7)
          | (remap[(int)(blk[4] * fac + add)] << 10)
          | (remap[(int)(blk[6] * fac + add)] << 13);

        *(dest++) = maxv;
        *(dest++) = minv;
        *(dest++) = a & 0xff;
        *(dest++) = (a >> 8) | (b & 0xf0);
        *(dest++) = b >> 8;
        *(dest++) = c & 0xff;
        *(dest++) = (c >> 8) | (d & 0xf0);
        *(dest++) = d >> 8;

        // Find the minimum and maximum green value in the block.
        blk = src + 1;
        minv = blk[0];
        maxv = blk[0];
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[4], minv); maxv = max(blk[4], maxv);
        minv = min(blk[6], minv); maxv = max(blk[6], maxv);
        blk += stride;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[4], minv); maxv = max(blk[4], maxv);
        minv = min(blk[6], minv); maxv = max(blk[6], maxv);
        blk += stride;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[4], minv); maxv = max(blk[4], maxv);
        minv = min(blk[6], minv); maxv = max(blk[6], maxv);
        blk += stride;
        minv = min(blk[0], minv); maxv = max(blk[0], maxv);
        minv = min(blk[2], minv); maxv = max(blk[2], maxv);
        minv = min(blk[4], minv); maxv = max(blk[4], maxv);
        minv = min(blk[6], minv); maxv = max(blk[6], maxv);

        // Now calculate the index for each pixel.
        if (maxv > minv) {
          fac = 7.5f / (maxv - minv);
        } else {
          fac = 0;
        }
        add = -minv * fac;
        blk = src + 1;
        a = (remap[(int)(blk[0] * fac + add)])
          | (remap[(int)(blk[2] * fac + add)] << 3)
          | (remap[(int)(blk[4] * fac + add)] << 6)
          | (remap[(int)(blk[6] * fac + add)] << 9);
        blk += stride;
        b = (remap[(int)(blk[0] * fac + add)] << 4)
          | (remap[(int)(blk[2] * fac + add)] << 7)
          | (remap[(int)(blk[4] * fac + add)] << 10)
          | (remap[(int)(blk[6] * fac + add)] << 13);
        blk += stride;
        c = (remap[(int)(blk[0] * fac + add)])
          | (remap[(int)(blk[2] * fac + add)] << 3)
          | (remap[(int)(blk[4] * fac + add)] << 6)
          | (remap[(int)(blk[6] * fac + add)] << 9);
        blk += stride;
        d = (remap[(int)(blk[0] * fac + add)] << 4)
          | (remap[(int)(blk[2] * fac + add)] << 7)
          | (remap[(int)(blk[4] * fac + add)] << 10)
          | (remap[(int)(blk[6] * fac + add)] << 13);

        *(dest++) = maxv;
        *(dest++) = minv;
        *(dest++) = a & 0xff;
        *(dest++) = (a >> 8) | (b & 0xf0);
        *(dest++) = b >> 8;
        *(dest++) = c & 0xff;
        *(dest++) = (c >> 8) | (d & 0xf0);
        *(dest++) = d >> 8;

        // Advance to the beginning of the next 4x4 block.
        src += 8;
      }
      src += stride * 3;
    }
    Thread::consider_yield();
  }
}

/**
 * Decompresses a RAM image compressed using BC4.
 */
void Texture::
do_uncompress_ram_image_bc4(const RamImage &compressed_image,
                            RamImage &uncompressed_image,
                            int x_size, int y_size, int num_pages) {
  int x_blocks = (x_size >> 2);
  int y_blocks = (y_size >> 2);

  for (int z = 0; z < num_pages; ++z) {
    unsigned char *dest = uncompressed_image._image.p() + z * uncompressed_image._page_size;
    unsigned const char *src = compressed_image._image.p() + z * compressed_image._page_size;

    // Unconvert one 4 x 4 block at a time.
    uint8_t tbl[8];
    for (int y = 0; y < y_blocks; ++y) {
      for (int x = 0; x < x_blocks; ++x) {
        unsigned char *blk = dest;
        tbl[0] = src[0];
        tbl[1] = src[1];
        if (tbl[0] > tbl[1]) {
          tbl[2] = (tbl[0] * 6 + tbl[1] * 1) / 7.0f;
          tbl[3] = (tbl[0] * 5 + tbl[1] * 2) / 7.0f;
          tbl[4] = (tbl[0] * 4 + tbl[1] * 3) / 7.0f;
          tbl[5] = (tbl[0] * 3 + tbl[1] * 4) / 7.0f;
          tbl[6] = (tbl[0] * 2 + tbl[1] * 5) / 7.0f;
          tbl[7] = (tbl[0] * 1 + tbl[1] * 6) / 7.0f;
        } else {
          tbl[2] = (tbl[0] * 4 + tbl[1] * 1) / 5.0f;
          tbl[3] = (tbl[0] * 3 + tbl[1] * 2) / 5.0f;
          tbl[4] = (tbl[0] * 2 + tbl[1] * 3) / 5.0f;
          tbl[5] = (tbl[0] * 1 + tbl[1] * 4) / 5.0f;
          tbl[6] = 0;
          tbl[7] = 255;
        }
        int v = src[2] + (src[3] << 8) + (src[4] << 16);
        blk[0] = tbl[v & 0x7];
        blk[1] = tbl[(v & 0x000038) >> 3];
        blk[2] = tbl[(v & 0x0001c0) >> 6];
        blk[3] = tbl[(v & 0x000e00) >> 9];
        blk += x_size;
        blk[0] = tbl[(v & 0x007000) >> 12];
        blk[1] = tbl[(v & 0x038000) >> 15];
        blk[2] = tbl[(v & 0x1c0000) >> 18];
        blk[3] = tbl[(v & 0xe00000) >> 21];
        blk += x_size;
        v = src[5] + (src[6] << 8) + (src[7] << 16);
        blk[0] = tbl[v & 0x7];
        blk[1] = tbl[(v & 0x000038) >> 3];
        blk[2] = tbl[(v & 0x0001c0) >> 6];
        blk[3] = tbl[(v & 0x000e00) >> 9];
        blk += x_size;
        blk[0] = tbl[(v & 0x007000) >> 12];
        blk[1] = tbl[(v & 0x038000) >> 15];
        blk[2] = tbl[(v & 0x1c0000) >> 18];
        blk[3] = tbl[(v & 0xe00000) >> 21];
        src += 8;
        dest += 4;
      }
      dest += x_size * 3;
    }
    Thread::consider_yield();
  }
}

/**
 * Decompresses a RAM image compressed using BC5.
 */
void Texture::
do_uncompress_ram_image_bc5(const RamImage &compressed_image,
                            RamImage &uncompressed_image,
                            int x_size, int y_size, int num_pages) {
  int x_blocks = (x_size >> 2);
  int y_blocks = (y_size >> 2);
  int stride = x_size * 2;

  for (int z = 0; z < num_pages; ++z) {
    unsigned char *dest = uncompressed_image._image.p() + z * uncompressed_image._page_size;
    unsigned const char *src = compressed_image._image.p() + z * compressed_image._page_size;

    // Unconvert one 4 x 4 block at a time.
    uint8_t red[8];
    uint8_t grn[8];
    for (int y = 0; y < y_blocks; ++y) {
      for (int x = 0; x < x_blocks; ++x) {
        unsigned char *blk = dest;
        red[0] = src[0];
        red[1] = src[1];
        if (red[0] > red[1]) {
          red[2] = (red[0] * 6 + red[1] * 1) / 7.0f;
          red[3] = (red[0] * 5 + red[1] * 2) / 7.0f;
          red[4] = (red[0] * 4 + red[1] * 3) / 7.0f;
          red[5] = (red[0] * 3 + red[1] * 4) / 7.0f;
          red[6] = (red[0] * 2 + red[1] * 5) / 7.0f;
          red[7] = (red[0] * 1 + red[1] * 6) / 7.0f;
        } else {
          red[2] = (red[0] * 4 + red[1] * 1) / 5.0f;
          red[3] = (red[0] * 3 + red[1] * 2) / 5.0f;
          red[4] = (red[0] * 2 + red[1] * 3) / 5.0f;
          red[5] = (red[0] * 1 + red[1] * 4) / 5.0f;
          red[6] = 0;
          red[7] = 255;
        }
        grn[0] = src[8];
        grn[1] = src[9];
        if (grn[0] > grn[1]) {
          grn[2] = (grn[0] * 6 + grn[1] * 1) / 7.0f;
          grn[3] = (grn[0] * 5 + grn[1] * 2) / 7.0f;
          grn[4] = (grn[0] * 4 + grn[1] * 3) / 7.0f;
          grn[5] = (grn[0] * 3 + grn[1] * 4) / 7.0f;
          grn[6] = (grn[0] * 2 + grn[1] * 5) / 7.0f;
          grn[7] = (grn[0] * 1 + grn[1] * 6) / 7.0f;
        } else {
          grn[2] = (grn[0] * 4 + grn[1] * 1) / 5.0f;
          grn[3] = (grn[0] * 3 + grn[1] * 2) / 5.0f;
          grn[4] = (grn[0] * 2 + grn[1] * 3) / 5.0f;
          grn[5] = (grn[0] * 1 + grn[1] * 4) / 5.0f;
          grn[6] = 0;
          grn[7] = 255;
        }
        int r = src[2] + (src[3] << 8) + (src[4] << 16);
        int g = src[10] + (src[11] << 8) + (src[12] << 16);
        blk[0] = red[r & 0x7];
        blk[1] = grn[g & 0x7];
        blk[2] = red[(r & 0x000038) >> 3];
        blk[3] = grn[(g & 0x000038) >> 3];
        blk[4] = red[(r & 0x0001c0) >> 6];
        blk[5] = grn[(g & 0x0001c0) >> 6];
        blk[6] = red[(r & 0x000e00) >> 9];
        blk[7] = grn[(g & 0x000e00) >> 9];
        blk += stride;
        blk[0] = red[(r & 0x007000) >> 12];
        blk[1] = grn[(g & 0x007000) >> 12];
        blk[2] = red[(r & 0x038000) >> 15];
        blk[3] = grn[(g & 0x038000) >> 15];
        blk[4] = red[(r & 0x1c0000) >> 18];
        blk[5] = grn[(g & 0x1c0000) >> 18];
        blk[6] = red[(r & 0xe00000) >> 21];
        blk[7] = grn[(g & 0xe00000) >> 21];
        blk += stride;
        r = src[5] + (src[6] << 8) + (src[7] << 16);
        g = src[13] + (src[14] << 8) + (src[15] << 16);
        blk[0] = red[r & 0x7];
        blk[1] = grn[g & 0x7];
        blk[2] = red[(r & 0x000038) >> 3];
        blk[3] = grn[(g & 0x000038) >> 3];
        blk[4] = red[(r & 0x0001c0) >> 6];
        blk[5] = grn[(g & 0x0001c0) >> 6];
        blk[6] = red[(r & 0x000e00) >> 9];
        blk[7] = grn[(g & 0x000e00) >> 9];
        blk += stride;
        blk[0] = red[(r & 0x007000) >> 12];
        blk[1] = grn[(g & 0x007000) >> 12];
        blk[2] = red[(r & 0x038000) >> 15];
        blk[3] = grn[(g & 0x038000) >> 15];
        blk[4] = red[(r & 0x1c0000) >> 18];
        blk[5] = grn[(g & 0x1c0000) >> 18];
        blk[6] = red[(r & 0xe00000) >> 21];
        blk[7] = grn[(g & 0xe00000) >> 21];
        src += 16;
        dest += 8;
      }
      dest += stride * 3;
    }
    Thread::consider_yield();
  }
}

/**
 *
 */
bool Texture::
do_has_all_ram_mipmap_images(const CData *cdata) const {
  if (cdata->_ram_images.empty() || cdata->_ram_images[0]._image.empty()) {
    // If we don't even have a base image, the answer is no.
    return false;
  }
  if (!uses_mipmaps()) {
    // If we have a base image and don't require mipmapping, the answer is
    // yes.
    return true;
  }

  // Check that we have enough mipmap levels to meet the size requirements.
  int size = max(cdata->_x_size, max(cdata->_y_size, cdata->_z_size));
  int n = 0;
  int x = 1;
  while (x < size) {
    x = (x << 1);
    ++n;
    if (n >= (int)cdata->_ram_images.size() || cdata->_ram_images[n]._image.empty()) {
      return false;
    }
  }

  return true;
}

/**
 * Considers whether the z_size (or num_views) should automatically be
 * adjusted when the user loads a new page.  Returns true if the z size is
 * valid, false otherwise.
 *
 * Assumes the lock is already held.
 */
bool Texture::
do_reconsider_z_size(CData *cdata, int z, const LoaderOptions &options) {
  if (z >= cdata->_z_size * cdata->_num_views) {
    bool num_views_specified = true;
    if (options.get_texture_flags() & LoaderOptions::TF_multiview) {
      // This flag is false if is a multiview texture with a specified number
      // of views.  It is true if it is not a multiview texture, or if it is
      // but the number of views is explicitly specified.
      num_views_specified = (options.get_texture_num_views() != 0);
    }

    if (num_views_specified &&
        (cdata->_texture_type == Texture::TT_3d_texture ||
         cdata->_texture_type == Texture::TT_2d_texture_array)) {
      // If we're loading a page past _z_size, treat it as an implicit request
      // to enlarge _z_size.  However, this is only legal if this is, in fact,
      // a 3-d texture or a 2d texture array (cube maps always have z_size 6,
      // and other types have z_size 1).
      nassertr(cdata->_num_views != 0, false);
      cdata->_z_size = (z / cdata->_num_views) + 1;

    } else if (cdata->_z_size != 0) {
      // In the case of a 2-d texture or cube map, or a 3-d texture with an
      // unspecified _num_views, assume we're loading views of a multiview
      // texture.
      cdata->_num_views = (z / cdata->_z_size) + 1;

    } else {
      // The first image loaded sets an implicit z-size.
      cdata->_z_size = 1;
    }

    // Increase the size of the data buffer to make room for the new texture
    // level.
    do_allocate_pages(cdata);
  }

  return true;
}

/**
 * Called internally by do_reconsider_z_size() to allocate new memory in
 * _ram_images[0] for the new number of pages.
 *
 * Assumes the lock is already held.
 */
void Texture::
do_allocate_pages(CData *cdata) {
  size_t new_size = do_get_expected_ram_image_size(cdata);
  if (!cdata->_ram_images.empty() &&
      !cdata->_ram_images[0]._image.empty() &&
      new_size > cdata->_ram_images[0]._image.size()) {
    cdata->_ram_images[0]._image.insert(cdata->_ram_images[0]._image.end(), new_size - cdata->_ram_images[0]._image.size(), 0);
    nassertv(cdata->_ram_images[0]._image.size() == new_size);
  }
}

/**
 * Resets the internal Texture properties when a new image file is loaded.
 * Returns true if the new image is valid, false otherwise.
 *
 * Assumes the lock is already held.
 */
bool Texture::
do_reconsider_image_properties(CData *cdata, int x_size, int y_size, int num_components,
                               Texture::ComponentType component_type, int z,
                               const LoaderOptions &options) {
  if (!cdata->_loaded_from_image || num_components != cdata->_num_components || component_type != cdata->_component_type) {
    // Come up with a default format based on the number of channels.  But
    // only do this the first time the file is loaded, or if the number of
    // channels in the image changes on subsequent loads.

    // TODO: handle sRGB properly
    switch (num_components) {
    case 1:
      cdata->_format = F_luminance;
      break;

    case 2:
      cdata->_format = F_luminance_alpha;
      break;

    case 3:
      cdata->_format = F_rgb;
      break;

    case 4:
      cdata->_format = F_rgba;
      break;

    default:
      // Eh?
      nassert_raise("unexpected channel count");
      cdata->_format = F_rgb;
      return false;
    }
  }

  if (!cdata->_loaded_from_image) {
    if ((options.get_texture_flags() & LoaderOptions::TF_allow_1d) &&
        cdata->_texture_type == TT_2d_texture && x_size != 1 && y_size == 1) {
      // If we're loading an Nx1 size texture, infer a 1-d texture type.
      cdata->_texture_type = TT_1d_texture;
    }

#ifndef NDEBUG
    switch (cdata->_texture_type) {
    case TT_1d_texture:
    case TT_buffer_texture:
      nassertr(y_size == 1, false);
      break;
    case TT_cube_map:
    case TT_cube_map_array:
      nassertr(x_size == y_size, false);
      break;
    default:
      break;
    }
#endif
    if ((cdata->_x_size != x_size)||(cdata->_y_size != y_size)) {
      do_set_pad_size(cdata, 0, 0, 0);
    }
    cdata->_x_size = x_size;
    cdata->_y_size = y_size;
    cdata->_num_components = num_components;
    do_set_component_type(cdata, component_type);

  } else {
    if (cdata->_x_size != x_size ||
        cdata->_y_size != y_size ||
        cdata->_num_components != num_components ||
        cdata->_component_type != component_type) {
      gobj_cat.error()
        << "Texture properties have changed for texture " << get_name()
        << " page " << z << ".\n";
      return false;
    }
  }

  return true;
}

/**
 *
 */
bool Texture::
do_rescale_texture(CData *cdata) {
  int new_x_size = cdata->_x_size;
  int new_y_size = cdata->_y_size;
  if (cdata->_z_size * cdata->_num_views != 1) {
    nassert_raise("rescale_texture() doesn't support 3-d or multiview textures.");
    return false;
  }

  if (do_adjust_this_size(cdata, new_x_size, new_y_size, get_name(), false)) {
    // OK, we have to scale the image.
    PNMImage orig_image;
    if (!do_store_one(cdata, orig_image, 0, 0)) {
      gobj_cat.warning()
        << "Couldn't get image in rescale_texture()\n";
      return false;
    }

    gobj_cat.info()
      << "Resizing " << get_name() << " to " << new_x_size << " x "
      << new_y_size << "\n";
    PNMImage new_image(new_x_size, new_y_size, orig_image.get_num_channels(),
                       orig_image.get_maxval(), orig_image.get_type(),
                       orig_image.get_color_space());
    new_image.quick_filter_from(orig_image);

    do_clear_ram_image(cdata);
    cdata->inc_image_modified();
    cdata->_x_size = new_x_size;
    cdata->_y_size = new_y_size;
    if (!do_load_one(cdata, new_image, get_name(), 0, 0, LoaderOptions())) {
      return false;
    }

    return true;
  }

  // Maybe we should pad the image.
  int pad_x_size = 0;
  int pad_y_size = 0;
  if (do_get_auto_texture_scale(cdata) == ATS_pad) {
    new_x_size = cdata->_x_size;
    new_y_size = cdata->_y_size;
    if (do_adjust_this_size(cdata, new_x_size, new_y_size, get_name(), true)) {
      pad_x_size = new_x_size - cdata->_x_size;
      pad_y_size = new_y_size - cdata->_y_size;

      PNMImage orig_image;
      if (!do_store_one(cdata, orig_image, 0, 0)) {
        gobj_cat.warning()
          << "Couldn't get image in rescale_texture()\n";
        return false;
      }
      PNMImage new_image(new_x_size, new_y_size, orig_image.get_num_channels(),
                         orig_image.get_maxval(), orig_image.get_type(),
                         orig_image.get_color_space());
      new_image.copy_sub_image(orig_image, 0, new_y_size - orig_image.get_y_size());

      do_clear_ram_image(cdata);
      cdata->_loaded_from_image = false;
      cdata->inc_image_modified();
      if (!do_load_one(cdata, new_image, get_name(), 0, 0, LoaderOptions())) {
        return false;
      }

      do_set_pad_size(cdata, pad_x_size, pad_y_size, 0);
      return true;
    }
  }

  // No changes needed.
  return false;
}

/**
 *
 */
PT(Texture) Texture::
make_copy_impl() const {
  CDReader cdata(_cycler);
  return do_make_copy(cdata);
}

/**
 *
 */
PT(Texture) Texture::
do_make_copy(const CData *cdata) const {
  PT(Texture) tex = new Texture(get_name());
  CDWriter cdata_tex(tex->_cycler, true);
  tex->do_assign(cdata_tex, this, cdata);
  return tex;
}

/**
 * The internal implementation of operator =().  Assumes the lock is already
 * held on both Textures.
 */
void Texture::
do_assign(CData *cdata, const Texture *copy, const CData *cdata_copy) {
  cdata->do_assign(cdata_copy);
}

/**
 * The protected implementation of clear().  Assumes the lock is already held.
 */
void Texture::
do_clear(CData *cdata) {
  Texture tex;
  tex.local_object();
  CDReader cdata_tex(tex._cycler);
  do_assign(cdata, &tex, cdata_tex);

  cdata->inc_properties_modified();
  cdata->inc_image_modified();
  cdata->inc_simple_image_modified();
}

/**
 *
 */
void Texture::
do_setup_texture(CData *cdata, Texture::TextureType texture_type,
                 int x_size, int y_size, int z_size,
                 Texture::ComponentType component_type,
                 Texture::Format format) {
  switch (texture_type) {
  case TT_1d_texture:
    nassertv(y_size == 1 && z_size == 1);
    break;

  case TT_2d_texture:
    nassertv(z_size == 1);
    break;

  case TT_3d_texture:
    break;

  case TT_2d_texture_array:
    break;

  case TT_cube_map:
    // Cube maps must always consist of six square images.
    nassertv(x_size == y_size && z_size == 6);

    // In principle the wrap mode shouldn't mean anything to a cube map, but
    // some drivers seem to misbehave if it's other than
    // SamplerState::WM_clamp.
    cdata->_default_sampler.set_wrap_u(SamplerState::WM_clamp);
    cdata->_default_sampler.set_wrap_v(SamplerState::WM_clamp);
    cdata->_default_sampler.set_wrap_w(SamplerState::WM_clamp);
    break;

  case TT_cube_map_array:
    // Cube maps array z_size needs to be a multiple of 6.
    nassertv(x_size == y_size && z_size % 6 == 0);

    cdata->_default_sampler.set_wrap_u(SamplerState::WM_clamp);
    cdata->_default_sampler.set_wrap_v(SamplerState::WM_clamp);
    cdata->_default_sampler.set_wrap_w(SamplerState::WM_clamp);
    break;

  case TT_buffer_texture:
    nassertv(y_size == 1 && z_size == 1);
    break;

  case TT_1d_texture_array:
    nassertv(z_size == 1);
    break;
  }

  if (texture_type != TT_2d_texture) {
    do_clear_simple_ram_image(cdata);
  }

  cdata->_texture_type = texture_type;
  cdata->_x_size = x_size;
  cdata->_y_size = y_size;
  cdata->_z_size = z_size;
  cdata->_num_views = 1;
  do_set_component_type(cdata, component_type);
  do_set_format(cdata, format);

  do_clear_ram_image(cdata);
  do_set_pad_size(cdata, 0, 0, 0);
  cdata->_orig_file_x_size = 0;
  cdata->_orig_file_y_size = 0;
  cdata->_loaded_from_image = false;
  cdata->_loaded_from_txo = false;
  cdata->_has_read_pages = false;
  cdata->_has_read_mipmaps = false;
}

/**
 *
 */
void Texture::
do_set_format(CData *cdata, Texture::Format format) {
  if (format == cdata->_format) {
    return;
  }
  cdata->_format = format;
  cdata->inc_properties_modified();

  switch (cdata->_format) {
  case F_color_index:
  case F_depth_stencil:
  case F_depth_component:
  case F_depth_component16:
  case F_depth_component24:
  case F_depth_component32:
  case F_red:
  case F_green:
  case F_blue:
  case F_alpha:
  case F_luminance:
  case F_r16:
  case F_r16i:
  case F_sluminance:
  case F_r32i:
  case F_r32:
  case F_r8i:
    cdata->_num_components = 1;
    break;

  case F_luminance_alpha:
  case F_luminance_alphamask:
  case F_rg16:
  case F_sluminance_alpha:
  case F_rg32:
  case F_rg8i:
  case F_rg:
    cdata->_num_components = 2;
    break;

  case F_rgb:
  case F_rgb5:
  case F_rgb8:
  case F_rgb12:
  case F_rgb332:
  case F_rgb16:
  case F_srgb:
  case F_rgb32:
  case F_rgb8i:
  case F_r11_g11_b10:
  case F_rgb9_e5:
    cdata->_num_components = 3;
    break;

  case F_rgba:
  case F_rgbm:
  case F_rgba4:
  case F_rgba5:
  case F_rgba8:
  case F_rgba12:
  case F_rgba16:
  case F_rgba32:
  case F_srgb_alpha:
  case F_rgba8i:
  case F_rgb10_a2:
    cdata->_num_components = 4;
    break;
  }
}

/**
 *
 */
void Texture::
do_set_component_type(CData *cdata, Texture::ComponentType component_type) {
  cdata->_component_type = component_type;

  switch (component_type) {
  case T_unsigned_byte:
  case T_byte:
    cdata->_component_width = 1;
    break;

  case T_unsigned_short:
  case T_short:
  case T_half_float:
    cdata->_component_width = 2;
    break;

  case T_float:
  case T_unsigned_int_24_8:
  case T_int:
  case T_unsigned_int:
    cdata->_component_width = 4;
    break;
  }
}

/**
 *
 */
void Texture::
do_set_x_size(CData *cdata, int x_size) {
  if (cdata->_x_size != x_size) {
    cdata->_x_size = x_size;
    cdata->inc_image_modified();
    do_clear_ram_image(cdata);
    do_set_pad_size(cdata, 0, 0, 0);
  }
}

/**
 *
 */
void Texture::
do_set_y_size(CData *cdata, int y_size) {
  if (cdata->_y_size != y_size) {
    nassertv((cdata->_texture_type != Texture::TT_buffer_texture &&
              cdata->_texture_type != Texture::TT_1d_texture) || y_size == 1);
    cdata->_y_size = y_size;
    cdata->inc_image_modified();
    do_clear_ram_image(cdata);
    do_set_pad_size(cdata, 0, 0, 0);
  }
}

/**
 * Changes the z size indicated for the texture.  This also implicitly unloads
 * the texture if it has already been loaded.
 */
void Texture::
do_set_z_size(CData *cdata, int z_size) {
  if (cdata->_z_size != z_size) {
    nassertv((cdata->_texture_type == Texture::TT_3d_texture) ||
             (cdata->_texture_type == Texture::TT_cube_map && z_size == 6) ||
             (cdata->_texture_type == Texture::TT_cube_map_array && z_size % 6 == 0) ||
             (cdata->_texture_type == Texture::TT_2d_texture_array) || (z_size == 1));
    cdata->_z_size = z_size;
    cdata->inc_image_modified();
    do_clear_ram_image(cdata);
    do_set_pad_size(cdata, 0, 0, 0);
  }
}

/**
 *
 */
void Texture::
do_set_num_views(CData *cdata, int num_views) {
  nassertv(num_views >= 1);
  if (cdata->_num_views != num_views) {
    cdata->_num_views = num_views;
    if (do_has_ram_image(cdata)) {
      cdata->inc_image_modified();
      do_clear_ram_image(cdata);
    }
    do_set_pad_size(cdata, 0, 0, 0);
  }
}

/**
 *
 */
void Texture::
do_set_wrap_u(CData *cdata, SamplerState::WrapMode wrap) {
  if (cdata->_default_sampler.get_wrap_u() != wrap) {
    cdata->inc_properties_modified();
    cdata->_default_sampler.set_wrap_u(wrap);
  }
}

/**
 *
 */
void Texture::
do_set_wrap_v(CData *cdata, SamplerState::WrapMode wrap) {
  if (cdata->_default_sampler.get_wrap_v() != wrap) {
    cdata->inc_properties_modified();
    cdata->_default_sampler.set_wrap_v(wrap);
  }
}

/**
 *
 */
void Texture::
do_set_wrap_w(CData *cdata, SamplerState::WrapMode wrap) {
  if (cdata->_default_sampler.get_wrap_w() != wrap) {
    cdata->inc_properties_modified();
    cdata->_default_sampler.set_wrap_w(wrap);
  }
}

/**
 *
 */
void Texture::
do_set_minfilter(CData *cdata, SamplerState::FilterType filter) {
  if (cdata->_default_sampler.get_minfilter() != filter) {
    cdata->inc_properties_modified();
    cdata->_default_sampler.set_minfilter(filter);
  }
}

/**
 *
 */
void Texture::
do_set_magfilter(CData *cdata, SamplerState::FilterType filter) {
  if (cdata->_default_sampler.get_magfilter() != filter) {
    cdata->inc_properties_modified();
    cdata->_default_sampler.set_magfilter(filter);
  }
}

/**
 *
 */
void Texture::
do_set_anisotropic_degree(CData *cdata, int anisotropic_degree) {
  if (cdata->_default_sampler.get_anisotropic_degree() != anisotropic_degree) {
    cdata->inc_properties_modified();
    cdata->_default_sampler.set_anisotropic_degree(anisotropic_degree);
  }
}

/**
 *
 */
void Texture::
do_set_border_color(CData *cdata, const LColor &color) {
  if (cdata->_default_sampler.get_border_color() != color) {
    cdata->inc_properties_modified();
    cdata->_default_sampler.set_border_color(color);
  }
}

/**
 *
 */
void Texture::
do_set_compression(CData *cdata, Texture::CompressionMode compression) {
  if (cdata->_compression != compression) {
    cdata->inc_properties_modified();
    cdata->_compression = compression;

    if (do_has_ram_image(cdata)) {
      bool has_compression = do_has_compression(cdata);
      bool has_ram_image_compression = (cdata->_ram_image_compression != CM_off);
      if (has_compression != has_ram_image_compression ||
          has_compression) {
        // Reload if we're turning compression on or off, or if we're changing
        // the compression mode to a different kind of compression.
        do_reload(cdata);
      }
    }
  }
}

/**
 *
 */
void Texture::
do_set_quality_level(CData *cdata, Texture::QualityLevel quality_level) {
  if (cdata->_quality_level != quality_level) {
    cdata->inc_properties_modified();
    cdata->_quality_level = quality_level;
  }
}

/**
 *
 */
bool Texture::
do_has_compression(const CData *cdata) const {
  if (cdata->_compression == CM_default) {
    return compressed_textures;
  } else {
    return (cdata->_compression != CM_off);
  }
}

/**
 * The protected implementation of has_ram_image(). Assumes the lock is
 * already held.
 */
bool Texture::
do_has_ram_image(const CData *cdata) const {
  return !cdata->_ram_images.empty() && !cdata->_ram_images[0]._image.empty();
}

/**
 * The protected implementation of has_uncompressed_ram_image().  Assumes the
 * lock is already held.
 */
bool Texture::
do_has_uncompressed_ram_image(const CData *cdata) const {
  return !cdata->_ram_images.empty() && !cdata->_ram_images[0]._image.empty() && cdata->_ram_image_compression == CM_off;
}

/**
 *
 */
CPTA_uchar Texture::
do_get_ram_image(CData *cdata) {
  if (!do_has_ram_image(cdata) && do_can_reload(cdata)) {
    do_reload_ram_image(cdata, true);

    if (do_has_ram_image(cdata)) {
      // Normally, we don't update the cdata->_modified semaphores in a
      // do_blah method, but we'll make an exception in this case, because
      // it's easiest to modify these here, and only when we know it's needed.
      cdata->inc_image_modified();
      cdata->inc_properties_modified();
    }
  }

  if (cdata->_ram_images.empty()) {
    return CPTA_uchar(get_class_type());
  }

  return cdata->_ram_images[0]._image;
}

/**
 *
 */
CPTA_uchar Texture::
do_get_uncompressed_ram_image(CData *cdata) {
  if (!cdata->_ram_images.empty() && cdata->_ram_image_compression != CM_off) {
    // We have an image in-ram, but it's compressed.  Try to uncompress it
    // first.
    if (do_uncompress_ram_image(cdata)) {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Uncompressed " << get_name() << "\n";
      }
      return cdata->_ram_images[0]._image;
    }
  }

  // Couldn't uncompress the existing image.  Try to reload it.
  if ((!do_has_ram_image(cdata) || cdata->_ram_image_compression != CM_off) && do_can_reload(cdata)) {
    do_reload_ram_image(cdata, false);
  }

  if (!cdata->_ram_images.empty() && cdata->_ram_image_compression != CM_off) {
    // Great, now we have an image.
    if (do_uncompress_ram_image(cdata)) {
      gobj_cat.info()
        << "Uncompressed " << get_name() << "\n";
      return cdata->_ram_images[0]._image;
    }
  }

  if (cdata->_ram_images.empty() || cdata->_ram_image_compression != CM_off) {
    return CPTA_uchar(get_class_type());
  }

  return cdata->_ram_images[0]._image;
}

/**
 * Returns the uncompressed system-RAM image data associated with the texture.
 * Rather than just returning a pointer to the data, like
 * get_uncompressed_ram_image, this function first processes the data and
 * reorders the components using the specified format string, and places these
 * into a new char array.
 *
 * The 'format' argument should specify in which order the components of the
 * texture must be.  For example, valid format strings are "RGBA", "GA",
 * "ABRG" or "AAA".  A component can also be written as "0" or "1", which
 * means an empty/black or a full/white channel, respectively.
 *
 * This function is particularly useful to copy an image in-memory to a
 * different library (for example, PIL or wxWidgets) that require a different
 * component order than Panda's internal format, BGRA. Note, however, that
 * this conversion can still be too slow if you want to do it every frame, and
 * should thus be avoided for that purpose.
 *
 * The only requirement for the reordering is that an uncompressed image must
 * be available.  If the RAM image is compressed, it will attempt to re-load
 * the texture from disk, if it doesn't find an uncompressed image there, it
 * will return NULL.
 */
CPTA_uchar Texture::
get_ram_image_as(const string &requested_format) {
  CDWriter cdata(_cycler, false);
  string format = upcase(requested_format);

  // Make sure we can grab something that's uncompressed.
  CPTA_uchar data = do_get_uncompressed_ram_image(cdata);
  if (data == nullptr) {
    gobj_cat.error() << "Couldn't find an uncompressed RAM image!\n";
    return CPTA_uchar(get_class_type());
  }
  int imgsize = cdata->_x_size * cdata->_y_size;
  nassertr(cdata->_num_components > 0 && cdata->_num_components <= 4, CPTA_uchar(get_class_type()));
  nassertr(data.size() == (size_t)(cdata->_component_width * cdata->_num_components * imgsize), CPTA_uchar(get_class_type()));

  // Check if the format is already what we have internally.
  if ((cdata->_num_components == 1 && format.size() == 1) ||
      (cdata->_num_components == 2 && format.size() == 2 && format.at(1) == 'A' && format.at(0) != 'A') ||
      (cdata->_num_components == 3 && format == "BGR") ||
      (cdata->_num_components == 4 && format == "BGRA")) {
    // The format string is already our format, so we just need to copy it.
    return CPTA_uchar(data);
  }

  // Check if we have an alpha channel, and remember which channel we use.
  int alpha = -1;
  if (Texture::has_alpha(cdata->_format)) {
    alpha = cdata->_num_components - 1;
  }

  // Validate the format beforehand.
  for (size_t i = 0; i < format.size(); ++i) {
    if (format[i] != 'B' && format[i] != 'G' && format[i] != 'R' &&
        format[i] != 'A' && format[i] != '0' && format[i] != '1') {
      gobj_cat.error() << "Unexpected component character '"
        << format[i] << "', expected one of RGBA01!\n";
      return CPTA_uchar(get_class_type());
    }
  }

  // Create a new empty array that can hold our image.
  PTA_uchar newdata = PTA_uchar::empty_array(imgsize * format.size() * cdata->_component_width, get_class_type());

  // These ifs are for optimization of commonly used image types.
  if (cdata->_component_width == 1) {
    if (format == "RGBA" && cdata->_num_components == 4) {
      const uint32_t *src = (const uint32_t *)data.p();
      uint32_t *dst = (uint32_t *)newdata.p();

      for (int p = 0; p < imgsize; ++p) {
        uint32_t v = *src++;
        *dst++ = ((v & 0xff00ff00u)) |
                 ((v & 0x00ff0000u) >> 16) |
                 ((v & 0x000000ffu) << 16);
      }
      return newdata;
    }
    if (format == "RGB" && cdata->_num_components == 4) {
      const uint32_t *src = (const uint32_t *)data.p();
      uint32_t *dst = (uint32_t *)newdata.p();

      // Convert blocks of 4 pixels at a time, so that we can treat both the
      // source and destination as 32-bit integers.
      int blocks = imgsize >> 2;
      for (int i = 0; i < blocks; ++i) {
        uint32_t v0 = *src++;
        uint32_t v1 = *src++;
        uint32_t v2 = *src++;
        uint32_t v3 = *src++;
        *dst++ = ((v0 & 0x00ff0000u) >> 16) |
                 ((v0 & 0x0000ff00u)) |
                 ((v0 & 0x000000ffu) << 16) |
                 ((v1 & 0x00ff0000u) << 8);
        *dst++ = ((v1 & 0x0000ff00u) >> 8) |
                 ((v1 & 0x000000ffu) << 8) |
                 ((v2 & 0x00ff0000u)) |
                 ((v2 & 0x0000ff00u) << 16);
        *dst++ = ((v2 & 0x000000ffu)) |
                 ((v3 & 0x00ff0000u) >> 8) |
                 ((v3 & 0x0000ff00u) << 8) |
                 ((v3 & 0x000000ffu) << 24);
      }

      // If the image size wasn't a multiple of 4, we may have a handful of
      // pixels left over.  Convert those the slower way.
      uint8_t *tail = (uint8_t *)dst;
      for (int i = (imgsize & ~0x3); i < imgsize; ++i) {
        uint32_t v = *src++;
        *tail++ = (v & 0x00ff0000u) >> 16;
        *tail++ = (v & 0x0000ff00u) >> 8;
        *tail++ = (v & 0x000000ffu);
      }
      return newdata;
    }
    if (format == "BGR" && cdata->_num_components == 4) {
      const uint32_t *src = (const uint32_t *)data.p();
      uint32_t *dst = (uint32_t *)newdata.p();

      // Convert blocks of 4 pixels at a time, so that we can treat both the
      // source and destination as 32-bit integers.
      int blocks = imgsize >> 2;
      for (int i = 0; i < blocks; ++i) {
        uint32_t v0 = *src++;
        uint32_t v1 = *src++;
        uint32_t v2 = *src++;
        uint32_t v3 = *src++;
        *dst++ = (v0 & 0x00ffffffu) | ((v1 & 0x000000ffu) << 24);
        *dst++ = ((v1 & 0x00ffff00u) >> 8) |  ((v2 & 0x0000ffffu) << 16);
        *dst++ = ((v2 & 0x00ff0000u) >> 16) | ((v3 & 0x00ffffffu) << 8);
      }

      // If the image size wasn't a multiple of 4, we may have a handful of
      // pixels left over.  Convert those the slower way.
      uint8_t *tail = (uint8_t *)dst;
      for (int i = (imgsize & ~0x3); i < imgsize; ++i) {
        uint32_t v = *src++;
        *tail++ = (v & 0x000000ffu);
        *tail++ = (v & 0x0000ff00u) >> 8;
        *tail++ = (v & 0x00ff0000u) >> 16;
      }
      return newdata;
    }
    const uint8_t *src = (const uint8_t *)data.p();
    uint8_t *dst = (uint8_t *)newdata.p();

    if (format == "RGB" && cdata->_num_components == 3) {
      for (int i = 0; i < imgsize; ++i) {
        *dst++ = src[2];
        *dst++ = src[1];
        *dst++ = src[0];
        src += 3;
      }
      return newdata;
    }
    if (format == "A" && cdata->_num_components != 3) {
      // We can generally rely on alpha to be the last component.
      for (int p = 0; p < imgsize; ++p) {
        dst[p] = src[alpha];
        src += cdata->_num_components;
      }
      return newdata;
    }
    // Fallback case for other 8-bit-per-channel formats.
    for (int p = 0; p < imgsize; ++p) {
      for (size_t i = 0; i < format.size(); ++i) {
        if (format[i] == 'B' || (cdata->_num_components <= 2 && format[i] != 'A')) {
          *dst++ = src[0];
        } else if (format[i] == 'G') {
          *dst++ = src[1];
        } else if (format[i] == 'R') {
          *dst++ = src[2];
        } else if (format[i] == 'A') {
          if (alpha >= 0) {
            *dst++ = src[alpha];
          } else {
            *dst++ = 0xff;
          }
        } else if (format[i] == '1') {
          *dst++ = 0xff;
        } else {
          *dst++ = 0x00;
        }
      }
      src += cdata->_num_components;
    }
    return newdata;
  }

  // The slow and general case.
  for (int p = 0; p < imgsize; ++p) {
    for (size_t i = 0; i < format.size(); ++i) {
      int component = 0;
      if (format[i] == 'B' || (cdata->_num_components <= 2 && format[i] != 'A')) {
        component = 0;
      } else if (format[i] == 'G') {
        component = 1;
      } else if (format[i] == 'R') {
        component = 2;
      } else if (format[i] == 'A') {
        if (alpha >= 0) {
          component = alpha;
        } else {
          memset((void*)(newdata + (p * format.size() + i) * cdata->_component_width), -1, cdata->_component_width);
          continue;
        }
      } else if (format[i] == '1') {
        memset((void*)(newdata + (p * format.size() + i) * cdata->_component_width), -1, cdata->_component_width);
        continue;
      } else {
        memset((void*)(newdata + (p * format.size() + i) * cdata->_component_width),  0, cdata->_component_width);
        continue;
      }
      memcpy((void*)(newdata + (p * format.size() + i) * cdata->_component_width),
             (void*)(data + (p * cdata->_num_components + component) * cdata->_component_width),
             cdata->_component_width);
    }
  }
  return newdata;
}

/**
 *
 */
void Texture::
do_set_simple_ram_image(CData *cdata, CPTA_uchar image, int x_size, int y_size) {
  nassertv(cdata->_texture_type == TT_2d_texture);
  size_t expected_page_size = (size_t)(x_size * y_size * 4);
  nassertv(image.size() == expected_page_size);

  cdata->_simple_x_size = x_size;
  cdata->_simple_y_size = y_size;
  cdata->_simple_ram_image._image = image.cast_non_const();
  cdata->_simple_ram_image._page_size = image.size();
  cdata->_simple_image_date_generated = (int32_t)time(nullptr);
  cdata->inc_simple_image_modified();
}

/**
 *
 */
int Texture::
do_get_expected_num_mipmap_levels(const CData *cdata) const {
  if (cdata->_texture_type == Texture::TT_buffer_texture) {
    return 1;
  }
  int size = max(cdata->_x_size, cdata->_y_size);
  if (cdata->_texture_type == Texture::TT_3d_texture) {
    size = max(size, cdata->_z_size);
  }
  int count = 1;
  while (size > 1) {
    size >>= 1;
    ++count;
  }
  return count;
}

/**
 *
 */
size_t Texture::
do_get_ram_mipmap_page_size(const CData *cdata, int n) const {
  if (cdata->_ram_image_compression != CM_off) {
    if (n >= 0 && n < (int)cdata->_ram_images.size()) {
      return cdata->_ram_images[n]._page_size;
    }
    return 0;
  } else {
    return do_get_expected_ram_mipmap_page_size(cdata, n);
  }
}

/**
 *
 */
int Texture::
do_get_expected_mipmap_x_size(const CData *cdata, int n) const {
  int size = max(cdata->_x_size, 1);
  while (n > 0 && size > 1) {
    size >>= 1;
    --n;
  }
  return size;
}

/**
 *
 */
int Texture::
do_get_expected_mipmap_y_size(const CData *cdata, int n) const {
  int size = max(cdata->_y_size, 1);
  while (n > 0 && size > 1) {
    size >>= 1;
    --n;
  }
  return size;
}

/**
 *
 */
int Texture::
do_get_expected_mipmap_z_size(const CData *cdata, int n) const {
  // 3-D textures have a different number of pages per each mipmap level.
  // Other kinds of textures--especially, cube map textures--always have the
  // same.
  if (cdata->_texture_type == Texture::TT_3d_texture) {
    int size = max(cdata->_z_size, 1);
    while (n > 0 && size > 1) {
      size >>= 1;
      --n;
    }
    return size;

  } else {
    return cdata->_z_size;
  }
}

/**
 *
 */
void Texture::
do_clear_simple_ram_image(CData *cdata) {
  cdata->_simple_x_size = 0;
  cdata->_simple_y_size = 0;
  cdata->_simple_ram_image._image.clear();
  cdata->_simple_ram_image._page_size = 0;
  cdata->_simple_image_date_generated = 0;

  // We allow this exception: we update the _simple_image_modified here, since
  // no one really cares much about that anyway, and it's convenient to do it
  // here.
  cdata->inc_simple_image_modified();
}

/**
 *
 */
void Texture::
do_clear_ram_mipmap_images(CData *cdata) {
  if (!cdata->_ram_images.empty()) {
    cdata->_ram_images.erase(cdata->_ram_images.begin() + 1, cdata->_ram_images.end());
  }
}

/**
 * Generates the RAM mipmap images for this texture, first uncompressing it as
 * required.  Will recompress the image if it was originally compressed,
 * unless allow_recompress is true.
 */
void Texture::
do_generate_ram_mipmap_images(CData *cdata, bool allow_recompress) {
  nassertv(do_has_ram_image(cdata));

  if (do_get_expected_num_mipmap_levels(cdata) == 1) {
    // Don't bother.
    return;
  }

  RamImage orig_compressed_image;
  CompressionMode orig_compression_mode = CM_off;

  if (cdata->_ram_image_compression != CM_off) {
    // The RAM image is compressed.  This means we need to uncompress it in
    // order to generate mipmap images.  Save the original first, to avoid
    // lossy recompression.
    orig_compressed_image = cdata->_ram_images[0];
    orig_compression_mode = cdata->_ram_image_compression;

    // Now try to get the uncompressed source image.
    do_get_uncompressed_ram_image(cdata);

    if (cdata->_ram_image_compression != CM_off) {
      gobj_cat.error()
        << "Cannot generate mipmap levels for image with compression "
        << cdata->_ram_image_compression << "\n";
      return;
    }
  }

  do_clear_ram_mipmap_images(cdata);

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Generating mipmap levels for " << *this << "\n";
  }

  if (cdata->_texture_type == Texture::TT_3d_texture && cdata->_z_size != 1) {
    // Eek, a 3-D texture.
    int x_size = cdata->_x_size;
    int y_size = cdata->_y_size;
    int z_size = cdata->_z_size;
    int n = 0;
    while (x_size > 1 || y_size > 1 || z_size > 1) {
      cdata->_ram_images.push_back(RamImage());
      do_filter_3d_mipmap_level(cdata, cdata->_ram_images[n + 1], cdata->_ram_images[n],
                                x_size, y_size, z_size);
      x_size = max(x_size >> 1, 1);
      y_size = max(y_size >> 1, 1);
      z_size = max(z_size >> 1, 1);
      ++n;
    }

  } else {
    // A 1-D, 2-D, or cube map texture.
    int x_size = cdata->_x_size;
    int y_size = cdata->_y_size;
    int n = 0;
    while (x_size > 1 || y_size > 1) {
      cdata->_ram_images.push_back(RamImage());
      do_filter_2d_mipmap_pages(cdata, cdata->_ram_images[n + 1], cdata->_ram_images[n],
                                x_size, y_size);
      x_size = max(x_size >> 1, 1);
      y_size = max(y_size >> 1, 1);
      ++n;
    }
  }

  if (orig_compression_mode != CM_off && allow_recompress) {
    // Now attempt to recompress the mipmap images according to the original
    // compression mode.  We don't need to bother compressing the first image
    // (it was already compressed, after all), so temporarily remove it from
    // the top of the mipmap stack, and compress all of the rest of them
    // instead.
    nassertv(cdata->_ram_images.size() > 1);
    int l0_x_size = cdata->_x_size;
    int l0_y_size = cdata->_y_size;
    int l0_z_size = cdata->_z_size;
    cdata->_x_size = do_get_expected_mipmap_x_size(cdata, 1);
    cdata->_y_size = do_get_expected_mipmap_y_size(cdata, 1);
    cdata->_z_size = do_get_expected_mipmap_z_size(cdata, 1);
    RamImage uncompressed_image = cdata->_ram_images[0];
    cdata->_ram_images.erase(cdata->_ram_images.begin());

    bool success = do_compress_ram_image(cdata, orig_compression_mode, QL_default, nullptr);
    // Now restore the toplevel image.
    if (success) {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Compressed " << get_name() << " generated mipmaps with "
          << cdata->_ram_image_compression << "\n";
      }
      cdata->_ram_images.insert(cdata->_ram_images.begin(), orig_compressed_image);
    } else {
      cdata->_ram_images.insert(cdata->_ram_images.begin(), uncompressed_image);
    }
    cdata->_x_size = l0_x_size;
    cdata->_y_size = l0_y_size;
    cdata->_z_size = l0_z_size;
  }
}

/**
 *
 */
void Texture::
do_set_pad_size(CData *cdata, int x, int y, int z) {
  if (x > cdata->_x_size) {
    x = cdata->_x_size;
  }
  if (y > cdata->_y_size) {
    y = cdata->_y_size;
  }
  if (z > cdata->_z_size) {
    z = cdata->_z_size;
  }

  cdata->_pad_x_size = x;
  cdata->_pad_y_size = y;
  cdata->_pad_z_size = z;
}

/**
 * Returns true if we can safely call do_reload_ram_image() in order to make
 * the image available, or false if we shouldn't do this (because we know from
 * a priori knowledge that it wouldn't work anyway).
 */
bool Texture::
do_can_reload(const CData *cdata) const {
  return (cdata->_loaded_from_image && !cdata->_fullpath.empty());
}

/**
 *
 */
bool Texture::
do_reload(CData *cdata) {
  if (do_can_reload(cdata)) {
    do_clear_ram_image(cdata);
    do_reload_ram_image(cdata, true);
    if (do_has_ram_image(cdata)) {
      // An explicit call to reload() should increment image_modified.
      cdata->inc_image_modified();
      return true;
    }
    return false;
  }

  // We don't have a filename to load from.
  return false;
}

/**
 * Returns true if there is a rawdata image that we have available to write to
 * the bam stream.  For a normal Texture, this is the same thing as
 * do_has_ram_image(), but a movie texture might define it differently.
 */
bool Texture::
do_has_bam_rawdata(const CData *cdata) const {
  return do_has_ram_image(cdata);
}

/**
 * If do_has_bam_rawdata() returned false, this attempts to reload the rawdata
 * image if possible.
 */
void Texture::
do_get_bam_rawdata(CData *cdata) {
  do_get_ram_image(cdata);
}

/**
 * Internal method to convert pixel data from the indicated PNMImage into the
 * given ram_image.
 */
void Texture::
convert_from_pnmimage(PTA_uchar &image, size_t page_size,
                      int row_stride, int x, int y, int z,
                      const PNMImage &pnmimage, int num_components,
                      int component_width) {
  int x_size = pnmimage.get_x_size();
  int y_size = pnmimage.get_y_size();
  xelval maxval = pnmimage.get_maxval();
  int pixel_size = num_components * component_width;

  int row_skip = 0;
  if (row_stride == 0) {
    row_stride = x_size;
  } else {
    row_skip = (row_stride - x_size) * pixel_size;
    nassertv(row_skip >= 0);
  }

  bool is_grayscale = (num_components == 1 || num_components == 2);
  bool has_alpha = (num_components == 2 || num_components == 4);
  bool img_has_alpha = pnmimage.has_alpha();

  int idx = page_size * z;
  nassertv(idx + page_size <= image.size());
  unsigned char *p = &image[idx];

  if (x != 0 || y != 0) {
    p += (row_stride * y + x) * pixel_size;
  }

  if (maxval == 255 && component_width == 1) {
    // Most common case: one byte per pixel, and the source image shows a
    // maxval of 255.  No scaling is necessary.  Because this is such a common
    // case, we break it out per component for best performance.
    const xel *array = pnmimage.get_array();
    switch (num_components) {
    case 1:
      for (int j = y_size-1; j >= 0; j--) {
        const xel *row = array + j * x_size;
        for (int i = 0; i < x_size; i++) {
          *p++ = (uchar)PPM_GETB(row[i]);
        }
        p += row_skip;
      }
      break;

    case 2:
      if (img_has_alpha) {
        const xelval *alpha = pnmimage.get_alpha_array();
        for (int j = y_size-1; j >= 0; j--) {
          const xel *row = array + j * x_size;
          const xelval *alpha_row = alpha + j * x_size;
          for (int i = 0; i < x_size; i++) {
            *p++ = (uchar)PPM_GETB(row[i]);
            *p++ = (uchar)alpha_row[i];
          }
          p += row_skip;
        }
      } else {
        for (int j = y_size-1; j >= 0; j--) {
          const xel *row = array + j * x_size;
          for (int i = 0; i < x_size; i++) {
            *p++ = (uchar)PPM_GETB(row[i]);
            *p++ = (uchar)255;
          }
          p += row_skip;
        }
      }
      break;

    case 3:
      for (int j = y_size-1; j >= 0; j--) {
        const xel *row = array + j * x_size;
        for (int i = 0; i < x_size; i++) {
          *p++ = (uchar)PPM_GETB(row[i]);
          *p++ = (uchar)PPM_GETG(row[i]);
          *p++ = (uchar)PPM_GETR(row[i]);
        }
        p += row_skip;
      }
      break;

    case 4:
      if (img_has_alpha) {
        const xelval *alpha = pnmimage.get_alpha_array();
        for (int j = y_size-1; j >= 0; j--) {
          const xel *row = array + j * x_size;
          const xelval *alpha_row = alpha + j * x_size;
          for (int i = 0; i < x_size; i++) {
            *p++ = (uchar)PPM_GETB(row[i]);
            *p++ = (uchar)PPM_GETG(row[i]);
            *p++ = (uchar)PPM_GETR(row[i]);
            *p++ = (uchar)alpha_row[i];
          }
          p += row_skip;
        }
      } else {
        for (int j = y_size-1; j >= 0; j--) {
          const xel *row = array + j * x_size;
          for (int i = 0; i < x_size; i++) {
            *p++ = (uchar)PPM_GETB(row[i]);
            *p++ = (uchar)PPM_GETG(row[i]);
            *p++ = (uchar)PPM_GETR(row[i]);
            *p++ = (uchar)255;
          }
          p += row_skip;
        }
      }
      break;

    default:
      nassertv(num_components >= 1 && num_components <= 4);
      break;
    }

  } else if (maxval == 65535 && component_width == 2) {
    // Another possible case: two bytes per pixel, and the source image shows
    // a maxval of 65535.  Again, no scaling is necessary.
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          store_unscaled_short(p, pnmimage.get_gray_val(i, j));
        } else {
          store_unscaled_short(p, pnmimage.get_blue_val(i, j));
          store_unscaled_short(p, pnmimage.get_green_val(i, j));
          store_unscaled_short(p, pnmimage.get_red_val(i, j));
        }
        if (has_alpha) {
          if (img_has_alpha) {
            store_unscaled_short(p, pnmimage.get_alpha_val(i, j));
          } else {
            store_unscaled_short(p, 65535);
          }
        }
      }
      p += row_skip;
    }

  } else if (component_width == 1) {
    // A less common case: one byte per pixel, but the maxval is something
    // other than 255.  In this case, we should scale the pixel values up to
    // the appropriate amount.
    double scale = 255.0 / (double)maxval;

    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          store_scaled_byte(p, pnmimage.get_gray_val(i, j), scale);
        } else {
          store_scaled_byte(p, pnmimage.get_blue_val(i, j), scale);
          store_scaled_byte(p, pnmimage.get_green_val(i, j), scale);
          store_scaled_byte(p, pnmimage.get_red_val(i, j), scale);
        }
        if (has_alpha) {
          if (img_has_alpha) {
            store_scaled_byte(p, pnmimage.get_alpha_val(i, j), scale);
          } else {
            store_unscaled_byte(p, 255);
          }
        }
      }
      p += row_skip;
    }

  } else {  // component_width == 2
    // Another uncommon case: two bytes per pixel, and the maxval is something
    // other than 65535.  Again, we must scale the pixel values.
    double scale = 65535.0 / (double)maxval;

    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        if (is_grayscale) {
          store_scaled_short(p, pnmimage.get_gray_val(i, j), scale);
        } else {
          store_scaled_short(p, pnmimage.get_blue_val(i, j), scale);
          store_scaled_short(p, pnmimage.get_green_val(i, j), scale);
          store_scaled_short(p, pnmimage.get_red_val(i, j), scale);
        }
        if (has_alpha) {
          if (img_has_alpha) {
            store_scaled_short(p, pnmimage.get_alpha_val(i, j), 1.0);
          } else {
            store_unscaled_short(p, 65535);
          }
        }
      }
      p += row_skip;
    }
  }
}

/**
 * Internal method to convert pixel data from the indicated PfmFile into the
 * given ram_image.
 */
void Texture::
convert_from_pfm(PTA_uchar &image, size_t page_size, int z,
                 const PfmFile &pfm, int num_components, int component_width) {
  nassertv(component_width == 4);  // Currently only PN_float32 is expected.
  int x_size = pfm.get_x_size();
  int y_size = pfm.get_y_size();

  int idx = page_size * z;
  nassertv(idx + page_size <= image.size());
  PN_float32 *p = (PN_float32 *)&image[idx];

  switch (num_components) {
  case 1:
    {
      for (int j = y_size-1; j >= 0; j--) {
        for (int i = 0; i < x_size; i++) {
          p[0] = pfm.get_channel(i, j, 0);
          ++p;
        }
      }
    }
    break;

  case 2:
    {
      for (int j = y_size-1; j >= 0; j--) {
        for (int i = 0; i < x_size; i++) {
          p[0] = pfm.get_channel(i, j, 0);
          p[1] = pfm.get_channel(i, j, 1);
          p += 2;
        }
      }
    }
    break;

  case 3:
    {
      // RGB -> BGR
      for (int j = y_size-1; j >= 0; j--) {
        for (int i = 0; i < x_size; i++) {
          p[0] = pfm.get_channel(i, j, 2);
          p[1] = pfm.get_channel(i, j, 1);
          p[2] = pfm.get_channel(i, j, 0);
          p += 3;
        }
      }
    }
    break;

  case 4:
    {
      // RGBA -> BGRA
      for (int j = y_size-1; j >= 0; j--) {
        for (int i = 0; i < x_size; i++) {
          p[0] = pfm.get_channel(i, j, 2);
          p[1] = pfm.get_channel(i, j, 1);
          p[2] = pfm.get_channel(i, j, 0);
          p[3] = pfm.get_channel(i, j, 3);
          p += 4;
        }
      }
    }
    break;

  default:
    nassert_raise("unexpected channel count");
    return;
  }

  nassertv((unsigned char *)p == &image[idx] + page_size);
}

/**
 * Internal method to convert pixel data to the indicated PNMImage from the
 * given ram_image.
 */
bool Texture::
convert_to_pnmimage(PNMImage &pnmimage, int x_size, int y_size,
                    int num_components, ComponentType component_type,
                    bool is_srgb, CPTA_uchar image, size_t page_size, int z) {
  xelval maxval = 0xff;
  if (component_type != T_unsigned_byte && component_type != T_byte) {
    maxval = 0xffff;
  }
  ColorSpace color_space = is_srgb ? CS_sRGB : CS_linear;
  pnmimage.clear(x_size, y_size, num_components, maxval, nullptr, color_space);
  bool has_alpha = pnmimage.has_alpha();
  bool is_grayscale = pnmimage.is_grayscale();

  int idx = page_size * z;
  nassertr(idx + page_size <= image.size(), false);

  xel *array = pnmimage.get_array();
  xelval *alpha = pnmimage.get_alpha_array();

  switch (component_type) {
  case T_unsigned_byte:
    if (is_grayscale) {
      const unsigned char *p = &image[idx];
      if (has_alpha) {
        for (int j = y_size-1; j >= 0; j--) {
          xel *row = array + j * x_size;
          xelval *alpha_row = alpha + j * x_size;
          for (int i = 0; i < x_size; i++) {
            PPM_PUTB(row[i], *p++);
            alpha_row[i] = *p++;
          }
        }
      } else {
        for (int j = y_size-1; j >= 0; j--) {
          xel *row = array + j * x_size;
          for (int i = 0; i < x_size; i++) {
            PPM_PUTB(row[i], *p++);
          }
        }
      }
      nassertr(p == &image[idx] + page_size, false);
    } else {
      const unsigned char *p = &image[idx];
      if (has_alpha) {
        for (int j = y_size-1; j >= 0; j--) {
          xel *row = array + j * x_size;
          xelval *alpha_row = alpha + j * x_size;
          for (int i = 0; i < x_size; i++) {
            PPM_PUTB(row[i], *p++);
            PPM_PUTG(row[i], *p++);
            PPM_PUTR(row[i], *p++);
            alpha_row[i] = *p++;
          }
        }
      } else {
        for (int j = y_size-1; j >= 0; j--) {
          xel *row = array + j * x_size;
          for (int i = 0; i < x_size; i++) {
            PPM_PUTB(row[i], *p++);
            PPM_PUTG(row[i], *p++);
            PPM_PUTR(row[i], *p++);
          }
        }
      }
      nassertr(p == &image[idx] + page_size, false);
    }
    break;

  case T_unsigned_short:
    {
      const uint16_t *p = (const uint16_t *)&image[idx];

      for (int j = y_size-1; j >= 0; j--) {
        xel *row = array + j * x_size;
        xelval *alpha_row = alpha + j * x_size;
        for (int i = 0; i < x_size; i++) {
          PPM_PUTB(row[i], *p++);
          if (!is_grayscale) {
            PPM_PUTG(row[i], *p++);
            PPM_PUTR(row[i], *p++);
          }
          if (has_alpha) {
            alpha_row[i] = *p++;
          }
        }
      }
      nassertr((const unsigned char *)p == &image[idx] + page_size, false);
    }
    break;

  case T_unsigned_int:
    {
      const uint32_t *p = (const uint32_t *)&image[idx];

      for (int j = y_size-1; j >= 0; j--) {
        xel *row = array + j * x_size;
        xelval *alpha_row = alpha + j * x_size;
        for (int i = 0; i < x_size; i++) {
          PPM_PUTB(row[i], (*p++) >> 16u);
          if (!is_grayscale) {
            PPM_PUTG(row[i], (*p++) >> 16u);
            PPM_PUTR(row[i], (*p++) >> 16u);
          }
          if (has_alpha) {
            alpha_row[i] = (*p++) >> 16u;
          }
        }
      }
      nassertr((const unsigned char *)p == &image[idx] + page_size, false);
    }
    break;

  case T_half_float:
    {
      const unsigned char *p = &image[idx];

      for (int j = y_size-1; j >= 0; j--) {
        for (int i = 0; i < x_size; i++) {
          pnmimage.set_blue(i, j, get_half_float(p));
          if (!is_grayscale) {
            pnmimage.set_green(i, j, get_half_float(p));
            pnmimage.set_red(i, j, get_half_float(p));
          }
          if (has_alpha) {
            pnmimage.set_alpha(i, j, get_half_float(p));
          }
        }
      }
      nassertr(p == &image[idx] + page_size, false);
    }
    break;

  default:
    return false;
  }

  return true;
}

/**
 * Internal method to convert pixel data to the indicated PfmFile from the
 * given ram_image.
 */
bool Texture::
convert_to_pfm(PfmFile &pfm, int x_size, int y_size,
               int num_components, int component_width,
               CPTA_uchar image, size_t page_size, int z) {
  nassertr(component_width == 4, false);  // Currently only PN_float32 is expected.
  pfm.clear(x_size, y_size, num_components);

  int idx = page_size * z;
  nassertr(idx + page_size <= image.size(), false);
  const PN_float32 *p = (const PN_float32 *)&image[idx];

  switch (num_components) {
  case 1:
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        pfm.set_channel(i, j, 0, p[0]);
        ++p;
      }
    }
    break;

  case 2:
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        pfm.set_channel(i, j, 0, p[0]);
        pfm.set_channel(i, j, 1, p[1]);
        p += 2;
      }
    }
    break;

  case 3:
    // BGR -> RGB
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        pfm.set_channel(i, j, 2, p[0]);
        pfm.set_channel(i, j, 1, p[1]);
        pfm.set_channel(i, j, 0, p[2]);
        p += 3;
      }
    }
    break;

  case 4:
    // BGRA -> RGBA
    for (int j = y_size-1; j >= 0; j--) {
      for (int i = 0; i < x_size; i++) {
        pfm.set_channel(i, j, 2, p[0]);
        pfm.set_channel(i, j, 1, p[1]);
        pfm.set_channel(i, j, 0, p[2]);
        pfm.set_channel(i, j, 3, p[3]);
        p += 4;
      }
    }
    break;

  default:
    nassert_raise("unexpected channel count");
    return false;
  }

  nassertr((unsigned char *)p == &image[idx] + page_size, false);
  return true;
}

/**
 * Called by read_dds for a DDS file in BGR8 format.
 */
PTA_uchar Texture::
read_dds_level_bgr8(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  // This is in order B, G, R.
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * 3;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    nassertr(p + row_bytes <= image.p() + size, PTA_uchar());
    in.read((char *)p, row_bytes);
  }

  return image;
}

/**
 * Called by read_dds for a DDS file in RGB8 format.
 */
PTA_uchar Texture::
read_dds_level_rgb8(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  // This is in order R, G, B.
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * 3;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    nassertr(p + row_bytes <= image.p() + size, PTA_uchar());
    in.read((char *)p, row_bytes);

    // Now reverse the r, g, b triples.
    for (int x = 0; x < x_size; ++x) {
      unsigned char r = p[0];
      p[0] = p[2];
      p[2] = r;
      p += 3;
    }
    nassertr(p <= image.p() + size, PTA_uchar());
  }

  return image;
}

/**
 * Called by read_dds for a DDS file in ABGR8 format.
 */
PTA_uchar Texture::
read_dds_level_abgr8(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  // This is laid out in order R, G, B, A.
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * 4;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    in.read((char *)p, row_bytes);

    uint32_t *pw = (uint32_t *)p;
    for (int x = 0; x < x_size; ++x) {
      uint32_t w = *pw;
#ifdef WORDS_BIGENDIAN
      // bigendian: convert R, G, B, A to B, G, R, A.
      w = ((w & 0xff00) << 16) | ((w & 0xff000000U) >> 16) | (w & 0xff00ff);
#else
      // littendian: convert A, B, G, R to to A, R, G, B.
      w = ((w & 0xff) << 16) | ((w & 0xff0000) >> 16) | (w & 0xff00ff00U);
#endif
      *pw = w;
      ++pw;
    }
    nassertr((unsigned char *)pw <= image.p() + size, PTA_uchar());
  }

  return image;
}

/**
 * Called by read_dds for a DDS file in RGBA8 format.
 */
PTA_uchar Texture::
read_dds_level_rgba8(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  // This is actually laid out in order B, G, R, A.
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * 4;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    nassertr(p + row_bytes <= image.p() + size, PTA_uchar());
    in.read((char *)p, row_bytes);
  }

  return image;
}

/**
 * Called by read_dds for a DDS file in ABGR16 format.
 */
PTA_uchar Texture::
read_dds_level_abgr16(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  // This is laid out in order R, G, B, A.
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * 8;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    in.read((char *)p, row_bytes);

    uint16_t *pw = (uint16_t *)p;
    for (int x = 0; x < x_size; ++x) {
      swap(pw[0], pw[2]);
      pw += 4;
    }
    nassertr((unsigned char *)pw <= image.p() + size, PTA_uchar());
  }

  return image;
}

/**
 * Called by read_dds for a DDS file in ABGR32 format.
 */
PTA_uchar Texture::
read_dds_level_abgr32(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  // This is laid out in order R, G, B, A.
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * 16;
  nassertr(row_bytes * y_size == size, PTA_uchar());
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    in.read((char *)p, row_bytes);

    uint32_t *pw = (uint32_t *)p;
    for (int x = 0; x < x_size; ++x) {
      swap(pw[0], pw[2]);
      pw += 4;
    }
    nassertr((unsigned char *)pw <= image.p() + size, PTA_uchar());
  }

  return image;
}

/**
 * Called by read_dds for a DDS file that needs no transformations applied.
 */
PTA_uchar Texture::
read_dds_level_raw(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * cdata->_num_components * cdata->_component_width;
  nassertr(row_bytes * y_size == size, PTA_uchar());
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    in.read((char *)p, row_bytes);
  }

  return image;
}

/**
 * Called by read_dds for a DDS file whose format isn't one we've specifically
 * optimized.
 */
PTA_uchar Texture::
read_dds_level_generic_uncompressed(Texture *tex, CData *cdata, const DDSHeader &header,
                                    int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  int pitch = (x_size * header.pf.rgb_bitcount) / 8;

  // MS says the pitch can be supplied in the header file and must be DWORD
  // aligned, but this appears to apply to level 0 mipmaps only (where it
  // almost always will be anyway).  Other mipmap levels seem to be tightly
  // packed, but there isn't a separate pitch for each mipmap level.  Weird.
  if (n == 0) {
    pitch = ((pitch + 3) / 4) * 4;
    if (header.dds_flags & DDSD_PITCH) {
      pitch = header.pitch;
    }
  }

  int bpp = header.pf.rgb_bitcount / 8;
  int skip_bytes = pitch - (bpp * x_size);
  nassertr(skip_bytes >= 0, PTA_uchar());

  unsigned int r_mask = header.pf.r_mask;
  unsigned int g_mask = header.pf.g_mask;
  unsigned int b_mask = header.pf.b_mask;
  unsigned int a_mask = header.pf.a_mask;

  // Determine the number of bits to shift each mask to the right so that the
  // lowest on bit is at bit 0.
  int r_shift = get_lowest_on_bit(r_mask);
  int g_shift = get_lowest_on_bit(g_mask);
  int b_shift = get_lowest_on_bit(b_mask);
  int a_shift = get_lowest_on_bit(a_mask);

  // Then determine the scale factor required to raise the highest color value
  // to 0xff000000.
  unsigned int r_scale = 0;
  if (r_mask != 0) {
    r_scale = 0xff000000 / (r_mask >> r_shift);
  }
  unsigned int g_scale = 0;
  if (g_mask != 0) {
    g_scale = 0xff000000 / (g_mask >> g_shift);
  }
  unsigned int b_scale = 0;
  if (b_mask != 0) {
    b_scale = 0xff000000 / (b_mask >> b_shift);
  }
  unsigned int a_scale = 0;
  if (a_mask != 0) {
    a_scale = 0xff000000 / (a_mask >> a_shift);
  }

  bool add_alpha = has_alpha(cdata->_format);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * cdata->_num_components;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    for (int x = 0; x < x_size; ++x) {

      // Read a little-endian numeric value of bpp bytes.
      unsigned int pixel = 0;
      int shift = 0;
      for (int bi = 0; bi < bpp; ++bi) {
        unsigned int ch = (unsigned char)in.get();
        pixel |= (ch << shift);
        shift += 8;
      }

      // Then break apart that value into its R, G, B, and maybe A components.
      unsigned int r = (((pixel & r_mask) >> r_shift) * r_scale) >> 24;
      unsigned int g = (((pixel & g_mask) >> g_shift) * g_scale) >> 24;
      unsigned int b = (((pixel & b_mask) >> b_shift) * b_scale) >> 24;

      // Store the components in the Texture's image data.
      store_unscaled_byte(p, b);
      store_unscaled_byte(p, g);
      store_unscaled_byte(p, r);
      if (add_alpha) {
        unsigned int a = (((pixel & a_mask) >> a_shift) * a_scale) >> 24;
        store_unscaled_byte(p, a);
      }
    }
    nassertr(p <= image.p() + size, PTA_uchar());
    for (int bi = 0; bi < skip_bytes; ++bi) {
      in.get();
    }
  }

  return image;
}

/**
 * Called by read_dds for a DDS file in uncompressed luminance or luminance-
 * alpha format.
 */
PTA_uchar Texture::
read_dds_level_luminance_uncompressed(Texture *tex, CData *cdata, const DDSHeader &header,
                                      int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  int pitch = (x_size * header.pf.rgb_bitcount) / 8;

  // MS says the pitch can be supplied in the header file and must be DWORD
  // aligned, but this appears to apply to level 0 mipmaps only (where it
  // almost always will be anyway).  Other mipmap levels seem to be tightly
  // packed, but there isn't a separate pitch for each mipmap level.  Weird.
  if (n == 0) {
    pitch = ((pitch + 3) / 4) * 4;
    if (header.dds_flags & DDSD_PITCH) {
      pitch = header.pitch;
    }
  }

  int bpp = header.pf.rgb_bitcount / 8;
  int skip_bytes = pitch - (bpp * x_size);
  nassertr(skip_bytes >= 0, PTA_uchar());

  unsigned int r_mask = header.pf.r_mask;
  unsigned int a_mask = header.pf.a_mask;

  // Determine the number of bits to shift each mask to the right so that the
  // lowest on bit is at bit 0.
  int r_shift = get_lowest_on_bit(r_mask);
  int a_shift = get_lowest_on_bit(a_mask);

  // Then determine the scale factor required to raise the highest color value
  // to 0xff000000.
  unsigned int r_scale = 0;
  if (r_mask != 0) {
    r_scale = 0xff000000 / (r_mask >> r_shift);
  }
  unsigned int a_scale = 0;
  if (a_mask != 0) {
    a_scale = 0xff000000 / (a_mask >> a_shift);
  }

  bool add_alpha = has_alpha(cdata->_format);

  size_t size = tex->do_get_expected_ram_mipmap_page_size(cdata, n);
  size_t row_bytes = x_size * cdata->_num_components;
  PTA_uchar image = PTA_uchar::empty_array(size);
  for (int y = y_size - 1; y >= 0; --y) {
    unsigned char *p = image.p() + y * row_bytes;
    for (int x = 0; x < x_size; ++x) {

      // Read a little-endian numeric value of bpp bytes.
      unsigned int pixel = 0;
      int shift = 0;
      for (int bi = 0; bi < bpp; ++bi) {
        unsigned int ch = (unsigned char)in.get();
        pixel |= (ch << shift);
        shift += 8;
      }

      unsigned int r = (((pixel & r_mask) >> r_shift) * r_scale) >> 24;

      // Store the components in the Texture's image data.
      store_unscaled_byte(p, r);
      if (add_alpha) {
        unsigned int a = (((pixel & a_mask) >> a_shift) * a_scale) >> 24;
        store_unscaled_byte(p, a);
      }
    }
    nassertr(p <= image.p() + size, PTA_uchar());
    for (int bi = 0; bi < skip_bytes; ++bi) {
      in.get();
    }
  }

  return image;
}

/**
 * Called by read_dds for DXT1 file format.
 */
PTA_uchar Texture::
read_dds_level_bc1(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  static const int div = 4;
  static const int block_bytes = 8;

  // The DXT1 image is divided into num_rows x num_cols blocks, where each
  // block represents 4x4 pixels.
  int num_cols = max(div, x_size) / div;
  int num_rows = max(div, y_size) / div;
  int row_length = num_cols * block_bytes;
  int linear_size = row_length * num_rows;

  if (n == 0) {
    if (header.dds_flags & DDSD_LINEARSIZE) {
      nassertr(linear_size == (int)header.pitch, PTA_uchar());
    }
  }

  PTA_uchar image = PTA_uchar::empty_array(linear_size);

  if (y_size >= 4) {
    // We have to flip the image as we read it, because of DirectX's inverted
    // sense of up.  That means we (a) reverse the order of the rows of blocks
    // . . .
    for (int ri = num_rows - 1; ri >= 0; --ri) {
      unsigned char *p = image.p() + row_length * ri;
      in.read((char *)p, row_length);

      for (int ci = 0; ci < num_cols; ++ci) {
        // . . . and (b) within each block, we reverse the 4 individual rows
        // of 4 pixels.
        uint32_t *cells = (uint32_t *)p;
        uint32_t w = cells[1];
        w = ((w & 0xff) << 24) | ((w & 0xff00) << 8) | ((w & 0xff0000) >> 8) | ((w & 0xff000000U) >> 24);
        cells[1] = w;

        p += block_bytes;
      }
    }

  } else if (y_size >= 2) {
    // To invert a two-pixel high image, we just flip two rows within a cell.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);

    for (int ci = 0; ci < num_cols; ++ci) {
      uint32_t *cells = (uint32_t *)p;
      uint32_t w = cells[1];
      w = ((w & 0xff) << 8) | ((w & 0xff00) >> 8);
      cells[1] = w;

      p += block_bytes;
    }

  } else if (y_size >= 1) {
    // No need to invert a one-pixel-high image.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);
  }

  return image;
}

/**
 * Called by read_dds for DXT2 or DXT3 file format.
 */
PTA_uchar Texture::
read_dds_level_bc2(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  static const int div = 4;
  static const int block_bytes = 16;

  // The DXT3 image is divided into num_rows x num_cols blocks, where each
  // block represents 4x4 pixels.  Unlike DXT1, each block consists of two
  // 8-byte chunks, representing the alpha and color separately.
  int num_cols = max(div, x_size) / div;
  int num_rows = max(div, y_size) / div;
  int row_length = num_cols * block_bytes;
  int linear_size = row_length * num_rows;

  if (n == 0) {
    if (header.dds_flags & DDSD_LINEARSIZE) {
      nassertr(linear_size == (int)header.pitch, PTA_uchar());
    }
  }

  PTA_uchar image = PTA_uchar::empty_array(linear_size);

  if (y_size >= 4) {
    // We have to flip the image as we read it, because of DirectX's inverted
    // sense of up.  That means we (a) reverse the order of the rows of blocks
    // . . .
    for (int ri = num_rows - 1; ri >= 0; --ri) {
      unsigned char *p = image.p() + row_length * ri;
      in.read((char *)p, row_length);

      for (int ci = 0; ci < num_cols; ++ci) {
        // . . . and (b) within each block, we reverse the 4 individual rows
        // of 4 pixels.
        uint32_t *cells = (uint32_t *)p;

        // Alpha.  The block is four 16-bit words of pixel data.
        uint32_t w0 = cells[0];
        uint32_t w1 = cells[1];
        w0 = ((w0 & 0xffff) << 16) | ((w0 & 0xffff0000U) >> 16);
        w1 = ((w1 & 0xffff) << 16) | ((w1 & 0xffff0000U) >> 16);
        cells[0] = w1;
        cells[1] = w0;

        // Color.  Only the second 32-bit dword of the color block represents
        // the pixel data.
        uint32_t w = cells[3];
        w = ((w & 0xff) << 24) | ((w & 0xff00) << 8) | ((w & 0xff0000) >> 8) | ((w & 0xff000000U) >> 24);
        cells[3] = w;

        p += block_bytes;
      }
    }

  } else if (y_size >= 2) {
    // To invert a two-pixel high image, we just flip two rows within a cell.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);

    for (int ci = 0; ci < num_cols; ++ci) {
      uint32_t *cells = (uint32_t *)p;

      uint32_t w0 = cells[0];
      w0 = ((w0 & 0xffff) << 16) | ((w0 & 0xffff0000U) >> 16);
      cells[0] = w0;

      uint32_t w = cells[3];
      w = ((w & 0xff) << 8) | ((w & 0xff00) >> 8);
      cells[3] = w;

      p += block_bytes;
    }

  } else if (y_size >= 1) {
    // No need to invert a one-pixel-high image.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);
  }

  return image;
}

/**
 * Called by read_dds for DXT4 or DXT5 file format.
 */
PTA_uchar Texture::
read_dds_level_bc3(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  static const int div = 4;
  static const int block_bytes = 16;

  // The DXT5 image is similar to DXT3, in that there each 4x4 block of pixels
  // consists of an alpha block and a color block, but the layout of the alpha
  // block is different.
  int num_cols = max(div, x_size) / div;
  int num_rows = max(div, y_size) / div;
  int row_length = num_cols * block_bytes;
  int linear_size = row_length * num_rows;

  if (n == 0) {
    if (header.dds_flags & DDSD_LINEARSIZE) {
      nassertr(linear_size == (int)header.pitch, PTA_uchar());
    }
  }

  PTA_uchar image = PTA_uchar::empty_array(linear_size);

  if (y_size >= 4) {
    // We have to flip the image as we read it, because of DirectX's inverted
    // sense of up.  That means we (a) reverse the order of the rows of blocks
    // . . .
    for (int ri = num_rows - 1; ri >= 0; --ri) {
      unsigned char *p = image.p() + row_length * ri;
      in.read((char *)p, row_length);

      for (int ci = 0; ci < num_cols; ++ci) {
        // . . . and (b) within each block, we reverse the 4 individual rows
        // of 4 pixels.
        uint32_t *cells = (uint32_t *)p;

        // Alpha.  The block is one 16-bit word of reference values, followed
        // by six words of pixel values, in 12-bit rows.  Tricky to invert.
        unsigned char p2 = p[2];
        unsigned char p3 = p[3];
        unsigned char p4 = p[4];
        unsigned char p5 = p[5];
        unsigned char p6 = p[6];
        unsigned char p7 = p[7];

        p[2] = ((p7 & 0xf) << 4) | ((p6 & 0xf0) >> 4);
        p[3] = ((p5 & 0xf) << 4) | ((p7 & 0xf0) >> 4);
        p[4] = ((p6 & 0xf) << 4) | ((p5 & 0xf0) >> 4);
        p[5] = ((p4 & 0xf) << 4) | ((p3 & 0xf0) >> 4);
        p[6] = ((p2 & 0xf) << 4) | ((p4 & 0xf0) >> 4);
        p[7] = ((p3 & 0xf) << 4) | ((p2 & 0xf0) >> 4);

        // Color.  Only the second 32-bit dword of the color block represents
        // the pixel data.
        uint32_t w = cells[3];
        w = ((w & 0xff) << 24) | ((w & 0xff00) << 8) | ((w & 0xff0000) >> 8) | ((w & 0xff000000U) >> 24);
        cells[3] = w;

        p += block_bytes;
      }
    }

  } else if (y_size >= 2) {
    // To invert a two-pixel high image, we just flip two rows within a cell.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);

    for (int ci = 0; ci < num_cols; ++ci) {
      uint32_t *cells = (uint32_t *)p;

      unsigned char p2 = p[2];
      unsigned char p3 = p[3];
      unsigned char p4 = p[4];

      p[2] = ((p4 & 0xf) << 4) | ((p3 & 0xf0) >> 4);
      p[3] = ((p2 & 0xf) << 4) | ((p4 & 0xf0) >> 4);
      p[4] = ((p3 & 0xf) << 4) | ((p2 & 0xf0) >> 4);

      uint32_t w0 = cells[0];
      w0 = ((w0 & 0xffff) << 16) | ((w0 & 0xffff0000U) >> 16);
      cells[0] = w0;

      uint32_t w = cells[3];
      w = ((w & 0xff) << 8) | ((w & 0xff00) >> 8);
      cells[3] = w;

      p += block_bytes;
    }

  } else if (y_size >= 1) {
    // No need to invert a one-pixel-high image.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);
  }

  return image;
}

/**
 * Called by read_dds for ATI1 compression.
 */
PTA_uchar Texture::
read_dds_level_bc4(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  static const int div = 4;
  static const int block_bytes = 8;

  // The ATI1 (BC4) format uses the same compression mechanism as the alpha
  // channel of DXT5.
  int num_cols = max(div, x_size) / div;
  int num_rows = max(div, y_size) / div;
  int row_length = num_cols * block_bytes;
  int linear_size = row_length * num_rows;

  if (n == 0) {
    if (header.dds_flags & DDSD_LINEARSIZE) {
      nassertr(linear_size == (int)header.pitch, PTA_uchar());
    }
  }

  PTA_uchar image = PTA_uchar::empty_array(linear_size);

  if (y_size >= 4) {
    // We have to flip the image as we read it, because of DirectX's inverted
    // sense of up.  That means we (a) reverse the order of the rows of blocks
    // . . .
    for (int ri = num_rows - 1; ri >= 0; --ri) {
      unsigned char *p = image.p() + row_length * ri;
      in.read((char *)p, row_length);

      for (int ci = 0; ci < num_cols; ++ci) {
        // . . . and (b) within each block, we reverse the 4 individual rows
        // of 4 pixels.  The block is one 16-bit word of reference values,
        // followed by six words of pixel values, in 12-bit rows.  Tricky to
        // invert.
        unsigned char p2 = p[2];
        unsigned char p3 = p[3];
        unsigned char p4 = p[4];
        unsigned char p5 = p[5];
        unsigned char p6 = p[6];
        unsigned char p7 = p[7];

        p[2] = ((p7 & 0xf) << 4) | ((p6 & 0xf0) >> 4);
        p[3] = ((p5 & 0xf) << 4) | ((p7 & 0xf0) >> 4);
        p[4] = ((p6 & 0xf) << 4) | ((p5 & 0xf0) >> 4);
        p[5] = ((p4 & 0xf) << 4) | ((p3 & 0xf0) >> 4);
        p[6] = ((p2 & 0xf) << 4) | ((p4 & 0xf0) >> 4);
        p[7] = ((p3 & 0xf) << 4) | ((p2 & 0xf0) >> 4);

        p += block_bytes;
      }
    }

  } else if (y_size >= 2) {
    // To invert a two-pixel high image, we just flip two rows within a cell.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);

    for (int ci = 0; ci < num_cols; ++ci) {
      unsigned char p2 = p[2];
      unsigned char p3 = p[3];
      unsigned char p4 = p[4];

      p[2] = ((p4 & 0xf) << 4) | ((p3 & 0xf0) >> 4);
      p[3] = ((p2 & 0xf) << 4) | ((p4 & 0xf0) >> 4);
      p[4] = ((p3 & 0xf) << 4) | ((p2 & 0xf0) >> 4);

      p += block_bytes;
    }

  } else if (y_size >= 1) {
    // No need to invert a one-pixel-high image.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);
  }

  return image;
}

/**
 * Called by read_dds for ATI2 compression.
 */
PTA_uchar Texture::
read_dds_level_bc5(Texture *tex, CData *cdata, const DDSHeader &header, int n, istream &in) {
  int x_size = tex->do_get_expected_mipmap_x_size(cdata, n);
  int y_size = tex->do_get_expected_mipmap_y_size(cdata, n);

  // The ATI2 (BC5) format uses the same compression mechanism as the ATI1
  // (BC4) format, but doubles the channels.
  int num_cols = max(4, x_size) / 2;
  int num_rows = max(4, y_size) / 4;
  int row_length = num_cols * 8;
  int linear_size = row_length * num_rows;

  if (n == 0) {
    if (header.dds_flags & DDSD_LINEARSIZE) {
      nassertr(linear_size == (int)header.pitch, PTA_uchar());
    }
  }

  PTA_uchar image = PTA_uchar::empty_array(linear_size);

  if (y_size >= 4) {
    // We have to flip the image as we read it, because of DirectX's inverted
    // sense of up.  That means we (a) reverse the order of the rows of blocks
    // . . .
    for (int ri = num_rows - 1; ri >= 0; --ri) {
      unsigned char *p = image.p() + row_length * ri;
      in.read((char *)p, row_length);

      for (int ci = 0; ci < num_cols; ++ci) {
        // . . . and (b) within each block, we reverse the 4 individual rows
        // of 4 pixels.  The block is one 16-bit word of reference values,
        // followed by six words of pixel values, in 12-bit rows.  Tricky to
        // invert.
        unsigned char p2 = p[2];
        unsigned char p3 = p[3];
        unsigned char p4 = p[4];
        unsigned char p5 = p[5];
        unsigned char p6 = p[6];
        unsigned char p7 = p[7];

        p[2] = ((p7 & 0xf) << 4) | ((p6 & 0xf0) >> 4);
        p[3] = ((p5 & 0xf) << 4) | ((p7 & 0xf0) >> 4);
        p[4] = ((p6 & 0xf) << 4) | ((p5 & 0xf0) >> 4);
        p[5] = ((p4 & 0xf) << 4) | ((p3 & 0xf0) >> 4);
        p[6] = ((p2 & 0xf) << 4) | ((p4 & 0xf0) >> 4);
        p[7] = ((p3 & 0xf) << 4) | ((p2 & 0xf0) >> 4);

        p += 8;
      }
    }

  } else if (y_size >= 2) {
    // To invert a two-pixel high image, we just flip two rows within a cell.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);

    for (int ci = 0; ci < num_cols; ++ci) {
      unsigned char p2 = p[2];
      unsigned char p3 = p[3];
      unsigned char p4 = p[4];

      p[2] = ((p4 & 0xf) << 4) | ((p3 & 0xf0) >> 4);
      p[3] = ((p2 & 0xf) << 4) | ((p4 & 0xf0) >> 4);
      p[4] = ((p3 & 0xf) << 4) | ((p2 & 0xf0) >> 4);

      p += 8;
    }

  } else if (y_size >= 1) {
    // No need to invert a one-pixel-high image.
    unsigned char *p = image.p();
    in.read((char *)p, row_length);
  }

  return image;
}

/**
 * Removes the indicated PreparedGraphicsObjects table from the Texture's
 * table, without actually releasing the texture.  This is intended to be
 * called only from PreparedGraphicsObjects::release_texture(); it should
 * never be called by user code.
 */
void Texture::
clear_prepared(int view, PreparedGraphicsObjects *prepared_objects) {
  PreparedViews::iterator pvi;
  pvi = _prepared_views.find(prepared_objects);
  if (pvi != _prepared_views.end()) {
    Contexts &contexts = (*pvi).second;
    Contexts::iterator ci;
    ci = contexts.find(view);
    if (ci != contexts.end()) {
      contexts.erase(ci);
    }

    if (contexts.empty()) {
      _prepared_views.erase(pvi);
    }
  }
}

/**
 * Reduces the number of channels in the texture, if necessary, according to
 * num_channels.
 */
void Texture::
consider_downgrade(PNMImage &pnmimage, int num_channels, const string &name) {
  if (num_channels != 0 && num_channels < pnmimage.get_num_channels()) {
    // One special case: we can't reduce from 3 to 2 components, since that
    // would require adding an alpha channel.
    if (pnmimage.get_num_channels() == 3 && num_channels == 2) {
      return;
    }

    gobj_cat.info()
      << "Downgrading " << name << " from "
      << pnmimage.get_num_channels() << " components to "
      << num_channels << ".\n";
    pnmimage.set_num_channels(num_channels);
  }
}

/**
 * Called by generate_simple_ram_image(), this compares the two PNMImages
 * pixel-by-pixel.  If they're similar enough (within a given threshold),
 * returns true.
 */
bool Texture::
compare_images(const PNMImage &a, const PNMImage &b) {
  nassertr(a.get_maxval() == 255 && b.get_maxval() == 255, false);
  nassertr(a.get_num_channels() == 4 && b.get_num_channels() == 4, false);
  nassertr(a.get_x_size() == b.get_x_size() &&
           a.get_y_size() == b.get_y_size(), false);

  const xel *a_array = a.get_array();
  const xel *b_array = b.get_array();
  const xelval *a_alpha = a.get_alpha_array();
  const xelval *b_alpha = b.get_alpha_array();

  int x_size = a.get_x_size();

  int delta = 0;
  for (int yi = 0; yi < a.get_y_size(); ++yi) {
    const xel *a_row = a_array + yi * x_size;
    const xel *b_row = b_array + yi * x_size;
    const xelval *a_alpha_row = a_alpha + yi * x_size;
    const xelval *b_alpha_row = b_alpha + yi * x_size;
    for (int xi = 0; xi < x_size; ++xi) {
      delta += abs(PPM_GETR(a_row[xi]) - PPM_GETR(b_row[xi]));
      delta += abs(PPM_GETG(a_row[xi]) - PPM_GETG(b_row[xi]));
      delta += abs(PPM_GETB(a_row[xi]) - PPM_GETB(b_row[xi]));
      delta += abs(a_alpha_row[xi] - b_alpha_row[xi]);
    }
  }

  double average_delta = (double)delta / ((double)a.get_x_size() * (double)b.get_y_size() * (double)a.get_maxval());
  return (average_delta <= simple_image_threshold);
}

/**
 * Generates the next mipmap level from the previous one.  If there are
 * multiple pages (e.g.  a cube map), generates each page independently.
 *
 * x_size and y_size are the size of the previous level.  They need not be a
 * power of 2, or even a multiple of 2.
 *
 * Assumes the lock is already held.
 */
void Texture::
do_filter_2d_mipmap_pages(const CData *cdata,
                          Texture::RamImage &to, const Texture::RamImage &from,
                          int x_size, int y_size) const {
  Filter2DComponent *filter_component;
  Filter2DComponent *filter_alpha;

  if (is_srgb(cdata->_format)) {
    // We currently only support sRGB mipmap generation for unsigned byte
    // textures, due to our use of a lookup table.
    nassertv(cdata->_component_type == T_unsigned_byte);

    if (has_sse2_sRGB_encode()) {
      filter_component = &filter_2d_unsigned_byte_srgb_sse2;
    } else {
      filter_component = &filter_2d_unsigned_byte_srgb;
    }

    // Alpha is always linear.
    filter_alpha = &filter_2d_unsigned_byte;

  } else {
    switch (cdata->_component_type) {
    case T_unsigned_byte:
      filter_component = &filter_2d_unsigned_byte;
      break;

    case T_unsigned_short:
      filter_component = &filter_2d_unsigned_short;
      break;

    case T_float:
      filter_component = &filter_2d_float;
      break;

    default:
      gobj_cat.error()
        << "Unable to generate mipmaps for 2D texture with component type "
        << cdata->_component_type << "!";
      return;
    }
    filter_alpha = filter_component;
  }

  size_t pixel_size = cdata->_num_components * cdata->_component_width;
  size_t row_size = (size_t)x_size * pixel_size;

  int to_x_size = max(x_size >> 1, 1);
  int to_y_size = max(y_size >> 1, 1);

  size_t to_row_size = (size_t)to_x_size * pixel_size;
  to._page_size = (size_t)to_y_size * to_row_size;
  to._image = PTA_uchar::empty_array(to._page_size * cdata->_z_size * cdata->_num_views, get_class_type());

  bool alpha = has_alpha(cdata->_format);
  int num_color_components = cdata->_num_components;
  if (alpha) {
    --num_color_components;
  }

  int num_pages = cdata->_z_size * cdata->_num_views;
  for (int z = 0; z < num_pages; ++z) {
    // For each level.
    unsigned char *p = to._image.p() + z * to._page_size;
    nassertv(p <= to._image.p() + to._image.size() + to._page_size);
    const unsigned char *q = from._image.p() + z * from._page_size;
    nassertv(q <= from._image.p() + from._image.size() + from._page_size);
    if (y_size != 1) {
      int y;
      for (y = 0; y < y_size - 1; y += 2) {
        // For each row.
        nassertv(p == to._image.p() + z * to._page_size + (y / 2) * to_row_size);
        nassertv(q == from._image.p() + z * from._page_size + y * row_size);
        if (x_size != 1) {
          int x;
          for (x = 0; x < x_size - 1; x += 2) {
            // For each pixel.
            for (int c = 0; c < num_color_components; ++c) {
              // For each component.
              filter_component(p, q, pixel_size, row_size);
            }
            if (alpha) {
              filter_alpha(p, q, pixel_size, row_size);
            }
            q += pixel_size;
          }
          if (x < x_size) {
            // Skip the last odd pixel.
            q += pixel_size;
          }
        } else {
          // Just one pixel.
          for (int c = 0; c < num_color_components; ++c) {
            // For each component.
            filter_component(p, q, 0, row_size);
          }
          if (alpha) {
            filter_alpha(p, q, 0, row_size);
          }
        }
        q += row_size;
        Thread::consider_yield();
      }
      if (y < y_size) {
        // Skip the last odd row.
        q += row_size;
      }
    } else {
      // Just one row.
      if (x_size != 1) {
        int x;
        for (x = 0; x < x_size - 1; x += 2) {
          // For each pixel.
          for (int c = 0; c < num_color_components; ++c) {
            // For each component.
            filter_component(p, q, pixel_size, 0);
          }
          if (alpha) {
            filter_alpha(p, q, pixel_size, 0);
          }
          q += pixel_size;
        }
        if (x < x_size) {
          // Skip the last odd pixel.
          q += pixel_size;
        }
      } else {
        // Just one pixel.
        for (int c = 0; c < num_color_components; ++c) {
          // For each component.
          filter_component(p, q, 0, 0);
        }
        if (alpha) {
          filter_alpha(p, q, pixel_size, 0);
        }
      }
    }

    nassertv(p == to._image.p() + (z + 1) * to._page_size);
    nassertv(q == from._image.p() + (z + 1) * from._page_size);
  }
}

/**
 * Generates the next mipmap level from the previous one, treating all the
 * pages of the level as a single 3-d block of pixels.
 *
 * x_size, y_size, and z_size are the size of the previous level.  They need
 * not be a power of 2, or even a multiple of 2.
 *
 * Assumes the lock is already held.
 */
void Texture::
do_filter_3d_mipmap_level(const CData *cdata,
                          Texture::RamImage &to, const Texture::RamImage &from,
                          int x_size, int y_size, int z_size) const {
  Filter3DComponent *filter_component;
  Filter3DComponent *filter_alpha;

  if (is_srgb(cdata->_format)) {
    // We currently only support sRGB mipmap generation for unsigned byte
    // textures, due to our use of a lookup table.
    nassertv(cdata->_component_type == T_unsigned_byte);

    if (has_sse2_sRGB_encode()) {
      filter_component = &filter_3d_unsigned_byte_srgb_sse2;
    } else {
      filter_component = &filter_3d_unsigned_byte_srgb;
    }

    // Alpha is always linear.
    filter_alpha = &filter_3d_unsigned_byte;

  } else {
    switch (cdata->_component_type) {
    case T_unsigned_byte:
      filter_component = &filter_3d_unsigned_byte;
      break;

    case T_unsigned_short:
      filter_component = &filter_3d_unsigned_short;
      break;

    case T_float:
      filter_component = &filter_3d_float;
      break;

    default:
      gobj_cat.error()
        << "Unable to generate mipmaps for 3D texture with component type "
        << cdata->_component_type << "!";
      return;
    }
    filter_alpha = filter_component;
  }

  size_t pixel_size = cdata->_num_components * cdata->_component_width;
  size_t row_size = (size_t)x_size * pixel_size;
  size_t page_size = (size_t)y_size * row_size;
  size_t view_size = (size_t)z_size * page_size;

  int to_x_size = max(x_size >> 1, 1);
  int to_y_size = max(y_size >> 1, 1);
  int to_z_size = max(z_size >> 1, 1);

  size_t to_row_size = (size_t)to_x_size * pixel_size;
  size_t to_page_size = (size_t)to_y_size * to_row_size;
  size_t to_view_size = (size_t)to_z_size * to_page_size;
  to._page_size = to_page_size;
  to._image = PTA_uchar::empty_array(to_page_size * to_z_size * cdata->_num_views, get_class_type());

  bool alpha = has_alpha(cdata->_format);
  int num_color_components = cdata->_num_components;
  if (alpha) {
    --num_color_components;
  }

  for (int view = 0; view < cdata->_num_views; ++view) {
    unsigned char *start_to = to._image.p() + view * to_view_size;
    const unsigned char *start_from = from._image.p() + view * view_size;
    nassertv(start_to + to_view_size <= to._image.p() + to._image.size());
    nassertv(start_from + view_size <= from._image.p() + from._image.size());
    unsigned char *p = start_to;
    const unsigned char *q = start_from;
    if (z_size != 1) {
      int z;
      for (z = 0; z < z_size - 1; z += 2) {
        // For each level.
        nassertv(p == start_to + (z / 2) * to_page_size);
        nassertv(q == start_from + z * page_size);
        if (y_size != 1) {
          int y;
          for (y = 0; y < y_size - 1; y += 2) {
            // For each row.
            nassertv(p == start_to + (z / 2) * to_page_size + (y / 2) * to_row_size);
            nassertv(q == start_from + z * page_size + y * row_size);
            if (x_size != 1) {
              int x;
              for (x = 0; x < x_size - 1; x += 2) {
                // For each pixel.
                for (int c = 0; c < num_color_components; ++c) {
                  // For each component.
                  filter_component(p, q, pixel_size, row_size, page_size);
                }
                if (alpha) {
                  filter_alpha(p, q, pixel_size, row_size, page_size);
                }
                q += pixel_size;
              }
              if (x < x_size) {
                // Skip the last odd pixel.
                q += pixel_size;
              }
            } else {
              // Just one pixel.
              for (int c = 0; c < num_color_components; ++c) {
                // For each component.
                filter_component(p, q, 0, row_size, page_size);
              }
              if (alpha) {
                filter_alpha(p, q, 0, row_size, page_size);
              }
            }
            q += row_size;
            Thread::consider_yield();
          }
          if (y < y_size) {
            // Skip the last odd row.
            q += row_size;
          }
        } else {
          // Just one row.
          if (x_size != 1) {
            int x;
            for (x = 0; x < x_size - 1; x += 2) {
              // For each pixel.
              for (int c = 0; c < num_color_components; ++c) {
                // For each component.
                filter_component(p, q, pixel_size, 0, page_size);
              }
              if (alpha) {
                filter_alpha(p, q, pixel_size, 0, page_size);
              }
              q += pixel_size;
            }
            if (x < x_size) {
              // Skip the last odd pixel.
              q += pixel_size;
            }
          } else {
            // Just one pixel.
            for (int c = 0; c < num_color_components; ++c) {
              // For each component.
              filter_component(p, q, 0, 0, page_size);
            }
            if (alpha) {
              filter_alpha(p, q, 0, 0, page_size);
            }
          }
        }
        q += page_size;
      }
      if (z < z_size) {
        // Skip the last odd page.
        q += page_size;
      }
    } else {
      // Just one page.
      if (y_size != 1) {
        int y;
        for (y = 0; y < y_size - 1; y += 2) {
          // For each row.
          nassertv(p == start_to + (y / 2) * to_row_size);
          nassertv(q == start_from + y * row_size);
          if (x_size != 1) {
            int x;
            for (x = 0; x < x_size - 1; x += 2) {
              // For each pixel.
              for (int c = 0; c < num_color_components; ++c) {
                // For each component.
                filter_component(p, q, pixel_size, row_size, 0);
              }
              if (alpha) {
                filter_alpha(p, q, pixel_size, row_size, 0);
              }
              q += pixel_size;
            }
            if (x < x_size) {
              // Skip the last odd pixel.
              q += pixel_size;
            }
          } else {
            // Just one pixel.
            for (int c = 0; c < num_color_components; ++c) {
              // For each component.
              filter_component(p, q, 0, row_size, 0);
            }
            if (alpha) {
              filter_alpha(p, q, 0, row_size, 0);
            }
          }
          q += row_size;
          Thread::consider_yield();
        }
        if (y < y_size) {
          // Skip the last odd row.
          q += row_size;
        }
      } else {
        // Just one row.
        if (x_size != 1) {
          int x;
          for (x = 0; x < x_size - 1; x += 2) {
            // For each pixel.
            for (int c = 0; c < num_color_components; ++c) {
              // For each component.
              filter_component(p, q, pixel_size, 0, 0);
            }
            if (alpha) {
              filter_alpha(p, q, pixel_size, 0, 0);
            }
            q += pixel_size;
          }
          if (x < x_size) {
            // Skip the last odd pixel.
            q += pixel_size;
          }
        } else {
          // Just one pixel.
          for (int c = 0; c < num_color_components; ++c) {
            // For each component.
            filter_component(p, q, 0, 0, 0);
          }
          if (alpha) {
            filter_alpha(p, q, 0, 0, 0);
          }
        }
      }
    }

    nassertv(p == start_to + to_z_size * to_page_size);
    nassertv(q == start_from + z_size * page_size);
  }
}

/**
 * Averages a 2x2 block of pixel components into a single pixel component, for
 * producing the next mipmap level.  Increments p and q to the next component.
 */
void Texture::
filter_2d_unsigned_byte(unsigned char *&p, const unsigned char *&q,
                        size_t pixel_size, size_t row_size) {
  unsigned int result = ((unsigned int)q[0] +
                         (unsigned int)q[pixel_size] +
                         (unsigned int)q[row_size] +
                         (unsigned int)q[pixel_size + row_size]) >> 2;
  *p = (unsigned char)result;
  ++p;
  ++q;
}

/**
 * Averages a 2x2 block of pixel components into a single pixel component, for
 * producing the next mipmap level.  Increments p and q to the next component.
 */
void Texture::
filter_2d_unsigned_byte_srgb(unsigned char *&p, const unsigned char *&q,
                             size_t pixel_size, size_t row_size) {
  float result = (decode_sRGB_float(q[0]) +
                  decode_sRGB_float(q[pixel_size]) +
                  decode_sRGB_float(q[row_size]) +
                  decode_sRGB_float(q[pixel_size + row_size]));

  *p = encode_sRGB_uchar(result * 0.25f);
  ++p;
  ++q;
}

/**
 * Averages a 2x2 block of pixel components into a single pixel component, for
 * producing the next mipmap level.  Increments p and q to the next component.
 */
void Texture::
filter_2d_unsigned_byte_srgb_sse2(unsigned char *&p, const unsigned char *&q,
                                  size_t pixel_size, size_t row_size) {
  float result = (decode_sRGB_float(q[0]) +
                  decode_sRGB_float(q[pixel_size]) +
                  decode_sRGB_float(q[row_size]) +
                  decode_sRGB_float(q[pixel_size + row_size]));

  *p = encode_sRGB_uchar_sse2(result * 0.25f);
  ++p;
  ++q;
}

/**
 * Averages a 2x2 block of pixel components into a single pixel component, for
 * producing the next mipmap level.  Increments p and q to the next component.
 */
void Texture::
filter_2d_unsigned_short(unsigned char *&p, const unsigned char *&q,
                         size_t pixel_size, size_t row_size) {
  unsigned int result = ((unsigned int)*(unsigned short *)&q[0] +
                         (unsigned int)*(unsigned short *)&q[pixel_size] +
                         (unsigned int)*(unsigned short *)&q[row_size] +
                         (unsigned int)*(unsigned short *)&q[pixel_size + row_size]) >> 2;
  store_unscaled_short(p, result);
  q += 2;
}

/**
 * Averages a 2x2 block of pixel components into a single pixel component, for
 * producing the next mipmap level.  Increments p and q to the next component.
 */
void Texture::
filter_2d_float(unsigned char *&p, const unsigned char *&q,
                size_t pixel_size, size_t row_size) {
  *(float *)p = (*(float *)&q[0] +
                 *(float *)&q[pixel_size] +
                 *(float *)&q[row_size] +
                 *(float *)&q[pixel_size + row_size]) / 4.0f;
  p += 4;
  q += 4;
}

/**
 * Averages a 2x2x2 block of pixel components into a single pixel component,
 * for producing the next mipmap level.  Increments p and q to the next
 * component.
 */
void Texture::
filter_3d_unsigned_byte(unsigned char *&p, const unsigned char *&q,
                        size_t pixel_size, size_t row_size, size_t page_size) {
  unsigned int result = ((unsigned int)q[0] +
                         (unsigned int)q[pixel_size] +
                         (unsigned int)q[row_size] +
                         (unsigned int)q[pixel_size + row_size] +
                         (unsigned int)q[page_size] +
                         (unsigned int)q[pixel_size + page_size] +
                         (unsigned int)q[row_size + page_size] +
                         (unsigned int)q[pixel_size + row_size + page_size]) >> 3;
  *p = (unsigned char)result;
  ++p;
  ++q;
}

/**
 * Averages a 2x2x2 block of pixel components into a single pixel component,
 * for producing the next mipmap level.  Increments p and q to the next
 * component.
 */
void Texture::
filter_3d_unsigned_byte_srgb(unsigned char *&p, const unsigned char *&q,
                             size_t pixel_size, size_t row_size, size_t page_size) {
  float result = (decode_sRGB_float(q[0]) +
                  decode_sRGB_float(q[pixel_size]) +
                  decode_sRGB_float(q[row_size]) +
                  decode_sRGB_float(q[pixel_size + row_size]) +
                  decode_sRGB_float(q[page_size]) +
                  decode_sRGB_float(q[pixel_size + page_size]) +
                  decode_sRGB_float(q[row_size + page_size]) +
                  decode_sRGB_float(q[pixel_size + row_size + page_size]));

  *p = encode_sRGB_uchar(result * 0.125f);
  ++p;
  ++q;
}

/**
 * Averages a 2x2x2 block of pixel components into a single pixel component,
 * for producing the next mipmap level.  Increments p and q to the next
 * component.
 */
void Texture::
filter_3d_unsigned_byte_srgb_sse2(unsigned char *&p, const unsigned char *&q,
                                  size_t pixel_size, size_t row_size, size_t page_size) {
  float result = (decode_sRGB_float(q[0]) +
                  decode_sRGB_float(q[pixel_size]) +
                  decode_sRGB_float(q[row_size]) +
                  decode_sRGB_float(q[pixel_size + row_size]) +
                  decode_sRGB_float(q[page_size]) +
                  decode_sRGB_float(q[pixel_size + page_size]) +
                  decode_sRGB_float(q[row_size + page_size]) +
                  decode_sRGB_float(q[pixel_size + row_size + page_size]));

  *p = encode_sRGB_uchar_sse2(result * 0.125f);
  ++p;
  ++q;
}

/**
 * Averages a 2x2x2 block of pixel components into a single pixel component,
 * for producing the next mipmap level.  Increments p and q to the next
 * component.
 */
void Texture::
filter_3d_unsigned_short(unsigned char *&p, const unsigned char *&q,
                         size_t pixel_size, size_t row_size,
                         size_t page_size) {
  unsigned int result = ((unsigned int)*(unsigned short *)&q[0] +
                         (unsigned int)*(unsigned short *)&q[pixel_size] +
                         (unsigned int)*(unsigned short *)&q[row_size] +
                         (unsigned int)*(unsigned short *)&q[pixel_size + row_size] +
                         (unsigned int)*(unsigned short *)&q[page_size] +
                         (unsigned int)*(unsigned short *)&q[pixel_size + page_size] +
                         (unsigned int)*(unsigned short *)&q[row_size + page_size] +
                         (unsigned int)*(unsigned short *)&q[pixel_size + row_size + page_size]) >> 3;
  store_unscaled_short(p, result);
  q += 2;
}

/**
 * Averages a 2x2x2 block of pixel components into a single pixel component,
 * for producing the next mipmap level.  Increments p and q to the next
 * component.
 */
void Texture::
filter_3d_float(unsigned char *&p, const unsigned char *&q,
                size_t pixel_size, size_t row_size, size_t page_size) {
  *(float *)p = (*(float *)&q[0] +
                 *(float *)&q[pixel_size] +
                 *(float *)&q[row_size] +
                 *(float *)&q[pixel_size + row_size] +
                 *(float *)&q[page_size] +
                 *(float *)&q[pixel_size + page_size] +
                 *(float *)&q[row_size + page_size] +
                 *(float *)&q[pixel_size + row_size + page_size]) / 8.0f;
  p += 4;
  q += 4;
}

/**
 * Invokes the squish library to compress the RAM image(s).
 */
bool Texture::
do_squish(CData *cdata, Texture::CompressionMode compression, int squish_flags) {
#ifdef HAVE_SQUISH
  if (!do_has_all_ram_mipmap_images(cdata)) {
    // If we're about to compress the RAM image, we should ensure that we have
    // all of the mipmap levels first.
    do_generate_ram_mipmap_images(cdata, false);
  }

  RamImages compressed_ram_images;
  compressed_ram_images.reserve(cdata->_ram_images.size());
  for (size_t n = 0; n < cdata->_ram_images.size(); ++n) {
    RamImage compressed_image;
    int x_size = do_get_expected_mipmap_x_size(cdata, n);
    int y_size = do_get_expected_mipmap_y_size(cdata, n);
    int num_pages = do_get_expected_mipmap_num_pages(cdata, n);
    int page_size = squish::GetStorageRequirements(x_size, y_size, squish_flags);
    int cell_size = squish::GetStorageRequirements(4, 4, squish_flags);

    compressed_image._page_size = page_size;
    compressed_image._image = PTA_uchar::empty_array(page_size * num_pages);
    for (int z = 0; z < num_pages; ++z) {
      unsigned char *dest_page = compressed_image._image.p() + z * page_size;
      unsigned const char *source_page = cdata->_ram_images[n]._image.p() + z * cdata->_ram_images[n]._page_size;
      unsigned const char *source_page_end = source_page + cdata->_ram_images[n]._page_size;
      // Convert one 4 x 4 cell at a time.
      unsigned char *d = dest_page;
      for (int y = 0; y < y_size; y += 4) {
        for (int x = 0; x < x_size; x += 4) {
          unsigned char tb[16 * 4];
          int mask = 0;
          unsigned char *t = tb;
          for (int i = 0; i < 16; ++i) {
            int xi = x + i % 4;
            int yi = y + i / 4;
            unsigned const char *s = source_page + (yi * x_size + xi) * cdata->_num_components;
            if (s < source_page_end) {
              switch (cdata->_num_components) {
              case 1:
                t[0] = s[0];   // r
                t[1] = s[0];   // g
                t[2] = s[0];   // b
                t[3] = 255;    // a
                break;

              case 2:
                t[0] = s[0];   // r
                t[1] = s[0];   // g
                t[2] = s[0];   // b
                t[3] = s[1];   // a
                break;

              case 3:
                t[0] = s[2];   // r
                t[1] = s[1];   // g
                t[2] = s[0];   // b
                t[3] = 255;    // a
                break;

              case 4:
                t[0] = s[2];   // r
                t[1] = s[1];   // g
                t[2] = s[0];   // b
                t[3] = s[3];   // a
                break;
              }
              mask |= (1 << i);
            }
            t += 4;
          }
          squish::CompressMasked(tb, mask, d, squish_flags);
          d += cell_size;
          Thread::consider_yield();
        }
      }
    }
    compressed_ram_images.push_back(compressed_image);
  }
  cdata->_ram_images.swap(compressed_ram_images);
  cdata->_ram_image_compression = compression;
  return true;

#else  // HAVE_SQUISH
  return false;

#endif  // HAVE_SQUISH
}

/**
 * Invokes the squish library to uncompress the RAM image(s).
 */
bool Texture::
do_unsquish(CData *cdata, int squish_flags) {
#ifdef HAVE_SQUISH
  RamImages uncompressed_ram_images;
  uncompressed_ram_images.reserve(cdata->_ram_images.size());
  for (size_t n = 0; n < cdata->_ram_images.size(); ++n) {
    RamImage uncompressed_image;
    int x_size = do_get_expected_mipmap_x_size(cdata, n);
    int y_size = do_get_expected_mipmap_y_size(cdata, n);
    int num_pages = do_get_expected_mipmap_num_pages(cdata, n);
    int page_size = squish::GetStorageRequirements(x_size, y_size, squish_flags);
    int cell_size = squish::GetStorageRequirements(4, 4, squish_flags);

    uncompressed_image._page_size = do_get_expected_ram_mipmap_page_size(cdata, n);
    uncompressed_image._image = PTA_uchar::empty_array(uncompressed_image._page_size * num_pages);
    for (int z = 0; z < num_pages; ++z) {
      unsigned char *dest_page = uncompressed_image._image.p() + z * uncompressed_image._page_size;
      unsigned char *dest_page_end = dest_page + uncompressed_image._page_size;
      unsigned const char *source_page = cdata->_ram_images[n]._image.p() + z * page_size;
      // Unconvert one 4 x 4 cell at a time.
      unsigned const char *s = source_page;
      for (int y = 0; y < y_size; y += 4) {
        for (int x = 0; x < x_size; x += 4) {
          unsigned char tb[16 * 4];
          squish::Decompress(tb, s, squish_flags);
          s += cell_size;

          unsigned char *t = tb;
          for (int i = 0; i < 16; ++i) {
            int xi = x + i % 4;
            int yi = y + i / 4;
            unsigned char *d = dest_page + (yi * x_size + xi) * cdata->_num_components;
            if (d < dest_page_end) {
              switch (cdata->_num_components) {
              case 1:
                d[0] = t[1];   // g
                break;

              case 2:
                d[0] = t[1];   // g
                d[1] = t[3];   // a
                break;

              case 3:
                d[2] = t[0];   // r
                d[1] = t[1];   // g
                d[0] = t[2];   // b
                break;

              case 4:
                d[2] = t[0];   // r
                d[1] = t[1];   // g
                d[0] = t[2];   // b
                d[3] = t[3];   // a
                break;
              }
            }
            t += 4;
          }
        }
        Thread::consider_yield();
      }
    }
    uncompressed_ram_images.push_back(uncompressed_image);
  }
  cdata->_ram_images.swap(uncompressed_ram_images);
  cdata->_ram_image_compression = CM_off;
  return true;

#else  // HAVE_SQUISH
  return false;

#endif  // HAVE_SQUISH
}

/**
 * Factory method to generate a Texture object
 */
void Texture::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void Texture::
write_datagram(BamWriter *manager, Datagram &me) {
  CDWriter cdata(_cycler, false);

  bool has_rawdata = false;
  do_write_datagram_header(cdata, manager, me, has_rawdata);
  do_write_datagram_body(cdata, manager, me);

  // If we are also including the texture's image data, then stuff it in here.
  if (has_rawdata) {
    do_write_datagram_rawdata(cdata, manager, me);
  }
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void Texture::
finalize(BamReader *) {
  // Unref the pointer that we explicitly reffed in make_from_bam().
  unref();

  // We should never get back to zero after unreffing our own count, because
  // we expect to have been stored in a pointer somewhere.  If we do get to
  // zero, it's a memory leak; the way to avoid this is to call unref_delete()
  // above instead of unref(), but this is dangerous to do from within a
  // virtual function.
  nassertv(get_ref_count() != 0);
}


/**
 * Writes the header part of the texture to the Datagram.  This is the common
 * part that is shared by all Texture subclasses, and contains the filename
 * and rawdata flags.  This method is not virtual because all Texture
 * subclasses must write the same data at this step.
 *
 * This part must be read first before calling do_fillin_body() to determine
 * whether to load the Texture from the TexturePool or directly from the bam
 * stream.
 *
 * After this call, has_rawdata will be filled with either true or false,
 * according to whether we expect to write the texture rawdata to the bam
 * stream following the texture body.
 */
void Texture::
do_write_datagram_header(CData *cdata, BamWriter *manager, Datagram &me, bool &has_rawdata) {
  // Write out the texture's raw pixel data if (a) the current Bam Texture
  // Mode requires that, or (b) there's no filename, so the file can't be
  // loaded up from disk, but the raw pixel data is currently available in
  // RAM.

  // Otherwise, we just write out the filename, and assume whoever loads the
  // bam file later will have access to the image file on disk.
  BamWriter::BamTextureMode file_texture_mode = manager->get_file_texture_mode();
  has_rawdata = (file_texture_mode == BamWriter::BTM_rawdata ||
                 (cdata->_filename.empty() && do_has_bam_rawdata(cdata)));
  if (has_rawdata && !do_has_bam_rawdata(cdata)) {
    do_get_bam_rawdata(cdata);
    if (!do_has_bam_rawdata(cdata)) {
      // No image data after all.
      has_rawdata = false;
    }
  }

  bool has_bam_dir = !manager->get_filename().empty();
  Filename bam_dir = manager->get_filename().get_dirname();
  Filename filename = cdata->_filename;
  Filename alpha_filename = cdata->_alpha_filename;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  switch (file_texture_mode) {
  case BamWriter::BTM_unchanged:
  case BamWriter::BTM_rawdata:
    break;

  case BamWriter::BTM_fullpath:
    filename = cdata->_fullpath;
    alpha_filename = cdata->_alpha_fullpath;
    break;

  case BamWriter::BTM_relative:
    filename = cdata->_fullpath;
    alpha_filename = cdata->_alpha_fullpath;
    bam_dir.make_absolute(vfs->get_cwd());
    if (!has_bam_dir || !filename.make_relative_to(bam_dir, true)) {
      filename.find_on_searchpath(get_model_path());
    }
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Texture file " << cdata->_fullpath
        << " found as " << filename << "\n";
    }
    if (!has_bam_dir || !alpha_filename.make_relative_to(bam_dir, true)) {
      alpha_filename.find_on_searchpath(get_model_path());
    }
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Alpha image " << cdata->_alpha_fullpath
        << " found as " << alpha_filename << "\n";
    }
    break;

  case BamWriter::BTM_basename:
    filename = cdata->_fullpath.get_basename();
    alpha_filename = cdata->_alpha_fullpath.get_basename();
    break;

  default:
    gobj_cat.error()
      << "Unsupported bam-texture-mode: " << (int)file_texture_mode << "\n";
  }

  if (filename.empty() && do_has_bam_rawdata(cdata)) {
    // If we don't have a filename, we have to store rawdata anyway.
    has_rawdata = true;
  }

  me.add_string(get_name());
  me.add_string(filename);
  me.add_string(alpha_filename);
  me.add_uint8(cdata->_primary_file_num_channels);
  me.add_uint8(cdata->_alpha_file_channel);
  me.add_bool(has_rawdata);

  if (manager->get_file_minor_ver() < 25 &&
      cdata->_texture_type == TT_cube_map) {
    // Between Panda3D releases 1.7.2 and 1.8.0 (bam versions 6.24 and 6.25),
    // we added TT_2d_texture_array, shifting the definition for TT_cube_map.
    me.add_uint8(TT_2d_texture_array);
  } else {
    me.add_uint8(cdata->_texture_type);
  }

  if (manager->get_file_minor_ver() >= 32) {
    me.add_bool(cdata->_has_read_mipmaps);
  }
}

/**
 * Writes the body part of the texture to the Datagram.  This is generally all
 * of the texture parameters except for the header and the rawdata.
 */
void Texture::
do_write_datagram_body(CData *cdata, BamWriter *manager, Datagram &me) {
  if (manager->get_file_minor_ver() >= 36) {
    cdata->_default_sampler.write_datagram(me);
  } else {
    const SamplerState &s = cdata->_default_sampler;
    me.add_uint8(s.get_wrap_u());
    me.add_uint8(s.get_wrap_v());
    me.add_uint8(s.get_wrap_w());
    me.add_uint8(s.get_minfilter());
    me.add_uint8(s.get_magfilter());
    me.add_int16(s.get_anisotropic_degree());
    s.get_border_color().write_datagram(me);
  }

  me.add_uint8(cdata->_compression);
  me.add_uint8(cdata->_quality_level);

  me.add_uint8(cdata->_format);
  me.add_uint8(cdata->_num_components);

  if (cdata->_texture_type == TT_buffer_texture) {
    me.add_uint8(cdata->_usage_hint);
  }

  if (manager->get_file_minor_ver() >= 28) {
    me.add_uint8(cdata->_auto_texture_scale);
  }
  me.add_uint32(cdata->_orig_file_x_size);
  me.add_uint32(cdata->_orig_file_y_size);

  bool has_simple_ram_image = !cdata->_simple_ram_image._image.empty();
  me.add_bool(has_simple_ram_image);

  // Write out the simple image too, so it will be available later.
  if (has_simple_ram_image) {
    me.add_uint32(cdata->_simple_x_size);
    me.add_uint32(cdata->_simple_y_size);
    me.add_int32(cdata->_simple_image_date_generated);
    me.add_uint32(cdata->_simple_ram_image._image.size());
    me.append_data(cdata->_simple_ram_image._image, cdata->_simple_ram_image._image.size());
  }
}

/**
 * Writes the rawdata part of the texture to the Datagram.
 */
void Texture::
do_write_datagram_rawdata(CData *cdata, BamWriter *manager, Datagram &me) {
  me.add_uint32(cdata->_x_size);
  me.add_uint32(cdata->_y_size);
  me.add_uint32(cdata->_z_size);

  if (manager->get_file_minor_ver() >= 30) {
    me.add_uint32(cdata->_pad_x_size);
    me.add_uint32(cdata->_pad_y_size);
    me.add_uint32(cdata->_pad_z_size);
  }

  if (manager->get_file_minor_ver() >= 26) {
    me.add_uint32(cdata->_num_views);
  }
  me.add_uint8(cdata->_component_type);
  me.add_uint8(cdata->_component_width);
  me.add_uint8(cdata->_ram_image_compression);
  me.add_uint8(cdata->_ram_images.size());
  for (size_t n = 0; n < cdata->_ram_images.size(); ++n) {
    me.add_uint32(cdata->_ram_images[n]._page_size);
    me.add_uint32(cdata->_ram_images[n]._image.size());
    me.append_data(cdata->_ram_images[n]._image, cdata->_ram_images[n]._image.size());
  }
}

/**
 * Factory method to generate a Texture object
 */
TypedWritable *Texture::
make_from_bam(const FactoryParams &params) {
  PT(Texture) dummy = new Texture;
  return dummy->make_this_from_bam(params);
}

/**
 * Called by make_from_bam() once the particular subclass of Texture is known.
 * This is called on a newly-constructed Texture object of the appropriate
 * subclass.  It will return either the same Texture object (e.g.  this), or a
 * different Texture object loaded via the TexturePool, as appropriate.
 */
TypedWritable *Texture::
make_this_from_bam(const FactoryParams &params) {
  // The process of making a texture is slightly different than making other
  // TypedWritable objects.  That is because all creation of Textures should
  // be done through calls to TexturePool, which ensures that any loads of the
  // same filename refer to the same memory.

  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  // Get the header information--the filenames and texture type--so we can
  // look up the file on disk first.
  string name = scan.get_string();
  Filename filename = scan.get_string();
  Filename alpha_filename = scan.get_string();

  int primary_file_num_channels = scan.get_uint8();
  int alpha_file_channel = scan.get_uint8();
  bool has_rawdata = scan.get_bool();
  TextureType texture_type = (TextureType)scan.get_uint8();
  if (manager->get_file_minor_ver() < 25) {
    // Between Panda3D releases 1.7.2 and 1.8.0 (bam versions 6.24 and 6.25),
    // we added TT_2d_texture_array, shifting the definition for TT_cube_map.
    if (texture_type == TT_2d_texture_array) {
      texture_type = TT_cube_map;
    }
  }
  bool has_read_mipmaps = false;
  if (manager->get_file_minor_ver() >= 32) {
    has_read_mipmaps = scan.get_bool();
  }

  Texture *me = nullptr;
  if (has_rawdata) {
    // If the raw image data is included, then just load the texture directly
    // from the stream, and return it.  In this case we return the "this"
    // pointer, since it's a newly-created Texture object of the appropriate
    // type.
    me = this;
    me->set_name(name);
    CDWriter cdata_me(me->_cycler, true);
    cdata_me->_filename = filename;
    cdata_me->_alpha_filename = alpha_filename;
    cdata_me->_primary_file_num_channels = primary_file_num_channels;
    cdata_me->_alpha_file_channel = alpha_file_channel;
    cdata_me->_texture_type = texture_type;
    cdata_me->_has_read_mipmaps = has_read_mipmaps;

    // Read the texture attributes directly from the bam stream.
    me->do_fillin_body(cdata_me, scan, manager);
    me->do_fillin_rawdata(cdata_me, scan, manager);

    // To manage the reference count, explicitly ref it now, then unref it in
    // the finalize callback.
    me->ref();
    manager->register_finalize(me);

  } else {
    // The raw image data isn't included, so we'll be loading the Texture via
    // the TexturePool.  In this case we use the "this" pointer as a temporary
    // object to read all of the attributes from the bam stream.
    Texture *dummy = this;
    AutoTextureScale auto_texture_scale = ATS_unspecified;
    {
      CDWriter cdata_dummy(dummy->_cycler, true);
      dummy->do_fillin_body(cdata_dummy, scan, manager);
      auto_texture_scale = cdata_dummy->_auto_texture_scale;
    }

    if (filename.empty()) {
      // This texture has no filename; since we don't have an image to load,
      // we can't actually create the texture.
      gobj_cat.info()
        << "Cannot create texture '" << name << "' with no filename.\n";

    } else {
      // This texture does have a filename, so try to load it from disk.
      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
      if (!manager->get_filename().empty()) {
        // If texture filename was given relative to the bam filename, expand
        // it now.
        Filename bam_dir = manager->get_filename().get_dirname();
        vfs->resolve_filename(filename, bam_dir);
        if (!alpha_filename.empty()) {
          vfs->resolve_filename(alpha_filename, bam_dir);
        }
      }

      LoaderOptions options = manager->get_loader_options();
      if (dummy->uses_mipmaps()) {
        options.set_texture_flags(options.get_texture_flags() | LoaderOptions::TF_generate_mipmaps);
      }
      options.set_auto_texture_scale(auto_texture_scale);

      switch (texture_type) {
      case TT_buffer_texture:
      case TT_1d_texture:
      case TT_2d_texture:
      case TT_1d_texture_array:
        if (alpha_filename.empty()) {
          me = TexturePool::load_texture(filename, primary_file_num_channels,
                                         has_read_mipmaps, options);
        } else {
          me = TexturePool::load_texture(filename, alpha_filename,
                                         primary_file_num_channels,
                                         alpha_file_channel,
                                         has_read_mipmaps, options);
        }
        break;

      case TT_3d_texture:
        me = TexturePool::load_3d_texture(filename, has_read_mipmaps, options);
        break;

      case TT_2d_texture_array:
      case TT_cube_map_array:
        me = TexturePool::load_2d_texture_array(filename, has_read_mipmaps, options);
        break;

      case TT_cube_map:
        me = TexturePool::load_cube_map(filename, has_read_mipmaps, options);
        break;
      }
    }

    if (me != nullptr) {
      me->set_name(name);
      CDWriter cdata_me(me->_cycler, true);
      me->do_fillin_from(cdata_me, dummy);

      // Since in this case me was loaded from the TexturePool, there's no
      // need to explicitly manage the reference count.  TexturePool will hold
      // it safely.
    }
  }

  return me;
}

/**
 * Reads in the part of the Texture that was written with
 * do_write_datagram_body().
 */
void Texture::
do_fillin_body(CData *cdata, DatagramIterator &scan, BamReader *manager) {
  cdata->_default_sampler.read_datagram(scan, manager);

  if (manager->get_file_minor_ver() >= 1) {
    cdata->_compression = (CompressionMode)scan.get_uint8();
  }
  if (manager->get_file_minor_ver() >= 16) {
    cdata->_quality_level = (QualityLevel)scan.get_uint8();
  }

  cdata->_format = (Format)scan.get_uint8();
  cdata->_num_components = scan.get_uint8();

  if (cdata->_texture_type == TT_buffer_texture) {
    cdata->_usage_hint = (GeomEnums::UsageHint)scan.get_uint8();
  }

  cdata->inc_properties_modified();

  cdata->_auto_texture_scale = ATS_unspecified;
  if (manager->get_file_minor_ver() >= 28) {
    cdata->_auto_texture_scale = (AutoTextureScale)scan.get_uint8();
  }

  bool has_simple_ram_image = false;
  if (manager->get_file_minor_ver() >= 18) {
    cdata->_orig_file_x_size = scan.get_uint32();
    cdata->_orig_file_y_size = scan.get_uint32();

    has_simple_ram_image = scan.get_bool();
  }

  if (has_simple_ram_image) {
    cdata->_simple_x_size = scan.get_uint32();
    cdata->_simple_y_size = scan.get_uint32();
    cdata->_simple_image_date_generated = scan.get_int32();

    size_t u_size = scan.get_uint32();

    // Protect against large allocation.
    if (u_size > scan.get_remaining_size()) {
      gobj_cat.error()
        << "simple RAM image extends past end of datagram, is texture corrupt?\n";
      return;
    }

    PTA_uchar image = PTA_uchar::empty_array(u_size, get_class_type());
    scan.extract_bytes(image.p(), u_size);

    cdata->_simple_ram_image._image = image;
    cdata->_simple_ram_image._page_size = u_size;
    cdata->inc_simple_image_modified();
  }
}

/**
 * Reads in the part of the Texture that was written with
 * do_write_datagram_rawdata().
 */
void Texture::
do_fillin_rawdata(CData *cdata, DatagramIterator &scan, BamReader *manager) {
  cdata->_x_size = scan.get_uint32();
  cdata->_y_size = scan.get_uint32();
  cdata->_z_size = scan.get_uint32();

  if (manager->get_file_minor_ver() >= 30) {
    cdata->_pad_x_size = scan.get_uint32();
    cdata->_pad_y_size = scan.get_uint32();
    cdata->_pad_z_size = scan.get_uint32();
  } else {
    do_set_pad_size(cdata, 0, 0, 0);
  }

  cdata->_num_views = 1;
  if (manager->get_file_minor_ver() >= 26) {
    cdata->_num_views = scan.get_uint32();
  }
  cdata->_component_type = (ComponentType)scan.get_uint8();
  cdata->_component_width = scan.get_uint8();
  cdata->_ram_image_compression = CM_off;
  if (manager->get_file_minor_ver() >= 1) {
    cdata->_ram_image_compression = (CompressionMode)scan.get_uint8();
  }

  int num_ram_images = 1;
  if (manager->get_file_minor_ver() >= 3) {
    num_ram_images = scan.get_uint8();
  }

  cdata->_ram_images.clear();
  cdata->_ram_images.reserve(num_ram_images);
  for (int n = 0; n < num_ram_images; ++n) {
    cdata->_ram_images.push_back(RamImage());
    cdata->_ram_images[n]._page_size = get_expected_ram_page_size();
    if (manager->get_file_minor_ver() >= 1) {
      cdata->_ram_images[n]._page_size = scan.get_uint32();
    }

    // fill the cdata->_image buffer with image data
    size_t u_size = scan.get_uint32();

    // Protect against large allocation.
    if (u_size > scan.get_remaining_size()) {
      gobj_cat.error()
        << "RAM image " << n << " extends past end of datagram, is texture corrupt?\n";
      return;
    }

    PTA_uchar image = PTA_uchar::empty_array(u_size, get_class_type());
    scan.extract_bytes(image.p(), u_size);

    cdata->_ram_images[n]._image = image;
  }
  cdata->_loaded_from_image = true;
  cdata->inc_image_modified();
}

/**
 * Called in make_from_bam(), this method properly copies the attributes from
 * the bam stream (as stored in dummy) into this texture, updating the
 * modified flags appropriately.
 */
void Texture::
do_fillin_from(CData *cdata, const Texture *dummy) {
  // Use the setters instead of setting these directly, so we can correctly
  // avoid incrementing cdata->_properties_modified if none of these actually
  // change.  (Otherwise, we'd have to reload the texture to the GSG every
  // time we loaded a new bam file that reference the texture, since each bam
  // file reference passes through this function.)

  CDReader cdata_dummy(dummy->_cycler);

  do_set_wrap_u(cdata, cdata_dummy->_default_sampler.get_wrap_u());
  do_set_wrap_v(cdata, cdata_dummy->_default_sampler.get_wrap_v());
  do_set_wrap_w(cdata, cdata_dummy->_default_sampler.get_wrap_w());
  do_set_border_color(cdata, cdata_dummy->_default_sampler.get_border_color());

  if (cdata_dummy->_default_sampler.get_minfilter() != SamplerState::FT_default) {
    do_set_minfilter(cdata, cdata_dummy->_default_sampler.get_minfilter());
  }
  if (cdata_dummy->_default_sampler.get_magfilter() != SamplerState::FT_default) {
    do_set_magfilter(cdata, cdata_dummy->_default_sampler.get_magfilter());
  }
  if (cdata_dummy->_default_sampler.get_anisotropic_degree() != 0) {
    do_set_anisotropic_degree(cdata, cdata_dummy->_default_sampler.get_anisotropic_degree());
  }
  if (cdata_dummy->_compression != CM_default) {
    do_set_compression(cdata, cdata_dummy->_compression);
  }
  if (cdata_dummy->_quality_level != QL_default) {
    do_set_quality_level(cdata, cdata_dummy->_quality_level);
  }

  Format format = cdata_dummy->_format;
  int num_components = cdata_dummy->_num_components;

  if (num_components == cdata->_num_components) {
    // Only reset the format if the number of components hasn't changed, since
    // if the number of components has changed our texture no longer matches
    // what it was when the bam was written.
    do_set_format(cdata, format);
  }

  if (!cdata_dummy->_simple_ram_image._image.empty()) {
    // Only replace the simple ram image if it was generated more recently
    // than the one we already have.
    if (cdata->_simple_ram_image._image.empty() ||
        cdata_dummy->_simple_image_date_generated > cdata->_simple_image_date_generated) {
      do_set_simple_ram_image(cdata,
                              cdata_dummy->_simple_ram_image._image,
                              cdata_dummy->_simple_x_size,
                              cdata_dummy->_simple_y_size);
      cdata->_simple_image_date_generated = cdata_dummy->_simple_image_date_generated;
    }
  }
}

/**
 *
 */
Texture::CData::
CData() {
  _primary_file_num_channels = 0;
  _alpha_file_channel = 0;
  _keep_ram_image = true;
  _compression = CM_default;
  _auto_texture_scale = ATS_unspecified;
  _ram_image_compression = CM_off;
  _render_to_texture = false;
  _match_framebuffer_format = false;
  _post_load_store_cache = false;
  _quality_level = QL_default;

  _texture_type = TT_2d_texture;
  _x_size = 0;
  _y_size = 1;
  _z_size = 1;
  _num_views = 1;

  // We will override the format in a moment (in the Texture constructor), but
  // set it to something else first to avoid the check in do_set_format
  // depending on an uninitialized value.
  _format = F_rgba;

  // Only used for buffer textures.
  _usage_hint = GeomEnums::UH_unspecified;

  _pad_x_size = 0;
  _pad_y_size = 0;
  _pad_z_size = 0;

  _orig_file_x_size = 0;
  _orig_file_y_size = 0;

  _loaded_from_image = false;
  _loaded_from_txo = false;
  _has_read_pages = false;
  _has_read_mipmaps = false;
  _num_mipmap_levels_read = 0;

  _simple_x_size = 0;
  _simple_y_size = 0;
  _simple_ram_image._page_size = 0;

  _has_clear_color = false;
}

/**
 *
 */
Texture::CData::
CData(const Texture::CData &copy) {
  _num_mipmap_levels_read = 0;

  do_assign(&copy);

  _properties_modified = copy._properties_modified;
  _image_modified = copy._image_modified;
  _simple_image_modified = copy._simple_image_modified;
}

/**
 *
 */
CycleData *Texture::CData::
make_copy() const {
  return new CData(*this);
}

/**
 *
 */
void Texture::CData::
do_assign(const Texture::CData *copy) {
  _filename = copy->_filename;
  _alpha_filename = copy->_alpha_filename;
  if (!copy->_fullpath.empty()) {
    // Since the fullpath is often empty on a file loaded directly from a txo,
    // we only assign the fullpath if it is not empty.
    _fullpath = copy->_fullpath;
    _alpha_fullpath = copy->_alpha_fullpath;
  }
  _primary_file_num_channels = copy->_primary_file_num_channels;
  _alpha_file_channel = copy->_alpha_file_channel;
  _x_size = copy->_x_size;
  _y_size = copy->_y_size;
  _z_size = copy->_z_size;
  _num_views = copy->_num_views;
  _pad_x_size = copy->_pad_x_size;
  _pad_y_size = copy->_pad_y_size;
  _pad_z_size = copy->_pad_z_size;
  _orig_file_x_size = copy->_orig_file_x_size;
  _orig_file_y_size = copy->_orig_file_y_size;
  _num_components = copy->_num_components;
  _component_width = copy->_component_width;
  _texture_type = copy->_texture_type;
  _format = copy->_format;
  _component_type = copy->_component_type;
  _loaded_from_image = copy->_loaded_from_image;
  _loaded_from_txo = copy->_loaded_from_txo;
  _has_read_pages = copy->_has_read_pages;
  _has_read_mipmaps = copy->_has_read_mipmaps;
  _num_mipmap_levels_read = copy->_num_mipmap_levels_read;
  _default_sampler = copy->_default_sampler;
  _keep_ram_image = copy->_keep_ram_image;
  _compression = copy->_compression;
  _match_framebuffer_format = copy->_match_framebuffer_format;
  _quality_level = copy->_quality_level;
  _auto_texture_scale = copy->_auto_texture_scale;
  _ram_image_compression = copy->_ram_image_compression;
  _ram_images = copy->_ram_images;
  _simple_x_size = copy->_simple_x_size;
  _simple_y_size = copy->_simple_y_size;
  _simple_ram_image = copy->_simple_ram_image;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Texture::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int Texture::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  return 0;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Geom.
 */
void Texture::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}

/**
 *
 */
ostream &
operator << (ostream &out, Texture::TextureType tt) {
  return out << Texture::format_texture_type(tt);
}

/**
 *
 */
ostream &
operator << (ostream &out, Texture::ComponentType ct) {
  return out << Texture::format_component_type(ct);
}

/**
 *
 */
ostream &
operator << (ostream &out, Texture::Format f) {
  return out << Texture::format_format(f);
}

/**
 *
 */
ostream &
operator << (ostream &out, Texture::CompressionMode cm) {
  return out << Texture::format_compression_mode(cm);
}

/**
 *
 */
ostream &
operator << (ostream &out, Texture::QualityLevel tql) {
  return out << Texture::format_quality_level(tql);
}

/**
 *
 */
istream &
operator >> (istream &in, Texture::QualityLevel &tql) {
  string word;
  in >> word;

  tql = Texture::string_quality_level(word);
  return in;
}

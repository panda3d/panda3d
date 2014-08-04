/* Filename: pandasymbols.h
 * Created by:  drose (18Feb00)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PANDASYMBOLS_H
#define PANDASYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

/* Note that the symbols declared in this file appear in alphabetical
   order.  Also note that we must use C-style comments only here, not
   C++-style comments, since this file is occasionally included by a C
   file. */

#if (defined(WIN32_VC) || defined(WIN64_VC)) && !defined(CPPPARSER) && !defined(LINK_ALL_STATIC)

#ifdef BUILDING_CFTALK
  #define EXPCL_CFTALK __declspec(dllexport)
  #define EXPTP_CFTALK
#else
  #define EXPCL_CFTALK __declspec(dllimport)
  #define EXPTP_CFTALK extern
#endif

#ifdef BUILDING_COLLADA
  #define EXPCL_COLLADA __declspec(dllexport)
  #define EXPTP_COLLADA
#else
  #define EXPCL_COLLADA __declspec(dllimport)
  #define EXPTP_COLLADA extern
#endif

#ifdef BUILDING_FFMPEG
  #define EXPCL_FFMPEG __declspec(dllexport)
  #define EXPTP_FFMPEG
#else
  #define EXPCL_FFMPEG __declspec(dllimport)
  #define EXPTP_FFMPEG extern
#endif

#ifdef BUILDING_FRAMEWORK
  #define EXPCL_FRAMEWORK __declspec(dllexport)
  #define EXPTP_FRAMEWORK
#else
  #define EXPCL_FRAMEWORK __declspec(dllimport)
  #define EXPTP_FRAMEWORK extern
#endif

#ifdef BUILDING_LINUX_AUDIO
  #define EXPCL_LINUX_AUDIO __declspec(dllexport)
  #define EXPTP_LINUX_AUDIO
#else
  #define EXPCL_LINUX_AUDIO __declspec(dllimport)
  #define EXPTP_LINUX_AUDIO extern
#endif

#ifdef BUILDING_MILES_AUDIO
  #define EXPCL_MILES_AUDIO __declspec(dllexport)
  #define EXPTP_MILES_AUDIO
#else
  #define EXPCL_MILES_AUDIO __declspec(dllimport)
  #define EXPTP_MILES_AUDIO extern
#endif

#ifdef BUILDING_FMOD_AUDIO
  #define EXPCL_FMOD_AUDIO __declspec(dllexport)
  #define EXPTP_FMOD_AUDIO
#else
  #define EXPCL_FMOD_AUDIO __declspec(dllimport)
  #define EXPTP_FMOD_AUDIO extern
#endif

#ifdef BUILDING_OCULUSVR
  #define EXPCL_OCULUSVR __declspec(dllexport)
  #define EXPTP_OCULUSVR
#else
  #define EXPCL_OCULUSVR __declspec(dllimport)
  #define EXPTP_OCULUSVR extern
#endif

#ifdef BUILDING_OPENAL_AUDIO
  #define EXPCL_OPENAL_AUDIO __declspec(dllexport)
  #define EXPTP_OPENAL_AUDIO
#else
  #define EXPCL_OPENAL_AUDIO __declspec(dllimport)
  #define EXPTP_OPENAL_AUDIO extern
#endif

#ifdef BUILDING_PANDA
  #define EXPCL_PANDA __declspec(dllexport)
  #define EXPTP_PANDA
#else
  #define EXPCL_PANDA __declspec(dllimport)
  #define EXPTP_PANDA extern
#endif

#ifdef BUILDING_PANDAAWESOMIUM
  #define EXPCL_PANDAAWESOMIUM __declspec(dllexport)
  #define EXPTP_PANDAAWESOMIUM
#else
  #define EXPCL_PANDAAWESOMIUM __declspec(dllimport)
  #define EXPTP_PANDAAWESOMIUM extern
#endif

#ifdef BUILDING_PANDABULLET
  #define EXPCL_PANDABULLET __declspec(dllexport)
  #define EXPTP_PANDABULLET
#else
  #define EXPCL_PANDABULLET __declspec(dllimport)
  #define EXPTP_PANDABULLET extern
#endif

#ifdef BUILDING_PANDACR
  #define EXPCL_PANDACR __declspec(dllexport)
  #define EXPTP_PANDACR
#else
  #define EXPCL_PANDACR __declspec(dllimport)
  #define EXPTP_PANDACR extern
#endif

#ifdef BUILDING_PANDADX
  #define EXPCL_PANDADX __declspec(dllexport)
  #define EXPTP_PANDADX
#else
  #define EXPCL_PANDADX __declspec(dllimport)
  #define EXPTP_PANDADX extern
#endif

#ifdef BUILDING_PANDAEGG
  #define EXPCL_PANDAEGG __declspec(dllexport)
  #define EXPTP_PANDAEGG
#else
  #define EXPCL_PANDAEGG __declspec(dllimport)
  #define EXPTP_PANDAEGG extern
#endif

#ifdef BUILDING_PANDAEXPRESS
  #define EXPCL_PANDAEXPRESS __declspec(dllexport)
  #define EXPTP_PANDAEXPRESS
#else
  #define EXPCL_PANDAEXPRESS __declspec(dllimport)
  #define EXPTP_PANDAEXPRESS extern
#endif

#ifdef BUILDING_PANDAFX
  #define EXPCL_PANDAFX __declspec(dllexport)
  #define EXPTP_PANDAFX
#else
  #define EXPCL_PANDAFX __declspec(dllimport)
  #define EXPTP_PANDAFX extern
#endif

#ifdef BUILDING_PANDAGL
  #define EXPCL_PANDAGL __declspec(dllexport)
  #define EXPTP_PANDAGL
#else
  #define EXPCL_PANDAGL __declspec(dllimport)
  #define EXPTP_PANDAGL extern
#endif

#ifdef BUILDING_PANDAGLES
  #define EXPCL_PANDAGLES __declspec(dllexport)
  #define EXPTP_PANDAGLES
#else
  #define EXPCL_PANDAGLES __declspec(dllimport)
  #define EXPTP_PANDAGLES extern
#endif

#ifdef BUILDING_PANDAGLES2
  #define EXPCL_PANDAGLES2 __declspec(dllexport)
  #define EXPTP_PANDAGLES2
#else
  #define EXPCL_PANDAGLES2 __declspec(dllimport)
  #define EXPTP_PANDAGLES2 extern
#endif

#ifdef BUILDING_PANDAODE
  #define EXPCL_PANDAODE __declspec(dllexport)
  #define EXPTP_PANDAODE
#else
  #define EXPCL_PANDAODE __declspec(dllimport)
  #define EXPTP_PANDAODE extern
#endif

#ifdef BUILDING_PANDAPHYSICS
  #define EXPCL_PANDAPHYSICS __declspec(dllexport)
  #define EXPTP_PANDAPHYSICS
#else
  #define EXPCL_PANDAPHYSICS __declspec(dllimport)
  #define EXPTP_PANDAPHYSICS extern
#endif

#ifdef BUILDING_PANDAPHYSX
  #define EXPCL_PANDAPHYSX __declspec(dllexport)
  #define EXPTP_PANDAPHYSX
#else
  #define EXPCL_PANDAPHYSX __declspec(dllimport)
  #define EXPTP_PANDAPHYSX extern
#endif

#ifdef BUILDING_PANDASPEEDTREE
  #define EXPCL_PANDASPEEDTREE __declspec(dllexport)
  #define EXPTP_PANDASPEEDTREE
#else
  #define EXPCL_PANDASPEEDTREE __declspec(dllimport)
  #define EXPTP_PANDASPEEDTREE extern
#endif

#ifdef BUILDING_PANDASKEL
  #define EXPCL_PANDASKEL __declspec(dllexport)
  #define EXPTP_PANDASKEL
#else
  #define EXPCL_PANDASKEL __declspec(dllimport)
  #define EXPTP_PANDASKEL extern
#endif

#ifdef BUILDING_PANDAWIN
  #define EXPCL_PANDAWIN __declspec(dllexport)
  #define EXPTP_PANDAWIN
#else
  #define EXPCL_PANDAWIN __declspec(dllimport)
  #define EXPTP_PANDAWIN extern
#endif

#ifdef BUILDING_PANDAX11
  #define EXPCL_PANDAX11 __declspec(dllexport)
  #define EXPTP_PANDAX11
#else
  #define EXPCL_PANDAX11 __declspec(dllimport)
  #define EXPTP_PANDAX11 extern
#endif

#ifdef BUILDING_ROCKET
  #define EXPCL_ROCKET __declspec(dllexport)
  #define EXPTP_ROCKET
#else
  #define EXPCL_ROCKET __declspec(dllimport)
  #define EXPTP_ROCKET extern
#endif

#ifdef BUILDING_SHADER
  #define EXPCL_SHADER __declspec(dllexport)
  #define EXPTP_SHADER
#else
  #define EXPCL_SHADER __declspec(dllimport)
  #define EXPTP_SHADER extern
#endif

#ifdef BUILDING_TINYDISPLAY
  #define EXPCL_TINYDISPLAY __declspec(dllexport)
  #define EXPTP_TINYDISPLAY
#else
  #define EXPCL_TINYDISPLAY __declspec(dllimport)
  #define EXPTP_TINYDISPLAY extern
#endif

#ifdef BUILDING_VISION
  #define EXPCL_VISION __declspec(dllexport)
  #define EXPTP_VISION
#else
  #define EXPCL_VISION __declspec(dllimport)
  #define EXPTP_VISION extern
#endif

#ifdef BUILDING_VRPN
  #define EXPCL_VRPN __declspec(dllexport)
  #define EXPTP_VRPN
#else
  #define EXPCL_VRPN __declspec(dllimport)
  #define EXPTP_VRPN extern
#endif

#else   /* !WIN32_VC */

#define EXPCL_CFTALK
#define EXPTP_CFTALK

#define EXPCL_COLLADA
#define EXPTP_COLLADA

#define EXPCL_FFMPEG
#define EXPTP_FFMPEG

#define EXPCL_FRAMEWORK
#define EXPTP_FRAMEWORK

#define EXPCL_LINUX_AUDIO
#define EXPTP_LINUX_AUDIO

#define EXPCL_MILES_AUDIO
#define EXPTP_MILES_AUDIO

#define EXPCL_FMOD_AUDIO
#define EXPTP_FMOD_AUDIO

#define EXPCL_OCULUSVR
#define EXPTP_OCULUSVR

#define EXPCL_OPENAL_AUDIO
#define EXPTP_OPENAL_AUDIO

#define EXPCL_PANDA
#define EXPTP_PANDA

#define EXPCL_PANDAAWESOMIUM
#define EXPTP_PANDAAWESOMIUM

#define EXPCL_PANDABULLET
#define EXPTP_PANDABULLET

#define EXPCL_PANDACR
#define EXPTP_PANDACR

#define EXPCL_PANDADX
#define EXPTP_PANDADX

#define EXPCL_PANDAEGG
#define EXPTP_PANDAEGG

#define EXPCL_PANDAEXPRESS
#define EXPTP_PANDAEXPRESS

#define EXPCL_PANDAFX
#define EXPTP_PANDAFX

#define EXPCL_PANDAGL
#define EXPTP_PANDAGL

#define EXPCL_PANDAGLES
#define EXPTP_PANDAGLES

#define EXPCL_PANDAGLES2
#define EXPTP_PANDAGLES2

#define EXPCL_PANDAODE
#define EXPTP_PANDAODE

#define EXPCL_PANDAPHYSICS
#define EXPTP_PANDAPHYSICS

#define EXPCL_PANDAPHYSX
#define EXPTP_PANDAPHYSX

#define EXPCL_PANDASPEEDTREE
#define EXPTP_PANDASPEEDTREE

#define EXPCL_PANDARIB
#define EXPTP_PANDARIB

#define EXPCL_PANDASKEL
#define EXPTP_PANDASKEL

#define EXPCL_PANDAWIN
#define EXPTP_PANDAWIN

#define EXPCL_PANDAX11
#define EXPTP_PANDAX11

#define EXPCL_ROCKET
#define EXPTP_ROCKET

#define EXPCL_SHADER
#define EXPTP_SHADER

#define EXPCL_TINYDISPLAY
#define EXPTP_TINYDISPLAY

#define EXPCL_VISION
#define EXPTP_VISION

#define EXPCL_VRPN
#define EXPTP_VRPN

#endif  /* WIN32_VC */

#if (defined(WIN32_VC) || defined(WIN64_VC)) && !defined(CPPPARSER)
#define INLINE_LINMATH __forceinline
#define INLINE_MATHUTIL __forceinline

#ifdef BUILDING_PANDA
#define INLINE_GRAPH __forceinline
#define INLINE_DISPLAY __forceinline
#else
#define INLINE_GRAPH
#define DONT_INLINE_GRAPH
#define INLINE_DISPLAY
#define DONT_INLINE_DISPLAY
#endif

#else
#define INLINE_LINMATH INLINE
#define INLINE_MATHUTIL INLINE
#define INLINE_GRAPH INLINE
#define INLINE_DISPLAY INLINE
#endif

#define INLINE_CHAR INLINE
#define INLINE_CHAT INLINE
#define INLINE_CHAN INLINE
#define INLINE_CHANCFG INLINE
#define INLINE_COLLIDE INLINE
#define INLINE_CULL INLINE
#define INLINE_DEVICE INLINE
#define INLINE_DGRAPH INLINE
#define INLINE_GOBJ INLINE
#define INLINE_GRUTIL INLINE
#define INLINE_GSGBASE INLINE
#define INLINE_GSGMISC INLINE
#define INLINE_LIGHT INLINE
#define INLINE_PARAMETRICS INLINE
#define INLINE_SGRATTRIB INLINE
#define INLINE_SGMANIP INLINE
#define INLINE_SGRAPH INLINE
#define INLINE_SGRAPHUTIL INLINE
#define INLINE_SWITCHNODE INLINE
#define INLINE_TEXT INLINE
#define INLINE_TFORM INLINE
#define INLINE_LERP INLINE
#define INLINE_LOADER INLINE
#define INLINE_PUTIL INLINE
#define INLINE_EFFECTS INLINE
#define INLINE_GUI INLINE
#define INLINE_AUDIO INLINE

#endif


#if defined(DIRECTORY_DLLS)

#else

#define EXPCL_PANDA_PGRAPH EXPCL_PANDA
#define EXPTP_PANDA_PGRAPH EXPTP_PANDA

#define EXPCL_PANDA_PGRAPHNODES EXPCL_PANDA
#define EXPTP_PANDA_PGRAPHNODES EXPTP_PANDA

#define EXPCL_PANDA_RECORDER EXPCL_PANDA
#define EXPTP_PANDA_RECORDER EXPTP_PANDA

#define EXPCL_PANDA_PIPELINE EXPCL_PANDA
#define EXPTP_PANDA_PIPELINE EXPTP_PANDA

#define EXPCL_PANDA_GRUTIL EXPCL_PANDA
#define EXPTP_PANDA_GRUTIL EXPTP_PANDA

#define EXPCL_PANDA_CHAN EXPCL_PANDA
#define EXPTP_PANDA_CHAN EXPTP_PANDA

#define EXPCL_PANDA_CHAR EXPCL_PANDA
#define EXPTP_PANDA_CHAR EXPTP_PANDA

#define EXPCL_PANDA_PSTATCLIENT EXPCL_PANDA
#define EXPTP_PANDA_PSTATCLIENT EXPTP_PANDA

#define EXPCL_PANDA_COLLIDE EXPCL_PANDA
#define EXPTP_PANDA_COLLIDE EXPTP_PANDA

#define EXPCL_PANDA_CULL EXPCL_PANDA
#define EXPTP_PANDA_CULL EXPTP_PANDA

#define EXPCL_PANDA_DEVICE EXPCL_PANDA
#define EXPTP_PANDA_DEVICE EXPTP_PANDA

#define EXPCL_PANDA_DGRAPH EXPCL_PANDA
#define EXPTP_PANDA_DGRAPH EXPTP_PANDA

#define EXPCL_PANDA_DISPLAY EXPCL_PANDA
#define EXPTP_PANDA_DISPLAY EXPTP_PANDA

#define EXPCL_PANDA_EVENT EXPCL_PANDA
#define EXPTP_PANDA_EVENT EXPTP_PANDA

#define EXPCL_PANDA_GOBJ EXPCL_PANDA
#define EXPTP_PANDA_GOBJ EXPTP_PANDA

#define EXPCL_PANDA_GSGBASE EXPCL_PANDA
#define EXPTP_PANDA_GSGBASE EXPTP_PANDA

#define EXPCL_PANDA_LINMATH EXPCL_PANDA
#define EXPTP_PANDA_LINMATH EXPTP_PANDA

#define EXPCL_PANDA_MATHUTIL EXPCL_PANDA
#define EXPTP_PANDA_MATHUTIL EXPTP_PANDA

#define EXPCL_PANDA_MOVIES EXPCL_PANDA
#define EXPTP_PANDA_MOVIES EXPTP_PANDA

#define EXPCL_PANDA_NET EXPCL_PANDA
#define EXPTP_PANDA_NET EXPTP_PANDA

#define EXPCL_PANDA_NATIVENET EXPCL_PANDA
#define EXPTP_PANDA_NATIVENET EXPTP_PANDA

#define EXPCL_PANDA_PARAMETRICS EXPCL_PANDA
#define EXPTP_PANDA_PARAMETRICS EXPTP_PANDA

#define EXPCL_PANDA_PNMIMAGETYPES EXPCL_PANDA
#define EXPTP_PANDA_PNMIMAGETYPES EXPTP_PANDA

#define EXPCL_PANDA_PNMIMAGE EXPCL_PANDA
#define EXPTP_PANDA_PNMIMAGE EXPTP_PANDA

#define EXPCL_PANDA_PNMTEXT EXPCL_PANDA
#define EXPTP_PANDA_PNMTEXT EXPTP_PANDA

#define EXPCL_PANDA_TEXT EXPCL_PANDA
#define EXPTP_PANDA_TEXT EXPTP_PANDA

#define EXPCL_PANDA_TFORM EXPCL_PANDA
#define EXPTP_PANDA_TFORM EXPTP_PANDA

#define EXPCL_PANDA_LERP EXPCL_PANDA
#define EXPTP_PANDA_LERP EXPTP_PANDA

#define EXPCL_PANDA_PUTIL EXPCL_PANDA
#define EXPTP_PANDA_PUTIL EXPTP_PANDA

#define EXPCL_PANDA_AUDIO EXPCL_PANDA
#define EXPTP_PANDA_AUDIO EXPTP_PANDA

#define EXPCL_PANDA_PGUI EXPCL_PANDA
#define EXPTP_PANDA_PGUI EXPTP_PANDA

#define EXPCL_PANDA_PANDABASE EXPCL_PANDA
#define EXPTP_PANDA_PANDABASE EXPTP_PANDA

#define EXPCL_PANDA_HELIX EXPCL_PANDA
#define EXPTP_PANDA_HELIX EXPTP_PANDA

#endif

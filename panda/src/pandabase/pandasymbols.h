/*
// Filename: pandasymbols.h
// Created by:  drose (18Feb00)
// 
////////////////////////////////////////////////////////////////////
*/

#ifndef PANDASYMBOLS_H
#define PANDASYMBOLS_H

/* See dtoolsymbols.h for a rant on the purpose of this file.  */

#if defined(PENV_WIN32) && !defined(CPPPARSER)

#ifdef BUILDING_PANDA
  #define EXPCL_PANDA __declspec(dllexport)
  #define EXPTP_PANDA
#else
  #define EXPCL_PANDA __declspec(dllimport)
  #define EXPTP_PANDA extern
#endif

#ifdef BUILDING_PANDAEXPRESS
  #define EXPCL_PANDAEXPRESS __declspec(dllexport)
  #define EXPTP_PANDAEXPRESS
#else
  #define EXPCL_PANDAEXPRESS __declspec(dllimport)
  #define EXPTP_PANDAEXPRESS extern
#endif

#ifdef BUILDING_PANDAEGG
  #define EXPCL_PANDAEGG __declspec(dllexport)
  #define EXPTP_PANDAEGG
#else
  #define EXPCL_PANDAEGG __declspec(dllimport)
  #define EXPTP_PANDAEGG extern
#endif

#ifdef BUILDING_PANDAPHYSICS
  #define EXPCL_PANDAPHYSICS __declspec(dllexport)
  #define EXPTP_PANDAPHYSICS
#else
  #define EXPCL_PANDAPHYSICS __declspec(dllimport)
  #define EXPTP_PANDAPHYSICS extern
#endif

#ifdef BUILDING_PANDAGL
  #define EXPCL_PANDAGL __declspec(dllexport)
  #define EXPTP_PANDAGL
#else
  #define EXPCL_PANDAGL __declspec(dllimport)
  #define EXPTP_PANDAGL extern
#endif

#ifdef BUILDING_PANDAGLUT
  #define EXPCL_PANDAGLUT __declspec(dllexport)
  #define EXPTP_PANDAGLUT
#else
  #define EXPCL_PANDAGLUT __declspec(dllimport)
  #define EXPTP_PANDAGLUT extern
#endif

#ifdef BUILDING_PANDADX
  #define EXPCL_PANDADX __declspec(dllexport)
  #define EXPTP_PANDADX
#else
  #define EXPCL_PANDADX __declspec(dllimport)
  #define EXPTP_PANDADX extern
#endif

#ifdef BUILDING_PANDARIB
  #define EXPCL_PANDARIB __declspec(dllexport)
  #define EXPTP_PANDARIB
#else
  #define EXPCL_PANDARIB __declspec(dllimport)
  #define EXPTP_PANDARIB extern
#endif

#ifdef BUILDING_SHADER
  #define EXPCL_SHADER __declspec(dllexport)
  #define EXPTP_SHADER
#else
  #define EXPCL_SHADER __declspec(dllimport)
  #define EXPTP_SHADER extern
#endif

#else   /* !PENV_WIN32 */

#define EXPCL_PANDA
#define EXPTP_PANDA

#define EXPCL_PANDAEXPRESS
#define EXPTP_PANDAEXPRESS

#define EXPCL_PANDAEGG
#define EXPTP_PANDAEGG

#define EXPCL_PANDAPHYSICS
#define EXPTP_PANDAPHYSICS

#define EXPCL_PANDAGL
#define EXPTP_PANDAGL

#define EXPCL_PANDAGLUT
#define EXPTP_PANDAGLUT

#define EXPCL_PANDADX
#define EXPTP_PANDADX

#define EXPCL_PANDARIB
#define EXPTP_PANDARIB

#define EXPCL_SHADER
#define EXPTP_SHADER

#define EXPCL_FRAMEWORK
#define EXPTP_FRAMEWORK

#endif  /* PENV_WIN32 */

#endif

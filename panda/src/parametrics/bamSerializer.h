/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamSerializer.h
 * @author TidalGator
 * @date 2026-03-10
 */

#ifndef BAMSERIALIZER_H
#define BAMSERIALIZER_H

#include "nurbsSurfaceEvaluator.h"
#include "datagramIterator.h"
#include "datagram.h"
#include "typedWritable.h"

/**
 * A helper class for the serialization/deserialization of NurbsSurfaceEvaluator.
 */
struct NurbsSurfaceEvaluatorBamSerializer
{
  static void write_datagram(const NurbsSurfaceEvaluator &evaluator, Datagram &datagram);
  static void fillin(NurbsSurfaceEvaluator &evaluator, DatagramIterator &di);
  static void complete_pointers(NurbsSurfaceEvaluator &evaluator);
  static TypedWritable *make_from_bam(DatagramIterator &di);
};

#endif
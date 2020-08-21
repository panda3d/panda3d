/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggConverter.h
 * @author drose
 * @date 2000-02-15
 */

#ifndef EGGCONVERTER_H
#define EGGCONVERTER_H

#include "pandatoolbase.h"

#include "eggFilter.h"

/**
 * This is a general base class for programs that convert between egg files
 * and some other format.  See EggToSomething and SomethingToEgg.
 */
class EggConverter : public EggFilter {
public:
  EggConverter(const std::string &format_name,
               const std::string &preferred_extension = std::string(),
               bool allow_last_param = true,
               bool allow_stdout = true);

protected:
  std::string _format_name;
};

#endif

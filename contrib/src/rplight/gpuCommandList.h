/**
 *
 * RenderPipeline
 *
 * Copyright (c) 2014-2016 tobspr <tobias.springer1@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef GPUCOMMANDLIST_H
#define GPUCOMMANDLIST_H

#include "pandabase.h"
#include "gpuCommand.h"

#include <queue>

/**
 * @brief Class to store a list of commands.
 * @details This is a class to store a list of GPUCommands. It provides
 *   functionality to only provide the a given amount of commands at one time.
 */
class GPUCommandList {
PUBLISHED:
  GPUCommandList();

  void add_command(const GPUCommand& cmd);
  size_t get_num_commands();
  size_t write_commands_to(const PTA_uchar &dest, size_t limit = 32);

  MAKE_PROPERTY(num_commands, get_num_commands);

protected:
  std::queue<GPUCommand> _commands;
};

#endif // GPUCOMMANDLIST_H

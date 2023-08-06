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

#include "gpuCommandList.h"


/**
 * @brief Constructs a new GPUCommandList
 * @details This constructs a new GPUCommandList. By default, there are no commands
 *   in the list.
 */
GPUCommandList::GPUCommandList() {
}

/**
 * @brief Pushes a GPUCommand to the command list.
 * @details This adds a new GPUCommand to the list of commands to be processed.
 *
 * @param cmd The command to add
 */
void GPUCommandList::add_command(const GPUCommand& cmd) {
  _commands.push(cmd);
}

/**
 * @brief Returns the number of commands in this list.
 * @details This returns the amount of commands which are currently stored in this
 *   list, and are waiting to get processed.
 * @return Amount of commands
 */
size_t GPUCommandList::get_num_commands() const {
  return _commands.size();
}

/**
 * @brief Writes the first n-commands to a destination.
 * @details This takes the first #limit commands, and writes them to the
 *   destination using GPUCommand::write_to. See GPUCommand::write_to for
 *   further information about #dest. The limit controls after how much
 *   commands the processing will be stopped. All commands which got processed
 *   will get removed from the list.
 *
 * @param dest Destination to write to, see GPUCommand::write_to
 * @param limit Maximum amount of commands to process
 *
 * @return Amount of commands processed, between 0 and #limit.
 */
size_t GPUCommandList::write_commands_to(const PTA_uchar &dest, size_t limit) {
  size_t num_commands_written = 0;

  while (num_commands_written < limit && !_commands.empty()) {
    // Write the first command to the stream, and delete it afterwards
    _commands.front().write_to(dest, num_commands_written);
    _commands.pop();
    num_commands_written ++;
  }

  return num_commands_written;
}

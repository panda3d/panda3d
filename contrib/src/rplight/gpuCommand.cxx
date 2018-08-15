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

#include "gpuCommand.h"

#include <iostream>
#include <iomanip>
#include <stdlib.h>


NotifyCategoryDef(gpucommand, "");

/**
 * @brief Constructs a new GPUCommand with the given command type.
 * @details This will construct a new GPUCommand of the given command type.
 *   The command type should be of GPUCommand::CommandType, and determines
 *   what data the GPUCommand contains, and how it will be handled.
 *
 * @param command_type The type of the GPUCommand
 */
GPUCommand::GPUCommand(CommandType command_type) {
  _command_type = command_type;
  _current_index = 0;
  memset(_data, 0x0, sizeof(float) * GPU_COMMAND_ENTRIES);

  // Store the command type as the first entry
  push_int(command_type);
}

/**
 * @brief Prints out the GPUCommand to the console
 * @details This method prints the type, size, and data of the GPUCommand to the
 *   console. This helps for debugging the contents of the GPUCommand. Keep
 *   in mind that integers might be shown in their binary float representation,
 *   depending on the setting in the GPUCommand::convert_int_to_float method.
 */
void GPUCommand::write(std::ostream &out) const {
  out << "GPUCommand(type=" << _command_type << ", size=" << _current_index << ", data = {" << std::endl;
  for (size_t k = 0; k < GPU_COMMAND_ENTRIES; ++k) {
    out << std::setw(12) << std::fixed << std::setprecision(5) << _data[k] << " ";
    if (k % 6 == 5 || k == GPU_COMMAND_ENTRIES - 1) out << std::endl;
  }
  out << "})" << std::endl;
}

/**
 * @brief Writes the GPU command to a given target.
 * @details This method writes all the data of the GPU command to a given target.
 *   The target should be a pointer to memory being big enough to hold the
 *   data. Presumably #dest will be a handle to texture memory.
 *   The command_index controls the offset where the data will be written
 *   to.
 *
 * @param dest Handle to the memory to write the command to
 * @param command_index Offset to write the command to. The command will write
 *   its data to command_index * GPU_COMMAND_ENTRIES. When writing
 *   the GPUCommand in a GPUCommandList, the command_index will
 *   most likely be the index of the command in the list.
 */
void GPUCommand::write_to(const PTA_uchar &dest, size_t command_index) {
  size_t command_size = GPU_COMMAND_ENTRIES * sizeof(float);
  size_t offset = command_index * command_size;
  memcpy(dest.p() + offset, &_data, command_size);
}

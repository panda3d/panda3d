/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleSpirV.I
 * @author rdb
 * @date 2020-06-01
 */

/**
 * Returns a pointer to the raw words.
 */
const uint32_t *ShaderModuleSpirV::
get_data() const {
  return _instructions.get_data();
}

/**
 * Returns the number of words in the module.
 */
size_t ShaderModuleSpirV::
get_data_size() const {
  return _instructions.get_data_size();
}

/**
 * Iterator dereference operator.
 */
INLINE ShaderModuleSpirV::Instruction ShaderModuleSpirV::InstructionIterator::
operator *() {
  return Instruction {
    (SpvOp)(_words[0] & SpvOpCodeMask),
    (_words[0] >> SpvWordCountShift) - 1,
    _words + 1,
  };
}

/**
 * Advances to the next instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator &ShaderModuleSpirV::InstructionIterator::
operator ++() {
  uint16_t wcount = _words[0] >> SpvWordCountShift;
  _words += wcount;
  nassertr(wcount > 0, *this);
  return *this;
}

/**
 * Returns true if these iterators point to the same instruction.
 */
INLINE bool ShaderModuleSpirV::InstructionIterator::
operator ==(const InstructionIterator &other) const {
  return _words == other._words;
}

/**
 * Returns false if these iterators point to the same instruction.
 */
INLINE bool ShaderModuleSpirV::InstructionIterator::
operator !=(const InstructionIterator &other) const {
  return _words != other._words;
}

/**
 * Constructs an iterator from the given word pointer.
 */
INLINE ShaderModuleSpirV::InstructionIterator::
InstructionIterator(uint32_t *words) : _words(words) {
}

/**
 * Initializes the instruction stream from an existing module.
 */
INLINE ShaderModuleSpirV::InstructionStream::
InstructionStream(const uint32_t *words, size_t num_words) :
  _words(words, words + num_words) {
}

/**
 * Initializes the instruction stream from an existing module.
 */
INLINE ShaderModuleSpirV::InstructionStream::
InstructionStream(std::vector<uint32_t> words) :
  _words(std::move(words)) {
}

/**
 * Direct cast to the underlying vector.
 */
INLINE ShaderModuleSpirV::InstructionStream::
operator std::vector<uint32_t> & () {
  return _words;
}

/**
 * Returns an iterator to the beginning of the instruction stream.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
begin() {
  return iterator(&_words[5]);
}

/**
 * Returns an iterator to the beginning of the annotations block.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
begin_annotations() {
  iterator it;
  for (it = begin(); it != end(); ++it) {
    SpvOp opcode = (*it).opcode;
    if (opcode != SpvOpNop &&
        opcode != SpvOpCapability &&
        opcode != SpvOpExtension &&
        opcode != SpvOpExtInstImport &&
        opcode != SpvOpMemoryModel &&
        opcode != SpvOpEntryPoint &&
        opcode != SpvOpExecutionMode &&
        opcode != SpvOpString &&
        opcode != SpvOpSourceExtension &&
        opcode != SpvOpSource &&
        opcode != SpvOpSourceContinued &&
        opcode != SpvOpName &&
        opcode != SpvOpMemberName &&
        opcode != SpvOpModuleProcessed) {
      break;
    }
  }
  return it;
}

/**
 * Returns an iterator past the end of the instruction stream.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
end() {
  return iterator(&_words[0] + _words.size());
}

/**
 * Inserts a new instruction before the given other one.  Invalidates existing
 * iterators, but returns a new one pointing to the location of the inserted
 * instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
insert(iterator &it, SpvOp opcode, std::initializer_list<uint32_t > args) {
  return insert(it, opcode, args.begin(), args.size());
}

/**
 * Inserts a new instruction before the given other one.  Invalidates existing
 * iterators, but returns a new one pointing to the location of the inserted
 * instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
insert(iterator &it, SpvOp opcode, const uint32_t *args, uint16_t nargs) {
  ptrdiff_t offset = it._words - &_words[0];

  // If this triggers, you used an invalid iterator.
  assert(offset >= 0 && offset <= _words.size());

  _words.insert(_words.begin() + offset, ((nargs + 1) << SpvWordCountShift) | opcode);
  _words.insert(_words.begin() + offset + 1, args, args + nargs);

  return iterator(&_words[offset]);
}

/**
 * Erases the given instruction.  Invalidates iterators at or after the
 * insertion point, but returns a new iterator pointing to the next element.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
erase(iterator &it) {
  ptrdiff_t offset = it._words - &_words[0];

  // If this triggers, you used an invalid iterator.
  assert(offset >= 0 && offset <= _words.size());

  uint16_t wcount = _words[offset] >> SpvWordCountShift;
  _words.erase(_words.begin() + offset, _words.begin() + offset + wcount);

  return iterator(&_words[offset]);
}

/**
 * Erases the given argument of the given instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
erase_arg(iterator &it, uint16_t arg) {
  ptrdiff_t offset = it._words - &_words[0];

  // If this triggers, you used an invalid iterator.
  assert(offset >= 0 && offset <= _words.size());

  uint16_t wcount = _words[offset] >> SpvWordCountShift;
  nassertr(arg < wcount - 1, it);

  SpvOp op = (SpvOp)(_words[offset] & SpvOpCodeMask);
  _words[offset] = op | ((wcount - 1) << SpvWordCountShift);
  _words.erase(_words.begin() + offset + 1 + arg);

  return iterator(&_words[offset]);
}

/**
 * Returns a pointer to the raw words of the instruction stream.
 */
const uint32_t *ShaderModuleSpirV::InstructionStream::
get_data() const {
  return &_words[0];
}

/**
 * Returns the number of words in the instruction stream.
 */
size_t ShaderModuleSpirV::InstructionStream::
get_data_size() const {
  return _words.size();
}

/**
 * Returns the number of ids allocated.
 */
INLINE uint32_t ShaderModuleSpirV::InstructionStream::
get_id_bound() const {
  return _words[3];
}

/**
 * Allocates a new identifier.
 */
INLINE uint32_t ShaderModuleSpirV::InstructionStream::
allocate_id() {
  return _words[3]++;
}

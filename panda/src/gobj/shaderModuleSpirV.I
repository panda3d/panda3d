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
 */
INLINE ShaderModuleSpirV::
ShaderModuleSpirV(Stage stage) :
  ShaderModule(stage)
{
}

/**
 * Returns a pointer to the raw words.
 */
INLINE const uint32_t *ShaderModuleSpirV::
get_data() const {
  return _instructions.get_data();
}

/**
 * Returns the number of words in the module.
 */
INLINE size_t ShaderModuleSpirV::
get_data_size() const {
  return _instructions.get_data_size();
}

/**
 * Returns true if this is a debug instruction.
 */
INLINE bool ShaderModuleSpirV::Instruction::
is_debug() const {
  return
    opcode == spv::OpSourceContinued ||
    opcode == spv::OpSource ||
    opcode == spv::OpSourceExtension ||
    opcode == spv::OpName ||
    opcode == spv::OpMemberName ||
    opcode == spv::OpString ||
    opcode == spv::OpLine ||
    opcode == spv::OpNoLine ||
    opcode == spv::OpModuleProcessed;
}

/**
 * Returns true if this is an annotation instruction.
 */
INLINE bool ShaderModuleSpirV::Instruction::
is_annotation() const {
  return
    opcode == spv::OpDecorate ||
    opcode == spv::OpMemberDecorate ||
    opcode == spv::OpGroupDecorate ||
    opcode == spv::OpGroupMemberDecorate ||
    opcode == spv::OpDecorationGroup;
}

/**
 * Iterator dereference operator.
 */
INLINE ShaderModuleSpirV::Instruction ShaderModuleSpirV::InstructionIterator::
operator *() {
  return Instruction {
    (spv::Op)(_words[0] & spv::OpCodeMask),
    (_words[0] >> spv::WordCountShift) - 1,
    _words + 1,
  };
}

/**
 * Advances to the next instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator &ShaderModuleSpirV::InstructionIterator::
operator ++() {
  uint16_t wcount = _words[0] >> spv::WordCountShift;
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
    spv::Op opcode = (*it).opcode;
    if (opcode != spv::OpNop &&
        opcode != spv::OpCapability &&
        opcode != spv::OpExtension &&
        opcode != spv::OpExtInstImport &&
        opcode != spv::OpMemoryModel &&
        opcode != spv::OpEntryPoint &&
        opcode != spv::OpExecutionMode &&
        opcode != spv::OpString &&
        opcode != spv::OpSourceExtension &&
        opcode != spv::OpSource &&
        opcode != spv::OpSourceContinued &&
        opcode != spv::OpName &&
        opcode != spv::OpMemberName &&
        opcode != spv::OpModuleProcessed) {
      break;
    }
  }
  return it;
}

/**
 * Returns an iterator to the end of the annotations block.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
end_annotations() {
  iterator it;
  for (it = begin_annotations(); it != end(); ++it) {
    if (!(*it).is_annotation()) {
      break;
    }
  }
  return it;
}

/**
 * Returns an iterator to the beginning of the functions block.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
begin_functions() {
  iterator it;
  for (it = begin(); it != end(); ++it) {
    spv::Op opcode = (*it).opcode;
    if (opcode == spv::OpFunction) {
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
insert(const iterator &it, spv::Op opcode, std::initializer_list<uint32_t > args) {
  return insert(it, opcode, args.begin(), args.size());
}

/**
 * Inserts a new instruction before the given other one.  Invalidates existing
 * iterators, but returns a new one pointing to the location of the inserted
 * instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
insert(const iterator &it, spv::Op opcode, const uint32_t *args, uint16_t nargs) {
  ptrdiff_t offset = it._words - &_words[0];

  // If this triggers, you used an invalid iterator.
  assert(offset >= 0 && (size_t)offset <= _words.size());

  _words.insert(_words.begin() + offset, ((nargs + 1) << spv::WordCountShift) | opcode);
  _words.insert(_words.begin() + offset + 1, args, args + nargs);

  return iterator(&_words[offset]);
}

/**
 * Inserts a new instruction before the given other one.  Invalidates existing
 * iterators, but returns a new one pointing to the location of the inserted
 * instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
insert(const iterator &it, const Instruction &op) {
  return insert(it, op.opcode, op.args, op.nargs);
}

/**
 * Inserts the given argument of the given instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
insert_arg(const iterator &it, uint16_t arg_index, uint32_t arg) {
  ptrdiff_t offset = it._words - &_words[0];

  // If this triggers, you used an invalid iterator.
  assert(offset >= 0 && (size_t)offset <= _words.size());

  uint16_t wcount = _words[offset] >> spv::WordCountShift;
  nassertr(arg_index < wcount, it);

  spv::Op op = (spv::Op)(_words[offset] & spv::OpCodeMask);
  _words[offset] = op | ((wcount + 1) << spv::WordCountShift);
  _words.insert(_words.begin() + offset + 1 + arg_index, arg);

  return iterator(&_words[offset]);
}

/**
 * Erases the given instruction.  Invalidates iterators at or after the
 * insertion point, but returns a new iterator pointing to the next element.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
erase(const iterator &it) {
  ptrdiff_t offset = it._words - &_words[0];

  // If this triggers, you used an invalid iterator.
  assert(offset >= 0 && (size_t)offset <= _words.size());

  uint16_t wcount = _words[offset] >> spv::WordCountShift;
  _words.erase(_words.begin() + offset, _words.begin() + offset + wcount);

  return iterator(&_words[offset]);
}

/**
 * Erases the given argument of the given instruction.
 */
INLINE ShaderModuleSpirV::InstructionIterator ShaderModuleSpirV::InstructionStream::
erase_arg(const iterator &it, uint16_t arg) {
  ptrdiff_t offset = it._words - &_words[0];

  // If this triggers, you used an invalid iterator.
  assert(offset >= 0 && (size_t)offset <= _words.size());

  uint16_t wcount = _words[offset] >> spv::WordCountShift;
  nassertr(arg < wcount - 1, it);

  spv::Op op = (spv::Op)(_words[offset] & spv::OpCodeMask);
  _words[offset] = op | ((wcount - 1) << spv::WordCountShift);
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

/**
 * For a variable or function parameter, returns true if its value has been
 * loaded or passed into a function call.  For a type or type pointer, returns
 * true if it is the type of at least one variable that is marked "used".  For
 * a function, returns true if it is called at least once.
 */
INLINE bool ShaderModuleSpirV::Definition::
is_used() const {
  return (_flags & DF_used) != 0;
}

/**
 * Returns true if this has the BuiltIn decoration.  See also has_builtin().
 */
INLINE bool ShaderModuleSpirV::Definition::
is_builtin() const {
  return _builtin != spv::BuiltInMax;
}

/**
 * Returns true if this has a Location decoration.
 */
INLINE bool ShaderModuleSpirV::Definition::
has_location() const {
  return _location >= 0;
}

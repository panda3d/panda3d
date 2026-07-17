/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVModule.cxx
 * @author rdb
 * @date 2026-07-07
 */

#include "spirVModule.h"
#include "spirVBuilder.h"

#include "GLSL.std.450.h"

#include <algorithm>

/**
 * Table of per-opcode metadata for all known "simple" opcodes: the minimum
 * operand count and a bitmask indicating which of their operands are ids
 * (the highest bit indicating that all remaining operands are ids).
 *
 * Note that some instructions aren't completely covered by the mask: a few
 * are special-cased in get_instruction_id_operands, and OpEntryPoint has its
 * interface excluded because we ignore it altogether and regenerate it at
 * emit-time.
 *
 * The table is sorted by opcode and dense up to and including OpLifetimeStop,
 * to allow fast indexing for the most commonly used instructions.
 */
struct InstructionInfo {
  uint16_t _opcode;
  uint16_t _min_args;
  uint16_t _id_mask;
};
template<class... Index>
static constexpr uint16_t
bits(Index... indices) {
  return (uint16_t)((0u | ... | (1u << indices)));
}
static constexpr uint16_t
bits_from(unsigned int index) {
  return (uint16_t)(0xffffu << index);
}
#define UNUSED_OPCODE(opcode) {opcode, 0xffff, 0} // sentinel for gaps
static constexpr InstructionInfo instruction_table[] = {
  {spv::OpNop, 0, 0},
  {spv::OpUndef, 2, bits(0)},
  {spv::OpSourceContinued, 1, 0},
  {spv::OpSource, 2, bits(2)},
  {spv::OpSourceExtension, 1, 0},
  {spv::OpName, 2, bits(0)},
  {spv::OpMemberName, 3, bits(0)},
  {spv::OpString, 2, 0},
  {spv::OpLine, 3, bits(0)},
  UNUSED_OPCODE(9),
  {spv::OpExtension, 1, 0},
  {spv::OpExtInstImport, 2, 0},
  {spv::OpExtInst, 4, bits(0, 2) | bits_from(4)},
  UNUSED_OPCODE(13),
  {spv::OpMemoryModel, 2, 0},
  {spv::OpEntryPoint, 3, bits(1)},  // NB. interface is excluded!
  {spv::OpExecutionMode, 2, bits(0)},
  {spv::OpCapability, 1, 0},
  UNUSED_OPCODE(18),
  {spv::OpTypeVoid, 1, 0},
  {spv::OpTypeBool, 1, 0},
  {spv::OpTypeInt, 3, 0},
  {spv::OpTypeFloat, 2, 0},
  {spv::OpTypeVector, 3, bits(1)},
  {spv::OpTypeMatrix, 3, bits(1)},
  {spv::OpTypeImage, 8, bits(1)},
  {spv::OpTypeSampler, 1, 0},
  {spv::OpTypeSampledImage, 2, bits(1)},
  {spv::OpTypeArray, 3, bits(1, 2)},
  {spv::OpTypeRuntimeArray, 2, bits(1)},
  {spv::OpTypeStruct, 1, bits_from(1)},
  {spv::OpTypeOpaque, 2, 0},
  {spv::OpTypePointer, 3, bits(2)},
  {spv::OpTypeFunction, 2, bits_from(1)},
  {spv::OpTypeEvent, 1, 0},
  {spv::OpTypeDeviceEvent, 1, 0},
  {spv::OpTypeReserveId, 1, 0},
  {spv::OpTypeQueue, 1, 0},
  {spv::OpTypePipe, 2, 0},
  {spv::OpTypeForwardPointer, 2, bits(0)},
  UNUSED_OPCODE(40),
  {spv::OpConstantTrue, 2, bits(0)},
  {spv::OpConstantFalse, 2, bits(0)},
  {spv::OpConstant, 3, bits(0)},
  {spv::OpConstantComposite, 2, bits(0) | bits_from(2)},
  {spv::OpConstantSampler, 5, bits(0)},
  {spv::OpConstantNull, 2, bits(0)},
  UNUSED_OPCODE(47),
  {spv::OpSpecConstantTrue, 2, bits(0)},
  {spv::OpSpecConstantFalse, 2, bits(0)},
  {spv::OpSpecConstant, 3, bits(0)},
  {spv::OpSpecConstantComposite, 2, bits(0) | bits_from(2)},
  {spv::OpSpecConstantOp, 3, bits(0)},  // special-cased in get_instruction_id_operands
  UNUSED_OPCODE(53),
  {spv::OpFunction, 4, bits(0, 3)},
  {spv::OpFunctionParameter, 2, bits(0)},
  {spv::OpFunctionEnd, 0, 0},
  {spv::OpFunctionCall, 3, bits(0) | bits_from(2)},
  UNUSED_OPCODE(58),
  {spv::OpVariable, 3, bits(0, 3)},
  {spv::OpImageTexelPointer, 5, bits(0, 2, 3, 4)},
  {spv::OpLoad, 3, bits(0, 2)},  // + trailing MemoryAccess operand(s)
  {spv::OpStore, 2, bits(0, 1)},  // + trailing MemoryAccess operand(s)
  {spv::OpCopyMemory, 2, bits(0, 1)},  // + trailing MemoryAccess operand(s)
  {spv::OpCopyMemorySized, 3, bits(0, 1, 2)},  // + trailing MemoryAccess operand(s)
  {spv::OpAccessChain, 3, bits(0) | bits_from(2)},
  {spv::OpInBoundsAccessChain, 3, bits(0) | bits_from(2)},
  {spv::OpPtrAccessChain, 4, bits(0) | bits_from(2)},
  {spv::OpArrayLength, 4, bits(0, 2)},
  {spv::OpGenericPtrMemSemantics, 3, bits(0, 2)},
  {spv::OpInBoundsPtrAccessChain, 4, bits(0) | bits_from(2)},
  {spv::OpDecorate, 2, bits(0)},
  {spv::OpMemberDecorate, 3, bits(0)},
  {spv::OpDecorationGroup, 1, 0},
  {spv::OpGroupDecorate, 1, bits_from(0)},
  {spv::OpGroupMemberDecorate, 1, bits(0)},  // special-cased in get_instruction_id_operands
  UNUSED_OPCODE(76),
  {spv::OpVectorExtractDynamic, 4, bits(0, 2, 3)},
  {spv::OpVectorInsertDynamic, 5, bits(0, 2, 3, 4)},
  {spv::OpVectorShuffle, 4, bits(0, 2, 3)},
  {spv::OpCompositeConstruct, 2, bits(0) | bits_from(2)},
  {spv::OpCompositeExtract, 3, bits(0, 2)},
  {spv::OpCompositeInsert, 4, bits(0, 2, 3)},
  {spv::OpCopyObject, 3, bits(0, 2)},
  {spv::OpTranspose, 3, bits(0, 2)},
  UNUSED_OPCODE(85),
  {spv::OpSampledImage, 4, bits(0, 2, 3)},
  {spv::OpImageSampleImplicitLod, 4, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSampleExplicitLod, 5, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSampleDrefImplicitLod, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSampleDrefExplicitLod, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSampleProjImplicitLod, 4, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSampleProjExplicitLod, 5, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSampleProjDrefImplicitLod, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSampleProjDrefExplicitLod, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageFetch, 4, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageGather, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageDrefGather, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageRead, 4, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageWrite, 3, bits(0, 1, 2) | bits_from(4)},
  {spv::OpImage, 3, bits(0, 2)},
  {spv::OpImageQueryFormat, 3, bits(0, 2)},
  {spv::OpImageQueryOrder, 3, bits(0, 2)},
  {spv::OpImageQuerySizeLod, 4, bits(0, 2, 3)},
  {spv::OpImageQuerySize, 3, bits(0, 2)},
  {spv::OpImageQueryLod, 4, bits(0, 2, 3)},
  {spv::OpImageQueryLevels, 3, bits(0, 2)},
  {spv::OpImageQuerySamples, 3, bits(0, 2)},
  UNUSED_OPCODE(108),
  {spv::OpConvertFToU, 3, bits(0, 2)},
  {spv::OpConvertFToS, 3, bits(0, 2)},
  {spv::OpConvertSToF, 3, bits(0, 2)},
  {spv::OpConvertUToF, 3, bits(0, 2)},
  {spv::OpUConvert, 3, bits(0, 2)},
  {spv::OpSConvert, 3, bits(0, 2)},
  {spv::OpFConvert, 3, bits(0, 2)},
  {spv::OpQuantizeToF16, 3, bits(0, 2)},
  {spv::OpConvertPtrToU, 3, bits(0, 2)},
  {spv::OpSatConvertSToU, 3, bits(0, 2)},
  {spv::OpSatConvertUToS, 3, bits(0, 2)},
  {spv::OpConvertUToPtr, 3, bits(0, 2)},
  {spv::OpPtrCastToGeneric, 3, bits(0, 2)},
  {spv::OpGenericCastToPtr, 3, bits(0, 2)},
  {spv::OpGenericCastToPtrExplicit, 4, bits(0, 2)},
  {spv::OpBitcast, 3, bits(0, 2)},
  UNUSED_OPCODE(125),
  {spv::OpSNegate, 3, bits(0, 2)},
  {spv::OpFNegate, 3, bits(0, 2)},
  {spv::OpIAdd, 4, bits(0, 2, 3)},
  {spv::OpFAdd, 4, bits(0, 2, 3)},
  {spv::OpISub, 4, bits(0, 2, 3)},
  {spv::OpFSub, 4, bits(0, 2, 3)},
  {spv::OpIMul, 4, bits(0, 2, 3)},
  {spv::OpFMul, 4, bits(0, 2, 3)},
  {spv::OpUDiv, 4, bits(0, 2, 3)},
  {spv::OpSDiv, 4, bits(0, 2, 3)},
  {spv::OpFDiv, 4, bits(0, 2, 3)},
  {spv::OpUMod, 4, bits(0, 2, 3)},
  {spv::OpSRem, 4, bits(0, 2, 3)},
  {spv::OpSMod, 4, bits(0, 2, 3)},
  {spv::OpFRem, 4, bits(0, 2, 3)},
  {spv::OpFMod, 4, bits(0, 2, 3)},
  {spv::OpVectorTimesScalar, 4, bits(0, 2, 3)},
  {spv::OpMatrixTimesScalar, 4, bits(0, 2, 3)},
  {spv::OpVectorTimesMatrix, 4, bits(0, 2, 3)},
  {spv::OpMatrixTimesVector, 4, bits(0, 2, 3)},
  {spv::OpMatrixTimesMatrix, 4, bits(0, 2, 3)},
  {spv::OpOuterProduct, 4, bits(0, 2, 3)},
  {spv::OpDot, 4, bits(0, 2, 3)},
  {spv::OpIAddCarry, 4, bits(0, 2, 3)},
  {spv::OpISubBorrow, 4, bits(0, 2, 3)},
  {spv::OpUMulExtended, 4, bits(0, 2, 3)},
  {spv::OpSMulExtended, 4, bits(0, 2, 3)},
  UNUSED_OPCODE(153),
  {spv::OpAny, 3, bits(0, 2)},
  {spv::OpAll, 3, bits(0, 2)},
  {spv::OpIsNan, 3, bits(0, 2)},
  {spv::OpIsInf, 3, bits(0, 2)},
  {spv::OpIsFinite, 3, bits(0, 2)},
  {spv::OpIsNormal, 3, bits(0, 2)},
  {spv::OpSignBitSet, 3, bits(0, 2)},
  {spv::OpLessOrGreater, 4, bits(0, 2, 3)},
  {spv::OpOrdered, 4, bits(0, 2, 3)},
  {spv::OpUnordered, 4, bits(0, 2, 3)},
  {spv::OpLogicalEqual, 4, bits(0, 2, 3)},
  {spv::OpLogicalNotEqual, 4, bits(0, 2, 3)},
  {spv::OpLogicalOr, 4, bits(0, 2, 3)},
  {spv::OpLogicalAnd, 4, bits(0, 2, 3)},
  {spv::OpLogicalNot, 3, bits(0, 2)},
  {spv::OpSelect, 5, bits(0, 2, 3, 4)},
  {spv::OpIEqual, 4, bits(0, 2, 3)},
  {spv::OpINotEqual, 4, bits(0, 2, 3)},
  {spv::OpUGreaterThan, 4, bits(0, 2, 3)},
  {spv::OpSGreaterThan, 4, bits(0, 2, 3)},
  {spv::OpUGreaterThanEqual, 4, bits(0, 2, 3)},
  {spv::OpSGreaterThanEqual, 4, bits(0, 2, 3)},
  {spv::OpULessThan, 4, bits(0, 2, 3)},
  {spv::OpSLessThan, 4, bits(0, 2, 3)},
  {spv::OpULessThanEqual, 4, bits(0, 2, 3)},
  {spv::OpSLessThanEqual, 4, bits(0, 2, 3)},
  {spv::OpFOrdEqual, 4, bits(0, 2, 3)},
  {spv::OpFUnordEqual, 4, bits(0, 2, 3)},
  {spv::OpFOrdNotEqual, 4, bits(0, 2, 3)},
  {spv::OpFUnordNotEqual, 4, bits(0, 2, 3)},
  {spv::OpFOrdLessThan, 4, bits(0, 2, 3)},
  {spv::OpFUnordLessThan, 4, bits(0, 2, 3)},
  {spv::OpFOrdGreaterThan, 4, bits(0, 2, 3)},
  {spv::OpFUnordGreaterThan, 4, bits(0, 2, 3)},
  {spv::OpFOrdLessThanEqual, 4, bits(0, 2, 3)},
  {spv::OpFUnordLessThanEqual, 4, bits(0, 2, 3)},
  {spv::OpFOrdGreaterThanEqual, 4, bits(0, 2, 3)},
  {spv::OpFUnordGreaterThanEqual, 4, bits(0, 2, 3)},
  UNUSED_OPCODE(192),
  UNUSED_OPCODE(193),
  {spv::OpShiftRightLogical, 4, bits(0, 2, 3)},
  {spv::OpShiftRightArithmetic, 4, bits(0, 2, 3)},
  {spv::OpShiftLeftLogical, 4, bits(0, 2, 3)},
  {spv::OpBitwiseOr, 4, bits(0, 2, 3)},
  {spv::OpBitwiseXor, 4, bits(0, 2, 3)},
  {spv::OpBitwiseAnd, 4, bits(0, 2, 3)},
  {spv::OpNot, 3, bits(0, 2)},
  {spv::OpBitFieldInsert, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpBitFieldSExtract, 5, bits(0, 2, 3, 4)},
  {spv::OpBitFieldUExtract, 5, bits(0, 2, 3, 4)},
  {spv::OpBitReverse, 3, bits(0, 2)},
  {spv::OpBitCount, 3, bits(0, 2)},
  UNUSED_OPCODE(206),
  {spv::OpDPdx, 3, bits(0, 2)},
  {spv::OpDPdy, 3, bits(0, 2)},
  {spv::OpFwidth, 3, bits(0, 2)},
  {spv::OpDPdxFine, 3, bits(0, 2)},
  {spv::OpDPdyFine, 3, bits(0, 2)},
  {spv::OpFwidthFine, 3, bits(0, 2)},
  {spv::OpDPdxCoarse, 3, bits(0, 2)},
  {spv::OpDPdyCoarse, 3, bits(0, 2)},
  {spv::OpFwidthCoarse, 3, bits(0, 2)},
  UNUSED_OPCODE(216),
  UNUSED_OPCODE(217),
  {spv::OpEmitVertex, 0, 0},
  {spv::OpEndPrimitive, 0, 0},
  {spv::OpEmitStreamVertex, 1, bits(0)},
  {spv::OpEndStreamPrimitive, 1, bits(0)},
  UNUSED_OPCODE(222),
  UNUSED_OPCODE(223),
  {spv::OpControlBarrier, 3, bits(0, 1, 2)},
  {spv::OpMemoryBarrier, 2, bits(0, 1)},
  UNUSED_OPCODE(226),
  {spv::OpAtomicLoad, 5, bits(0, 2, 3, 4)},
  {spv::OpAtomicStore, 4, bits(0, 1, 2, 3)},
  {spv::OpAtomicExchange, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicCompareExchange, 8, bits(0, 2, 3, 4, 5, 6, 7)},
  {spv::OpAtomicCompareExchangeWeak, 8, bits(0, 2, 3, 4, 5, 6, 7)},
  {spv::OpAtomicIIncrement, 5, bits(0, 2, 3, 4)},
  {spv::OpAtomicIDecrement, 5, bits(0, 2, 3, 4)},
  {spv::OpAtomicIAdd, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicISub, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicSMin, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicUMin, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicSMax, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicUMax, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicAnd, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicOr, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicXor, 6, bits(0, 2, 3, 4, 5)},
  UNUSED_OPCODE(243),
  UNUSED_OPCODE(244),
  {spv::OpPhi, 2, bits(0) | bits_from(2)},
  {spv::OpLoopMerge, 3, bits(0, 1)},
  {spv::OpSelectionMerge, 2, bits(0)},
  {spv::OpLabel, 1, 0},
  {spv::OpBranch, 1, bits(0)},
  {spv::OpBranchConditional, 3, bits(0, 1, 2)},
  {spv::OpSwitch, 2, bits(0, 1)},  // special-cased in get_instruction_id_operands
  {spv::OpKill, 0, 0},
  {spv::OpReturn, 0, 0},
  {spv::OpReturnValue, 1, bits(0)},
  {spv::OpUnreachable, 0, 0},
  {spv::OpLifetimeStart, 2, bits(0)},
  {spv::OpLifetimeStop, 2, bits(0)},
  // end of dense table section; the rest is sparse
  {spv::OpImageSparseSampleImplicitLod, 4, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSparseSampleExplicitLod, 5, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSparseSampleDrefImplicitLod, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSparseSampleDrefExplicitLod, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSparseSampleProjImplicitLod, 4, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSparseSampleProjExplicitLod, 5, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSparseSampleProjDrefImplicitLod, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSparseSampleProjDrefExplicitLod, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSparseFetch, 4, bits(0, 2, 3) | bits_from(5)},
  {spv::OpImageSparseGather, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSparseDrefGather, 5, bits(0, 2, 3, 4) | bits_from(6)},
  {spv::OpImageSparseTexelsResident, 3, bits(0, 2)},
  {spv::OpNoLine, 0, 0},
  {spv::OpAtomicFlagTestAndSet, 5, bits(0, 2, 3, 4)},
  {spv::OpAtomicFlagClear, 3, bits(0, 1, 2)},
  {spv::OpImageSparseRead, 4, bits(0, 2, 3) | bits_from(5)},
  {spv::OpSizeOf, 3, bits(0, 2)},
  {spv::OpModuleProcessed, 1, 0},
  {spv::OpExecutionModeId, 2, bits(0) | bits_from(2)},
  {spv::OpDecorateId, 2, bits(0) | bits_from(2)},
  {spv::OpGroupNonUniformElect, 3, bits(0, 2)},
  {spv::OpGroupNonUniformAll, 4, bits(0, 2, 3)},
  {spv::OpGroupNonUniformAny, 4, bits(0, 2, 3)},
  {spv::OpGroupNonUniformAllEqual, 4, bits(0, 2, 3)},
  {spv::OpGroupNonUniformBroadcast, 5, bits(0, 2, 3, 4)},
  {spv::OpGroupNonUniformBroadcastFirst, 4, bits(0, 2, 3)},
  {spv::OpGroupNonUniformBallot, 4, bits(0, 2, 3)},
  {spv::OpGroupNonUniformInverseBallot, 4, bits(0, 2, 3)},
  {spv::OpGroupNonUniformBallotBitExtract, 5, bits(0, 2, 3, 4)},
  {spv::OpGroupNonUniformBallotBitCount, 5, bits(0, 2, 4)},
  {spv::OpGroupNonUniformBallotFindLSB, 4, bits(0, 2, 3)},
  {spv::OpGroupNonUniformBallotFindMSB, 4, bits(0, 2, 3)},
  {spv::OpGroupNonUniformShuffle, 5, bits(0, 2, 3, 4)},
  {spv::OpGroupNonUniformShuffleXor, 5, bits(0, 2, 3, 4)},
  {spv::OpGroupNonUniformShuffleUp, 5, bits(0, 2, 3, 4)},
  {spv::OpGroupNonUniformShuffleDown, 5, bits(0, 2, 3, 4)},
  {spv::OpGroupNonUniformIAdd, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformFAdd, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformIMul, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformFMul, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformSMin, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformUMin, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformFMin, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformSMax, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformUMax, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformFMax, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformBitwiseAnd, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformBitwiseOr, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformBitwiseXor, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformLogicalAnd, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformLogicalOr, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformLogicalXor, 5, bits(0, 2, 4, 5)},
  {spv::OpGroupNonUniformQuadBroadcast, 5, bits(0, 2, 3, 4)},
  {spv::OpGroupNonUniformQuadSwap, 5, bits(0, 2, 3, 4)},
  {spv::OpCopyLogical, 3, bits(0, 2)},
  {spv::OpPtrEqual, 4, bits(0, 2, 3)},
  {spv::OpPtrNotEqual, 4, bits(0, 2, 3)},
  {spv::OpPtrDiff, 4, bits(0, 2, 3)},
  {spv::OpColorAttachmentReadEXT, 3, bits(0, 2, 3)},
  {spv::OpDepthAttachmentReadEXT, 2, bits(0, 2)},
  {spv::OpStencilAttachmentReadEXT, 2, bits(0, 2)},
  {spv::OpTerminateInvocation, 0, 0},
  {spv::OpTypeUntypedPointerKHR, 2, 0},
  {spv::OpUntypedVariableKHR, 3, bits(0, 3, 4)},
  {spv::OpUntypedAccessChainKHR, 2, bits(0) | bits_from(2)},
  {spv::OpUntypedInBoundsAccessChainKHR, 2, bits(0) | bits_from(2)},
  {spv::OpSubgroupBallotKHR, 3, bits(0, 2)},
  {spv::OpSubgroupFirstInvocationKHR, 3, bits(0, 2)},
  {spv::OpUntypedPtrAccessChainKHR, 2, bits(0) | bits_from(2)},
  {spv::OpUntypedInBoundsPtrAccessChainKHR, 2, bits(0) | bits_from(2)},
  {spv::OpUntypedArrayLengthKHR, 4, bits(0, 2, 3)},
  {spv::OpSubgroupAllKHR, 3, bits(0, 2)},
  {spv::OpSubgroupAnyKHR, 3, bits(0, 2)},
  {spv::OpSubgroupAllEqualKHR, 3, bits(0, 2)},
  {spv::OpGroupNonUniformRotateKHR, 5, bits(0, 2, 3, 4, 5)},
  {spv::OpSubgroupReadInvocationKHR, 4, bits(0, 2, 3)},
  {spv::OpExtInstWithForwardRefsKHR, 4, bits(0, 2) | bits_from(4)},
  {spv::OpSDot, 4, bits(0, 2, 3)},
  {spv::OpUDot, 4, bits(0, 2, 3)},
  {spv::OpSUDot, 4, bits(0, 2, 3)},
  {spv::OpSDotAccSat, 5, bits(0, 2, 3, 4)},
  {spv::OpUDotAccSat, 5, bits(0, 2, 3, 4)},
  {spv::OpSUDotAccSat, 5, bits(0, 2, 3, 4)},
  {spv::OpTypeCooperativeMatrixKHR, 1, bits_from(1)},
  {spv::OpCooperativeMatrixLoadKHR, 4, bits(0, 2, 3, 4)},  // + trailing MemoryAccess operand(s)
  {spv::OpCooperativeMatrixStoreKHR, 3, bits(0, 1, 2, 3)},  // + trailing MemoryAccess operand(s)
  {spv::OpCooperativeMatrixMulAddKHR, 5, bits(0, 2, 3, 4)},
  {spv::OpCooperativeMatrixLengthKHR, 3, bits(0, 2)},
  {spv::OpConstantCompositeReplicateEXT, 3, bits(0, 2)},
  {spv::OpSpecConstantCompositeReplicateEXT, 3, bits(0, 2)},
  {spv::OpCompositeConstructReplicateEXT, 3, bits(0, 2)},
  {spv::OpEmitMeshTasksEXT, 3, bits(0, 1, 2, 3)},
  {spv::OpSetMeshOutputsEXT, 2, bits(0, 1)},
  {spv::OpWritePackedPrimitiveIndices4x8NV, 2, bits(0, 1)},
  {spv::OpTypeCooperativeMatrixNV, 1, bits_from(1)},
  {spv::OpCooperativeMatrixLoadNV, 5, bits(0, 2, 3, 4)},  // + trailing MemoryAccess operand(s)
  {spv::OpCooperativeMatrixStoreNV, 4, bits(0, 1, 2, 3)},  // + trailing MemoryAccess operand(s)
  {spv::OpCooperativeMatrixMulAddNV, 5, bits(0, 2, 3, 4)},
  {spv::OpCooperativeMatrixLengthNV, 3, bits(0, 2)},
  {spv::OpDemoteToHelperInvocation, 0, 0},
  {spv::OpIsHelperInvocationEXT, 2, bits(0)},
  {spv::OpAtomicFMinEXT, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAtomicFMaxEXT, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpAssumeTrueKHR, 1, bits(0)},
  {spv::OpExpectKHR, 4, bits(0, 2, 3)},
  {spv::OpDecorateString, 2, bits(0)},
  {spv::OpMemberDecorateString, 3, bits(0)},
  {spv::OpAtomicFAddEXT, 6, bits(0, 2, 3, 4, 5)},
  {spv::OpConvertFToBF16INTEL, 3, bits(0, 2)},
  {spv::OpConvertBF16ToFINTEL, 3, bits(0, 2)},
};
#undef UNUSED_OPCODE

// Check at compile-time whether the table is sorted properly.
static constexpr bool
is_sorted_by_opcode(const InstructionInfo *table, size_t count) {
  for (size_t i = 1; i < count; ++i) {
    if (table[i - 1]._opcode >= table[i]._opcode) {
      return false;
    }
  }
  return true;
}

static_assert(is_sorted_by_opcode(instruction_table,
              sizeof(instruction_table) / sizeof(instruction_table[0])),
              "instruction_table must be sorted by opcode");

static_assert(instruction_table[spv::OpLifetimeStop]._opcode == spv::OpLifetimeStop,
              "instruction_table must be dense up to OpLifetimeStop");

/**
 * Returns the instruction_table entry for the given opcode, or null if the
 * opcode is not in the table.
 */
static const InstructionInfo *
find_instruction_info(spv::Op opcode) {
  // For most instructions, the table is dense and indexed by opcode.
  if ((uint16_t)opcode <= spv::OpLifetimeStop) {
    const InstructionInfo *info = &instruction_table[(uint16_t)opcode];
    if (info->_min_args != 0xffff) {
      return info;
    }
    return nullptr;
  }

  // Beyond that, the opcode numbering becomes too sparse, so do a binary
  // search through the rest of the table.
  const InstructionInfo *begin = instruction_table + spv::OpLifetimeStop + 1;
  const InstructionInfo *end = instruction_table +
    sizeof(instruction_table) / sizeof(instruction_table[0]);
  const InstructionInfo *it = std::lower_bound(begin, end, (uint16_t)opcode,
    [](const InstructionInfo &entry, uint16_t opcode) {
      return entry._opcode < opcode;
    });
  if (it != end && it->_opcode == (uint16_t)opcode) {
    return it;
  }
  return nullptr;
}

/**
 * Returns the number of operands that an instruction with the given opcode
 * must at least carry.  For opcodes that aren't in the table above, this falls
 * back to the result type/result prefix reported by spv::HasResultAndType.
 */
static uint32_t
get_min_instruction_args(spv::Op opcode) {
  const InstructionInfo *info = find_instruction_info(opcode);
  if (info != nullptr) {
    return info->_min_args;
  }

  bool has_result, has_result_type;
  spv::HasResultAndType(opcode, &has_result, &has_result_type);
  return (uint32_t)has_result + (uint32_t)has_result_type;
}

/**
 * Returns true if the given decoration feeds into the type resolution, so
 * that adding or removing it must invalidate the cached resolutions.
 */
static bool
is_structural_decoration(spv::Decoration decoration) {
  switch (decoration) {
  case spv::DecorationBlock:
  case spv::DecorationBufferBlock:
  case spv::DecorationBuiltIn:
  case spv::DecorationNonWritable:
  case spv::DecorationNonReadable:
  case spv::DecorationOffset:
  case spv::DecorationArrayStride:
  case spv::DecorationMatrixStride:
  case spv::DecorationRowMajor:
  case spv::DecorationColMajor:
    return true;
  default:
    return false;
  }
}

/**
 * Returns the type that a variable with the given builtin decoration is
 * declared with, or null if the builtin is not known here.
 */
static const ShaderType *
get_builtin_type(spv::BuiltIn builtin) {
  switch (builtin) {
  case spv::BuiltInVertexId:
  case spv::BuiltInInstanceId:
  case spv::BuiltInPrimitiveId:
  case spv::BuiltInInvocationId:
  case spv::BuiltInLayer:
  case spv::BuiltInViewportIndex:
  case spv::BuiltInPatchVertices:
  case spv::BuiltInSampleId:
  case spv::BuiltInVertexIndex:
  case spv::BuiltInInstanceIndex:
    return ShaderType::INT;

  case spv::BuiltInPointSize:
  case spv::BuiltInFragDepth:
  case spv::BuiltInTessLevelOuter:
  case spv::BuiltInTessLevelInner:
    return ShaderType::FLOAT;

  case spv::BuiltInTessCoord:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 3));

  case spv::BuiltInPosition:
  case spv::BuiltInFragCoord:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  case spv::BuiltInPointCoord:
  case spv::BuiltInSamplePosition:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));

  case spv::BuiltInFrontFacing:
  case spv::BuiltInHelperInvocation:
    return ShaderType::BOOL;

  case spv::BuiltInNumWorkgroups:
  case spv::BuiltInWorkgroupSize:
  case spv::BuiltInWorkgroupId:
  case spv::BuiltInLocalInvocationId:
  case spv::BuiltInGlobalInvocationId:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_uint, 3));

  case spv::BuiltInLocalInvocationIndex:
    return ShaderType::UINT;

  default:
    return nullptr;
  }
}

/**
 * Appends a string literal (padded with at least one zero byte) to the given
 * argument vector (a pvector or an instruction's Args).
 */
template<class ArgsVector>
static void
pack_string(ArgsVector &args, std::string_view str) {
  size_t offset = args.size();
  args.resize(offset + str.size() / 4 + 1, 0u);
  memcpy((char *)(args.data() + offset), str.data(), str.size());
}

/**
 * Returns true if this instruction produces a result id.
 */
bool SpirVModule::Instruction::
has_result() const {
  bool has_result, has_type;
  spv::HasResultAndType(opcode, &has_result, &has_type);
  return has_result;
}

/**
 * Returns true if this instruction has a result type operand.
 */
bool SpirVModule::Instruction::
has_result_type() const {
  bool has_result, has_type;
  spv::HasResultAndType(opcode, &has_result, &has_type);
  return has_type;
}

/**
 * Returns the result id of this instruction, or 0 if it has none.
 */
SpirVId SpirVModule::Instruction::
get_result() const {
  bool has_result, has_type;
  spv::HasResultAndType(opcode, &has_result, &has_type);
  if (!has_result || args.size() <= (size_t)has_type) {
    return Id();
  }
  return Id(args[has_type]);
}

/**
 * Returns the result type id of this instruction, or 0 if it has none.
 */
SpirVId SpirVModule::Instruction::
get_result_type() const {
  bool has_result, has_type;
  spv::HasResultAndType(opcode, &has_result, &has_type);
  if (!has_type || args.empty()) {
    return Id();
  }
  return Id(args[0]);
}

/**
 * Extracts a null-terminated string literal starting at the given operand
 * index.  If end_index is not null, it receives the operand index just past
 * the string.
 */
std::string SpirVModule::Instruction::
get_string(size_t arg_index, size_t *end_index) const {
  std::string result;
  size_t i = arg_index;
  while (i < args.size()) {
    uint32_t word = args[i];
    ++i;
    for (int b = 0; b < 4; ++b) {
      char c = (char)((word >> (b * 8)) & 0xff);
      if (c == 0) {
        if (end_index != nullptr) {
          *end_index = i;
        }
        return result;
      }
      result += c;
    }
  }
  if (end_index != nullptr) {
    *end_index = i;
  }
  return result;
}

/**
 * Materializes the module from the given instruction stream.
 */
SpirVModule::
SpirVModule(const InstructionStream &stream) {
  parse(stream.get_data(), stream.get_data_size());
}

/**
 * Parses the given words into this module.  Returns false if the stream is
 * malformed; the module contents are undefined in that case.
 */
bool SpirVModule::
parse(const uint32_t *words, size_t num_words) {
  if (num_words < 5 || words[0] != spv::MagicNumber) {
    shader_cat.error()
      << "Invalid SPIR-V file header\n";
    return false;
  }

  // Reset any state from a previous parse, so that this may safely be called
  // on a module that already holds contents.
  _capabilities.clear();
  _extensions.clear();
  _ext_inst_imports.clear();
  _memory_model = Instruction();
  _entry_points.clear();
  _execution_modes.clear();
  _debug.clear();
  _names.clear();
  _member_names.clear();
  _module_processed.clear();
  _annotations.clear();
  _declarations.clear();
  _functions.clear();
  _defs.clear();
  _type_map.clear();
  _type_map_valid = false;

  _version = words[1];
  _generator = words[2];
  _id_bound = words[3];
  _schema = words[4];

  _defs.resize(_id_bound);

  Function *function = nullptr;

  size_t i = 5;
  while (i < num_words) {
    spv::Op opcode = (spv::Op)(words[i] & spv::OpCodeMask);
    uint32_t wcount = words[i] >> spv::WordCountShift;
    if (wcount == 0 || i + wcount > num_words) {
      shader_cat.error()
        << "Encountered malformed instruction during SPIR-V parse\n";
      return false;
    }
    const uint32_t *args = words + i + 1;
    uint32_t nargs = wcount - 1;
    i += wcount;

    // Every instruction with a result id (and result type) must at least
    // carry those operands; this is what makes record_result and the various
    // places that read op.args[0] of a type declaration safe.
    if (nargs < get_min_instruction_args(opcode)) {
      shader_cat.error()
        << "Truncated instruction (opcode " << (uint32_t)opcode << ", "
        << nargs << " operands) during SPIR-V parse\n";
      return false;
    }

    Instruction op(opcode, args, nargs);

    switch (opcode) {
    case spv::OpNop:
      continue;

    case spv::OpCapability:
      _capabilities.push_back(std::move(op));
      continue;

    case spv::OpExtension:
      _extensions.push_back(std::move(op));
      continue;

    case spv::OpExtInstImport:
      record_result(op);
      _ext_inst_imports.push_back(std::move(op));
      continue;

    case spv::OpMemoryModel:
      _memory_model = std::move(op);
      continue;

    case spv::OpEntryPoint:
      {
        // The trailing interface ids are deliberately not stored; emit()
        // derives the interface from the instructions, so a declared list
        // would only go stale.
        EntryPoint ep;
        ep.model = (spv::ExecutionModel)args[0];
        ep.function_id = Id(args[1]);
        ep.name = op.get_string(2);
        _entry_points.push_back(std::move(ep));
      }
      continue;

    case spv::OpExecutionMode:
    case spv::OpExecutionModeId:
      _execution_modes.push_back(std::move(op));
      continue;

    case spv::OpString:
      record_result(op);
      _debug.push_back(std::move(op));
      continue;

    case spv::OpSource:
    case spv::OpSourceContinued:
    case spv::OpSourceExtension:
      _debug.push_back(std::move(op));
      continue;

    case spv::OpModuleProcessed:
      _module_processed.push_back(std::move(op));
      continue;

    case spv::OpName:
      if (nargs >= 2) {
        std::string name = op.get_string(1);
        if (!name.empty()) {
          _names[args[0]] = std::move(name);
        }
      }
      continue;

    case spv::OpMemberName:
      if (nargs >= 3) {
        std::string name = op.get_string(2);
        if (!name.empty()) {
          pvector<std::string> &names = _member_names[args[0]];
          if (args[1] >= names.size()) {
            names.resize(args[1] + 1);
          }
          names[args[1]] = std::move(name);
        }
      }
      continue;

    case spv::OpLine:
    case spv::OpNoLine:
      // Kept inside function bodies, stripped elsewhere.
      if (function != nullptr) {
        function->instructions.push_back(std::move(op));
      }
      continue;

    case spv::OpDecorate:
    case spv::OpMemberDecorate:
    case spv::OpDecorateId:
    case spv::OpDecorateString:
    case spv::OpMemberDecorateString:
    case spv::OpDecorationGroup:
    case spv::OpGroupDecorate:
    case spv::OpGroupMemberDecorate:
      if (nargs >= 1) {
        Annotation annotation;
        annotation.opcode = opcode;
        annotation.args.assign(op.args.begin() + 1, op.args.end());
        _annotations[args[0]].push_back(std::move(annotation));
      }
      continue;

    case spv::OpFunction:
      if (function != nullptr) {
        shader_cat.error()
          << "OpFunction may not occur within another function!\n";
        return false;
      }
      nassertr(args[0] == get_type_id(Id(args[3])), false);
      record_result(op);
      {
        //NB. args[0] is the return type, not the function type!
        Function function;
        function.id = Id(args[1]);
        function.type_id = Id(args[3]);
        function.control = (spv::FunctionControlMask)args[2];
        _functions.push_back(std::move(function));
      }
      function = &_functions.back();
      continue;

    case spv::OpFunctionParameter:
      if (function == nullptr) {
        shader_cat.error()
          << "OpFunctionParameter may only occur within a function!\n";
        return false;
      }
      for (const Instruction &body_op : function->instructions) {
        if (body_op.opcode != spv::OpLine &&
            body_op.opcode != spv::OpNoLine && !body_op.is_nop()) {
          shader_cat.error()
            << "OpFunctionParameter may not occur in a function body!\n";
          return false;
        }
      }
      record_result(op, function->id);
      function->parameters.push_back(Id(args[1]));
      continue;

    case spv::OpFunctionEnd:
      if (function == nullptr) {
        shader_cat.error()
          << "OpFunctionEnd may only occur within a function!\n";
        return false;
      }
      function = nullptr;
      continue;

    default:
      if (function != nullptr) {
        record_result(op, function->id);
        function->instructions.push_back(std::move(op));
      } else {
        record_result_at(op, Id(), (int32_t)_declarations.size());
        _declarations.push_back(std::move(op));
      }
      continue;
    }
  }

  if (function != nullptr) {
    shader_cat.error()
      << "Encountered end of stream before function end\n";
    return false;
  }
  return true;
}

/**
 * Serializes the module back into an instruction stream.  This is where the
 * module-wide invariants are enforced: declarations are emitted in topological
 * order, tombstones are skipped, names and decorations are generated from the
 * module's own maps, and entry point interfaces are derived from the
 * instructions, so ids that have been deleted leave no trace.
 *
 * Call deduplicate_types() first if a pass may have created duplicate
 * declarations of unique types (SpirVTransformer::run does this after every
 * pass).
 */
SpirVModule::InstructionStream SpirVModule::
emit() const {
  std::vector<uint32_t> words {
    spv::MagicNumber, _version, _generator, _id_bound, _schema,
  };

  auto emit_instruction = [&](spv::Op opcode, const uint32_t *args, size_t nargs) {
    words.push_back(((uint32_t)(nargs + 1) << spv::WordCountShift) | opcode);
    if (nargs > 0) {
      words.insert(words.end(), args, args + nargs);
    }
  };

  for (const Instruction &op : _capabilities) {
    emit_instruction(op.opcode, op.args.data(), op.args.size());
  }
  for (const Instruction &op : _extensions) {
    emit_instruction(op.opcode, op.args.data(), op.args.size());
  }
  for (const Instruction &op : _ext_inst_imports) {
    emit_instruction(op.opcode, op.args.data(), op.args.size());
  }
  if (_memory_model.opcode == spv::OpMemoryModel) {
    emit_instruction(_memory_model.opcode, _memory_model.args.data(),
                     _memory_model.args.size());
  }

  for (const EntryPoint &ep : _entry_points) {
    pvector<uint32_t> args({(uint32_t)ep.model, ep.function_id});
    pack_string(args, ep.name);
    for (Id var_id : collect_interface_vars(ep.function_id)) {
      args.push_back(var_id);
    }
    emit_instruction(spv::OpEntryPoint, args.data(), args.size());
  }
  for (const Instruction &op : _execution_modes) {
    emit_instruction(op.opcode, op.args.data(), op.args.size());
  }

  // Debug section: strings and sources, then names, then OpModuleProcessed.
  for (const Instruction &op : _debug) {
    emit_instruction(op.opcode, op.args.data(), op.args.size());
  }
  for (const auto &item : _names) {
    if (item.second.empty()) {
      continue;
    }
    pvector<uint32_t> args({item.first});
    pack_string(args, item.second);
    emit_instruction(spv::OpName, args.data(), args.size());
  }
  for (const auto &item : _member_names) {
    for (size_t mi = 0; mi < item.second.size(); ++mi) {
      if (!item.second[mi].empty()) {
        pvector<uint32_t> args({item.first, (uint32_t)mi});
        pack_string(args, item.second[mi]);
        emit_instruction(spv::OpMemberName, args.data(), args.size());
      }
    }
  }
  for (const Instruction &op : _module_processed) {
    emit_instruction(op.opcode, op.args.data(), op.args.size());
  }

  // Annotations, in id order.
  for (const auto &item : _annotations) {
    for (const Annotation &annotation : item.second) {
      pvector<uint32_t> args({item.first});
      args.insert(args.end(), annotation.args.begin(), annotation.args.end());
      emit_instruction(annotation.opcode, args.data(), args.size());
    }
  }

  // Globals, in topological order.  A pass may have appended a declaration
  // that an earlier declaration was redirected to (via replace_type_id), so
  // dependency order is restored here rather than maintained incrementally.
  for (size_t index : sort_declarations()) {
    const Instruction &op = _declarations[index];
    emit_instruction(op.opcode, op.args.data(), op.args.size());
  }

  for (const Function &function : _functions) {
    uint32_t function_args[4] = {
      (uint32_t)get_type_id(function.type_id),
      (uint32_t)function.id,
      (uint32_t)function.control,
      (uint32_t)function.type_id,
    };
    emit_instruction(spv::OpFunction, function_args, 4u);

    const Instruction *function_type = find_declaration(function.type_id);
    for (size_t pi = 0; pi < function.parameters.size(); ++pi) {
      Id parameter_id = function.parameters[pi];
      Id parameter_type_id = get_type_id(parameter_id);
      if (function_type != nullptr &&
          function_type->opcode == spv::OpTypeFunction &&
          pi + 2 < function_type->args.size()) {
        parameter_type_id = Id(function_type->args[pi + 2]);
      }
      uint32_t parameter_args[2] = {
        (uint32_t)parameter_type_id,
        (uint32_t)parameter_id,
      };
      emit_instruction(spv::OpFunctionParameter, parameter_args, 2u);
    }

    for (const Instruction &op : function.instructions) {
      if (!op.is_nop()) {
        emit_instruction(op.opcode, op.args.data(), op.args.size());
      }
    }

    emit_instruction(spv::OpFunctionEnd, nullptr, 0u);
  }

  return InstructionStream(std::move(words));
}

/**
 * Checks that the module's id index and cached type resolutions (the only
 * derived state the module keeps) are consistent with a fresh parse of the
 * emitted module, then runs the SPIR-V validator on the emitted stream.  The
 * consistency check catches a pass editing an instruction's result type in
 * place without calling record_result, a stale declaration index (which
 * find_declaration and the emit-time topological sort navigate by), a
 * mutation path failing to invalidate the type resolutions, and any operand
 * left referencing a deleted id.  Logs an error for every mismatch and
 * returns false if any was found.  Intended for validation
 * in the test suite and via the shader-paranoid-validation config variable.
 */
bool SpirVModule::
validate() const {
  InstructionStream emitted = emit();
  SpirVModule fresh(emitted);
  nassertr(get_id_bound() == fresh.get_id_bound(), false);

  bool consistent = true;
  auto report = [&](uint32_t id, const char *field, int64_t value, int64_t fresh_value) {
    shader_cat.error()
      << "Inconsistent id index after transformation: id " << id << " has "
      << field << " " << value << ", but " << fresh_value
      << " when parsed from the module\n";
    consistent = false;
  };

  for (uint32_t word = 0; word < get_id_bound(); ++word) {
    Id id(word);
    DefinitionType dtype = get_definition_type(id);
    DefinitionType fresh_dtype = fresh.get_definition_type(id);

    if (dtype != fresh_dtype) {
      report(id, "definition type", (int64_t)dtype, (int64_t)fresh_dtype);
      continue;
    }

    // The recorded declaration index must locate the instruction that
    // actually declares the id: find_declaration, replace_type_id and the
    // emit-time topological sort all navigate by it.  This is checked for
    // DT_none ids too, to catch a stale index on a cleared id.
    const Instruction *decl = find_declaration(id);
    const Instruction *fresh_decl = fresh.find_declaration(id);
    if ((decl != nullptr) != (fresh_decl != nullptr)) {
      report(id, "module-scope declaration", decl != nullptr,
             fresh_decl != nullptr);
    }
    else if (decl != nullptr) {
      if (decl->get_result() != fresh_decl->get_result()) {
        report(id, "declared result id", decl->get_result(),
               fresh_decl->get_result());
      }
      if (decl->opcode != fresh_decl->opcode) {
        report(id, "declaration opcode", (int64_t)decl->opcode,
               (int64_t)fresh_decl->opcode);
      }
    }

    if (dtype == DT_none) {
      continue;
    }

    if (dtype != DT_type &&
        dtype != DT_pointer_type &&
        dtype != DT_function_type &&
        dtype != DT_ext_inst &&
        dtype != DT_string &&
        dtype != DT_typeless) {
      // The index mirrors the result type operand for values; the mirror
      // must match what the instructions say.
      Id type_id = get_type_id(id);
      Id fresh_type_id = fresh.get_type_id(id);
      if (type_id != fresh_type_id) {
        report(id, "type id", type_id, fresh_type_id);
      }
    }

    if (get_function_id(id) != fresh.get_function_id(id)) {
      report(id, "function id", get_function_id(id),
             fresh.get_function_id(id));
    }

    // The resolutions are derived from the same declarations by the same
    // code, so a mismatch means a cached resolution went stale (a mutation
    // path failed to invalidate).
    const ShaderType *type = nullptr;
    const ShaderType *fresh_type = nullptr;
    switch (dtype) {
    case DT_type:
    case DT_pointer_type:
    case DT_variable:
    case DT_constant:
    case DT_spec_constant:
    case DT_function_parameter:
      type = resolve_type(id);
      fresh_type = fresh.resolve_type(id);
      break;

    default:
      break;
    }
    if (type != fresh_type) {
      shader_cat.error()
        << "Inconsistent type resolution after transformation: id " << id
        << " resolves to ";
      if (type != nullptr) {
        shader_cat.error(false) << *type;
      } else {
        shader_cat.error(false) << "(null)";
      }
      shader_cat.error(false) << ", but ";
      if (fresh_type != nullptr) {
        shader_cat.error(false) << *fresh_type;
      } else {
        shader_cat.error(false) << "(null)";
      }
      shader_cat.error(false) << " when parsed from the module\n";
      consistent = false;
    }
  }

  // Deleting an id cleans up the annotations *on* it, but a handful of
  // annotation forms embed ids in their operands, and execution modes can
  // reference constants; a pass that deletes an id these refer to must
  // remove them itself, which is easy to forget.  Sweep for danglers.
  auto check_reference = [&](Id id, const char *referrer, uint32_t referrer_id) {
    if (id == 0 || id >= get_id_bound() ||
        get_definition_type(id) == DT_none) {
      shader_cat.error()
        << "Dangling reference after transformation: " << referrer << " on id "
        << referrer_id << " refers to nonexistent id " << id << "\n";
      consistent = false;
    }
  };

  for (uint32_t word = 0; word < get_id_bound(); ++word) {
    const Annotations *annotations = get_annotations(Id(word));
    if (annotations == nullptr) {
      continue;
    }
    for (const Annotation &annotation : *annotations) {
      switch (annotation.opcode) {
      case spv::OpDecorateId:
        // Every operand after the decoration is an id.
        for (size_t ai = 1; ai < annotation.args.size(); ++ai) {
          check_reference(Id(annotation.args[ai]), "OpDecorateId", word);
        }
        break;

      case spv::OpGroupDecorate:
        // Every operand is a decoration target.
        for (size_t ai = 0; ai < annotation.args.size(); ++ai) {
          check_reference(Id(annotation.args[ai]), "OpGroupDecorate", word);
        }
        break;

      case spv::OpGroupMemberDecorate:
        // Pairs of target id and member index.
        for (size_t ai = 0; ai + 1 < annotation.args.size(); ai += 2) {
          check_reference(Id(annotation.args[ai]), "OpGroupMemberDecorate", word);
        }
        break;

      default:
        break;
      }
    }
  }

  for (const Instruction &op : _execution_modes) {
    if (op.args.empty()) {
      continue;
    }
    check_reference(Id(op.args[0]), "an execution mode", op.args[0]);
    if (op.opcode == spv::OpExecutionModeId) {
      // Every operand after the mode is an id (eg. LocalSizeId constants).
      for (size_t ai = 2; ai < op.args.size(); ++ai) {
        check_reference(Id(op.args[ai]), "OpExecutionModeId", op.args[0]);
      }
    }
  }

  // Now check every id operand of every instruction (that we can classify)
  // for dangling references (ids with DT_none).
  small_vector<uint16_t, 8> id_operands;
  auto check_instruction = [&](const Instruction &op) {
    if (op.is_nop() || !get_instruction_id_operands(op, id_operands)) {
      return;
    }
    for (uint16_t arg_index : id_operands) {
      Id ref_id(op.args[arg_index]);
      if (ref_id == 0 || ref_id >= get_id_bound() ||
          get_definition_type(ref_id) == DT_none) {
        shader_cat.error()
          << "Dangling reference after transformation: operand " << arg_index
          << " of an opcode " << (uint32_t)op.opcode
          << " instruction refers to nonexistent id " << ref_id << "\n";
        consistent = false;
      }
    }
  };
  for (const Instruction &op : _debug) {
    check_instruction(op);
  }
  for (const Instruction &op : _declarations) {
    check_instruction(op);
  }
  for (const Function &function : _functions) {
    for (const Instruction &op : function.instructions) {
      check_instruction(op);
    }
  }

  // Finally, run the SPIR-V validator on the emitted stream, against the
  // version the header declares.
  spv_target_env env;
  switch ((_version >> 8) & 0xff) {
  case 0: env = SPV_ENV_UNIVERSAL_1_0; break;
  case 1: env = SPV_ENV_UNIVERSAL_1_1; break;
  case 2: env = SPV_ENV_UNIVERSAL_1_2; break;
  case 3: env = SPV_ENV_UNIVERSAL_1_3; break;
  case 4: env = SPV_ENV_UNIVERSAL_1_4; break;
  case 5: env = SPV_ENV_UNIVERSAL_1_5; break;
  default: env = SPV_ENV_UNIVERSAL_1_6; break;
  }
  if (!emitted.validate(env)) {
    consistent = false;
  }

  return consistent;
}

/**
 * Returns a snapshot describing how variables are used.
 */
SpirVUsageAnalysis SpirVModule::
analyze_usage() const {
  SpirVUsageAnalysis usage(_id_bound);

  // Seed the module-scope state: variables and parameters originate from
  // themselves, constants (including spec constants and spec constant
  // expressions) are constant expressions.
  for (uint32_t word = 0; word < _id_bound; ++word) {
    Id id(word);
    switch (get_definition(id)._dtype) {
    case SpirVModule::DT_variable:
    case SpirVModule::DT_function_parameter:
      usage._origins[word] = id;
      break;

    case SpirVModule::DT_constant:
    case SpirVModule::DT_spec_constant:
      usage._flags[word] |= SpirVUsageAnalysis::UF_constant_expression;
      break;

    default:
      break;
    }
  }

  // Identify the GLSL.std.450 extended instruction set, whose interpolation
  // and pointer-output functions consume variable pointers.
  // Technically, there may be multiple, even if that's rare in practice.
  pset<uint32_t> glsl_std_450;
  for (const SpirVModule::Instruction &op : _ext_inst_imports) {
    if (!op.args.empty() && op.get_string(1) == "GLSL.std.450") {
      glsl_std_450.insert(op.args[0]);
    }
  }

  // Set once the first deferred-use flag is assigned, so that the operand
  // scans below can be skipped entirely for modules without opaque loads.
  bool any_deferred = false;

  small_vector<uint16_t, 8> id_operands;
  for (const SpirVModule::Function &function : _functions) {
    for (const SpirVModule::Instruction &op : function.instructions) {
      switch (op.opcode) {
      case spv::OpLoad:
        if (op.args.size() >= 3) {
          usage.set_origin(Id(op.args[1]), usage.get_origin(Id(op.args[2])));

          // For an opaque (image/sampler) value, marking the variable used is
          // deferred until an instruction consumes the loaded value: a
          // transform pass (eg. texture query emulation) can orphan such a
          // load, and the variable must then read as unused, since a leftover
          // image resource cannot always be expressed in the target shading
          // language.  A load of any other type marks the variable used
          // directly; tracking usage through arbitrary values would be
          // prohibitive.
          const Instruction *type_decl = find_declaration(Id(op.args[0]));
          if (type_decl != nullptr &&
              (type_decl->opcode == spv::OpTypeImage ||
               type_decl->opcode == spv::OpTypeSampledImage ||
               type_decl->opcode == spv::OpTypeSampler)) {
            usage.set_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_deferred_use);
            any_deferred = true;
          } else {
            usage.mark_used(Id(op.args[1]));
          }
        }
        break;

      case spv::OpAtomicLoad:
      case spv::OpAtomicExchange:
      case spv::OpAtomicCompareExchange:
      case spv::OpAtomicCompareExchangeWeak:
      case spv::OpAtomicIIncrement:
      case spv::OpAtomicIDecrement:
      case spv::OpAtomicIAdd:
      case spv::OpAtomicISub:
      case spv::OpAtomicSMin:
      case spv::OpAtomicUMin:
      case spv::OpAtomicSMax:
      case spv::OpAtomicUMax:
      case spv::OpAtomicAnd:
      case spv::OpAtomicOr:
      case spv::OpAtomicXor:
      case spv::OpAtomicFlagTestAndSet:
      case spv::OpAtomicFMinEXT:
      case spv::OpAtomicFMaxEXT:
      case spv::OpAtomicFAddEXT:
      case spv::OpCooperativeMatrixLoadKHR:
        // Loading through a pointer makes its variable used, because it would
        // be prohibitive to track usage through values.
        if (op.args.size() >= 3) {
          usage.set_origin(Id(op.args[1]), usage.get_origin(Id(op.args[2])));
          usage.mark_used(Id(op.args[1]));
        }
        break;

      case spv::OpStore:
      case spv::OpAtomicStore:
      case spv::OpAtomicFlagClear:
      case spv::OpCooperativeMatrixStoreKHR:
        // A write creates no result id, but the variable counts as used.
        if (!op.args.empty()) {
          usage.mark_used(Id(op.args[0]));
        }
        // A store of an opaque value consumes it.  Valid modules keep opaque
        // variables in UniformConstant, which is read-only, so this cannot
        // occur in practice; but were an unconsumed load to feed a store, the
        // load's removal would leave the store referencing a deleted id.
        if (op.opcode == spv::OpStore && op.args.size() >= 2 &&
            usage.has_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_deferred_use)) {
          usage.mark_used(Id(op.args[1]));
        }
        break;

      case spv::OpCopyMemory:
      case spv::OpCopyMemorySized:
        if (op.args.size() >= 2) {
          usage.mark_used(Id(op.args[0]));
          usage.mark_used(Id(op.args[1]));
        }
        break;

      case spv::OpAccessChain:
      case spv::OpInBoundsAccessChain:
      case spv::OpPtrAccessChain:
      case spv::OpInBoundsPtrAccessChain:
        // Chains propagate their base variable, so that a load through them
        // can transitively mark it used.
        if (op.args.size() >= 3) {
          usage.set_origin(Id(op.args[1]), usage.get_origin(Id(op.args[2])));

          // If one of the indices (including the base element for
          // OpPtrAccessChain) isn't a constant expression, the variable is
          // dynamically indexed.
          for (size_t i = 3; i < op.args.size(); ++i) {
            if (!usage.has_flag(Id(op.args[i]), SpirVUsageAnalysis::UF_constant_expression)) {
              usage.set_origin_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_dynamically_indexed);
              break;
            }
          }
        }
        break;

      case spv::OpImageTexelPointer:
        if (op.args.size() >= 3) {
          usage.set_origin(Id(op.args[1]), usage.get_origin(Id(op.args[2])));
        }
        break;

      case spv::OpSampledImage:
        // Combining an image with a sampler consumes both eagerly; tracking
        // the pair through to an eventual sample would require tracking two
        // origins per value.
        if (op.args.size() >= 3) {
          usage.set_origin(Id(op.args[1]), usage.get_origin(Id(op.args[2])));
          usage.mark_used(Id(op.args[2]));
          if (op.args.size() >= 4) {
            usage.mark_used(Id(op.args[3]));
          }
        }
        break;

      case spv::OpImage:
        if (op.args.size() >= 3) {
          usage.set_origin(Id(op.args[1]), usage.get_origin(Id(op.args[2])));
          usage.set_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_sampled_image);
          if (usage.has_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_deferred_use)) {
            usage.set_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_deferred_use);
          }
        }
        break;

      case spv::OpCopyObject:
      case spv::OpExpectKHR:
      case spv::OpCompositeExtract:
        // These preserve where the value came from and whether it is a
        // constant expression.  (OpCompositeExtract does not propagate the
        // origin, only the flags; composites of pointers do not occur, and a
        // deferred use may only be propagated together with its origin.)
        if (op.args.size() >= 3) {
          if (op.opcode != spv::OpCompositeExtract) {
            usage.set_origin(Id(op.args[1]), usage.get_origin(Id(op.args[2])));
            if (usage.has_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_deferred_use)) {
              usage.set_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_deferred_use);
            }
          }
          else if (usage.has_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_deferred_use)) {
            usage.mark_used(Id(op.args[2]));
          }
          if (op.args[1] < usage._flags.size() && op.args[2] < usage._flags.size()) {
            usage._flags[op.args[1]] |= usage._flags[op.args[2]] &
              (SpirVUsageAnalysis::UF_constant_expression | SpirVUsageAnalysis::UF_sampled_image);
          }
        }
        break;

      case spv::OpBitcast:
        if (op.args.size() >= 3) {
          usage.set_origin(Id(op.args[1]), usage.get_origin(Id(op.args[2])));
          if (usage.has_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_constant_expression)) {
            usage.set_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_constant_expression);
          }
          // Casting to a non-pointer type counts as a load, because (as above)
          // we can't track loads through values right now.
          if (get_definition(Id(op.args[0]))._dtype != SpirVModule::DT_pointer_type) {
            usage.mark_used(Id(op.args[1]));
          }
        }
        break;

      case spv::OpFunctionCall:
        // Mark all arguments as used.  In the future we could be smart
        // enough to only mark the arguments used if the relevant parameters
        // are used within the function itself.
        for (size_t i = 3; i < op.args.size(); ++i) {
          usage.mark_used(Id(op.args[i]));
        }
        // Mark the function as used (even if its return value is unused, as
        // the function may have side effects).
        if (op.args.size() >= 3) {
          usage.set_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_used);
        }
        break;

      case spv::OpExtInst:
        // Some GLSL standard functions take pointers as arguments.
        if (op.args.size() >= 5 && glsl_std_450.count(op.args[2])) {
          switch (op.args[3]) {
          case GLSLstd450Modf:
          case GLSLstd450Frexp:
            if (op.args.size() >= 6) {
              usage.mark_used(Id(op.args[5]));
            }
            break;

          case GLSLstd450InterpolateAtCentroid:
            usage.mark_used(Id(op.args[4]));
            break;

          case GLSLstd450InterpolateAtSample:
          case GLSLstd450InterpolateAtOffset:
            usage.mark_used(Id(op.args[4]));
            if (op.args.size() >= 6) {
              usage.mark_used(Id(op.args[5]));
            }
            break;
          }
        }
        // An operand referencing an opaque value (eg. a non-semantic debug
        // instruction) consumes it.  Operands are ids in the instruction sets
        // that occur in practice (GLSL.std.450 and the non-semantic sets).
        // If some other set were to use literal operands and a literal value
        // were to happen to overlap with an id, it may result in the variable
        // being kept alive unnecessarily.
        if (any_deferred) {
          for (size_t i = 4; i < op.args.size(); ++i) {
            if (usage.has_flag(Id(op.args[i]), SpirVUsageAnalysis::UF_deferred_use)) {
              usage.mark_used(Id(op.args[i]));
            }
          }
        }
        break;

      case spv::OpImageSampleImplicitLod:
      case spv::OpImageSampleExplicitLod:
      case spv::OpImageSampleProjImplicitLod:
      case spv::OpImageSampleProjExplicitLod:
      case spv::OpImageFetch:
      case spv::OpImageGather:
      case spv::OpImageSparseSampleImplicitLod:
      case spv::OpImageSparseSampleExplicitLod:
      case spv::OpImageSparseSampleProjImplicitLod:
      case spv::OpImageSparseSampleProjExplicitLod:
      case spv::OpImageSparseFetch:
      case spv::OpImageSparseGather:
        if (op.args.size() >= 3) {
          usage.set_origin_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_non_dref_sampled);
          usage.mark_used(Id(op.args[2]));
        }
        break;

      case spv::OpImageSampleDrefImplicitLod:
      case spv::OpImageSampleDrefExplicitLod:
      case spv::OpImageSampleProjDrefImplicitLod:
      case spv::OpImageSampleProjDrefExplicitLod:
      case spv::OpImageDrefGather:
      case spv::OpImageSparseSampleDrefImplicitLod:
      case spv::OpImageSparseSampleDrefExplicitLod:
      case spv::OpImageSparseSampleProjDrefImplicitLod:
      case spv::OpImageSparseSampleProjDrefExplicitLod:
      case spv::OpImageSparseDrefGather:
        if (op.args.size() >= 3) {
          usage.set_origin_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_dref_sampled);
          usage.mark_used(Id(op.args[2]));
        }
        break;

      case spv::OpImageQuerySizeLod:
      case spv::OpImageQuerySize:
      case spv::OpImageQueryLevels:
        if (op.args.size() >= 3) {
          usage.set_origin_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_queried_size_levels);
          usage.mark_used(Id(op.args[2]));
        }
        break;

      case spv::OpImageQueryLod:
      case spv::OpImageQuerySamples:
      case spv::OpImageQueryFormat:
      case spv::OpImageQueryOrder:
        if (op.args.size() >= 3) {
          usage.mark_used(Id(op.args[2]));
        }
        break;

      case spv::OpImageRead:
      case spv::OpImageSparseRead:
        if (op.args.size() >= 3) {
          usage.mark_used(Id(op.args[2]));
        }
        break;

      case spv::OpImageWrite:
        if (!op.args.empty()) {
          usage.mark_used(Id(op.args[0]));
        }
        break;

      case spv::OpArrayLength:
      case spv::OpConvertPtrToU:
        if (op.args.size() >= 3) {
          usage.mark_used(Id(op.args[2]));
        }
        break;

      case spv::OpPhi:
        // Like OpSelect below, this can operate on pointers (with the
        // VariablePointers capability).  The origin of the result cannot be
        // tracked statically, so conservatively mark every incoming value
        // used.
        for (size_t i = 2; i + 1 < op.args.size(); i += 2) {
          usage.mark_used(Id(op.args[i]));
        }
        break;

      case spv::OpSelect:
        // This can in theory operate on pointers, which is why we handle it.
        if (op.args.size() >= 5) {
          usage.mark_used(Id(op.args[3]));
          usage.mark_used(Id(op.args[4]));
          if (usage.has_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_constant_expression) &&
              usage.has_flag(Id(op.args[3]), SpirVUsageAnalysis::UF_constant_expression) &&
              usage.has_flag(Id(op.args[4]), SpirVUsageAnalysis::UF_constant_expression)) {
            usage.set_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_constant_expression);
          }
        }
        break;

      case spv::OpReturnValue:
        // A pointer can be returned when certain caps are present.
        if (!op.args.empty()) {
          usage.mark_used(Id(op.args[0]));
        }
        break;

      case spv::OpPtrEqual:
      case spv::OpPtrNotEqual:
      case spv::OpPtrDiff:
        // Consider a variable "used" if its pointer value is being compared,
        // to be on the safe side.
        if (op.args.size() >= 4) {
          usage.mark_used(Id(op.args[2]));
          usage.mark_used(Id(op.args[3]));
        }
        break;

      // Unary operators that preserve constant-expression status.
      case spv::OpConvertFToU:
      case spv::OpConvertFToS:
      case spv::OpConvertSToF:
      case spv::OpConvertUToF:
      case spv::OpQuantizeToF16:
      case spv::OpSatConvertSToU:
      case spv::OpSatConvertUToS:
      case spv::OpConvertUToPtr:
      case spv::OpSNegate:
      case spv::OpFNegate:
      case spv::OpAny:
      case spv::OpAll:
      case spv::OpIsNan:
      case spv::OpIsInf:
      case spv::OpIsFinite:
      case spv::OpIsNormal:
      case spv::OpSignBitSet:
      case spv::OpNot:
      case spv::OpConvertFToBF16INTEL:
      case spv::OpConvertBF16ToFINTEL:
        if (op.args.size() >= 3 &&
            usage.has_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_constant_expression)) {
          usage.set_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_constant_expression);
        }
        break;

      // Binary operators that preserve constant-expression status.
      case spv::OpIAdd:
      case spv::OpFAdd:
      case spv::OpISub:
      case spv::OpFSub:
      case spv::OpIMul:
      case spv::OpFMul:
      case spv::OpUDiv:
      case spv::OpSDiv:
      case spv::OpFDiv:
      case spv::OpUMod:
      case spv::OpSRem:
      case spv::OpSMod:
      case spv::OpFRem:
      case spv::OpFMod:
      case spv::OpVectorTimesScalar:
      case spv::OpMatrixTimesScalar:
      case spv::OpVectorTimesMatrix:
      case spv::OpMatrixTimesVector:
      case spv::OpMatrixTimesMatrix:
      case spv::OpOuterProduct:
      case spv::OpDot:
      case spv::OpIAddCarry:
      case spv::OpISubBorrow:
      case spv::OpUMulExtended:
      case spv::OpSMulExtended:
      case spv::OpShiftRightLogical:
      case spv::OpShiftRightArithmetic:
      case spv::OpShiftLeftLogical:
      case spv::OpBitwiseOr:
      case spv::OpBitwiseXor:
      case spv::OpBitwiseAnd:
        if (op.args.size() >= 4 &&
            usage.has_flag(Id(op.args[2]), SpirVUsageAnalysis::UF_constant_expression) &&
            usage.has_flag(Id(op.args[3]), SpirVUsageAnalysis::UF_constant_expression)) {
          usage.set_flag(Id(op.args[1]), SpirVUsageAnalysis::UF_constant_expression);
        }
        break;

      default:
        // Safety net for the deferred loads: an opcode without a curated
        // case above that references an opaque value consumes it (eg.
        // OpCompositeInsert, whose inserted object can be an opaque value).
        // The id operand classification makes this exact for known opcodes
        // (an OpLine line number or OpSwitch case literal is never mistaken
        // for an id); in an unknown opcode (eg. a vendor image extension)
        // every plausible id word is included, and a literal colliding with
        // a deferred value id merely keeps the variable alive.
        if (any_deferred) {
          get_instruction_id_operands(op, id_operands);
          for (uint16_t arg_index : id_operands) {
            if (usage.has_flag(Id(op.args[arg_index]), SpirVUsageAnalysis::UF_deferred_use)) {
              usage.mark_used(Id(op.args[arg_index]));
            }
          }
        }
        break;
      }
    }
  }

  return usage;
}

/**
 * Adds the given capability to the module, if it is not already declared.
 */
void SpirVModule::
add_capability(spv::Capability capability) {
  for (const Instruction &op : _capabilities) {
    if (!op.args.empty() && op.args[0] == (uint32_t)capability) {
      return;
    }
  }
  _capabilities.push_back(Instruction(spv::OpCapability, {(uint32_t)capability}));
}

/**
 * Adds the given extension to the module, if it is not already declared.
 */
void SpirVModule::
add_extension(std::string_view name) {
  for (const Instruction &op : _extensions) {
    if (op.get_string(0) == name) {
      return;
    }
  }
  Instruction op(spv::OpExtension);
  pack_string(op.args, name);
  _extensions.push_back(std::move(op));
}

/**
 * Adds an OpExtInstImport with the given set name and returns its id.
 */
SpirVId SpirVModule::
add_ext_inst_import(std::string_view name) {
  for (const Instruction &op : _ext_inst_imports) {
    if (op.get_string(1) == name) {
      return Id(op.args[0]);
    }
  }
  Id id = allocate_id();
  Instruction op(spv::OpExtInstImport, {id});
  pack_string(op.args, name);
  record_result(op);
  _ext_inst_imports.push_back(std::move(op));
  return id;
}

/**
 * Sets the addressing and memory model of the module.
 */
void SpirVModule::
set_memory_model(spv::AddressingModel addressing, spv::MemoryModel memory) {
  _memory_model = Instruction(spv::OpMemoryModel,
    {(uint32_t)addressing, (uint32_t)memory});
}

/**
 * Adds an entry point for the given function.
 */
void SpirVModule::
add_entry_point(spv::ExecutionModel model, Id function_id, std::string name) {
  EntryPoint ep;
  ep.model = model;
  ep.function_id = function_id;
  ep.name = std::move(name);
  _entry_points.push_back(std::move(ep));
}

/**
 * Returns the entry point interface for the given entry function: every
 * module-scope variable referenced by a function reachable from it,
 * restricted to the Input and Output storage classes before SPIR-V 1.4
 * (which lists all storage classes).
 */
pvector<SpirVId> SpirVModule::
collect_interface_vars(Id function_id) const {
  pset<Id> globals;
  pset<Id> visited {function_id};
  small_vector<Id, 4> worklist {function_id};
  small_vector<uint16_t, 8> id_operands;

  while (!worklist.empty()) {
    Id fid = worklist.back();
    worklist.pop_back();

    const Function *fn = nullptr;
    for (const Function &function : _functions) {
      if (function.id == fid) {
        fn = &function;
        break;
      }
    }
    if (fn == nullptr) {
      continue;
    }

    for (const Instruction &op : fn->instructions) {
      if (op.opcode == spv::OpFunctionCall && op.args.size() >= 3 &&
          visited.insert(Id(op.args[2])).second) {
        worklist.push_back(Id(op.args[2]));
      }
      get_instruction_id_operands(op, id_operands);
      for (uint16_t arg_index : id_operands) {
        uint32_t word = op.args[arg_index];
        if (word < _defs.size() &&
            _defs[word]._dtype == DT_variable &&
            _defs[word]._function_id == 0) {
          globals.insert(Id(word));
        }
      }
    }
  }

  pvector<Id> result;
  for (Id var_id : globals) {
    if (_version < 0x10400) {
      spv::StorageClass storage_class = get_storage_class(var_id);
      if (storage_class != spv::StorageClassInput &&
          storage_class != spv::StorageClassOutput) {
        continue;
      }
    }
    result.push_back(var_id);
  }
  return result;
}

/**
 * Adds an execution mode for the given entry point function.
 */
void SpirVModule::
add_execution_mode(Id function_id, spv::ExecutionMode mode,
                   std::initializer_list<uint32_t> args) {
  Instruction op(spv::OpExecutionMode, {function_id, (uint32_t)mode});
  op.args.insert(op.args.end(), args.begin(), args.end());
  _execution_modes.push_back(std::move(op));
}

/**
 * Returns the name that will be emitted for the given id.
 */
const std::string &SpirVModule::
get_name(Id id) const {
  static const std::string empty;
  auto it = _names.find(id);
  return it != _names.end() ? it->second : empty;
}

/**
 * Sets the name emitted for the given id.
 */
void SpirVModule::
set_name(Id id, std::string name) {
  _names[id] = std::move(name);
}

/**
 * Returns the name of the given struct member, or an empty string.
 */
const std::string &SpirVModule::
get_member_name(Id type_id, uint32_t member_index) const {
  static const std::string empty;
  auto it = _member_names.find(type_id);
  if (it == _member_names.end() || member_index >= it->second.size()) {
    return empty;
  }
  return it->second[member_index];
}

/**
 * Sets the name of the given struct member.
 */
void SpirVModule::
set_member_name(Id type_id, uint32_t member_index, std::string name) {
  pvector<std::string> &names = _member_names[type_id];
  if (member_index >= names.size()) {
    names.resize(member_index + 1);
  }
  names[member_index] = std::move(name);

  // The member names are baked into the resolved struct type.
  invalidate_types();
}

/**
 * Returns the first id carrying the given OpName, or 0 if there is none.
 */
SpirVId SpirVModule::
find_named(std::string_view name) const {
  for (const auto &item : _names) {
    if (item.second == name) {
      return Id(item.first);
    }
  }
  return Id();
}

/**
 * Adds an OpString with the given contents and returns its id.
 */
SpirVId SpirVModule::
add_string(std::string_view str) {
  Id id = allocate_id();
  Instruction op(spv::OpString, {id});
  pack_string(op.args, str);
  record_result(op);
  _debug.push_back(std::move(op));
  return id;
}

/**
 * Returns true if the given string id is referenced by an instruction in the
 * module.  This is exact for known opcodes (a literal that collides with the
 * string id is never mistaken for a reference); an unknown opcode's operands
 * are scanned conservatively, which can only over-report.
 */
bool SpirVModule::
is_string_referenced(Id id) const {
  small_vector<uint16_t, 8> id_operands;
  auto references_string = [&](const Instruction &op) {
    get_instruction_id_operands(op, id_operands);
    for (uint16_t arg_index : id_operands) {
      if (op.args[arg_index] == id) {
        return true;
      }
    }
    return false;
  };

  for (const Instruction &op : _debug) {
    if (references_string(op)) {
      return true;
    }
  }
  for (const Instruction &op : _declarations) {
    if (references_string(op)) {
      return true;
    }
  }
  for (const Function &function : _functions) {
    for (const Instruction &op : function.instructions) {
      if (references_string(op)) {
        return true;
      }
    }
  }
  return false;
}

/**
 * Returns true if the given id carries the given (non-member) decoration.
 */
bool SpirVModule::
has_decoration(Id id, spv::Decoration decoration) const {
  const Annotations *annotations = get_annotations(id);
  if (annotations == nullptr) {
    return false;
  }
  for (const Annotation &annotation : *annotations) {
    if (!annotation.is_member_annotation() &&
        annotation.get_decoration() == decoration) {
      return true;
    }
  }
  return false;
}

/**
 * Returns the first extra operand of the given (non-member) decoration on
 * the given id, or the given default if the id does not carry it.
 */
uint32_t SpirVModule::
get_decoration(Id id, spv::Decoration decoration, uint32_t default_value) const {
  const Annotations *annotations = get_annotations(id);
  if (annotations == nullptr) {
    return default_value;
  }
  for (const Annotation &annotation : *annotations) {
    if (!annotation.is_member_annotation() &&
        annotation.get_decoration() == decoration) {
      return (annotation.args.size() >= 2) ? annotation.args[1] : default_value;
    }
  }
  return default_value;
}

/**
 * Returns true if the given struct member carries the given decoration.
 */
bool SpirVModule::
has_member_decoration(Id type_id, uint32_t member_index, spv::Decoration decoration) const {
  const Annotations *annotations = get_annotations(type_id);
  if (annotations == nullptr) {
    return false;
  }
  for (const Annotation &annotation : *annotations) {
    if (annotation.is_member_annotation() && annotation.args.size() >= 2 &&
        annotation.args[0] == member_index &&
        annotation.get_decoration() == decoration) {
      return true;
    }
  }
  return false;
}

/**
 * Returns the first extra operand of the given decoration on the given
 * struct member, or the given default if the member does not carry it.
 */
uint32_t SpirVModule::
get_member_decoration(Id type_id, uint32_t member_index, spv::Decoration decoration, uint32_t default_value) const {
  const Annotations *annotations = get_annotations(type_id);
  if (annotations == nullptr) {
    return default_value;
  }
  for (const Annotation &annotation : *annotations) {
    if (annotation.is_member_annotation() && annotation.args.size() >= 2 &&
        annotation.args[0] == member_index &&
        annotation.get_decoration() == decoration) {
      return (annotation.args.size() >= 3) ? annotation.args[2] : default_value;
    }
  }
  return default_value;
}

/**
 * Sets or replaces the location decoration of the given id.
 */
void SpirVModule::
set_location(Id id, int location) {
  remove_decoration(id, spv::DecorationLocation);
  decorate(id, spv::DecorationLocation, (uint32_t)location);
}

/**
 * Attaches the given annotation to the given id.  If the decoration feeds
 * into the type resolution, the cached resolutions are invalidated.
 */
void SpirVModule::
decorate(Id id, const Annotation &annotation) {
  _annotations[id].push_back(annotation);

  if (is_structural_decoration(annotation.get_decoration())) {
    invalidate_types();
  }
}

/**
 * Adds a member decoration without extra operands.
 */
void SpirVModule::
decorate_member(Id type_id, uint32_t member_index, spv::Decoration decoration) {
  decorate(type_id, Annotation {spv::OpMemberDecorate,
    {member_index, (uint32_t)decoration}});
}

/**
 * Adds a member decoration with one extra operand.
 */
void SpirVModule::
decorate_member(Id type_id, uint32_t member_index, spv::Decoration decoration, uint32_t value) {
  decorate(type_id, Annotation {spv::OpMemberDecorate,
    {member_index, (uint32_t)decoration, value}});
}

/**
 * Removes all non-member annotations with the given decoration from the
 * given id.  Returns true if any was removed.
 */
bool SpirVModule::
remove_decoration(Id id, spv::Decoration decoration) {
  auto it = _annotations.find(id);
  if (it == _annotations.end()) {
    return false;
  }
  bool any = false;
  Annotations &annotations = it->second;
  for (size_t i = annotations.size(); i-- > 0;) {
    if (!annotations[i].is_member_annotation() &&
        annotations[i].get_decoration() == decoration) {
      annotations.erase(annotations.begin() + i);
      any = true;
    }
  }
  if (any && is_structural_decoration(decoration)) {
    invalidate_types();
  }
  return any;
}

/**
 * Sets or replaces a single-operand decoration on the given id.
 */
void SpirVModule::
set_decoration(Id id, spv::Decoration decoration, uint32_t value) {
  remove_decoration(id, decoration);
  decorate(id, decoration, value);
}

/**
 * Removes all non-member annotations with the given decoration from every
 * id in the module.
 */
void SpirVModule::
strip_decoration(spv::Decoration decoration) {
  bool any = false;
  for (auto &item : _annotations) {
    Annotations &annotations = item.second;
    for (size_t i = annotations.size(); i-- > 0;) {
      if (!annotations[i].is_member_annotation() &&
          annotations[i].get_decoration() == decoration) {
        annotations.erase(annotations.begin() + i);
        any = true;
      }
    }
  }
  if (any && is_structural_decoration(decoration)) {
    invalidate_types();
  }
}

/**
 * Moves all annotations from one id to another, appending them to any the
 * target already has.
 */
void SpirVModule::
transfer_annotations(Id from, Id to) {
  auto it = _annotations.find(from);
  if (it == _annotations.end()) {
    return;
  }
  Annotations &target = _annotations[to];
  target.insert(target.end(),
                std::make_move_iterator(it->second.begin()),
                std::make_move_iterator(it->second.end()));
  _annotations.erase(it);

  // The moved annotations may include structural ones (BuiltIn and access
  // qualifiers move along with a variable swap).
  invalidate_types();
}

/**
 * Returns the declaring instruction of the given id, or null if the id is
 * not declared at module scope.
 */
const SpirVModule::Instruction *SpirVModule::
find_declaration(Id id) const {
  const Definition &def = get_definition(id);
  if (def._declaration_index < 0 ||
      (size_t)def._declaration_index >= _declarations.size()) {
    return nullptr;
  }
  const Instruction &op = _declarations[def._declaration_index];
  return op.is_nop() ? nullptr : &op;
}

/**
 * Mutable overload.  Since the declarations section is append-only and
 * stored in a deque, the returned pointer remains valid when declarations
 * are appended.  The declaration itself may still be modified or tombstoned.
 */
SpirVModule::Instruction *SpirVModule::
find_declaration(Id id) {
  const Definition &def = get_definition(id);
  if (def._declaration_index < 0 ||
      (size_t)def._declaration_index >= _declarations.size()) {
    return nullptr;
  }
  Instruction &op = _declarations[def._declaration_index];
  return op.is_nop() ? nullptr : &op;
}

/**
 * Adds the given instruction to the declarations section, recording its result
 * (if any) in the id index.
 */
void SpirVModule::
add_declaration(Instruction op) {
  int32_t index = (int32_t)_declarations.size();
  _declarations.push_back(std::move(op));
  record_result_at(_declarations.back(), Id(), index);
}

/**
 * Returns the function with the given id, or null if it does not exist.
 */
SpirVModule::Function *SpirVModule::
find_function(Id function_id) {
  for (Function &function : _functions) {
    if (function.id == function_id) {
      return &function;
    }
  }
  return nullptr;
}

/**
 * Starts a new function returning the given type and taking parameters of the
 * given types, at the end of the module, positioning the cursor inside it,
 * at the beginning of its body (an OpLabel is created implicitly).  Inspect
 * parameters on the created Function to obtain the parameter ids.
 *
 * The return_type may be nullptr if the function should return void.  The
 * parameter types are given as type ids, since parameters are commonly
 * pointer types, which ShaderType cannot express.
 */
SpirVBuilder SpirVModule::
make_function(const ShaderType *return_type, const pvector<Id> &param_type_ids,
              spv::FunctionControlMask control) {
  Id id = allocate_id();
  Id function_type_id = define_function_type(return_type, param_type_ids);
  Id return_type_id = get_type_id(function_type_id);

  // We need something for record_result to parse, since normally the actual
  // instruction is only generated by emit().
  record_result(Instruction(spv::OpFunction, {return_type_id, id, (uint32_t)control, function_type_id}), id);

  Function function;
  function.id = id;
  function.type_id = function_type_id;
  function.control = control;
  _functions.push_back(std::move(function));

  for (Id param_type_id : param_type_ids) {
    Id param_id = allocate_id();
    _functions.back().parameters.push_back(param_id);
    record_result(Instruction(spv::OpFunctionParameter,
                              {param_type_id, param_id}), id);
  }

  SpirVBuilder builder(*this, id, 0);
  builder.op_label();
  return builder;
}

/**
 * Starts a new function taking no arguments and returning void, and registers
 * it as an entry point.  This is a convenience method for make_function +
 * add_entry_point, and returns a SpirVBuilder positioned at the beginning of
 * the body, after the implicitly created OpLabel.
 */
SpirVBuilder SpirVModule::
make_entry_point(spv::ExecutionModel model, std::string name) {
  SpirVBuilder builder = make_function(nullptr, {});
  add_entry_point(model, builder.get_current_function_id(), std::move(name));
  return builder;
}

/**
 * Returns the type id of the given id.  For a value, this is the result type
 * operand of its declaring instruction (so for a variable, its pointer type);
 * for a function, its function type.  For a composite type declaration, this
 * is the element, component, pointee, image or return type, read from the
 * declaration.
 */
SpirVId SpirVModule::
get_type_id(Id id) const {
  const Definition &def = get_definition(id);
  if (def._dtype == DT_type || def._dtype == DT_pointer_type ||
      def._dtype == DT_function_type) {
    const Instruction *decl = find_declaration(id);
    if (decl != nullptr && decl->args.size() >= 2) {
      switch (decl->opcode) {
      case spv::OpTypeVector:
      case spv::OpTypeMatrix:
      case spv::OpTypeArray:
      case spv::OpTypeRuntimeArray:
      case spv::OpTypeSampledImage:
        return Id(decl->args[1]);

      case spv::OpTypeFunction:
        // The return type.
        return Id(decl->args[1]);

      case spv::OpTypePointer:
        return Id((decl->args.size() >= 3) ? decl->args[2] : 0);

      default:
        break;
      }
    }
    nassertr(false, Id());
    return Id();
  }
  nassertr(def._type_id != 0, Id());
  return def._type_id;
}

/**
 * Returns the storage class of the given variable or pointer type.  A legacy
 * buffer block (Uniform storage class, BufferBlock decoration) is reported
 * as StorageBuffer, matching how the type is resolved.
 */
spv::StorageClass SpirVModule::
get_storage_class(Id id) const {
  const Definition &def = get_definition(id);
  if (def._dtype == DT_variable) {
    if (def._function_id != 0) {
      return spv::StorageClassFunction;
    }
    const Instruction *decl = find_declaration(id);
    if (decl != nullptr && decl->args.size() >= 3) {
      const Instruction *ptr_decl = find_declaration(Id(decl->args[0]));
      Id pointee((ptr_decl != nullptr && ptr_decl->args.size() >= 3)
        ? ptr_decl->args[2] : 0);
      return canonicalize_storage_class((spv::StorageClass)decl->args[2], pointee);
    }
  }
  else if (def._dtype == DT_pointer_type) {
    const Instruction *decl = find_declaration(id);
    if (decl != nullptr && decl->args.size() >= 3) {
      return canonicalize_storage_class((spv::StorageClass)decl->args[1], Id(decl->args[2]));
    }
  }
  return spv::StorageClassMax;
}

/**
 * Looks up the given constant's (first word) value.  Also accepts a spec
 * constant, whose default value is returned; the value of an OpSpecConstantOp
 * expression is not knowable here, so 0 is returned for it.
 */
uint32_t SpirVModule::
resolve_constant(Id id) const {
  const Definition &def = get_definition(id);
  if (def._dtype != DT_constant && def._dtype != DT_spec_constant) {
    return error_expected(id, "a constant");
  }
  const Instruction *decl = find_declaration(id);
  if (decl == nullptr) {
    return error_expected(id, "a constant");
  }
  switch (decl->opcode) {
  case spv::OpConstantTrue:
  case spv::OpSpecConstantTrue:
    return 1;
  case spv::OpConstant:
  case spv::OpSpecConstant:
    return (decl->args.size() >= 3) ? decl->args[2] : 0;
  default:
    // Null and composite constants have no meaningful first word, and the
    // value of a spec constant expression cannot be evaluated here.
    return 0;
  }
}

/**
 * Returns the pointee type id of the given pointer type id.
 */
SpirVId SpirVModule::
unwrap_pointer_type(Id id) const {
  if (get_definition(id)._dtype != DT_pointer_type) {
    return error_expected(id, "a pointer type");
  }
  const Instruction *decl = find_declaration(id);
  nassertr(decl != nullptr && decl->args.size() >= 3, Id());
  return Id(decl->args[2]);
}

/**
 * Returns the number of members of the given struct type, or 0 if the id is
 * not a struct type.
 */
uint32_t SpirVModule::
get_num_members(Id type_id) const {
  const Instruction *decl = find_declaration(type_id);
  if (decl == nullptr || decl->opcode != spv::OpTypeStruct) {
    return 0;
  }
  return (uint32_t)decl->args.size() - 1;
}

/**
 * Returns the type id of the given member of the given struct type.
 */
SpirVId SpirVModule::
get_member_type_id(Id type_id, uint32_t member_index) const {
  const Instruction *decl = find_declaration(type_id);
  nassertr(decl != nullptr && decl->opcode == spv::OpTypeStruct, Id());
  nassertr((size_t)member_index + 1 < decl->args.size(), Id());
  return Id(decl->args[member_index + 1]);
}

/**
 * Returns the type id obtained by indexing into the given composite type
 * with the given (literal) index: the member type for a struct, the element
 * or component type for arrays, vectors and matrices.
 */
SpirVId SpirVModule::
get_composite_member_type_id(Id type_id, uint32_t member_index) const {
  const Instruction *decl = find_declaration(type_id);
  nassertr(decl != nullptr, Id());
  switch (decl->opcode) {
  case spv::OpTypeStruct:
    nassertr((size_t)member_index + 1 < decl->args.size(), Id());
    return Id(decl->args[member_index + 1]);

  case spv::OpTypeVector:
  case spv::OpTypeMatrix:
  case spv::OpTypeArray:
  case spv::OpTypeRuntimeArray:
    return Id(decl->args[1]);

  default:
    return error_expected(type_id, "a composite type");
  }
}

/**
 * (Re)derives the index entry for the given instruction's result id from the
 * instruction itself.  Any previously recorded declaration index is kept for
 * module-scope instructions (so a pass may edit a declaration in place and
 * call this to make the index follow).
 */
void SpirVModule::
record_result(const Instruction &op, Id function_id) {
  int32_t declaration_index = -1;
  Id result_id = op.get_result();
  if (function_id == 0 && result_id != 0 && result_id < _defs.size()) {
    declaration_index = _defs[result_id]._declaration_index;
  }
  record_result_at(op, function_id, declaration_index);
}

/**
 * Resolves the given id to a ShaderType.  A type id resolves to the type
 * itself, and a pointer type id to the pointed-to type; a pointer to a
 * storage buffer block resolves to a StorageBuffer type wrapping the struct.
 * A value id (a variable, constant, parameter or instruction result) resolves
 * to the type of the value, with pointers unwrapped as above.  Returns nullptr
 * for ids that have no meaningful type.
 * The result is cached; see invalidate_types.
 */
const ShaderType *SpirVModule::
resolve_type(Id id) const {
  const Definition &def = get_definition(id);
  switch (def._dtype) {
  case DT_type:
    return r_resolve_type(id);

  case DT_pointer_type:
    {
      if (def._type != nullptr) {
        return def._type;
      }
      const Instruction *decl = find_declaration(id);
      nassertr(decl != nullptr && decl->args.size() >= 3, nullptr);

      Id pointee_id(decl->args[2]);
      const ShaderType *type = resolve_type(pointee_id);

      spv::StorageClass storage_class = (spv::StorageClass)decl->args[1];
      if (type != nullptr && (storage_class == spv::StorageClassUniform ||
                              storage_class == spv::StorageClassStorageBuffer)) {
        // Unwrap arrays to get to the underlying block type.
        const Instruction *block_decl = find_declaration(pointee_id);
        while (block_decl != nullptr && (block_decl->opcode == spv::OpTypeArray ||
                                         block_decl->opcode == spv::OpTypeRuntimeArray)) {
          block_decl = find_declaration(Id(block_decl->args[1]));
        }
        if (block_decl != nullptr && block_decl->opcode == spv::OpTypeStruct) {
          Id block_type_id(block_decl->args[0]);
          bool buffer_block = has_decoration(block_type_id, spv::DecorationBufferBlock);
          bool block = has_decoration(block_type_id, spv::DecorationBlock);
          if (buffer_block || block) {
            if (buffer_block) {
              // Old SPIR-V way of declaring an SSBO.
              storage_class = spv::StorageClassStorageBuffer;
            }
            if (storage_class == spv::StorageClassStorageBuffer) {
              bool non_writable, non_readable;
              get_aggregate_access_flags(block_type_id, non_writable, non_readable);
              ShaderType::Access access = ShaderType::Access::READ_WRITE;
              if (non_writable) {
                access = (access & ShaderType::Access::READ_ONLY);
              }
              if (non_readable) {
                access = (access & ShaderType::Access::WRITE_ONLY);
              }
              const ShaderType *block_type = r_resolve_type(block_type_id);
              const ShaderType *new_type = ShaderType::register_type(
                ShaderType::StorageBuffer(block_type, access));
              type = type->replace_type(block_type, new_type);
            }
          }
        }
      }

      def._type = type;
      return type;
    }

  case DT_variable:
    {
      if (def._type != nullptr) {
        return def._type;
      }
      const ShaderType *type = resolve_type(def._type_id);
      if (type == nullptr) {
        return nullptr;
      }

      // If the variable itself has the readonly/writeonly qualifiers, then
      // we inject those into the (image or storage buffer) type.
      bool non_writable = has_decoration(id, spv::DecorationNonWritable);
      bool non_readable = has_decoration(id, spv::DecorationNonReadable);
      if (non_writable || non_readable) {
        const ShaderType *unwrapped_type = type;
        while (const ShaderType::Array *array_type = unwrapped_type->as_array()) {
          unwrapped_type = array_type->get_element_type();
        }

        const ShaderType *new_type = unwrapped_type;
        if (const ShaderType::Image *image = unwrapped_type->as_image()) {
          ShaderType::Access access = image->get_access();
          if (non_writable) {
            access = (access & ShaderType::Access::READ_ONLY);
          }
          if (non_readable) {
            access = (access & ShaderType::Access::WRITE_ONLY);
          }
          if (access != image->get_access()) {
            new_type = ShaderType::register_type(ShaderType::Image(
              image->get_texture_type(),
              image->get_sampled_type(),
              access));
          }
        }
        else if (const ShaderType::StorageBuffer *buffer = unwrapped_type->as_storage_buffer()) {
          ShaderType::Access access = buffer->get_access();
          if (non_writable) {
            access = (access & ShaderType::Access::READ_ONLY);
          }
          if (non_readable) {
            access = (access & ShaderType::Access::WRITE_ONLY);
          }
          if (access != buffer->get_access()) {
            new_type = ShaderType::register_type(ShaderType::StorageBuffer(
              buffer->get_contained_type(), access));
          }
        }

        if (new_type != unwrapped_type) {
          type = type->replace_type(unwrapped_type, new_type);
        }
      }

      def._type = type;
      return type;
    }

  case DT_constant:
  case DT_spec_constant:
  case DT_function_parameter:
  case DT_temporary:
    {
      if (def._type != nullptr) {
        return def._type;
      }
      Id type_id = def._type_id;
      if (type_id == 0) {
        return nullptr;
      }
      const ShaderType *type = resolve_type(type_id);
      def._type = type;
      return type;
    }

  default:
    return nullptr;
  }
}

/**
 * Returns the id of a live, canonical declaration of the given type, or 0 if
 * there is none.
 */
SpirVId SpirVModule::
find_type(const ShaderType *type) const {
  ensure_type_map();
  TypeMap::const_iterator tit = _type_map.find(type);
  if (tit != _type_map.end()) {
    return tit->second;
  }
  // A matrix type with an explicit stride is declared as the plain matrix
  // type; the stride lives in a decoration on the containing struct member.
  if (type != nullptr) {
    if (const ShaderType::Matrix *matrix = type->as_matrix()) {
      const ShaderType *plain = ShaderType::register_type(ShaderType::Matrix(
        matrix->get_scalar_type(), matrix->get_num_rows(), matrix->get_num_columns()));
      if (plain != type) {
        tit = _type_map.find(plain);
        if (tit != _type_map.end()) {
          return tit->second;
        }
      }
    }
  }
  return Id();
}

/**
 * For a given type id, recursively collects all struct types nested therein
 * (chasing pointer, array and member types) and writes them to the given
 * map.
 */
void SpirVModule::
collect_nested_structs(pmap<Id, const ShaderType::Struct *> &result, Id type_id) const {
  const Instruction *decl = find_declaration(type_id);
  if (decl == nullptr) {
    return;
  }
  switch (decl->opcode) {
  case spv::OpTypePointer:
    collect_nested_structs(result, Id(decl->args[2]));
    break;

  case spv::OpTypeArray:
  case spv::OpTypeRuntimeArray:
  case spv::OpTypeVector:
  case spv::OpTypeMatrix:
    collect_nested_structs(result, Id(decl->args[1]));
    break;

  case spv::OpTypeStruct:
    {
      const ShaderType *type = r_resolve_type(type_id);
      const ShaderType::Struct *struct_type = (type != nullptr) ? type->as_struct() : nullptr;
      if (struct_type != nullptr) {
        result[type_id] = struct_type;
      }
      for (size_t ai = 1; ai < decl->args.size(); ++ai) {
        collect_nested_structs(result, Id(decl->args[ai]));
      }
    }
    break;

  default:
    break;
  }
}

/**
 * Throws away all cached ShaderType resolutions and the type reuse map.
 * The next query re-derives from the current declarations and annotations.
 */
void SpirVModule::
invalidate_types() {
  for (Definition &def : _defs) {
    def._type = nullptr;
  }
  _type_map.clear();
  _type_map_valid = false;
}

/**
 * Ensures that the given type is declared, adding declarations to the
 * declarations section as necessary, and returns its id.
 */
SpirVId SpirVModule::
define_type(const ShaderType *type) {
  Id id = find_type(type);
  if (id != 0) {
    return id;
  }

  if (type != nullptr) {
    if (const ShaderType::Image *image_type = type->as_image()) {
      // Unspecified depthness, storage image.
      return define_image_type(image_type, 2, 2, spv::ImageFormatUnknown);
    }
  }

  id = allocate_id();

  if (type == nullptr) {
    add_declaration(Instruction(spv::OpTypeVoid, {id}));
  }
  else if (const ShaderType::Scalar *scalar_type = type->as_scalar()) {
    switch (scalar_type->get_scalar_type()) {
    case ShaderType::ST_float:
      add_declaration(Instruction(spv::OpTypeFloat, {id, 32}));
      break;
    case ShaderType::ST_double:
      add_declaration(Instruction(spv::OpTypeFloat, {id, 64}));
      break;
    case ShaderType::ST_int:
      add_declaration(Instruction(spv::OpTypeInt, {id, 32, 1}));
      break;
    case ShaderType::ST_uint:
      add_declaration(Instruction(spv::OpTypeInt, {id, 32, 0}));
      break;
    case ShaderType::ST_bool:
      add_declaration(Instruction(spv::OpTypeBool, {id}));
      break;
    default:
      add_declaration(Instruction(spv::OpTypeVoid, {id}));
      break;
    }
  }
  else if (const ShaderType::Vector *vector_type = type->as_vector()) {
    Id component_type = define_type(
      ShaderType::register_type(ShaderType::Scalar(vector_type->get_scalar_type())));

    add_declaration(Instruction(spv::OpTypeVector,
      {id, component_type, vector_type->get_num_components()}));
  }
  else if (const ShaderType::Matrix *matrix_type = type->as_matrix()) {
    Id row_type = define_type(
      ShaderType::register_type(ShaderType::Vector(matrix_type->get_scalar_type(), matrix_type->get_num_columns())));

    add_declaration(Instruction(spv::OpTypeMatrix,
      {id, row_type, matrix_type->get_num_rows()}));
  }
  else if (const ShaderType::Struct *struct_type = type->as_struct()) {
    size_t num_members = struct_type->get_num_members();
    Instruction op(spv::OpTypeStruct);
    op.args.push_back(id);

    for (size_t i = 0; i < num_members; ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);
      Id member_type_id = define_type(member.type);
      op.args.push_back(member_type_id);
    }

    add_declaration(std::move(op));

    // Setting the member names invalidates the cached resolutions; do it
    // before caching this type below.
    for (size_t i = 0; i < num_members; ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);
      if (!member.name.empty()) {
        set_member_name(id, (uint32_t)i, member.name);
      }
    }
  }
  else if (const ShaderType::Array *array_type = type->as_array()) {
    Id element_type = define_type(array_type->get_element_type());

    auto size = array_type->get_num_elements();
    if (size != 0) {
      Id constant_id = define_int_constant(size);
      add_declaration(Instruction(spv::OpTypeArray, {id, element_type, constant_id}));
    } else {
      add_declaration(Instruction(spv::OpTypeRuntimeArray, {id, element_type}));
    }
  }
  else if (type->as_sampler() != nullptr) {
    add_declaration(Instruction(spv::OpTypeSampler, {id}));
  }
  else if (const ShaderType::SampledImage *sampled_image_type = type->as_sampled_image()) {
    const ShaderType *image_type = ShaderType::register_type(ShaderType::Image(
      sampled_image_type->get_texture_type(),
      sampled_image_type->get_sampled_type(),
      ShaderType::Access::NONE));

    // Sampled image, depthness 1 if shadow sampler
    uint32_t depth = sampled_image_type->is_shadow() ? (uint32_t)1 : (uint32_t)0;
    Id image_id = define_image_type(image_type, depth, 1, spv::ImageFormatUnknown);

    // If the module already declares a sampled image type for this image
    // type (which is possible even on a type map miss, since multiple
    // sampled image types may map to the same ShaderType), this creates a
    // duplicate, which deduplicate_types() merges before the module is
    // emitted.
    add_declaration(Instruction(spv::OpTypeSampledImage, {id, image_id}));
  }
  else {
    add_declaration(Instruction(spv::OpTypeVoid, {id}));
  }

  // Memoize the requested type and put it in the reuse map: everything above
  // declares its type exactly the way the canonicality rules expect.  (The
  // void type is keyed under null, like a reseed of the map would key it.)
  modify_definition(id)._type = type;
  if (_type_map_valid) {
    _type_map[type] = id;
  }
  return id;
}

/**
 * Declares a new image type with the given extra properties (which are not
 * stored on ShaderType).
 */
SpirVId SpirVModule::
define_image_type(const ShaderType *type, uint32_t depth, uint32_t sampled, spv::ImageFormat format) {
  const ShaderType::Image *image_type = type->as_image();
  nassertr(image_type != nullptr, Id());

  const ShaderType *sampled_scalar_type =
    ShaderType::register_type(ShaderType::Scalar(image_type->get_sampled_type()));

  uint32_t args[9] = {
    0, // Result id, assigned below
    0, // Sampled type, assigned below
    0, // Dimensionality, see below
    depth,
    0, // Arrayness, see below
    0, // Multisample not supported
    sampled,
    (uint32_t)format,
    0, // Access qualifier
  };

  switch (image_type->get_texture_type()) {
  case Texture::TT_1d_texture:
    args[2] = spv::Dim1D;
    args[4] = 0;
    break;
  case Texture::TT_2d_texture:
    args[2] = spv::Dim2D;
    args[4] = 0;
    break;
  case Texture::TT_3d_texture:
    args[2] = spv::Dim3D;
    args[4] = 0;
    break;
  case Texture::TT_2d_texture_array:
    args[2] = spv::Dim2D;
    args[4] = 1;
    break;
  case Texture::TT_cube_map:
    args[2] = spv::DimCube;
    args[4] = 0;
    break;
  case Texture::TT_buffer_texture:
    args[2] = spv::DimBuffer;
    args[4] = 0;
    break;
  case Texture::TT_cube_map_array:
    args[2] = spv::DimCube;
    args[4] = 1;
    break;
  case Texture::TT_1d_texture_array:
    args[2] = spv::Dim1D;
    args[4] = 1;
    break;
  }

  uint32_t nargs = 8;
  if (sampled != 1) {
    switch (image_type->get_access()) {
    case ShaderType::Access::NONE:
      break;
    case ShaderType::Access::READ_ONLY:
      args[8] = spv::AccessQualifierReadOnly;
      ++nargs;
      break;
    case ShaderType::Access::WRITE_ONLY:
      args[8] = spv::AccessQualifierWriteOnly;
      ++nargs;
      break;
    case ShaderType::Access::READ_WRITE:
      args[8] = spv::AccessQualifierReadWrite;
      ++nargs;
      break;
    }
  }

  // If the module already declares a type with these exact operands, this
  // creates a duplicate, which deduplicate_types() merges before the module
  // is emitted.
  Id id = allocate_id();

  args[0] = id;
  args[1] = define_type(sampled_scalar_type);

  add_declaration(Instruction(spv::OpTypeImage, args, nargs));

  modify_definition(id)._type = type;

  // This only becomes the canonical declaration for this ShaderType if it
  // is declared exactly the way define_type() would declare it.
  if (_type_map_valid &&
      depth == 2 && sampled == 2 && format == spv::ImageFormatUnknown) {
    _type_map[type] = id;
  }
  return id;
}

/**
 * Ensures that a pointer type to the given (already declared) type with the
 * given storage class is declared, and returns its id.
 */
SpirVId SpirVModule::
define_pointer_type(Id type_id, spv::StorageClass storage_class) {
  nassertr_always(type_id != 0u, Id());

  // The comparison is against the storage class as declared, not the
  // canonicalized one that get_storage_class reports: pointer types are
  // requested and declared in emission terms (a legacy-style buffer block
  // keeps its declared Uniform class).
  for (const Instruction &op : _declarations) {
    if (op.opcode == spv::OpTypePointer && op.args.size() >= 3 &&
        op.args[2] == type_id &&
        op.args[1] == (uint32_t)storage_class) {
      return Id(op.args[0]);
    }
  }

  Id pointer_type_id = allocate_id();
  add_declaration(Instruction(spv::OpTypePointer,
    {pointer_type_id, (uint32_t)storage_class, type_id}));
  return pointer_type_id;
}

/**
 * Ensures that a pointer type to the given type with the given storage class
 * is declared, and returns its id.
 */
SpirVId SpirVModule::
define_pointer_type(const ShaderType *type, spv::StorageClass storage_class) {
  return define_pointer_type(define_type(type), storage_class);
}

/**
 * Ensures that a function type with the given return type and parameter type
 * ids is declared, and returns its id.
 */
SpirVId SpirVModule::
define_function_type(const ShaderType *return_type,
                     const pvector<Id> &param_type_ids) {
  Id return_type_id = define_type(return_type);

  for (const Instruction &op : _declarations) {
    if (op.opcode == spv::OpTypeFunction &&
        op.args.size() == 2 + param_type_ids.size() &&
        op.args[1] == return_type_id &&
        std::equal(param_type_ids.begin(), param_type_ids.end(),
                   op.args.begin() + 2)) {
      return Id(op.args[0]);
    }
  }

  Id function_type_id = allocate_id();
  Instruction op(spv::OpTypeFunction, {function_type_id, return_type_id});
  op.args.insert(op.args.end(), param_type_ids.begin(), param_type_ids.end());
  add_declaration(std::move(op));
  return function_type_id;
}

/**
 * Adds a new module-scope variable declaration, along with any type
 * declarations and layout annotations that may be necessary, and returns its
 * id.
 */
SpirVId SpirVModule::
define_variable(const ShaderType *type, spv::StorageClass storage_class) {
  Id pointer_type_id = define_pointer_type(type, storage_class);
  Id variable_id = allocate_id();

  add_declaration(Instruction(spv::OpVariable, {
    pointer_type_id,
    variable_id,
    (uint32_t)storage_class,
  }));

  // Depending on the storage class, we may need to make sure it is laid out.
  if (storage_class == spv::StorageClassStorageBuffer ||
      storage_class == spv::StorageClassPhysicalStorageBuffer ||
      storage_class == spv::StorageClassUniform ||
      storage_class == spv::StorageClassPushConstant) {
    r_annotate_struct_layout(unwrap_pointer_type(pointer_type_id));
  }

  return variable_id;
}

/**
 * Defines a new constant.  Does not attempt to reuse constants.
 */
SpirVId SpirVModule::
define_constant(const ShaderType *type, uint32_t constant) {
  Id type_id = define_type(type);

  Id constant_id = allocate_id();
  if (type == ShaderType::BOOL) {
    add_declaration(Instruction(constant ? spv::OpConstantTrue : spv::OpConstantFalse,
                                {type_id, constant_id}));
  } else {
    add_declaration(Instruction(spv::OpConstant, {type_id, constant_id, constant}));
  }
  return constant_id;
}

/**
 * Defines a new constant of type double.  Does not attempt to reuse
 * constants.
 */
SpirVId SpirVModule::
define_double_constant(double constant) {
  Id type_id = define_type(ShaderType::DOUBLE);

  uint64_t bits;
  memcpy(&bits, &constant, sizeof(bits));

  // 64-bit literals are stored low-order word first.
  Id constant_id = allocate_id();
  add_declaration(Instruction(spv::OpConstant,
    {type_id, constant_id, (uint32_t)bits, (uint32_t)(bits >> 32)}));
  return constant_id;
}

/**
 * Defines a new integral constant, either of type uint or int, reusing an
 * existing one if one is already defined (except for OpConstantNull, which
 * can't be used to index structure members).
 */
SpirVId SpirVModule::
define_int_constant(int32_t constant) {
  for (const Instruction &op : _declarations) {
    if (op.opcode != spv::OpConstant || op.args.size() != 3 ||
        op.args[2] != (uint32_t)constant) {
      continue;
    }
    // Skip constants of a non-canonical type declaration (eg. a 16-bit int),
    // which cannot be reused interchangeably with a plain int constant.
    const Instruction *type_decl = find_declaration(Id(op.args[0]));
    if (type_decl != nullptr && type_decl->opcode == spv::OpTypeInt &&
        type_decl->args.size() == 3 && type_decl->args[1] == 32) {
      if (type_decl->args[2] != 0 || constant >= 0) {
        return Id(op.args[1]);
      }
    }
  }

  Id type_id = define_type(ShaderType::INT);
  Id constant_id = allocate_id();

  add_declaration(Instruction(spv::OpConstant, {type_id, constant_id, (uint32_t)constant}));
  return constant_id;
}

/**
 * Defines a new null constant of the given type, reusing an existing one if
 * one is already defined.
 */
SpirVId SpirVModule::
define_null_constant(const ShaderType *type) {
  Id type_id = define_type(type);

  for (const Instruction &op : _declarations) {
    if (op.opcode == spv::OpConstantNull && op.args.size() >= 2 &&
        op.args[0] == type_id) {
      return Id(op.args[1]);
    }
  }

  Id constant_id = allocate_id();
  add_declaration(Instruction(spv::OpConstantNull, {type_id, constant_id}));
  return constant_id;
}

/**
 * Defines a new spec constant.  Follow up with a call to decorate() to set
 * the SpecId.
 */
SpirVId SpirVModule::
define_spec_constant(const ShaderType *type, uint32_t def_value) {
  Id type_id = define_type(type);

  Id id = allocate_id();
  if (type == ShaderType::BOOL) {
    add_declaration(Instruction(def_value ? spv::OpSpecConstantTrue : spv::OpSpecConstantFalse, {type_id, id}));
  } else {
    add_declaration(Instruction(spv::OpSpecConstant, {type_id, id, def_value}));
  }
  return id;
}

/**
 * Deletes the given id: removes its declaration, name and annotations, and
 * clears its index entry.  Takes effect immediately.
 *
 * References to the id in function bodies are the pass's responsibility
 * (see delete_dead_code for the common cascade); deletions do not cascade to
 * dependent declarations automatically.
 */
void SpirVModule::
delete_id(Id id) {
  // Copied, since clearing the state of contained ids may grow the index.
  const Definition def = get_definition(id);

  if (def._dtype == DT_function) {
    // Remove the entire function, clearing the state of everything declared
    // inside it.
    for (size_t fi = 0; fi < _functions.size(); ++fi) {
      if (_functions[fi].id != id) {
        continue;
      }
      for (Id param_id : _functions[fi].parameters) {
        clear_id_state(param_id);
      }
      for (const Instruction &op : _functions[fi].instructions) {
        Id result_id = op.get_result();
        if (result_id != 0 && result_id != id) {
          clear_id_state(result_id);
        }
      }
      _functions.erase(_functions.begin() + fi);
      break;
    }

    // Also drop any execution modes and entry points that refer to it.
    for (size_t i = _execution_modes.size(); i-- > 0;) {
      if (!_execution_modes[i].args.empty() && _execution_modes[i].args[0] == id) {
        _execution_modes.erase(_execution_modes.begin() + i);
      }
    }
    for (size_t i = _entry_points.size(); i-- > 0;) {
      if (_entry_points[i].function_id == id) {
        _entry_points.erase(_entry_points.begin() + i);
      }
    }
  }
  else if (def._function_id != 0) {
    // Defined inside a function.
    Function *function = find_function(def._function_id);
    if (function != nullptr) {
      if (def._dtype == DT_function_parameter) {
        for (size_t pi = function->parameters.size(); pi-- > 0;) {
          if (function->parameters[pi] == id) {
            function->parameters.erase(function->parameters.begin() + pi);
            break;
          }
        }
      } else {
        for (Instruction &op : function->instructions) {
          if (!op.is_nop() && op.get_result() == id) {
            op = Instruction(spv::OpNop);
            break;
          }
        }
      }
    }
  }
  else if (def._dtype == DT_ext_inst) {
    for (size_t i = _ext_inst_imports.size(); i-- > 0;) {
      if (!_ext_inst_imports[i].args.empty() && _ext_inst_imports[i].args[0] == id) {
        _ext_inst_imports.erase(_ext_inst_imports.begin() + i);
      }
    }
  }
  else if (def._dtype == DT_string) {
    for (size_t i = _debug.size(); i-- > 0;) {
      if (_debug[i].opcode == spv::OpString &&
          !_debug[i].args.empty() && _debug[i].args[0] == id) {
        _debug.erase(_debug.begin() + i);
      }
    }
  }
  else {
    Instruction *decl = find_declaration(id);
    if (decl != nullptr) {
      *decl = Instruction(spv::OpNop);
    }
  }

  if (is_type(id)) {
    // The reuse map may hand out the deleted id; rebuild it on next lookup.
    // Resolutions cached on other ids stay valid: a live declaration may
    // not reference a deleted one.
    _type_map.clear();
    _type_map_valid = false;
  }

  clear_id_state(id);
}

/**
 * Redirects all references to the given type id to the given other type id,
 * and removes the old declaration.  Takes effect immediately: subsequent
 * reads of the module see the new id everywhere.
 *
 * May only be called for type ids (including pointer and function types).
 * The literal payload of a constant is encoded according to its type, so a
 * pass that replaces a type with a differently laid out one must rewrite the
 * constants of that type itself.
 *
 * There is deliberately no general replace_id for value ids: those can be
 * forward-referenced (eg. by OpPhi) and appear in operand positions that
 * cannot be distinguished from literals without a full grammar; a pass that
 * replaces values rewrites the affected instructions itself.
 */
void SpirVModule::
replace_type_id(Id before, Id after) {
  nassertv(before != 0u && after != 0u);
  if (before == after) {
    return;
  }
  nassertv(is_type(before));
  nassertv(is_type(after));

  rewrite_type_references(before, after);

  Instruction *decl = find_declaration(before);
  if (decl != nullptr) {
    *decl = Instruction(spv::OpNop);
  }

  clear_id_state(before);

  // The replacement may resolve differently (eg. a different depth flag), so
  // every cached resolution that could embed it is suspect.
  invalidate_types();
}

/**
 * Deletes the given member (by current index) from the given struct type
 * declaration: the OpTypeStruct operand, the member's name and its member
 * decorations are all removed, and higher member indices are renumbered.
 *
 * Access chains indexing into the struct are the pass's responsibility.
 */
void SpirVModule::
delete_struct_member(Id type_id, uint32_t member_index) {
  Instruction *decl = find_declaration(type_id);
  nassertv(decl != nullptr && decl->opcode == spv::OpTypeStruct);
  nassertv(member_index + 1 < decl->args.size());
  decl->args.erase(decl->args.begin() + 1 + member_index);

  auto nit = _member_names.find(type_id);
  if (nit != _member_names.end() && member_index < nit->second.size()) {
    nit->second.erase(nit->second.begin() + member_index);
  }

  auto it = _annotations.find(type_id);
  if (it != _annotations.end()) {
    Annotations &annotations = it->second;
    for (size_t i = annotations.size(); i-- > 0;) {
      Annotation &annotation = annotations[i];
      if (annotation.is_member_annotation() && !annotation.args.empty()) {
        if (annotation.args[0] == member_index) {
          annotations.erase(annotations.begin() + i);
        }
        else if (annotation.args[0] > member_index) {
          --annotation.args[0];
        }
      }
    }
  }

  invalidate_types();
}

/**
 * Removes the given parameters (by index) from the given function type: the
 * OpTypeFunction declaration is rebuilt without them, the matching
 * parameter ids of every function of this type are removed, and the matching
 * arguments are stripped from every call to such a function.  Body
 * instructions based on a removed pointer parameter (access chains, pointer
 * copies) are deleted via delete_dead_code().
 *
 * If this leaves the declaration identical to another function type, the two
 * should be merged by deduplicate_types() afterwards.
 */
void SpirVModule::
remove_function_parameters(Id type_id, const pset<uint32_t> &param_indices) {
  if (param_indices.empty()) {
    return;
  }

  Instruction *decl = find_declaration(type_id);
  nassertv(decl != nullptr && decl->opcode == spv::OpTypeFunction);

  for (auto it = param_indices.rbegin(); it != param_indices.rend(); ++it) {
    uint32_t index = *it;
    nassertd(index + 2 < decl->args.size()) continue;
    decl->args.erase(decl->args.begin() + 2 + index);
  }

  // Remove the matching parameter ids from every function of this type, and
  // strip the matching arguments from every call to such a function.
  pset<uint32_t> affected_functions;
  for (uint32_t id = 0; id < _defs.size(); ++id) {
    const Definition &def = _defs[id];
    if (def._dtype == DT_function && def._type_id == type_id) {
      affected_functions.insert(id);
    }
  }

  pset<Id> dead_ids;
  for (Function &function : _functions) {
    if (affected_functions.count(function.id)) {
      for (auto it = param_indices.rbegin(); it != param_indices.rend(); ++it) {
        uint32_t param_index = *it;
        nassertd(param_index < function.parameters.size()) continue;
        Id param_id = function.parameters[param_index];
        function.parameters.erase(function.parameters.begin() + param_index);
        clear_id_state(param_id);
        dead_ids.insert(param_id);
      }
    }

    for (Instruction &op : function.instructions) {
      if (op.opcode == spv::OpFunctionCall && op.args.size() >= 3 &&
          affected_functions.count(op.args[2])) {
        for (auto it = param_indices.rbegin(); it != param_indices.rend(); ++it) {
          size_t arg_index = 3 + *it;
          nassertd(arg_index < op.args.size()) continue;
          op.args.erase(op.args.begin() + arg_index);
        }
      }
    }
  }

  // A removed pointer parameter may still be the base of access chains and
  // pointer copies in the function body (which never marked it used); delete
  // those and cascade to anything based on their results.
  delete_dead_code(dead_ids);
}

/**
 * Removes function-body instructions that reference the given dead ids:
 * access chains, pointer copies, casts, loads and image handle extractions
 * based on a dead id are removed and their results added to the dead set
 * (cascading forward, which suffices since SSA order puts uses after
 * definitions), while a store to or atomic on a dead id triggers an
 * assertion, since the id evidently was not dead.  Instructions whose own
 * result id is in the set are removed as well.
 */
void SpirVModule::
delete_dead_code(pset<Id> &dead_ids) {
  if (dead_ids.empty()) {
    return;
  }

  for (Function &function : _functions) {
    for (Instruction &op : function.instructions) {
      bool remove = false;
      switch (op.opcode) {
      case spv::OpNop:
        continue;

      case spv::OpAccessChain:
      case spv::OpInBoundsAccessChain:
      case spv::OpPtrAccessChain:
      case spv::OpInBoundsPtrAccessChain:
      case spv::OpImageTexelPointer:
      case spv::OpCopyObject:
      case spv::OpExpectKHR:
      case spv::OpBitcast:
      case spv::OpCopyLogical:
      case spv::OpLoad:
      case spv::OpImage:
        remove = op.args.size() >= 3 && dead_ids.count(Id(op.args[2])) != 0;
        break;

      case spv::OpSampledImage:
        remove = (op.args.size() >= 3 && dead_ids.count(Id(op.args[2])) != 0) ||
                 (op.args.size() >= 4 && dead_ids.count(Id(op.args[3])) != 0);
        break;

      case spv::OpAtomicLoad:
      case spv::OpAtomicExchange:
      case spv::OpAtomicCompareExchange:
      case spv::OpAtomicCompareExchangeWeak:
      case spv::OpAtomicIIncrement:
      case spv::OpAtomicIDecrement:
      case spv::OpAtomicIAdd:
      case spv::OpAtomicISub:
      case spv::OpAtomicSMin:
      case spv::OpAtomicUMin:
      case spv::OpAtomicSMax:
      case spv::OpAtomicUMax:
      case spv::OpAtomicAnd:
      case spv::OpAtomicOr:
      case spv::OpAtomicXor:
      case spv::OpAtomicFlagTestAndSet:
      case spv::OpAtomicFMinEXT:
      case spv::OpAtomicFMaxEXT:
      case spv::OpAtomicFAddEXT:
        nassertd(op.args.size() < 3 || !dead_ids.count(Id(op.args[2]))) {}
        break;

      case spv::OpStore:
      case spv::OpAtomicStore:
        nassertd(op.args.empty() || !dead_ids.count(Id(op.args[0]))) {}
        break;

      case spv::OpCopyMemory:
      case spv::OpCopyMemorySized:
        nassertd(op.args.size() < 2 || (!dead_ids.count(Id(op.args[0])) &&
                                        !dead_ids.count(Id(op.args[1])))) {}
        break;

      default:
        break;
      }

      Id result_id = op.get_result();
      if (!remove && result_id != 0 && dead_ids.count(result_id)) {
        remove = true;
      }

      if (remove) {
        if (result_id != 0) {
          dead_ids.insert(result_id);
          clear_id_state(result_id);
        }
        op = Instruction(spv::OpNop);
      }
    }
  }
}

/**
 * Merges duplicate declarations of types that SPIR-V requires to be unique
 * within a module.  A later declaration is merged onto the earlier identical
 * one; all references are rewritten.  Aggregate types (structs and arrays)
 * are exempt, since SPIR-V permits them to be duplicated; pointer types are
 * only merged when they carry no annotations at all (see below).
 *
 * SpirVTransformer::run calls this after every pass; passes that run outside
 * the transformer (or tests) may call it directly before emit().
 */
void SpirVModule::
deduplicate_types() {
  pmap<pvector<uint32_t>, uint32_t> seen;

  // Visit the declarations in topological order rather than section order:
  // a replacement type appended at the end of the section is then visited
  // before the declarations that reference it, so that merges cascade
  // through dependent types (sampled image, pointer, function types) in a
  // single pass.
  for (size_t index : sort_declarations()) {
    Instruction &op = _declarations[index];
    switch (op.opcode) {
    case spv::OpTypeVoid:
    case spv::OpTypeBool:
    case spv::OpTypeInt:
    case spv::OpTypeFloat:
    case spv::OpTypeVector:
    case spv::OpTypeMatrix:
    case spv::OpTypeImage:
    case spv::OpTypeSampler:
    case spv::OpTypeSampledImage:
    case spv::OpTypeOpaque:
    case spv::OpTypeFunction:
    case spv::OpTypeEvent:
    case spv::OpTypeDeviceEvent:
    case spv::OpTypeReserveId:
    case spv::OpTypeQueue:
    case spv::OpTypePipe:
    case spv::OpTypeCooperativeMatrixKHR:
    case spv::OpTypeCooperativeMatrixNV:
    case spv::OpTypeAccelerationStructureKHR:
    case spv::OpTypeRayQueryKHR:
    case spv::OpTypeHitObjectNV:
    case spv::OpTypeHitObjectEXT:
      break;

    // Pointer types are exempt from the uniqueness rule, specifically so
    // that operand-identical pointers may be decorated differently.  However,
    // it is still useful to merge them, so that the merge can cascade into the
    // function types referencing them, which the uniqueness rule does cover.
    // In practice, very few pointers carry annotations, so we simply apply
    // deduping only to pointers without annotations.
    case spv::OpTypePointer:
      {
        const Annotations *annotations = get_annotations(Id(op.args[0]));
        if (annotations != nullptr && !annotations->empty()) {
          continue;
        }
      }
      break;

    default:
      // Not a type, or not required to be unique.
      continue;
    }

    // Do not be tempted to exempt the other declarations from merging based
    // on their decorations, the way pointer types are: the uniqueness rule
    // applies to them no matter how they are decorated, so the decorations of
    // the merged-away id are dropped deliberately.
    pvector<uint32_t> key;
    key.reserve(op.args.size());
    key.push_back((uint32_t)op.opcode);
    key.insert(key.end(), op.args.begin() + 1, op.args.end());

    auto result = seen.insert(std::make_pair(std::move(key), op.args[0]));
    if (!result.second && result.first->second != op.args[0]) {
      // An identical declaration already exists; merge this one onto it.
      // This may in turn make downstream declarations identical, which the
      // rest of this scan picks up, since the rewrite has already been
      // applied by the time they are visited.
      replace_type_id(Id(op.args[0]), Id(result.first->second));
    }
  }
}

/**
 * Reassigns the result id of the given module-scope declaration, moving its
 * index entry along.  Names and annotations stay with the old id; the caller
 * transfers or reassigns those as appropriate.
 */
void SpirVModule::
reassign_declaration_id(Id old_id, Id new_id) {
  Definition def = get_definition(old_id);
  nassertv(def._declaration_index >= 0 && (size_t)def._declaration_index < _declarations.size());
  nassertv(new_id != 0 && new_id < _id_bound);

  Instruction &op = _declarations[def._declaration_index];
  nassertv(op.get_result() == old_id);
  op.args[op.has_result_type() ? 1 : 0] = new_id;

  modify_definition(new_id) = def;
  modify_definition(old_id) = Definition();

  if (is_type(new_id)) {
    _type_map.clear();
    _type_map_valid = false;
  }
}

/**
 * Implementation of record_result, with an explicit declaration index.
 */
void SpirVModule::
record_result_at(const Instruction &op, Id function_id,
                 int32_t declaration_index) {
  nassertv(function_id == Id() || declaration_index == -1);

  Id result_id = op.get_result();
  if (result_id == 0) {
    nassertv(!op.has_result());
    return;
  }

  Definition &def = modify_definition(result_id);
  def = Definition();
  def._function_id = function_id;
  def._declaration_index = declaration_index;

  switch (op.opcode) {
  case spv::OpTypeVoid:
  case spv::OpTypeBool:
  case spv::OpTypeInt:
  case spv::OpTypeFloat:
  case spv::OpTypeVector:
  case spv::OpTypeMatrix:
  case spv::OpTypeImage:
  case spv::OpTypeSampler:
  case spv::OpTypeSampledImage:
  case spv::OpTypeArray:
  case spv::OpTypeRuntimeArray:
  case spv::OpTypeStruct:
  case spv::OpTypeOpaque:
  case spv::OpTypeEvent:
  case spv::OpTypeDeviceEvent:
  case spv::OpTypeReserveId:
  case spv::OpTypeQueue:
  case spv::OpTypePipe:
  case spv::OpTypePipeStorage:
  case spv::OpTypeNamedBarrier:
  case spv::OpTypeCooperativeMatrixKHR:
  case spv::OpTypeCooperativeMatrixNV:
  case spv::OpTypeAccelerationStructureKHR:
  case spv::OpTypeRayQueryKHR:
  case spv::OpTypeHitObjectNV:
  case spv::OpTypeHitObjectEXT:
    def._dtype = DT_type;
    break;

  case spv::OpTypePointer:
    def._dtype = DT_pointer_type;
    break;

  case spv::OpTypeFunction:
    def._dtype = DT_function_type;
    break;

  case spv::OpVariable:
    def._dtype = DT_variable;
    def._type_id = Id(op.args[0]);
    break;

  case spv::OpConstant:
  case spv::OpConstantTrue:
  case spv::OpConstantFalse:
  case spv::OpConstantNull:
  case spv::OpConstantComposite:
  case spv::OpSpecConstantComposite:
    def._dtype = DT_constant;
    def._type_id = Id(op.args[0]);
    break;

  case spv::OpSpecConstant:
  case spv::OpSpecConstantTrue:
  case spv::OpSpecConstantFalse:
  // A spec constant expression too: its value is not knowable here
  // (resolve_constant returns 0 for it), but it is a module-scope constant,
  // notably usable as an OpTypeArray length and as a constant expression.
  case spv::OpSpecConstantOp:
    def._dtype = DT_spec_constant;
    def._type_id = Id(op.args[0]);
    break;

  case spv::OpFunction:
    def._dtype = DT_function;
    def._type_id = Id((op.args.size() >= 4) ? op.args[3] : 0);
    def._function_id = result_id;
    break;

  case spv::OpFunctionParameter:
    def._dtype = DT_function_parameter;
    def._type_id = Id(op.args[0]);
    break;

  case spv::OpExtInstImport:
    def._dtype = DT_ext_inst;
    break;

  case spv::OpString:
    def._dtype = DT_string;
    break;

  default:
    if (op.has_result_type()) {
      def._dtype = DT_temporary;
      def._type_id = Id(op.args[0]);
    } else {
      // Results without a type (labels, decoration groups): there is nothing
      // further to index, but DT_typeless distinguishes a live id from a
      // cleared or never-assigned one (DT_none).
      def._dtype = DT_typeless;
    }
    break;
  }
}

/**
 * Clears every trace of the given id from the index, names and annotations.
 * The declaring instruction itself is the caller's to remove.
 */
void SpirVModule::
clear_id_state(Id id) {
  _names.erase(id);
  _member_names.erase(id);
  _annotations.erase(id);
  modify_definition(id) = Definition();
}

/**
 * Recursive helper for resolve_type; id must be a non-pointer type id.
 */
const ShaderType *SpirVModule::
r_resolve_type(Id id) const {
  const Definition &def = get_definition(id);
  if (def._type != nullptr) {
    return def._type;
  }
  const Instruction *decl = find_declaration(id);
  if (decl == nullptr) {
    return nullptr;
  }

  const ShaderType *type = nullptr;
  switch (decl->opcode) {
  case spv::OpTypeVoid:
    // Never cached; a null cache slot means "not resolved yet".
    return nullptr;

  case spv::OpTypeBool:
    type = ShaderType::BOOL;
    break;

  case spv::OpTypeInt:
    type = decl->args[2] ? ShaderType::INT : ShaderType::UINT;
    break;

  case spv::OpTypeFloat:
    type = (decl->args.size() >= 2 && decl->args[1] >= 64)
      ? ShaderType::DOUBLE : ShaderType::FLOAT;
    break;

  case spv::OpTypeVector:
    {
      const ShaderType *component = r_resolve_type(Id(decl->args[1]));
      const ShaderType::Scalar *scalar = (component != nullptr) ? component->as_scalar() : nullptr;
      nassertr(scalar != nullptr, nullptr);
      type = ShaderType::register_type(
        ShaderType::Vector(scalar->get_scalar_type(), decl->args[2]));
    }
    break;

  case spv::OpTypeMatrix:
    {
      // SPIR-V uses GLSL parlance, in which a column is a row and a row is a
      // column, compared to Panda conventions.  We flip it around here.
      const ShaderType *row = r_resolve_type(Id(decl->args[1]));
      const ShaderType::Vector *row_type = (row != nullptr) ? row->as_vector() : nullptr;
      nassertr(row_type != nullptr, nullptr);
      type = ShaderType::register_type(
        ShaderType::Matrix(row_type->get_scalar_type(), decl->args[2],
                           row_type->get_num_components()));
    }
    break;

  case spv::OpTypeSampler:
    type = ShaderType::SAMPLER;
    break;

  case spv::OpTypeImage:
    {
      const ShaderType *sampled = r_resolve_type(Id(decl->args[1]));
      const ShaderType::Scalar *scalar = (sampled != nullptr) ? sampled->as_scalar() : nullptr;
      nassertr(scalar != nullptr, nullptr);

      Texture::TextureType texture_type;
      switch ((spv::Dim)decl->args[2]) {
      case spv::Dim1D:
        texture_type = decl->args[4] ? Texture::TT_1d_texture_array
                                     : Texture::TT_1d_texture;
        break;
      case spv::Dim2D:
        texture_type = decl->args[4] ? Texture::TT_2d_texture_array
                                     : Texture::TT_2d_texture;
        break;
      case spv::Dim3D:
        texture_type = Texture::TT_3d_texture;
        break;
      case spv::DimCube:
        texture_type = decl->args[4] ? Texture::TT_cube_map_array
                                     : Texture::TT_cube_map;
        break;
      case spv::DimBuffer:
        texture_type = Texture::TT_buffer_texture;
        break;
      case spv::DimRect:
        shader_cat.error()
          << "imageRect shader inputs are not supported.\n";
        return nullptr;
      case spv::DimSubpassData:
        shader_cat.error()
          << "subpassInput shader inputs are not supported.\n";
        return nullptr;
      default:
        shader_cat.error()
          << "Unknown image dimensionality in OpTypeImage instruction.\n";
        return nullptr;
      }

      ShaderType::Access access = ShaderType::Access::READ_WRITE;
      if (decl->args.size() > 8) {
        switch ((spv::AccessQualifier)decl->args[8]) {
        case spv::AccessQualifierReadOnly:
          access = ShaderType::Access::READ_ONLY;
          break;
        case spv::AccessQualifierWriteOnly:
          access = ShaderType::Access::WRITE_ONLY;
          break;
        case spv::AccessQualifierReadWrite:
          access = ShaderType::Access::READ_WRITE;
          break;
        default:
          shader_cat.error()
            << "Invalid access qualifier in OpTypeImage instruction.\n";
          break;
        }
      }
      if (has_decoration(id, spv::DecorationNonWritable)) {
        access = (access & ShaderType::Access::READ_ONLY);
      }
      if (has_decoration(id, spv::DecorationNonReadable)) {
        access = (access & ShaderType::Access::WRITE_ONLY);
      }

      type = ShaderType::register_type(
        ShaderType::Image(texture_type, scalar->get_scalar_type(), access));
    }
    break;

  case spv::OpTypeSampledImage:
    {
      const ShaderType *image_shader_type = r_resolve_type(Id(decl->args[1]));
      const ShaderType::Image *image = (image_shader_type != nullptr)
        ? image_shader_type->as_image() : nullptr;
      if (image == nullptr) {
        shader_cat.error()
          << "OpTypeSampledImage must refer to an image type!\n";
        return nullptr;
      }
      // The image's depth flag determines shadow sampler status.
      const Instruction *image_decl = find_declaration(Id(decl->args[1]));
      bool shadow = (image_decl != nullptr && image_decl->args.size() > 3 &&
                     image_decl->args[3] == 1);
      type = ShaderType::register_type(
        ShaderType::SampledImage(image->get_texture_type(),
                                 image->get_sampled_type(), shadow));
    }
    break;

  case spv::OpTypeArray:
    {
      const ShaderType *element = r_resolve_type(Id(decl->args[1]));
      if (element == nullptr) {
        return nullptr;
      }
      type = ShaderType::register_type(ShaderType::Array(
        element, resolve_constant(Id(decl->args[2])), get_array_stride(id)));
    }
    break;

  case spv::OpTypeRuntimeArray:
    {
      const ShaderType *element = r_resolve_type(Id(decl->args[1]));
      if (element == nullptr) {
        return nullptr;
      }
      type = ShaderType::register_type(ShaderType::Array(
        element, 0, get_array_stride(id)));
    }
    break;

  case spv::OpTypeStruct:
    {
      ShaderType::Struct struct_type;
      for (size_t ai = 1; ai < decl->args.size(); ++ai) {
        uint32_t member_index = (uint32_t)(ai - 1);
        Id member_type_id(decl->args[ai]);

        if (get_member_builtin(id, member_index) != spv::BuiltInMax) {
          // Built-in members are not part of the Panda type.
          continue;
        }

        const ShaderType *member_type = r_resolve_type(member_type_id);
        if (member_type == nullptr) {
          shader_cat.error()
            << "Struct type with id " << id
            << " contains invalid member type " << member_type_id << "\n";
          return nullptr;
        }
        member_type = apply_member_decorations(id, member_index, member_type);

        const std::string &name = get_member_name(id, member_index);
        int offset = get_member_offset(id, member_index);
        if (offset >= 0) {
          struct_type.add_member(member_type, name, (uint32_t)offset);
        } else {
          struct_type.add_member(member_type, name);
        }
      }
      type = ShaderType::register_type(std::move(struct_type));
    }
    break;

  default:
    // Function types, pointer types and unsupported types do not resolve.
    return nullptr;
  }

  def._type = type;
  return type;
}

/**
 * Applies the readonly/writeonly and matrix stride decorations of the given
 * struct member to its resolved type, recursing into arrays.
 */
const ShaderType *SpirVModule::
apply_member_decorations(Id type_id, uint32_t member_index, const ShaderType *type) const {
  bool non_writable = has_member_decoration(type_id, member_index, spv::DecorationNonWritable);
  bool non_readable = has_member_decoration(type_id, member_index, spv::DecorationNonReadable);
  if (non_writable || non_readable) {
    if (const ShaderType::Image *image = type->as_image()) {
      ShaderType::Access access = image->get_access();
      if (non_writable) {
        access = (access & ShaderType::Access::READ_ONLY);
      }
      if (non_readable) {
        access = (access & ShaderType::Access::WRITE_ONLY);
      }
      if (access != image->get_access()) {
        return ShaderType::register_type(ShaderType::Image(
          image->get_texture_type(),
          image->get_sampled_type(),
          access));
      }
    }
  }

  uint32_t matrix_stride = get_member_decoration(type_id, member_index, spv::DecorationMatrixStride, 0);
  if (matrix_stride > 0) {
    // Inject matrix stride into the matrix type.
    if (const ShaderType::Matrix *matrix = type->as_matrix()) {
      if (matrix->get_row_stride_bytes() != matrix_stride) {
        return ShaderType::register_type(ShaderType::Matrix(
          matrix->get_scalar_type(), matrix->get_num_rows(),
          matrix->get_num_columns(), matrix_stride));
      }
    }
  }

  if (const ShaderType::Array *array = type->as_array()) {
    const ShaderType *element_type = array->get_element_type();
    const ShaderType *new_element_type = apply_member_decorations(type_id, member_index, element_type);
    if (element_type != new_element_type) {
      return ShaderType::register_type(ShaderType::Array(
        new_element_type, array->get_num_elements(), array->get_stride_bytes()));
    }
  }

  return type;
}

/**
 * Computes whether the given struct type is effectively readonly and/or
 * writeonly: either decorated so itself, or with every non-builtin member so
 * decorated (glslang decorates the members of an SSBO rather than the block).
 */
void SpirVModule::
get_aggregate_access_flags(Id struct_id, bool &non_writable, bool &non_readable) const {
  non_writable = has_decoration(struct_id, spv::DecorationNonWritable);
  non_readable = has_decoration(struct_id, spv::DecorationNonReadable);
  if (non_writable && non_readable) {
    return;
  }

  bool members_non_writable = true;
  bool members_non_readable = true;
  uint32_t num_members = get_num_members(struct_id);
  for (uint32_t i = 0; i < num_members; ++i) {
    if (get_member_builtin(struct_id, i) != spv::BuiltInMax) {
      continue;
    }
    members_non_writable = members_non_writable &&
      has_member_decoration(struct_id, i, spv::DecorationNonWritable);
    members_non_readable = members_non_readable &&
      has_member_decoration(struct_id, i, spv::DecorationNonReadable);
  }
  non_writable = non_writable || members_non_writable;
  non_readable = non_readable || members_non_readable;
}

/**
 * Maps the Uniform storage class of a legacy-style buffer block (a struct,
 * possibly wrapped in arrays, carrying the BufferBlock decoration) to
 * StorageBuffer, the way newer SPIR-V declares it.
 */
spv::StorageClass SpirVModule::
canonicalize_storage_class(spv::StorageClass storage_class, Id pointee_type_id) const {
  if (storage_class != spv::StorageClassUniform || pointee_type_id == 0) {
    return storage_class;
  }
  const Instruction *decl = find_declaration(pointee_type_id);
  while (decl != nullptr && (decl->opcode == spv::OpTypeArray ||
                             decl->opcode == spv::OpTypeRuntimeArray)) {
    decl = find_declaration(Id(decl->args[1]));
  }
  if (decl != nullptr && decl->opcode == spv::OpTypeStruct &&
      has_decoration(Id(decl->args[0]), spv::DecorationBufferBlock)) {
    return spv::StorageClassStorageBuffer;
  }
  return storage_class;
}

/**
 * Rebuilds the type reuse map by visiting all type declarations in
 * topological order (so that a component type is classified before the types
 * that reference it) and entering those that are declared exactly the way
 * define_type() would declare them.
 */
void SpirVModule::
ensure_type_map() const {
  if (_type_map_valid) {
    return;
  }
  _type_map_valid = true;
  _type_map.clear();

  for (size_t index : sort_declarations()) {
    const Instruction &op = _declarations[index];
    switch (op.opcode) {
    case spv::OpTypeVoid:
    case spv::OpTypeBool:
    case spv::OpTypeInt:
    case spv::OpTypeFloat:
    case spv::OpTypeVector:
    case spv::OpTypeMatrix:
    case spv::OpTypeImage:
    case spv::OpTypeSampler:
    case spv::OpTypeSampledImage:
    case spv::OpTypeArray:
    case spv::OpTypeRuntimeArray:
    case spv::OpTypeStruct:
      break;
    default:
      continue;
    }

    Id id(op.args[0]);
    const ShaderType *type = r_resolve_type(id);
    if (type == nullptr && op.opcode != spv::OpTypeVoid) {
      continue;
    }
    if (has_builtin_members(id)) {
      // Only put types we can fully round-trip in the type map.
      continue;
    }
    if (is_canonical_type_decl(op)) {
      _type_map[type] = id;
    }
  }
}

/**
 * Returns true if the given type declaration is exactly what define_type()
 * would emit for its resolved ShaderType (eg. not a 16-bit float, not a
 * multisampled image).  Only such declarations enter the type reuse map.
 * Assumes the map has been seeded up to this declaration in topological
 * order, so that component canonicality can be checked through the map.
 */
bool SpirVModule::
is_canonical_type_decl(const Instruction &op) const {
  // Checks whether the given (already visited) component type declaration is
  // the canonical declaration for its ShaderType.
  auto canonical_component = [this](uint32_t component_word) {
    Id component_id(component_word);
    const ShaderType *component = r_resolve_type(component_id);
    if (component == nullptr) {
      return false;
    }
    TypeMap::const_iterator tit = _type_map.find(component);
    return tit != _type_map.end() && tit->second == component_id;
  };

  switch (op.opcode) {
  case spv::OpTypeVoid:
  case spv::OpTypeBool:
  case spv::OpTypeSampler:
    return op.args.size() == 1;

  case spv::OpTypeInt:
    return op.args.size() == 3 && op.args[1] == 32;

  case spv::OpTypeFloat:
    return op.args.size() == 2 && (op.args[1] == 32 || op.args[1] == 64);

  case spv::OpTypeVector:
  case spv::OpTypeMatrix:
  case spv::OpTypeRuntimeArray:
    return canonical_component(op.args[1]);

  case spv::OpTypeArray:
    {
      // The length must be a plain OpConstant; a specialization constant
      // makes the array size specialization-dependent, so the declaration
      // cannot stand in for the fixed-size array type it resolves to.
      const Instruction *length_decl = find_declaration(Id(op.args[2]));
      return length_decl != nullptr &&
             length_decl->opcode == spv::OpConstant &&
             canonical_component(op.args[1]);
    }

  case spv::OpTypeImage:
    // Unspecified depth, storage image, unknown format, as define_type()
    // declares images (the access qualifier operand is part of the resolved
    // type, so its presence does not affect canonicality).
    return op.args.size() >= 8 &&
           op.args[3] == 2 && op.args[5] == 0 && op.args[6] == 2 &&
           op.args[7] == (uint32_t)spv::ImageFormatUnknown &&
           canonical_component(op.args[1]);

  case spv::OpTypeSampledImage:
    {
      // Canonical only if the image is declared exactly the way define_type()
      // would declare it for this sampled image type: sampled, single-sample,
      // known depthness, unknown format.
      const Instruction *image_decl = find_declaration(Id(op.args[1]));
      return image_decl != nullptr && image_decl->opcode == spv::OpTypeImage &&
             image_decl->args.size() >= 8 &&
             image_decl->args[6] == 1 && image_decl->args[5] == 0 &&
             (image_decl->args[3] == 0 || image_decl->args[3] == 1) &&
             image_decl->args[7] == (uint32_t)spv::ImageFormatUnknown;
    }

  case spv::OpTypeStruct:
    for (size_t ai = 1; ai < op.args.size(); ++ai) {
      if (get_member_builtin(Id(op.args[0]), (uint32_t)(ai - 1)) != spv::BuiltInMax) {
        continue;
      }
      if (!canonical_component(op.args[ai])) {
        return false;
      }
    }
    return true;

  default:
    return false;
  }
}

/**
 * Returns true if the given id or any of its struct members carry the
 * BuiltIn decoration.
 */
bool SpirVModule::
has_builtin_members(Id type_id) const {
  const Annotations *annotations = get_annotations(type_id);
  if (annotations == nullptr) {
    return false;
  }
  for (const Annotation &annotation : *annotations) {
    if (annotation.get_decoration() == spv::DecorationBuiltIn) {
      return true;
    }
  }
  return false;
}

/**
 * Makes sure that the given type has all its structure members correctly laid
 * out using offsets and strides.
 */
void SpirVModule::
r_annotate_struct_layout(Id type_id) {
  const Instruction *decl = find_declaration(type_id);
  nassertv(decl != nullptr);

  if (decl->opcode == spv::OpTypeArray || decl->opcode == spv::OpTypeRuntimeArray) {
    // Make sure there's an ArrayStride decoration for this array, then
    // recurse into the element type.
    if (get_array_stride(type_id) == 0) {
      const ShaderType *type = resolve_type(type_id);
      const ShaderType::Array *array_type = (type != nullptr) ? type->as_array() : nullptr;
      nassertv(array_type != nullptr);
      decorate(type_id, spv::DecorationArrayStride, array_type->get_stride_bytes());
    }
    r_annotate_struct_layout(Id(decl->args[1]));
    return;
  }

  if (decl->opcode != spv::OpTypeStruct) {
    return;
  }

  const ShaderType *type = resolve_type(type_id);
  const ShaderType::Struct *struct_type = (type != nullptr) ? type->as_struct() : nullptr;
  nassertv(struct_type != nullptr);

  uint32_t num_members = struct_type->get_num_members();

  for (uint32_t i = 0; i < num_members; ++i) {
    const ShaderType::Struct::Member &member = struct_type->get_member(i);

    if (get_member_offset(type_id, i) < 0) {
      decorate_member(type_id, i, spv::DecorationOffset, member.offset);
    }

    // Unwrap arrays to see if there's a matrix here, adding any missing array
    // stride decorations along the way.
    Id member_type_id = get_member_type_id(type_id, i);
    const ShaderType *base_type = member.type;
    while (const ShaderType::Array *array_type = base_type->as_array()) {
      base_type = array_type->get_element_type();

      // Also make sure there's an ArrayStride decoration for this array.
      if (get_array_stride(member_type_id) == 0) {
        decorate(member_type_id, spv::DecorationArrayStride, array_type->get_stride_bytes());
      }
      const Instruction *member_decl = find_declaration(member_type_id);
      nassertv(member_decl != nullptr && member_decl->args.size() >= 2);
      member_type_id = Id(member_decl->args[1]);
    }

    if (const ShaderType::Matrix *matrix_type = base_type->as_matrix()) {
      // Matrix types need to be explicitly laid out.
      if (get_member_decoration(type_id, i, spv::DecorationMatrixStride, 0) == 0) {
        decorate_member(type_id, i, spv::DecorationMatrixStride, matrix_type->get_row_stride_bytes());
        decorate_member(type_id, i, spv::DecorationColMajor);
      }
    } else {
      r_annotate_struct_layout(member_type_id);
    }
  }
}

/**
 * Appends the id operand indices of a MemoryAccess operand whose mask literal
 * sits at the given index, and stores the index just past the operand in
 * next_arg_index.  Returns false if an unknown mask bit makes the trailing
 * layout unclassifiable.  An Aligned literal follows the mask; pointer scopes
 * and aliasing-scope lists are ids.
 */
static bool
scan_memory_access_operands(const SpirVModule::Instruction &op,
                             size_t arg_index,
                             size_t &next_arg_index,
                             small_vector<uint16_t, 8> &indices) {
  if (arg_index >= op.args.size()) {
    next_arg_index = arg_index;
    return true;
  }
  uint32_t mask = op.args[arg_index];
  ++arg_index;

  const uint32_t known_mask =
    spv::MemoryAccessVolatileMask |
    spv::MemoryAccessAlignedMask |
    spv::MemoryAccessNontemporalMask |
    spv::MemoryAccessMakePointerAvailableMask |
    spv::MemoryAccessMakePointerVisibleMask |
    spv::MemoryAccessNonPrivatePointerMask |
    spv::MemoryAccessAliasScopeINTELMaskMask |
    spv::MemoryAccessNoAliasINTELMaskMask;
  if (mask & ~known_mask) {
    // A future mask bit may insert either a literal or an id anywhere among
    // the trailing operands.  Their layout is therefore unknown; include all
    // of them conservatively and prevent a caller from treating a following
    // word as a second MemoryAccess operand.
    for (size_t i = arg_index; i < op.args.size(); ++i) {
      indices.push_back((uint16_t)i);
    }
    next_arg_index = op.args.size();
    return false;
  }

  if (mask & spv::MemoryAccessAlignedMask) {
    ++arg_index;
  }
  if (mask & spv::MemoryAccessMakePointerAvailableMask) {
    if (arg_index < op.args.size()) {
      indices.push_back((uint16_t)arg_index);
    }
    ++arg_index;
  }
  if (mask & spv::MemoryAccessMakePointerVisibleMask) {
    if (arg_index < op.args.size()) {
      indices.push_back((uint16_t)arg_index);
    }
    ++arg_index;
  }
  if (mask & spv::MemoryAccessAliasScopeINTELMaskMask) {
    if (arg_index < op.args.size()) {
      indices.push_back((uint16_t)arg_index);
    }
    ++arg_index;
  }
  if (mask & spv::MemoryAccessNoAliasINTELMaskMask) {
    if (arg_index < op.args.size()) {
      indices.push_back((uint16_t)arg_index);
    }
    ++arg_index;
  }
  next_arg_index = arg_index;
  return true;
}

/**
 * Collects the indices of all operands of the given instruction that
 * reference an id (the result type counts as a reference; the result itself,
 * being a definition, does not).  Returns false if the operand layout cannot
 * be classified exactly, in which case every word that could plausibly be an
 * id was conservatively included.  Each consumer chooses its failure
 * direction on false:
 * over-approximate where a false negative is the danger (interface
 * derivation, usage analysis), skip or assert where a false positive is
 * (validation, id replacement).
 *
 * This is a member function because some opcodes (OpSwitch) need module
 * context to classify their operands.
 */
bool SpirVModule::
get_instruction_id_operands(const Instruction &op, small_vector<uint16_t, 8> &indices) const {
  indices.clear();

  size_t num_args = op.args.size();

  // A few instruction forms carry more than a fixed operand mask can
  // express; they are classified here, before the table.  OpEntryPoint is
  // deliberately not among them: parse() dissolves it into an EntryPoint
  // record and emit() re-derives its interface, so no consumer ever passes
  // one here, and a stray one gets the conservative fallback.
  switch (op.opcode) {
  case spv::OpSpecConstantOp:
    // Classify a synthetic instance of the nested instruction, supplying its
    // omitted result type and result operands.  This reuses the ordinary
    // opcode table for both all-id operations and operations with literal
    // tails (OpCompositeExtract, OpCompositeInsert and OpVectorShuffle), and
    // propagates conservative classification for an unknown nested opcode.
    if (num_args >= 1) {
      indices.push_back(0);
    }
    if (num_args >= 3) {
      spv::Op nested_opcode = (spv::Op)op.args[2];
      if (nested_opcode == spv::OpSpecConstantOp) {
        for (size_t i = 3; i < num_args; ++i) {
          indices.push_back((uint16_t)i);
        }
        return false;
      }

      Instruction nested(nested_opcode);
      nested.args.push_back(num_args >= 1 ? op.args[0] : 0);
      nested.args.push_back(0);  // Synthetic result id; never a reference.
      nested.args.insert(nested.args.end(), op.args.begin() + 3, op.args.end());

      small_vector<uint16_t, 8> nested_indices;
      bool known = get_instruction_id_operands(nested, nested_indices);
      for (uint16_t nested_index : nested_indices) {
        if (nested_index >= 2) {
          indices.push_back((uint16_t)(nested_index + 1));
        }
      }
      return known;
    }
    return true;

  case spv::OpGroupMemberDecorate:
    // The group id, then pairs of a target id and a literal member index.
    if (num_args >= 1) {
      indices.push_back(0);
      for (size_t i = 1; i + 1 < num_args; i += 2) {
        indices.push_back((uint16_t)i);
      }
    }
    return true;

  case spv::OpSwitch: {
    // The selector and default label, then pairs of a literal (as wide as
    // the selector type) and a case label.
    if (num_args >= 1) {
      indices.push_back(0);
    }
    if (num_args >= 2) {
      indices.push_back(1);
    }
    if (num_args <= 2) {
      return true;
    }
    const Instruction *type_decl = find_declaration(get_type_id(Id(op.args[0])));
    if (type_decl == nullptr || type_decl->opcode != spv::OpTypeInt ||
        type_decl->args.size() < 2 || type_decl->args[1] == 0) {
      // The selector type cannot be resolved, so the pair layout is unknown;
      // conservatively include every remaining word.
      for (size_t i = 2; i < num_args; ++i) {
        indices.push_back((uint16_t)i);
      }
      return false;
    }
    size_t literal_words = (type_decl->args[1] + 31) / 32;
    for (size_t i = 2; i + literal_words < num_args; i += literal_words + 1) {
      indices.push_back((uint16_t)(i + literal_words));
    }
    return true;
  }

  default:
    break;
  }

  const InstructionInfo *info = find_instruction_info(op.opcode);
  if (info != nullptr) {
    uint16_t mask = info->_id_mask;
    for (size_t i = 0; i < num_args && i < 15; ++i) {
      if (mask & (1u << i)) {
        indices.push_back((uint16_t)i);
      }
    }
    if (mask & 0x8000u) {
      for (size_t i = 15; i < num_args; ++i) {
        indices.push_back((uint16_t)i);
      }
    }

    // The memory access opcodes additionally carry a trailing optional
    // MemoryAccess operand, whose layout the fixed mask cannot describe.
    size_t mem_access_index;
    switch (op.opcode) {
    case spv::OpStore:
    case spv::OpCopyMemory:
      mem_access_index = 2;
      break;
    case spv::OpLoad:
    case spv::OpCopyMemorySized:
      mem_access_index = 3;
      break;
    case spv::OpCooperativeMatrixStoreKHR:
    case spv::OpCooperativeMatrixStoreNV:
      mem_access_index = 4;
      break;
    case spv::OpCooperativeMatrixLoadKHR:
    case spv::OpCooperativeMatrixLoadNV:
      mem_access_index = 5;
      break;
    default:
      return true;
    }
    size_t next;
    if (!scan_memory_access_operands(op, mem_access_index, next, indices)) {
      return false;
    }
    if (op.opcode == spv::OpCopyMemory || op.opcode == spv::OpCopyMemorySized) {
      // These take a second MemoryAccess operand for the source pointer
      // (added in SPIR-V 1.4).
      return scan_memory_access_operands(op, next, next, indices);
    }
    return true;
  }

  // Opcode absent from the table: conservatively include every word that
  // could plausibly be an id, which is everything except the result (a
  // definition, not a reference).
  bool has_result, has_result_type;
  spv::HasResultAndType(op.opcode, &has_result, &has_result_type);
  size_t result_index = has_result ? (size_t)has_result_type : num_args;
  for (size_t i = 0; i < num_args; ++i) {
    if (i != result_index) {
      indices.push_back((uint16_t)i);
    }
  }

  // If there are no args except for the result and result type, we can still
  // return true here, since there are no operands we can't classify.
  size_t result_prefix_size = (size_t)has_result + (size_t)has_result_type;
  return num_args == result_prefix_size;
}

/**
 * Returns the indices of the (non-tombstoned) declarations in topological
 * order: every declaration is preceded by the declarations it references.
 * This is the order in which emit() writes them out, and the order in which
 * deduplicate_types() visits them (so that merges cascade in one pass).
 */
pvector<size_t> SpirVModule::
sort_declarations() const {
  pvector<size_t> order;
  order.reserve(_declarations.size());
  pvector<int> state(_declarations.size(), 0);
  for (size_t i = 0; i < _declarations.size(); ++i) {
    r_sort_declaration(i, state, order);
  }
  return order;
}

/**
 * Appends the declaration with the given index to the order, after first
 * appending any not-yet-visited declarations it references (found through the
 * declaration indices recorded in the id index).
 */
void SpirVModule::
r_sort_declaration(size_t index, pvector<int> &state,
                   pvector<size_t> &order) const {
  if (state[index] != 0) {
    // Done, or currently being visited further up the stack (which would be
    // a reference cycle; SPIR-V forbids those for the types supported here,
    // so this is only hit if a literal was misidentified as an id, in which
    // case original order is the best we can do).
    return;
  }
  state[index] = 1;

  const Instruction &op = _declarations[index];
  if (!op.is_nop()) {
    small_vector<uint16_t, 8> id_operands;
    if (get_instruction_id_operands(op, id_operands)) {
      for (uint16_t arg_index : id_operands) {
        int32_t ref_index =
          get_definition(Id(op.args[arg_index]))._declaration_index;
        nassertd(ref_index < (int32_t)_declarations.size()) continue;
        if (ref_index >= 0 && (size_t)ref_index != index) {
          r_sort_declaration((size_t)ref_index, state, order);
        }
      }
    }
    // Do not follow conservative outgoing edges: one could invent a cycle and
    // emit a real dependent before this instruction.  During the normal
    // outer traversal this also leaves an unknown declaration after all
    // declarations that originally preceded it.
    order.push_back(index);
  }
  state[index] = 2;
}

/**
 * Rewrites all references to the given type id, in both the instructions and
 * the recorded result types in the id index.
 */
void SpirVModule::
rewrite_type_references(Id before, Id after) {
  small_vector<uint16_t, 8> id_operands;
  auto rewrite_instruction = [&](Instruction &op) {
    if (op.is_nop()) {
      return;
    }
    if (!get_instruction_id_operands(op, id_operands)) {
      // The conservative operand set may include literal words, which a
      // rewrite would corrupt if one collided with the replaced id.  A known
      // result-type position remains safe to update, however.
      if (op.has_result_type() && !op.args.empty() && op.args[0] == before) {
        op.args[0] = after;
      }
      return;
    }
    for (uint16_t arg_index : id_operands) {
      if (op.args[arg_index] == before) {
        op.args[arg_index] = after;
      }
    }
  };

  for (Instruction &op : _declarations) {
    rewrite_instruction(op);
  }

  for (Function &function : _functions) {
    // The function type is also cached on the Function record, which backs the
    // OpFunction operands written by emit().
    if (function.type_id == before) {
      function.type_id = after;
    }

    for (Instruction &op : function.instructions) {
      rewrite_instruction(op);
    }
  }

  // The index mirrors the result type of every value id, so follow suit.
  for (Definition &def : _defs) {
    if (def._type_id == before) {
      def._type_id = after;
    }
  }
}

/**
 * Makes sure that a module-scope variable of the given storage class with
 * the given builtin exists, and returns its id.
 */
SpirVId SpirVModule::
ensure_builtin(spv::StorageClass storage_class, spv::BuiltIn builtin) {
  Id var_id;
  for (uint32_t word = 0; word < _defs.size(); ++word) {
    Id id(word);
    const Definition &def = _defs[word];
    if (def._dtype == DT_variable && def._function_id == 0 &&
        get_storage_class(id) == storage_class &&
        get_builtin(id) == builtin) {
      var_id = id;
      break;
    }
  }

  if (var_id == 0) {
    const ShaderType *type = get_builtin_type(builtin);
    if (type == nullptr) {
      // Unhandled / invalid builtin
      nassertr(false, Id());
      return Id();
    }

    var_id = define_variable(type, storage_class);
    decorate(var_id, spv::DecorationBuiltIn, builtin);
  }

  return var_id;
}

/**
 * Writes a simple description of the given id to the output stream.
 */
void SpirVModule::
output_id(std::ostream &out, Id id) const {
  const Definition &def = get_definition(id);
  switch (def._dtype) {
  case DT_none:
    out << "undefined";
    break;

  case DT_typeless:
    out << "typeless result";
    break;

  case DT_type:
    {
      const ShaderType *type = r_resolve_type(id);
      if (type != nullptr) {
        out << "type " << *type;
      } else {
        out << "type";
      }
    }
    break;

  case DT_pointer_type:
    {
      const Instruction *decl = find_declaration(id);
      if (decl != nullptr && decl->args.size() >= 3) {
        out << "pointer to " << decl->args[2];
      } else {
        out << "pointer type";
      }
    }
    break;

  case DT_function_type:
    out << "function type";
    break;

  case DT_variable:
    out << "variable of type " << def._type_id;
    break;

  case DT_constant:
    out << "constant";
    break;

  case DT_spec_constant:
    out << "spec constant";
    break;

  case DT_function:
    out << "function";
    break;

  case DT_function_parameter:
    out << "function parameter";
    break;

  case DT_temporary:
    out << "temporary";
    break;

  case DT_ext_inst:
    out << "ext inst";
    break;

  case DT_string:
    out << "string";
    break;

  default:
    out << "invalid";
    break;
  }
}

/**
 * Issues an error about the given id.  Returns 0.
 */
SpirVId SpirVModule::
error_expected(Id id, const char *msg) const {
  std::ostringstream str;
  str << id << " (";
  output_id(str, id);
  str << ") was expected to be " << msg << "\n";
  nassert_raise(str.str());
  return Id();
}

// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !V8_ENABLE_WEBASSEMBLY
#error This header should only be included if WebAssembly is enabled.
#endif  // !V8_ENABLE_WEBASSEMBLY

#ifndef V8_WASM_MODULE_DECODER_H_
#define V8_WASM_MODULE_DECODER_H_

#include <memory>

#include "src/common/globals.h"
#include "src/logging/metrics.h"
#include "src/wasm/function-body-decoder.h"
#include "src/wasm/wasm-constants.h"
#include "src/wasm/wasm-features.h"
#include "src/wasm/wasm-module.h"
#include "src/wasm/wasm-result.h"

namespace v8 {
namespace internal {

class Counters;

namespace wasm {

struct CompilationEnv;

inline bool IsValidSectionCode(uint8_t byte) {
  // Allow everything within [kUnknownSectionCode, kLastKnownModuleSection].
  static_assert(kUnknownSectionCode == 0);
  return byte <= kLastKnownModuleSection;
}

const char* SectionName(SectionCode code);

using ModuleResult = Result<std::shared_ptr<WasmModule>>;
using FunctionResult = Result<std::unique_ptr<WasmFunction>>;
using FunctionOffsets = std::vector<std::pair<int, int>>;
using FunctionOffsetsResult = Result<FunctionOffsets>;

struct AsmJsOffsetEntry {
  int byte_offset;
  int source_position_call;
  int source_position_number_conversion;
};
struct AsmJsOffsetFunctionEntries {
  int start_offset;
  int end_offset;
  std::vector<AsmJsOffsetEntry> entries;
};
struct AsmJsOffsets {
  std::vector<AsmJsOffsetFunctionEntries> functions;
};
using AsmJsOffsetsResult = Result<AsmJsOffsets>;

class DecodedNameSection {
 public:
  explicit DecodedNameSection(base::Vector<const uint8_t> wire_bytes,
                              WireBytesRef name_section);

 private:
  friend class NamesProvider;

  IndirectNameMap local_names_;
  IndirectNameMap label_names_;
  NameMap type_names_;
  NameMap table_names_;
  NameMap memory_names_;
  NameMap global_names_;
  NameMap element_segment_names_;
  NameMap data_segment_names_;
  IndirectNameMap field_names_;
  NameMap tag_names_;
};

enum class DecodingMethod {
  kSync,
  kAsync,
  kSyncStream,
  kAsyncStream,
  kDeserialize
};

// Decodes the bytes of a wasm module between {module_start} and {module_end}.
V8_EXPORT_PRIVATE ModuleResult DecodeWasmModule(
    const WasmFeatures& enabled, const byte* module_start,
    const byte* module_end, bool verify_functions, ModuleOrigin origin,
    Counters* counters, std::shared_ptr<metrics::Recorder> metrics_recorder,
    v8::metrics::Recorder::ContextId context_id, DecodingMethod decoding_method,
    AccountingAllocator* allocator);

// Exposed for testing. Decodes a single function signature, allocating it
// in the given zone. Returns {nullptr} upon failure.
V8_EXPORT_PRIVATE const FunctionSig* DecodeWasmSignatureForTesting(
    const WasmFeatures& enabled, Zone* zone, const byte* start,
    const byte* end);

// Decodes the bytes of a wasm function between
// {function_start} and {function_end}.
V8_EXPORT_PRIVATE FunctionResult DecodeWasmFunctionForTesting(
    const WasmFeatures& enabled, Zone* zone, const ModuleWireBytes& wire_bytes,
    const WasmModule* module, const byte* function_start,
    const byte* function_end, Counters* counters);

V8_EXPORT_PRIVATE ConstantExpression
DecodeWasmInitExprForTesting(const WasmFeatures& enabled, const byte* start,
                             const byte* end, ValueType expected);

struct CustomSectionOffset {
  WireBytesRef section;
  WireBytesRef name;
  WireBytesRef payload;
};

V8_EXPORT_PRIVATE std::vector<CustomSectionOffset> DecodeCustomSections(
    const byte* start, const byte* end);

// Extracts the mapping from wasm byte offset to asm.js source position per
// function.
AsmJsOffsetsResult DecodeAsmJsOffsets(
    base::Vector<const uint8_t> encoded_offsets);

// Decode the function names from the name section. Returns the result as an
// unordered map. Only names with valid utf8 encoding are stored and conflicts
// are resolved by choosing the last name read.
void DecodeFunctionNames(const byte* module_start, const byte* module_end,
                         NameMap& names);

class ModuleDecoderImpl;

class ModuleDecoder {
 public:
  explicit ModuleDecoder(const WasmFeatures& enabled);
  ~ModuleDecoder();

  void StartDecoding(Counters* counters,
                     std::shared_ptr<metrics::Recorder> metrics_recorder,
                     v8::metrics::Recorder::ContextId context_id,
                     AccountingAllocator* allocator,
                     ModuleOrigin origin = ModuleOrigin::kWasmOrigin);

  void DecodeModuleHeader(base::Vector<const uint8_t> bytes, uint32_t offset);

  void DecodeSection(SectionCode section_code,
                     base::Vector<const uint8_t> bytes, uint32_t offset,
                     bool verify_functions = true);

  void StartCodeSection();

  bool CheckFunctionsCount(uint32_t functions_count, uint32_t error_offset);

  void DecodeFunctionBody(uint32_t index, uint32_t size, uint32_t offset,
                          bool verify_functions = true);

  ModuleResult FinishDecoding(bool verify_functions = true);

  void set_code_section(uint32_t offset, uint32_t size);

  const std::shared_ptr<WasmModule>& shared_module() const;

  WasmModule* module() const { return shared_module().get(); }

  bool ok();

  // Translates the unknown section that decoder is pointing to to an extended
  // SectionCode if the unknown section is known to decoder.
  // The decoder is expected to point after the section length and just before
  // the identifier string of the unknown section.
  // The return value is the number of bytes that were consumed.
  static size_t IdentifyUnknownSection(ModuleDecoder* decoder,
                                       base::Vector<const uint8_t> bytes,
                                       uint32_t offset, SectionCode* result);

 private:
  const WasmFeatures enabled_features_;
  std::unique_ptr<ModuleDecoderImpl> impl_;
};

}  // namespace wasm
}  // namespace internal
}  // namespace v8

#endif  // V8_WASM_MODULE_DECODER_H_

// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_CODEGEN_CODE_REFERENCE_H_
#define V8_CODEGEN_CODE_REFERENCE_H_

#include "src/base/platform/platform.h"
#include "src/handles/handles.h"
#include "src/objects/code.h"

namespace v8 {
namespace internal {

class InstructionStream;
class CodeDataContainer;
class CodeDesc;

namespace wasm {
class WasmCode;
}  // namespace wasm

class CodeReference {
 public:
  CodeReference() : kind_(Kind::NONE), null_(nullptr) {}
  explicit CodeReference(const wasm::WasmCode* wasm_code)
      : kind_(Kind::WASM_CODE), wasm_code_(wasm_code) {}
  explicit CodeReference(const CodeDesc* code_desc)
      : kind_(Kind::CODE_DESC), code_desc_(code_desc) {}
  explicit CodeReference(Handle<InstructionStream> code)
      : kind_(Kind::INSTRUCTION_STREAM), instruction_stream_(code) {}
  explicit CodeReference(Handle<CodeDataContainer> code_data_container)
      : kind_(Kind::CODE_DATA_CONTAINER),
        code_data_container_(code_data_container) {}

  Address constant_pool() const;
  Address instruction_start() const;
  Address instruction_end() const;
  int instruction_size() const;
  const byte* relocation_start() const;
  const byte* relocation_end() const;
  int relocation_size() const;
  Address code_comments() const;
  int code_comments_size() const;

  bool is_null() const { return kind_ == Kind::NONE; }
  bool is_instruction_stream() const {
    return kind_ == Kind::INSTRUCTION_STREAM;
  }
  bool is_code_data_container() const {
    return kind_ == Kind::CODE_DATA_CONTAINER;
  }
  bool is_wasm_code() const { return kind_ == Kind::WASM_CODE; }

  Handle<InstructionStream> as_instruction_stream() const {
    DCHECK_EQ(Kind::INSTRUCTION_STREAM, kind_);
    return instruction_stream_;
  }

  Handle<CodeDataContainer> as_code_data_container() const {
    DCHECK_EQ(Kind::CODE_DATA_CONTAINER, kind_);
    return code_data_container_;
  }

  const wasm::WasmCode* as_wasm_code() const {
    DCHECK_EQ(Kind::WASM_CODE, kind_);
    return wasm_code_;
  }

 private:
  enum class Kind {
    NONE,
    INSTRUCTION_STREAM,
    CODE_DATA_CONTAINER,
    WASM_CODE,
    CODE_DESC
  } kind_;
  union {
    std::nullptr_t null_;
    const wasm::WasmCode* wasm_code_;
    const CodeDesc* code_desc_;
    Handle<InstructionStream> instruction_stream_;
    Handle<CodeDataContainer> code_data_container_;
  };

  DISALLOW_NEW_AND_DELETE()
};
ASSERT_TRIVIALLY_COPYABLE(CodeReference);

}  // namespace internal
}  // namespace v8

#endif  // V8_CODEGEN_CODE_REFERENCE_H_

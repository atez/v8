// Copyright 2019 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_HEAP_READ_ONLY_HEAP_INL_H_
#define V8_HEAP_READ_ONLY_HEAP_INL_H_

#include "src/execution/isolate-utils-inl.h"
#include "src/heap/read-only-heap.h"
#include "src/roots/roots-inl.h"

namespace v8 {
namespace internal {

ReadOnlyHeap* ReadOnlyHeap::GetSharedReadOnlyHeap() {
#ifdef V8_COMPRESS_POINTERS_IN_MULTIPLE_CAGES
  return IsolateGroup::current()->shared_read_only_heap();
#else
  return ReadOnlyHeap::shared_ro_heap_;
#endif  // V8_COMPRESS_POINTERS_IN_MULTIPLE_CAGES
}

// static
ReadOnlyRoots ReadOnlyHeap::EarlyGetReadOnlyRoots(Tagged<HeapObject> object) {
#ifdef V8_SHARED_RO_HEAP
  ReadOnlyHeap* shared_ro_heap = GetSharedReadOnlyHeap();
  if (shared_ro_heap && shared_ro_heap->roots_init_complete()) {
    return ReadOnlyRoots(shared_ro_heap->read_only_roots_);
  }
  return ReadOnlyRoots(GetHeapFromWritableObject(object));
#else
  return GetReadOnlyRoots(object);
#endif  // V8_SHARED_RO_HEAP
}

// static
ReadOnlyRoots ReadOnlyHeap::GetReadOnlyRoots(Tagged<HeapObject> object) {
#ifdef V8_SHARED_RO_HEAP
  ReadOnlyHeap* shared_ro_heap = GetSharedReadOnlyHeap();
  // If this check fails in code that runs during initialization use
  // EarlyGetReadOnlyRoots instead.
  DCHECK(shared_ro_heap && shared_ro_heap->roots_init_complete());
  return ReadOnlyRoots(shared_ro_heap->read_only_roots_);
#else
  return ReadOnlyRoots(GetHeapFromWritableObject(object));
#endif  // V8_SHARED_RO_HEAP
}

}  // namespace internal
}  // namespace v8

#endif  // V8_HEAP_READ_ONLY_HEAP_INL_H_

// Copyright 2022 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Flags: --allow-natives-syntax

function f(i) {
  var x = 1;
  if (i) {} else { x = 2 }
  return x;
}

%PrepareFunctionForOptimization(f);
assertEquals(1, f(true));
assertEquals(2, f(false));

%OptimizeMaglevOnNextCall(f);
assertEquals(1, f(true));
assertEquals(2, f(false));

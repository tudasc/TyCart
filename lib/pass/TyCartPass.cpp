// TyCart library
//
// Copyright (c) 2021-2021 TyCart Authors
// Distributed under the BSD 3-Clause license.
// (See accompanying file LICENSE or copy at
// https://opensource.org/licenses/BSD-3-Clause)
//
// Project home: https://github.com/tudasc/TyCart
//
// SPDX-License-Identifier: BSD-3-Clause
//

#include "TyCartPass.h"

#include "support/Log.h"
#include "typegen/TypeGenerator.h"  // TypeART part
#include "typelib/TypeInterface.h"  // TypeART part

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FormatVariadic.h"

using namespace llvm;

#define DEBUG_TYPE "tycart-transform-pass"

static cl::opt<std::string> cl_type_file("tycart-types", cl::desc("Location of the generated type file."), cl::Hidden,
                                         cl::init("types.yaml"));

namespace tycart {

class TyCartPass : public ModulePass {
 private:
  std::unique_ptr<typeart::TypeGenerator> typegen_;

 public:
  static char ID;  // NOLINT

  TyCartPass() : ModulePass(ID) {
    assert(("Default type file not set", !cl_type_file.empty()));
  }

  bool doInitialization(Module& m) override {
    typegen_       = typeart::make_typegen(cl_type_file.getValue());
    const auto ret = typegen_->load();
    if (!ret.first) {
      LOG_WARNING("Could not load type file: " << cl_type_file.getValue())
    }
    return true;
  }

  bool doFinalization(Module&) override {
    if (typegen_) {
      const auto stored = typegen_->store();
    }
    return true;
  }

  bool runOnModule(Module& module) override {
    const auto instrumented_function = llvm::count_if(module.functions(), [&](auto& f) { return runOnFunc(f); }) > 0;
    return instrumented_function;
  }

  bool runOnFunc(Function& f) {
    if (f.isDeclaration() || f.getName().startswith("__typeart") || f.getName().startswith("tycart")) {
      return false;
    }
    bool mod{false};

    return mod;
  }

  ~TyCartPass() override = default;
};

}  // namespace tycart

char tycart::TyCartPass::ID = 0;

static RegisterPass<tycart::TyCartPass> x("tycart", "TyCart Pass");

ModulePass* createTyCartPass() {
  return new tycart::TyCartPass();
}

#include "llvm-c/Types.h"
#include "llvm/IR/LegacyPassManager.h"

extern "C" void AddTyCartPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createTyCartPass());
}

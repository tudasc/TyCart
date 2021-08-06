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
#include "support/TypeUtil.h"
#include "typegen/TypeGenerator.h"
#include "typelib/TypeInterface.h"

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

namespace callback {

struct FunctionDecl {
  struct TyCartFunction {
    const std::string name;
    llvm::Function* f{nullptr};
  };

  TyCartFunction assert_tycart{"tycart_assert_"};
  TyCartFunction assert_tycart_auto{"tycart_assert_auto_"};
  TyCartFunction assert_tycart_fti_t{"tycart_register_FTI_t_"};

  void initialize(Module& m) {
    using namespace llvm;
    auto& c = m.getContext();

    const auto add_optimizer_attributes = [&](auto& arg) {
      arg.addAttr(Attribute::NoCapture);
      arg.addAttr(Attribute::ReadOnly);
    };

    const auto make_function = [&](auto& f_struct, auto f_type) {
      auto func_callee = m.getOrInsertFunction(f_struct.name, f_type);
      f_struct.f       = dyn_cast<Function>(func_callee.getCallee());
      if (auto f = dyn_cast<Function>(f_struct.f)) {
        f->setLinkage(GlobalValue::ExternalLinkage);
        auto& first_param = *(f->arg_begin());
        if (first_param.getType()->isPointerTy()) {
          add_optimizer_attributes(first_param);
        }
      }
    };

    Type* assert_arg_types_tycart_auto[] = {Type::getInt32Ty(c), Type::getInt8PtrTy(c), Type::getInt64Ty(c),
                                            Type::getInt32Ty(c)};
    Type* assert_arg_types_tycart[]      = {Type::getInt32Ty(c), Type::getInt8PtrTy(c), Type::getInt64Ty(c),
                                       Type::getInt64Ty(c), Type::getInt32Ty(c)};
    Type* assert_arg_types_tycart_fti[]  = {Type::getInt32Ty(c)};

    make_function(assert_tycart_auto, FunctionType::get(Type::getVoidTy(c), assert_arg_types_tycart_auto, false));
    make_function(assert_tycart, FunctionType::get(Type::getVoidTy(c), assert_arg_types_tycart, false));
    make_function(assert_tycart_fti_t, FunctionType::get(Type::getVoidTy(c), assert_arg_types_tycart_fti, false));
  }
};

}  // namespace callback

namespace analysis {
enum class AssertKind { kTycart, kTycartFtiT, kTycartAuto };

const llvm::StringMap<AssertKind> kAssertMap{{"tycart_assert_stub_", AssertKind::kTycart},
                                             {"tycart_register_FTI_t_stub_", AssertKind::kTycartFtiT},
                                             {"tycart_assert_auto_stub_", AssertKind::kTycartAuto}};

struct AssertData {
  // TODO  asserts have bitcasts on various args
  llvm::CallBase* call{nullptr};
  AssertKind kind;
};

using AssertDataVec = llvm::SmallVector<AssertData, 4>;

class AssertOpVisitor : public llvm::InstVisitor<AssertOpVisitor> {
  AssertDataVec asserts_;

 public:
  const AssertDataVec& getAsserts() {
    return asserts_;
  }

  void clear() {
    asserts_.clear();
  }

  void visitCallBase(llvm::CallBase& cb) {
    if (auto* f = cb.getCalledFunction()) {
      auto kind = getKind(f);
      if (kind) {
        asserts_.push_back(AssertData{&cb, kind.getValue()});
      }
    }
  }

 private:
  static llvm::Optional<AssertKind> getKind(Function* f) {
    const auto function = f->getName();
    if (auto it = kAssertMap.find(function); it != std::end(kAssertMap)) {
      return it->second;
    }
    return None;
  }
};

}  // namespace analysis

namespace transform {

inline llvm::Error make_string_error(const char* message) {
  return llvm::make_error<llvm::StringError>(llvm::inconvertibleErrorCode(), message);
}

struct TycartAssertArgs {
  Value* buffer;
  Value* length;
  Value* checkpoint_id;
  Value* type_size;
  Value* typeart_type_id;
  analysis::AssertData assert;
};

llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const TycartAssertArgs& args) {
  os << "Buffer: " << *args.buffer << "| Length: " << *args.length << "| Check id: " << *args.checkpoint_id
     << "| Size: " << *args.type_size << "| Type ID: " << *args.typeart_type_id;

  return os;
}

class AssertStubCollector {
  Function* f_;
  DataLayout* dl_;
  const typeart::TypeGenerator* type_gen_;

 public:
  AssertStubCollector(Function* f, DataLayout* dl, const typeart::TypeGenerator* type_gen)
      : f_(f), dl_(dl), type_gen_(type_gen) {
  }

  Expected<TycartAssertArgs> handleTycart(const analysis::AssertData& ad) const {
    auto* assert_stub = ad.call;

    assert(assert_stub->arg_size() >= 4);

    Value* arg_buffer        = assert_stub->getArgOperand(0);
    Value* arg_type          = assert_stub->getArgOperand(1);
    Value* arg_length        = assert_stub->getArgOperand(2);
    Value* arg_checkpoint_id = assert_stub->getArgOperand(3);

    if (!arg_buffer) {
      return {make_string_error("Error buffer argument of assert stub is null.")};
    }
    if (!arg_type) {
      return {make_string_error("Error type argument of assert stub is null.")};
    }
    if (!arg_length) {
      return {make_string_error("Error length argument of assert stub is null.")};
    }
    if (!arg_checkpoint_id) {
      return {make_string_error("Error checkpoint argument of assert stub is null.")};
    }

    auto* arg_type_ptr = arg_type->getType();
    if (!arg_type_ptr->isPointerTy()) {
      return {make_string_error("Error type argument must be a pointer type.")};
    }

    // TODO testing:
    if (auto* arg_type_ptr_cast = dyn_cast<BitCastInst>(arg_type)) {
      arg_type_ptr = arg_type_ptr_cast->getSrcTy();
    }

    auto* type           = arg_type_ptr->getPointerElementType();
    auto type_size_bytes = util::getTypeSizeInBytes(type, *dl_);
    auto* type_size      = ConstantInt::get(IntegerType::get(f_->getContext(), 64), type_size_bytes);

    int typeart_type_id = type_gen_->getTypeID(type, *dl_);
    if (typeart_type_id == TYPEART_UNKNOWN_TYPE) {
      return {make_string_error("Error assert type is not supported/unknown.")};
    }
    auto* typeart_type = ConstantInt::get(IntegerType::get(f_->getContext(), 32), typeart_type_id);

    return TycartAssertArgs{arg_buffer, arg_length, arg_checkpoint_id, type_size, typeart_type, ad};
  }
};

class AssertStubTransformer {
  Function* f_;
  callback::FunctionDecl* decls_;

 public:
  AssertStubTransformer(Function* f, callback::FunctionDecl* decls) : f_(f), decls_(decls) {
  }

  [[nodiscard]] bool handleTycart(const TycartAssertArgs& ad) const {
    using namespace llvm;
    LOG_DEBUG(ad);

    auto* call_inst      = ad.assert.call;
    const bool is_invoke = llvm::isa<llvm::InvokeInst>(call_inst);

    if (is_invoke) {
    } else {
      // normal callinst:
      auto* target_callback = decls_->assert_tycart.f;
      IRBuilder<> irb(call_inst->getNextNode());
      irb.CreateCall(target_callback,
                     ArrayRef<Value*>{ad.checkpoint_id, ad.buffer, ad.length, ad.type_size, ad.typeart_type_id});
      call_inst->eraseFromParent();
    }

    return true;
  }
};

}  // namespace transform

class TyCartPass : public ModulePass {
 private:
  std::unique_ptr<typeart::TypeGenerator> typegen_;
  callback::FunctionDecl tycart_decls_;

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
    tycart_decls_.initialize(m);
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
    DataLayout dl(f.getParent());

    analysis::AssertOpVisitor visitor;
    visitor.visit(f);
    const auto asserts = visitor.getAsserts();

    if (asserts.empty()) {
      return false;
    }

    transform::AssertStubCollector collector{&f, &dl, typegen_.get()};
    transform::AssertStubTransformer transformer{&f, &tycart_decls_};

    const auto process_type_assert = [&](const analysis::AssertData& ad) -> bool {
      switch (ad.kind) {
        case analysis::AssertKind::kTycart: {
          auto data = collector.handleTycart(ad);
          if (data) {
            return transformer.handleTycart(data.get());
          }
          // TODO error handling
          break;
        }
        case analysis::AssertKind::kTycartFtiT:
          assert(("kTycartFtiT - Not yet supported", false));
          break;
        case analysis::AssertKind::kTycartAuto:
          assert(("kTycartAuto - Not yet supported", false));
          break;
        default:
          assert(("Missing case stmt in processTypeAssert", false));
          break;
      }
      return false;
    };

    mod |= llvm::count_if(asserts, process_type_assert) > 0;

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

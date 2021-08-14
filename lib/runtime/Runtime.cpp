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

#include "RuntimeInterface.h"
#include "TycartAssert.h"
#include "support/Log.h"

#include <cstdlib>
#include <sstream>

namespace tycart {
enum class AssertKind { kStrict, kRelaxed };

class Runtime final {
  AssertKind assert_kind_{AssertKind::kStrict};

 public:
  Runtime() {
    const char* mode = std::getenv("TYCART_ASSERT");
    if (mode) {
      if (strcmp(mode, "relaxed") == 0) {
        LOG_DEBUG("Mode is set to relaxed")
        assert_kind_ = AssertKind::kRelaxed;
      }
    }
  }

  [[nodiscard]] static Runtime& get() {
    static Runtime instance;
    return instance;
  }

  [[nodiscard]] AssertKind mode() const {
    return assert_kind_;
  }
};

class AssertHandler {
  // enum class AssertStatus { kTypeMismatch, kTypeMismatchRecurse, kCountMismatch, kCountMismatchRecurse, kTypeARTFail
  // };

 public:
  static void do_assert(const void* addr, const int type_id, const size_t count,
                        AssertKind assertk = AssertKind::kStrict) {
    int actual_type_id{TYPEART_UNKNOWN_TYPE};
    size_t actual_count{0};

    const auto fetch_type_information = [](const void* addr, int& actual_type_id, size_t& actual_count) {
      auto status = typeart_get_type(addr, &actual_type_id, &actual_count);
      if (status != TYPEART_OK) {
        // TODO log
        fail_typeart_status(status);
      }
    };

    const auto resolve_type = [&](auto id, typeart_struct_layout& layout) {
      auto status = typeart_resolve_type_id(id, &layout);
      if (status != TYPEART_OK && status != TYPEART_WRONG_KIND) {
        // TODO log
        fail_typeart_status(status);
      }

      return status;
    };

    fetch_type_information(addr, actual_type_id, actual_count);

    if (assertk == AssertKind::kStrict) {
      if (actual_type_id != type_id) {
        fail_type_mismatch(type_id, actual_type_id);
      }
      if (actual_count != count) {
        fail_count_mismatch(count, actual_count);
      }
    } else if (assertk == AssertKind::kRelaxed) {
      if (actual_type_id != type_id) {
        bool descent    = false;
        auto current_id = actual_type_id;
        typeart_struct_layout layout;

        do {
          descent = false;

          auto status = resolve_type(current_id, layout);

          // we cannot resolve, actual_type_id is not a struct:
          if (status == TYPEART_WRONG_KIND) {
            fail_type_mismatch_recurse(type_id, actual_type_id, current_id);
          }

          // we have a struct, take first member id
          if (layout.num_members > 0) {
            current_id = layout.member_types[0];
          }

          // only continue searching if the current type ID does not match
          // descent = layout.count > 0 && current_id != typeId;
          if (current_id != type_id) {
            if (layout.num_members == 0) {
              fail_type_mismatch_recurse(type_id, actual_type_id, current_id);
            } else {
              descent = true;
            }
          }
        } while (descent);

        // the type was resolved, but is the length as expected?
        if (count != layout.count[0]) {
          fail_count_mismatch_recurse(count, actual_count, layout.count[0]);
        }
      } else if (actual_count != count) {
        fail_count_mismatch(count, actual_count);
      }
    }
  }

  static size_t get_allocation_count(const void* pointer, const int type_id) {
    int queried_type_id{-1};
    size_t count{0};

    auto status = typeart_get_type(pointer, &queried_type_id, &count);

    if (status != TYPEART_OK) {
      fail_typeart_status(status);
    }

    if (type_id != queried_type_id) {
      fail_type_mismatch(type_id, queried_type_id);
    }

    return count;
  }

 private:
  static void fail(const std::string& message) {
    LOG_FATAL("Assert failed: " << message);
    exit(EXIT_FAILURE);
  }

  static void fail_type_mismatch(int type_id, int actual_type_id) {
    const char* expected_name = typeart_get_type_name(type_id);
    const char* actual_name   = typeart_get_type_name(actual_type_id);
    std::stringstream ss;
    ss << "Expected type " << expected_name << "(id=" << type_id << ") but got " << actual_name
       << "(id=" << actual_type_id << ")";
    fail(ss.str());
  }

  static void fail_type_mismatch_recurse(int type_id, int actual_type_id, int resolved_id) {
    const char* expected_name = typeart_get_type_name(type_id);
    const char* actual_name   = typeart_get_type_name(actual_type_id);
    const char* recursed_name = typeart_get_type_name(resolved_id);
    std::stringstream ss;
    ss << "During recursive resolve: Expected type " << expected_name << "(id=" << type_id << ") but got "
       << actual_name << "(id=" << actual_type_id << "). This resolved to " << recursed_name;
    fail(ss.str());
  };

  static void fail_count_mismatch(int count, int actual_count) {
    std::stringstream ss;
    ss << "Expected number of elements is " << count << " but actual number is " << actual_count;
    fail(ss.str());
  };

  static void fail_count_mismatch_recurse(int count, int actual_count, int resolved_count) {
    std::stringstream ss;
    ss << "Expected number of elements is " << count << " resolved to  " << resolved_count << " (from initial "
       << actual_count << ")";
    fail(ss.str());
  };

  static void fail_typeart_status(int status) {
    switch (status) {
      case TYPEART_OK:
        // No need to abort here.
        break;
      case TYPEART_INVALID_ID:
        fail("Type ID is invalid");
        break;
      case TYPEART_BAD_ALIGNMENT:
        fail("Pointer does not align to a type");
        break;
      case TYPEART_UNKNOWN_ADDRESS:
        fail("Address is unknown");
        break;
      default:
        fail("Unexpected error during type resolution");
    }
  }
};

}  // namespace tycart

void tycart_assert_(int /*checkpoint_id*/, const void* pointer, size_t count, size_t /*type_size*/, int type_id) {
  const void* return_address = __builtin_return_address(0);
  const auto assert_mode     = tycart::Runtime::get().mode();
  tycart::AssertHandler::do_assert(pointer, type_id, count, assert_mode);
}

void tycart_assert_auto_(int /*checkpoint_id*/, const void* pointer, size_t /*type_size*/, int type_id) {
  const void* return_address = __builtin_return_address(0);
  const auto assert_mode     = tycart::Runtime::get().mode();
  const size_t count         = tycart::AssertHandler::get_allocation_count(pointer, type_id);
  tycart::AssertHandler::do_assert(pointer, type_id, count, assert_mode);
}

void tycart_register_FTI_t_(int) {
  const void* return_address = __builtin_return_address(0);
}

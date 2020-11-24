// Copyright 2018-2020 Delft University of Technology
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <iostream>
#include <utility>

#include <tl/expected.hpp>
#include <putong/status.h>

namespace cerata {

/// Return on status error.
#define RETURN_SERR(s) {       \
  auto _status = s;                  \
  if (!_status.ok()) return _status; \
}                                    \
void()

/// Return error result on error status
#define RETURN_RERRS(s) {                 \
  if (!s.ok()) return error(s.err(), s.msg()); \
}                                              \
void()

#define RETURN_RERR(r) { if (!r) { return r.error(); } } void()

enum class Err {
  Generic,
  IO,
  Edge,
  Graph,
  Node,
  YAML
};

class Error {
 public:
  Error(Err type, std::string msg) : type_(type), msg_(std::move(msg)) {}
  [[nodiscard]] std::string msg() const { return msg_; }
  [[nodiscard]] Err type() const { return type_; }
 private:
  Err type_;
  std::string msg_;
};

using Status = putong::Status<Err>;

template<typename T>
using Result = tl::expected<T, Error>;

inline tl::unexpected<Error> error(Err type, const std::string &msg) {
  return tl::make_unexpected(Error(type, msg));
}

template<typename T>
inline Status status(const Result<T> &result) {
  if (result) {
    return Status::OK();
  } else {
    return Status(result.error().type(), result.error().msg());
  }
}

}
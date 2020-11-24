// Copyright 2018-2019 Delft University of Technology
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

#include <unordered_map>
#include <string>
#include <filesystem>

#include "cerata/errors.h"
#include "cerata/utils.h"
#include "cerata/logging.h"
#include "cerata_config/config.h"

namespace cerata {

std::string ToUpper(std::string str) {
  for (auto &ch : str) ch = std::toupper(ch);
  return str;
}

std::string ToLower(std::string str) {
  for (auto &ch : str) ch = std::tolower(ch);
  return str;
}

std::string ToString(const std::unordered_map<std::string, std::string> &meta) {
  std::string result;
  if (!meta.empty()) {
    result += "{";
    size_t i = 0;
    for (const auto &kv : meta) {
      result += kv.first + "=" + kv.second;
      if (i != meta.size() - 1) {
        result += ",";
      }
      i++;
    }
    result += "}";
  }
  return result;
}

Status CreateDir(const std::filesystem::path &path) {
  try {
    std::filesystem::create_directories(path);
  } catch (std::filesystem::filesystem_error &e) {
    return Status(Err::IO, e.what());
  }

  return Status::OK();
}

std::string version() {
  return "cerata " + std::to_string(CERATA_VERSION_MAJOR)
      + "." + std::to_string(CERATA_VERSION_MINOR)
      + "." + std::to_string(CERATA_VERSION_PATCH);
}

}  // namespace cerata

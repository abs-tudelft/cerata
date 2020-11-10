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

#include <yaml-cpp/yaml.h>

#include "cerata/api.h"
#include "cerata/yaml/yaml.h"

namespace cerata::yaml {

// Expected keys and values.
constexpr auto YAML_KEY_FIELD = "field";
constexpr auto YAML_KEY_RECORD = "record";
constexpr auto YAML_KEY_FIELDS = "fields";
constexpr auto YAML_KEY_NAME = "name";
constexpr auto YAML_KEY_WIDTH = "width";
constexpr auto YAML_KEY_VECTOR = "vector";
constexpr auto YAML_KEY_REVERSE = "reverse";

/**
 * \brief Converts YAMLs to Cerata types.
 */
class YamlConverter {
 public:
  /**
   * \brief Construct a new YAML converter.
   * \param yaml The YAML text to convert.
   * \param out The Field to produce.
   */
  explicit YamlConverter(const std::string &yaml, std::shared_ptr<cerata::Field> *out);

  /**
   * \brief Construct a new YAML converter.
   * \param root The YAML root node to convert.
   * \param out The Field to produce.
   */
  explicit YamlConverter(const YAML::Node &root, std::shared_ptr<cerata::Field> *out);

  /**
   * \brief Attempt to convert the YAML to a field.
   * \return Status::OK() if successful, some error otherwise.
   */
  auto Convert() -> Status;

 protected:
  /**
   * \brief Visit a YAML node.
   * \param node The node to visit.
   * \return Status::OK() if successful, some error otherwise.
   */
  auto Visit(const YAML::Node &node) -> Status;

 private:
  YAML::Node root_;
  std::shared_ptr<cerata::Field> *out_;

  std::string name_;
  unsigned int width_ = 0;
  bool force_vector_ = false;
  bool reverse_ = false;
  std::vector<std::shared_ptr<cerata::Field>> fields_;
};

}
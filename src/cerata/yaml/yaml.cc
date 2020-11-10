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

#include <yaml-cpp/yaml.h>

#include "cerata/api.h"
#include "cerata/yaml/yaml.h"

namespace cerata::yaml {

YamlConverter::YamlConverter(const std::string &yaml, std::shared_ptr<cerata::Field> *out) {
  this->root_ = YAML::Load(yaml);
  this->out_ = out;
}

YamlConverter::YamlConverter(const YAML::Node &root, std::shared_ptr<cerata::Field> *out) {
  this->root_ = root;
  this->out_ = out;
}

auto ToString(const YAML::Mark &mark) -> std::string {
  return "Pos: " + std::to_string(mark.pos) + " Line: " + std::to_string(mark.line) + " Col:"
      + std::to_string(mark.column);
}

auto YamlConverter::Visit(const YAML::Node &node) -> Status {
  try {
    switch (node.Type()) {
      case YAML::NodeType::Sequence: {
        for (const auto &item : node) {
          if (item[YAML_KEY_FIELD] || item[YAML_KEY_RECORD]) {
            std::shared_ptr<cerata::Field> field;
            auto conv = YamlConverter(item, &field);
            CERATA_ROE(conv.Convert());
            fields_.push_back(field);
          } else {
            return Status(Error::YAMLError,
                          "Sequence can only contain \"" + std::string(YAML_KEY_FIELD) + "\" or \""
                              + std::string(YAML_KEY_RECORD) + "\"");
          }
        }
        break;
      }

      case YAML::NodeType::Map: {
        if (node[YAML_KEY_NAME]) {
          name_ = node[YAML_KEY_NAME].as<std::string>();
        } else {
          return Status(Error::YAMLError, "Field must have \"name\"");
        }
        if (node[YAML_KEY_WIDTH]) {
          width_ = node[YAML_KEY_WIDTH].as<unsigned int>();
        }
        if (node[YAML_KEY_VECTOR]) {
          force_vector_ = node[YAML_KEY_VECTOR].as<bool>();
        }
        if (node[YAML_KEY_DIR]) {
          auto val = node[YAML_KEY_DIR].as<std::string>();
          if (val == YAML_VAL_KERNEL_TO_PLATFORM) {
            reverse_ = true;
          } else if (val == YAML_VAL_PLATFORM_TO_KERNEL) {
            reverse_ = false;
          } else {
            return Status(Error::YAMLError,
                          "Field " + name_ + " direction must be either " + std::string(YAML_VAL_PLATFORM_TO_KERNEL)
                              + " or " + std::string(YAML_VAL_KERNEL_TO_PLATFORM));
          }
        }
        if (node[YAML_KEY_FIELDS] && (width_ == 0)) {
          auto fields = node[YAML_KEY_FIELDS];
          CERATA_ROE(Visit(node[YAML_KEY_FIELDS]));
        } else if (node[YAML_KEY_FIELDS] && (width_ > 0)) {
          return Status(Error::YAMLError, "Field " + name_ + " cannot have both width and subfields.");
        } else if (!node[YAML_KEY_WIDTH]) {
          return Status(Error::YAMLError, "Field " + name_ + " must have either width or subfields.");
        }
        break;
      }

        // Some nodes we should never encounter through this visitor.
      case YAML::NodeType::Undefined: {
        return Status(Error::YAMLError,
                      "Unexpected undefined at " + std::to_string(node.Mark().line) + ":"
                          + std::to_string(node.Mark().column));
      }
      case YAML::NodeType::Null: {
        return Status(Error::YAMLError, "Unexpected null at " + ToString(node.Mark()));
      }
      case YAML::NodeType::Scalar: {
        return Status(Error::YAMLError, "Unexpected scalar at " + ToString(node.Mark()));
      }
    }
  } catch (const YAML::Exception &e) {
    return Status(Error::YAMLError, "YAML parsing error: " + std::string(e.msg));
  }
  return Status::OK();
}

auto YamlConverter::Convert() -> Status {
  auto status = Visit(root_);
  if (!status.ok()) { return status; }

  if (fields_.empty()) {
    // Check if this is to be a record, vector or just a bit.
    if ((width_ > 1) || force_vector_) {
      *out_ = cerata::field(name_, cerata::vector(width_), reverse_);
    } else if (width_ > 0) {
      *out_ = cerata::field(name_, cerata::bit(name_), reverse_);
    } else {
      return Status(Error::YAMLError, "Width for " + name_ + " must be greater than 0.");
    }
  } else {
    *out_ = cerata::field(name_, cerata::record(name_, {fields_}), reverse_);
  }
  return Status::OK();
}

}

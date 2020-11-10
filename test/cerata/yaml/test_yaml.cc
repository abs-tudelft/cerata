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

#include <gmock/gmock.h>
#include <yaml-cpp/yaml.h>

#include <string>

#include "cerata/api.h"
#include "cerata/yaml/yaml.h"

namespace cerata::yaml {

void TestYaml(const std::string &yaml, const std::shared_ptr<Type> &expected) {
  auto exp = field(expected);
  std::shared_ptr<Field> actual;
  auto conv = yaml::YamlConverter(YAML::Load(yaml), &actual);
  auto status = conv.Convert();
  ASSERT_TRUE(status.ok());
  ASSERT_TRUE(exp->type()->IsEqual(*actual->type()));
}

TEST(YAML, Bit) {
  TestYaml(R"(
- field:
  name: hi
  width: 1
)", record({field("hi", bit())}));
}

TEST(YAML, ForcedVectorBit) {
  TestYaml(R"(
- field:
  name: hi
  width: 1
  vector: true
)", record({field("hi", vector(1))}));
}

TEST(YAML, Vector) {
  TestYaml(R"(
- field:
  name: hi
  width: 2
)", record({field("hi", vector(2))}));
}

TEST(YAML, Reversed) {
  TestYaml(R"(
- field:
  name: hi
  width: 2
  reverse: true
)", record({field("hi", vector(2), true)}));
}

TEST(YAML, TopLevelRecord) {
  TestYaml(R"(
- field:
  name: a
  width: 2
  reverse: true
- field:
  name: b
  width: 1
)", record({field("a", vector(2), true),
            field("b", bit())}));
}

TEST(YAML, NestedRecord) {
  TestYaml(R"(
- field:
  name: a
  width: 2
  vector: true
- record:
  name: b
  reverse: true
  fields:
  - field:
    name: x
    width: 1
  - field:
    name: y
    width: 2
  - record:
    name: z
    fields:
    - field:
      name: k
      width: 1
    - field:
      name: l
      width: 3
)", record({field("a", vector(2)),
            field("b",
                  record({field("x", bit()),
                          field("y", vector(2)),
                          field("z", record({field("k", bit()),
                                             field("l", vector(3))}))}),
                  true)}));
}

}

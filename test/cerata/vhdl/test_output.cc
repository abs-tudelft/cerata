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
#include <cerata/api.h>
#include <cerata/vhdl/api.h>

#include "cerata/test_designs.h"
#include "cerata/test_utils.h"

namespace cerata {

TEST(VHDL_OUTPUT, OutputGenerator) {
  default_component_pool()->Clear();
  auto top = GetExampleDesign();
  auto og = vhdl::VHDLOutputGenerator(std::filesystem::current_path(), {{top.get(), {}}});
  auto status = og.Generate();
  ASSERT_TRUE(status.ok());
  ASSERT_TRUE(std::filesystem::exists(
      std::filesystem::current_path() / "vhdl" / "top.gen.vhd"));
  ASSERT_TRUE(std::filesystem::remove(
      std::filesystem::current_path() / "vhdl" / "top.gen.vhd"));
}

}  // namespace cerata

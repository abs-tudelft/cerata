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

#include "cerata/vhdl/api.h"

#include <vector>
#include <string>
#include <memory>

#include "cerata/logging.h"
#include "cerata/utils.h"

#include "cerata/vhdl/defaults.h"

namespace cerata::vhdl {

Status VHDLOutputGenerator::Generate() {
  // Make sure the subdirectory exists.
  auto path = root_ / subdir();
  RETURN_SERR(CreateDir(path));

  size_t num_graphs = 0;
  for (const auto &o : outputs_) {
    // Check if output spec is valid
    if (o.comp == nullptr) {
      CERATA_LOG(WARNING, "OutputSpec contained no component.");
      continue;
    }

    CERATA_LOG(DEBUG,
               "VHDL: Transforming Component " + o.comp->name()
                   + " to VHDL-compatible version.");
    auto vhdl_design = Design(o.comp, notice_, DEFAULT_LIBS);

    CERATA_LOG(DEBUG, "VHDL: Generating sources for component " + o.comp->name());
    auto vhdl_source = vhdl_design.Generate().ToString();
    auto vhdl_path = path / (o.comp->name() + ".gen.vhd");
    // Disable backup by default.
    bool backup = (o.meta.count(meta::BACKUP_EXISTING) > 0)
        && (o.meta.at(meta::BACKUP_EXISTING) == "true");

    CERATA_LOG(DEBUG, "VHDL: Saving design to: " + vhdl_path.string());
    if (!std::filesystem::exists(vhdl_path) || !backup) {
      auto vhdl_file = std::ofstream(vhdl_path);
      vhdl_file << vhdl_source;
      vhdl_file.close();
    } else {
      auto vhdl_backup_path = vhdl_path += ".bak";
      CERATA_LOG(DEBUG, "VHDL: File exists, backing up to " + vhdl_backup_path.string());
      // Attempt to copy the old file.
      try {
        std::filesystem::copy(vhdl_path, vhdl_backup_path);
      } catch (std::filesystem::filesystem_error &e) {
        return Status(Err::IO, e.what());
      }

      // Save the new file.
      auto vhdl_file = std::ofstream(vhdl_path);
      vhdl_file << vhdl_source;
    }

    num_graphs++;
  }
  CERATA_LOG(DEBUG,
             "VHDL: Generated output for " + std::to_string(num_graphs) + " graphs.");
  return Status::OK();
}

}  // namespace cerata::vhdl


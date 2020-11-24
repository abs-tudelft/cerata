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

#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <utility>
#include <string>
#include <memory>

#include "cerata/graph.h"

namespace cerata {

/// \brief Structure to specify output properties per graph
struct OutputSpec {
  /// The component to output.
  Component *comp;
  /// Metadata for back-ends.
  std::unordered_map<std::string, std::string> meta = {};
};

/**
 * \brief Abstract class to generate language specific output from Graphs
 */
class OutputGenerator {
 public:
  /// \brief Construct an OutputGenerator.
  explicit OutputGenerator(std::filesystem::path root,
                           std::vector<OutputSpec> outputs = {});

  /// \brief Add a graph to the list of graphs to generate output for.
  OutputGenerator &AddOutput(const OutputSpec &output);

  /// \brief Start the output generation.
  virtual Status Generate() = 0;

  /// \brief Return the subdirectory this OutputGenerator will generate into.
  virtual std::string subdir() = 0;

 protected:
  /// \brief The root directory to generate the output in.
  std::filesystem::path root_;
  /// \brief A list of things to put out.
  std::vector<OutputSpec> outputs_;
};

}  // namespace cerata

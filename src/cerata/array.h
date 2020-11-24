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

#include <utility>
#include <optional>
#include <string>
#include <memory>
#include <vector>

#include "cerata/node.h"
#include "cerata/port.h"
#include "cerata/type.h"
#include "cerata/signal.h"

namespace cerata {

/// An array of nodes.
class NodeArray : public Object {
 public:
  static Result<std::shared_ptr<NodeArray>> Make(std::string name,
                                                 Node::NodeID id,
                                                 std::shared_ptr<Node> base,
                                                 const std::shared_ptr<Node> &size);
  /// \brief Return the type ID of the nodes in this NodeArray.
  Node::NodeID node_id() { return node_id_; }

  /// \brief Set the parent of this NodeArray base node and array nodes.
  void SetParent(Graph *new_parent) override;

  /// \brief Return the size node.
  inline Node *size() const { return size_.get(); }

  /// \brief Set the size node.
  [[nodiscard]] Status SetSize(const std::shared_ptr<Node> &size);
  /// \brief Set the type of the base node and array nodes.
  void SetType(const std::shared_ptr<Type> &type);
  /// \brief Return the type of the nodes in the NodeArray.
  Type *type() const { return base_->type(); }

  /// \brief Deep-copy the NodeArray, but not the array nodes. Resets the size node to an integer literal of 0.
  std::shared_ptr<Object> Copy() const override;

  /// \brief Copy the NodeArray onto a graph, but not the array nodes. Creates a new size node set to zero.
  Result<NodeArray *> CopyOnto(Graph *dst, const std::string &name, NodeMap *rebinding);

  /// \brief Append a node to this array, optionally incrementing the size node. Returns a pointer to that node.
  std::shared_ptr<Node> Append(bool increment_size = true);
  /// \brief Return all nodes of this NodeArray.
  std::vector<Node *> nodes() const { return ToRawPointers(nodes_); }
  /// \brief Return element node i.
  Node *node(size_t i) const;
  /// \brief Return element node i.
  Node *operator[](size_t i) const { return node(i); }
  /// \brief Return the number of element nodes.
  size_t num_nodes() const { return nodes_.size(); }
  /// \brief Return the index of a specific node.
  Result<size_t> IndexOf(const Node &n) const;

  /// \brief Return a human-readable representation of this NodeArray.
  std::string ToString() const { return name(); }

  /// \brief Return the base node of this NodeArray.
  std::shared_ptr<Node> base() const { return base_; }

  // \brief Append all objects referenced by this node array.
  void AppendReferences(std::vector<Object *> *out) const override {
    // This includes the size node and its references.
    out->push_back(size_.get());
    size_->AppendReferences(out);
    // And the references of the base node.
    base_->AppendReferences(out);
  }

 protected:
  /// \brief ArrayNode constructor.
  NodeArray(std::string name,
            Node::NodeID id,
            std::shared_ptr<Node> base,
            const std::shared_ptr<Node> &size,
            Status *status);

  /// The type ID of the nodes in this NodeArray.
  Node::NodeID node_id_;
  /// \brief Increment the size of the ArrayNode.
  [[nodiscard]] Status IncrementSize();
  /// A node representing the template for each of the element nodes.
  std::shared_ptr<Node> base_;
  /// A node representing the number of concatenated edges.
  std::shared_ptr<Node> size_;
  /// The nodes contained by this array
  std::vector<std::shared_ptr<Node>> nodes_;
};

/// An array of signal nodes.
class SignalArray : public NodeArray {
 public:
  static Result<std::shared_ptr<SignalArray>> Make(const std::string &name,
                                                   const std::shared_ptr<Type> &type,
                                                   const std::shared_ptr<Node> &size,
                                                   const std::shared_ptr<ClockDomain> &domain = default_domain());
 protected:
  /// SignalArray constructor.
  SignalArray(const std::shared_ptr<Signal> &base,
              const std::shared_ptr<Node> &size,
              Status *status)
      : NodeArray(base->name(), Node::NodeID::SIGNAL, base, size, status) {}
};

/**
 * \brief An array of port nodes
 */
class PortArray : public NodeArray, public Term {
 public:
  /// \brief Get a smart pointer to a new ArrayPort.
  static Result<std::shared_ptr<PortArray>> Make(const std::string &name,
                                                 const std::shared_ptr<Type> &type,
                                                 const std::shared_ptr<Node> &size,
                                                 Port::Dir dir = Port::Dir::IN,
                                                 const std::shared_ptr<ClockDomain> &domain = default_domain());

/// \brief Get a smart pointer to a new ArrayPort with a base type other than the default
/// Port.
  static Result<std::shared_ptr<PortArray>> Make(const std::shared_ptr<Port> &base_node,
                                                 const std::shared_ptr<Node> &size);

  /// \brief Make a copy of this port array
  std::shared_ptr<Object> Copy() const override;

 protected:
  /// \brief Construct a new port array.
  PortArray(const std::shared_ptr<Port> &base,
            const std::shared_ptr<Node> &size,
            Status *status);
};

}  // namespace cerata

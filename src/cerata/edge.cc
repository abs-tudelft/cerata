// Copyright 2018 Delft University of Technology
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

#include "cerata/edge.h"

#include <memory>
#include <deque>
#include <string>
#include <optional>

#include "cerata/graph.h"
#include "cerata/node.h"
#include "cerata/logging.h"

namespace cerata {

Edge::Edge(std::string name, Node *dst, Node *src)
    : Named(std::move(name)), dst_(dst), src_(src) {
  if ((dst == nullptr) || (src == nullptr)) {
    throw std::runtime_error("Cannot create edge with nullptrs.");
  }
}

std::shared_ptr<Edge> Edge::Make(const std::string &name,
                                 Node *dst,
                                 Node *src) {
  auto e = new Edge(name, dst, src);
  return std::shared_ptr<Edge>(e);
}

static void CheckDomains(Node *src, Node *dst) {
  if ((src->IsPort() || src->IsSignal()) && (dst->IsPort() || dst->IsSignal())) {
    auto src_dom = dynamic_cast<Synchronous *>(src)->domain();
    auto dst_dom = dynamic_cast<Synchronous *>(dst)->domain();
    if (src_dom != dst_dom) {
      std::stringstream warning;
      warning << "Attempting to connect Synchronous nodes, but clock domains differ.\n";

      warning << "Src: [" + src->ToString() + "] in domain: [" + dst_dom->name() + "]";
      if (src->parent()) {
        warning << " on parent: [" + src->parent().value()->name() + "]";
      }

      warning << "\nDst: [" + dst->ToString() + "] in domain: [" + src_dom->name() + "]";
      if (dst->parent()) {
        warning << " on parent: [" + dst->parent().value()->name() + "]";
      }

      warning << "\nAutomated CDC crossings are not yet implemented or instantiated.";
      warning << "This behavior may cause incorrect designs.";
      CERATA_LOG(WARNING, warning.str());
    }
  }
}

std::shared_ptr<Edge> Connect(Node *dst, Node *src) {
  // Check for potential errors
  if (src == nullptr) {
    CERATA_LOG(ERROR, "Source node is null");
    return nullptr;
  } else if (dst == nullptr) {
    CERATA_LOG(ERROR, "Destination node is null");
    return nullptr;
  }

  // Check if the clock domains correspond. Currently, this doesn't result in an error as automated CDC support is not
  // in place yet. Just generate a warning for now:
  CheckDomains(src, dst);

  // Check if the types can be mapped onto each other
  if (!src->type()->GetMapper(dst->type())) {
    CERATA_LOG(FATAL, "No known type mapping available for connection between node "
        + dst->ToString() + " (" + dst->type()->ToString() + ")  and ("
        + src->ToString() + " (" + src->type()->ToString() + ")");
  }

  // If the destination is a terminator
  if (dst->IsPort()) {
    auto port = dynamic_cast<Port *>(dst);
    // Check if it has a parent
    if (dst->parent()) {
      auto parent = *dst->parent();
      if (parent->IsInstance() && port->IsOutput()) {
        // If the parent is an instance, and the terminator node is an output, then we may not drive it.
        CERATA_LOG(FATAL, "Cannot drive instance port " + dst->ToString() + " of mode output.");
      } else if (parent->IsComponent() && port->IsInput()) {
        // If the parent is a component, and the terminator node is an input, then we may not drive it.
        CERATA_LOG(FATAL, "Cannot drive component port " + dst->ToString() + " of mode input.");
      }
    }
  }

  // If the source is a terminator
  if (src->IsPort()) {
    auto port = dynamic_cast<Port *>(src);
    // Check if it has a parent
    if (src->parent()) {
      auto parent = *src->parent();
      if (parent->IsInstance() && port->IsInput()) {
        // If the parent is an instance, and the terminator node is an input, then we may not source from it.
        CERATA_LOG(FATAL, "Cannot source from instance port " + src->ToString() + " of mode input "
                                                                                  "on " + parent->ToString());
      } else if (parent->IsComponent() && port->IsOutput()) {
        // If the parent is a component, and the terminator node is an output, then we may not source from it.
        CERATA_LOG(FATAL, "Cannot source from component port " + src->ToString() + " of mode output "
                                                                                   "on " + parent->ToString());
      }
    }
  }

  std::string edge_name = src->name() + "_to_" + dst->name();
  auto edge = Edge::Make(edge_name, dst, src);
  src->AddEdge(edge);
  dst->AddEdge(edge);
  return edge;
}

std::shared_ptr<Edge> operator<<=(Node *dst, const std::shared_ptr<Node> &src) {
  return Connect(dst, src.get());
}

std::shared_ptr<Edge> operator<<=(const std::weak_ptr<Node> &dst, const std::weak_ptr<Node> &src) {
  return Connect(dst.lock().get(), src.lock().get());
}

std::shared_ptr<Edge> operator<<=(const std::weak_ptr<Node> &dst, const std::shared_ptr<Node> &src) {
  return Connect(dst.lock().get(), src.get());
}

std::shared_ptr<Edge> operator<<=(const std::shared_ptr<Node> &dst, const std::shared_ptr<Node> &src) {
  return Connect(dst.get(), src.get());
}

std::shared_ptr<Edge> operator<<=(const std::weak_ptr<Node> &dst, Node *src) {
  return Connect(dst.lock().get(), src);
}

std::deque<Edge *> GetAllEdges(const Graph &graph) {
  std::deque<Edge *> all_edges;

  // Get all normal nodes
  for (const auto &node : graph.GetAll<Node>()) {
    auto out_edges = node->sinks();
    for (const auto &e : out_edges) {
      all_edges.push_back(e);
    }
    auto in_edges = node->sources();
    for (const auto &e : in_edges) {
      all_edges.push_back(e);
    }
  }

  for (const auto &array : graph.GetAll<NodeArray>()) {
    for (const auto &node : array->nodes()) {
      auto out_edges = node->sinks();
      for (const auto &e : out_edges) {
        all_edges.push_back(e);
      }
      auto in_edges = node->sources();
      for (const auto &e : in_edges) {
        all_edges.push_back(e);
      }
    }
  }

  if (graph.IsComponent()) {
    auto &comp = dynamic_cast<const Component &>(graph);
    for (const auto &g : comp.children()) {
      auto child_edges = GetAllEdges(*g);
      all_edges.insert(all_edges.end(), child_edges.begin(), child_edges.end());
    }
  }

  return all_edges;
}

std::shared_ptr<Signal> insert(Edge *edge, const std::string &name_prefix, std::optional<Graph *> new_owner) {
  auto src = edge->src();
  auto dst = edge->dst();

  // Make sure we're inserting between signal/port nodes.
  if (!(src->IsPort() || src->IsSignal()) && (dst->IsPort() || dst->IsSignal())) {
    CERATA_LOG(FATAL, "Attempting to insert signal node on edge between non-port/signal node.");
  }

  // When they were connected, their clock domains must have matched. Just grab the domain from the source.
  std::shared_ptr<ClockDomain> domain;
  if (src->IsPort()) domain = src->AsPort().domain();
  if (src->IsSignal()) domain = src->AsSignal().domain();

  // Get the destination type
  auto type = src->type();
  auto name = name_prefix + src->name();
  // Create the signal and take shared ownership of the type
  auto signal = Signal::Make(name, type->shared_from_this(), domain);

  // Share ownership of the new signal with the potential new_owner
  if (new_owner) {
    (*new_owner)->AddObject(signal);
  }

  // Remove the original edge from the source and destination node
  src->RemoveEdge(edge);
  dst->RemoveEdge(edge);
  // From this moment onward, the edge may be deconstructed and should not be used anymore.
  // Make the new connections, effectively creating two new edges on either side of the signal node.
  signal <<= src;
  dst <<= signal;
  // Return the new signal
  return signal;
}

std::optional<Node *> Edge::GetOtherNode(const Node &node) const {
  if (src_ == &node) {
    return dst_;
  } else if (dst_ == &node) {
    return src_;
  } else {
    return std::nullopt;
  }
}

}  // namespace cerata

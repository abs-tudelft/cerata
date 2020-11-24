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

#include "cerata/dot/api.h"

#include <sstream>

#include "cerata/errors.h"
#include "cerata/logging.h"
#include "cerata/edge.h"
#include "cerata/type.h"
#include "cerata/output.h"
#include "cerata/utils.h"
#include "cerata/node.h"
#include "cerata/expression.h"

namespace cerata::dot {

Status DOTOutputGenerator::Generate() {
  auto path = root_ / subdir();
  RETURN_SERR(CreateDir(path));
  cerata::dot::GraphGenerator dot;
  for (const auto &o : outputs_) {
    auto file = path / (o.comp->name() + ".dot");
    if (o.comp != nullptr) {
      CERATA_LOG(DEBUG, "DOT: Generating output for Graph: " + o.comp->name());
      dot.GenFile(*o.comp, file);
    }
  }
  return Status::OK();
}

static std::string ToHex(const Node &n) {
  std::stringstream ret;
  ret << std::hex << reinterpret_cast<uint64_t>(&n);
  return ret.str();
}

std::string GraphGenerator::GenEdges(const Graph &graph, int level) {
  std::stringstream ret;
  auto all_edges = GetAllEdges(graph);
  for (const auto &e : all_edges) {
    if (!Contains(drawn_edges, e)) {
      // Remember we've drawn this edge
      drawn_edges.push_back(e);

      // Check if edge is complete
      auto *dst = e->dst();
      auto *src = e->src();
      if ((dst == nullptr) || (src == nullptr)) {
        continue;
      }
      // Don't draw literals
      if (dst->IsLiteral() || src->IsLiteral()) {
        continue;
      }

      auto draw_style = false;
      // Draw edge
      ret << tab(level);
      if (src->IsExpression() && style.config.nodes.expand.expression) {
        auto src_name = ToHex(*src);
        ret << " -> ";
        ret << NodeName(*dst);
        ret << "\"" + src_name + "\"";
        draw_style = true;
      } else if ((src->IsParameter() && style.config.nodes.parameters)
          || !src->IsParameter()) {
        auto src_name = NodeName(*src);
        ret << src_name;
        ret << " -> ";
        ret << NodeName(*dst);
        draw_style = true;
      }

      if (draw_style) {
        // Set style
        StyleBuilder sb;
        ret << " [";
        switch (src->type()->id()) {
          default: {
            sb << style.edge.base;
            break;
          }
        }

        // Put array index label
        if (src->array() && !dst->array()) {
          sb << "label=\"" + std::to_string((*src->array())->IndexOf(*src).value())
              + "\"";
        }
        if (!src->array() && dst->array()) {
          sb << "label=\"" + std::to_string((*dst->array())->IndexOf(*dst).value())
              + "\"";
        }
        if (src->array() && dst->array()) {
          sb << "label=\"" + std::to_string((*src->array())->IndexOf(*src).value())
              + " to "
              + std::to_string((*dst->array())->IndexOf(*dst).value()) + "\"";
        }

        if ((src->IsPort()) && style.config.nodes.ports) {
          if (dst->IsSignal()) {
            // Port to signal
            sb << style.edge.port_to_sig;
          } else if (dst->IsPort()) {
            sb << style.edge.port_to_port;
          }
        } else if (src->IsSignal() && style.config.nodes.signals) {
          if (dst->IsPort()) {
            // Signal to port
            sb << style.edge.sig_to_port;
          }
        } else if (src->IsParameter() && style.config.nodes.parameters) {
          sb << style.edge.param;
        } else if (src->IsLiteral() && style.config.nodes.literals) {
          sb << style.edge.lit;
        } else if (src->IsExpression() && style.config.nodes.expressions) {
          sb << style.edge.expr;
          if (style.config.nodes.expand.expression) {
            sb << "lhead=\"cluster_" + NodeName(*src) + "\"";
          }
        } else {
          ret << "]\n";
          continue;
        }
        ret << sb.ToString();
        // Generic edge
        ret << "]\n";
      }
    }
  }

  return ret.str();
}

std::string Style::GenHTMLTableCell(const Type &t,
                                    const std::string &name,
                                    int level) {
  std::stringstream str;
  // Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn
  if (t.Is(Type::RECORD)) {
    auto rec = dynamic_cast<const Record &>(t);
    str << R"(<TABLE BORDER="1" CELLBORDER="0" CELLSPACING="0")";
    if (level == 0) {
      str << R"( PORT="cell")";
    }
    str << ">";
    str << "<TR>";
    str << "<TD";
    str << R"( BGCOLOR=")" + node.color.record + R"(">)";
    str << name;
    str << "</TD>";
    str << "<TD ";
    if (level == 0) {
      str << R"( PORT="cell")";
    }
    str << R"( BGCOLOR=")" + node.color.record_child + R"(">)";
    str << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0">)";
    for (const auto &f : rec.fields()) {
      str << "<TR><TD>";
      str << GenHTMLTableCell(*f->type(), f->name(), level + 1);
      str << "</TD></TR>";
    }
    str << "</TABLE>";
    str << "</TD>";
    str << "</TR></TABLE>";
  } else {
    str << name;
    if (t.Is(Type::VECTOR)) {
      auto vec = dynamic_cast<const Vector &>(t);
      auto width = vec.width();
      if (width) {
        str << "[" + width.value()->ToString() + "]";
      } else {
        str << "[..]";
      }
    }
  }
  return str.str();
}

std::string Style::GenDotRecordCell(const Type &t,
                                    const std::string &name,
                                    int level) {
  std::stringstream str;
  // Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn
  if (t.Is(Type::RECORD)) {
    auto rec = dynamic_cast<const Record &>(t);
    if (level == 0) {
      str << "<cell>";
    }
    str << name;
    str << "|";
    str << "{";
    auto record_fields = rec.fields();
    for (const auto &f : record_fields) {
      str << GenDotRecordCell(*f->type(), f->name(), level + 1);
      if (f != record_fields.back()) {
        str << "|";
      }
    }
    str << "}";
  } else {
    str << name;
  }
  return str.str();
}

std::string GraphGenerator::GenNode(const Node &n, int level) {
  std::stringstream str;
  if (n.IsExpression() && style.config.nodes.expand.expression) {
    str << GenExpr(n);
  } else {
    // Indent
    str << tab(level);
    str << NodeName(n);
    // Draw style
    str << " [";
    str << style.GetStyle(n);
    str << "];\n";
  }
  return str.str();
}

std::string GraphGenerator::GenNodes(const Graph &graph,
                                     Node::NodeID id,
                                     int level,
                                     bool no_group) {
  std::stringstream ret;
  auto nodes = graph.GetNodesOfType(id);
  auto arrays = graph.GetArraysOfType(id);
  if (!nodes.empty() || !arrays.empty()) {
    if (!no_group) {
      ret << tab(level) << "subgraph cluster_"
          << sanitize(graph.name()) + "_" + ToString(id) << " {\n";
      // ret << tab(level + 1) << "label=\"" << ToString(id) << "s\";\n";
      ret << tab(level + 1) << "rankdir=LR;\n";
      ret << tab(level + 1) << "label=\"\";\n";
      ret << tab(level + 1) << "style=" + style.nodegroup.base + ";\n";
      ret << tab(level + 1) << "color=\"" + style.nodegroup.color + "\";\n";
    }
    for (const auto &n : nodes) {
      ret << GenNode(*n, level + no_group + 1);
    }
    for (const auto &a : arrays) {
      ret << GenNode(*a->base(), level + no_group + 1);
    }
    if (!no_group) {
      ret << tab(level) << "}\n";
    }
  }
  return ret.str();
}

std::string GraphGenerator::GenGraph(const Graph &graph, int level) {
  std::stringstream ret;

  // (sub)graph header
  if (level == 0) {
    ret << "digraph {\n";

    // Preferably we would want to use splines=ortho, but dot is bugged when using html tables w.r.t. arrow directions
    // resulting from this setting
    ret << tab(level + 1) << "splines=ortho;\n";
    ret << tab(level + 1) << "rankdir=LR;\n";
  } else {
    ret << tab(level) << "subgraph cluster_" << sanitize(graph.name()) << " {\n";
    ret << tab(level + 1) << "rankdir=TB;\n";
    ret << tab(level + 1) << "style=" + style.subgraph.base + ";\n";
    ret << tab(level + 1) << "color=\"" + style.subgraph.color + "\";\n";
    ret << tab(level + 1) << "label=\"" << sanitize(graph.name()) << "\";\n";
  }

  // Nodes
  if (style.config.nodes.expressions)
    ret << GenNodes(graph, Node::NodeID::EXPRESSION, level + 1);

  if (style.config.nodes.literals)
    ret << GenNodes(graph, Node::NodeID::LITERAL, level + 1);

  if (style.config.nodes.parameters)
    ret << GenNodes(graph, Node::NodeID::PARAMETER, level + 1);

  if (style.config.nodes.ports)
    ret << GenNodes(graph, Node::NodeID::PORT, level + 1);

  if (style.config.nodes.signals)
    ret << GenNodes(graph, Node::NodeID::SIGNAL, level + 1, true);

  if (graph.IsComponent()) {
    const auto &comp = dynamic_cast<const Component &>(graph);
    if (!comp.children().empty()) {
      ret << "\n";
    }

    // Graph children
    for (const auto &child : comp.children()) {
      ret << GenGraph(*child, level + 1);
    }
    if (level == 0) {
      ret << GenEdges(graph, level + 1);
    }
  }

  ret << tab(level) << "}\n";

  return ret.str();
}

Status GraphGenerator::GenFile(const Graph &graph,
                               const std::filesystem::path &path) {
  std::string dot = GenGraph(graph);
  std::ofstream out(path);
  out << dot;
  out.close();
  return Status::OK();
}

std::string GraphGenerator::GenExpr(const Node &node,
                                    const std::string &prefix,
                                    int level) {
  std::stringstream str;

  std::string node_id;
  if (!prefix.empty()) {
    node_id = prefix + "_";
  }
  node_id += ToHex(node);

  if (level == 0) {
    str << "subgraph cluster_" + NodeName(node) + " {\n";
  }

  str << "\"" + node_id + "\" [label=\"" + sanitize(node.name()) + "\" ";
  if (level == 0) {
    str << ", color=red";
  }
  str << "];\n";
  if (node.IsExpression()) {
    auto expr = dynamic_cast<const Expression &>(node);
    auto left_node_id = node_id + "_" + ToHex(*expr.lhs());
    auto right_node_id = node_id + "_" + ToHex(*expr.rhs());
    str << "\"" + node_id + "\" -> \"" + left_node_id + "\"\n";
    str << "\"" + node_id + "\" -> \"" + right_node_id + "\"\n";
    str << GenExpr(*expr.lhs(), node_id, level + 1);
    str << GenExpr(*expr.rhs(), node_id, level + 1);
  }
  if (level == 0) {
    str << "}\n";
  }
  return str.str();
}

std::string NodeName(const Node &node, const std::string &suffix) {
  std::stringstream ret;
  if (node.parent()) {
    auto name = (*node.parent())->name();
    ret << name + ":" + ToString(node.node_id()) + ":";
  }
  if (node.IsExpression()) {
    ret << "Anon_" + ToString(node.node_id()) + "_" + ToHex(node);
  } else if (!node.name().empty()) {
    ret << node.name();
  }

  return sanitize(ret.str()) + suffix;
}

}  // namespace cerata::dot

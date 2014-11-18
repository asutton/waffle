
#include "nodes.hpp"
#include "debug.hpp"

#include <unordered_map>

namespace {

// Global table of node names. This is used primarily for debugging
// purposes.
static std::unordered_map<Node_kind, String> node_names_;

// Register the given name with the node category.
void
node_name(Node_kind k, const char* s) {
  lang_assert(node_names_.count(k) == 0, 
              format("node kind '{0}' already named", s));
  node_names_.insert({k, s});
}

} // namespace

// Initialize the node.
void
init_node(Node_kind k, const char* s) {
  node_name(k, s);
}

// Return a name associated with the node category.
String
node_name(Node_kind k) {
  auto iter = node_names_.find(k);
  if (iter != node_names_.end())
    return iter->second;
  else
    return "<unknown node>";
}

// Return a string representation of the node category.
String
node_name(Node* t) { return node_name(t->kind); }


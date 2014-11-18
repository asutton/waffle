
#ifndef NODES_HPP
#define NODES_HPP

#include "string.hpp"
#include "location.hpp"

#include <cstdint>

// -------------------------------------------------------------------------- //
// Node classification

// The node kind is a 32-bit integer value of the form:
//
//    kkkkkkkk NNNNNNNN NNNNNNNN NNNNNNNN
//
// Where the high-order k bits designate the class of the term, and
// the next 6 bits are various flags used for internal bookkeeping.
// These are currently reserved.
using Node_kind = std::uint32_t;

// The ndoe class characterizes the kinding level of the node. There
// are 4 levels: terms, types, kinds, and other.
using Node_class = std::uint32_t;
constexpr Node_class util_class = 0;
constexpr Node_class name_class = 1;
constexpr Node_class type_class = 2;
constexpr Node_class kind_class = 3;
constexpr Node_class term_class = 4;
constexpr Node_class stmt_class = 5;
constexpr Node_class decl_class = 6;
constexpr Node_class tree_class = 7;

constexpr Node_kind
make_node_class(Node_class k) { return k << 24; }

// Returns the node class.
constexpr Node_class
get_node_class(Node_kind k) { return k >> 24; }


// Returns true if the node is a utility node.
constexpr bool
is_util_node(Node_kind k) { return get_node_class(k) == util_class; }

// Returns true if the node is a type.
constexpr bool
is_type_node(Node_kind k) { return get_node_class(k) == type_class; }

// Returns true if the node is a kind.
constexpr bool
is_kind_node(Node_kind k) { return get_node_class(k) == kind_class; }

// Returns true if the node is a term.
constexpr bool
is_term_node(Node_kind k) { return get_node_class(k) >= term_class; }

// Returns true if the node is a statement.
constexpr bool
is_stmt_node(Node_kind k) { return get_node_class(k) >= stmt_class; }

// Returns true if the node is a declaration.
constexpr bool
is_decl_node(Node_kind k) { return get_node_class(k) == decl_class; }

constexpr bool
is_tree_node(Node_kind k) { return get_node_class(k) == tree_class; }

// Create a utility node kind.
constexpr Node_kind 
make_util_node(std::uint32_t n) { return n; }

// Create a name node with the given id.
constexpr Node_kind
make_name_node(std::uint32_t n) { return make_node_class(name_class) | n; }

// Create a type node with the given id.
constexpr Node_kind 
make_type_node(std::uint32_t n) { return make_node_class(type_class) | n; }

// Create a kind node with the given id.
constexpr Node_kind 
make_kind_node(std::uint32_t n) { return make_node_class(kind_class) | n; }

// Create a term node with the given id.
constexpr Node_kind 
make_term_node(std::uint32_t n) { return make_node_class(term_class) | n; }

// Create a statement node with the given id.
constexpr Node_kind
make_stmt_node(std::uint32_t n) { return make_node_class(stmt_class) | n; }

// Create a declaration node with the given id.
constexpr Node_kind
make_decl_node(std::uint32_t n) { return make_node_class(decl_class) | n; }

// Creeate a tree node with the given id.
constexpr Node_kind
make_tree_node(std::uint32_t n) { return make_node_class(tree_class) | n; }

// Initialization and query
void init_node(Node_kind, const char*);
String node_name(Node_kind);


// -------------------------------------------------------------------------- //
// Default nodes

constexpr Node_kind seq_node = make_util_node(1);


// -------------------------------------------------------------------------- //
// Nodes

// The base class of all terms and types.
struct Node {
  Node(Node_kind k) 
    : loc(no_location), kind(k) { }
  Node(Node_kind k, const Location& loc) 
    : loc(loc), kind(k) { }
  virtual ~Node() { }

  Node_kind kind;
  Location loc;
};


// The Seq class provides a facility for aggregating a sequence
// of nodes. This class also provides the same interface as
// std::vector<T*> where T is the type of aggregated node.
//
// Note that T must be derived from Node.
template<typename T>
  struct Seq : Node, std::vector<T*> {
    Seq()
      : Node(seq_node) { }
    Seq(std::initializer_list<T*> list)
      : Node(seq_node), std::vector<T*>(list) { }
    Seq(std::size_t n, T* p = nullptr)
      : Node(seq_node), std::vector<T*>(n, p) { }
  };


// -------------------------------------------------------------------------- //
// Conversion and testing

template<typename U, typename T> U* as(T* t);
template<typename U, typename T> const U* as(const T* t);
template<typename U, typename T> bool is(const T* t);

String node_name(Node*);

#include "nodes.ipp"

#endif

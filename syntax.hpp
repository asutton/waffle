
#ifndef SYNTAX_HPP
#define SYNTAX_HPP

#include "pretty.hpp"

#include "lang/nodes.hpp"
#include "lang/tokens.hpp"

constexpr Node_kind id_tree      = make_tree_node(1);   // identifiers
constexpr Node_kind lit_tree     = make_tree_node(2);   // values and types
constexpr Node_kind def_tree     = make_tree_node(100); // def x = t
constexpr Node_kind init_tree    = make_tree_node(101); // x=t
constexpr Node_kind var_tree     = make_tree_node(110); // x:T
constexpr Node_kind abs_tree     = make_tree_node(111); // \v.t
constexpr Node_kind fn_tree      = make_tree_node(112); // \(v*).t
constexpr Node_kind app_tree     = make_tree_node(113); // t1 t2
constexpr Node_kind func_tree    = make_tree_node(114); // n(v*)->T
constexpr Node_kind if_tree      = make_tree_node(120); // if t1 ...
constexpr Node_kind succ_tree    = make_tree_node(130); // succ t
constexpr Node_kind pred_tree    = make_tree_node(131); // pred t
constexpr Node_kind iszero_tree  = make_tree_node(132); // iszero t
constexpr Node_kind arrow_tree   = make_tree_node(140); // t1 -> t2
constexpr Node_kind tuple_tree   = make_tree_node(150); // {t1, ..., tn}
constexpr Node_kind list_tree    = make_tree_node(151); // [t1, ..., tn]
constexpr Node_kind variant_tree = make_tree_node(152); // <t1, ..., tn>
constexpr Node_kind comma_tree   = make_tree_node(153); // t1, ..., tn
constexpr Node_kind dot_tree     = make_tree_node(154); // t1.t2
constexpr Node_kind print_tree   = make_tree_node(200); // print t
constexpr Node_kind typeof_tree  = make_tree_node(201); // typeof t
constexpr Node_kind prog_tree    = make_tree_node(500); // stmts

struct Tree : Node { using Node::Node; };

using Tree_seq = Seq<Tree>;

struct Id_tree : Tree {
  Id_tree(const Token* k)
    : Tree(id_tree, k->loc), t1(k) { }

  const Token* value() const { return t1; }
  
  const Token* t1;
};

struct Lit_tree : Tree {
  Lit_tree(const Token* k)
    : Tree(lit_tree, k->loc), t1(k) { }

  const Token* value() const { return t1; }
  
  const Token* t1;
};

// A labeled initializer of the form 'x=t'.
struct Init_tree : Tree {
  Init_tree(Tree* n, Tree* t)
    : Tree(init_tree, n->loc), t1(n), t2(t) { }

  Tree* name() const { return t1; }
  Tree* term() const { return t2; }

  Tree* t1;
  Tree* t2;
};

struct Var_tree : Tree {
  Var_tree(Tree* t1, Tree* t2)
    : Tree(var_tree, t1->loc), t1(t1), t2(t2) { }

  Tree* id() const { return t1; }
  Tree* type() const { return t2; }

  Tree* t1;
  Tree* t2;
};

struct Abs_tree : Tree {
  Abs_tree(const Token* k, Tree* t1, Tree* t2)
    : Tree(abs_tree, k->loc), t1(t1), t2(t2) { }

  Tree* var() const { return t1; }
  Tree* term() const { return t2; }
  
  Tree* t1;
  Tree* t2;
};

struct Fn_tree : Tree {
  Fn_tree(const Token* k, Tree_seq* t1, Tree* t2)
    : Tree(fn_tree, k->loc), t1(t1), t2(t2) { }

  Tree_seq* parms() const { return t1; }
  Tree* term() const { return t2; }
  Tree_seq* t1;
  Tree* t2;
};

struct Func_tree : Tree {
  Func_tree(Tree* n, Tree_seq* t2, Tree* t3)
    : Tree(func_tree, n->loc), t1(n), t2(t2), t3(t3) { }

  Tree* name() const{ return t1; }   
  Tree_seq* parms() const { return t2; }
  Tree* type() const { return t3; }
  
  Tree_seq* t2;
  Tree* t1;
  Tree* t3;
};


struct App_tree : Tree {
  App_tree(Tree* t1, Tree* t2)
    : Tree(app_tree, t1->loc), t1(t1), t2(t2) { }

  Tree* fn() const { return t1; }
  Tree* arg() const { return t2; }

  Tree* t1;
  Tree* t2;
};

struct If_tree : Tree {
  If_tree(const Token* k, Tree* t1, Tree* t2, Tree* t3)
    : Tree(if_tree, k->loc), t1(t1), t2(t2), t3(t3) { }

  Tree* cond() const { return t1; }
  Tree* if_true() const { return t2; }
  Tree* if_false() const  { return t3; }

  Tree* t1;
  Tree* t2;
  Tree* t3;
};

struct Succ_tree : Tree {
  Succ_tree(const Token* k, Tree* t)
    : Tree(succ_tree, k->loc), t1(t) { }

  Tree* arg() const { return t1; }

  Tree* t1;
};

struct Pred_tree : Tree {
  Pred_tree(const Token* k, Tree* t)
    : Tree(pred_tree, k->loc), t1(t) { }

  Tree* arg() const { return t1; }

  Tree* t1;
};

struct Iszero_tree : Tree {
  Iszero_tree(const Token* k, Tree* t)
    : Tree(iszero_tree, k->loc), t1(t) { }

  Tree* arg() const { return t1; }

  Tree* t1;
};

struct Arrow_tree : Tree {
  Arrow_tree(Tree* t1, Tree* t2)
    : Tree(arrow_tree, t1->loc), t1(t1), t2(t2) { }

  Tree* left() const { return t1; }
  Tree* right() const { return t2; }

  Tree* t1;
  Tree* t2;
};

struct Def_tree : Tree {
  Def_tree(const Token* k, Tree* n, Tree* e)
    : Tree(def_tree, k->loc), t1(n), t2(e) { }

  Tree* name() const { return t1; }
  Tree* value() const { return t2; }

  Tree* t1;
  Tree* t2;
};

struct Print_tree : Tree {
  Print_tree(const Token* k, Tree* t)
    : Tree(print_tree, k->loc), t1(t) { }

  Tree* expr() const { return t1; }

  Tree* t1;
};

struct Typeof_tree : Tree {
  Typeof_tree(const Token* k, Tree* t)
    : Tree(typeof_tree, k->loc), t1(t) { }

  Tree* expr() const { return t1; }

  Tree* t1;
};

// A tuple of the form '{t1, ..., tn}' where each ti is one of
// a term, a variable of the form 'x:T', or an initializer of 
// the form 'x=t'. This is used to represent both tuples and 
// records, and their corresponding types.
struct Tuple_tree : Tree {
  Tuple_tree(const Token* k, Tree_seq* ts)
    : Tree(tuple_tree, k->loc), t1(ts) { }

  Tree_seq* elems() const { return t1; }

  Tree_seq* t1;
};

// A list of the form '[t1, ..., tn]' where each 'ti' is simply
// some other term.
struct List_tree : Tree {
  List_tree(const Token* k, Tree_seq* ts)
    : Tree(list_tree, k->loc), t1(ts) { }

  Tree_seq* elems() const { return t1; }

  Tree_seq* t1;
};

// A variant of the form '<t1, ..., tn>' where each ti is a
// a variable of the form 'x:T' or a member of the form 'x=t'.
//
// TODO: Can we allow arbitrary terms? <true, 0> as if the
// variant type were of the form <0=true, 1=0>?
struct Variant_tree : Tree {
  Variant_tree(const Token* k, Tree_seq* ts)
    : Tree(variant_tree, k->loc), t1(ts) { }

  Tree_seq* elems() const { return t1; }

  Tree_seq* t1;
};

// A comma-separated sequence of terms.
struct Comma_tree : Tree {
  Comma_tree(const Token* k, Tree_seq* ts)
    : Tree(comma_tree, k->loc), t1(ts) { }

  Tree_seq* elems() const { return t1; }

  Tree_seq* t1;
};

// An expression of the form 't1.t2'.
struct Dot_tree : Tree {
  Dot_tree(Tree* t1, Tree* t2)
    : Tree(dot_tree, t1->loc), t1(t1), t2(t2) { }

  Tree* object() const { return t1; }
  Tree* elem() const { return t2; }

  Tree* t1;
  Tree* t2;
};

// A complete program.
struct Prog_tree : Tree {
  Prog_tree(Tree_seq* ts)
    : Tree(prog_tree, no_location), t1(ts) { }
  
  Tree_seq* stmts() const { return t1; }

  Tree_seq* t1;
};

// -------------------------------------------------------------------------- //
// Pretty printing

// Pretty node constructor
inline pretty_printer<Tree> 
pretty(Tree* t) { return pretty_printer<Tree>{t}; }

std::ostream& operator<<(std::ostream&, pretty_printer<Tree>);

#endif

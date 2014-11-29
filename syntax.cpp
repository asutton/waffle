
#include "syntax.hpp"

#include "lang/debug.hpp"

#include <iostream>

void
init_trees() {
  init_node(id_tree, "id-tree");
  init_node(lit_tree, "lit-tree");
  init_node(def_tree, "def-tree");
  init_node(init_tree, "init-tree");
  init_node(var_tree, "var-tree");
  init_node(init_tree, "init-tree");
  init_node(abs_tree, "abs-tree");
  init_node(app_tree, "app-tree");
  init_node(if_tree, "if-tree");
  init_node(succ_tree, "succ-tree");
  init_node(pred_tree, "pred-tree");
  init_node(iszero_tree, "iszero-tree");
  init_node(arrow_tree, "arrow-tree");
  init_node(print_tree, "print-tree");
  init_node(typeof_tree, "typeof-tree");
  init_node(tuple_tree, "tuple-tree");
  init_node(list_tree, "list-tree");
  init_node(variant_tree, "variant-tree");
  init_node(comma_tree, "comma-tree");
  init_node(dot_tree, "dot-tree");
  init_node(select_tree, "select-tree");
  init_node(join_on_tree, "join-on-tree");
  init_node(union_tree, "union-tree");
  init_node(intersect_tree, "intersect-tree");
  init_node(except_tree, "except-tree");
  init_node(and_tree, "and-tree");
  init_node(or_tree, "or-tree");
  init_node(not_tree, "not-tree");
  init_node(eq_comp_tree, "eq-comp-tree");
  init_node(less_tree, "less-tree");
  init_node(prog_tree, "prog-tree");
}

// -------------------------------------------------------------------------- //
// Pretty printing

bool
is_terminal(Tree* t) {
  return t->kind == id_tree || t->kind == lit_tree;
}

namespace {

void
pp_var(std::ostream& os, Var_tree* t) { 
  os << pretty(t->t1) << ':' << pretty(t->t2); 
}

void
pp_init(std::ostream& os, Init_tree* t) {
  os << pretty(t->t1) << '=' << pretty(t->t2); 
}

void
pp_abs(std::ostream& os, Abs_tree* t) { 
  os << '\\' << pretty(t->t1) << '.' << group(pretty(t->t2)); 
}

void
pp_fn(std::ostream& os, Fn_tree* t) { 
  os << '\\' <<  commas(t->parms()) << "=>" << group(pretty(t->term())); 
}

void
pp_func(std::ostream& os, Func_tree* t) { 
  os << pretty(t->name())<<" ("<<commas(t->parms()) << " ) ->" << group(pretty(t->type())); 
}

void
pp_app(std::ostream& os, App_tree* t) {
  os << '(' << pretty(t->t1) << ' ' << pretty(t->t2) << ')';
}

void
pp_if(std::ostream& os, If_tree* t) {
  os << "if " << group(pretty(t->t1))
     << " then " << group(pretty(t->t2))
     << " else " << group(pretty(t->t3));
}

void
pp_succ(std::ostream& os, Succ_tree* t) {
  os << "succ " << group(pretty(t->t1));
}

void
pp_pred(std::ostream& os, Pred_tree* t) {
  os << "pred " << group(pretty(t->t1));
}

void
pp_iszero(std::ostream& os, Iszero_tree* t) {
  os << "iszero " << group(pretty(t->t1));
}

void
pp_arrow(std::ostream& os, Arrow_tree* t) {
  os << pretty(t->left()) << " -> " << group(pretty(t->right()));
}

void
pp_def(std::ostream& os, Def_tree* t) {
  os << "def " << pretty(t->name()) << " = " << pretty(t->value());
}

void
pp_print(std::ostream& os, Print_tree* t) {
  os  << "print " << pretty(t->expr());
}

void
pp_typeof(std::ostream& os, Typeof_tree* t) {
  os  << "typeof " << pretty(t->expr());
}

void
pp_tuple(std::ostream& os, Tuple_tree* t) {
  os << '{' << commas(t->elems()) << '}';
}

void
pp_list(std::ostream& os, List_tree* t) {
  os << '[' << commas(t->elems()) << ']';
}

void
pp_variant(std::ostream& os, Variant_tree* t) {
  os << '<' << commas(t->elems()) << '\n';
}

void
pp_comma(std::ostream& os, Comma_tree* t) {
  os << '(' << commas(t->elems()) << ')';
}

void
pp_dot(std::ostream& os, Dot_tree* t) {
  os << pretty(t->object()) << '.' << pretty(t->elem());
}

void
pp_select(std::ostream& os, Select_tree* t) {
  os << "select " << pretty(t->t1) 
     << " from " << pretty(t->t2) 
     << " where " << pretty(t->t3);
}

void
pp_join(std::ostream& os, Join_on_tree* t) {
  os << pretty(t->t1) << " join " << pretty(t->t2) 
                   << " on "  << pretty(t->t3);
}

void
pp_union(std::ostream& os, Union_tree* t) {
  os << pretty(t->t1) << " union " << pretty(t->t2);
}

void
pp_intersect(std::ostream& os, Intersect_tree* t) {
  os << pretty(t->t1) << " intersect " << pretty(t->t2);
}

void
pp_except(std::ostream& os, Except_tree* t) {
  os << pretty(t->t1) << " except " << pretty(t->t2);
}

void
pp_and(std::ostream& os, And_tree* t) {
  os << pretty(t->t1) << " and " << pretty(t->t2);
}

void
pp_or(std::ostream& os, Or_tree* t) {
  os << pretty(t->t1) << " or " << pretty(t->t2);
}

void
pp_eq_comp(std::ostream& os, Eq_comp_tree* t) {
  os << pretty(t->t1) << " == " << pretty(t->t2);
}

void
pp_less(std::ostream& os, Less_tree* t) {
  os << pretty(t->t1) << " < " << pretty(t->t2);
}

void
pp_not(std::ostream& os, Not_tree* t) {
  os << "not " << pretty(t->t1);
}

void
pp_prog(std::ostream& os, Prog_tree* t) {
  for (Tree* s : *t->stmts())
    os << pretty(s) << ';' << '\n';
}

} // namespace

// Render the given term into the output stream.
void
print(std::ostream& os, Tree* t) {
  if (not t) {
    os << "<null>";
    return;
  }

  switch (t->kind) {
  case id_tree: return pp_terminal(os, as<Id_tree>(t));
  case lit_tree: return pp_terminal(os, as<Lit_tree>(t));
  case var_tree: return pp_var(os, as<Var_tree>(t));
  case init_tree: return pp_init(os, as<Init_tree>(t));
  case abs_tree: return pp_abs(os, as<Abs_tree>(t));
  case fn_tree: return pp_fn(os, as<Fn_tree>(t));
  case func_tree: return pp_func(os, as<Func_tree>(t));
  case app_tree: return pp_app(os, as<App_tree>(t));
  case if_tree:  return pp_if(os, as<If_tree>(t));
  case succ_tree: return pp_succ(os, as<Succ_tree>(t));
  case pred_tree: return pp_pred(os, as<Pred_tree>(t));
  case iszero_tree: return pp_iszero(os, as<Iszero_tree>(t));
  case arrow_tree: return pp_arrow(os, as<Arrow_tree>(t));
  case def_tree: return pp_def(os, as<Def_tree>(t));
  case print_tree: return pp_print(os, as<Print_tree>(t));
  case typeof_tree: return pp_typeof(os, as<Typeof_tree>(t));
  case tuple_tree: return pp_tuple(os, as<Tuple_tree>(t));
  case list_tree: return pp_list(os, as<List_tree>(t));
  case variant_tree: return pp_variant(os, as<Variant_tree>(t));
  case comma_tree: return pp_comma(os, as<Comma_tree>(t));
  case dot_tree: return pp_dot(os, as<Dot_tree>(t));
  case select_tree: return pp_select(os, as<Select_tree>(t));
  case join_on_tree: return pp_join(os, as<Join_on_tree>(t));
  case union_tree: return pp_union(os, as<Union_tree>(t));
  case intersect_tree: return pp_intersect(os, as<Intersect_tree>(t));
  case except_tree: return pp_except(os, as<Except_tree>(t));
  case and_tree: return pp_and(os, as<And_tree>(t));
  case or_tree: return pp_or(os, as<Or_tree>(t));
  case not_tree: return pp_not(os, as<Not_tree>(t));
  case eq_comp_tree: return pp_eq_comp(os, as<Eq_comp_tree>(t));
  case less_tree: return pp_less(os, as<Less_tree>(t));
  case prog_tree: return pp_prog(os, as<Prog_tree>(t));
  }
  lang_unreachable(format("print unknown node '{}'", node_name(t)));
}

std::ostream&
operator<<(std::ostream& os, pretty_printer<Tree> t) {
  print(os, t.node);
  return os;
}

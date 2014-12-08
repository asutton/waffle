
#include "ast.hpp"
#include "type.hpp"
#include "value.hpp"

#include "lang/debug.hpp"

#include <iostream>
#include <unordered_map>

void
init_nodes() {
  // Terms
  init_node(def_term, "def");
  init_node(init_term, "init");
  init_node(unit_term, "unit");
  init_node(true_term, "true");
  init_node(false_term, "false");
  init_node(int_term, "int");
  init_node(var_term, "var");
  init_node(abs_term, "abs");
  init_node(app_term, "app");
  init_node(tuple_term, "tuple");
  init_node(list_term, "list");
  init_node(record_term, "record");
  init_node(comma_term, "comma");
  init_node(proj_term, "proj");
  init_node(mem_term, "mem");
  // Types
  init_node(kind_type, "kind-type");
  init_node(unit_type, "unit-type");
  init_node(bool_type, "bool-type");
  init_node(nat_type, "nat-type");
  init_node(str_type, "str-type");
  init_node(arrow_type, "arrow-type");
  init_node(tuple_type, "tuple-type");
  init_node(list_type, "list-type");
}

// -------------------------------------------------------------------------- //
// Pretty printing

bool
is_term_literal(Term* t) {
  return is_unit(t) 
      or is_boolean_value(t) 
      or is_integer_value(t) 
      or is_string_value(t);
}

bool
is_type_literal(Type* t) {
  return is_unit_type(t) 
      or is_bool_type(t) 
      or is_nat_type(t)
      or is_str_type(t);
}

// Returns true if t is a term literal or type literal.
bool
is_literal(Expr* t) {
  if (Term* t0 = as<Term>(t))
    return is_term_literal(t0);
  if (Type* t0 = as<Type>(t))
    return is_type_literal(t0);
  return false;
}

// Returns true when t is a reference.
bool
is_identifier(Expr* t) {
  return t->kind == ref_term;
}

// Returns true if t is a terminal node. In the simply typed
// lambda calculus this is true for only varirables and base
// types. This is usd to determine when to put parens around
// a term when pretty printing.
bool
is_terminal(Expr* t) { 
  return is_literal(t) || t->kind == ref_term;
}

namespace {

// TODO: This should move into the printing support library.
template<typename T>
  void
  pp_value(std::ostream& os, const T& x) { os << x; }

void
pp_if(std::ostream& os, If* t) {
  os << "if " << group(pretty(t->cond()))
     << " then " << group(pretty(t->if_true()))
     << " else " << group(pretty(t->if_false()));
}

void
pp_succ(std::ostream& os, Succ* t) {
  os << "succ " << group(pretty(t->arg()));
}

void
pp_pred(std::ostream& os, Pred* t) {
  os << "pred " << group(pretty(t->arg()));
}

void
pp_iszero(std::ostream& os, Iszero* t) {
  os << "iszero " << group(pretty(t->arg()));
}

void
pp_var(std::ostream& os, Var* t) { 
  os << pretty(t->name()) << ':' << pretty(t->type()); 
}

void
pp_abs(std::ostream& os, Abs* t) { 
  os << '\\' << pretty(t->var()) << "=>" << group(pretty(t->term())); 
}

void
pp_fn(std::ostream& os, Fn* t) { 
  os << "\\(" << commas(t->parms()) << ")=>" << group(pretty(t->term())); 
}

void
pp_app(std::ostream& os, App* t) {
  os << '(' << pretty(t->t1) << ' ' << pretty(t->t2) << ')';
}

void
pp_call(std::ostream& os, Call* t) {
  os << pretty(t->fn()) << '(' << commas(t->args()) << ')';
}

void
pp_def(std::ostream& os, Def* t) {
  os << "def " << pretty(t->name()) << " = " << group(pretty(t->value()));
}

void
pp_func(std::ostream& os, Func* t) { 
  os << "(" << commas(t->parms()) << ") ->" <<pretty(t->returntype())<<" . "<< group(pretty(t->term())); 
}

void
pp_init(std::ostream& os, Init* t) {
  os << pretty(t->name()) << " = " << pretty(t->value());
}

void
pp_tuple(std::ostream& os, Tuple* t) {
  os << '{' << commas(t->elems()) << '}';
}

void
pp_list(std::ostream& os, List* t) {
  os << '[' << commas(t->elems()) << ']';
}

void
pp_record(std::ostream& os, Record* t) {
  os << '{' << commas(t->members()) << '}';
}

void
pp_comma(std::ostream& os, Comma* t) {
  os << '(' << commas(t->elems()) << ')';
}

void
pp_proj(std::ostream& os, Proj* t) {
  os << pretty(t->tuple()) << '.' << pretty(t->elem());
}

void
pp_mem(std::ostream& os, Mem* t) {
  os << pretty(t->record()) << '.' << pretty(t->member());
}

template<typename T>
  void 
  pp_ref_id(std::ostream& os, T* t) {
    os << pretty(t->name());
  }

// Print the name of the referred to expression.
void
pp_ref(std::ostream& os, Ref* t) {
  Expr* e = t->decl();
  if (Var* v = as<Var>(e))
    return pp_ref_id(os, v);
  if (Def* d = as<Def>(e))
    return pp_ref_id(os, d);
  lang_unreachable(format("print unhandled reference to '{}'", pretty(e)));
}

void
pp_print(std::ostream& os, Print* p) {
  os << "print " << pretty(p->expr());
}

void
pp_prog(std::ostream& os, Prog* t) {
  for (Term* s : *t->stmts())
    os << pretty(s) << ';' << '\n';
}

void
pp_arrow_type(std::ostream& os, Arrow_type* t) { 
  os << pretty(t->t1) << " -> " << group(pretty(t->t2));
}

void
pp_fn_type(std::ostream& os, Fn_type* t) { 
  os << '(' << commas(t->parms()) << ")->" << group(pretty(t->result())); 
}

void
pp_tuple_type(std::ostream& os, Tuple_type* t) {
  os << '{' << commas(t->types()) << '}';
}

void
pp_list_type(std::ostream& os, List_type* t) {
  os << '[' << pretty(t->type()) << ']';
}

void
pp_record_type(std::ostream& os, Record_type* t) {
  os << '{' << commas(t->members()) << '}';
}

// Print the wildcard type. Omit the explicit type qualifier
// if the wildcard is actually a type variable.
void
pp_wild_type(std::ostream& os, Wild_type* t) {
  os << '*' << pretty(t->name());
  if (not is_kind(t->type()))
     os << ':' << pretty(t->type());
}

} // namespace

// Render the given term into the output stream.
void
pp_expr(std::ostream& os, Node* t) {
  if (not t) {
    os << "<null>";
    return;
  }

  switch(t->kind) {
  // Names
  case id_expr: return pp_terminal(os, as<Id>(t));
  // Terms
  case unit_term: return pp_string(os, "unit");
  case true_term: return pp_string(os, "true");
  case false_term: return pp_string(os, "false");
  case int_term: return pp_value(os, as<Int>(t)->value());
  case str_term: return pp_value(os, as<Str>(t)->value());
  case if_term: return pp_if(os, as<If>(t));
  case succ_term: return pp_succ(os, as<Succ>(t));
  case pred_term: return pp_pred(os, as<Pred>(t));
  case iszero_term: return pp_iszero(os, as<Iszero>(t));
  case var_term: return pp_var(os, as<Var>(t));
  case abs_term: return pp_abs(os, as<Abs>(t));
  case fn_term: return pp_fn(os, as<Fn>(t));
  case func_term: return pp_func(os, as<Func>(t));
  case app_term: return pp_app(os, as<App>(t));
  case call_term: return pp_call(os, as<Call>(t));
  case ref_term: return pp_ref(os, as<Ref>(t));
  case def_term: return pp_def(os, as<Def>(t));
  case init_term: return pp_init(os, as<Init>(t));
  case tuple_term: return pp_tuple(os, as<Tuple>(t));
  case list_term: return pp_list(os, as<List>(t));
  case record_term: return pp_record(os, as<Record>(t));
  case comma_term: return pp_comma(os, as<Comma>(t));
  case proj_term: return pp_proj(os, as<Proj>(t));
  case print_term: return pp_print(os, as<Print>(t));
  case prog_term: return pp_prog(os, as<Prog>(t));
  // Types
  case unit_type: return pp_string(os, "Unit");
  case bool_type: return pp_string(os, "Bool");
  case nat_type: return pp_string(os, "Nat");
  case str_type: return pp_string(os, "Str");
  case arrow_type: return pp_arrow_type(os, as<Arrow_type>(t));
  case fn_type: return pp_fn_type(os, as<Fn_type>(t));
  case tuple_type: return pp_tuple_type(os, as<Tuple_type>(t));
  case list_type: return pp_list_type(os, as<List_type>(t));
  case record_type: return pp_record_type(os, as<Record_type>(t));
  case wild_type: return pp_wild_type(os, as<Wild_type>(t));
  default: break;
  }
  lang_unreachable(format("print unknown node '{}'", node_name(t)));
}

std::ostream&
operator<<(std::ostream& os, pretty_printer<Expr> t) {
  pp_expr(os, t.node);
  return os;
}

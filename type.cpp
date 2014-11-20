
#include "ast.hpp"


// -------------------------------------------------------------------------- //
// Built-in types

namespace {

Kind_type* kind_type_;
Unit_type* unit_type_;
Bool_type* bool_type_;
Nat_type* nat_type_;
Str_type* str_type_;

} // namespace

void
init_types() {
  kind_type_ = new Kind_type();
  unit_type_ = new Unit_type(kind_type_);
  bool_type_ = new Bool_type(kind_type_);
  nat_type_ = new Nat_type(kind_type_);
  str_type_ = new Str_type(kind_type_);
}

Type*
get_kind_type() { return kind_type_; }

Type*
get_unit_type() { return unit_type_; }

Type*
get_bool_type() { return bool_type_; }

Type*
get_nat_type() { return nat_type_; }

Type*
get_str_type() { return str_type_; }


// -------------------------------------------------------------------------- //
// Typing

bool
is_type(Expr* e) { return is<Type>(e); }

// Returns true if t is the unit type.
bool
is_unit_type(Type* t) { return t->kind == unit_type; }

// Returns true if t is the bool type.
bool
is_bool_type(Type* t) { return t->kind == bool_type; }

// Returns true if t is the nat type.
bool
is_nat_type(Type* t) { return t->kind == nat_type; }

// Returns true if t is the str type.
bool
is_str_type(Type* t) { return t->kind == str_type; }

// Returns true if t is an arrow type.
bool 
is_arrow_type(Type* t) { return t->kind == arrow_type; }

// Returns true if t is the kind of a type.
bool
is_kind(Expr* e) { return e->kind == kind_type; }


// Returns the type of an expression.
Type*
get_type(Expr* e) { return e->tr; }

// Return a sequence of types for the sequence of terms.
Type_seq*
get_type(Term_seq* e) { 
  Type_seq* types = new Type_seq();
  for (Term* t : *e)
    types->push_back(get_type(t));
  return types; 
}



#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "ast.hpp"

#include <map>

// Determines the kind of scope.
enum Scope_kind {
  global_scope,
  lambda_scope,
  func_scope
};

// A scope records a set of named terms (e.g., variables), allowing 
// the lookup of bound identifiers. Each scope is linked to its 
// parent or enclosing scope, allowing lookup to work "outwards" 
// as a declaration corresponding to that name is searched for.
struct Scope : std::map<Name*, Expr*, Expr_less> {
  Scope(Scope_kind k)
    : kind(k), parent(nullptr), counter(0) { }
  Scope(Scope_kind k, Scope* p)
    : kind(k), parent(p), counter(0) { }

  Scope_kind kind;
  Scope* parent;
  int counter;
};

void push_scope(Scope_kind);
void pop_scope();
Scope* current_scope();

bool in_global_scope();
bool in_lambda_scope();
bool in_func_scope();

Expr* declare(Name*, Expr*);
Expr* declare(Expr*);
Expr* lookup(Name*);
Expr* declare_outside(Expr*);

Name* fresh_name();

// A helper class that guarantees that a scope is popped
// when it goes out of scope.
struct Scope_guard {
  Scope_guard(Scope_kind k) { push_scope(k); }
  ~Scope_guard() { pop_scope(); }
};

#endif


#include "scope.hpp"

#include "lang/error.hpp"
#include "lang/debug.hpp"

namespace {

// The global current scope.
Scope* current_scope_ = nullptr;

} // namespace

void
push_scope(Scope_kind k) {
  Scope* s = new Scope(k, current_scope_);
  current_scope_ = s;
}

void
pop_scope() {
  lang_assert(current_scope_, "no current scope");
  Scope* s = current_scope_->parent;
  delete current_scope_;
  current_scope_ = s;
}

// Returns the current scope.
Scope* 
current_scope() {
  lang_assert(current_scope_, "no current scope");
  return current_scope_;
}

// Returns true if the system is currently in global scope.
bool
in_global_scope() { return current_scope()->kind == global_scope; }

// Returns true if the system is currently in lambda scope.
bool
in_lambda_scope() { return current_scope()->kind == lambda_scope; }

// Associate the term t with the name n in the current scope.
Expr*
declare(Name* n, Expr* e) {
  Scope* s = current_scope();
  if (s->count(n) != 0) {
    error(e->loc) << format("name '{}' already bound in this scope", pretty(n));
    return nullptr;
  }
  s->insert({n, e});
  return e;
}

// Save the named term t in the current scope.
Expr*
declare(Expr* t) {
  if (Var* v = as<Var>(t))
    return declare(v->name(), v);
  if (Def* d = as<Def>(t))
    return declare(d->name(), d);
  lang_unreachable(format("cannot declare expression '{}'", node_name(t)));
}

// Return the declaration associated with the name n,
// or nullptr if no such name exists.
Expr*
lookup(Name* n) {
  Scope* s = current_scope();
  while (s) {
    auto iter = s->find(n);
    if (iter != s->end())
      return iter->second;
    s = s->parent;
  }
  return nullptr;
}

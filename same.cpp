
#include "ast.hpp"

#include "lang/debug.hpp"

#include <cassert>

// -------------------------------------------------------------------------- //
// Same term
//
// The same-term relation defines when two terms are exactly the same.
// We can define the members of this relation using inference rules.

namespace {

inline bool
is_same(String a, String b) { return a == b; }

bool
same_value(const Integer& a, const Integer& b) {
  return a == b;
}

template<typename T>
  inline bool
  same_unary(T* a, T* b) {
    return is_same(a->t1, b->t1);
  }

template<typename T>
  inline bool
  same_binary(T* a, T* b) {
    if(is_same(a->t1, b->t1))
      return true;
    if (is_same(b->t1, a->t1))
      return false;
    return is_same(a->t2, b->t2);
  }

template<typename T>
  inline bool
  same_ternary(T* a, T* b) {
    if(is_same(a->t1, b->t1))
      return true;
    if (is_same(b->t1, a->t1))
      return false;
    if(is_same(a->t2, b->t2))
      return true;
    if (is_same(b->t2, a->t2))
      return false;
    return is_same(a->t3, b->t3);
  }

// Two refs are the same when they refer to the same declaration.
inline bool
same_ref(Ref* a, Ref* b) {
  return a->decl() == b->decl();
}

} // namespace


bool
is_same(Expr* a, Expr* b) {
  if (a->kind != b->kind)
    return false;
  switch (a->kind) {
  case id_expr: return same_unary(as<Id>(a), as<Id>(b));
  case unit_term: return true;
  case true_term: return true;
  case false_term: return true;
  case int_term: return as<Int>(a)->value() == as<Int>(b)->value();
  case if_term: return same_ternary(as<If>(a), as<If>(b));
  case succ_term: return same_unary(as<Succ>(a), as<Succ>(b));
  case pred_term: return same_unary(as<Pred>(a), as<Pred>(b));
  case iszero_term: return same_unary(as<Iszero>(a), as<Iszero>(b));
  case var_term: return same_binary(as<Var>(a), as<Var>(b));
  case abs_term: return same_binary(as<Var>(a), as<Var>(b));
  case app_term: return same_binary(as<Var>(a), as<Var>(b));
  case ref_term: return same_ref(as<Ref>(a), as<Ref>(b));
  case kind_type: return true;
  case unit_type: return true;
  case bool_type: return true;
  case nat_type: return true;
  case arrow_type: return same_binary(as<Arrow_type>(a), as<Arrow_type>(b));

  }
}

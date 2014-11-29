
#include "ast.hpp"

#include "lang/debug.hpp"

#include <cassert>

// -------------------------------------------------------------------------- //
// Less-than-comparison for expressions
//
// The less-than comparison for expresssions weakly orders them by
// their kind and their subterms.

namespace {

inline bool
is_less(String a, String b) { return a < b; }

template<typename T>
  inline bool
  less_unary(T* a, T* b) {
    return is_less(a->t1, b->t1);
  }

template<typename T>
  inline bool
  less_binary(T* a, T* b) {
    if(is_less(a->t1, b->t1))
      return true;
    if (is_less(b->t1, a->t1))
      return false;
    return is_less(a->t2, b->t2);
  }

template<typename T>
  inline bool
  less_ternary(T* a, T* b) {
    if(is_less(a->t1, b->t1))
      return true;
    if (is_less(b->t1, a->t1))
      return false;
    if(is_less(a->t2, b->t2))
      return true;
    if (is_less(b->t2, a->t2))
      return false;
    return is_less(a->t3, b->t3);
  }

} // namespace

bool
is_less(Expr* a, Expr* b) {
  if (a->kind < b->kind)
    return true;
  if (b->kind < a->kind)
    return false;
  switch(a->kind) {
  case id_expr: return less_unary(as<Id>(a), as<Id>(b));
  case unit_term: return false;
  case true_term: return false;
  case false_term: return false;
  case int_term: return as<Int>(a)->value() < as<Int>(b)->value();
  case if_term: return less_ternary(as<If>(a), as<If>(b));
  case succ_term: return less_unary(as<Succ>(a), as<Succ>(b));
  case pred_term: return less_unary(as<Pred>(a), as<Pred>(b));
  case iszero_term: return less_unary(as<Iszero>(a), as<Iszero>(b));
  case var_term: return less_binary(as<Var>(a), as<Var>(b));
  case abs_term: return less_binary(as<Abs>(a), as<Abs>(b));
  case app_term: return less_binary(as<App>(a), as<App>(a));
  case ref_term: return less_unary(as<Ref>(a), as<Ref>(b));
  case def_term: return less_unary(as<Def>(a), as<Def>(b));
  case kind_type: return false;
  case unit_type: return false;
  case bool_type: return false;
  case nat_type: return false;
  case arrow_type: return less_binary(as<Arrow_type>(a), as<Arrow_type>(b));
  default: break;
  }
  lang_unreachable(format("comparison of unknown node '{}'", node_name(a)));
}


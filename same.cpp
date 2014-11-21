
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

// Two Init terms are the same if they have the same Name and expr
// This was necessary because same_binary was returning true even though it wasn't
inline bool
same_init(Init* a, Init* b) {
  return (is_same(a->name(), b->name()) and is_same(a->value(), b->value()));
}

// Two Var terms are the same if they have the same name and type
// This was necessary because same_binary was returning true even though it wasn't
inline bool
same_var(Var* a, Var* b) {
  return (is_same(a->name(), b->name()) and is_same(a->type(), b->type()));
}

// Two record types are the same if each respective label and type 
inline bool
same_record_type(Record_type* a, Record_type* b) {
  if(a->members()->size() == b->members()->size()) {
    Term_seq* type_a = a->members();
    Term_seq* type_b = b->members();

    auto it_a = type_a->begin();
    auto it_b = type_b->begin();
    for(it_a; it_a != type_a->end(); ++it_a) {
      if (!is_same(*it_a, *it_b))
        return false;
      ++it_b;
    }
    return true;
  }
  else
    return false;
}

// Two records are the same if every subterm of type Init is the same
inline bool
same_record(Record* a, Record* b) {
  if(a->members()->size() == b->members()->size()) {
    auto it_a = a->members()->begin();
    auto it_b = b->members()->begin();
    for(it_a; it_a != a->members()->end(); ++it_a) {
      if(!is_same(*it_a, *it_b)) {
        return false;
      }
      ++it_b;
    }
  }
  else {
    return false;
  }

  return true;
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
  case var_term: return same_var(as<Var>(a), as<Var>(b));
  case abs_term: return same_binary(as<Var>(a), as<Var>(b));
  case app_term: return same_binary(as<Var>(a), as<Var>(b));
  case ref_term: return same_ref(as<Ref>(a), as<Ref>(b));
  case init_term: return same_init(as<Init>(a), as<Init>(b));
  case record_term: return same_record(as<Record>(a), as<Record>(b));
  case kind_type: return true;
  case unit_type: return true;
  case bool_type: return true;
  case nat_type: return true;
  case arrow_type: return same_binary(as<Arrow_type>(a), as<Arrow_type>(b));
  case record_type: return same_record_type(as<Record_type>(a), as<Record_type>(b));
  case list_type: return is_same(as<List_type>(a)->type(), as<List_type>(b)->type());
  }
}

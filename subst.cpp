#include <iostream>

#include "subst.hpp"
#include "ast.hpp"
#include "type.hpp"

#include "lang/debug.hpp"

// -------------------------------------------------------------------------- //
// Substitution class

// Construct a substitution mapping the declaration 'x' to the
// replacement term 's'.
Subst::Subst(Expr* x, Expr* s) {
  insert({x, s});
}

// Return the substitution for the binding b. 
Expr*
Subst::get(Expr* b) const {
  auto iter = find(b);
  if (iter != end())
    return iter->second;
  else
    return nullptr;
}

// -------------------------------------------------------------------------- //
// Substitution rules

namespace {

// Substitute into a unary term of the form 'op t'
//
//    [x->s]op t = op [x->s]t
//
template<typename T>
  inline Expr*
  subst_unary_term(T* t, const Subst& sub) {
    Term* t1 = subst_term(t->t1, sub);
    return new T(t->loc, get_type(t), t1);
  }

// Substitute into a unary term of the form 'op t1 t2'
//
//    [x->s]op t1 t2 = op [x->s]t1 [x->s]t2
//
template<typename T>
  inline Expr*
  subst_binary_term(T* t, const Subst& sub) {
    Term* t1 = subst_term(t->t1, sub);
    Term* t2 = subst_term(t->t2, sub);
    return new T(t->loc, get_type(t), t1, t2);
  }

// Substitute into a ternary term of the form 'op t1 t2 t3'
//
//    [x->s]op t1 t2 t3 = op [x->s]t1 [x->s]t2 [x->s]t3
//
template<typename T>
  inline Expr*
  subst_ternary_term(T* t, const Subst& sub) {
    Term* t1 = subst_term(t->t1, sub);
    Term* t2 = subst_term(t->t2, sub);
    Term* t3 = subst_term(t->t3, sub);
    return new T(t->loc, get_type(t), t1, t2, t3);
  }

// Substitute into the variable declaration of an abstraction.
//
//    [x->s](y : T) -> y : T
//
// No term substitutions are performed on variable declaration
// since that would allow a lambda to capture a substituted
// reference.
//
// Note that System F does require substitutions through the
// variable, but only affecting type variables.
inline Expr*
subst_var(Var* v, const Subst& sub) { return v; }

// Substitute for the reference.
//
//    [x->s]x = s
//    [x->s]y = y for all y != x
//
// A substitution is perofrmed only when the reference refers to
// mapping in the declaration.
inline Expr*
subst_ref(Ref* t, const Subst& sub) {
  if (Expr* s = sub.get(t->decl())) {
    return s;
  }
  else
    return t;
}

inline Expr*
subst_mem(Mem* t, const Subst& sub) {
  Term* t1 = subst_term(t->t1, sub);
  Term* t2 = subst_term(t->t2, sub);
  return new Mem(t->loc, get_unit_type(), t1, t2);
}

} // namespace

Expr*
subst(Expr* e, const Subst& sub) {
  switch (e->kind) {
  case id_expr: return e;
  case unit_term: return e;
  case true_term: return e;
  case false_term: return e;
  case if_term: return subst_ternary_term(as<If>(e), sub);
  case int_term: return e;
  case and_term: return subst_binary_term(as<And>(e), sub);
  case or_term: return subst_binary_term(as<Or>(e), sub);
  case equals_term: return subst_binary_term(as<Equals>(e), sub);
  case less_term: return subst_binary_term(as<Less>(e), sub);
  case not_term: return subst_unary_term(as<Not>(e), sub);
  case succ_term: return subst_unary_term(as<Succ>(e), sub);
  case pred_term: return subst_unary_term(as<Pred>(e), sub);
  case iszero_term: return subst_unary_term(as<Iszero>(e), sub);
  case var_term: return subst_var(as<Var>(e), sub);
  case abs_term: return subst_binary_term(as<Abs>(e), sub);
  case app_term: return subst_binary_term(as<App>(e), sub);
  case ref_term: return subst_ref(as<Ref>(e), sub);
  case mem_term: return subst_mem(as<Mem>(e), sub);
  case kind_type: return e;
  case unit_type: return e;
  case bool_type: return e;
  case nat_type: return e;
  case arrow_type: return e;
  default: break;
  }
  lang_unreachable(format("substitution into unkown term '{}'", node_name(e)));
}

// Return the substituion of sub throught the given term.
inline Term*
subst_term(Term* t, const Subst& sub) {
  return as<Term>(subst(t, sub));
}

// Return the substituion of sub throught the given type.
inline Type*
subst_type(Type* t, const Subst& sub) {
  return as<Type>(subst(t, sub));
}


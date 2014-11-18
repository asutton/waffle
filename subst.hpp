
#ifndef SUBST_HPP
#define SUBST_HPP

#include "ast.hpp"

// -------------------------------------------------------------------------- //
// Substitution

// A substitution maps bindings to the terms that will replace 
// them. For example, in the substitution [x->s]t, we map 
// occurrences of the variable 'x' to the term 's'.
//
// This class allows multiple substitutions to occur at the same 
// time (e.g., [x->s1, y->s2]t).
//
// Note that while the key type of the map is an expr, it refers
// to terms that declare names or values.
struct Subst : std::map<Expr*, Expr*, Expr_less> {
  Subst() = default;
  Subst(Expr*, Expr*);
  
  template<typename T, typename U>
    Subst(Seq<T>*, Seq<U>*);

  Expr* get(Expr*) const;
};

Expr* subst(Expr*, const Subst&);
Type* subst_type(Type*, const Subst&);
Term* subst_term(Term*, const Subst&);

#include "subst.ipp"

#endif

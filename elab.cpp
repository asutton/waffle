
#include "elab.hpp"
#include "token.hpp"
#include "syntax.hpp"
#include "ast.hpp"
#include "scope.hpp"
#include "type.hpp"
#include "language.hpp"

#include "lang/debug.hpp"

#include <iostream>

namespace {

// Declarations
Expr* elab_expr(Tree*);


// -------------------------------------------------------------------------- //
// Diagnostic helpers

struct typed_printer { Expr* e; };

inline typed_printer
typed(Expr* e) { return {e}; }

template<typename C, typename T>
  inline std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, typed_printer p) {
    Expr* e = p.e;
    Type* t = get_type(p.e);
    return os << format("'{}' (of type '{}')", pretty(e), pretty(t));
  }


// -------------------------------------------------------------------------- //
// Elaboration support

// Create and id from an id-tree.
Name*
elab_name(Id_tree* t) {
  const Token* tok = t->value();
  return new Id(t->loc, tok->text);
}

// Create an id from a parse tree representing a name.
Name*
elab_name(Tree* t) {
  if (Id_tree* id = as<Id_tree>(t))
    return elab_name(id);
  lang_unreachable(format("elaborating unknown name '{}'", pretty(t)));
}

// Elaborate a parse tree as a type.
Type*
elab_type(Tree* t) {
  Expr* e = elab_expr(t);
  if (not e)
    return nullptr;

  // If the elaboration of t is reference to a type
  // definition, then unwind that reference now.
  if (Ref* ref = as<Ref>(e))
    if (Def* def = as<Def>(ref->decl()))
      e = def->value();

  if (Type* type = as<Type>(e))
    return type;
  
  error(t->loc) << format("expression '{}' does not name a type", pretty(t));
  return nullptr;
}

// Elaborate a parse tree as a term.
Term*
elab_term(Tree* t) {
  Expr* e = elab_expr(t);
  if (not e)
    return nullptr;
  if (Term* term = as<Term>(e))
    return term;
  else
    error(t->loc) << format("expression '{}' is not a term", pretty(t));
  return nullptr;
}


// -------------------------------------------------------------------------- //
// Elaboration rules

// Elaborate an id by looking it up in the current context.
//
//    n : T in G
//    ---------- T-id
//    G |- n : T
//
// The result of an elaborated id is a reference to its declaring
// expression.
Expr*
elab_id(Id_tree* t) { 
  Name* name = elab_name(t);
  if (Expr* decl = lookup(name))
    return new Ref(t->loc, decl);
  else
    error(t->loc) << format("no matching declaration for '{}'", pretty(name));
  return nullptr; 
}

// Literals are typed by the following axioms.
//
//    G |- unit : Unit
//
//    G |- true : Bool
//
//    G |- false : Bool
//
//    G |- 0 : Nat
//
// Types are kinded by the following axiom. The kind of each built
// in type is "kind" or "*".
//
//    G |- Unit :: *
//
//    G |- Bool :: *
//
//    G |- Nat :: *
Expr*
elab_lit(Lit_tree* t) { 
  const Token* k = t->value();
  switch (k->kind) {
  case unit_tok: 
    return new Unit(t->loc, get_unit_type());
  case true_tok: 
    return new True(t->loc, get_bool_type());
  case false_tok: 
    return new False(t->loc, get_bool_type());
  case decimal_literal_tok: 
    return new Int(t->loc, get_nat_type(), as_integer(*k));
  case string_literal_tok:
    return new Str(t->loc, get_str_type(), as_string(*k));
  case unit_type_tok: 
    return new Unit_type(t->loc, get_kind_type());
  case bool_type_tok: 
    return new Bool_type(t->loc, get_kind_type());
  case nat_type_tok: 
    return new Nat_type(t->loc, get_kind_type());
  default: 
    break;
  }
  lang_unreachable(format("elaborating unknown literal '{}'", pretty(t)));
}

// Elaborate the const definition.
//
//        G |- n : T
//    ------------------- T-def
//    G |- def n = e : T
//
// The definition is declared in the enclosing scope after,
// allowing subsequent declarations to be elaborated.
Expr*
elab_const(Id_tree* t,Tree* e) {
  Name* name = elab_name(t);
  if (not name )
    return nullptr;

  Expr* value = elab_expr(e);
  if (not value)
    return value;

  // The type is deduced from the value.
  Type* type = get_type(value);
  Def* def = new Def(t->loc, type, name, value);
  return declare(def);
}


// Elaborate an anonymous multi-parameter function.
//
//    G, for each f(ti:Ti)->Tk : (Ti)->Tk |- e : Tk
//    ---------------------------------------------------E-func-def
//             G |- def f(t1:T1…. tn:Tn)->Tk = e
//
// Note that this defines a function scope, just like an
// abstraction term.
Expr*
elab_func(Func_tree* t,Tree* e) { 
 
  // Elaborate the name.
  Name* name = elab_name(t->name());
  if (not name)
    return nullptr;

  // Stub out the function
  Term_seq* parms = new Term_seq();
  Func* func=  new Func(t->loc, nullptr, parms, nullptr, nullptr);

  //Enter the function scope	
  Scope_guard scope(func_scope);

  //Elaborate function parameters. Note that
  // each paramteer is declared as it is elaborated.
  for (Tree *t0 : *t->parms()) {
    Term* t1 = elab_term(t0);
    if (not t1)
      return nullptr;
    if (not is<Var>(t1)) {
      error(t1->loc) << format("ill-formed parameter '{}'", pretty(t1));
      return nullptr;
    }
    parms->push_back(t1);
  }

  //Elaborate the result type.
  Type* result =elab_type(t->type());
  if (not result)
    return nullptr;
  func->t2=result;

  // Compute the type of function and update.
  Type* kind = get_kind_type();
  Type_seq* t0 = get_type(parms);
  Type* type = new Fn_type(no_location, kind, t0, result);
  func->tr=type;

  //Declare def
  Def* def=new Def(t->loc,type,name,func);
  if (not declare_outside(def))
    return nullptr;

  //Elaborate the function body
  Term* value = elab_term(e);
  if (not value)
    return nullptr;
  func->t3=value;
 
  return def;
}

//Elaboration of definitions
//A definition is either function or constant.
//The typing is handled by elab_const or elab_func 
Expr*
elab_def(Def_tree* t){
  if (Id_tree* v = as<Id_tree>(t->name()))
    return elab_const(v, t->value());
  if (Func_tree* f = as<Func_tree>(t->name()))
    return elab_func(f, t->value());
  lang_unreachable(format("{}: elaboration failure", t->loc));
}

// Elaborate an initializer.
//
//     G |- t : T
//    ------------ T-init
//    G |- n=t : T
Expr*
elab_init(Init_tree* t) {
  Name* name = elab_name(t->name());
  if (not name)
    return nullptr;
  Term* term = elab_term(t->term());
  if (not name)
    return nullptr;
  Type* type = get_type(term);
  return new Init(t->loc, type, name, term);
}

// Elaborate a variable declared as part of a lambda expression.
//
//    G |- T :: K
//    ----------- T-var-decl
//    G |- n : T
//
// Note that the variable is declared here rather than in the
// elaboration of lambda abstractions.
Expr*
elab_var(Var_tree* t) { 
  Name* name = elab_name(t->t1);
  if (not name)
    return nullptr;
  Type* type = elab_type(t->t2);
  if (not type)
    return nullptr;

  // Create and declare the parameter.
  Var* var = new Var(t->loc, name, type);
  return declare(var);
}

// Elaborate a lambda abstraction.
//
//      G, x:T |- t : U
//    -------------------- T-abs
//    G |- \x:T.t : T -> U
Expr*
elab_abs(Abs_tree* t) { 
  Scope_guard scope(lambda_scope);

  // Elaborate the subterms.
  Term* var = elab_term(t->var());
  if (not var)
    return nullptr;
  Term* term = elab_term(t->term());
  if (not term)
    return nullptr;

  // Create the result type.
  Type* kind = get_kind_type();
  Type* t0 = get_type(var);
  Type* u0 = get_type(term);
  Type* type = new Arrow_type(no_location, kind, t0, u0);

  // Create the abstraction.
  return new Abs(t->loc, type, var, term);
}

// Elaborate an anonymous multi-parameter function.
//
//      G, for each xi:Ti |- t : U
//    ------------------------------------ T-abs
//    G |- \(x1:T1, ..., xn:Tn).t : T -> U
//
// Note that this defines a lambda scope, just like an
// abstraction term.
Expr*
elab_fn(Fn_tree* t) { 
  Scope_guard scope(lambda_scope);

  // Elaborate the parameters and the abstracted term. Note that
  // each paramteer is declared as it is elaborated.
  Term_seq* parms = new Term_seq();
  for (Tree *t0 : *t->parms()) {
    Term* t1 = elab_term(t0);
    if (not t1)
      return nullptr;
    if (not is<Var>(t1)) {
      error(t1->loc) << format("ill-formed parameter '{}'", pretty(t1));
      return nullptr;
    }
    parms->push_back(t1);
  }
  Term* term = elab_term(t->term());
  if (not term)
    return nullptr;

  // Create the result type.
  Type* kind = get_kind_type();
  Type_seq* t0 = get_type(parms);
  Type* u0 = get_type(term);
  Type* type = new Fn_type(no_location, kind, t0, u0);

  // Create the abstraction.
  return new Fn(t->loc, type, parms, term);
}


// Elaborate an application of an abstraction to a term.
//
//    G |- t1 : T -> U   G |- t2 : T
//    ------------------------------ T-app
//           G |- t1 t2 : U
Expr*
elab_app(App_tree* t, Term* fn, Term* arg, Arrow_type* fn_type) {
  Type* parm_type = fn_type->parm();

  // Check that t2 has the type of T in the type type T -> U.
  Type* arg_type = get_type(arg);
  if (not is_same(arg_type, parm_type)) {
    error(arg->loc) << 
      format("argument '{}' (of type '{}') does not have type '{}'",
             pretty(arg),
             pretty(arg_type),
             pretty(parm_type));
    return nullptr;
  }

  // Build the application.
  Type* result_type = fn_type->result();
  return new App(t->loc, result_type, fn, arg);
}

// Create a list of arguments from a list of elaborated expresions.
Term_seq*
make_args(Expr_seq* es) {
  Term_seq* ts = new Term_seq();
  for (Expr* e : *es) {
    if (Term* t = as<Term>(e)) {
      ts->push_back(t);
    } else {
      error(e->loc) << format("invalid function argument '{}'", pretty(e));
      return nullptr;
    }
  }
  return ts;
}

// Create a singleton list of arguments.
Term_seq*
make_args(Term* arg) {
  return new Term_seq {arg};
}

// Elaborate a funtion call.
//
// TODO: Overload resolution happens HERE.
Expr*
elab_call(App_tree* t, Term* fn, Term* arg, Fn_type* fn_type) {
  // Build a sequence of arguments based on kind of expression
  // given at the call site.
  Term_seq* args;
  if (Comma* c = as<Comma>(arg))
    args = make_args(c->elems());
  else
    args = make_args(arg);

  // Check that the type of each argument matches those of the
  // the parameters.
  Type_seq* parm_types = fn_type->parms();
  
  // Make sure we have enough arguments.
  if (args->size() != parm_types->size()) {
    error(t->loc) << format("'{}' requires {} arguments, {} given",
                            pretty(fn),
                            parm_types->size(),
                            args->size());
    return nullptr;
  }

  // Check that each argument has the appropriate type.
  auto arg_iter = args->begin();
  auto arg_end = args->end();
  auto parm_iter = parm_types->begin();
  bool ok = true;
  while (arg_iter != arg_end) {
    Term* arg = *arg_iter;
    Type* parm = *parm_iter;
    if (not is_same(get_type(arg), parm)) {
      error(arg->loc) << format("argument {} does not have type '{}'",
                                typed(arg),
                                pretty(parm));
      ok = false;
    }
    ++arg_iter;
    ++parm_iter;
  }
  if (not ok)
    return nullptr;

  // Build the function call.
  Type* result_type = fn_type->result();
  return new Call(t->loc, result_type, fn, args);
}

// Elaborate an application or function call.
Expr*
elab_app(App_tree* t) { 
  Term* t1 = elab_term(t->fn());
  if (not t1)
    return nullptr;
  Term* t2 = elab_term(t->arg());
  if (not t2)
    return nullptr;

  // Determine whether the application is a lambda application
  // or a function call.
  Type* type = get_type(t1);
  if (Arrow_type* atype = as<Arrow_type>(type))
    return elab_app(t, t1, t2, atype);
  if (Fn_type* ftype = as<Fn_type>(type))
    return elab_call(t, t1, t2, ftype);

  error(t1->loc) << format("term '{}' (of type '{}') is not a function type",
                           pretty(t1), 
                           pretty(type));
  return nullptr;
}

// Elaborate an if term.
//
//    G | t1 : Bool   G |- t2 : T   G |- t3 : T
//    ----------------------------------------- T-if
//         G |- if t1 then t2 else t3 : T
Expr*
elab_if(If_tree* t) { 
  Term* t1 = elab_term(t->cond());
  if (not t1)
    return nullptr;
  Term* t2 = elab_term(t->if_true());
  if (not t2)
    return nullptr;
  Term* t3 = elab_term(t->if_false());
  if (not t3)
    return nullptr;

  // Check that t1 has type Bool.
  Type* bool_type = get_bool_type();
  Type* type1 = get_type(t1);
  if (not is_same(type1, get_bool_type())) {
    error(t1->loc) << 
      format("term {} does not have type '{}'", typed(t1), pretty(bool_type));
    return nullptr;
  }

  // Check that t2 and t3 have the same type.
  Type* type2 = get_type(t2);
  Type* type3 = get_type(t3);
  if (not is_same(type2, type3)) {
    error(t3->loc) << 
      format("term {} does not have type '{}'", typed(t3), pretty(type2));
    return nullptr;
  }

  return new If(t->loc, type2, t1, t2, t3);
}

// TODO: The succ, pred, and iszero elaborators have a lot in
// common. They could be simplified quite a bit.

// Elaborate a successor term.
//
//      G |- t : Nat
//    ----------------- T-succ
//    G |- succ t : Nat
Expr*
elab_succ(Succ_tree* t) {
  Term* t1 = elab_term(t->arg());
  if (not t1)
    return nullptr;

  // Check that t1 has type nat
  Type* nat_type = get_nat_type();
  Type* type1 = get_type(t1);
  if (not is_same(type1, nat_type)) {
    error(t1->loc) << 
      format("term {} does not have type '{}'", typed(t1), pretty(nat_type));
      return nullptr;
  }

  return new Succ(t->loc, nat_type, t1); 
}

// Elaborate a predecessor term.
//
//      G |- t : Nat
//    ----------------- T-pred
//    G |- pred t : Nat
Expr*
elab_pred(Pred_tree* t) { 
  Term* t1 = elab_term(t->arg());
  if (not t1)
    return nullptr;

  // Check that t1 has type nat
  Type* nat_type = get_nat_type();
  Type* type1 = get_type(t1);
  if (not is_same(type1, nat_type)) {
    error(t1->loc) << 
      format("term {} does not have type '{}'", typed(t1), pretty(nat_type));
      return nullptr;
  }

  return new Pred(t->loc, nat_type, t1);
}

// Elaborate a iszero term.
//
//      G |- t : Nat
//    ------------------ T-iszero
//    G |- iszer t : Bool
Expr*
elab_iszero(Iszero_tree* t) { 
  Term* t1 = elab_term(t->arg());
  if (not t1)
    return nullptr;

  // Check that t1 has type nat
  Type* nat_type = get_nat_type();
  Type* type1 = get_type(t1);
  if (not is_same(type1, nat_type)) {
    error(t1->loc) << 
      format("term {} does not have type '{}'", typed(t1), pretty(nat_type));
      return nullptr;
  }
  Type* bool_type = get_bool_type();

  return new Iszero(t->loc, bool_type, t1); 
}

// Elaborate an arrow expression. When both sub-expressions
// are types, the resulting epression is an arrow type.
//
//    G |- t1 :: *   G |- t2 :: *
//    --------------------------- K-arrow
//        G |- t1 -> t2 :: *
Expr*
elab_arrow(Arrow_tree* t) {
  Expr* t1 = elab_expr(t->left());
  if (not t1)
    return nullptr;
  Expr* t2 = elab_expr(t->right());
  if (not t2)
    return nullptr;

  // Check that both t1 and t2 are types
  if (not is_type(t1)) {
    error(t1->loc) << format("'{}' does not name a type", pretty(t1));
    return nullptr;
  }
  if (not is_type(t2)) {
    error(t2->loc) << format("'{}' does not name a type", pretty(t2));
    return nullptr;
  }
  Type* kind_type = get_kind_type();
  Type* type1 = static_cast<Type*>(t1);
  Type* type2 = static_cast<Type*>(t2);

  return new Arrow_type(t->loc, kind_type, type1, type2);
}

// Elaborate a tuple.
//
//         for each i G |- ti : Ti
//    ---------------------------------- T-tuple
//    G |- {t1, ..., tn} : {T1, ..., Tn}
Expr*
elab_tuple(Tuple_tree* t, Term* t0) {
  Term_seq* terms = new Term_seq();
  Type_seq* types = new Type_seq();

  // Add the previously elaborated term to the tuple.
  terms->push_back(t0);
  types->push_back(get_type(t0));

  // Elaborate each of the remaining terms in the sequence.
  auto iter = std::next(t->elems()->begin());
  auto end = t->elems()->end();
  while (iter != end) {
    Expr* ei = elab_expr(*iter);
    if (Term* ti = as<Term>(ei)) {
      terms->push_back(ti);
      types->push_back(get_type(ti));
    } else if(ei) {
      error(ei->loc) << format("'{}' cannot appear in a tuple", pretty(ei));
      return nullptr;
    } else {
      return nullptr;
    }
    ++iter;
  }

  Type* type = new Tuple_type(get_kind_type(), types);
  return new Tuple(t->loc, type, terms);
}

// Elaborate a tuple type.
//
//    for each i G |- Ti :: *
//    ----------------------- K-tuple
//    G |- {T1, ..., Tn} :: *
Expr*
elab_tuple_type(Tuple_tree* t, Type* t0) {
  Type_seq* types = new Type_seq();

  // Add the previously elaborated term to the tuple.
  types->push_back(t0);

  // Elaborate each of the remaining types in the sequence.
  auto iter = std::next(t->elems()->begin());
  auto end = t->elems()->end();
  while (iter != end) {
    Expr* ei = elab_expr(*iter);
    if (Type* ti = as<Type>(ei)) {
      types->push_back(ti);
    } else if(ei) {
      error(ei->loc) << format("'{}' cannot appear in a tuple type", pretty(ei));
      return nullptr;
    } else {
      return nullptr;
    }
    ++iter;
  }

  return new Tuple_type(t->loc, get_kind_type(), types);
}


// Return the variable describing an initializer.
Term*
get_var(Init* t) {
  return new Var(t->name(), get_type(t->value()));
}

// Elaborate a record.
//
//               for each i G |- ti : Ti
//    ---------------------------------------------- T-record
//    G |- {n1=t1, ..., nn=tn} : {n1:T1, ..., n2:Tn}
//
// TODO: There is a lot of duplication with this and the
// tuple elaborator. We could probably unify parts of these
// functions.
Expr*
elab_record(Tuple_tree* t, Init* t0) {
  Term_seq* inits = new Term_seq();
  Term_seq* vars = new Term_seq();

  // Add the previously elaborated term to the tuple.
  inits->push_back(t0);
  vars->push_back(get_var(t0));

  // Elaborate each of the remaining initialiers in the sequence.
  auto iter = std::next(t->elems()->begin());
  auto end = t->elems()->end();
  while (iter != end) {
    Expr* ei = elab_expr(*iter);
    if (Init* ti = as<Init>(ei)) {
      inits->push_back(ti);
      vars->push_back(get_var(ti));
    } else if(ei) {
      error(ei->loc) << format("'{}' cannot appear in a record", pretty(ei));
      return nullptr;
    } else {
      return nullptr;
    }
    ++iter;
  }

  Type* type = new Record_type(get_kind_type(), vars);
  return new Record(t->loc, type, inits);
}

// Elaborate a record type.
//
//      for each i G |- Ti :: *
//    ---------------------------- K-record
//    G |- {n1:T1, ..., nn:Tn} : *
Expr*
elab_record_type(Tuple_tree* t, Var* t0) {
  Term_seq* vars = new Term_seq();

  // Add the previously elaborated term to the tuple.
  vars->push_back(t0);

  // Elaborate each of the remaining initialiers in the sequence.
  auto iter = std::next(t->elems()->begin());
  auto end = t->elems()->end();
  while (iter != end) {
    Expr* ei = elab_expr(*iter);
    if (Var* ti = as<Var>(ei)) {
      vars->push_back(ti);
    } else if(ei) {
      error(ei->loc) << format("'{}' cannot appear in a record type", pretty(ei));
      return nullptr;
    } else {
      return nullptr;
    }
    ++iter;
  }

  return new Record_type(get_kind_type(), vars);
}

// Elaborate a tuple expression. Note that there are many
// forms that this tuple can represent. These are:
//
//    - {} is the empty tuple. Note that this is always a term.
//
// FIXME: How do we parse the type of the empty tuple.
Expr*
elab_tuple(Tuple_tree* t) {
  if (t->elems()->empty()) {
    Tuple_type* type = new Tuple_type(get_kind_type(), new Type_seq());
    return new Tuple(t->loc, type, new Term_seq());
  }

  // Elaborate the first element of the tuple. It determines
  // the elaboration of the remaining elements.
  Expr* expr = elab_expr(t->elems()->front());
  if (Init* init = as<Init>(expr))
    return elab_record(t, init);
  if (Var* var = as<Var>(expr))
    return elab_record_type(t, var);
  if (Term* term = as<Term>(expr))
    return elab_tuple(t, term);
  if (Type* type = as<Type>(expr))
    return elab_tuple_type(t, type);

  error(t->loc) << format("ill-formed expression '{}'", pretty(t));
  return nullptr;
}

// Elaboreate a list type.
//
Expr*
elab_list_type(List_tree* t, Type* t0) {
  if (t->elems()->size() > 1) {
    error(t->loc) << format("ill-formed list type '{}'", pretty(t));
    return nullptr;
  }
  return new List_type(get_kind_type(), t0);
}

// Elaborate a list of terms.
Expr*
elab_list(List_tree* t, Term* t0) {
  Term_seq* terms = new Term_seq {t0};
  Type* value_type = get_type(t0);

  auto iter = std::next(t->elems()->begin());
  auto end = t->elems()->end();
  while (iter != end) {
    Expr* ei = elab_expr(*iter);
    if (Term* ti = as<Term>(ei)) {
      if (!is_same (get_type(ti), value_type)) {
        error(ti->loc) << format("list element {} does not have type '{}'",
                                 typed(ti), 
                                 pretty(value_type));
        return nullptr;
      }
      terms->push_back(ti);
    } else if(ei) {
      error(ei->loc) << format("'{}' cannot appear in a list", pretty(ei));
      return nullptr;
    } else {
      return nullptr;
    }
    ++iter;
  }

  Type* type = new List_type(get_kind_type(), value_type);
  return new List(t->loc, type, terms);
}

// Elaborate a list of expressions. The elaboration depends on the
// form of the list. When the list has the form
//
//  - [], this is the empty list with type [*x]
//  - [T] where T is a type, this is the list type [T].
//  - [t1, ..., tn] where each ti is a term whose type is T, then
//    this is the a list whose type is [T].
//
// The prorgram is ill-formed if the list has any other form.
//
// FIXME: When the list expresison has the form [], create an empty
// list whose type is unspecified, and then require ascription for
// a well-typed expression? That's a bit flimsy.
Expr*
elab_list(List_tree *t) {
  if (t->elems()->empty()) {
    Name* n = fresh_name();
    Type* wild = new Wild_type(get_kind_type(), n, get_kind_type());
    Type* type = new List_type(get_kind_type(), wild);
    Term* list = new List(type, new Term_seq());
    return list;
  }

  Expr* expr = elab_expr(t->elems()->front());
  if (Type* type = as<Type>(expr))
    return elab_list_type(t, type);
  if (Term* term = as<Term>(expr))
    return elab_list(t, term);

  error(t->loc) << format("ill-formed list expression '{}'", pretty(t));
}


// Return a variable describing an initializer.
Expr*
elab_variant(Variant_tree* t) {
  return new Unit(t->loc, get_unit_type());
}

// Elaborate a print expression.
//
//        G |- t : T
//    ------------------- T-print
//    G |- print t : Unit
Expr*
elab_print(Print_tree* t) {
  Expr* t1 = elab_expr(t->expr());
  if (not t1)
    return nullptr;
  return new Print(t->loc, get_unit_type(), t1);
}

// A typeof expression is an alias for the type of the 
// given term. It is not a term in the abstract syntax.
//
// Note that if the language evolves features where the type
// of an expression cannot be determined outright (i.e., it
// may be one of several unspecified types), then we would
// need to preserve this term in the abstract language. This
// would, in fact, be analogous to C++'s decltype feature
// when used inside a template.
Expr*
elab_typeof(Typeof_tree* t) {
  Expr* t1 = elab_expr(t->expr());
  if (not t1)
    return nullptr;
  return get_type(t1);
}

// Elaborate a comma expression. The type of the expression
// is the unit type.
//
//     for each i G |- ti : Ti
//    ------------------------- T-comma
//    G |- (t1, ..., tn) : Unit
Expr*
elab_comma(Comma_tree* t) {
  Expr_seq* exprs = new Expr_seq();
  for (Tree* t0 : *t->elems()) {
    if (Expr* e = elab_expr(t0))
      exprs->push_back(e);
    else
      return nullptr;
  }
  return new Comma(t->loc, get_unit_type(), exprs);
}

// FIXME: Implement me.
Expr*
elab_proj(Dot_tree* t, Term* t1, Term* t2, Tuple_type* tup_type) {
  return new Unit(t->loc, get_unit_type());
}

// FIXME: Implement me too.
Expr*
elab_mem(Dot_tree* t, Term* t1, Term* t2, Record_type* rec_type) {
  return new Unit(t->loc, get_unit_type());
}

// Elaborate a dotted access expression. Note that the elaboration
// depends on the type of the object.
//
//    G |- t : {T1, ..., Tn}   0 < i < n
//    ---------------------------------- T-proj
//             G |- t.i : Ti
//
//    G |- t : {l1:T1, ..., ln:Tn}
//    ---------------------------- T-proj
//           G |- t.li : Ti
Expr*
elab_dot(Dot_tree* t) {
  Term* t1 = elab_term(t->object());
  Term* t2 = elab_term(t->elem());

  if (not t1 or not t2)
    return nullptr;

  Type* type = get_type(t1);
  if (Tuple_type* tup = as<Tuple_type>(type))
    return elab_proj(t, t1, t2, tup);
  if (Record_type* rec = as<Record_type>(type))
    return elab_mem(t, t1, t2, rec);

  error(t1->loc) << format("'{}' is not a tuple or record", pretty(t1));
  return nullptr;
}


// Elaborate a program. The result type of the entire program
// is that of the last statement.
//
//    for each i  G, ei-1 : Ti-1 |- ei : Ti
//    ------------------------------------- T-prog
//           G |- e1; ...; en; : Tn
//
// Note that we push the global scope before elaborating each
// statement in the program.
Expr*
elab_prog(Prog_tree* t) {
  Scope_guard scope(global_scope);

  // Elaborate each statement in turn.
  Term_seq* stmts = new Term_seq();
  for (Tree* s : *t->stmts()) {
    if (Term* term = elab_term(s)) {
      stmts->push_back(term);
    }
    else
      return nullptr;
  }

  // The type is that of the last statement.
  Type* type = get_type(stmts->back());
  return new Prog(type, stmts);
}

Expr* 
elab_expr(Tree* t) {
  if (not t)
    return nullptr;

  switch (t->kind) {
  case id_tree: return elab_id(as<Id_tree>(t));
  case lit_tree: return elab_lit(as<Lit_tree>(t));
  case def_tree: return elab_def(as<Def_tree>(t));
  case init_tree: return elab_init(as<Init_tree>(t));
  case var_tree: return elab_var(as<Var_tree>(t));
  case abs_tree: return elab_abs(as<Abs_tree>(t));
  case fn_tree: return elab_fn(as<Fn_tree>(t));
  case app_tree: return elab_app(as<App_tree>(t));
  case if_tree: return elab_if(as<If_tree>(t));
  case succ_tree: return elab_succ(as<Succ_tree>(t));
  case pred_tree: return elab_pred(as<Pred_tree>(t));
  case iszero_tree: return elab_iszero(as<Iszero_tree>(t));
  case arrow_tree: return elab_arrow(as<Arrow_tree>(t));
  case tuple_tree: return elab_tuple(as<Tuple_tree>(t));
  case list_tree: return elab_list(as<List_tree>(t));
  case variant_tree: return elab_variant(as<Variant_tree>(t));
  case print_tree: return elab_print(as<Print_tree>(t));
  case typeof_tree: return elab_typeof(as<Typeof_tree>(t));
  case comma_tree: return elab_comma(as<Comma_tree>(t));
  case dot_tree: return elab_dot(as<Dot_tree>(t));
  case prog_tree: return elab_prog(as<Prog_tree>(t));
  default: break;
  }
  lang_unreachable(format("elaborating unknown node '{}'", node_name(t)));
}

} // namespace


Expr*
Elaborator::operator()(Tree* t) {
  use_diagnostics(diags);
  return elab_expr(t);
}

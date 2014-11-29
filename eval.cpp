
#include "eval.hpp"
#include "ast.hpp"
#include "scope.hpp"
#include "type.hpp"
#include "value.hpp"
#include "subst.hpp"

#include "lang/debug.hpp"

#include <iostream>
#include <set>

// -------------------------------------------------------------------------- //
// Evaluator class

Term*
Evaluator::operator()(Term* t) {
  return eval(t);
}


// -------------------------------------------------------------------------- //
// Multi-step evaluation
//
// The following function computes the multi-step evaluation (or
// simply evaluation) of a term t. Note that the evaluation is reflexive, 
// meaning that the evaluation of a value (or normal form) is simply
// an identity operation.

Term* eval(Term*);

namespace {

// Compute the multistep evaluation of an if term
//
//             t1 ->* true
//    ---------------------------- E-if-true
//    if t1 then t2 else t3 ->* t2
//
//             t1 ->* true
//    ---------------------------- E-if-false
//    if t1 then t2 else t3 ->* t2
Term*
eval_if(If* t) {
  Term* bv = eval(t->cond());
  if (is_true(bv))
    return eval(t->if_true());
  if (is_false(bv))
    return eval(t->if_false());
  lang_unreachable(format("'{}' is not a boolean value", pretty(bv)));
}

// Compute the multi-step evaluation of a successor term.
//
//         t ->* n
//    ---------------- E-succ
//    succ t ->* n + 1
//
// Here, 'n' is an integer value.
Term*
eval_succ(Succ* t) {
  Term* t1 = eval(t->arg());
  if (Int* n = as<Int>(t1)) {
    const Integer& z = n->value();
    return new Int(t->loc, get_type(t), z + 1);
  }
  lang_unreachable(format("'{}' is not a numeric value", pretty(t1)));
}

// Evalutae a predecessor term.
//
//      t ->* 0
//    ------------ E-pred-0
//    pred t ->* 0
//
//         t ->* n
//    ---------------- E-pred-succ
//    pred t ->* n - 1
//
// Here, 'n' is an integer value.
Term*
eval_pred(Pred* t) {
  Term* t1 = eval(t->arg());
  if (Int* n = as<Int>(t1)) {
    const Integer& z = n->value();
    if (z == 0)
      return n;
    else
      return new Int(t->loc, get_type(t), z - 1);
  }
  lang_unreachable(format("'{}' is not a numeric value", pretty(t1)));
}

// Evaluate an iszero term.
//
//         t ->* 0
//    ----------------- E-iszero-0
//    iszero t ->* true
//
//         t ->* n
//    ------------------ E-iszero-succ
//    iszero t ->* false
Term*
eval_iszero(Iszero* t) {
  Term* t1 = eval(t->arg());
  if (Int* n = as<Int>(t1)) {
    const Integer& z = n->value();
    if (z == 0)
      return new True(t->loc, get_bool_type());
    else
      return new False(t->loc, get_bool_type());
  }
  lang_unreachable(format("'{}' is not a numeric value", pretty(t1)));
}

// Evaluate an application.
//
//        t1 ->* \x:T.t
//    --------------------- E-app-1
//    t1 t2 ->* (\x:T.t) t2
//
//          t2 ->* v
//    --------------------- E-app-2
//    \x:T.t t2 ->* [x->v]t
Term*
eval_app(App* t) {
  Abs* fn = as<Abs>(eval(t->abs())); // E-app-1
  lang_assert(fn, format("ill-formed application target '{}'", pretty(t->abs())));

  Term* arg = eval(t->arg()); // E-app-2
    
  // Perform a beta reduction and evaluate the result.
  Subst sub {fn->var(), arg};
  Term* res = subst_term(fn->term(), sub);
  return eval(res);
}

// Evaluate a function call. This is virtually identical to
// application except that all arguments are evaluated in turn.
//
// TODO: Document the semantics of these operations.
Term*
eval_call(Call* t) {
  // Evaluate the function.
  Fn* fn = as<Fn>(eval(t->fn()));
  lang_assert(fn, format("ill-formed call target '{}'", pretty(t->fn())));

  // Evaluate arguments in place. That is, we're not creating
  // a new sequence of arguments, just replacing the entries
  // in the existing sequence.
  Term_seq* args = t->args();
  for (Term*& a : *args)
    a = eval(a);

  // Beta reduce and evaluate.
  Subst sub {fn->parms(), args};
  Term* result = subst_term(fn->term(), sub);
  return eval(result);
}

// Elaborate a declaration reference. When the reference
// is to a definition, replace it with the definition's value.
// Otherwise, preserve the reference.
//
// If the reference is to a type, then we can't evaluate this.
// Just return nullptr and hope that the caller knows how to
// handle the results.
Term*
eval_ref(Ref* t) {
  if (Def* def = as<Def>(t->decl())) {
    if (Term* replace = as<Term>(def->value()))
      return replace;
    else
      return nullptr;
  } else {
    return t;
  }
}

// Evaluate the definition by evaluating the defined term. When the
// definition's value is not a term, then there isn't anything
// interesting that we can do. Just return the value.
Term*
eval_def(Def* t) {
  if (Term* t0 = as<Term>(t->value())) {
    // This is a little weird. We're actually going to update
    // the defined term with its evaluated initializer. We do this
    // because other expressions may already refer to t and we don't
    // really want to re-resolve all of those things.
    //
    // Note that we could choose to do this during elaboration
    // in order to avoid the weirdness.
    t->t2 = eval(t0);
  }
  return t;
}

// Elaborate a print statement.
//
//          t ->* v
//    ------------------- E-print-term
//    print t ->* print v
//
//    --------------- E-print-value
//    print v -> unit
//
//    --------------- E-print-type
//    print T -> unit
//
Term*
eval_print(Print* t) {
  // Try to evaluate the expression.
  Term* val = nullptr;
  if (Term* term = as<Term>(t->expr()))
    val = eval(term);

  // Print the result, or if the expression is not
  // evaluable, just print the expression.
  if (val)
    std::cout << pretty(val) << '\n';
  else
    std::cout << pretty(t->expr()) << '\n';

  return new Unit(t->loc, get_unit_type());
}

// FIXME: Actually evaluate each expression in turn.
Term*
eval_comma(Comma* t) {
  return get_unit();
}

// Evaluate each statement in turn; the result of the program is
// the result of the last statemnt. 
//
//    for each i ei ->* vi
//    -------------------- E-prog
//     e1; ...; en ->* vn
Term*
eval_prog(Prog* t) {
  Term* tn;
  for (Term* ti : *t->stmts())
    tn = eval(ti);
  return tn;
}

// Evaluation for 't1 and t2'
//
// t1 ->* true   t2 -> true
// ------------------------
// t1 and t2 ->* true
//
// t1 ->* false   t2 -> true
// ------------------------
// t1 and t2 ->* false
//
// t1 ->* true   t2 -> false
// ------------------------
// t1 and t2 ->* false
//
// t1 ->* false  t2 -> false
// ------------------------
// t1 and t2 ->* false
Term*
eval_and(And* t) {
  Term* t1 = eval(t->t1);
  Term* t2 = eval(t->t2);

  if(is_true(t1) && is_true(t2))
    return get_true();
  else 
    return get_false();
}

// Evaluation for 't1 or t2'
//
// t1 ->* true   t2 -> true
// ------------------------
// t1 or t2 ->* true
//
// t1 ->* false   t2 -> true
// ------------------------
// t1 or t2 ->* true
//
// t1 ->* true   t2 -> false
// ------------------------
// t1 or t2 ->* true
//
// t1 ->* false  t2 -> false
// ------------------------
// t1 or t2 ->* false
//
Term*
eval_or(Or* t) {
  Term* t1 = eval(t->t1);
  Term* t2 = eval(t->t2);

  if(is_false(t1) && is_false(t2))
    return get_false();
  else 
    return get_true();
}

// Evaluation for 'not t1'
// 
// t1 ->* false
// --------------
// not t1 ->* true
//
// t1 ->* true
// --------------
// not t1 ->* false
//
Term*
eval_not(Not* t) {
  Term* t1 = eval(t->t1);

  if(is_true(t1))
    return get_false();
  if(is_false(t1))
    return get_true();
}

// Evaluation for t1 == t2
// 
// Online works for types defined by is_equals
// Does not actually require the same type on both
// operands since different typed terms fail the first cond anyway
Term*
eval_equals(Equals* t) {
  Term* t1 = eval(t->t1);
  Term* t2 = eval(t->t2);

  if(is_same(t1, t2))
    return get_true();
  else
    return get_false();
}

// Evaluation for the term 't1 < t2'
// Only works on types defined by is_less
//
Term*
eval_less(Less* t) {
  Term* t1 = eval(t->t1);
  Term* t2 = eval(t->t2);

  if(is_less(t1, t2))
    return get_true();
  else
    return get_false();
}

///////////////////////////////////
//
// Evaluation for Relational Algebra
//
///////////////////////////////////

Term*
eval_proj(Proj* t) {
  return nullptr;
}

// Returns a column projection for tables
Term*
eval_col(Mem* t) {
  List* table = as<List>(eval(t->t1));
  Ref* member = as<Ref>(t->member());
  Var* v = as<Var>(member->decl());
  Name* n = v->name();

  Term_seq* records = table->elems();
  Term_seq* vars = new Term_seq();
  vars->push_back(v);
  Type* rec_type = new Record_type(get_kind_type(), vars);

  // resulting column
  Term_seq* col = new Term_seq();
  for (auto r : *records) {
    for (auto i : *as<Record>(r)->members()) {
      if (is_same(n, as<Init>(i)->name())) {
        Term_seq* e = new Term_seq();
        e->push_back(i);
        col->push_back(new Record(rec_type, e));
      }
    }
  }
  Type* type = new List_type(get_kind_type(), rec_type);
  return new List(type, col);
}

// Returns a term from the record such that the label in the record matches l
// Returns nullptr if the label l does not match anything in record r
Term*
eval_mem(Mem* t) {
  Term* t1 = eval(t->t1);

  // If its a record type get the term with the corresponding label
  if (Record_type* r_type = as<Record_type>(get_type(t1))) {
    Term_seq* r = as<Record>(t1)->members();
    Ref* ref = as<Ref>(t->member());
    Name* n = as<Var>(ref->decl())->name();

    for (auto i : *r) {
      if (is_same(n, as<Init>(i)->name())) {
        return as<Term>(as<Init>(i)->value());
      }
    }
  }

  // Else return the column
  if (List_type* l_type = as<List_type>(get_type(t1))) {
    return eval_col(t);
  }

  return nullptr;
}

// Merges records a and b
Record*
merge_records(Record* a, Record* b) {
  Term_seq* a_mem = a->members();
  Term_seq* b_mem = b->members();
  Term_seq* elems = new Term_seq();

  elems->insert(elems->end(), a_mem->begin(), a_mem->end());
  elems->insert(elems->end(), b_mem->begin(), b_mem->end());
  
  Record_type* ar_type = as<Record_type>(get_type(a));
  Record_type* br_type = as<Record_type>(get_type(b));

  Term_seq* vars = new Term_seq();
  vars->insert(vars->end(), ar_type->members()->begin(), ar_type->members()->end());
  vars->insert(vars->end(), br_type->members()->begin(), br_type->members()->end());

  Record_type* type = new Record_type(get_kind_type(), vars);
  return new Record(type, elems);
}

// Appends table b to the rhs of table a
// a and b must be a list of Records to work
// else bad things happen
List*
merge_tables(List* a, List* b) {
  // assume the record type because these are supposed to be tables
  List_type* al_type = as<List_type>(get_type(a));
  List_type* bl_type = as<List_type>(get_type(b));
  Record_type* ar_type = as<Record_type>(al_type->type());
  Record_type* br_type = as<Record_type>(bl_type->type());

  Term_seq* vars = new Term_seq();
  vars->insert(vars->end(), ar_type->members()->begin(), ar_type->members()->end());
  vars->insert(vars->end(), br_type->members()->begin(), br_type->members()->end());
  Record_type* nr_type = new Record_type(get_kind_type(), vars);

  //merge the individual records in the table
  Term_seq* rec = new Term_seq();
  auto it_a = a->elems()->begin();
  auto it_b = b->elems()->begin();
  while (it_a != a->elems()->end()) {
    rec->push_back(merge_records(as<Record>(*it_a), as<Record>(*it_b)));
    ++it_a;
    ++it_b;
  }

  List_type* l_type = new List_type(get_kind_type(), nr_type);
  List* res = new List(l_type, rec);
  return res;
}

//evaluation for select t1 from t2 where t3
Term*
eval_select_from_where(Select_from_where* t) {
  //evaluate the list first
  List* t2 = as<List>(eval(t->t2));

  //new term seq to hold var list for record type
  Term_seq* cols = new Term_seq();
  //if its more than one projection operator
  if (Comma* c = as<Comma>(t->t1)) {
    for (auto p : *c->elems()) {
      Mem* m = as<Mem>(p);
      List* col = as<List>(eval(m));
      cols->push_back(col);
    }
  }

  //in case its just one projection
  if (Mem* m = as<Mem>(t->t1)) {
    cols->push_back(eval(m));
  }

  //construct the new table after projection
  List* n_table = as<List>(*cols->begin());
  if(cols->size() > 1) {
    auto it1 = cols->begin() + 1;
    while (it1 != cols->end()) {
      n_table = merge_tables(n_table, as<List>(*it1));
      ++it1;
    }
  }

  // t2 should be a Def or a Ref
  // we cannot have a table with no name here
  Term* subst;
  if (Ref* ref = as<Ref>(t->t2))
    subst = eval(as<Def>(ref->decl()));
  if (Def* def = as<Def>(t->t2))
    subst = eval(def);

  // first we need to produce a set of conditions
  // t3 is not just 1 condition, it is a condition for every record in the list
  // for each record in t2, we need to substitute the ref t2 in t3 with that record
  // this gives us a list of conditions which we can evaluate
  Term_seq* records = t2->elems();
  Term_seq* conds = new Term_seq();
  for(auto r : *records) {
    Subst sub { subst, r };
    Term* res = subst_term(t->cond(), sub);
    conds->push_back(res);
  }

  // iterate through the table's records and the conditions
  // if the condition evaluates to true then add the record to result
  auto cond_it = conds->begin();
  auto table_it = n_table->elems()->begin();
  if(conds->size() == n_table->elems()->size()) {
    Term_seq* sel_rec = new Term_seq();
    // true
    Term* _true = get_true();
    while(cond_it != conds->end()) {
      //check if the evaluation of the condition is true
      if(is_same(_true, eval(*cond_it))) {
        sel_rec->push_back(*table_it);
      }
      ++cond_it;
      ++table_it;
    }
    n_table->t1 = sel_rec;
  }

  return eval(n_table);
}

Term*
eval_join(Join* t) {
  //perform product between tables
  //perform selection of resulting product
  //perform projection on table
  //populate new table
  //ereturn new table
  return nullptr;
}

Term*
eval_intersect(Intersect* t) {
  //eval t1
  Term* t1 = eval(t->t1);
  Term_seq* e1 = as<List>(t1)->elems();
  //eval t2
  Term* t2 = eval(t->t2);
  Term_seq* e2 = as<List>(t2)->elems();
  //perform intersect
  Term_seq* u = new Term_seq();
  for(auto re1 : *e1) {
    bool contained = false;
    for(auto re2: *e2) {
      if(is_same(re1, re2)) {
        contained = true;
        break;
      }
    }
    if(contained)
      u->push_back(re1);
  }

  //remove duplicates
  return new List(get_type(t1), u);
}

//Assum t1 and t2 are both lists
Term*
eval_union(Union* t) {
  //eval t1
  Term* t1 = eval(t->t1);
  Term_seq* e1 = as<List>(t1)->elems();
  //eval t2
  Term* t2 = eval(t->t2);
  Term_seq* e2 = as<List>(t2)->elems();

  //perform union
  Term_seq* u = new Term_seq();
  u->assign(e1->begin(), e1->end());
  for(auto e0 : *e2) {
    bool contained = false;
    for(auto elem : *u) {
      //if we find it in the union set already break
      if(is_same(e0, elem)) {
        contained = true;
        break;
      } 
    }
    if(!contained)
      u->push_back(e0);
  }
  return new List(get_type(t1), u);
}

//Assume t1 and t2 are both lists
Term*
eval_except(Except* t) {
  //eval t1
  Term* t1 = eval(t->t1);
  Term_seq* e1 = as<List>(t1)->elems();
  //eval t2
  Term* t2 = eval(t->t2);
  Term_seq* e2 = as<List>(t2)->elems();
  //perform except
  Term_seq* u = new Term_seq();
  for(auto re1 : *e1) {
    bool contained = false;
    for(auto re2: *e2) {
      if(is_same(re1, re2)) {
        contained = true;
        break;
      }
    }
    if(!contained)
      u->push_back(re1);
  }

  //remove duplicates
  return new List(get_type(t1), u);
}

} // namespace

// Compute the multi-step evaluation of the term t. 
Term*
eval(Term* t) {
  switch (t->kind) {
  case if_term: return eval_if(as<If>(t));
  case and_term: return eval_and(as<And>(t));
  case or_term: return eval_or(as<Or>(t));
  case not_term: return eval_not(as<Not>(t));
  case equals_term: return eval_equals(as<Equals>(t));
  case less_term: return eval_less(as<Less>(t));
  case succ_term: return eval_succ(as<Succ>(t));
  case pred_term: return eval_pred(as<Pred>(t));
  case iszero_term: return eval_iszero(as<Iszero>(t));
  case app_term: return eval_app(as<App>(t));
  case call_term: return eval_call(as<Call>(t));
  case ref_term: return eval_ref(as<Ref>(t));
  case print_term: return eval_print(as<Print>(t));
  case def_term: return eval_def(as<Def>(t));
  case prog_term: return eval_prog(as<Prog>(t));
  case comma_term: return eval_comma(as<Comma>(t));
  case proj_term: return eval_proj(as<Proj>(t));
  case mem_term: return eval_mem(as<Mem>(t));
  //case col_term: return eval_col(as<Col>(t));
  case select_term: return eval_select_from_where(as<Select_from_where>(t));
  case join_on_term: return eval_join(as<Join>(t));
  case union_term: return eval_union(as<Union>(t));
  case intersect_term: return eval_intersect(as<Intersect>(t));
  case except_term: return eval_except(as<Except>(t));
  default: break;
  }
  return t;
}


// Compute the one-step evaluation of the term t.
Term*
step(Term* t) {
  lang_unreachable("not implemented");
}


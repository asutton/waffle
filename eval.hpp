
#ifndef EVAL_HPP
#define EVAL_HPP

#include "lang/error.hpp"

// This module defines the interface to the evaluation rules of
// the programming language.

struct Term;

// The evaluator class is the primary interface for evaluating
// terms. Note that it keeps its own 
struct Evaluator {
  Term* operator()(Term*);

  Diagnostics diags;
};

Term* step(Term*);
Term* eval(Term*);

#endif

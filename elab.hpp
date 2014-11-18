
#ifndef ELAB_HPP
#define EALB_HPP

#include "lang/error.hpp"

struct Expr;
struct Tree;

struct Elaborator {
  Expr* operator()(Tree* t);

  Diagnostics diags;
};

#endif

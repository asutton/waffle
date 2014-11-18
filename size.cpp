
#include "ast.hpp"

#include "lang/debug.hpp"

// TODO: Implement me.
int
size(Term* t) {
  switch(t->kind) {
  default: break;
  }
  lang_unreachable(format("size of unhandled term '{}'", node_name(t)));
}


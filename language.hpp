
#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "ast.hpp"

// The language class provides a global initialization of resources
// for steve-related programs (compiler, analyzers, etc). In particular,
// it allocates a number of internal types and facilities used by the
// various routines the steve core.
struct Language {
  Language();
  ~Language();
};

#endif

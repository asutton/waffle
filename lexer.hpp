
#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"

#include "lang/error.hpp"

// The lexer is responsible for decomposing a character stream into
// a token stream.
struct Lexer {
  using Iterator = std::string::const_iterator;

  Tokens operator()(const std::string&);
  Tokens operator()(Iterator, Iterator);

  Iterator    first;
  Iterator    last;
  Location    loc;
  Tokens      toks;
  Diagnostics diags;
};

#include "lexer.ipp"

#endif

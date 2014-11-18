
#ifndef PARSER_HPP
#define PARSER_HPP

#include "token.hpp"

#include "lang/error.hpp"

// Declaration
struct Tree;

// The parser transforms a token stream into a parse tree. For
// this language, the parse tree is indistinguishable from the
// abstract syntax tree.
//
// FIXME: Rewrite in terms of a pair of token iterators.
struct Parser {
  using Token_type = Token;

  Tree* operator()(const Tokens&);
  Tree* operator()(Token_iterator, Token_iterator);

  Token_iterator first;   // The beginning token
  Token_iterator last;    // Past the end of the last token
  Token_iterator current; // The current token
  Diagnostics    diags;   // The current diagnostics
};

#include "parser.ipp"

#endif

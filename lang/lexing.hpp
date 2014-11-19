
#ifndef LEXING_HPP
#define LEXING_HPP

#include "tokens.hpp"
#include "location.hpp"
#include "error.hpp"

namespace lex {

// -------------------------------------------------------------------------- //
// Characters

bool is_id_head(char c);
bool is_id_rest(char c);
bool is_digit(char c);
bool is_bin_digit(char c);
bool is_hex_digit(char c);

// -------------------------------------------------------------------------- //
// Lexer control

template<typename L>
  void advance(L&, int = 1);

template<typename L>
  void save(L& lex, Token_kind k, String str);

// -------------------------------------------------------------------------- //
// Lexing rules

template<typename L> bool next_char_is(L& lex, char);

template<typename L> void space(L&);
template<typename L> void newline(L&);
template<typename L> void comment(L& lex);
template<typename L> void ngraph(L&, Token_kind, int);
template<typename L> void unigraph(L&, Token_kind);
template<typename L> void digraph(L&, Token_kind);
template<typename L> void error(L&);
template<typename L> void id(L&);
template<typename L> void integer(L&);
template<typename L> void string(L&);

} // namespace lex

#include "lexing.ipp"

#endif

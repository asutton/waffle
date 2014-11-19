
#include <cctype>
#include <iostream>

#include "lexer.hpp"

#include "lang/lexing.hpp"

namespace {

void 
lex_tokens(Lexer& lex) {
  switch (*lex.first) {
  // Horizontal whitespace
  case ' ':
  case '\t': lex::space(lex); break;

  // Vertical whitespace
  case '\n': lex::newline(lex); break;

  case '/':
    if (lex::next_char_is(lex, '/'))
      lex::comment(lex);
    else
      lex::error(lex);
    break;

  case '(': lex::unigraph(lex, lparen_tok); break;
  case ')': lex::unigraph(lex, rparen_tok); break;
  case '{': lex::unigraph(lex, lbrace_tok); break;
  case '}': lex::unigraph(lex, rbrace_tok); break;
  case '[': lex::unigraph(lex, lbracket_tok); break;
  case ']': lex::unigraph(lex, rbracket_tok); break;
  case '<': lex::unigraph(lex, langle_tok); break;
  case '>': lex::unigraph(lex, rangle_tok); break;
  case ';': lex::unigraph(lex, semicolon_tok); break;
  
  case '=': 
    if (lex::next_char_is(lex, '>'))
      return lex::digraph(lex, map_tok);
    else
      lex::unigraph(lex, equal_tok); break;
    break;
  
  case '\\': lex::unigraph(lex, backslash_tok); break;
  case ':': lex::unigraph(lex, colon_tok); break;
  case '.': lex::unigraph(lex, dot_tok); break;
  case ',': lex::unigraph(lex, comma_tok); break;

  case '-':
    if (lex::next_char_is(lex, '>'))
      return lex::digraph(lex, arrow_tok);
    else
      lex::error(lex);
    break;

  case '"':
    lex::string(lex);
    break;

  default:
    // Maybe this is an identifier, keyowrd, or number.
    if (lex::is_id_head(*lex.first))
      lex::id(lex);
    else if (lex::is_digit(*lex.first))
      lex::integer(lex);
    else
      lex::error(lex);
    break;
  }
}

} // namespace

Tokens
Lexer::operator()(Iterator f, Iterator l) {
  first = f;
  last = l;
  loc = Location();
  use_diagnostics(diags);
  while (first != last)
    lex_tokens(*this);
  return toks;
}


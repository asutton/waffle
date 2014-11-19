
#include <unordered_map>

#include "tokens.hpp"
#include "debug.hpp"

namespace {

// A mapping from token kinds to token names. This is used in
// the parser and lexer (and for debugging) to print the class
// of values expected tokens.
std::unordered_map<Token_kind, String> tokens_;

// A mapping from keyword strings to tokens. This is used by the
// lexer to find token kinds based on the spelling of strings
std::unordered_map<String, Token_kind> keywords_;

inline String
get_name(Token_kind k) {
  char buf[4];
  buf[0] = *((char*)&k);
  buf[1] = *((char*)&k + 1);
  buf[2] = *((char*)&k + 2);
  buf[3] = 0;
  return buf;
}

// Bind the spelling of the token to the given name.
void
save_token(Token_kind k, const char* s) {
  lang_assert(tokens_.count(k) == 0, 
              format("token kind '{0}' already registered", s));
  tokens_.insert({k, s});
}

// Insert the token as a keyword.
void
save_keyword(Token_kind k, const char* s) { 
  lang_assert(keywords_.count(s) == 0, 
              format("keyword '{0}' already registered", s));
  keywords_.insert({s, k}); 
}

} // namespace

void
init_token(Token_kind k, const char* s) {
  save_token(k, s);
  if (token::is_keyword(k))
    save_keyword(k, s);
}

// Given a token kind, return the spelling associated with that 
// identifier. Note that numbered tokens must be registered in the
// token spelling table using the spelling() function. This is typically
// done during language initialization.
String
token_name(Token_kind k) {
  // Check for direct spelling first.
  if (token::is_spelled(k))
    return get_name(k);
  
  // Check the spelling table.
  auto iter = tokens_.find(k);
  if (iter != tokens_.end())
    return iter->second;
  else
    return "<unknown token>";
}

// Returns the token kind associated with the given keyword spelling
// or error_tok if no such keyword is availble.
Token_kind
keyword(String s) {
  auto iter = keywords_.find(s);
  if (iter != keywords_.end())
    return iter->second;
  return error_tok;
}

String
as_string(const Token& k) {
  lang_assert(token::get_type(k.kind) == token_str_type,
              format("token '{0}' is not a string value", k));
  return k.text;
}

// Returns the integer elaboration of the token. If the token is
// not one of the integer tokens, behavior is undefined.
Integer
as_integer(const Token& k) {
  lang_assert(token::get_type(k.kind) == token_int_type,
               format("token '{0}' is not an integer value", k));
  switch (k.kind) {
  case binary_literal_tok: return {k.text, 2};
  case octal_literal_tok: return {k.text, 8};
  case decimal_literal_tok: return {k.text, 10};
  case hexadecimal_literal_tok: return {k.text, 16};
  default: break;
  }
  lang_unreachable("invalid integer token");
}

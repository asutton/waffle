
#ifndef TOKENS_HPP
#define TOKENS_HPP

#include "string.hpp"
#include "integer.hpp"
#include "location.hpp"


// -------------------------------------------------------------------------- //
// Token kind

// The token kind is an encoded binary representation. The high-order
// byte of the token encodes its source language and a bit describing how
// its spelling can be obtained. In paricular, the layout looks like this:
//
//    bXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
//
// Where b indicates whether the token spelling is embedded in the
// representation or is represented externally, in a token table.
//
// When set, the spelling of the token is taken from the low 3 bytes. When
// not set, the remaining 3 bytes are an integer value that indexes the
// the token kind. That is, we have this pattern:
//
//    10000000 CCCCCCCC CCCCCCCC CCCCCCCC
//
// where C denotes the caracter bits. That is, the token can store the
// spelling of tokens of at least 3 characters in length. 
//
// When not set, the token is a unique numeric identifier of 3 bytes. The
// remaining bits in the high-order byte are flags indicating various
// properties of the token.
//
//    0000tttt NNNNNNNN NNNNNNNN NNNNNNNN
//
// Where N denotes values that can be used for identifiers and t denotes
// the type of the token (there are at most 8 types of tokens). The values
// stored in the t-bits are defined by the Token_type value. When the type
// is 0, the token does not represent a concrete value. In this case, the
// token denotes a keyword.
using Token_kind = std::uint32_t;

// The token type denotes how a compiler should elaborate the token.
// That is, how it's internal value should be acquired from the
// token's spelling.
//
// Currently, the primary use of these identifiers is to signify that
// a token is actually a multi-valued set.
using Token_type = std::uint32_t;
constexpr Token_type token_id_type   = 1; // identifier
constexpr Token_type token_bool_type = 2; // bools
constexpr Token_type token_int_type  = 3; // integers
constexpr Token_type token_real_type = 4; // reals
constexpr Token_type token_char_type = 5; // characters
constexpr Token_type token_str_type  = 6; // strings

namespace token {

// Returns true if spelled explicitly.
constexpr bool
is_spelled(Token_kind k) { return k & 0x80000000; }

// Returns true if the token is numbered.
constexpr bool
is_numbered(Token_kind k) { return not is_spelled(k); }

// Returns the token type. Behavior is undefined when then token is
// not numbered.
constexpr Token_type
get_type(Token_kind k) { return k >> 24; }

// Return true if the token is typed. This is only true for numbered
// tokens.
constexpr bool
is_typed(Token_kind k) { return is_numbered(k) and get_type(k) != 0; }

// Returns true if the token denotes a keyword.
constexpr bool
is_keyword(Token_kind k) { return is_numbered(k) and get_type(k) == 0; }

constexpr Token_kind
make_spelled (const char* p, const char* q, std::size_t n) {
  return p == q ? 0 : (*p << (n * 8)) | make_spelled(p + 1, q, n + 1);
}

constexpr Token_kind
make_spelled(const char* str, std::size_t n) {
  return make_spelled(str, str + n, 0);
}

// Make a token whose spelling is represented by the given string. Behavior
// is undefined when the length of the string is greater than 3.
constexpr Token_kind
make_spelled(const char* str) {
  return 0x80000000 | make_spelled(str, std::strlen(str));
}

// Make a numbered token. 
constexpr Token_kind
make_numbered(std::uint32_t n) { return n; }

// Make a typed token. Behavior is undefined if t == 0.
constexpr Token_kind
make_typed(Token_type t, std::uint32_t n) { return (t << 24) | n; }

} // namespace token


// -------------------------------------------------------------------------- //
// Token construction
//
// The following funcitons are used to define tokens.

// Create a common token with the given compact spelling.
constexpr Token_kind
make_token(const char* str) { return token::make_spelled(str); }

// Create a token having the given id.
constexpr Token_kind
make_token(std::uint32_t n) { return token::make_numbered(n); }

// Create a token having the specified type and identifier.
constexpr Token_kind 
make_token(Token_type t, std::uint32_t n) { return token::make_typed(t, n); }


// -------------------------------------------------------------------------- //
// Tokens

// Utility tokens
constexpr Token_kind error_tok               = make_token(0u);
// Identifiers and literals
constexpr Token_kind identifier_tok          = make_token(token_id_type, 1);
constexpr Token_kind binary_literal_tok      = make_token(token_int_type, 2);
constexpr Token_kind octal_literal_tok       = make_token(token_int_type, 3);
constexpr Token_kind decimal_literal_tok     = make_token(token_int_type, 4);
constexpr Token_kind hexadecimal_literal_tok = make_token(token_int_type, 5);
constexpr Token_kind string_literal_tok      = make_token(token_str_type, 10);

// -------------------------------------------------------------------------- //
// Token structure

// A token represents a symbol at a particular location in a
// program's source text.
struct Token {
  Token(Token_kind k, String t);
  Token(Location l, Token_kind k, String t);

  Location   loc;  // The location of the token
  Token_kind kind; // The kind of symbol represented
  String     text; // A textual represntation of the symbol
};

using Tokens = std::vector<Token>;
using Token_iterator = Tokens::const_iterator;


// -------------------------------------------------------------------------- //
// Operations

Token_kind kind(const Token&);
Token_kind kind(const Token*);

// Streaming
template<typename C, typename T>
  std::basic_ostream<C, T>& 
  operator<<(std::basic_ostream<C, T>&, const Token&);

template<typename C, typename T>
  std::basic_ostream<C, T>& 
  operator<<(std::basic_ostream<C, T>&, const Token*);

// -------------------------------------------------------------------------- //
// Token initialization and lookup

void init_token(Token_kind, const char*);
String token_name(Token_kind);
Token_kind keyword(String);

// -------------------------------------------------------------------------- //
// Token elaboration

String as_string(const Token&);
Integer as_integer(const Token&);

#include "tokens.ipp"

#endif

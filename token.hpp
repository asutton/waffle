
#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "lang/string.hpp"
#include "lang/location.hpp"
#include "lang/tokens.hpp"

// -------------------------------------------------------------------------- //
// Token classification

// Operators and punctuators
constexpr Token_kind lparen_tok    = make_token("(");
constexpr Token_kind rparen_tok    = make_token(")");
constexpr Token_kind lbrace_tok    = make_token("{");
constexpr Token_kind rbrace_tok    = make_token("}");
constexpr Token_kind lbracket_tok  = make_token("[");
constexpr Token_kind rbracket_tok  = make_token("]");
constexpr Token_kind langle_tok    = make_token("<");
constexpr Token_kind rangle_tok    = make_token(">");
constexpr Token_kind semicolon_tok = make_token(";");
constexpr Token_kind equal_tok     = make_token("=");
constexpr Token_kind backslash_tok = make_token("\\");
constexpr Token_kind colon_tok     = make_token(":");
constexpr Token_kind dot_tok       = make_token(".");
constexpr Token_kind comma_tok     = make_token(",");
constexpr Token_kind arrow_tok     = make_token("->");
constexpr Token_kind map_tok       = make_token("=>");
// Keywords
constexpr Token_kind def_tok       = make_token(100);
constexpr Token_kind else_tok      = make_token(101);
constexpr Token_kind false_tok     = make_token(102);
constexpr Token_kind if_tok        = make_token(103);
constexpr Token_kind iszero_tok    = make_token(104);
constexpr Token_kind pred_tok      = make_token(105);
constexpr Token_kind print_tok     = make_token(106);
constexpr Token_kind succ_tok      = make_token(107);
constexpr Token_kind then_tok      = make_token(108);
constexpr Token_kind true_tok      = make_token(109);
constexpr Token_kind typeof_tok    = make_token(110);
constexpr Token_kind unit_tok      = make_token(111);
// Type names
constexpr Token_kind bool_type_tok = make_token(200);
constexpr Token_kind nat_type_tok  = make_token(201);
constexpr Token_kind unit_type_tok = make_token(202);

#endif

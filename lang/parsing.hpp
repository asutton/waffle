
#ifndef PARSING_HPP
#define PARSING_HPP

#include "location.hpp"
#include "tokens.hpp"
#include "error.hpp"

// This is a library of generic parsing algorithms. There are two
// concepts, Parser and Token, that must be supplied by a concrete
// parser.
//
// TODO: Make the Parser and Token concepts a little more formal.

namespace parse {

// An alias naming a parser's associated token type.
template<typename P>
  using Token_type = typename P::Token_type;

template<typename P> bool end_of_stream(const P&);

template<typename P> const Token_type<P>* peek(const P&);
template<typename P> const Token_type<P>* peek(const P&, std::size_t);

template<typename P> bool next_token_is(const P&, Token_kind);
template<typename P> bool next_token_is_not(const P&, Token_kind);
template<typename P> bool nth_token_is(const P&, std::size_t, Token_kind);
template<typename P> bool last_token_was(const P&, Token_kind);

template<typename P> const Token_type<P>* consume(P& p);
template<typename P> const Token_type<P>* accept(P&, Token_kind);
template<typename P> const Token_type<P>* expect(P&, Token_kind);

// -------------------------------------------------------------------------- //
// Parser combinators

// A helper type function for extracting the result type from a
// parsing function. 
template<typename P, typename T>
  using Parse_result = decltype(std::declval<T>()(std::declval<P&>()));

template<typename P, typename R>
  Parse_result<P, R> expected(P&, R, const char*);

template<typename P, typename R1, typename R2, typename M>
  Parse_result<P, R1> left(P&, R1, R2, M, const char*);

template<typename P, typename R1, typename R2, typename M>
  Parse_result<P, R1> right(P&, R1, R2, M, const char*);

template<typename P, typename R1, typename R2, typename M>
  Parse_result<P, R1> unary(P&, R1, R2, M, const char*);

template<typename P, typename R>
  Parse_result<P, R> tentative(P&, R);

} // namespace parse

#include "parsing.ipp"

#endif

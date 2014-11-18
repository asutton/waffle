
namespace parse {

// An alias naming a parser's associated token type.
template<typename P>
  using Token_type = typename P::Token_type;

// Returns true if there are no more tokens.
template<typename P>
  inline bool 
  end_of_stream(const P& p) { return p.current == p.last; }

// Returns a pointer to the current token or nullptr if
// the parser has consume the last token.
template<typename P>
  inline const Token_type<P>*
  peek(const P& p) { 
    if (end_of_stream(p))
      return nullptr;
    else
      return &*p.current; 
  }

// Returns a pointer to the nth token past the current token. If the
// nth token is past the end of the token stream, returns nullptr.
template<typename P>
  inline const Token_type<P>*
  peek(const P& p, std::size_t n) {
    if (p.last - p.current > std::ptrdiff_t(n))
      return &*(p.current + n); 
    else
      return nullptr;
  }

// Returns true if the next token has type t.
template<typename P>
  inline bool
  next_token_is(const P& p, Token_kind t) {
    if (auto* k = peek(p))
      return k->kind == t;
    else
      return false;
  }

// Returns true if the next token is something other than type t.
template<typename P>
  inline bool
  next_token_is_not(const P& p, Token_kind t) {
    return not next_token_is(p, t);
  }

// Returns true if the last token had type t.
template<typename P>
  inline bool
  last_token_was(const P& p, Token_kind t) {
    if (p.current == p.first)
      return false;
    auto iter = std::prev(p.current);
    return iter->kind == t;
  }

// Returns true if the nth token has type t.
template<typename P>
  inline bool
  nth_token_is(const P& p, std::size_t n, Token_kind t) { 
    if (auto* tok = peek(p, n))
      return tok->kind == t;
    else
      return false;
  }

// Returns the current location in the program source.
//
// FIXME: REturn EOF instead of no-location.
template<typename P>
  Location
  location(const P& p) { 
    if (auto* k = peek(p))
      return peek(p)->loc; 
    else
      return eof_location;
  }

// Emit an error at the current input location.
template<typename P>
  inline Diagnostic_stream
  parse_error(const P& p) { return error(location(p)); }

// Returns the current token, and advances the parser.
//
// TODO: Implement brace matching for consumed tokens.
template<typename P>
  inline const Token_type<P>*
  consume(P& p) {
    auto* tok = &*p.current;
    ++p.current;
    return tok;
  }

// If the current token is of type T, advance to the next token
// and return true. Otherwise, return false.
template<typename P>
  inline const Token_type<P>*
  accept(P& p, Token_kind k) {
    if (auto* tok = peek(p)) {
      if (tok->kind == k)
        return consume(p);
    }
    return nullptr;
  }

// Require the current token to match t, consuming it. Generate a
// diagnostic if the current token does not match.
template<typename P>
  const Token_type<P>*
  expect(P& p, Token_kind k) {
    if (auto* tok = accept(p, k))
      return tok;

    if (end_of_stream(p)) {
      error(eof_location) << format("expected '{}' but found end of file", 
                                    token_name(k));
    } else {
      error(location(p)) << format("expected '{}' but found '{}'",
                                   token_name(k), 
                                   token_name(p.current->kind));
    }

    return nullptr;
  }

// -------------------------------------------------------------------------- //
// Parser combinators

// A helper type function for extracting the result type from a
// parsing function. 
template<typename P, typename T>
  using Parse_result = decltype(std::declval<T>()(std::declval<P&>()));

// Parse the given rule with the expectation that it succeed. If it
// doesn't succeed, emit a diagnostic.
template<typename P, typename R>
  inline Parse_result<P, R>
  expected(P& p, R rule, const char* msg) {
    if (auto* t = rule(p))
      return t;
    parse_error(p) << format("expected '{}'", msg);
    return nullptr;
  }

// NOTE: The left, right, and unary parser combinators are slightly less
// efficient than hand-coded versions could be because the token kind
// is evaluated twice. Once to match the token, and once to create the
// corresponding node. However, this is only the case when the operator
// matches a set of tokens, mapping to a set of nodes.

// Parse a left-associative binary expression.
//
//    left(sub, op) ::= sub [op sub]*
//
// In this parameterized rule, sub is the sub-grammar matched by the
// parser, and op is a parser that recognizes the set of binary operators
// in the parse.
//
// The make argument is responsible for creating the node based on the
// kind of token.
//
// The msg argument is provided for diagnostics.
template<typename P, typename R1, typename R2, typename M>
  Parse_result<P, R1>
  left(P& p, R1 sub, R2 op, M make, const char* msg) {
    if (auto* l = sub(p)) {
      while (auto* k = op(p)) {
        if (auto* r = expected(p, sub, msg))
          l = make(*k, l, r);
        else
          return nullptr;
      }
      return l;
    }
    return nullptr;
  }

// Parse a right-associative binary expression.
// 
//    right(sub, op) ::= sub [op right(sub, op)]
//
// In this parameterized rule, sub is the sub-grammar matched by the
// parser, and op is a parser that recognizes the set of binary operators
// in the parse.
//
// The make argument is responsible for creating the node based on the
// kind of token.
//
// The msg argument is provided for diagnostics.
template<typename P, typename R1, typename R2, typename M>
  Parse_result<P, R1>
  right(P& p, R1 sub, R2 op, M make, const char* msg) {
    auto recur = [=](P& p) { return right(p, sub, op, make, msg); };
    if (auto* l = sub(p)) {
      if (auto* k = op(p)) {
        if (auto* r = expected(p, recur, msg))
          l = make(*k, l, r);
        else
          return nullptr;
      }
      return l;
    }
    return nullptr;
  }

// Parse a unary expression.
//
//    unary(top, sub, op) ::= sub | op unary(sub, op)
//
// In this parameterized rule, top is the lowest precedence parse matched
// after an operator, sub is the higher precedence parse matched when
// the operator is not present, and op matches the operator.
//
//
// The make argument is responsible for creating the node based on the
// kind of token.
//
// The msg argument is provided for diagnostics.
template<typename P, typename R1, typename R2, typename R3, typename M>
  inline Parse_result<P, R1>
  unary(P& p, R1 top, R2 sub, R3 op, M make, const char* msg) {
    if (auto* k = op(p)) {
      if (auto* t = expected(p, top, msg))
        return make(*k, t);
    } 
    return sub(p);
  }

namespace {

// Begin a tentative parse. This initialize tp with the current
// state of op (but with an empty set of diagnostics).
template<typename P>
  inline void
  begin_tentative_parse(P& op, P& tp) {
    tp.first = op.first;
    tp.last = op.last;
    tp.current = op.current;
    use_diagnostics(tp.diags);
  }

// Commit to a tentative parse. This updates the parser p with
// the state of tp and restores the diagnostics to the original
// parser. Note that any diagnostics accrued during the tentative
// parse are merged into the diagnostics of the original parser.
template<typename P>
  inline void
  commit_tentative_parse(P& op, P& tp) {
    op.current = tp.current;
    op.diags.insert(op.diags.end(), tp.diags.begin(), tp.diags.end());
    use_diagnostics(op.diags);
  }

// Abort the tentative parse. Reset the diagnostics to those
// of the original parser.
template<typename P>
  inline void
  abort_tentative_parse(P& op, P& tp) { 
    use_diagnostics(op.diags);
  }

} // namespace

// Parse a rule tentatively. If the parse fails (returning nullptr), 
// the no changes are made to the parser.
//
// It is currently the responsibility of the writer of the
// tentatively parsed rule to ensure that no changes to the global
// state are leaked from an aborted tentative parse (i.e., scopes).
template<typename P, typename R>
  inline Parse_result<P, R>
  tentative(P& p, R rule) {
    P tp;
    begin_tentative_parse(p, tp);
    if (auto* e = rule(tp)) {
      commit_tentative_parse(p, tp);
      return e;
    }
    abort_tentative_parse(p, tp);
    return nullptr;
  }

} // namespace parse

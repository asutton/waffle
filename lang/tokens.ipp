
inline
Token::Token(Token_kind k, String t)
  : loc(), kind(k), text(t) { }

inline
Token::Token(Location l, Token_kind k, String t)
  : loc(l), kind(k), text(t) { }


// -------------------------------------------------------------------------- //
// Operations

// Returns the token's kind.
inline Token_kind
kind(const Token& t) { return t.kind; }

// Returns the token's kind.
inline Token_kind
kind(const Token* t) { return t->kind; }


// -------------------------------------------------------------------------- //
// Printing

// Quoted output for tokens.

template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const Token& tok) {
    return os << tok.text;
  }

template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const Token* tok) {
    return os << tok->text;
  }

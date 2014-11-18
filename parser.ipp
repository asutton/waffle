
// Parse a sequence of tokens.
inline Tree*
Parser::operator()(const Tokens& toks) {
  return (*this)(toks.begin(), toks.end());
}

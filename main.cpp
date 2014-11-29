
#include <iostream>

#include "language.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "syntax.hpp"
#include "elab.hpp"
#include "ast.hpp"
#include "eval.hpp"

//remove after testing
#include "type.hpp"

int main() {
  Language lang;

  // ------------------------------------------------------------------------ //
  // Character input
  using Iter = std::istreambuf_iterator<char>;
  std::string text(Iter(std::cin), Iter());


  // ------------------------------------------------------------------------ //
  // Lexical analysis
  //
  // Lex the given input text.
  Lexer lex;
  Tokens toks = lex(text);
  if (not lex.diags.empty()) {
    std::cerr << lex.diags;
    return -1;
  }


  // ------------------------------------------------------------------------ //
  // Syntactic analysis
  //
  // Parse the result.
  Parser parse;
  Tree* tree = parse(toks);
  if (not parse.diags.empty()) {
    std::cerr << parse.diags;
    return -1;
  }
  std::cout << "== parsed ==\n" << pretty(tree) << '\n';

  // ------------------------------------------------------------------------ //
  // Elaboration
  //
  // Elaborate the parse tree, producing a fully typed abstract
  // syntax tree.
  Elaborator elab;
  Expr* prog = elab(tree);
  if (not elab.diags.empty()) {
    std::cerr << elab.diags;
    return -1;
  }
  std::cout << "== elaborated ==\n" << pretty(prog) << '\n';

  // ------------------------------------------------------------------------ //
  // Evaluation
  //
  // Evaluate the syntax tree, producing a partially evalutaed
  // abstract syntax tree.
  if (Term* term = as<Term>(prog)) {
    Evaluator eval;
    std::cout << "== output ==\n";
    Expr* result = eval(term);
    std::cout << "== result ==\n" << pretty(result) << '\n';
  } else {
    std::cout << "== no evaluation ==\n";
  }

}

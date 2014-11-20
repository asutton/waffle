
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
  // Testing tables
  // {
  //   //
  //   // Make Table 1
  //   //

  //   Term_seq* schema1 = new Term_seq();
  //   Term_seq* records1 = new Term_seq();

  //   //attributes of the table
  //   Id* name1 = new Id("id");
  //   Id* name2 = new Id("age");
  //   Id* name3 = new Id("is_adult");
  //   Var* attr1 = new Var(name1, get_nat_type());
  //   Var* attr2 = new Var(name2, get_nat_type());
  //   Var* attr3 = new Var(name3, get_bool_type());

  //   schema1->push_back(attr1);
  //   schema1->push_back(attr2);
  //   schema1->push_back(attr3);

  //   //define some values
  //   Zero* zero = new Zero(get_nat_type()); //0
  //   Succ* one = new Succ(get_nat_type(), zero); //1
  //   Succ* two = new Succ(get_nat_type(), one); //2
  //   True* true_ = new True(get_bool_type());
  //   False* false_ = new False(get_bool_type());

  //   //add records
  //   Init* id1 = new Init(get_nat_type(), name1, zero);
  //   Init* age1 = new Init(get_nat_type(), name2, one);
  //   Init* is_adult1 = new Init(get_bool_type(), name3, false_);

  //   Init* id2 = new Init(get_nat_type(), name1, one);
  //   Init* age2 = new Init(get_nat_type(), name2, two);
  //   Init* is_adult2 = new Init(get_bool_type(), name3, true_);

  //   Init* id3 = new Init(get_nat_type(), name1, two);
  //   Init* age3 = new Init(get_nat_type(), name2, one);
  //   Init* is_adult3 = new Init(get_bool_type(), name3, true_);

  //   Term_seq* members1 = new Term_seq();
  //   members1->push_back(id1);
  //   members1->push_back(age1);
  //   members1->push_back(is_adult1);
  //   Term_seq* members2 = new Term_seq();
  //   members2->push_back(id2);
  //   members2->push_back(age2);
  //   members2->push_back(is_adult3);
  //   Term_seq* members3 = new Term_seq();
  //   members3->push_back(id3);
  //   members3->push_back(age3);
  //   members3->push_back(is_adult3);

  //   Record* r1 = new Record(get_kind_type(), members1);
  //   Record* r2 = new Record(get_kind_type(), members2);
  //   Record* r3 = new Record(get_kind_type(), members3);

  //   records1->push_back(r1);
  //   records1->push_back(r2);
  //   records1->push_back(r3);

  //   //make the table
  //   Table* table1 = new Table(get_kind_type(), schema1, records1);


  //   //
  //   // Make Table 2
  //   //

  //   //schema and records for table2
  //   Term_seq* schema2 = new Term_seq();
  //   Term_seq* records2 = new Term_seq();

  //   //reverse the schema so they are no longer the same table
  //   schema2->push_back(attr2); //age
  //   schema2->push_back(attr1); //id
  //   schema2->push_back(attr3); //is_adult

  //   //define some records
  //   Term_seq* members21 = new Term_seq();
  //   members21->push_back(age1);
  //   members21->push_back(id1);
  //   members21->push_back(is_adult1);
  //   Term_seq* members22 = new Term_seq();
  //   members22->push_back(age2);
  //   members22->push_back(id2);
  //   members22->push_back(is_adult3);
  //   Term_seq* members23 = new Term_seq();
  //   members23->push_back(age3);
  //   members23->push_back(id3);
  //   members23->push_back(is_adult3);

  //   Record* r21 = new Record(get_kind_type(), members21);
  //   Record* r22 = new Record(get_kind_type(), members22);
  //   Record* r23 = new Record(get_kind_type(), members23);

  //   records2->push_back(r21);
  //   records2->push_back(r22);
  //   records2->push_back(r23);

  //   //make the table
  //   Table* table2 = new Table(get_kind_type(), schema2, records2);

  //   // 
  //   // Table 3 == Table 1
  //   //
  //   Table* table3 = new Table(get_kind_type(), schema1, records1);

  //   //union
  //   //intersect
  //   //except
  // }

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

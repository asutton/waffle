
#include "token.hpp"

#include "lang/debug.hpp"

void
init_tokens() {
  // Keywords
  init_token(def_tok, "def");
  init_token(else_tok, "else");
  init_token(false_tok, "false");
  init_token(if_tok, "if");
  init_token(iszero_tok, "iszero");
  init_token(print_tok, "print");
  init_token(pred_tok, "pred");
  init_token(succ_tok, "succ");
  init_token(then_tok, "then");
  init_token(true_tok, "true");
  init_token(typeof_tok, "typeof");
  init_token(unit_tok, "unit");
  init_token(and_tok, "and");
  init_token(or_tok, "or");
  init_token(not_tok, "not");
  init_token(eq_comp_tok, "eq");
  init_token(less_tok, "lt");
  // Type names
  init_token(bool_type_tok, "Bool");
  init_token(nat_type_tok, "Nat");
  init_token(unit_type_tok, "Unit");
  // Identifiers and literals
  init_token(identifier_tok, "identifier");
  init_token(decimal_literal_tok, "decimal");
  // Relational algebra identifiers
  init_token(select_tok, "select");
  init_token(from_tok, "from");
  init_token(where_tok, "where");
  init_token(join_tok, "join");
  init_token(on_tok, "on");
  init_token(union_tok, "union");
  init_token(intersect_tok, "intersect");
  init_token(except_tok, "except");
  init_token(as_tok, "as");
}

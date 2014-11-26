
#include "parser.hpp"
#include "syntax.hpp"

#include "lang/parsing.hpp"
#include "lang/debug.hpp"

#include <iostream>

// -------------------------------------------------------------------------- //
// Parsers
//
// The grammar of the term language is laid out like much like the
// that of C or C++. That is, there are several "extension points" in
// the grammar where a term may be added. These are:
//
//    primary -- Parsed at the highest precedence
//    postfix -- A left-associative binary operator
//    prefix -- A unary expression whose operator precedes the operand
//
// Note that the same structure exists for the type parser.

Tree* parse_primary_expr(Parser&);
Tree* parse_postfix_expr(Parser&);
Tree* parse_prefix_expr(Parser&);
Tree* parse_expr(Parser&);
Tree* parse_name(Parser&);


// Parse a name.
//
//    name ::= identifier
Tree*
parse_name(Parser& p) {
  if (const Token* k = parse::accept(p, identifier_tok))
    return new Id_tree(k);
  return nullptr;
}

// Parse a unit literal.
//
//    unit-lit ::= 'unit'
Tree*
parse_unit_lit(Parser& p) {
  if (const Token* k = parse::accept(p, unit_tok))
    return new Lit_tree(k);
  return nullptr;
}

// Parse a boolean literal.
//
//    boolean-lit ::= 'true' | 'false'
Tree*
parse_boolean_lit(Parser& p) {
  if (const Token* k = parse::accept(p, true_tok))
    return new Lit_tree(k);
  if (const Token* k = parse::accept(p, false_tok))
    return new Lit_tree(k);
  return nullptr;
}

// Parse an integer literal.
//
//    integer-literal ::= decimal-literal-token
//
// TODO: Allow for binary, octal, and hexadecimal integers.
Tree*
parse_integer_lit(Parser& p) {
  if (const Token* k = parse::accept(p, decimal_literal_tok))
    return new Lit_tree(k);
  return nullptr;
}

// Parse a string literal.
//
//    string-literal ::= string-literal-token
Tree*
parse_string_lit(Parser& p) {
  if (const Token* k = parse::accept(p, string_literal_tok))
    return new Lit_tree(k);
  return nullptr;
}

// Parse a type literal.
//
//    type-literal ::= 'Unit' | 'Bool' | 'Nat'
Tree*
parse_type_lit(Parser& p) {
  if (const Token* k = parse::accept(p, unit_type_tok))
    return new Lit_tree(k);
  if (const Token* k = parse::accept(p, bool_type_tok))
    return new Lit_tree(k);
  if (const Token* k = parse::accept(p, nat_type_tok))
    return new Lit_tree(k);
  return nullptr;
}

// Parse a literal expression.
//
//    literal-expr ::= unit-literal
//                   | boolean-literal 
//                   | integer-literal 
//                   | string-literal
//                   | type-literal
Tree*
parse_literal_expr(Parser& p) {
  if (Tree* t = parse_unit_lit(p))
    return t;
  if (Tree* t = parse_boolean_lit(p))
    return t;
  if (Tree* t = parse_integer_lit(p))
    return t;
  if (Tree* t = parse_string_lit(p))
    return t;
  if (Tree* t = parse_type_lit(p))
    return t;
  return nullptr;  
}

// Parse an identifer.
//
//    id-expr ::= name
Tree*
parse_id_expr(Parser& p) { return parse_name(p); }

// Parse an inititializer.
//
//    init-expr ::= identifier '=' expr
Tree*
parse_init_expr(Parser& p) {
  if (Tree* n = parse_name(p))
    if (parse::expect(p, equal_tok)) {
      if (Tree* t = parse_expr(p))
        return new Init_tree(n, t);
      else
        parse::parse_error(p) << "expected 'expr' after '='";
    }
  return nullptr;
}

// Parse a variable declaration in a lambda expression.
//
//    parm-decl ::= identifier ':' expr
Tree*
parse_parm_decl(Parser& p) {
  if (Tree* n = parse_name(p))
    if (parse::expect(p, colon_tok)) {
      if (Tree* t = parse_expr(p))
        return new Var_tree(n, t);
      else
        parse::parse_error(p) << "expected 'expr' after ':'";
    }
  return nullptr;
}

// Parse a parameter list.
//
//    parm-list ::= parm | parm-list ',' parm
Tree_seq* 
parse_parm_list (Parser& p) {
  Tree_seq* ts = new Tree_seq();
  while (1) {
    // Parse the next paramter.
    if (Tree* t = parse_parm_decl(p)) {
      ts->push_back(t);
    } else {
      parse::parse_error(p) << "expected 'parm'" ;
      return nullptr;
    }

    // Check for the end of the list.
    if (parse::next_token_is(p, rparen_tok))
      break;

    // Ensure that the next element is a comma.
    if (not parse::expect(p, comma_tok))
      return nullptr;
  }
  return ts;
}

// Parse a parameter clause.
//
//    parm-list ::= '(' [parm-list]? ')'
Tree_seq*
parse_parm_clause(Parser& p) {
  if (parse::accept(p, lparen_tok))
    if (Tree_seq* ps = parse_parm_list(p))
      if (parse::expect(p, rparen_tok))
        return ps;
  return nullptr;
}

// Parse a lambda abstraction.
//
//    lambda-expr ::= '\' parm-decl ':' type '=>' term
//                  | '\' parm-list '=>' term

Tree*
parse_lambda_expr(Parser& p) {
  if (const Token* k = parse::accept(p, backslash_tok)) {
    if(Tree* v = parse_parm_decl(p)) {
      if (parse::expect(p, map_tok)) {
        if (Tree* t = parse_expr(p))
          return new Abs_tree(k, v, t);
        else
          parse::parse_error(p) << "expected 'expr' after '.'";
      }
    } else if (Tree_seq* ps = parse_parm_clause(p)) {
      if (parse::expect(p, map_tok)) {
        if (Tree* t = parse_expr(p))
          return new Fn_tree(k, ps, t);
        else
          parse::parse_error(p) << "expected 'expr' after '.'";
      }
    } else {
      parse::parse_error(p) << "expected 'var-decl' or 'parm-clause' after '\\'";
    }
  }
  return nullptr;
}

// Parse an element of a tuple or a variant.
//
//    elem ::= var | init | expr
Tree*
parse_elem(Parser& p) {
  if (parse::next_token_is(p, identifier_tok)) {
    if (parse::nth_token_is(p, 1, colon_tok))
      return parse_parm_decl(p);
    if (parse::nth_token_is(p, 1, equal_tok))
      return parse_init_expr(p);
    parse::parse_error(p) << "expected ':' or '=' after 'identifier'";
    return nullptr;
  }
  return parse_expr(p);
}

// Parse a comma separated sequence of elements in 
// either a tuple or a variant.
//
//    elem-list ::= elem | elem-list ',' elem
Tree_seq*
parse_elem_list(Parser& p, Token_kind close_tok) {
  Tree_seq* ts = new Tree_seq();
  while (true) {
    // Try parsing the next element.
    if (Tree* t = parse_elem(p))
      ts->push_back(t);
    else
      return nullptr;

    // Either break or continue.
    if (parse::next_token_is(p, close_tok))
      break;
    parse::expect(p, comma_tok);
  }
  return ts;
}

// Parse a comma-separated sequence of expressions that are by a 
// enclosed by pair of tokens.
//
//    enclosed-seq(open, close) ::= open [elem-seq]?  close
//
// Here 'open' and 'close' are tokens represening the opening and
// closing tokens of the enclosed sequence. Note that the sequence
// may be empty.
template<typename T>
  Tree*
  parse_enclosed_seq(Parser& p, Token_kind open_tok, Token_kind close_tok) {
    if (const Token* k = parse::accept(p, open_tok)) {

      if (parse::accept (p, close_tok))
        return new T(k, new Tree_seq());

      if (Tree_seq* ts = parse_elem_list(p, close_tok)) {
        if (parse::expect(p, close_tok))
          return new T(k, ts);
      } else {
        // TODO: expected after the open_token
        parse::parse_error(p) << "expected 'elem-list'";
      }
    }
    return nullptr;
  }


// Parse a tuple expression.
//
//    tuple-expr ::= '{' t1, ..., tn '}'
//
// TODO: There is currently no way to distinguish between the type of
// the empty tuple and its value. There are at least two reasonable
// solutions: allow the empty tuple to be used in place of a type, or
// provide explicit syntax for differentiating the two.
Tree*
parse_tuple_expr(Parser& p) {
  return parse_enclosed_seq<Tuple_tree>(p, lbrace_tok, rbrace_tok);
}

// Parse a list expression.
//
//    list-expr ::= '[' t1, ..., tn ']'
Tree*
parse_list_expr(Parser& p) {
  return parse_enclosed_seq<List_tree>(p, lbracket_tok, rbracket_tok);
}

// Parse a variant expression.
//
//    tuple-expr ::= '<' t1, ..., tn '>'
Tree*
parse_variant_expr(Parser& p) {
  parse_enclosed_seq<Variant_tree>(p, langle_tok, rangle_tok);
}

// Parse a grouped expression.
//
//    grouped-expr ::= '(' [comma-expr]? ')'s
//
// Note the this is only a grouped expression when the form
// is '(e)' where 'e' is some expression. If the expression
// is of the form '()' or '(e1, ..., en)', then this is a comma
// epxression.
Tree*
parse_grouped_expr(Parser& p) {
  if (const Token* k = parse::accept(p, lparen_tok)) {
    // This is a comma expression.
    if (parse::accept(p, rparen_tok))
      return new Comma_tree(k, new Tree_seq());

    if (Tree* t = parse_expr(p)) {
      // This is a grouped subexpression.
      if (parse::accept(p, rparen_tok)) {
        return t;
      }

      // This is a grouped sub-expression.
      Tree_seq* ts = new Tree_seq {t};
      while (parse::accept(p, comma_tok)) {
        if (Tree* t = parse_expr(p))
          ts->push_back(t);
        else
          return nullptr;
      }

      // Make sure we have a closing rparen.
      if (parse::expect(p, rparen_tok)) {
        return new Comma_tree(k, ts);
      }
    } else {
      parse::parse_error(p) << "expected 'expr' after '('";
    }
  }
  return nullptr;
}

// Parse a primary expression.
//
//    primary-term ::= primary-lambda-term | grouped-term
Tree*
parse_primary_expr(Parser& p) {
  if (Tree* t = parse_literal_expr(p))
    return t;
  if (Tree* t = parse_lambda_expr(p))
    return t;
  if (Tree* t = parse_id_expr(p))
    return t;
  if (Tree* t = parse_tuple_expr(p))
    return t;
  if (Tree* t = parse_list_expr(p))
    return t;
  if (Tree* t = parse_variant_expr(p))
    return t;
  if (Tree* t = parse_grouped_expr(p))
    return t;
  return nullptr;
}

// Parse an application expr.
//
//    application-expr ::= postfix-expr primary-expr
Tree*
parse_application_expr(Parser& p, Tree* t1) {
  if (Tree* t2 = parse_primary_expr(p))
    return new App_tree(t1, t2);
  return nullptr;
}

// Parse a dot expression.
//
//    dot-expr ::= postfix-expr '.' primary-expr
Tree*
parse_dot_expr(Parser& p, Tree* t1) {
  if (parse::accept(p, dot_tok))
    if (Tree* t2 = parse_primary_expr(p))
      return new Dot_tree(t1, t2);
    else
      parse::parse_error(p) << "expected 'primary-expr' after '.'";
  return nullptr;
}

// Parse a postfix expr.
//
//    postfix-expr ::= applicaiton-expr
Tree*
parse_postfix_expr(Parser& p) {
  if (Tree* t1 = parse_primary_expr(p)) {
    while (t1) {
      if (Tree* t2 = parse_dot_expr(p, t1))
        t1 = t2;
      else if (Tree* t2 = parse_application_expr(p, t1)) 
        t1 = t2;
      else 
        break;
    }
    return t1;
  }
  return parse_primary_expr(p);
}

// Parse an if-term.
//
//    if-term ::= 'if' term 'then' term 'else' term
Tree*
parse_if_expr(Parser& p) {
  if (const Token* k = parse::accept(p, if_tok))
    if (Tree* t1 = parse_expr(p)) {
      if (parse::expect(p, then_tok))
        if (Tree* t2 = parse_expr(p)) {
          if (parse::expect(p, else_tok)) {
            if (Tree* t3 = parse_expr(p))
              return new If_tree(k, t1, t2, t3);
            else
              parse::parse_error(p) << "expected 'expr' after 'else'";
          }
        } else {
          parse::parse_error(p) << "expected 'expr' after 'then'";
        }
    } else {
      parse::parse_error(p) << "expected 'expr' after 'if'";
    }
  return nullptr;
}

// Parse a successor expression.
//
//    succ-expr ::= 'succ' prefix-expr
Tree*
parse_succ_expr(Parser& p) {
  if (const Token* k = parse::accept(p, succ_tok)) {
    if (Tree* t = parse_prefix_expr(p))
      return new Succ_tree(k, t);
    else
      parse::parse_error(p) << "expected 'prefix-expr' after 'succ'";
  }
  return nullptr;
}

// Parse a predecessor expression.
//
//    pred-expr ::= 'pred' prefix-expr
Tree*
parse_pred_expr(Parser& p) {
  if (const Token* k = parse::accept(p, pred_tok)) {
    if (Tree* t = parse_prefix_expr(p))
      return new Pred_tree(k, t);
    else
      parse::parse_error(p) << "expected 'prefix-expr' after 'pred'";
  }
  return nullptr;
}

// Parse an is-zero expression.
//
//    iszero-expr ::= 'iszero' prefix-expr
Tree*
parse_iszero_expr(Parser& p) {
  if (const Token* k = parse::accept(p, iszero_tok)) {
    if (Tree* t = parse_prefix_expr(p))
      return new Iszero_tree(k, t);
    else
      parse::parse_error(p) << "expected 'prefix-expr' after 'iszero'";
  }
  return nullptr;
}

// Parse a print expression.
//
//    print-expr ::= 'print' expr
Tree*
parse_print_expr(Parser& p) {
  if (const Token* k = parse::accept(p, print_tok)) {
    if (Tree* t = parse_expr(p))
      return new Print_tree(k, t);
    else
      parse::parse_error(p) << "expected 'expr' after 'print'";
  }
  return nullptr;
}

// Parse a typeof expression.
//
//    typeof-expr ::= 'typeof' expr
Tree*
parse_typeof_expr(Parser& p) {
  if (const Token* k = parse::accept(p, typeof_tok)) {
    if (Tree* t = parse_expr(p))
      return new Typeof_tree(k, t);
    else
      parse::parse_error(p) << "expected 'expr' after 'typeof'";
  }
  return nullptr;
}

// Parse a prefix expr.
//
//    prefix-expr ::= if-expr | succ-epxr | pred-expr | iszero-expr
//                  | print-expr | typeof-expr
Tree*
parse_prefix_expr(Parser& p) {
  if (Tree* t = parse_if_expr(p))
    return t;
  if (Tree* t = parse_succ_expr(p))
    return t;
  if (Tree* t = parse_pred_expr(p))
    return t;
  if (Tree* t = parse_iszero_expr(p))
    return t;
  if (Tree* t = parse_print_expr(p))
    return t;
  if (Tree* t = parse_typeof_expr(p))
    return t;
  return parse_postfix_expr(p);
}

// Parse an arrow expression.
//
//    imp-expr ::= prefix-expr
//               | prefix-expr '->' imp-expr
Tree*
parse_arrow_expr(Parser& p) {
  if (Tree* l = parse_prefix_expr(p)) {
    if (parse::accept(p, arrow_tok))
      if (Tree* r = parse_arrow_expr(p))
        return new Arrow_tree(l, r);
    return l;
  }
  return nullptr;
}

// Parse an expression
//
//    expr ::= prefix-expr
Tree*
parse_expr(Parser& p) {
  return parse_arrow_expr(p);
}

// Parse an intializer clause
//    initializer-clause:= '=' expr
Tree*
parse_required_initializer_clause(Parser& p) {
  if (parse::accept(p, equal_tok))
   if (Tree* t = parse_expr(p))
      return t;
   else
      parse::parse_error(p) << "expected 'expr' after '='";
  return nullptr;
}

// Parse a return type. This specifies the result type of a 
// function declarator.
//
//    return-type ::= '->' expr
Tree*
parse_return_type(Parser& p) {

      if (parse::expect(p, arrow_tok))
       if (Tree* t = parse_type_lit(p))
          return t;
      return nullptr;
    
}

// Parse a function declarator.
// fn-decl ::= n(p1, p2, ...) -> t
Tree*
parse_fn_decl(Parser& p, Tree* n) {
  if (Tree_seq* ps = parse_parm_clause(p))    
  //parsing parameter list as Lambda abstraction \(pi:Ti).e
  if (Tree* t = parse_return_type(p))
     return new Func_tree(n, ps, t);
  return nullptr;
}

// Parse a definition expression.
//
//    def_const-expr ::=  name '=' expr
Tree*
parse_const_decl(Parser& p,Tree *n,const Token *k) {
  if (parse::accept(p, equal_tok)) {
   if (Tree* e = parse_expr(p))
      return new Def_tree(k, n, e);
   else
      parse::parse_error(p) << "expected 'expr' after '='";
  }
  return nullptr;
}

// Parse a def-decl.
//
//    def-decl ::= const-decl | fn-decl
//
Tree*
parse_def_decl(Parser& p) {
  if (const Token* k = parse::accept(p, def_tok)) 
    if (Tree* n = parse_name(p)) {
      // Parse the declarator.
      Tree*d1=nullptr;
      if (Tree* d2 = parse_const_decl(p, n,k))
	d1=d2;
      else if (Tree* d2 = parse_fn_decl(p, n))
	d1=d2;
      if (not d1) {
        parse::parse_error(p) << "expected 'parameter-clause' after 'name'";
        return nullptr;
      }
      // Parse the initializer.
      if (Tree* e = parse_required_initializer_clause(p))
        return new Def_tree(k, d1, e);
      else 
        return d1;
  }
  else 
   return nullptr;
}


// Parse a statement.
//
//    stmt ::= def-stmt | expr-stmt
Tree*
parse_stmt(Parser& p) {
  if (Tree* t = parse_def_decl(p))
    return t;
  if (Tree* t = parse_expr(p))
    return t;
  return nullptr;
}

// Parse a program.
//
//    program ::= stmt-list
//    stmt-list ::= stmt ';' | stmt-list ';' stmt
//
Tree*
parse_program(Parser& p) {
  Tree_seq* stmts = new Tree_seq();
  while (not parse::end_of_stream(p)) {
    // Parse the next statement...
    if (Tree* s = parse_stmt(p))
      stmts->push_back(s);
    else
      return nullptr;

    // ... and it's trailing ';'
    if (not parse::expect(p, semicolon_tok))
      return nullptr;
  }
  return new Prog_tree(stmts);
}


// -------------------------------------------------------------------------- //
// Parser

// Parse a range of tokens.
Tree*
Parser::operator()(Token_iterator f, Token_iterator l) {
  if (f == l)
    return nullptr;
  first = f; 
  last = l;
  current = first; 
  use_diagnostics(diags);
  return parse_program(*this);
}

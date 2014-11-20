
#ifndef TYPE_HPP
#define TYPE_HPP

#include "ast.hpp"

// This module defines support functions for querying the type
// of an expression.

Type* get_kind_type();
Type* get_unit_type();
Type* get_bool_type();
Type* get_nat_type();
Type* get_str_type();

bool is_type(Expr*);
bool is_unit_type(Type*);
bool is_bool_type(Type*);
bool is_nat_type(Type*);
bool is_str_type(Type*);
bool is_arrow_type(Type*);
bool is_kind(Expr*);

Type* get_type(Expr*);
Type_seq* get_type(Term_seq*);

#endif

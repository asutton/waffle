
#ifndef VALUE_HPP
#define VALUE_HPP

struct Term;

// This module provides support for querying properties related
// to values.

Term* get_unit();
Term* get_true();
Term* get_false();
Term* get_zero();

bool is_value(Term*);
bool is_boolean_value(Term*);
bool is_integer_value(Term*);
bool is_string_value(Term*);
bool is_list_value(Term*);

bool is_true(Term*);
bool is_false(Term*);
bool is_abs(Term*);
bool is_unit(Term*);

#endif


#ifndef PRETTY_HPP
#define PRETTY_HPP

#include "lang/tokens.hpp"
#include "lang/nodes.hpp"
#include "lang/debug.hpp"

struct Expr;
struct Tree;

template<typename T>
  struct pretty_printer { T* node; };

template<typename T> 
  struct grouped_printer { T phrase; };

template<typename T>
  inline grouped_printer<T> group(T x) { 
    return grouped_printer<T>{x}; 
  }

template<typename T>
  struct commas_printer { Seq<T>* seq; };

template<typename T>
  inline commas_printer<T> 
  commas(Seq<T>* s) { return commas_printer<T>{s}; }

// Print a string.
template<typename C, typename T>
  inline void
  pp_string(std::basic_ostream<C, T>& os, const char* str) {
    os << str;
  }

// Print a node that represents a terminal.
template<typename C, typename T, typename N>
  inline void
  pp_terminal(std::basic_ostream<C, T>& os, N* n) {
    os << n->t1;
  }

bool is_terminal(Tree*);
bool is_terminal(Expr*);

template<typename C, typename T, typename X>
  inline void
  pp_grouped(std::basic_ostream<C, T>& os, X x) {
    if (is_terminal(x.node))
      os << x;
    else
      os << '(' << x << ')';
  }

// Pretty print a comma-separated sequence of t.
template<typename T>
  inline void
  pp_commas(std::ostream& os, Seq<T>* t) {
    auto iter = t->begin();
    auto end = t->end();
    while (iter != end) {
      os << pretty(*iter);
      if (std::next(iter) != end)
        os << ", ";
      ++iter;
    }
  }

// Write out the quoted term. Note that quoting is not applied
// when the expression cannot be more deeply nested.
template<typename C, typename T, typename X>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, grouped_printer<X> g) {
    pp_grouped(os, g.phrase);
    return os;
  }

// Write out the comma-separated sequence.
template<typename C, typename T, typename X>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, commas_printer<X> cs) {
    pp_commas(os, cs.seq);
    return os;
  }

#endif

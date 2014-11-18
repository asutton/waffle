
#ifndef INTEGER_HPP
#define INTEGER_HPP

#include <memory>

#include <gmp.h>

#include "string.hpp"

// The Integer class represents arbitrary integer values.
class Integer {
public:
  // Default constructor
  Integer(int = 10);

  // Copy semantics
  Integer(const Integer&);
  Integer& operator=(const Integer&);

  // Value initialization
  Integer(long, int = 10);
  Integer(String, int = 10);

  // Destructor
  ~Integer();

  // Arithmetic compound assignmnt operators.
  Integer& operator+=(const Integer&);
  Integer& operator-=(const Integer&);
  Integer& operator*=(const Integer&);
  Integer& operator/=(const Integer&);
  Integer& operator%=(const Integer&);

  Integer& neg();
  Integer& abs();

  // Observers
  int bits() const;
  int base() const;
  const mpz_t& data() const;
  
private:
  mpz_t value_;
  int   base_;
};

// Equality
bool operator==(const Integer&, const Integer&);
bool operator!=(const Integer&, const Integer&);

// Ordering
bool operator<(const Integer&, const Integer&);
bool operator>(const Integer&, const Integer&);
bool operator<=(const Integer&, const Integer&);
bool operator>=(const Integer&, const Integer&);

// Arithmetic
Integer operator+(const Integer&, const Integer&);
Integer operator-(const Integer&, const Integer&);
Integer operator*(const Integer&, const Integer&);
Integer operator/(const Integer&, const Integer&);
Integer operator%(const Integer&, const Integer&);
Integer operator-(const Integer&);
Integer operator+(const Integer&);

// Comparison
bool operator==(const Integer&, const Integer&);
bool operator!=(const Integer&, const Integer&);
bool operator<(const Integer&, const Integer&);
bool operator>(const Integer&, const Integer&);
bool operator<=(const Integer&, const Integer&);
bool operator>=(const Integer&, const Integer&);

// Streaming
template<typename C, typename T>
  std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>&, const Integer&);

#include "integer.ipp"

#endif

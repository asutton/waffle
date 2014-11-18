// Default initialize the integer value.
//
// Note that GMP does not optimize for default-intialized values. 
inline
Integer::Integer(int b) : base_(b) { mpz_init(value_); }

// Copy initialize this object with x.
inline
Integer::Integer(const Integer& x) 
  : base_(x.base_) 
{ mpz_init_set(value_, x.value_); }

// Copy assign this object to the value of x.
inline Integer&
Integer::operator=(const Integer& x) {
  if (this != &x) {
    mpz_clear(value_);
    mpz_set(value_, x.value_);
    base_ = x.base_;
  }
  return *this;
}

// Construct an integer with the value n.
inline
Integer::Integer(long n, int b)
  : base_(b)
{ mpz_init_set_si(value_, n); }

// Destroy the ionteger, releasing resources.
inline
Integer::~Integer() { mpz_clear(value_); }

inline Integer& 
Integer::operator+=(const Integer& x) {
  mpz_add(value_, value_, x.value_);
  return *this;
}

inline Integer& 
Integer::operator-=(const Integer& x) {
  mpz_sub(value_, value_, x.value_);
  return *this;
}

inline Integer& 
Integer::operator*=(const Integer& x) {
  mpz_mul(value_, value_, x.value_);
  return *this;
}

// Divide this integer value by x. Integer division is implemented as
// floor division. A discussion of alternatives can be found in the paper,
// "The Euclidean definition of the functions div and mod" by Raymond T.
// Boute (http://dl.acm.org/citation.cfm?id=128862).
inline Integer& 
Integer::operator/=(const Integer& x) {
  mpz_fdiv_q(value_, value_, x.value_);
  return *this;
}

// Compute the remainder of the division of this value_ by x. Integer division
// is implemented as floor division. See the notes on operator/= for more
// discussion.
inline Integer& 
Integer::operator%=(const Integer& x) {
  mpz_fdiv_r(value_, value_, x.value_);
  return *this;
}

// Negate this value.
inline Integer&
Integer::neg() {
  mpz_neg(value_, value_);
  return *this;
}

// Set this value to its absolute value.
inline Integer&
Integer::abs() {
  mpz_abs(value_, value_);
  return *this;
}

// Returns the number of bits in the integer representation.
inline int
Integer::bits() const { return mpz_sizeinbase(value_, 2); }

// Returns the base of in which the inteer should be formatted.
inline int
Integer::base() const { return base_; }

inline const mpz_t& 
Integer::data() const { return value_; }

// Equality comparison
// Returns true when the two integers have the same value.
inline bool
operator==(const Integer& a, const Integer& b) {
  return mpz_cmp(a.data(), b.data()) == 0;
}

inline bool 
operator!=(const Integer& a, const Integer& b) {
  return not(a == b);
}

// Ordering
// Returns true when a is less than b.
inline bool
operator<(const Integer& a, const Integer& b) {
  return mpz_cmp(a.data(), b.data()) < 0;
}

inline bool
operator>(const Integer& a, const Integer& b) {
  return b < a;
}

inline bool
operator<=(const Integer& a, const Integer& b) {
  return not(b < a);
}

inline bool
operator>=(const Integer& a, const Integer& b) {
  return not(a < b);
}

// Arithmetic
inline Integer
operator+(const Integer& a, const Integer& b) {
  return Integer(a) += b;
}

inline Integer
operator-(const Integer& a, const Integer& b) {
  return Integer(a) -= b;
}

inline Integer
operator*(const Integer& a, const Integer& b) {
  return Integer(a) *= b;
}

inline Integer
operator/(const Integer& a, const Integer& b) {
  return Integer(a) /= b;
}

inline Integer
operator%(const Integer& a, const Integer& b) {
  return Integer(a) %= b;
}

inline Integer 
operator-(const Integer& x) { 
  return Integer(x).neg(); 
}

inline Integer 
operator+(const Integer& x) { 
  return x; 
}

// Streaming
template<typename C, typename T>
  inline std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const Integer& z) {
    int  base = z.base();
    std::size_t n = mpz_sizeinbase(z.data(), base) + 1;
    std::unique_ptr<char[]> buf(new char[n]);
    switch (base) {
      case 8:
        gmp_snprintf(buf.get(), n, "%Zo", z.data());
        break;
      case 10:
        gmp_snprintf(buf.get(), n, "%Zd", z.data());
        break;
      case 16:
        gmp_snprintf(buf.get(), n, "%Zx", z.data());
        break;
    }
    return os << buf.get(); 
  }


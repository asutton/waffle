
#include <stdexcept>

#include "integer.hpp"
#include "debug.hpp"

// Consruct an integer with the value in s in base b. Behavior is undefined
// if s does not represent an integer in base b.
Integer::Integer(String s, int b) 
  : base_(b)
{
  if (mpz_init_set_str(value_, s.data(), base_) == -1)
    lang_unreachable("invalid integer representation");
}

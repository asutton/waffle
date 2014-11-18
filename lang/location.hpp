
#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iosfwd>

// Types for special location constructors.
enum no_location_t { no_location };
enum eof_location_t { eof_location };

// A location represents a position in a source file, indicated by its
// line and character offset.
//
// TODO: Ensure that we can enregister source locations.
//
// TODO: Map source locations to files? 
struct Location {
  Location() = default;
  Location(no_location_t);
  Location(eof_location_t);
  
  bool is_internal() const;
  bool is_eof() const;

  int line = 1;
  int col = 1;
};

// Output formatting
template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>&, const Location&);

#include "location.ipp"

#endif

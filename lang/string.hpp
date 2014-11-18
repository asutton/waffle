#ifndef STRING_HPP
#define STRING_HPP

#include <cstring>
#include <string>
#include <iosfwd>

#include "format.hpp"

// The String class is a handle to an interned string. Its usage guarantees
// that each unique occurrence of a string in the text of a program appears
// only once in the memory of the program.
//
// The String class is a regular, but reference semantic type.
class String {
public:
  using iterator       = std::string::const_iterator;
  using const_iterator = std::string::const_iterator;

  // Constructors
  String();
  String(const std::string& s);
  String(const char* s);
  String(const char* s, std::size_t n);

  template<typename I> String(I first, I last);

  // Conversion to bool
  explicit operator bool() const;

  // Observers
  std::size_t size() const;
  const std::string* ptr() const;
  const std::string& str() const;
  const char* data() const;

  // Iterators
  iterator begin();
  iterator end();

  const_iterator begin() const;
  const_iterator end() const;

private:
  static const std::string* intern(const std::string&);

private:
  const std::string* str_;
};

// Equality comparison
bool operator==(String, String);
bool operator!=(String, String);

// Ordering
bool operator<(String, String);
bool operator>(String, String);
bool operator<=(String, String);
bool operator>=(String, String);

// String algorithms
String to_lower(String);
String to_upper(String);

// Formatting
template<typename C> 
  fmt::StrFormatSpec<C> pad(const String&, unsigned, C);

// Streaming
template<typename C, typename T>
  std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>&, String);

// Hash support for Strings.
namespace std {

template<>
  struct hash<String> {
    std::size_t operator()(String str) const;
  };

} // namesapce std

#include "string.ipp"

#endif

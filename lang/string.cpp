
#include <cctype>
#include <algorithm>
#include <unordered_set>

#include "string.hpp"

namespace {

// The string table.
static std::unordered_set<std::string> strings_;

} // namesapce

// Returns a pointer to a unique string with the same spelling as str.
const std::string* 
String::intern(const std::string& str) { return &*strings_.insert(str).first; }

// Convert a string to lowercase.
String
to_lower(String s) {
  std::string r(s.size(), 0);
  std::transform(s.begin(), s.end(), r.begin(), tolower);
  return r;
}

String
to_upper(String s) {
  std::string r(s.size(), 0);
  std::transform(s.begin(), s.end(), r.begin(), toupper);
  return r;
}


inline 
String::String() 
  : str_(nullptr) { }
  
inline
String::String(const std::string& s)
  : str_(intern(s)) { }

inline
String::String(const char* s)
  : String(std::string(s)) { }

inline
String::String(const char* s, std::size_t n)
  : String(std::string(s, n)) { }

template<typename I>
inline
String::String(I first, I last)
  : String(std::string(first, last)) { }


/// Returns true if the string is non-null.
inline 
String::operator bool() const { return str_; }

/// Returns the number of characters in the string.
inline std::size_t 
String::size() const { return str_->size(); }

/// Returns the underlying string pointer.
inline const std::string* 
String::ptr() const { return str_; }

/// Returns a reference to the underlying string.
inline const std::string& 
String::str() const { return *str_; }

/// Returns a pointer to the underlying character data.
inline const char* 
String::data() const { return str().c_str(); }

// Iterators
inline String::iterator 
String::begin() { return str_->begin(); }

inline String::iterator 
String::end() { return str_->end(); }

inline String::const_iterator
String::begin() const { return str_->begin(); }

inline String::const_iterator 
String::end() const { return str_->end(); }

// Equality comparison
// Returns true when two strings refer to the same object.

inline bool
operator==(String a, String b) { return a.ptr() == b.ptr(); }

inline bool
operator!=(String a, String b) { return a.ptr() != b.ptr(); }

// Ordering
// Returns true when a points to an object with a lesser address
// than b. This operation does not define a lexicographical order.

inline bool
operator<(String a, String b) { return a.ptr() < b.ptr(); }

inline bool
operator>(String a, String b) { return a.ptr() > b.ptr(); }

inline bool
operator<=(String a, String b) { return a.ptr() <= b.ptr(); }

inline bool
operator>=(String a, String b) { return a.ptr() >= b.ptr(); }

// Formatting
template<typename C> 
  inline fmt::StrFormatSpec<C> 
  pad(const String& s, unsigned w, C f) {
    return fmt::StrFormatSpec<C>(s.data(), w, f);
  }

// Streaming
template<typename C, typename T>
  inline std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, String s) { return os << s.str(); }

namespace std {

inline std::size_t 
hash<String>::operator()(String str) const {
  hash<const void*> h;
  return h(str.ptr());
}

} // namespace std

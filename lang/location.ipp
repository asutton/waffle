
// Initialize an empty location.
inline
Location::Location(no_location_t)
  : line(0), col(0) { }

// Initialize an empty location.
inline
Location::Location(eof_location_t)
  : line(-1), col(0) { }

inline bool
Location::is_internal() const { return line == 0; }

inline bool
Location::is_eof() const { return line == -1; }

// Output for source locations.
template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const Location& loc) {
    if (loc.is_internal())
      return os ;
    if (loc.is_eof())
      return os << "<eof>:";
    return os << loc.line << ':' << loc.col;
  }

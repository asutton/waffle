
inline
Assertion_error::Assertion_error(const char* msg)
  : std::runtime_error(msg) { }

inline
Assertion_error::Assertion_error(const std::string& msg)
  : std::runtime_error(msg) { }

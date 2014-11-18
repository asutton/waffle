

add_library(waffle-support STATIC
  debug.cpp
  format.cpp
  string.cpp
  integer.cpp
  location.cpp
  error.cpp
  tokens.cpp
  nodes.cpp
  lexing.cpp
  parsing.cpp
  printing.cpp)
target_link_libraries(waffle-support gmp)


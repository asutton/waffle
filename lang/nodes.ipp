
// Returns the node t dynamically converted to the node type U. If t does
// not have the dynamic type U, the resulting term is null.
template<typename U, typename T>
  inline U*
  as(T* t) { return dynamic_cast<U*>(t); }

template<typename U, typename T>
  inline const U*
  as(const T* t) { return dynamic_cast<const U*>(t); }

// Returns true if node t has dynamic type U.
template<typename U, typename T>
  inline bool 
  is(const T* t) { return as<U>(t); }




// Create a mapping from each x in xs to its corresponding
// s in ss.
template<typename T, typename U>
  inline
  Subst::Subst(Seq<T>* xs, Seq<U>* ss) {
    lang_assert(xs->size() == ss->size(), "invalid substitution");
    auto xi = xs->begin();
    auto xe = xs->end();
    auto si = ss->begin();
    while (xi != xe) {
      insert({*xi, *si});
      ++xi;
      ++si;
    }
  }
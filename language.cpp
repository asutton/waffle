
#include "language.hpp"

#include "lang/debug.hpp"

extern void init_tokens();
extern void init_nodes();
extern void init_types();
extern void init_values();

namespace {
// Language initialization flag.
bool init_ = false;

void
init_lang() {
  lang_assert(not init_, "language already initialzied");
  if (not init_)
    init_ = true;
}

} // naemspace


Language::Language() {
  init_lang();
  init_tokens();
  init_nodes();
  init_types();
  init_values();
}

Language::~Language() { }


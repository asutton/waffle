
#include "error.hpp"
#include "debug.hpp"

#include <iostream>

namespace {

// The global diagnostics pointer.
Diagnostics* diags_ = nullptr;

// Register a diagnostic with the diagnostic list.
template<typename D>
  inline Diagnostic*
  make_diag(Diagnostics& ds, const Location& loc) {
    D* d = new D(loc);
    ds.push_back(d);
    return d;
  }

// Make sure that diagnostics are installed.
inline void
check_diagnostics() {
  lang_assert(diags_, "diagnostics not initialized");
}

} // namespace

// The set the global diagnostics pointer to the give diagnostics. This
// is generally set in the constructor of a processing phase (e.g.,
// the parser). All calls to diagnostic constructors will modify this
// object.
void
use_diagnostics(Diagnostics& ds) {
  diags_ = &ds;
}

// -------------------------------------------------------------------------- //
// Streaming

Diagnostic_stream
operator<<(Diagnostic_stream ds, char c) {
  ds.diag->msg += c;
  return ds;
}

Diagnostic_stream
operator<<(Diagnostic_stream ds, const char* msg) {
  ds.diag->msg += msg;
  return ds;
}

Diagnostic_stream
operator<<(Diagnostic_stream ds, const std::string& msg) {
  ds.diag->msg += msg;
  return ds;
}

Diagnostic_stream
operator<<(Diagnostic_stream ds, String msg) {
  ds.diag->msg += msg.str();
  return ds;
}

Diagnostic_stream
operator<<(Diagnostic_stream ds, int n) {
  std::stringstream ss;
  ss << n;
  ds.diag->msg += ss.str();
  return ds;
}

Diagnostic_stream
operator<<(Diagnostic_stream ds, Integer n) {
  std::stringstream ss;
  ss << n;
  ds.diag->msg += ss.str();
  return ds;
}

// Create a new error diagnostic.
Diagnostic_stream
error(const Location& loc) { 
  check_diagnostics();
  return {make_diag<Error>(*diags_, loc)};
}

// Create a new error diagnostic.
Diagnostic_stream
error(Diagnostics& ds, const Location& loc) {
  return {make_diag<Error>(ds, loc)};
}

// Create a new warning diagnostic.
Diagnostic_stream
warn(const Location& loc) { 
  check_diagnostics();
  return warn(*diags_, loc); 
}

// Create a new warning diagnostic.
Diagnostic_stream
warn(Diagnostics& ds, const Location& loc) {
  return {make_diag<Warning>(ds, loc)};
}

// Create a new note diagnostic.
Diagnostic_stream
note(const Location& loc) { 
  check_diagnostics();
  return note(*diags_, loc); 
}

// Create a new note diagnostic.
Diagnostic_stream
note(Diagnostics& ds, const Location& loc) {
  return {make_diag<Note>(ds, loc)};
}

// Create a new sorry diagnostic.
Diagnostic_stream
sorry(const Location& loc) { 
  check_diagnostics();
  return sorry(*diags_, loc); 
}

// Create a new sorry diagnostic.
Diagnostic_stream
sorry(Diagnostics& ds, const Location& loc) {
  return {make_diag<Sorry>(ds, loc)};
}

// -------------------------------------------------------------------------- //
// Print diagnostics

namespace {

String
diagnostic_name(const Diagnostic* d) {
  switch (d->kind) {
  case diag_error: return "error";
  case diag_warning: return "warning";
  case diag_note: return "note";
  case diag_sorry: return "sorry";
  default: break;
  }
  lang_unreachable("unknown diagnostic kind");
}

} // namespace

// Write the diagnostics to the output stream.
void
print(std::ostream& os, const Diagnostics& ds) {
  for (const Diagnostic* d : ds) {
    os << diagnostic_name(d) << ": " ;
    os << d->loc << ": ";
    os << d->msg << "\n";
  }
}

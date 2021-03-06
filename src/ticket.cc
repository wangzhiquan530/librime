//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2013-10-28 GONG Chen <chen.sst@gmail.com>
//
#include <rime/engine.h>
#include <rime/ticket.h>

namespace rime {

Ticket::Ticket(Schema* s, const std::string& ns)
    : schema(s), name_space(ns) {
}

Ticket::Ticket(Engine* e, const std::string& ns,
               const std::string& prescription)
    : engine(e),
      schema(e ? e->schema() : NULL),
      name_space(ns),
      klass(prescription) {
  size_t separator = klass.find('@');
  if (separator != std::string::npos) {
    name_space = klass.substr(separator + 1);
    klass.resize(separator);
  }
}

}  // namespace rime

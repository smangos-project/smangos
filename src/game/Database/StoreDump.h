#ifndef MANGOS_STOREDUMP_H
#define MANGOS_STOREDUMP_H

#include <string>

// Writes every world store loaded at start-up as one TSV file per store into dir.
bool DumpStores(std::string const& dir);

#endif

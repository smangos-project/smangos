#pragma once

// Diagnostic-only hooks (ws2_32 + secur32) to see what the client does with
// the REST login socket (127.0.0.1:8081) between TLS handshake completion and
// the connection closing. See NetDiagnostics.cpp for what's actually hooked.
bool InstallNetDiagnosticsHooks();

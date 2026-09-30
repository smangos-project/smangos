#pragma once

// Hooks crypt32.dll!CertVerifyCertificateChainPolicy so the client accepts our
// self-signed bnetserver certificate without it being present in the Windows
// trusted root store. See CertBypass.cpp for exactly which chain-trust errors
// get suppressed.
bool InstallCertBypassHook();

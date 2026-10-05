# Certificate bypass

**Code:** `client/client_library/src/CertBypass.cpp`

The client refuses the TLS handshake with `bnetserver` unless its self-signed certificate is in the Windows trusted root store. `custom.dll` hooks the system certificate verification calls (`CertVerifyCertificateChainPolicy`, `CertGetCertificateChain`) and clears only the chain-trust errors, so a real bad signature still fails.

## What you can do

- Connect the client to a local `bnetserver` without installing its certificate.
- Log in through the REST login portal and the Battle.net launcher flow (see `client/autologin.sh`).

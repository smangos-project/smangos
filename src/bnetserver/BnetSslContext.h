#ifndef BNETSERVER_BNETSSLCONTEXT_H
#define BNETSERVER_BNETSSLCONTEXT_H

#include <string>

typedef struct ssl_ctx_st SSL_CTX;

// Builds the server-side TLS context shared by the REST portal and the RPC port. Returns nullptr
// (and logs why) if the certificate chain or private key cannot be loaded or do not match.
SSL_CTX* BnetCreateServerSslContext(std::string const& certificateChainFile, std::string const& privateKeyFile);

#endif

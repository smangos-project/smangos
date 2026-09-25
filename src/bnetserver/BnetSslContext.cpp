#include "BnetSslContext.h"
#include "Log.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

SSL_CTX* BnetCreateServerSslContext(std::string const& certificateChainFile, std::string const& privateKeyFile)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Could not create the TLS context");
        return nullptr;
    }

    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

    if (SSL_CTX_use_certificate_chain_file(ctx, certificateChainFile.c_str()) != 1)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Could not load the TLS certificate chain from '%s'", certificateChainFile.c_str());
        ERR_clear_error();
        SSL_CTX_free(ctx);
        return nullptr;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, privateKeyFile.c_str(), SSL_FILETYPE_PEM) != 1)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Could not load the TLS private key from '%s'", privateKeyFile.c_str());
        ERR_clear_error();
        SSL_CTX_free(ctx);
        return nullptr;
    }

    if (SSL_CTX_check_private_key(ctx) != 1)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "The TLS private key '%s' does not match the certificate '%s'", privateKeyFile.c_str(), certificateChainFile.c_str());
        ERR_clear_error();
        SSL_CTX_free(ctx);
        return nullptr;
    }

    return ctx;
}

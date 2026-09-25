#!/bin/sh
# Development-only self-signed certificate for bnetserver's TLS ports. Production servers configure
# their own CertificatesFile and PrivateKeyFile instead of using this one.
openssl req -x509 -newkey rsa:2048 -nodes -days 3650 -subj "/CN=localhost" \
    -addext "subjectAltName=DNS:localhost,IP:127.0.0.1" \
    -keyout bnetserver.key.pem -out bnetserver.cert.pem

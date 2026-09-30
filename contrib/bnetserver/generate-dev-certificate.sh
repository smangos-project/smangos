#!/bin/sh
# Development-only self-signed certificate for bnetserver's TLS ports. Production servers configure
# their own CertificatesFile and PrivateKeyFile instead of using this one. The client matches the
# address it connects to against the DNS names only, so 127.0.0.1 is listed as one as well as an IP.
openssl req -x509 -newkey rsa:2048 -nodes -days 3650 -subj "/CN=127.0.0.1" \
    -addext "subjectAltName=DNS:127.0.0.1,DNS:localhost,IP:127.0.0.1" \
    -addext "basicConstraints=critical,CA:TRUE" \
    -keyout bnetserver.key.pem -out bnetserver.cert.pem

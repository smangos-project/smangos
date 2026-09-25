/*
 * Per-connection AES-256-GCM packet cipher for the world socket. Takes a PacketCryptRole so the
 * same class serves both the bot (Client) and the server (Server), tested half against half with
 * no socket/session dependency on either side.
 *
 * Nonce = [8-byte LE counter][4-byte LE magic], magic distinguishes direction so the same key
 * never reuses a nonce between client->server and server->client traffic. Counters increment
 * on every packet in that direction, starting from before the cipher is armed (IsInitialized()
 * false) -- both sides must stay in lockstep from packet #0, not just from the first encrypted
 * packet, or the stream desyncs the moment encryption is armed.
 */

#ifndef _MANGOS_CRYPTO_ENCRYPTION_PACKETCRYPT_H
#define _MANGOS_CRYPTO_ENCRYPTION_PACKETCRYPT_H

#include "AES_GCM.h"

enum class PacketCryptRole
{
    Client,
    Server,
};

class PacketCrypt
{
    public:
        explicit PacketCrypt(PacketCryptRole role);
        PacketCrypt(PacketCryptRole role, AES_GCM::Key const& key);

        void Init(AES_GCM::Key const& key);
        bool PeekDecryptRecv(uint8* data, size_t length);
        bool DecryptRecv(uint8* data, size_t length, AES_GCM::Tag& tag);
        bool EncryptSend(uint8* data, size_t length, AES_GCM::Tag& tag);

        bool IsInitialized() const { return m_initialized; }

        // Diagnostics only (see PacketHexDumper's counter/tag logging) -- the counter value that
        // will be used for the NEXT EncryptSend()/DecryptRecv() call, i.e. pre-increment.
        uint64 GetSendCounter() const { return m_sendCounter; }
        uint64 GetRecvCounter() const { return m_recvCounter; }

    protected:
        AES_GCM m_decrypt;
        AES_GCM m_encrypt;
        uint32 m_sendMagic;
        uint32 m_recvMagic;
        uint64 m_sendCounter;
        uint64 m_recvCounter;
        bool m_initialized;
};

#endif

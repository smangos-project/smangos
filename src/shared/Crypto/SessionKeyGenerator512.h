/*
 * Same algorithm as SessionKeyGenerator.h, but keyed on SHA-512 -- the cata classic client's
 * world session key is derived via SessionKeyGenerator<SHA512> (TrinityCore's
 * WorldSocket::HandleAuthSessionCallback), not SHA-256 like the older 1.14.2 protocol. Kept as
 * a separate class rather than templating SessionKeyGenerator itself, matching that file's own
 * stated preference for duplication over genericity.
 */

#ifndef _MANGOS_CRYPTO_SESSIONKEYGENERATOR512_H
#define _MANGOS_CRYPTO_SESSIONKEYGENERATOR512_H

#include "Crypto/Hash/SHA512.h"

class SessionKeyGenerator512
{
    public:
        template <typename C>
        explicit SessionKeyGenerator512(C const& buf) : m_o0(SHA512::CreateZero()), m_o0Pos(0)
        {
            uint8 const* data = buf.data();
            size_t const len = buf.size();
            size_t const halfLen = len / 2;

            m_o1 = SHA512::ComputeFrom(data, halfLen);
            m_o2 = SHA512::ComputeFrom(data + halfLen, len - halfLen);
            m_o0 = NextRound();
            m_o0Pos = 0;
        }

        void Generate(uint8* buf, uint32 sz)
        {
            for (uint32 i = 0; i < sz; ++i)
            {
                if (m_o0Pos == m_o0.size())
                {
                    m_o0 = NextRound();
                    m_o0Pos = 0;
                }

                buf[i] = m_o0[m_o0Pos++];
            }
        }

    private:
        SHA512::Digest NextRound()
        {
            SHA512::Generator gen;
            gen.UpdateData(m_o1);
            gen.UpdateData(m_o0);
            gen.UpdateData(m_o2);
            return gen.GetDigest();
        }

        SHA512::Digest m_o0;
        SHA512::Digest m_o1;
        SHA512::Digest m_o2;
        size_t m_o0Pos;
};

#endif

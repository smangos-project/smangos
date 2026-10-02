/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MANGOS_BNET_SRP6_H
#define _MANGOS_BNET_SRP6_H

#include "Common.h"
#include "Crypto/BigNumber.h"

#include <string>
#include <vector>

/*! Battle.net SRP6, protocol version 1 (SHA-256 flavor).
 *
 * This is the SRP6 variant modern Battle.net clients speak for bnetserver's REST login
 * (the only SRP6 implementation left in the tree -- the vanilla realmd-style SRP6 class
 * the legacy world-login handshake used is gone with realmd itself). Only version 1 is
 * implemented: the client offers versions 1 and 2 in its SrpLoginChallenge response and
 * works with whichever one the server advertised, so v1 (no PBKDF2 stretching) is enough here.
 *
 * `username` passed to every method must already be normalized to uppercase, matching
 * how `account.username` is stored (see AccountMgr::normalizeString).
 *
 * The host (server) half verifies a login; the client half performs one -- standard SRP6 client
 * math (RFC 2945-style: S = (B - k*g^x)^(a+u*x) mod N), built from the exact same
 * CalculateX/CalculateU/GetK/ComputeEvidence primitives the host half uses, so the two are
 * provably the same protocol from either side (see tests-shared's "BnetSRP6 client and server
 * halves agree" for the half-against-half proof).
 */
class BnetSRP6
{
    public:
        static const int SALT_BYTE_SIZE = 32;
        static const int N_BYTE_SIZE = 128;  // byte length of the v1 modulus N (1024-bit)
        static const int EVIDENCE_SIZE = 32; // SHA-256 digest size, used for M1/M2

        BnetSRP6();

        //! Registration / password-change: picks a fresh random salt and computes the
        //! verifier for (username, password). Populates GetSalt()/GetVerifier().
        void CalculateVerifier(std::string const& username, std::string const& password);

        //! Loads a previously stored salt+verifier to run a login challenge/response
        //! exchange, or to use CheckCredentials() directly.
        void Initialize(std::vector<uint8> const& salt, std::vector<uint8> const& verifier);

        //! Recomputes the verifier for (username, password) against the salt passed to
        //! Initialize() and compares it to the stored verifier. Non-interactive shortcut,
        //! doesn't require the ephemeral exchange below.
        bool CheckCredentials(std::string const& username, std::string const& password) const;

        std::vector<uint8> const& GetSalt() const { return m_salt; }
        std::vector<uint8> GetVerifier() const;

        //! Algorithm constants, for advertising in a SrpLoginChallenge.
        static std::vector<uint8> GetModulus();
        static std::vector<uint8> GetGenerator();

        //! The SRP "username"/identity actually used in the protocol is hex(SHA256(login))
        //! -- NOT the raw account name -- both when a verifier is first computed
        //! (CalculateVerifier's `username` argument) and in the `username` field of a
        //! SrpLoginChallenge sent to the client (which echoes it back verbatim on its proof
        //! submission). `login` must already be normalized/uppercased.
        static std::string ComputeSrpUsername(std::string const& login);

        //! Interactive login, host (server) side (call after Initialize()): server's
        //! ephemeral key pair.
        void CalculateHostPublicEphemeral();
        std::vector<uint8> GetHostPublicEphemeral() const; // B, fixed N_BYTE_SIZE bytes

        //! Verifies the client's public ephemeral (A) and evidence (M1, up to EVIDENCE_SIZE
        //! bytes). Returns false if the exchange is invalid (degenerate A/u, or M1
        //! mismatch) -- do not call CalculateServerEvidence() in that case.
        bool VerifyClientEvidence(std::vector<uint8> const& A, std::vector<uint8> const& clientEvidenceM1);

        //! Only valid after VerifyClientEvidence() returned true. Returns M2
        //! (EVIDENCE_SIZE bytes) to send back to the client as proof this side also
        //! knows the shared secret.
        std::vector<uint8> CalculateServerEvidence() const;

        //! Interactive login, client (bot) side: the bot's own ephemeral key pair.
        void CalculateClientPublicEphemeral();
        std::vector<uint8> GetClientPublicEphemeral() const; // A, fixed N_BYTE_SIZE bytes

        //! Derives the shared secret from (username, password, salt, B) -- the salt and B
        //! a SrpLoginChallenge response carries -- and returns the client evidence M1 to
        //! submit. Returns an empty vector if B is degenerate (0 mod N); call
        //! CalculateClientPublicEphemeral() first.
        std::vector<uint8> CalculateClientEvidence(std::string const& username, std::string const& password,
            std::vector<uint8> const& salt, std::vector<uint8> const& hostPublicEphemeralB);

        //! Only valid after CalculateClientEvidence() returned a non-empty M1. Compares the
        //! server's M2 against this side's own recomputation.
        bool VerifyServerEvidence(std::vector<uint8> const& serverEvidenceM2) const;

    private:
        BigNumber CalculateX(std::string const& username, std::string const& password, std::vector<uint8> const& salt) const;

        std::vector<uint8> m_salt;
        BigNumber m_v;
        BigNumber m_b, m_B;
        BigNumber m_a, m_A;
        BigNumber m_S, m_clientM1;
};

#endif

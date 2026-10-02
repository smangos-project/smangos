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

#include "Crypto/Authentication/BnetSRP6.h"
#include "Crypto/Hash/SHA256.h"
#include "Util.h"

namespace
{
    // Algorithm parameters for Battle.net SRP6 v1 (N and g).
    BigNumber const& GetN()
    {
        static BigNumber const n = []
        {
            BigNumber b;
            b.SetHexStr("86A7F6DEEB306CE519770FE37D556F29944132554DED0BD68205E27F3231FEF5A10108238A3150C59CAF7B0B6478691C13A6ACF5E1B5ADAFD4A943D4A21A142B800E8A55F8BFBAC700EB77A7235EE5A609E350EA9FC19F10D921C2FA832E4461B7125D38D254A0BE873DFC27858ACB3F8B9F258461E4373BC3A6C2A9634324AB");
            return b;
        }();
        return n;
    }

    BigNumber const& Getg()
    {
        static BigNumber const g(2);
        return g;
    }

    // I = H(username), interpreted little-endian -- vmangos's BigNumber::SetBinary already
    // performs that interpretation directly (it reverses the input bytes before handing
    // them to BN_bin2bn, which reads big-endian), matching the reference's default
    // (reverse=true) BigNumber-from-digest construction used for CalculateX.
    BigNumber DigestToBigNumberLE(SHA256::Digest const& digest)
    {
        BigNumber bn;
        bn.SetBinary(digest.data(), (int)digest.size());
        return bn;
    }

    // Inverse of BigNumber::AsByteArray(size, /*reverse=*/false): reconstructs a BigNumber
    // from its raw big-endian byte representation. BigNumber::SetBinary interprets its
    // input little-endian (it reverses before calling BN_bin2bn, which reads big-endian),
    // so reversing the already-big-endian input here cancels that out.
    BigNumber BigNumberFromBigEndianBytes(std::vector<uint8> const& bytes)
    {
        std::vector<uint8> reversed(bytes.rbegin(), bytes.rend());
        BigNumber bn;
        bn.SetBinary(reversed.data(), (int)reversed.size());
        return bn;
    }

    // BigNumber::operator% follows BN_mod, whose remainder can come out negative for a
    // negative dividend (same as C's %) -- not what SRP6's "mod N" steps mean. Used for the
    // client's S = (B - k*g^x)^(a+u*x) mod N, where B - k*g^x is frequently negative.
    // By-value params: BigNumber's arithmetic operators aren't const-qualified.
    BigNumber NonNegativeMod(BigNumber value, BigNumber modulus)
    {
        return ((value % modulus) + modulus) % modulus;
    }

    // BnetSRP6Base::GetBrokenEvidenceVector: the minimal big-endian encoding of `bn`, with
    // a guard 0x00 byte prepended when the top bit of the top byte is set (so the value can
    // never be mistaken for negative/ambiguous by anything re-parsing the hash input).
    std::vector<uint8> BrokenEvidenceVector(BigNumber const& bn)
    {
        int numBytes = bn.GetNumBytes();
        std::vector<uint8> bytes = bn.AsByteArray(numBytes, false);
        if (!bytes.empty() && (bytes[0] & 0x80))
        {
            std::vector<uint8> padded;
            padded.reserve(bytes.size() + 1);
            padded.push_back(0);
            padded.insert(padded.end(), bytes.begin(), bytes.end());
            return padded;
        }
        return bytes;
    }

    // BnetSRP6Base::DoCalculateEvidence: SHA256 over the broken-evidence encoding of each
    // of the three inputs, concatenated in order.
    SHA256::Digest ComputeEvidence(BigNumber const& a, BigNumber const& b, BigNumber const& c)
    {
        SHA256::Generator gen;
        gen.UpdateData(BrokenEvidenceVector(a));
        gen.UpdateData(BrokenEvidenceVector(b));
        gen.UpdateData(BrokenEvidenceVector(c));
        return gen.GetDigest();
    }

    // BnetSRP6v1::CalculateU: SHA256(A || B), each fixed-width padded to N's byte length
    // (unlike the variable-length broken-evidence encoding above).
    BigNumber CalculateU(BigNumber const& A, BigNumber const& B)
    {
        SHA256::Generator gen;
        gen.UpdateData(A.AsByteArray(BnetSRP6::N_BYTE_SIZE, false));
        gen.UpdateData(B.AsByteArray(BnetSRP6::N_BYTE_SIZE, false));
        SHA256::Digest digest = gen.GetDigest();
        return BigNumberFromBigEndianBytes(std::vector<uint8>(digest.begin(), digest.end()));
    }

    // k = H(N || g), both fixed-width padded to N's byte length -- constant for v1.
    BigNumber const& GetK()
    {
        static BigNumber const k = []
        {
            SHA256::Generator gen;
            gen.UpdateData(GetN().AsByteArray(BnetSRP6::N_BYTE_SIZE, false));
            gen.UpdateData(Getg().AsByteArray(BnetSRP6::N_BYTE_SIZE, false));
            SHA256::Digest digest = gen.GetDigest();
            return BigNumberFromBigEndianBytes(std::vector<uint8>(digest.begin(), digest.end()));
        }();
        return k;
    }
}

BnetSRP6::BnetSRP6()
{
}

BigNumber BnetSRP6::CalculateX(std::string const& username, std::string const& password, std::vector<uint8> const& salt) const
{
    // x = H(salt || H(username || ':' || password))
    SHA256::Generator inner;
    inner.UpdateData(username);
    inner.UpdateData(std::string(":"));
    inner.UpdateData(password);
    SHA256::Digest innerDigest = inner.GetDigest();

    SHA256::Generator outer;
    outer.UpdateData(salt);
    outer.UpdateData(std::vector<uint8>(innerDigest.begin(), innerDigest.end()));
    SHA256::Digest xDigest = outer.GetDigest();

    return DigestToBigNumberLE(xDigest);
}

void BnetSRP6::CalculateVerifier(std::string const& username, std::string const& password)
{
    BigNumber salt;
    salt.SetRand(SALT_BYTE_SIZE * 8);
    m_salt = salt.AsByteArray(SALT_BYTE_SIZE, false);

    // v = g ^ x mod N
    BigNumber x = CalculateX(username, password, m_salt);
    BigNumber g = Getg();
    m_v = g.ModExp(x, GetN());
}

void BnetSRP6::Initialize(std::vector<uint8> const& salt, std::vector<uint8> const& verifier)
{
    m_salt = salt;
    m_v = BigNumberFromBigEndianBytes(verifier);
}

bool BnetSRP6::CheckCredentials(std::string const& username, std::string const& password) const
{
    BigNumber x = CalculateX(username, password, m_salt);
    BigNumber g = Getg();
    BigNumber v = g.ModExp(x, GetN());
    return v.AsByteArray(0, false) == m_v.AsByteArray(0, false);
}

std::vector<uint8> BnetSRP6::GetVerifier() const
{
    return m_v.AsByteArray(0, false);
}

std::vector<uint8> BnetSRP6::GetModulus()
{
    BigNumber n = GetN();
    return n.AsByteArray(0, false);
}

std::vector<uint8> BnetSRP6::GetGenerator()
{
    BigNumber g = Getg();
    return g.AsByteArray(0, false);
}

std::string BnetSRP6::ComputeSrpUsername(std::string const& login)
{
    SHA256::Digest digest = SHA256::ComputeFrom(login);
    std::string hex;
    hexEncodeByteArray(digest.data(), uint32(digest.size()), hex);
    return hex;
}

void BnetSRP6::CalculateHostPublicEphemeral()
{
    BigNumber N = GetN();

    m_b.SetRand(N_BYTE_SIZE * 8);
    m_b %= (N - BigNumber(1));

    BigNumber g = Getg();
    BigNumber gb = g.ModExp(m_b, N);
    BigNumber vk = m_v * GetK();
    m_B = (gb + vk) % N;
}

std::vector<uint8> BnetSRP6::GetHostPublicEphemeral() const
{
    return m_B.AsByteArray(N_BYTE_SIZE, false);
}

bool BnetSRP6::VerifyClientEvidence(std::vector<uint8> const& A, std::vector<uint8> const& clientEvidenceM1)
{
    // M1 is a number; the client drops its leading zero bytes.
    if (clientEvidenceM1.empty() || clientEvidenceM1.size() > EVIDENCE_SIZE)
        return false;
    std::vector<uint8> m1(EVIDENCE_SIZE - clientEvidenceM1.size(), 0);
    m1.insert(m1.end(), clientEvidenceM1.begin(), clientEvidenceM1.end());

    BigNumber N = GetN();
    BigNumber a = BigNumberFromBigEndianBytes(A);

    BigNumber aModN = a % N;
    if (aModN.isZero())
        return false;

    BigNumber u = CalculateU(a, m_B);
    BigNumber uModN = u % N;
    if (uModN.isZero())
        return false;

    // S = (A * v^u mod N)^b mod N
    BigNumber vu = m_v.ModExp(u, N);
    BigNumber aTimesVu = a * vu;
    BigNumber S = aTimesVu.ModExp(m_b, N);

    SHA256::Digest ourM1 = ComputeEvidence(a, m_B, S);
    if (!std::equal(ourM1.begin(), ourM1.end(), m1.begin()))
        return false;

    m_A = a;
    m_S = S;
    m_clientM1 = BigNumberFromBigEndianBytes(m1);
    return true;
}

std::vector<uint8> BnetSRP6::CalculateServerEvidence() const
{
    SHA256::Digest m2 = ComputeEvidence(m_A, m_clientM1, m_S);
    return std::vector<uint8>(m2.begin(), m2.end());
}

void BnetSRP6::CalculateClientPublicEphemeral()
{
    BigNumber N = GetN();

    m_a.SetRand(N_BYTE_SIZE * 8);
    m_a %= (N - BigNumber(1));

    BigNumber g = Getg();
    m_A = g.ModExp(m_a, N);
}

std::vector<uint8> BnetSRP6::GetClientPublicEphemeral() const
{
    return m_A.AsByteArray(N_BYTE_SIZE, false);
}

std::vector<uint8> BnetSRP6::CalculateClientEvidence(std::string const& username, std::string const& password,
    std::vector<uint8> const& salt, std::vector<uint8> const& hostPublicEphemeralB)
{
    BigNumber N = GetN();
    BigNumber B = BigNumberFromBigEndianBytes(hostPublicEphemeralB);

    BigNumber bModN = B % N;
    if (bModN.isZero())
        return {};

    BigNumber u = CalculateU(m_A, B);
    BigNumber uModN = u % N;
    if (uModN.isZero())
        return {};

    BigNumber x = CalculateX(username, password, salt);

    // S = (B - k*g^x)^(a + u*x) mod N
    BigNumber g = Getg();
    BigNumber gx = g.ModExp(x, N);
    BigNumber k = GetK();
    BigNumber kgx = k * gx;
    BigNumber base = NonNegativeMod(B - kgx, N);
    BigNumber exponent = m_a + (u * x);
    m_S = base.ModExp(exponent, N);

    SHA256::Digest m1 = ComputeEvidence(m_A, B, m_S);
    m_clientM1 = BigNumberFromBigEndianBytes(std::vector<uint8>(m1.begin(), m1.end()));
    return std::vector<uint8>(m1.begin(), m1.end());
}

bool BnetSRP6::VerifyServerEvidence(std::vector<uint8> const& serverEvidenceM2) const
{
    if (serverEvidenceM2.size() != EVIDENCE_SIZE)
        return false;

    SHA256::Digest ourM2 = ComputeEvidence(m_A, m_clientM1, m_S);
    return std::equal(ourM2.begin(), ourM2.end(), serverEvidenceM2.begin());
}


#include "elog_crypt.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <stdint.h>

#ifdef SIRIUS_HAVE_OPENSSL
#include <openssl/evp.h>
#endif

namespace elog_detail {

// ########################################################################
// SHA-256 primitive
// ########################################################################

#ifdef SIRIUS_HAVE_OPENSSL

struct sha256::impl {
    EVP_MD_CTX* ctx;
};

sha256::sha256()
    : p(new impl)
{
    p->ctx = EVP_MD_CTX_new();
    if( !p->ctx || EVP_DigestInit_ex(p->ctx, EVP_sha256(), 0) != 1 ) {
        EVP_MD_CTX_free(p->ctx);
        delete p;
        throw std::runtime_error("EVP sha256 init failed");
    }
}

sha256::~sha256()
{
    EVP_MD_CTX_free(p->ctx);
    delete p;
}

void sha256::update(const void* data, std::size_t len)
{
    EVP_DigestUpdate(p->ctx, data, len);
}

void sha256::final(unsigned char* out)
{
    unsigned int n = DIGEST_LENGTH;
    EVP_DigestFinal_ex(p->ctx, out, &n);
}

#else // built-in SHA-256 (FIPS 180-4)

namespace {

const uint32_t K256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline uint32_t rotr(uint32_t x, unsigned n) { return (x >> n) | (x << (32 - n)); }

} // anonymous namespace

struct sha256::impl {
    uint32_t h[8];
    unsigned char buf[64];
    std::size_t buflen;
    uint64_t total;

    void block(const unsigned char* b)
    {
        uint32_t w[64];
        for(int i=0; i<16; ++i)
            w[i] = (uint32_t(b[4*i]) << 24) | (uint32_t(b[4*i+1]) << 16)
                 | (uint32_t(b[4*i+2]) << 8) | uint32_t(b[4*i+3]);
        for(int i=16; i<64; ++i) {
            const uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
            const uint32_t s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        uint32_t a=h[0], bb=h[1], c=h[2], d=h[3], e=h[4], f=h[5], g=h[6], hh=h[7];
        for(int i=0; i<64; ++i) {
            const uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            const uint32_t ch = (e & f) ^ (~e & g);
            const uint32_t t1 = hh + S1 + ch + K256[i] + w[i];
            const uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            const uint32_t mj = (a & bb) ^ (a & c) ^ (bb & c);
            const uint32_t t2 = S0 + mj;
            hh = g; g = f; f = e; e = d + t1; d = c; c = bb; bb = a; a = t1 + t2;
        }
        h[0]+=a; h[1]+=bb; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
    }
};

sha256::sha256()
    : p(new impl)
{
    static const uint32_t H0[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    std::memcpy(p->h, H0, sizeof(H0));
    p->buflen = 0;
    p->total = 0;
}

sha256::~sha256()
{
    delete p;
}

void sha256::update(const void* data, std::size_t len)
{
    const unsigned char* d = static_cast<const unsigned char*>(data);
    p->total += len;
    while( len > 0 ) {
        const std::size_t n = std::min<std::size_t>(64 - p->buflen, len);
        std::memcpy(p->buf + p->buflen, d, n);
        p->buflen += n;
        d += n;
        len -= n;
        if( p->buflen == 64 ) {
            p->block(p->buf);
            p->buflen = 0;
        }
    }
}

void sha256::final(unsigned char* out)
{
    const uint64_t bits = p->total * 8;
    const unsigned char pad = 0x80;
    update(&pad, 1);
    const unsigned char zero = 0;
    while( p->buflen != 56 )
        update(&zero, 1);
    unsigned char lenbuf[8];
    for(int i=0; i<8; ++i)
        lenbuf[i] = static_cast<unsigned char>(bits >> (56 - 8*i));
    update(lenbuf, 8);
    for(int i=0; i<8; ++i) {
        out[4*i  ] = static_cast<unsigned char>(p->h[i] >> 24);
        out[4*i+1] = static_cast<unsigned char>(p->h[i] >> 16);
        out[4*i+2] = static_cast<unsigned char>(p->h[i] >> 8);
        out[4*i+3] = static_cast<unsigned char>(p->h[i]);
    }
}

#endif // SIRIUS_HAVE_OPENSSL

// ########################################################################
// SHA-crypt ($5$), see https://www.akkadia.org/drepper/SHA-crypt.txt
// ########################################################################

namespace {

const char B64[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void b64_from_24bit(std::string& out, unsigned char b2, unsigned char b1, unsigned char b0, int n)
{
    unsigned int w = (unsigned(b2) << 16) | (unsigned(b1) << 8) | unsigned(b0);
    while( n-- > 0 ) {
        out += B64[w & 0x3f];
        w >>= 6;
    }
}

// Add 'seq' (DIGEST_LENGTH bytes) repeatedly until 'len' bytes are added.
void add_repeated(sha256& ctx, const unsigned char* seq, std::size_t len)
{
    for( ; len >= sha256::DIGEST_LENGTH; len -= sha256::DIGEST_LENGTH )
        ctx.update(seq, sha256::DIGEST_LENGTH);
    ctx.update(seq, len);
}

} // anonymous namespace

std::string sha256_crypt(const std::string& key, const std::string& salt_spec)
{
    static const std::string prefix = "$5$";
    static const std::string rounds_prefix = "rounds=";
    const unsigned long ROUNDS_DEFAULT = 5000, ROUNDS_MIN = 1000, ROUNDS_MAX = 999999999;

    std::string s = salt_spec;
    if( s.compare(0, prefix.size(), prefix) == 0 )
        s.erase(0, prefix.size());

    unsigned long rounds = ROUNDS_DEFAULT;
    bool rounds_custom = false;
    if( s.compare(0, rounds_prefix.size(), rounds_prefix) == 0 ) {
        const char* num = s.c_str() + rounds_prefix.size();
        char* endp = 0;
        const unsigned long r = std::strtoul(num, &endp, 10);
        if( *endp == '$' ) {
            s.erase(0, (endp + 1) - s.c_str());
            rounds = std::max(ROUNDS_MIN, std::min(r, ROUNDS_MAX));
            rounds_custom = true;
        }
    }

    // salt ends at the first '$' and is at most 16 characters
    std::string salt = s.substr(0, s.find('$'));
    if( salt.size() > 16 )
        salt.resize(16);

    const std::size_t klen = key.size(), slen = salt.size();
    unsigned char a[sha256::DIGEST_LENGTH], b[sha256::DIGEST_LENGTH];

    // digest B = H(key salt key)
    {
        sha256 ctx;
        ctx.update(key);
        ctx.update(salt);
        ctx.update(key);
        ctx.final(b);
    }

    // digest A
    {
        sha256 ctx;
        ctx.update(key);
        ctx.update(salt);
        add_repeated(ctx, b, klen);
        for(std::size_t cnt = klen; cnt > 0; cnt >>= 1) {
            if( cnt & 1 )
                ctx.update(b, sizeof(b));
            else
                ctx.update(key);
        }
        ctx.final(a);
    }

    // P sequence
    std::vector<unsigned char> pseq(klen);
    {
        unsigned char dp[sha256::DIGEST_LENGTH];
        sha256 ctx;
        for(std::size_t i=0; i<klen; ++i)
            ctx.update(key);
        ctx.final(dp);
        for(std::size_t i=0; i<klen; ++i)
            pseq[i] = dp[i % sizeof(dp)];
    }

    // S sequence
    std::vector<unsigned char> sseq(slen);
    {
        unsigned char ds[sha256::DIGEST_LENGTH];
        sha256 ctx;
        for(unsigned i=0; i < 16u + a[0]; ++i)
            ctx.update(salt);
        ctx.final(ds);
        for(std::size_t i=0; i<slen; ++i)
            sseq[i] = ds[i % sizeof(ds)];
    }

    const unsigned char* P = pseq.empty() ? a : &pseq[0];
    const unsigned char* S = sseq.empty() ? a : &sseq[0];

    // the slow part
    for(unsigned long r=0; r<rounds; ++r) {
        sha256 ctx;
        if( r & 1 )
            ctx.update(P, klen);
        else
            ctx.update(a, sizeof(a));
        if( r % 3 != 0 )
            ctx.update(S, slen);
        if( r % 7 != 0 )
            ctx.update(P, klen);
        if( r & 1 )
            ctx.update(a, sizeof(a));
        else
            ctx.update(P, klen);
        ctx.final(a);
    }

    std::string out = prefix;
    if( rounds_custom ) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "rounds=%lu$", rounds);
        out += buf;
    }
    out += salt;
    out += '$';

    b64_from_24bit(out, a[ 0], a[10], a[20], 4);
    b64_from_24bit(out, a[21], a[ 1], a[11], 4);
    b64_from_24bit(out, a[12], a[22], a[ 2], 4);
    b64_from_24bit(out, a[ 3], a[13], a[23], 4);
    b64_from_24bit(out, a[24], a[ 4], a[14], 4);
    b64_from_24bit(out, a[15], a[25], a[ 5], 4);
    b64_from_24bit(out, a[ 6], a[16], a[26], 4);
    b64_from_24bit(out, a[27], a[ 7], a[17], 4);
    b64_from_24bit(out, a[18], a[28], a[ 8], 4);
    b64_from_24bit(out, a[ 9], a[19], a[29], 4);
    b64_from_24bit(out, 0,     a[31], a[30], 3);

    return out;
}

// ########################################################################

std::string elog_password_hash(const std::string& password)
{
    // elog.cxx: strlcpy(d, sha256_crypt(s, "$5$") + 4, size);
    const std::string c = sha256_crypt(password, "$5$");
    return c.size() > 4 ? c.substr(4) : std::string();
}

} // namespace elog_detail

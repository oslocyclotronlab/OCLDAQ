// -*- c++ -*-
//
// Internal helpers for the elog client: SHA-256 and the SHA-crypt ($5$)
// password scheme used by elogd. Not part of the public sirius API.

#ifndef ELOG_CRYPT_H
#define ELOG_CRYPT_H 1

#include <cstddef>
#include <string>

namespace elog_detail {

//! Minimal incremental SHA-256. Uses OpenSSL's EVP interface when sirius is
//! built with OpenSSL (SIRIUS_HAVE_OPENSSL), otherwise a built-in
//! implementation of FIPS 180-4.
class sha256 {
public:
    enum { DIGEST_LENGTH = 32 };

    sha256();
    ~sha256();

    void update(const void* data, std::size_t len);
    void update(const std::string& s) { update(s.data(), s.size()); }

    //! Finish and write DIGEST_LENGTH bytes to out. The object must not be
    //! updated again afterwards.
    void final(unsigned char* out);

private:
    sha256(const sha256&);
    sha256& operator=(const sha256&);

    struct impl;
    impl* p;
};

//! Ulrich Drepper's SHA-256 based crypt ("$5$[rounds=N$]salt$hash").
//! salt_spec is e.g. "$5$" or "$5$rounds=10000$saltstring".
std::string sha256_crypt(const std::string& key, const std::string& salt_spec);

//! The password string elog sends in the "upwd" form field:
//! sha256_crypt(pw, "$5$") with the leading "$5$$" removed.
std::string elog_password_hash(const std::string& password);

} // namespace elog_detail

#endif /* ELOG_CRYPT_H */

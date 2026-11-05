// -*- c++ -*-
//
// Native client for submitting entries to an elogd server
// (https://elog.psi.ch/elog/). Speaks the same multipart/form-data
// protocol as the `elog` command line tool, so no external binary is
// needed.
//
// TLS is detected automatically: if the server speaks TLS it is always
// used, otherwise the entry is sent as plain HTTP and a warning is
// reported. TLS requires sirius to be built with OpenSSL.

#ifndef ELOG_CLIENT_H
#define ELOG_CLIENT_H 1

#include <string>
#include <utility>
#include <vector>

class command_list;

typedef std::vector<std::pair<std::string, std::string> > elog_attributes;

struct elog_config {
    std::string host;          //!< elogd host name or address
    int         port;          //!< 0: try 443 with TLS, fall back to 80 without
    std::string subdir;        //!< optional URL prefix, e.g. "elog"
    std::string logbook;       //!< logbook ("experiment") name
    std::string user;          //!< user name, empty for none
    std::string password;      //!< plain text password, hashed before sending
    std::string encoding;      //!< "plain" (default), "ELCode" or "HTML"
    bool        ssl_verify;    //!< true: certificate verification failure is fatal
    int         timeout_s;     //!< connect/send/receive timeout in seconds
    elog_attributes attributes;//!< default attributes (Type, Category, Author, ...)

    elog_config();

    //! True if enough information is present to submit entries.
    bool enabled() const { return !host.empty() && !logbook.empty(); }

    //! Human readable description, e.g. "elog.example.org/MyLogbook as daq".
    std::string describe() const;
};

struct elog_result {
    bool ok;                           //!< true if elogd accepted the entry
    int  message_id;                   //!< id of the new entry, -1 if unknown
    std::string error;                 //!< error description if !ok
    std::vector<std::string> warnings; //!< e.g. unencrypted connection

    elog_result() : ok(false), message_id(-1) { }
};

//! True if sirius was built with OpenSSL, i.e. TLS can be used.
bool elog_ssl_available();

//! Read the elog configuration from a command list (acq_master_commands.txt).
//!
//! Recognised keys: elog_host, elog_port, elog_subdir, elog_logbook,
//! elog_user, elog_password (falls back to $ELOG_PASSWORD), elog_encoding,
//! elog_ssl_verify, elog_timeout and elog_attr_<Name> for each attribute.
//!
//! If neither elog_host nor elog_logbook is given, a legacy line of the form
//! "elog = elog -h host -p port -l logbook -a Name=Value ..." is parsed.
elog_config elog_config_from_commands(const command_list& commands);

//! Submit an entry and wait for the answer. 'attrs' are added to (and
//! override by name) the default attributes in 'cfg'.
elog_result elog_submit(const elog_config& cfg, const elog_attributes& attrs,
                        const std::string& text);

//! Submit an entry from a detached background thread. Warnings and errors
//! are printed to std::cerr prefixed with "elog: ".
void elog_submit_async(const elog_config& cfg, const elog_attributes& attrs,
                       const std::string& text);

#endif /* ELOG_CLIENT_H */

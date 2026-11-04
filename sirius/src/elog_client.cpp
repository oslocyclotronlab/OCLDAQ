
#include "elog_client.h"

#include "elog_crypt.h"
#include "run_command.h"
#include "utilities.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <map>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef SIRIUS_HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#endif

// ########################################################################
// helpers
// ########################################################################

namespace {

const std::size_t MAX_RESPONSE = 1024*1024;

std::string to_lower(std::string s)
{
    for(std::size_t i=0; i<s.size(); ++i)
        s[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
    return s;
}

bool parse_bool(const std::string& v, bool def)
{
    const std::string l = to_lower(strip(v));
    if( l == "yes" || l == "true" || l == "on" || l == "1" )
        return true;
    if( l == "no" || l == "false" || l == "off" || l == "0" )
        return false;
    return def;
}

bool parse_int(const std::string& v, int& out)
{
    const std::string s = strip(v);
    if( s.empty() )
        return false;
    char* endp = 0;
    const long l = std::strtol(s.c_str(), &endp, 10);
    if( *endp != 0 || l < 0 || l > 65535*16 )
        return false;
    out = static_cast<int>(l);
    return true;
}

std::string int_to_string(long v)
{
    std::ostringstream o;
    o << v;
    return o.str();
}

std::string basename_of(const std::string& path)
{
    const std::string::size_type s = path.rfind('/');
    return (s == std::string::npos) ? path : path.substr(s+1);
}

// elog.cxx url_encode(): escape %&=#?+ and bytes > 127, space -> '+'
std::string url_encode(const std::string& in)
{
    std::string out;
    for(std::size_t i=0; i<in.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(in[i]);
        if( std::strchr("%&=#?+", c) != 0 || c > 127 ) {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%%%02X", c);
            out += buf;
        } else if( c == ' ' ) {
            out += '+';
        } else {
            out += static_cast<char>(c);
        }
    }
    return out;
}

// elog.cxx stou(): special characters in attribute names -> '_'
std::string stou(std::string s)
{
    for(std::size_t i=0; i<s.size(); ++i) {
        const char c = s[i];
        if( c == ' ' || c == '.' || c == '/' || c == '\\' || c == '-' || c == '(' || c == ')' )
            s[i] = '_';
    }
    return s;
}

// convert bare LF to CRLF, like elog.cxx add_crlf()
std::string to_crlf(const std::string& in)
{
    std::string out;
    out.reserve(in.size() + in.size()/16);
    for(std::size_t i=0; i<in.size(); ++i) {
        if( in[i] == '\n' && (i == 0 || in[i-1] != '\r') )
            out += '\r';
        out += in[i];
    }
    return out;
}

std::string strip_html(const std::string& in)
{
    std::string out;
    bool in_tag = false;
    for(std::size_t i=0; i<in.size(); ++i) {
        if( in[i] == '<' )
            in_tag = true;
        else if( in[i] == '>' )
            in_tag = false;
        else if( !in_tag )
            out += in[i];
    }
    return strip(out);
}

std::string first_line(const std::string& s)
{
    return s.substr(0, s.find_first_of("\r\n"));
}

#ifdef SIRIUS_HAVE_OPENSSL
bool is_ip_literal(const std::string& host)
{
    unsigned char buf[sizeof(struct in6_addr)];
    return inet_pton(AF_INET, host.c_str(), buf) == 1
        || inet_pton(AF_INET6, host.c_str(), buf) == 1;
}
#endif

// ########################################################################
// TCP connection with timeout
// ########################################################################

enum { CONNECT_FAILED = -1, CONNECT_DNS_ERROR = -2 };

void set_io_timeouts(int fd, int timeout_s)
{
    struct timeval tv;
    tv.tv_sec = timeout_s;
    tv.tv_usec = 0;
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#ifdef SO_NOSIGPIPE
    int one = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
}

int tcp_connect(const std::string& host, int port, int timeout_s, std::string& err)
{
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = 0;
    const std::string port_str = int_to_string(port);
    const int g = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if( g != 0 ) {
        err = "cannot resolve host '" + host + "': " + gai_strerror(g);
        return CONNECT_DNS_ERROR;
    }

    const std::string where = host + ":" + port_str;
    err = "cannot connect to " + where;
    for(struct addrinfo* ai = res; ai; ai = ai->ai_next) {
        const int fd = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if( fd < 0 )
            continue;

        const int flags = ::fcntl(fd, F_GETFL, 0);
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        int rc = ::connect(fd, ai->ai_addr, ai->ai_addrlen);
        if( rc < 0 && errno == EINPROGRESS ) {
            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLOUT;
            pfd.revents = 0;
            int pr;
            do {
                pr = ::poll(&pfd, 1, timeout_s*1000);
            } while( pr < 0 && errno == EINTR );
            if( pr == 0 ) {
                err = "timeout connecting to " + where;
                ::close(fd);
                continue;
            } else if( pr < 0 ) {
                err = "cannot connect to " + where + ": " + std::strerror(errno);
                ::close(fd);
                continue;
            }
            int soerr = 0;
            socklen_t len = sizeof(soerr);
            ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &soerr, &len);
            if( soerr != 0 ) {
                err = "cannot connect to " + where + ": " + std::strerror(soerr);
                ::close(fd);
                continue;
            }
            rc = 0;
        } else if( rc < 0 ) {
            err = "cannot connect to " + where + ": " + std::strerror(errno);
            ::close(fd);
            continue;
        }

        ::fcntl(fd, F_SETFL, flags);
        set_io_timeouts(fd, timeout_s);
        ::freeaddrinfo(res);
        return fd;
    }
    ::freeaddrinfo(res);
    return CONNECT_FAILED;
}

// ########################################################################
// connection: plain socket or TLS on top of it
// ########################################################################

class connection {
public:
    connection() : fd(-1)
#ifdef SIRIUS_HAVE_OPENSSL
        , ssl(0)
#endif
        { }
    ~connection() { close(); }

    void close()
    {
#ifdef SIRIUS_HAVE_OPENSSL
        if( ssl ) {
            if( SSL_is_init_finished(ssl) )
                SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = 0;
        }
#endif
        if( fd >= 0 ) {
            ::close(fd);
            fd = -1;
        }
    }

    bool is_tls() const
    {
#ifdef SIRIUS_HAVE_OPENSSL
        return ssl != 0;
#else
        return false;
#endif
    }

    bool write_all(const std::string& data, std::string& err)
    {
        std::size_t done = 0;
        while( done < data.size() ) {
            const char* p = data.data() + done;
            const std::size_t n = data.size() - done;
            long w;
#ifdef SIRIUS_HAVE_OPENSSL
            if( ssl ) {
                w = SSL_write(ssl, p, static_cast<int>(std::min<std::size_t>(n, 1<<30)));
                if( w <= 0 ) {
                    err = "TLS write failed";
                    return false;
                }
            } else
#endif
            {
#ifdef MSG_NOSIGNAL
                w = ::send(fd, p, n, MSG_NOSIGNAL);
#else
                w = ::send(fd, p, n, 0);
#endif
                if( w < 0 ) {
                    if( errno == EINTR )
                        continue;
                    err = std::string("send failed: ") + std::strerror(errno);
                    return false;
                }
            }
            done += static_cast<std::size_t>(w);
        }
        return true;
    }

    //! Read until the server closes the connection (HTTP/1.0).
    bool read_all(std::string& out, std::string& err)
    {
        char buf[8192];
        while( out.size() < MAX_RESPONSE ) {
            long r;
#ifdef SIRIUS_HAVE_OPENSSL
            if( ssl ) {
                r = SSL_read(ssl, buf, sizeof(buf));
                if( r <= 0 ) {
                    const int se = SSL_get_error(ssl, static_cast<int>(r));
                    ERR_clear_error();
                    if( se == SSL_ERROR_ZERO_RETURN || !out.empty() )
                        return true; // closed (possibly without close_notify)
                    err = "no response from server (TLS read failed)";
                    return false;
                }
            } else
#endif
            {
                r = ::recv(fd, buf, sizeof(buf), 0);
                if( r == 0 )
                    return true;
                if( r < 0 ) {
                    if( errno == EINTR )
                        continue;
                    if( !out.empty() )
                        return true;
                    err = (errno == EAGAIN || errno == EWOULDBLOCK)
                        ? std::string("timeout waiting for server response")
                        : std::string("recv failed: ") + std::strerror(errno);
                    return false;
                }
            }
            out.append(buf, static_cast<std::size_t>(r));
        }
        return true;
    }

    int fd;
#ifdef SIRIUS_HAVE_OPENSSL
    SSL* ssl;
#endif
};

// ########################################################################
// TLS
// ########################################################################

enum handshake_result { HS_OK, HS_NO_TLS, HS_ERROR };

#ifdef SIRIUS_HAVE_OPENSSL

SSL_CTX* make_tls_context()
{
    OPENSSL_init_ssl(0, 0);
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if( !ctx )
        return 0;
    SSL_CTX_set_default_verify_paths(ctx);
    // verification is checked after the handshake, so that a bad
    // certificate can be reported as a warning only (like elog.cxx)
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
    return ctx;
}

SSL_CTX* tls_context()
{
    static SSL_CTX* ctx = make_tls_context(); // thread safe init in C++11
    return ctx;
}

bool reason_means_no_tls(int reason)
{
    switch( reason ) {
#ifdef SSL_R_WRONG_VERSION_NUMBER
    case SSL_R_WRONG_VERSION_NUMBER:
#endif
#ifdef SSL_R_UNKNOWN_PROTOCOL
    case SSL_R_UNKNOWN_PROTOCOL:
#endif
#ifdef SSL_R_PACKET_LENGTH_TOO_LONG
    case SSL_R_PACKET_LENGTH_TOO_LONG:
#endif
#ifdef SSL_R_HTTP_REQUEST
    case SSL_R_HTTP_REQUEST:
#endif
#ifdef SSL_R_HTTPS_PROXY_REQUEST
    case SSL_R_HTTPS_PROXY_REQUEST:
#endif
#ifdef SSL_R_UNEXPECTED_EOF_WHILE_READING
    case SSL_R_UNEXPECTED_EOF_WHILE_READING:
#endif
        return true;
    default:
        return false;
    }
}

handshake_result tls_handshake(connection& c, const elog_config& cfg, std::string& err,
                               std::vector<std::string>& warnings)
{
    SSL_CTX* ctx = tls_context();
    if( !ctx ) {
        err = "cannot create TLS context";
        return HS_ERROR;
    }

    ERR_clear_error();
    c.ssl = SSL_new(ctx);
    if( !c.ssl ) {
        err = "cannot create TLS session";
        return HS_ERROR;
    }
    SSL_set_fd(c.ssl, c.fd);
    if( is_ip_literal(cfg.host) ) {
        X509_VERIFY_PARAM_set1_ip_asc(SSL_get0_param(c.ssl), cfg.host.c_str());
    } else {
        SSL_set_tlsext_host_name(c.ssl, cfg.host.c_str());
        SSL_set1_host(c.ssl, cfg.host.c_str());
    }

    errno = 0;
    const int r = SSL_connect(c.ssl);
    if( r == 1 ) {
        const long vr = SSL_get_verify_result(c.ssl);
        if( vr != X509_V_OK ) {
            const std::string msg = "TLS certificate of " + cfg.host + " not trusted: "
                + X509_verify_cert_error_string(vr);
            if( cfg.ssl_verify ) {
                err = msg;
                return HS_ERROR;
            }
            warnings.push_back("WARNING: " + msg
                               + " (continuing; set elog_ssl_verify = yes to refuse)");
        }
        return HS_OK;
    }

    const int saved_errno = errno;
    const int se = SSL_get_error(c.ssl, r);
    bool no_tls = false;
    std::string detail;
    unsigned long e;
    while( (e = ERR_get_error()) != 0 ) {
        if( detail.empty() ) {
            char buf[256];
            ERR_error_string_n(e, buf, sizeof(buf));
            detail = buf;
        }
        if( ERR_GET_LIB(e) == ERR_LIB_SSL && reason_means_no_tls(ERR_GET_REASON(e)) )
            no_tls = true;
    }

    if( se == SSL_ERROR_WANT_READ || se == SSL_ERROR_WANT_WRITE ) {
        // the socket timeout (SO_RCVTIMEO/SO_SNDTIMEO) expired
        err = "timeout during TLS handshake with " + cfg.host;
        return HS_ERROR;
    }
    if( se == SSL_ERROR_SYSCALL && detail.empty() ) {
        if( saved_errno == EAGAIN || saved_errno == EWOULDBLOCK ) {
            err = "timeout during TLS handshake with " + cfg.host;
            return HS_ERROR;
        }
        if( saved_errno == 0 || saved_errno == ECONNRESET ) {
            // peer closed the connection when it received the ClientHello
            no_tls = true;
            detail = "connection closed by server";
        } else {
            detail = std::strerror(saved_errno);
        }
    } else if( se == SSL_ERROR_ZERO_RETURN ) {
        no_tls = true;
        detail = "connection closed by server";
    }

    err = "TLS handshake with " + cfg.host + " failed" + (detail.empty() ? "" : ": " + detail);
    return no_tls ? HS_NO_TLS : HS_ERROR;
}

#endif // SIRIUS_HAVE_OPENSSL

// ########################################################################
// remember per server whether TLS works
// ########################################################################

struct server_mode {
    bool tls;
    int port;
    std::string why_plain;
};

std::mutex mode_mutex;
std::map<std::string, server_mode> mode_cache;

std::string mode_key(const elog_config& cfg)
{
    return cfg.host + ":" + int_to_string(cfg.port);
}

bool mode_get(const std::string& key, server_mode& m)
{
    std::lock_guard<std::mutex> lock(mode_mutex);
    std::map<std::string, server_mode>::const_iterator it = mode_cache.find(key);
    if( it == mode_cache.end() )
        return false;
    m = it->second;
    return true;
}

void mode_put(const std::string& key, const server_mode& m)
{
    std::lock_guard<std::mutex> lock(mode_mutex);
    mode_cache[key] = m;
}

void mode_erase(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mode_mutex);
    mode_cache.erase(key);
}

void warn_plain(const elog_config& cfg, const server_mode& m, elog_result& res)
{
    std::string w = "WARNING: " + m.why_plain + "; sending entry UNENCRYPTED to http://"
        + cfg.host + ":" + int_to_string(m.port);
    if( !cfg.user.empty() )
        w += " (including user name and password hash)";
    res.warnings.push_back(w);
}

//! Connect, preferring TLS. On success 'port' is the port actually used.
bool open_connection(const elog_config& cfg, connection& c, int& port, elog_result& res)
{
    std::string err;
    const std::string key = mode_key(cfg);

    server_mode m;
    if( mode_get(key, m) ) {
        const int fd = tcp_connect(cfg.host, m.port, cfg.timeout_s, err);
        if( fd >= 0 ) {
            c.fd = fd;
            port = m.port;
            if( !m.tls ) {
                warn_plain(cfg, m, res);
                return true;
            }
#ifdef SIRIUS_HAVE_OPENSSL
            if( tls_handshake(c, cfg, err, res.warnings) == HS_OK )
                return true;
#endif
        }
        // something changed; probe again next time
        mode_erase(key);
        res.error = err;
        return false;
    }

    m.tls = false;
#ifdef SIRIUS_HAVE_OPENSSL
    const int tls_port = cfg.port > 0 ? cfg.port : 443;
    const int fd = tcp_connect(cfg.host, tls_port, cfg.timeout_s, err);
    if( fd == CONNECT_DNS_ERROR || (fd < 0 && cfg.port > 0) ) {
        res.error = err;
        return false;
    }
    if( fd >= 0 ) {
        c.fd = fd;
        const handshake_result hs = tls_handshake(c, cfg, err, res.warnings);
        if( hs == HS_OK ) {
            m.tls = true;
            m.port = tls_port;
            mode_put(key, m);
            port = tls_port;
            return true;
        } else if( hs == HS_ERROR ) {
            res.error = err;
            return false;
        }
        c.close();
        m.why_plain = cfg.host + ":" + int_to_string(tls_port) + " does not support SSL ("
            + err + ")";
    } else {
        m.why_plain = "SSL port " + int_to_string(tls_port) + " of " + cfg.host
            + " is not reachable (" + err + ")";
    }
#else
    m.why_plain = "sirius was built without OpenSSL, SSL is not available";
#endif

    m.port = cfg.port > 0 ? cfg.port : 80;
    const int pfd = tcp_connect(cfg.host, m.port, cfg.timeout_s, err);
    if( pfd < 0 ) {
        res.error = err;
        return false;
    }
    c.fd = pfd;
    port = m.port;
    mode_put(key, m);
    warn_plain(cfg, m, res);
    return true;
}

// ########################################################################
// request building and response parsing (follows elog.cxx submit_elog)
// ########################################################################

std::string make_boundary()
{
    static std::mutex rng_mutex;
    static std::mt19937 rng(std::random_device{}());
    std::lock_guard<std::mutex> lock(rng_mutex);
    std::uniform_int_distribution<unsigned> dist(0, 0xFFFF);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "---------------------------%04X%04X%04X",
                  dist(rng), dist(rng), dist(rng));
    return buf;
}

void add_part(std::string& body, const std::string& boundary,
              const std::string& name, const std::string& value)
{
    body += boundary;
    body += "\r\nContent-Disposition: form-data; name=\"";
    body += name;
    body += "\"\r\n\r\n";
    body += value;
    body += "\r\n";
}

elog_attributes merge_attributes(const elog_attributes& defaults, const elog_attributes& extra)
{
    elog_attributes merged = defaults;
    for(std::size_t i=0; i<extra.size(); ++i) {
        const std::string lname = to_lower(extra[i].first);
        bool found = false;
        for(std::size_t j=0; j<merged.size(); ++j) {
            if( to_lower(merged[j].first) == lname ) {
                merged[j].second = extra[i].second;
                found = true;
            }
        }
        if( !found )
            merged.push_back(extra[i]);
    }
    return merged;
}

std::string build_body(const elog_config& cfg, const elog_attributes& attrs,
                       const std::string& text, const std::string& boundary)
{
    std::string body;
    add_part(body, boundary, "cmd", "Submit");
    if( !cfg.user.empty() ) {
        add_part(body, boundary, "unm", cfg.user);
        if( !cfg.password.empty() )
            add_part(body, boundary, "upwd", elog_detail::elog_password_hash(cfg.password));
    }
    add_part(body, boundary, "exp", cfg.logbook);
    add_part(body, boundary, "encoding", cfg.encoding.empty() ? std::string("plain") : cfg.encoding);
    for(std::size_t i=0; i<attrs.size(); ++i) {
        if( !attrs[i].first.empty() )
            add_part(body, boundary, stou(attrs[i].first), attrs[i].second);
    }
    add_part(body, boundary, "Text", to_crlf(text));
    body += boundary;
    body += "\r\n";
    return body;
}

std::string build_header(const elog_config& cfg, int port, bool tls,
                         const std::string& boundary, std::size_t content_length)
{
    std::string h = "POST /";
    if( !cfg.subdir.empty() )
        h += cfg.subdir + "/";
    h += url_encode(cfg.logbook) + "/";
    h += " HTTP/1.0\r\n";
    h += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
    h += "Host: " + cfg.host;
    if( port != (tls ? 443 : 80) )
        h += ":" + int_to_string(port);
    h += "\r\n";
    h += "User-Agent: ELOG\r\n";
    h += "Content-Length: " + int_to_string(static_cast<long>(content_length)) + "\r\n";
    h += "\r\n";
    return h;
}

void parse_response(const std::string& response, elog_result& res)
{
    const std::string::size_type loc = response.find("Location:");
    if( response.find("302 Found") != std::string::npos ) {
        if( loc != std::string::npos ) {
            std::string l = response.substr(loc + 9);
            l = l.substr(0, l.find_first_of("\r\n"));
            if( response.find("has moved") != std::string::npos ) {
                res.error = "elogd server has moved to another location";
            } else if( l.find("fail") != std::string::npos ) {
                res.error = "Invalid user name or password";
            } else {
                l = strip(l.substr(0, l.find('?')));
                while( !l.empty() && l[l.size()-1] == '/' )
                    l.erase(l.size()-1);
                const std::string::size_type s = l.rfind('/');
                const std::string id = (s == std::string::npos) ? l : l.substr(s+1);
                char* endp = 0;
                const long n = std::strtol(id.c_str(), &endp, 10);
                res.message_id = (!id.empty() && *endp == 0) ? static_cast<int>(n) : -1;
                res.ok = true;
            }
        } else {
            res.ok = true;
        }
    } else if( response.find("Logbook Selection") != std::string::npos ) {
        res.error = "No logbook specified (or logbook does not exist)";
    } else if( response.find("enter password") != std::string::npos ) {
        res.error = "Missing or invalid password";
    } else if( response.find("Error: Attribute") != std::string::npos ) {
        const std::string::size_type p = response.find("Error: Attribute");
        std::string line = response.substr(p);
        line = line.substr(0, line.find_first_of("\r\n"));
        // the line typically ends in HTML markup; keep up to the end of the message
        res.error = strip_html(line);
        if( res.error.empty() )
            res.error = "Missing or non-existing attribute";
    } else if( response.find("form name=form1") != std::string::npos ) {
        res.error = "Missing or invalid user name/password";
    } else {
        const std::string status = first_line(response);
        res.error = "Error transmitting message"
            + (status.empty() ? std::string(" (empty response)") : " (server replied '" + status + "')");
    }
}

// ########################################################################

#if !defined(SO_NOSIGPIPE)
//! Block SIGPIPE in this thread while OpenSSL writes to the socket
//! (there is no MSG_NOSIGNAL for SSL_write), and swallow it afterwards.
class sigpipe_guard {
public:
    sigpipe_guard()
    {
        sigemptyset(&set);
        sigaddset(&set, SIGPIPE);
        sigset_t pending;
        sigpending(&pending);
        was_pending = sigismember(&pending, SIGPIPE) == 1;
        blocked = pthread_sigmask(SIG_BLOCK, &set, &old) == 0;
    }
    ~sigpipe_guard()
    {
        if( !blocked )
            return;
        if( !was_pending ) {
            struct timespec zero = { 0, 0 };
            while( sigtimedwait(&set, 0, &zero) > 0 )
                ;
        }
        pthread_sigmask(SIG_SETMASK, &old, 0);
    }
private:
    sigset_t set, old;
    bool was_pending, blocked;
};
#endif

} // anonymous namespace

// ########################################################################
// public interface
// ########################################################################

elog_config::elog_config()
    : port(0)
    , encoding("plain")
    , ssl_verify(false)
    , timeout_s(10)
{
}

// ########################################################################

std::string elog_config::describe() const
{
    std::string d = host;
    if( port > 0 )
        d += ":" + int_to_string(port);
    d += "/";
    if( !subdir.empty() )
        d += subdir + "/";
    d += logbook;
    if( !user.empty() )
        d += " as " + user;
    return d;
}

// ########################################################################

bool elog_ssl_available()
{
#ifdef SIRIUS_HAVE_OPENSSL
    return true;
#else
    return false;
#endif
}

// ########################################################################

static void parse_legacy_elog_command(const std::string& cmdline, elog_config& cfg)
{
    if( strip(cmdline).empty() )
        return;
    const std::vector<std::string> w = word_expand(cmdline.c_str());
    if( w.empty() || basename_of(w[0]) != "elog" )
        return; // e.g. "echo" => disabled

    for(std::size_t i=1; i<w.size(); ++i) {
        const std::string& a = w[i];
        const bool has1 = i+1 < w.size();
        if( a == "-h" && has1 ) {
            cfg.host = w[++i];
        } else if( a == "-p" && has1 ) {
            parse_int(w[++i], cfg.port);
        } else if( a == "-l" && has1 ) {
            cfg.logbook = w[++i];
        } else if( a == "-d" && has1 ) {
            cfg.subdir = w[++i];
        } else if( a == "-u" && i+2 < w.size() ) {
            cfg.user = w[++i];
            cfg.password = w[++i];
        } else if( a == "-n" && has1 ) {
            const std::string e = w[++i];
            cfg.encoding = (e == "0") ? "ELCode" : (e == "2") ? "HTML" : "plain";
        } else if( a == "-a" && has1 ) {
            const std::string& av = w[++i];
            const std::string::size_type eq = av.find('=');
            if( eq != std::string::npos )
                cfg.attributes.push_back(std::make_pair(av.substr(0, eq), av.substr(eq+1)));
        } else if( (a == "-f" || a == "-r" || a == "-e" || a == "-w" || a == "-m") && has1 ) {
            ++i; // options with an argument we do not use
        }
        // -s (SSL; now auto-detected), -v, -x, -q and the message text are ignored
    }
}

elog_config elog_config_from_commands(const command_list& commands)
{
    elog_config cfg;

    cfg.host = commands.get("elog_host");
    cfg.logbook = commands.get("elog_logbook");
    if( cfg.host.empty() && cfg.logbook.empty() )
        parse_legacy_elog_command(commands.get("elog"), cfg);

    std::string v;
    if( !(v = commands.get("elog_port")).empty() && !parse_int(v, cfg.port) )
        std::cerr << "elog: ignoring invalid elog_port '" << v << "'\n";
    if( !(v = commands.get("elog_subdir")).empty() )
        cfg.subdir = v;
    if( !(v = commands.get("elog_user")).empty() )
        cfg.user = v;
    if( !(v = commands.get("elog_password")).empty() )
        cfg.password = v;
    if( !(v = commands.get("elog_encoding")).empty() )
        cfg.encoding = v;
    if( !(v = commands.get("elog_ssl_verify")).empty() )
        cfg.ssl_verify = parse_bool(v, cfg.ssl_verify);
    if( !(v = commands.get("elog_timeout")).empty() && !parse_int(v, cfg.timeout_s) )
        std::cerr << "elog: ignoring invalid elog_timeout '" << v << "'\n";
    if( cfg.timeout_s <= 0 )
        cfg.timeout_s = 10;

    // strip slashes around the subdir, they are added when building the URL
    while( !cfg.subdir.empty() && cfg.subdir[0] == '/' )
        cfg.subdir.erase(0, 1);
    while( !cfg.subdir.empty() && cfg.subdir[cfg.subdir.size()-1] == '/' )
        cfg.subdir.erase(cfg.subdir.size()-1);

    if( cfg.password.empty() && !cfg.user.empty() ) {
        const char* env = std::getenv("ELOG_PASSWORD");
        if( env )
            cfg.password = env;
    }

    const std::map<std::string, std::string> attrs = commands.get_prefixed("elog_attr_");
    elog_attributes extra(attrs.begin(), attrs.end());
    cfg.attributes = merge_attributes(cfg.attributes, extra);

    return cfg;
}

// ########################################################################

elog_result elog_submit(const elog_config& cfg, const elog_attributes& attrs,
                        const std::string& text)
{
    elog_result res;
    if( !cfg.enabled() ) {
        res.error = "elog not configured (elog_host and elog_logbook are required)";
        return res;
    }

#if !defined(SO_NOSIGPIPE)
    sigpipe_guard sg;
#endif

    connection c;
    int port = 0;
    if( !open_connection(cfg, c, port, res) )
        return res;

    const std::string boundary = make_boundary();
    const std::string body = build_body(cfg, merge_attributes(cfg.attributes, attrs), text, boundary);
    const std::string header = build_header(cfg, port, c.is_tls(), boundary, body.size());

    std::string err;
    if( !c.write_all(header, err) || !c.write_all(body, err) ) {
        res.error = err;
        mode_erase(mode_key(cfg));
        return res;
    }

    std::string response;
    if( !c.read_all(response, err) ) {
        res.error = err;
        return res;
    }
    c.close();

    parse_response(response, res);
    return res;
}

// ########################################################################

static void submit_and_report(const elog_config cfg, const elog_attributes attrs,
                              const std::string text)
{
    // one entry at a time: keeps entries in order and probes TLS only once
    static std::mutex submit_mutex;
    std::lock_guard<std::mutex> lock(submit_mutex);

    elog_result res;
    try {
        res = elog_submit(cfg, attrs, text);
    } catch( std::exception& e ) {
        res.ok = false;
        res.error = e.what();
    }

    std::string subject;
    for(std::size_t i=0; i<attrs.size(); ++i)
        if( to_lower(attrs[i].first) == "subject" )
            subject = attrs[i].second;

    std::ostringstream out;
    for(std::size_t i=0; i<res.warnings.size(); ++i)
        out << "elog: " << res.warnings[i] << '\n';
    if( res.ok ) {
        out << "elog: entry '" << subject << "' submitted to " << cfg.logbook;
        if( res.message_id >= 0 )
            out << " (ID=" << res.message_id << ")";
        out << '\n';
    } else {
        out << "elog: ERROR: could not submit entry '" << subject << "' to "
            << cfg.describe() << ": " << res.error << '\n';
    }
    const std::string s = out.str();
    std::cerr.write(s.data(), static_cast<std::streamsize>(s.size()));
    std::cerr.flush();
}

void elog_submit_async(const elog_config& cfg, const elog_attributes& attrs,
                       const std::string& text)
{
    try {
        std::thread(submit_and_report, cfg, attrs, text).detach();
    } catch( std::exception& e ) {
        std::cerr << "elog: ERROR: cannot start submit thread: " << e.what() << std::endl;
    }
}

// ########################################################################
// ########################################################################

#ifdef TEST_ELOG_CLIENT

int main(int argc, char* argv[])
{
    const std::string expect = "$5$saltstring$5B8vYYiY.CVt1RlTTf8KbXBH3hsxY/GNooZaBBGWEc5";
    const std::string got = elog_detail::sha256_crypt("Hello world!", "$5$saltstring");
    const std::string expect_r = "$5$rounds=10000$saltstringsaltst$3xv.VbSHBb41AL9AvLeujZkZRBAwqFMz2.opqey6IcA";
    // reference value from elog's own crypt.cxx: sha256_crypt("secret", "$5$") + 4
    const std::string expect_e = "o1w7HPzHt8gHmL5zUzDkRqt6Az.BjDfkvuDNNLWnXx5";
    const std::string got_r = elog_detail::sha256_crypt("Hello world!", "$5$rounds=10000$saltstringsaltstring");
    std::cout << "sha256 backend : " << (elog_ssl_available() ? "OpenSSL" : "built-in") << "\n"
              << "crypt vector 1 : " << (got == expect ? "OK" : "FAILED: " + got) << "\n"
              << "crypt vector 2 : " << (got_r == expect_r ? "OK" : "FAILED: " + got_r) << "\n"
              << "elog hash      : " << (elog_detail::elog_password_hash("secret") == expect_e ? "OK" : "FAILED") << std::endl;
    if( argc == 2 )
        std::cout << "hash(argv[1])  : " << elog_detail::elog_password_hash(argv[1]) << std::endl;
    if( got != expect || got_r != expect_r || elog_detail::elog_password_hash("secret") != expect_e )
        return EXIT_FAILURE;

    if( argc < 4 ) {
        std::cerr << argv[0] << " <commands-file> <subject> <text>" << std::endl;
        return argc <= 2 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    command_list c;
    if( !c.read(argv[1]) ) {
        std::cerr << "Could not read '" << argv[1] << "'" << std::endl;
        return EXIT_FAILURE;
    }
    const elog_config cfg = elog_config_from_commands(c);
    std::cout << "posting to     : " << cfg.describe() << std::endl;

    elog_attributes attrs;
    attrs.push_back(std::make_pair(std::string("Subject"), std::string(argv[2])));
    const elog_result res = elog_submit(cfg, attrs, argv[3]);
    for(std::size_t i=0; i<res.warnings.size(); ++i)
        std::cerr << "elog: " << res.warnings[i] << '\n';
    if( !res.ok ) {
        std::cerr << "elog: ERROR: " << res.error << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "submitted, ID=" << res.message_id << std::endl;
    return EXIT_SUCCESS;
}

#endif /* TEST_ELOG_CLIENT */

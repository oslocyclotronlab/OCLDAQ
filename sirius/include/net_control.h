// -*- c++ -*-

#ifndef NET_NEW2_H
#define NET_NEW2_H 1

#include <map>
#include <set>
#include <sstream>

#include <sys/select.h>
#include <time.h>

// ########################################################################

class do_not_copy {
public:
    do_not_copy() { }
    ~do_not_copy() { }
private:
    do_not_copy(const do_not_copy&);
    do_not_copy& operator=(const do_not_copy&);
};

// ########################################################################

class io_control;

class io_channel : private do_not_copy {
public:
    io_channel(io_control& ic, int fd=-1);
    virtual ~io_channel();

    int get_fd() const { return fd; }

    virtual void handle_read()  { }
    virtual void handle_write() { }

protected:
    io_control& ioc() { return _ioc; }
    void close();

private:
    io_control& _ioc;
    int fd;
};

// ########################################################################

class io_control : private do_not_copy {
public:
    io_control() { };
    virtual ~io_control() { };

    virtual void update(io_channel* ic, bool read, bool write)=0;
    virtual void remove(io_channel* ic)=0;
};

// ########################################################################

class io_select : public io_control {
public:
    io_select();
    ~io_select();

    bool run(struct timeval* timeout);

    virtual void update(io_channel* ic, bool read, bool write);
    virtual void remove(io_channel* ic);

private:
    typedef std::map<int,io_channel*> io_channel_map;
    io_channel_map io_channels;

    fd_set read_fd_set, write_fd_set;
};

// ########################################################################

class tcp_server : public io_channel {
public:
    tcp_server(io_control& ioc, int port, std::string const& name);
    ~tcp_server();

    bool listen();
    
    void handle_read();

    virtual io_channel* new_channel(int /*fd*/) { return 0; }
    virtual void remove_channel(io_channel* ic);

protected:
    std::string name;
    std::set<io_channel*> io_channels;
};

// ########################################################################

class line_channel;

class line_callback {
public:
    line_callback() { }
    virtual ~line_callback() { }
    virtual void run(line_channel* lc) = 0;
};

// ########################################################################

class line_cb : public line_callback {
public:
    typedef void (*callback_t)(line_channel* lc, void* user_data);

    line_cb(callback_t c, void* ud=0)
	: callback(c), user_data(ud) { }

    virtual void run(line_channel* lc)
	{ if(callback) callback(lc, user_data); }

private:
    callback_t callback;
    void* user_data;
};

// ########################################################################

class line_server : public tcp_server {
public:
    line_server(io_control& ioc, int port, std::string const& name,
		line_callback* cb_connected, line_callback* cb_disconnected, line_callback* cb_have_line);
    ~line_server();
    
    io_channel* new_channel(int fd);
    void send_all(const std::string& message);

    void client_disconnected(line_channel* lc);
    void client_has_line(line_channel* lc);

private:
    line_callback *cb_connected, *cb_disconnected, *cb_have_line;
};

// ########################################################################

class line_channel : public io_channel {
public:
    line_channel(io_control& ioc, int fd, line_callback *disconnected,
		 line_callback *have_line);
    ~line_channel();

    void send(const std::string& message);

    bool has_line() const { return !line.empty(); }
    std::string get_line() { return line; }

    void handle_read();
    void handle_write();

    void disconnect();

private:
    void check_line(size_t old_inbuf);

    std::string inbuf, outbuf, line;
    line_callback *cb_disconnected, *cb_have_line;
};

line_channel* line_connect(io_control& ioc, const char* host, int port,
			   line_callback *cb_disconnected, line_callback *cb_have_line);

// ########################################################################

class line_sender : public std::ostringstream {
public:
    line_sender(line_channel* lc_) : lc(lc_), sent(false) { }
    void send();
    ~line_sender() { send(); }
private:
    line_channel* lc;
    bool sent;
};

// ########################################################################

class command_cb : public line_callback {
public:
    typedef void (*cmd_cb)(line_channel* lc, const std::string& line, void* user_data);

    struct command {
	const char* name;
	const bool has_args;
	cmd_cb callback;
	void* user_data;
    };

    command_cb(command* command_list, std::string err_prefix)
	: commands(command_list), error_prefix(err_prefix) { }
    
    virtual void run(line_channel* lc);

private:
    command* commands;
    std::string error_prefix;
};

// ########################################################################

#endif /* NET_NEW2_H */

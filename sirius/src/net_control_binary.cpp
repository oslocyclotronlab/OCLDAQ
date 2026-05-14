
// ########################################################################

binary_channel::binary_channel(io_control& ioc_, int fd, binary_callback *cb_dis, binary_callback *cb_bin)
    : io_channel(ioc_, fd)
    , _state(STATE_HEADER)
    , cb_disconnected(cb_dis)
    , cb_binary(cb_bin)
{
    DBGL;
    ioc().update(this, true, false);
}

// ########################################################################

binary_channel::~binary_channel()
{
    DBGL;
    delete cb_disconnected;
    delete cb_binary;
}

// ########################################################################

void binary_channel::send_frame(unsigned int s, unsigned int us, const void* data, size_t size)
{
    const unsigned int magic = 0xDEADBEEF;
    const size_t header_size = 4 * 4; // magic, s, us, size
    
    outbuf.resize(header_size + size);
    
    unsigned int* header = reinterpret_cast<unsigned int*>(outbuf.data());
    header[0] = magic;
    header[1] = s;
    header[2] = us;
    header[3] = static_cast<unsigned int>(size);
    
    std::memcpy(outbuf.data() + header_size, data, size);
    
    handle_write();
}

// ########################################################################

void binary_channel::handle_read()
{
    DBGL;
    char tmp_buf[1024];
    const int n_read = read(get_fd(), tmp_buf, sizeof(tmp_buf));

    const bool would_have_blocked = (n_read < 0 && errno == EAGAIN);
    if( n_read == 0 || (n_read < 0 && !would_have_blocked) ) {
        disconnect();
        return;
    }
    if( n_read > 0 ) {
        inbuf.insert(inbuf.end(), tmp_buf, tmp_buf + n_read);
        
        while(true) {
            if (_state == STATE_HEADER) {
                if (inbuf.size() < 4) break;
                unsigned int magic = *reinterpret_cast<unsigned int*>(inbuf.data());
                if (magic != 0xDEADBEEF) {
                    // Sync error: discard one byte and try again
                    inbuf.erase(inbuf.begin());
                    continue;
                }
                _state = STATE_METADATA;
            }
            
            if (_state == STATE_METADATA) {
                if (inbuf.size() < 16) break; // 4 magic + 3*4 metadata
                _state = STATE_PAYLOAD;
            }
            
            if (_state == STATE_PAYLOAD) {
                unsigned int size = *reinterpret_cast<unsigned int*>(inbuf.data() + 12);
                if (inbuf.size() < 16 + size) break;
                
                unsigned int s = *reinterpret_cast<unsigned int*>(inbuf.data() + 4);
                unsigned int us = *reinterpret_cast<unsigned int*>(inbuf.data() + 8);
                const unsigned char* data = inbuf.data() + 16;
                
                if (cb_binary)
                    cb_binary->run(this, s, us, data, size);
                
                inbuf.erase(inbuf.begin(), inbuf.begin() + 16 + size);
                _state = STATE_HEADER;
            }
        }
    }
}

// ########################################################################

void binary_channel::handle_write()
{
    DBGL;
    const size_t os = outbuf.size();
    if( os == 0 )
	return;

    int w = write(get_fd(), outbuf.data(), os);
    if( w == 0 || (w < 0 && errno != EAGAIN) ) {
	disconnect();
    } else if( w > 0 ) {
	outbuf.erase(outbuf.begin(), outbuf.begin() + w);
	ioc().update(this, true, !outbuf.empty());
    }
}

// ########################################################################

void binary_channel::disconnect()
{
    DBGL;
    close();
    if( cb_disconnected )
	cb_disconnected->run(this, 0, 0, nullptr, 0); // Note: binary_callback signature
}

// ########################################################################

binary_server::binary_server(io_control& ioc, int port, std::string const& name,
			  binary_callback* cb_conn, binary_callback* cb_dis, binary_callback* cb_bin)
    : tcp_server(ioc, port, name)
    , cb_connected(cb_conn)
    , cb_disconnected(cb_dis)
    , cb_binary(cb_bin)
{
    DBGL;
    if( !listen() ) {
        throw std::runtime_error(name + ": could not listen on port " + std::to_string(port));
    }
}

// ########################################################################

binary_server::~binary_server()
{
    DBGL;
    delete cb_connected;
    delete cb_disconnected;
    delete cb_binary;
}

// ########################################################################

io_channel* binary_server::new_channel(int fd)
{
    DBGL;
    if( io_channels.size() >= 16 )
	return 0;

    fcntl(fd, F_SETFL, O_NONBLOCK);
    binary_channel* bc = new binary_channel(ioc(), fd,
					 new binary_cb(binary_server_cb_disconnected, this),
					 new binary_cb(binary_server_cb_has_binary, this));
    if( cb_connected )
	    cb_connected->run(bc, 0, 0, nullptr, 0);
    return bc;
}

// ########################################################################

void binary_server::send_all(unsigned int s, unsigned int us, const void* data, size_t size)
{
    DBGL;
    std::set<io_channel*>::iterator it;
    for(it = io_channels.begin(); it != io_channels.end(); ++it) {
	binary_channel* bc = static_cast<binary_channel*>( *it );
	bc->send_frame(s, us, data, size);
    }
}

// ########################################################################

void binary_server::client_disconnected(binary_channel* bc)
{
    DBGL;
    remove_channel(bc);
    if( cb_disconnected )
	cb_disconnected->run(bc, 0, 0, nullptr, 0);
}

// ########################################################################

void binary_server::client_has_binary(binary_channel* bc, unsigned int s, unsigned int us, const unsigned char* data, size_t size)
{
    DBGL;
    if( cb_binary )
	cb_binary->run(bc, s, us, data, size);
}

// ########################################################################

binary_channel* binary_connect(io_control& ioc, const char* host, int port,
			   binary_callback *cb_disconnected, binary_callback *cb_binary)
{
    DBGL;
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0)
	return 0;
    
    struct sockaddr_in servername;
    init_sockaddr(&servername, host, port);
	
    if(connect(sock, (struct sockaddr*)&servername, sizeof(servername)) < 0) {
	close(sock);
	return 0;
    }

    fcntl(sock, F_SETFL, O_NONBLOCK);
    return new binary_channel(ioc, sock, cb_disconnected, cb_binary);
}

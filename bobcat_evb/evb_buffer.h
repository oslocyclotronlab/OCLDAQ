
#ifndef evb_buffer_H
#define evb_buffer_H 1

/// test a bit of an integer
inline int bit(unsigned int value, int bit) { return (value & (1L << bit)) != 0; }
/* this does not really fit here, but making a header file for one
   line seems exaggerated to me. */

// ########################################################################
// ########################################################################

const int MAXBUF = 0x8000; // size of buffer (in 4-byte words)

// these three variables should not be used outside evb_buffer.*
extern unsigned long* buffer;
extern long idx_data, idx_start;

/// allocate the buffer memory and initialize the data pointers
void buffer_open();


/// release the buffer memory
void buffer_close();


/// add data to the buffer
inline void buffer_put(unsigned int boxid, unsigned int channel, unsigned int data)
{
    buffer[idx_data++] = boxid | channel | data;
}


/// copy buffer[start...start+count] to dest
void buffer_copy(long start, long count, unsigned long* dest);


/// returns a non-zero value if there is data in the buffer
inline int buffer_is_empty()
{
    return idx_data == 1;
}


/// add an end-of-buffer marker, and fill the remainder of the buffer
/// with zeroes
void buffer_fill0();


/// empties the buffer, i.e. all contents are forgotton
inline void buffer_reset()
{
    idx_data = 1;
    idx_start = 0;
}


/// returns the maximum possible event data size that can still be
/// stored
inline int buffer_capacity()
{
    return (MAXBUF-idx_data);
}


/// length of the current event
inline int buffer_eventlength()
{
    return (idx_data - idx_start - 1);
}


/// write the event length into the header and prepare for the next
/// event
void buffer_accept();


/// write the buffer contents to a file
void buffer_dump(const char* filename);

#endif /* evb_buffer_H */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** mode: c++ ***/
/*** End: ***/

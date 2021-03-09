// -*- c++ -*-

#ifndef ENGINE_SHM_H
#define ENGINE_SHM_H 1

enum {
    ENGINE_TIME_US,
    ENGINE_TIME_S,
    ENGINE_DATA_START,
    ENGINE_DATA_SIZE,
    ENGINE_FIRST_HEADER
};

enum {
    ENGINE_SHM_KEY = 2009
};

struct engine_shm_t {
    unsigned int *time_us;
    unsigned int *time_s;

    unsigned int *data;
    unsigned int size;
};

unsigned int* engine_shm_attach(bool write);
bool engine_shm_detach();
bool engine_shm_read(unsigned int *, engine_shm_t *);

#endif /* ENGINE_SHM_H */

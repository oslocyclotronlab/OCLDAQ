// -*- c++ -*-

#ifndef ENGINE_SHM_H
#define ENGINE_SHM_H 1

enum {
    ENGINE_TIME_US,
    ENGINE_TIME_S,
    ENGINE_DATA_START,
    ENGINE_DATA_SIZE
};

enum {
    ENGINE_SHM_KEY = 2009
};

unsigned int* engine_shm_attach(bool write);
bool engine_shm_detach();

#endif /* ENGINE_SHM_H */

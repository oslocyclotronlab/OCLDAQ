// -*- c++ -*-

#ifndef M_ENGINE_H
#define M_ENGINE_H 1

class io_control;

void m_engine_read();
bool m_engine_connect(io_control& ioc);
bool m_engine_connect(io_control& ioc, const char *host);
bool m_engine_disconnect();
bool m_engine_quit();
bool m_engine_stop();
bool m_engine_start();
bool m_engine_status();
bool m_engine_reload();
bool m_engine_output_none();
bool m_engine_output_filename(const char* filename);
bool m_engine_output_dir(const char* dirname);

bool m_engine_is_started();
bool m_engine_is_connected();
const char* m_engine_get_output();
const char* m_engine_get_output_dir();
int m_engine_get_buffercount();
float m_engine_get_buffer_rate();

#endif /* M_ENGINE_H */

// -*- c++ -*-

#ifndef M_SORT_H
#define M_SORT_H 1

class io_control;

void m_sort_read();
bool m_sort_connect(io_control& ioc);
bool m_sort_connect(io_control& ioc, const char *host);
bool m_sort_disconnect();
bool m_sort_quit();
bool m_sort_clear();
bool m_sort_dump();
bool m_sort_gainshift(const char* filename);
bool m_sort_telewin(const char* filename);
bool m_sort_range(const char* filename);

bool m_sort_status_gain();
bool m_sort_status_telewin();
bool m_sort_status_user();

bool m_sort_is_connected();
void m_sort_get_buffers(int& buffer_count, int& error_count, float& average_length);

bool m_sort_change_cwd(const char* dirname);
const char* m_sort_get_cwd();

#endif /* M_SORT_H */

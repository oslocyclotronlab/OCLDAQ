// -*- c++ -*-

#ifndef RUN_COMMAND_H
#define RUN_COMMAND_H 1

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

std::vector<std::string> word_expand(const char* cmd);

bool run_background(const std::string& cmd);

// ########################################################################

class command_list {
public:
    command_list();
    ~command_list();

    bool read(std::istream& input);
    bool read(const char* filename);

    bool read_text(const char* text);

    bool run(const std::string& cmd_key, const std::vector<std::string>& xargs);

    bool run(const std::string& cmd_key)
        { const std::vector<std::string> xargs; return run(cmd_key, xargs); }

    bool run(const std::string& cmd_key, const std::string& xarg)
        { const std::vector<std::string> xargs(1, xarg); return run(cmd_key, xargs); }

private:
    std::map<std::string, std::string> commands;
};

#endif /* RUN_COMMAND_H */

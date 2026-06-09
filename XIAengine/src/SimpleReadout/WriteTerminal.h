#ifndef WRITETERMINAL_H
#define WRITETERMINAL_H

#include <mutex>

/*!
 * \brief The WriteTerminal class
 * This class provides a safe way
 * for all threads to write text
 * to the terminal without text
 * being written at the same time
 * for two different threads.
 */
class WriteTerminal
{
public:

    /*!
     * \brief WriteError: Will write the message to the terminal via the std::cerr
     * \param message - String to be written to the terminal.
     */
    void WriteError(const char *message);

    /*!
     * \brief Write: Will write the message to the terminal via the std::cout
     * \param message - String to be written to the terminal.
     */
    void Write(const char *message);


private:
    std::mutex io_mutex;
};

#endif // WRITETERMINAL_H

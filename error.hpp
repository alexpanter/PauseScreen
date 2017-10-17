
// I/O REDIRECTION
#pragma once

#include <iostream>
#include <fstream>
#include <string>

//using std::string;


// #define IO_REDIRECT
#define ERROR_FILE "error_log.txt"

typedef enum {
    MSG_CERR,
    MSG_COUT
} _msg_type_t;

typedef const char* c;

void write_msg(_msg_type_t type, c msg, c _file, int _line)
{
    std::string t("");
    switch(type) {
    case MSG_CERR:
        t += "ERROR: "; break;
    case MSG_COUT:
        t += "MESSAGE: "; break;
    default:
        break;
    }

#ifdef IO_REDIRECT
    static std::ofstream err_file;
    err_file.open(ERROR_FILE);

    err_file << t << "'" << msg << "', in " << _file << ": " << _line << std::endl;
    err_file.close();
#else
    switch(type) {
    case MSG_CERR:
        std::cerr << t << "'" << msg << "', in " << _file << ": " << _line << std::endl;
        break;
    case MSG_COUT:
        std::cout << t << "'" << msg << "', in " << _file << ": " << _line << std::endl;
        break;
    default:
        break;
    }
#endif
}
void write_msg(_msg_type_t type, std::string msg, c _file, int _line)
{
    write_msg(type, const_cast<char*>(msg.c_str()), _file, _line);
}


#define PRINT_MSG_COUT(msg) ( write_msg ( MSG_COUT, msg, __FILE__, __LINE__ ) )
#define PRINT_MSG_CERR(msg) ( write_msg ( MSG_CERR, msg, __FILE__, __LINE__ ) )


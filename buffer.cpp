#include <stdlib.h>
#include <string.h>
#include <string>
#include "./error.hpp"

class Buffer
{
private:
    const unsigned short _size;
    char* _buffer;
    unsigned short _read_index;
    const char* _correct;

public:
    Buffer(unsigned short size, const char* correct)
        : _size(size), _correct(correct)
    {
        _buffer = (char*) malloc(sizeof(char) * _size);
        _read_index = 0;
        this->Reset();
    }

    ~Buffer()
    {
        free(_buffer);
    }


    void Reset()
    {
        _read_index = 0;
        memset(_buffer, '\0', _size);
    }

    void Print()
    {
        std::stringstream s;
        s << "entry: '" << _buffer << "'";
        PRINT_MSG_COUT(s.str());
    }

    void Insert(int ch)
    {
        if(!(_read_index >= _size))
        {
            _buffer[_read_index++] = ch;
        }
    }

    void Delete()
    {
        if(_read_index > 0)
        {
            _buffer[--_read_index] = '\0';
        }
    }

    bool Test()
    {
        if(strncmp(_correct, _buffer, _size) == 0)
        {
            PRINT_MSG_COUT("correct password!");
            return true;
        }
        PRINT_MSG_CERR("invalid password!");
        Reset();
        return false;
    }
};

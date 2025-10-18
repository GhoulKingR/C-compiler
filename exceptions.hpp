#pragma once

#include <exception>
#include <string>

class syntax_error : public std::exception
{
    std::string message;
public:
    syntax_error(std::string e)
    : message(e) {}

    const char * what() const noexcept override
    {
        return message.c_str();
    }
};

#pragma once
#include <stdlib.h>
#include <iostream>
#include <string.h>

class Exception
{
private:
    char *msgbuf;
public:
    Exception(const std::string & _message = "unknown", int _n=0) throw()
    {
      size_t len = _message.length();
      msgbuf = new char[len + 1];
      if (!msgbuf) return;
      strcpy(msgbuf, _message.c_str());
    }
    virtual ~Exception() throw(){delete msgbuf;}
    // Returns a C-style character string describing the general cause
    //  of the current error.
    virtual const char* what() const throw() {return msgbuf;}
};

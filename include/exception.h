#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <string>
using std::string;

class BaseException
{
public:
    BaseException(const string& f, int l, const string& e):
        filename(f),
        line_num(l),
        error_info(e)
    {
    }
    
    string filename;
    int line_num;
    string error_info;
};

class ServeFileException : public BaseException 
{
public:
    ServeFileException(const string& f, int l, const string& e):
        BaseException(f, l, e)
    {
    }
};

class PutFileException : public BaseException 
{
public:
    PutFileException(const string& f, int l, const string& e):
        BaseException(f, l, e)
    {
    }
};

class ExecuteCGIException : public BaseException 
{
public:
    ExecuteCGIException(const string& f, int l, const string& e):
        BaseException(f, l, e)
    {
    }
};

#endif
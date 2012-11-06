#ifndef _READEREXCEPT_
#define _READEREXCEPT_

#include <exception>
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;

// FileError and its children FileOpenError and FileReadError

class FileError : public exception
{
    public:
        FileError(string inFilename);
        virtual ~FileError()
            throw();
        virtual const char* what();
    private:
        string filename;
        string message;
};

class FileOpenError : public FileError
{
    public:
        FileOpenError(string inFilename);
        virtual ~FileOpenError()
            throw();
        virtual const char* what();
    private:
        string filename;
        string message;
};

class FileReadError : public FileError
{
    public:
        FileReadError(string inFilename, int line=0);
        virtual ~FileReadError()
            throw();
        virtual const char* what();
    private:
        string filename;
        string message;

};

// DelimError and its children NoDelimsError and MissingDelimError

class DelimError : public exception
{
    public:
        DelimError(string inFilename);
        virtual ~DelimError()
            throw();
        virtual const char* what();
    private:
        string filename;
        string message;
};

class NoDelimsError : public DelimError
{
    public:
        NoDelimsError(string inFilename);
        virtual ~NoDelimsError()
            throw();
        virtual const char* what();
    private:
        string filename;
        string message;
};

class MissingDelimError : public DelimError
{
    public:
        MissingDelimError(string inFilename, int inRow);
        virtual ~MissingDelimError()
            throw();
        virtual const char* what();
    private:
        string filename;
        int rowNumber;
        string message;
};


#endif

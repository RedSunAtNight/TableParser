#include "readerExcept.h"

using namespace std;

// FileError methods. Should be overridden in all useful cases in favor of more specific problems.

FileError::FileError(string inFilename) : filename(inFilename), message("Error acessing file " + inFilename)
{
    // not much to do here, I don't think. I believe exception has a default constructor
}

FileError::~FileError()
    throw()
{
    // not much to do here, either. Just exists so that the FileError base class has a virtual constructor
}

const char* FileError::what()
{
    return message.c_str();
}

// FileOpenError methods. Should be used when there is an error opening a data file.

FileOpenError::FileOpenError(string inFilename) : FileError(inFilename), filename(inFilename)
{
    message = "Error occurred while opening file " + filename + ". Make sure that the name is spelled correctly and the directory exists.";
}

FileOpenError::~FileOpenError()
    throw()
{
    // nothing much here. Just need an overriding destructor
}

const char* FileOpenError::what()
{
    return message.c_str();
}

// FileReadError methods. Should be used when there is an error reading a file that has been successfully opened.

// There is a default argument of zero for the integer parameter, in case line information is not forthcoming.
FileReadError::FileReadError(string inFilename, int line) : FileError(inFilename), filename(inFilename)
{
    if (line == 0)
    {
        message = "Error while reading file " + filename + ". Line at which error occurred was not supplied.";
    }
    else
    {
        string strLine = boost::lexical_cast<string>(line);
        message = "Error while reading file " + filename + ". Error occurred at line " + strLine + ".";
    }
}

FileReadError::~FileReadError()
    throw()
{
    // nothing much here. Just making sure the base class destructor gets overridden.
}

const char* FileReadError::what()
{
    return message.c_str();
}


// DelimError methods. for exceptions thrown by delimFinder objects
DelimError::DelimError(string inFilename) : filename(inFilename)
{
    message = "Error dealing with delimiters for data file " + inFilename + ".";
}

DelimError::~DelimError()
    throw()
{
    // nothing here. Just set as virtual to make sure the hierarchy works right.
}

const char* DelimError::what()
{
    return message.c_str();
}

// NoDelimsError methods. thrown when a delimFinder finds no delimiters. Does not necessarily need to end the program.
NoDelimsError::NoDelimsError(string inFilename) : DelimError(inFilename), filename(inFilename)
{
    message = "No delimiters found for data in file" + inFilename + ". Data may consist of only one column.";
}

NoDelimsError::~NoDelimsError()
    throw()
{
    // nothing here. Just set as virtual to make sure the hierarchy works right.
}

const char* NoDelimsError::what()
{
    return message.c_str();
}

// MissingDelimError methods, thrown when TableReader fails for encounter a sufficient number of instances of the delimiter in a row of data
MissingDelimError::MissingDelimError(string inFilename, int inRow) : DelimError(inFilename), filename(inFilename), rowNumber(inRow)
{
    string strRow = boost::lexical_cast<string>(inRow);
    message = "Encountered too few instances of the delimiter character in row " + strRow + " of file " + inFilename;
}

MissingDelimError::~MissingDelimError()
    throw()
{
    // nothing to do here
}

const char* MissingDelimError::what()
{
    return message.c_str();
}

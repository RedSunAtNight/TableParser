// header file for TableReader class
#ifndef _TABLE_READER_
#define _TABLE_READER_

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <boost/regex.hpp>
#include "readerExcept.h"

// forward declarations, for sanity
class DelimFinder;
class Delimiter;


class TableReader
{
    public:
        TableReader(); // need to make another version thar takes arguments
        virtual ~TableReader(); // should check if dataFile.is_open() is true. If it is, close the file.
        virtual void setDelim(const std::string& inDelim); // sets the character(s) used as the delimiter in the file. Also sets autoDelim to false, so that the user's decision does not get overridden.
        virtual void setDelim(const char inDelim); // sets the character(s) used as the delimiter in the file. Also sets autoDelim to false, so that the user's decision does not get overridden.
        virtual void unsetDelim();
        virtual void loadData(std::string fileName); // reads the data from the file and puts it into rawRows. Checks whether the dataFile is open first. There should be a constructor that does this automatically. This cannot be the final step, as you need to give the user ample time to call setDelim() if they want.
        virtual void makeRawColumns();
        // "get" methods:
        virtual std::string getDelim(); // returns the value of delim. lets you check what delimiter you're using.
        virtual std::string getFilename(); // returns the filename you're using
        virtual std::string getRawRow(int index); // returns the unprocessed row at index
        virtual int getNumRawRows(); // returns the number of raw rows
        virtual string getDelimFinderInfo(); // returns finderCode, as well as what it means
        virtual int getNumCols(); // returns the number of columns in the data (held in numCols)
        virtual vector< vector<string> > getRawCols();
    friend class DelimFinder;
    protected:
        // helper functions:
       // virtual void determineDelim(); // guesses the delimiter based on a reading of the data from the file
        // data members
        std::ifstream dataFile; // the file stream object used to load in the data from the text file
        std::string filename; // the name of the file containing the data to be loaded
        bool autoDelim; // whether or not to use determineDelim(). defaults to true
        std::string delim; // the delimiter used for separating columns, can be set by setDelim() or by determineDelim().
        std::vector<std::string> rawRows; // each element is one line from the data file, or a single row of unprocessed data
        std::vector< vector<string> > rawCols; // the columns of data, as strings. Includes column labels, if there are any.
        int numCols;
        int finderCode; // holds the return value of DelimFinder::guessDelim.

};

// the following class should only every be called in the context of an existing TableReader object
class DelimFinder
{
    public:
        DelimFinder(TableReader& inReader); // a DelimFinder without a TableReader cannot exist
        virtual ~DelimFinder();
        virtual int guessDelim(); // actually does the figuring out of the delimiter
            // return values : 0 -> no delims found ; 1 -> found only one delim ; 2 -> found multiple delims, used order of precedence to choose one
    protected:
        // helper functions:
        virtual void findPossibleDelims(); // goes through the last row of the raw text data and pulls out all the nonletter, nonnumber characters. Counts the occurrences of each.
        virtual void countDelimOccurrences(); // goes through the rest of the data set and counts how many times each possible delim (each character in lastRowCharas) occurs.
        virtual tuple<bool, int> searchDelims(char inChar); // searches through delimList to see if inChar has already been listed. Returns bool and a position.
        // data members
        std::vector<Delimiter> delimList; // each element is a nonletter, nonnumber character in the last row. There are no repeats. Items are removed from the list if they do not occur the same number of times in each row.
        vector<int> badInds; // the indices referring to Delimiter objects in the delimList, which have been determined to NOT be suitable delimiters
        bool usedTruncatedTable; // stores whether or not the DelimFinder object is basing its decision on a shortened version of the table. Starts as false, changes to true if the table is too long.
        TableReader& callingReader; // the TableReader object for which you're trying to find the delimiter

};

class Delimiter
{
    public:
        Delimiter(char inDelim);
        virtual ~Delimiter();
        virtual void addInstance(int row);
        virtual char getChara();
        virtual vector<int> getOccurrences();
        virtual void lengthenList(int rows);
        friend bool operator>(const Delimiter& lhs, const Delimiter& rhs);
        friend bool operator<(const Delimiter& lhs, const Delimiter& rhs);
        friend bool operator==(const Delimiter& lhs, const Delimiter& rhs);
    protected:
        char delimChar;
        vector<int> occurrences;
};


#endif

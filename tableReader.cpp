#include "tableReader.h"

// implementation for TableReader class

using namespace std;

// default constructor
TableReader::TableReader() : filename("NO_FILE"), autoDelim(true), delim("NO_DELIMITER"), finderCode(-10)
{
    // Sets the filename to "NO_FILE", the delimiter to "NO_DELIMITER", and autoDelim to true
    // C++'s default initializations are fine for everything else.
}

// destructor
TableReader::~TableReader()
{
    // checks if dataFile is still open. If it is, closes it.
    if (dataFile.is_open())
    {
        dataFile.close();
    }
}

// sets the delimiter to whatever string the user thinks appropriate. Can be multiple characters in length.
void TableReader::setDelim(const string& inDelim)
{
    // sets the string to be used as a delimiter. Also sets autoDelim to false, so the the user's decision does not get overridden.
    delim = inDelim;
    autoDelim = false;
}

void TableReader::setDelim(const char inDelim)
{
    // sets the char to be used as a delimiter. Also sets autoDelim to false, so the the user's decision does not get overridden.
    delim = boost::lexical_cast<string> (inDelim);
    autoDelim = false;
}

// lets you undo the actions of setDelim().
void TableReader::unsetDelim()
{
    // resets the delimiter to "NO_DELIMITER" and tell the code to switch autodelim back one
    delim = "NO_DELIMITER";
    autoDelim = true;
}

// returns the delimiter being used by the TableReader object. Returns "NO_DELIMITER" if no delimiter has been set.
string TableReader::getDelim()
{
    return delim;
}

// returns the name of the data file. Returns "NO_FILE" if no filename has been given.
string TableReader::getFilename()
{
    return filename;
}

// returns the raw, unprocessed text of one row of data
string TableReader::getRawRow(int index)
{
    return rawRows[index]; // make sure this throws an error if index is out of range
}

// returns the number of raw rows, or, the number of lines of data read from the file
int TableReader::getNumRawRows()
{
    return rawRows.size();
}

// returns the number of columns in the data table
int TableReader::getNumCols()
{
    return numCols;
}

// returns the set of raw (as in, string) columns
vector< vector<string> > TableReader::getRawCols()
{
    return rawCols;
}

void TableReader::loadData(string inFilename)
{
    // sets filename to the name of the file entered. Checks if the file is already open. Opens it if it is not.
    filename = inFilename;
    if (! dataFile.is_open())
    {
        dataFile.open(inFilename.c_str());  // C++ file io is bad about exceptions--in that it doesn't really throw any. You need to check the state of the file using " if dataFile.bad() " for reading and writing failure, and " if dataFile.eof() " fro what to do at the end of a file. You can throw an exception inside the if-statements.
        // Now, the file should be open no matter what. Check. If it's still not open, throw an exception.
        if (! dataFile.is_open())
        {
            throw FileOpenError(filename);
        }

    }

    int linecount = 0;
    while (dataFile.good())
    {
        linecount++;
        string row;
        getline(dataFile, row); // C++ file io is bad about exceptions--in that it doesn't really throw any. You need to check the state of the file using " if dataFile.bad() " for reading and writing failure, and " if dataFile.eof() " fro what to do at the end of a file. You can throw an exception inside the if-statements.
        rawRows.push_back(row);
    }
    // now, make sure that you didn't break out of the while loop before reaching the end of the file. if you did, it's because there was an error reading the file.
    if (! dataFile.eof())
    {
        throw FileReadError(filename, linecount);
    }

    // make sure you don't have an empty string at the end, messing things up
    int lastrow = rawRows.size() - 1;
    if (rawRows[lastrow] == "")
    {
        rawRows.erase(rawRows.begin() + lastrow);
    }
    // You're done with the file -- all the data is loaded in
    dataFile.close();

}
// makes columns of data.
void TableReader::makeRawColumns()
{
    // STEP ONE: Establish how many columns there are in the data table.

    // First, check if autoDelim is set to true. If it is, create a DelimFinder object and find the delim character:
    if (autoDelim)
    {
        //cout << "Automatically guessing delim..." << endl;
        DelimFinder newfinder(*this);
        finderCode = newfinder.guessDelim();
        // guessDelim automatically sets the value of numCols. Now, just add that number of empty vectors to rawCols:
        for (int n = 0; n < numCols; n++)
        {
            vector<string> emptyColumn;
            rawCols.push_back(emptyColumn);
        }
    } // end if block -- what to do if autoDelim is set to true
    // Now, there should be a character (or string, if user-set) in the delim field.
    //However, if the user set their own delim, it is not known how many columns there are in the data. Take care of that:
    else
    {
        //cout << "Delimiter already set by user to be " << delim << endl;
        // If autoDelim is false, that means the delimiter was set by the user. It still remains to determine the number of columns in the data table, and to add that number of empty vectors to rawCols.
        numCols = 1;
        vector<string> emptyColumn;
        rawCols.push_back(emptyColumn);
        // If there is an identifiable delimiter, the number of columns is equal to the number of times the delimiter appears per row, plus one:
        // item1 <delim> item2 <delim> item3   --> three items, two instances of the delimiter
        // look at the first row of data to determine the number of columns
        string firstRow = rawRows[0];
        size_t delimPos;
        int startPos = 0;
        bool moreCols = true;
        while (moreCols)
        {
            // search the first row of data for an instance of the delimiter character (or sequence).
            delimPos = firstRow.find(delim, startPos);
            if (delimPos != string::npos)
            {
                // if the delimiter character occurs, make a substring consisting of everything between the last delimiter character (or beginning of the row), and the next one.
                // add a column to the list rawCols
                rawCols.push_back(emptyColumn);
                // increase the column count
                numCols += 1;
                // move along to the next place to start searching for delimiters
                startPos = delimPos + delim.size();
            }// end of if block -- if the delimiter is found in the row
            else
            {
                // delimiter is not found, which means the end of the row has been reached
                moreCols = false; // break out of the while loop
            } // end of else -- if the delimiter is not found in the row
        } // end of while loop

        // At this point, numCols should contain the number of columns in the data table.
        // rawCols should contain some empty vectors-- the number of empty vectors should be equal to numCols.
    } // end else block -- what to do if the user has set their own delimiter
    //cout << "The number of columns is " << numCols << endl;
    //cout << "The number of vectors in RawCols is " << rawCols.size() << endl;

    // STEP TWO: Split each row into its component elements and organize these into columns.
    for (int rownum = 0; rownum < rawRows.size(); rownum++)
    {
        // in each row, find each instance of the delimiter
        string currentRow = rawRows[rownum];
        int startPos = 0;
        size_t foundPos;
        for (int k = 0; k < numCols-1; k++)
        {
            foundPos = currentRow.find(delim, startPos);
            //cout << "The character matching the delimiter is: " << currentRow[foundPos] << " at position " << foundPos << endl;
            // if the delimiter is not found, but there should still be more columns to go, throw an error
            if (foundPos == string::npos)
            {
                throw MissingDelimError(filename, rownum);
            }
            // if the exception wasn't thrown, move on
            // make a substring just containing the part of the row in between the delimiters
            string currentItem = currentRow.substr(startPos, foundPos - startPos);
            //cout << "The starting position for the substring is " << startPos << " and the ending position is " << foundPos << endl;
            //cout << "The current item being added to the list is: " << currentItem << endl;
            // add that substring to the right column and row in rawCols
            rawCols[k].push_back(currentItem);
            // move startPos so that the next iteration will find the next delimiter
            startPos = foundPos + 1;

        } // end of for loop through each row
        //cout << "Made it to the end of the for loop through row " << rownum << endl;
        // Now, add in the last column
        string lastItem = rawRows[rownum].substr(startPos, string::npos);
        //cout << "The last item in the row is " << lastItem << endl;
        rawCols[numCols - 1].push_back(lastItem);
        //cout << "... competed analyzing row " << rownum << endl;
    } // end for loop through the all the rows
    //cout << "Broke up the elements in all rows." << endl;
}

string TableReader::getDelimFinderInfo()
{
    string delimFinderMessage = "DelimFinder::guessDelim returned ";
    switch(finderCode)
    {
        case 0:
            delimFinderMessage += "0. This means that the DelimFinder object was unable to find any valid delimiters.";
            break;
        case 1:
            delimFinderMessage += "1. This means that the DelimFinder object found exactly one valid delimiter.";
            break;
        case 2:
            delimFinderMessage += "2. This means that the DelimFinder object found more than one character that could have been a valid delimiter, and chose one according to its built-in order of precedence.";
            break;
        case -10:
            delimFinderMessage += "-10. This is the value initially given to the variable that holds the return value of guessDelim. This means that guessDelim never actually executed.";
            break;
        default:
            delimFinderMessage += boost::lexical_cast<string>(finderCode);
            delimFinderMessage += ". This is not an established return code. An error must have occurred in DelimFinder::guessDelim.";
            break;
    }
    return delimFinderMessage;
}

// here are the protected helper functions


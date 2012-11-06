README

Project: TableParser
Author: RedSunAtNight
Version: 1.0 (5 November 2012)

CONTENTS:
I	PURPOSE
II	REQUIREMENTS
III	HOW TO USE THESE CLASSES
III	SUMMARY OF CLASSES

PURPOSE: 
The classes contained here will read a text file containing a flat table of data, and parse the data into columns so that the data can be used in a C++ program. The column delimiter can be set manually, or guessed automatically by the code.

REQUIREMENTS: 
This program makes use of the following Boost libraries: lexical_cast and regex. In order to use boost/regex, the code must be linked with libboost_regex.so. The code should be compiled with the flag -lboost_regex

HOW TO USE THESE CLASSES:
To parse a data table from a text file, first declare a TableReader object:
TableReader myreader;
Next, load the text file holding the table:
myreader.loadData(sample.dat);
If you know what the delimiter for the data table is, you can set it:
myreader.setDelim(",");
If you don't know what the delimiter is, or don't want to set it manually, just go on to the next step. The program will guess the delimiter based on what characters are used in the table.
Once the data is loaded, you can parse it into columns:
myreader.makeRawColumns();
Now, you can access one column from the table using:
myColumn = myreader.getRawCols()[column#];
where column# is a number, starting with zero and going to the number of columns - 1.
You can access an individual entry from the table with:
myItem = myreader.getRawCols()[column#][row#]

SUMMARY OF CLASSES: 

TableReader:
	*Header file: tableReader.h
	*Data members:
		ifstream dataFile - file stream object, used to bring in the text file containing the data to be parsed
		string filename - the name of the file, including the file path
		bool autoDelim - whether or not to guess the delimiter automatically; defaults to true
		string delim - the field delimiter used in the input data file; may be set by the user or guessed automatically
		vector<string> rawRows - each element in this vector is a line from the original text file; or, a row of data.
		vector<vector<string> > rawCols - a vector of vectors, this vector contains columns of data. Each element rawCols[i][j] contains the information held in column i, row j of the original data table.
		int numCols - the number of columns in the data table
		int finderCode - a numerical code returned by the DelimFinder object, which gives some information about the process that resulted in guessing the delimiter.
	*Methods:
		TableReader() - constructor, sets filename to "NO_FILE", delim to "NO_DELIMITER", autoDelim to true, and finderCode to -10 (indicating that the delimiter has not been guessed)
		~TableReader() - destructor, checks if the file stream is still open, and closes it if it is
		loadData(string inFilename) - sets the value of filename, opens the file indicated by inFilename, reads in the text from the file line-by line, adding each line to rawRows. If the last line in the data file is empty, it removes that line from rawRows. Can throw FileOpenError and FileReadError.
		setDelim(const string& inDelim) and setDelim(const char inDelim) - sets the value of delim, thus setting which string or character will be used to separate fields. Also sets autoDelim to false. This is how the user sets the delimiter manually. A delimiter set this way can be any string or character.
		unsetDelim() - resets delim to "NO_DELMITER" and autoDelim to true.
		makeRawColumns() - divides the lines of data in rawRows into columns, and stores these in rawCols. If the delimiter has been set, it uses that delimiter. Otherwise, it automatically guesses a delimiter (by creating and using a DelimFinder object) based on the contents of the file.
		getDelim() - returns the value of delim, as a string
		getFilename() - returns the name of the data file, as a string
		getRawRow(int index) - returns the row of data that is held at the given index in rawRows, as a string
		getNumRawRows() - returns the number of rows of data, as an int. Includes column headings.
		getDelimFinderInfo() - returns the value of finderCode, along with an explanation, as a string.
		getNumCols() - returns the number of columns of data as an int
		getRawCols() - returns the value of rawCols, as a vector of vectors of strings
	*Friend Class: DelimFinder

DelimFinder:
	*Header file: tableReader.h
	*Data members:
		TableReader& callingReader - a reference to the TableReader object that created the DelimFinder object
		bool usedTruncatedTable - whether or not the delimiter was guessed using a truncated version of the data table. Defaults to false, set to true if the table is large to save some time.
		vector<Delimiter> delimList - a vector of Delimiter objects, will contain all the valid delimiters that could be guessed for the data
		vector<int> badInds - the indices (in delimList) of those delimiters determined to not be the right ones.
	*Methods: 
		DelimFinder(TableReader& inReader) - constructor, takes a reference to the TableReader object that created it as an argument. Sets callingReader to inReader, and usedTruncatedTable to false. Calls getNumRawRows on callingReader, and if there are more than 100 rows of data, sets usedTruncatedTable to true.
		~DelimFinder() - destructor, placed there for consistency
		guessDelim() - guesses the delimiter used in the input data. Assumes the delim is a single, nonletter, nonnumber character. Sets the value of delim in callingReader. Returns an integer: 0=found no valid delimiters, 1=found exactly one valid delimiter, 2=found multiple valid delimiters and guessed which one was correct.
		findPossibleDelims() - protected method, used in guessDelim(). Scans the last row of data and records every nonletter, nonnumber character used, along with the number of times it occurred. This information is recorded as a Delimiter object for each character, and the Delimiter objects are stored in delimList.
		countDelimOccurrences() - protected method, used in guessDelim(). Scans the first through 98th lines of data (or the entire table except for the last row, if the table is smaller than 100 lines). Counts the number of times each possible delimiter found in findPossibleDelims comes up in each row, and records that number in the character's Delimiter object. Characters that come up different numbers of times in different rows are not considered valid delimiters.
		searchDelims() - protected method, used in guessDelim. A simple binary search, which searches for a character in a list of Delimiter objects. Returns a tuple<bool, int> containing whether or not the character already has a Delimiter object in the list, and if so, where in the list is it held.

Delimiter:
	*Header file: tableReader.h
	*Data members:
		char delimChar - the character for which the Delimiter object was made. Delimiter objects are made for all possible delimiter characters in the last row of data.
		vector<int> occurrences - the number of times the character delimChar occurred in each row of data. Almost in order; the last row of data corresponds to the first element in occurrences, the first row of the table corresponds to the second element in the list, and then the list goes in order after that.
	*Methods:
		Delimiter(char inDelim) - constructor, sets delimChar to the value of inDelim. Also adds the number 1 to the vector or occurrences, as a Delimiter object is only made when the character is found in the data table.
		~Delimiter() - destructor, included for consistency.
		addInstance(int row) - adds 1 to whatever number currently resides at index row in occurrences. If occurrences is not long enough and there is no element at that index, it pads the vector with zeroes until it gets to the right index.
		lengthenList(int rows) - pads the vector occurrences with a number of zeroes indicated by rows, by adding them to the end.
		getChara() - returns the value of delimChar as a char
		getOccurrences() - returns the value of occurrences as a vector<int>
	*Friend methods:
		operator>(const Delimiter& lhs, const Delimiter& rhs)
		operator>(const Delimiter& lhs, const Delimiter& rhs)
		operator==(const Delimiter& lhs, const Delimiter& rhs)
		- comparison operators used so that Delimiter objects can be sorted and searched, such as with DelimFinder::searchDelims().

FileError:
	*Header file: readerExcept.h
	*Base class for FileOpenError and FileReadError
	*Thrown when there is an error openeing or reading a file.

DelimError:
	*Header file: readerExcept.h
	*Base class for NoDelimsError and MissingDelimError

NoDelimsError:
	*Header file: readerExcept.h
	*Thrown when DelimFinder::countOccurrences() attempts to find the number of times each character in delimList occurs, but delimList is empty. Caught in DelimFinder::guessDelim(). This can happen when the data file only contains one column, or when something has gone wrong and delimList was never populated.

MissingDelimError:
	*Header file: readerExcept.h
	*Thrown when a TableReader::MakeRowColumns() is attempting to separate the rows of data into their individual elements, but reaches the end of the row before passing by the required number of delimiters. This can mean that either that row has fewer fields than all the other rows, or the delimiter was chosen incorrectly.

#include "tableReader.h"
// implementation for the DelimFinder class.
using namespace std;
using namespace boost;

// constructor. Note how there is no default constructor. callingReader NEEDS to be initialized. Its value tells the code which TableReader object will use this delimFinder object to find its delimiter
DelimFinder::DelimFinder(TableReader& inReader) : callingReader(inReader), usedTruncatedTable(false)
{
    if (callingReader.getNumRawRows() > 100)
    {
        usedTruncatedTable = true;
    }
}

// destructor. Only written to make sure it's virtual, so if I ever want to make a subclass of this, I don't have to worry.
DelimFinder::~DelimFinder()
{
    // nothing to do here
}

// WARNING: This is going to be SLOW. You need to search a vector for a match for a lot of characters.
int DelimFinder::guessDelim()
{
    findPossibleDelims(); // goes through the last row of the raw text data and pulls out all the nonletter, nonnumber characters. Counts the occurrences of each.
    try
    {
        countDelimOccurrences(); // counts up how many times each character found in findPossibleDelims occurs in the rest of the data table
    }
    catch (NoDelimsError& nde)
    {
        cerr << nde.what() << endl;
        return 0; // returning zero indicates zero delimiters
        // note that returning zero without setting the delim leaves it as "NO_DELIMITER"
    }
    catch (DelimError& de)
    {
        cerr << de.what() << endl;
        // change this to rethrow the exception, so it winds up at the top level
        // or take it out entirely
        // actually, in retrospect, you don't want it too far out, because you might really have a table with only one column
    }
    // Now, the delimList is fully populated with every possible delimiter and how many times it came up in each row
    // If a character comes up twice in the second row and five times in the third, it is not a suitable delimiter. Remove those.
    for (int delimpos = 0; delimpos < delimList.size(); delimpos++)
    {
        //cout << "Looking at possible delim character: " << delimList[delimpos].getChara() << endl;
        int lastRowOccrs = delimList[delimpos].getOccurrences()[0];
        //cout << "\tOccurrs " << lastRowOccrs << " times in the last row." << endl;
        bool allMatched = true;
        int initInd = 1;
        vector <int> occurrenceList = delimList[delimpos].getOccurrences();
        //cout << "There are " << occurrenceList.size() << " rows to work through:" << endl;
        //for (int k = 0; k < occurrenceList.size(); k++)
        //{
            //cout << occurrenceList[k] << endl;
        //}
        while (initInd < occurrenceList.size())
        {
            // will end the loop and set allMatched to false if any element in the occurrences list of the Delimiter does not match the first one
            if (occurrenceList[initInd] != lastRowOccrs)
            {
                //cout << "\t" << occurrenceList[initInd] << " does not match " << lastRowOccrs << endl;
                allMatched = false;
                initInd = occurrenceList.size() + 1;
            }
            //else { cout << "\t" << occurrenceList[initInd] << " matches " << lastRowOccrs << endl; }
            initInd++;
        }
        if (! allMatched)
        {
            //cout << "\tAdded index " << delimpos << " to the bad list." << endl;
            badInds.push_back(delimpos);
        }
    } // end of for loop
    // Now, the vector called badInds contains the indices (in delimList) of the characters that cannot possibly be the delimiter
    // Next, purge delimList of any bad delims -- the ones whose indices are indicated in badInds
    // Count backwards, to avoid indices shifting when you erase stuff.
    for (int q = badInds.size(); q > 0; q--)
    {
        //cout << "Removing character " << delimList[q].getChara() << " from consideration." << endl;
        delimList.erase(delimList.begin() + q);
    }
    // delimList now contains all valid, possible delimiter characters.
    // If there is only one valid delim left, end the function and set the delimiter.
    if (delimList.size() == 1)
    {
        callingReader.delim = lexical_cast<string>(delimList[0].getChara());
        int numDelimsPerRow = delimList[0].getOccurrences()[0];
        callingReader.numCols = numDelimsPerRow + 1;
        return 1;
    }
    // If there are no valid delimiters, the data contains only one column.
    if (delimList.size() == 0)
    {
        cerr << "No valid delimiters found for file " << callingReader.getFilename() << endl;
        cerr << "The file may contain only one column. If that is not the case, then the data may be incorrectly formatted." << endl;
        callingReader.numCols = 0;
        return 0;
        // don't actually change the delimiter in the TableReader object. It will stay as "NO_DELIMITER".
    }
    // If the function gets to this point (that is, has not yet returned anything), that means there are at least two characters that appear the same number of times in each line. The followign establishes an order of precedence.
    // Tabs get top priority:
    tuple<bool, int> isTabs = searchDelims('\t');
    if (get<0>(isTabs))
    {
        callingReader.delim = "\t";
        int numDelimsPerRow = delimList[get<1>(isTabs)].getOccurrences()[0];
        callingReader.numCols = numDelimsPerRow + 1;
        return 2;
    }
    // If there are no tabs, the next priority is
    // If there are two possibilities, and one occurs once per line, and another occurs three or more times per line, the one that occurs once is probably not the delim.
    if (delimList.size() == 2 && delimList[0].getOccurrences()[0] == 1 && delimList[1].getOccurrences()[0] > 2 )
    {
        callingReader.delim = lexical_cast<string>(delimList[1].getChara());
        int numDelimsPerRow = delimList[1].getOccurrences()[0];
        callingReader.numCols = numDelimsPerRow + 1;
        return 2;
    }
    else if (delimList.size() == 2 && delimList[0].getOccurrences()[1] == 1 && delimList[0].getOccurrences()[0] > 2 )
    {
        callingReader.delim = lexical_cast<string>(delimList[0].getChara());
        int numDelimsPerRow = delimList[0].getOccurrences()[0];
        callingReader.numCols = numDelimsPerRow + 1;
        return 2;
    }
    // If there are two possiblities, and if each occurs more than once, AND one occurs one more time than the other, the one that occurs one less time is probably the delimiter
    // This corresponds to the one that occurs the most being part of the entry in each cell
    if (delimList.size() == 2 && delimList[0].getOccurrences()[0] > 1 && delimList[1].getOccurrences()[0] > 1 && (delimList[0].getOccurrences()[0] == delimList[1].getOccurrences()[0] + 1) )
    {
        callingReader.delim = lexical_cast<string>(delimList[0].getChara());
        int numDelimsPerRow = delimList[0].getOccurrences()[0];
        callingReader.numCols = numDelimsPerRow + 1;
        return 2;
    }
    else if (delimList.size() == 2 && delimList[0].getOccurrences()[0] > 1 && delimList[1].getOccurrences()[0] > 1 && (delimList[1].getOccurrences()[0] == delimList[0].getOccurrences()[0] + 1) )
    {
        callingReader.delim = lexical_cast<string>(delimList[1].getChara());
        int numDelimsPerRow = delimList[1].getOccurrences()[0];
        callingReader.numCols = numDelimsPerRow + 1;
        return 2;
    }
    // Quote-surrounded fields:
    // If the character that occurs the most is double quotes, AND those quotes occur at least four times, AND there is another character that occurs (N/2) - 1 times (where N is how many times the quotes appear), the other character is probably the delimiter.
    vector<int> charOccs;
    for (int u = 0; u < delimList.size(); u++)
    {
        charOccs.push_back(delimList[u].getOccurrences()[0]);
    }
    int maxInd = 0;
    for (int g = 1; g < charOccs.size(); g++)
    {
        if (charOccs[g] > charOccs[g-1])
        {
            maxInd = g;
        }
    }
    // Now charOccs contains a list of how many occurrences per line there are for each possible delimiter character
    // while maxInd is the index of the one with the most occurrences
    int maxOccurs = delimList[maxInd].getOccurrences()[0];
    if (delimList[maxInd].getChara() == '\"' && maxOccurs > 3 && maxOccurs % 2 == 0)
    {
        // seerch for a character that occurs (N/2)-1 times
        int quoteOccs = delimList[maxInd].getOccurrences()[0];
        for (int y = 0; y < delimList.size();)
        {
            if (charOccs[y] == (quoteOccs/2) - 1)
            {
                // the character at position y will be set as the delimiter
                callingReader.delim = lexical_cast<string>(delimList[y].getChara());
                int numDelimsPerRow = delimList[y].getOccurrences()[0];
                callingReader.numCols = numDelimsPerRow + 1;
                y = delimList.size() + 1;
                return 2;
            } // end of if block -- what to do if the character is the right delimiter for a quote-surrounded list
            else { y++; } // end of what to do if the character is not the right fit
        } // end of for loop
    } // end of if block -- what to do if the max-occurring character is double quotes
    // Need to do the same check, but for single quotes
    if (delimList[maxInd].getChara() == '\'' && delimList[maxInd].getOccurrences()[0] > 3 && delimList[maxInd].getOccurrences()[0] % 2 == 0)
    {
        // seerch for a character that occurs (N/2)-1 times
        int quoteOccs = delimList[maxInd].getOccurrences()[0];
        for (int y = 0; y < delimList.size();)
        {
            if (charOccs[y] == (quoteOccs/2) - 1)
            {
                // the character at position y will be set as the delimiter
                callingReader.delim = lexical_cast<string>(delimList[y].getChara());
                int numDelimsPerRow = delimList[y].getOccurrences()[0];
                callingReader.numCols = numDelimsPerRow + 1;
                y = delimList.size() + 1;
                return 2;
            } // end of if block -- what to do if the character is the right delimiter for a quote-surrounded list
            else { y++; } // end of what to do if the character is not the right fit
        } // end of for loop
    } // end of if block -- what to do if the max-occurring character is single quotes
    // Commas get priority over periods
    tuple<bool, int> isPers = searchDelims('.');
    tuple<bool, int> isComs = searhDelims(',');
    if (get<0>isPers && get<0>isComs)
    {
        callingReader.delim = ",";
        int numDelimsPerRow = delimList[get<1>(isComs)].getOccurrences()[0];
        callingReader.numCols = numDelimsPerRow + 1;
        return 2;
    }

    // Okay, if you've gotten all the way to here and you still haven't returned a value, time to do the defaul behavior:
    // select the possible delim that occurs the most as the delimiter
    callingReader.delim = lexical_cast<string>(delimList[maxInd].getChara());
    int numDelimsPerRow = delimList[maxInd].getOccurrences()[0];
    callingReader.numCols = numDelimsPerRow + 1;
    return 2;
}

// PROTECTED METHODS. Only used for helping other methods.

void DelimFinder::findPossibleDelims()
{
    int lastRowInd = callingReader.getNumRawRows() - 1;
    //cout << "The index of the last row is " << lastRowInd << endl;
    // populate lastRowCharas with all the nonletter, nonnumber characters in the last row.
    // at the same time, count how many times each character has appeared, and add that to the appropriate spot in numCharas.
    string baseRow = callingReader.getRawRow(lastRowInd);
    //cout << "The base row for this delimiter search consists of " << baseRow << endl;

    for (int iter = 0; iter < baseRow.length(); iter++)
    {
        char thisPiece = baseRow[iter];
        //cout << "Iterated to spot " << iter << " in the line, which holds the character " << thisPiece << endl;
        //const char* cstr_piece = new char(thisPiece);
        string strPiece = boost::lexical_cast<string>(thisPiece);
        //cout << "As a C++ style string, it is seen as " << strPiece << endl;
        // if the character is neither a letter nor a number, perform the next check:
        const char* exp = "[^a-zA-Z0-9]";
        boost::regex expression(exp);
        smatch what;
        if (boost::regex_search(strPiece, what, expression ))
        {
            //cout << "The character is neither a letter nor a number." << endl;
            // the chara isn't a letter or a number. see if it's already been added to lastRowCharas:
            tuple<bool, int> search_results = searchDelims(thisPiece);
            if ( get<0>(search_results) ) // if the result of the search is true
            {
                delimList[get<1>(search_results)].addInstance(0); // add a count to the number of occurrences
            }
            // If you didn't find a match, however:
            else // add a new delimiter to the list
            {
                Delimiter newDelim(thisPiece);
                delimList.push_back(newDelim);
                // sort the list, so that it's in order for the next search
                sort(delimList.begin(), delimList.end());
            }
        } // end of if statement that executes if the character is neither a letter or a number
        // if the character is either a letter or a number, do nothing
    }// end of for loop iterating through base row

}

void DelimFinder::countDelimOccurrences()
{
    // if there are no possible delimiter characters listed in lastRowCharas, throw an error.
    //  This will be caught in guessDelim(), but it keeps the rest of the function from carrying on to no purpose.
    if (delimList.size() < 1)
    {
        throw NoDelimsError(callingReader.getFilename());
    }

    // establish how many rows of data will be searched for possible delimiters
    int maxRows = 1;
    if (usedTruncatedTable)
    {
        maxRows = 98;
    }
    else
    {
        maxRows = callingReader.getNumRawRows() - 1;
        //cout << "Total rows: " << maxRows << endl;
    }
    // add that number of entries to the occurrences list for each delimiter
    for (int h = 0; h < delimList.size(); h++)
    {
        delimList[h].lengthenList(maxRows);
    }
    //cout << "Number of rows to be looked at: " << maxRows << endl;
    // go through each line of data, starting at the first
    for (int iter = 0; iter < maxRows; iter++)
    {
        string currentRow = callingReader.getRawRow(iter);
        //cout << "Analyzing current row: " << currentRow << "\t";
        // iterate through every character in the row
        for (int charpos = 0; charpos < currentRow.size(); charpos++)
        {
            char currentChar = currentRow[charpos];
            //cout << "On character: " << currentChar << " ... ";
            tuple<bool, int> searchRes = searchDelims(currentChar);
            if ( get<0>(searchRes) )
            {
                // if the current char matches one already found, add an occurrence for that char
                int foundInd = get<1>(searchRes);
                delimList[foundInd].addInstance(iter+1);
                //cout << "Got a match with " << delimList[foundInd].getChara() << endl;

            }
            // do nothing if the char isn't found
            //else {cout << "no match" << endl;}
        }// end of for loop iterating through the characters in a single row
    } // end of for loop iterating through the rows of data
}

// a simple binary search algorithm, speeding up sorting through an ordered list.
tuple<bool, int> DelimFinder::searchDelims(char inChar)
{
    int mid;
    int high = delimList.size();
    int low = 0;
    while (low < high)
    {
        mid = (int)(high + low)/2;
        if (delimList[mid] < inChar)
        {
            low = mid + 1;
        }
        else if (delimList[mid] > inChar)
        {
            high = mid;
        }
        else
        {
            tuple<bool, int> result(true, mid);
            return result;
        }
    }
    tuple<bool, int> result(false, mid);
    return result;
}

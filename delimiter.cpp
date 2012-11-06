#include "tableReader.h"

Delimiter::Delimiter(char inDelim) : delimChar(inDelim)
{
    occurrences.push_back(1);
}

Delimiter::~Delimiter()
{
    // nothing here
}

void Delimiter::addInstance(int row)
{
    if (row + 1 > occurrences.size())
    {
        // if adding to a row which does not exist, add more rows till you get to it
        int difference = row + 1 - occurrences.size();
        for (int i = 0; i < difference; i++)
        {
            occurrences.push_back(0);
        }
    }

    occurrences[row] += 1;
    //cout << "Added an instance of " << delimChar << " to row " << row << endl;
}

char Delimiter::getChara()
{
    return delimChar;
}

vector<int> Delimiter::getOccurrences()
{
    return occurrences;
}

void Delimiter::lengthenList(int rows)
{
    for (int t = 0; t < rows; t++)
    {
        occurrences.push_back(0);
    }
}

// friend functions for comparison
bool operator>(const Delimiter& lhs, const Delimiter& rhs)
{
    return (lhs.delimChar > rhs.delimChar);
}

bool operator<(const Delimiter& lhs, const Delimiter& rhs)
{
    return (lhs.delimChar < rhs.delimChar);
}

bool operator==(const Delimiter& lhs, const Delimiter& rhs)
{
    return (lhs.delimChar == rhs.delimChar);
}

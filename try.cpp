#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>
#include <sys/stat.h>

using namespace std;

// Constants
const int HASH_BASE = 257;       // Base for rolling hash
const int HASH_MOD = 1000000007; // Modulus for hash computation

// Function to compute a rolling hash for a sequence of items
size_t rollingHash(const vector<int> &sequence, int base, int mod)
{
    size_t hashValue = 0;
    for (int item : sequence)
    {
        hashValue = (hashValue * base + item) % mod;
    }
    return hashValue;
}

// Function to read transactions from a file
vector<string> readTransactions(const string &filename)
{
    vector<string> dataset;
    ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        cerr << "Error opening file" << endl;
        return dataset;
    }

    string line;
    while (getline(inputFile, line))
    {
        dataset.push_back(line);
    }
    inputFile.close();
    return dataset;
}

// Function to identify sequences of a given length and populate the sequence map
void identifySequences(const vector<string> &dataset, unordered_map<string, int> &sequenceMap, int sequenceLength, int &sequenceIndex)
{
    for (const string &line : dataset)
    {
        stringstream ss(line);
        vector<int> transaction;
        int itemId;

        // Read transaction items into a vector
        while (ss >> itemId)
        {
            transaction.push_back(itemId);
        }

        // Process sequences of the given length
        for (size_t i = 0; i + sequenceLength <= transaction.size(); ++i)
        {
            vector<int> sequence(transaction.begin() + i, transaction.begin() + i + sequenceLength);

            // Convert sequence to a string for mapping
            string sequenceStr;
            for (int item : sequence)
            {
                sequenceStr += to_string(item) + ",";
            }

            // Check if sequence is already in the map
            if (sequenceMap.find(sequenceStr) == sequenceMap.end())
            {
                sequenceMap[sequenceStr] = sequenceIndex++;
            }
        }
    }
}

// Function to replace sequences in transactions with indices and track used sequences
void replaceSequences(vector<string> &dataset, const unordered_map<string, int> &sequenceMap, int sequenceLength, unordered_set<string> &usedSequences)
{
    for (string &transaction : dataset)
    {
        stringstream ss(transaction);
        vector<int> items;
        int itemId;

        while (ss >> itemId)
        {
            items.push_back(itemId);
        }

        stringstream newTransaction;
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (i + sequenceLength <= items.size())
            {
                vector<int> sequence(items.begin() + i, items.begin() + i + sequenceLength);
                string sequenceStr;
                for (int item : sequence)
                {
                    sequenceStr += to_string(item) + ",";
                }

                if (sequenceMap.find(sequenceStr) != sequenceMap.end())
                {
                    newTransaction << sequenceMap.at(sequenceStr) << " ";
                    usedSequences.insert(sequenceStr); // Mark this sequence as used
                    i += sequenceLength - 1;           // Skip ahead by the length of the sequence
                }
                else
                {
                    newTransaction << items[i] << " ";
                }
            }
            else
            {
                newTransaction << items[i] << " ";
            }
        }

        transaction = newTransaction.str();
    }
}

// Function to calculate and display size reduction analysis
void analyzeSizeReduction(const string &originalFilename, const string &mapFilename, const string &updatedFilename)
{
    struct stat stat_buf;
    size_t originalSize = stat(originalFilename.c_str(), &stat_buf) == 0 ? stat_buf.st_size : 0;
    size_t mapSize = stat(mapFilename.c_str(), &stat_buf) == 0 ? stat_buf.st_size : 0;
    size_t updatedSize = stat(updatedFilename.c_str(), &stat_buf) == 0 ? stat_buf.st_size : 0;

    size_t reducedSize = mapSize + updatedSize;

    cout << "Original size: " << originalSize << " bytes" << endl;
    cout << "Reduced size: " << reducedSize << " bytes" << endl;
    cout << "Size reduction: " << (originalSize - reducedSize) << " bytes" << endl;
    cout << "Reduction percentage: " << (static_cast<double>(originalSize - reducedSize) / originalSize) * 100 << "%" << endl;
}

// Function to save sequence map to a file without special characters
void saveSequenceMap(const unordered_map<string, int> &sequenceMap, const string &filename)
{
    ofstream outputFile(filename);
    if (!outputFile.is_open())
    {
        cerr << "Error opening file for writing sequence map" << endl;
        return;
    }

    for (const auto &seq_idx : sequenceMap)
    {
        outputFile << seq_idx.first << " " << seq_idx.second << endl; // Write key followed by value with space
    }
    outputFile.close();
}

// Function to save updated dataset to a file
void saveUpdatedDataset(const vector<string> &dataset, const string &filename)
{
    ofstream outputFile(filename);
    if (!outputFile.is_open())
    {
        cerr << "Error opening file for writing updated dataset" << endl;
        return;
    }

    for (const auto &transaction : dataset)
    {
        outputFile << transaction << endl;
    }
    outputFile.close();
}

// Function to remove unused sequences from the map
void removeUnusedSequences(unordered_map<string, int> &sequenceMap, const unordered_set<string> &usedSequences)
{
    for (auto it = sequenceMap.begin(); it != sequenceMap.end();)
    {
        if (usedSequences.find(it->first) == usedSequences.end())
        {
            it = sequenceMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

int main()
{
    // Read transactions from file
    vector<string> dataset = readTransactions("D_small.dat");

    if (dataset.empty())
    {
        return 1;
    }

    // Identify sequences in transactions for lengths 5, 4, 3
    unordered_map<string, int> sequenceMap;
    unordered_set<string> usedSequences;
    int sequenceIndex = 0;

    identifySequences(dataset, sequenceMap, 5, sequenceIndex);
    identifySequences(dataset, sequenceMap, 4, sequenceIndex);
    identifySequences(dataset, sequenceMap, 3, sequenceIndex);

    // Replace sequences in transactions for lengths 5, 4, 3
    replaceSequences(dataset, sequenceMap, 5, usedSequences);
    replaceSequences(dataset, sequenceMap, 4, usedSequences);
    replaceSequences(dataset, sequenceMap, 3, usedSequences);

    // Remove unused sequences from the map
    removeUnusedSequences(sequenceMap, usedSequences);

    // Save the sequence map and updated dataset to files
    saveSequenceMap(sequenceMap, "sequence_map.txt");
    saveUpdatedDataset(dataset, "updated_transactions.txt");

    // Perform size reduction analysis
    analyzeSizeReduction("D_small.dat", "sequence_map.txt", "updated_transactions.txt");

    return 0;
}
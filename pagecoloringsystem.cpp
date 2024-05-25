#include <iostream>
#include <bitset>
#include <vector>
#include <unordered_map>
#include <cmath> 

using namespace std;

const int CACHE_SIZE = 1024;
const int BLOCK_SIZE = 16;
const int NUM_COLORS = 4;

struct CacheLine
{
    int tag;
    bool valid;
    int data[4];
    int color;
};

vector<CacheLine> cache(CACHE_SIZE / BLOCK_SIZE);
unordered_map<int, int> pageToColor;

int getCacheIndex(unsigned long address)
{
    int offsetBits = floor(log2(BLOCK_SIZE)); 
    int indexBits = floor(log2((CACHE_SIZE / BLOCK_SIZE) / NUM_COLORS));

    unsigned long tag = address >> (offsetBits + indexBits);
    unsigned long pageIndex = (address >> offsetBits) >> indexBits;

    int color = pageToColor.count(pageIndex) ? pageToColor[pageIndex] : (pageToColor[pageIndex] = rand() % NUM_COLORS);

    int index = (pageIndex * NUM_COLORS + color) % (CACHE_SIZE / BLOCK_SIZE);

    cout << "Address: " << hex << address << " (Page: " << pageIndex << ", Color: " << color << ")" << endl;
    cout << "Tag: " << hex << tag << endl;
    cout << "Index (colored): " << index << endl;
    return index;
}

// Simulate a cache access
void accessCache(unsigned long address)
{
    int index = getCacheIndex(address);

    int shiftAmount = static_cast<int>(floor(log2(BLOCK_SIZE)) + floor(log2((CACHE_SIZE / BLOCK_SIZE) / NUM_COLORS)));

    if (cache[index].valid && cache[index].tag == (address >> shiftAmount))
    {
        cout << "Cache Hit!" << endl
             << endl;
    }
    else
    {
        cout << "Cache Miss!" << endl;
        cout << "Cache will now load data form Main Memory" << endl
             << endl;
        cache[index].tag = address >> shiftAmount; 
        cache[index].valid = true;
    }
}

int main()
{
    
    cout << "==================================================================" << endl;
    accessCache(0x0000012C);
    accessCache(0x00002120); 
    accessCache(0x0000212C); 

    return 0;
}

#include <iostream>
#include <bitset>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <mutex>
#include <thread>
#include <random>   // For random number generation

using namespace std;

const int CACHE_SIZE = 1024;
const int BLOCK_SIZE = 16;
const int NUM_COLORS = 4;

struct CacheLine {
    int tag;
    bool valid;
    int data[4];
    int color;
};

vector<CacheLine> cache(CACHE_SIZE / BLOCK_SIZE);
unordered_map<int, int> pageToColor;
mutex cacheMutex; // Global mutex to protect cache accesses

// Thread-local random number generators
thread_local static mt19937 generator(random_device{}());
thread_local static uniform_int_distribution<int> distribution(0, NUM_COLORS - 1);

int getCacheIndex(unsigned long address) {
    int offsetBits = floor(log2(BLOCK_SIZE));
    int indexBits = floor(log2((CACHE_SIZE / BLOCK_SIZE) / NUM_COLORS));

    unsigned long tag = address >> (offsetBits + indexBits);
    unsigned long pageIndex = (address >> offsetBits) >> indexBits;

    int color;
    { // Scope for locking to ensure thread-safety
        lock_guard<mutex> lock(cacheMutex);
        color = pageToColor.count(pageIndex) 
                    ? pageToColor[pageIndex] 
                    : (pageToColor[pageIndex] = distribution(generator)); // Thread-local random color
    }

    int index = (pageIndex * NUM_COLORS + color) % (CACHE_SIZE / BLOCK_SIZE);

    cout << "Address: " << hex << address << " (Page: " << pageIndex << ", Color: " << color << ")" << endl;
    cout << "Tag: " << hex << tag << endl;
    cout << "Index (colored): " << index << endl;
    return index;
}

void accessCache(unsigned long address) {
    int index = getCacheIndex(address);

    lock_guard<mutex> lock(cacheMutex); // Lock cache access

    int shiftAmount = static_cast<int>(floor(log2(BLOCK_SIZE)) + floor(log2((CACHE_SIZE / BLOCK_SIZE) / NUM_COLORS)));

    if (cache[index].valid && cache[index].tag == (address >> shiftAmount)) {
        cout << "Cache Hit!" << endl << endl;
    } else {
        cout << "Cache Miss!" << endl;
        cout << "Cache will now load data from Main Memory" << endl << endl;
        cache[index].tag = address >> shiftAmount; 
        cache[index].valid = true;
    }
}
void simulateThread(int threadId, const vector<unsigned long>& addresses) {
    for (unsigned long address : addresses) {
        cout << "Thread " << threadId << " accessing: " << hex << address << endl;
        accessCache(address);
    }
}


int main() {
    // Example addresses for each thread (you can modify these)
    vector<unsigned long> addressesThread1 = {0x0000012C, 0x0000212C, 0x0000412C};
    vector<unsigned long> addressesThread2 = {0x0000012D, 0x0000212D, 0x0000412D};
    vector<unsigned long> addressesThread3 = {0x0000012E, 0x0000212E, 0x0000412E};
    vector<unsigned long> addressesThread4 = {0x0000012F, 0x0000212F, 0x0000412F};

    cout << "==================================================================" << endl;

    // Create and run threads
    thread thread1(simulateThread, 1, addressesThread1);
    thread thread2(simulateThread, 2, addressesThread2);
    thread thread3(simulateThread, 3, addressesThread3);
    thread thread4(simulateThread, 4, addressesThread4);

    // Wait for threads to finish
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    cout << "==================================================================" << endl;

    return 0;
}

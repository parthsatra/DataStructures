#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

void printOutput(int *seek, int *hit, int size, long sec, long usec) {
    for (int i = 0; i < size; i++) {
        if (hit[i] == 1) {
            printf("%12d: Yes\n", seek[i]);
        } else {
            printf("%12d: No\n", seek[i]);
        }
    }
    printf("Time: %ld.%06ld", sec, usec);
}

streampos readSeekFile(char *seekFileName, char **seekBuffer) {
    ifstream seekFile(seekFileName, ios::in | ios::binary | ios::ate);
    if (seekFile.is_open()) {
        streampos seekSize = seekFile.tellg();
        *seekBuffer = new char[seekSize];
        seekFile.seekg(0, ios::beg);
        seekFile.read(*seekBuffer, seekSize);
        seekFile.close();
        return seekSize;
    }
    else {
        cout << "Seek File not Found" << endl;
        exit(0);
    }
}

void initializeArray(int **hit, int size) {
    *hit = new int[size];
    for (int i = 0; i < size; i++) {
        (*hit)[i] = 0;
    }
}

void doLinearSearchInMemory(char *keyFileName, char *seekFileName) {
    char *seekBuffer;
    int *s;
    streampos seekSize;
    struct timeval start_tm;
    struct timeval end_tm;
    int *hit;
    char *keyBuffer;
    int *k;
    streampos keySize;

    // Reading Seek seekFile
    seekSize = readSeekFile(seekFileName, &seekBuffer);
    s = (int *) seekBuffer;

    initializeArray(&hit, seekSize / sizeof(int));

    // Reading Key seekFile
    gettimeofday(&start_tm, NULL);
    ifstream keyFile(keyFileName, ios::in | ios::binary | ios::ate);
    if (keyFile.is_open()) {
        keySize = keyFile.tellg();
        keyBuffer = new char[keySize];
        keyFile.seekg(0, ios::beg);
        keyFile.read(keyBuffer, keySize);
        keyFile.close();
        k = (int *) keyBuffer;
    } else {
        cout << "Key file not found" << endl;
        exit(0);
    }

    // Perform Linear Search
    for (int i = 0; i < seekSize / sizeof(int); i++) {
        for (int j = 0; j < keySize / sizeof(int); j++) {
            if (k[j] == s[i]) {
                hit[i] = 1;
                break;
            }
        }
    }

    gettimeofday(&end_tm, NULL);

    long sec = end_tm.tv_sec - start_tm.tv_sec;
    long usec = end_tm.tv_usec - start_tm.tv_usec;
    if (usec < 0) {
        usec = 1000000 + usec;
        sec = sec - 1;
    }
    printOutput(s, hit, seekSize / sizeof(int), sec, usec);

    delete[] s;
    delete[] k;
    delete[] hit;
}

void doBinarySearchInMemory(char *keyFileName, char *seekFileName) {
    char *seekBuffer;
    int *s;
    streampos seekSize;
    int *hit;
    char *keyBuffer;
    int *k;
    streampos keySize;
    struct timeval start_tm;
    struct timeval end_tm;

    // Reading Seek seekFile
    seekSize = readSeekFile(seekFileName, &seekBuffer);
    s = (int *) seekBuffer;

    initializeArray(&hit, seekSize / sizeof(int));

    // Reading Key seekFile
    gettimeofday(&start_tm, NULL);
    ifstream keyFile(keyFileName, ios::in | ios::binary | ios::ate);
    if (keyFile.is_open()) {
        keySize = keyFile.tellg();
        keyBuffer = new char[keySize];
        keyFile.seekg(0, ios::beg);
        keyFile.read(keyBuffer, keySize);
        keyFile.close();
        k = (int *) keyBuffer;
    } else {
        cout << "Key file not found" << endl;
        exit(0);
    }

    // Perform Binary Search for each Element
    for (int i = 0; i < seekSize / sizeof(int); i++) {
        // Doing Binary Search
        int low = 0;
        int high = keySize / sizeof(int);
        while (low <= high) {
            int mid = (high + low) / 2;
            if (s[i] == k[mid]) {
                hit[i] = 1;
                break;
            } else if (s[i] > k[mid]) {
                low = mid + 1;
            } else if (s[i] < k[mid]) {
                high = mid - 1;
            }
        }
    }
    gettimeofday(&end_tm, NULL);

    long sec = end_tm.tv_sec - start_tm.tv_sec;
    long usec = end_tm.tv_usec - start_tm.tv_usec;
    if (usec < 0) {
        usec = 1000000 + usec;
        sec = sec - 1;
    }
    printOutput(s, hit, seekSize / sizeof(int), sec, usec);

    delete[] s;
    delete[] k;
    delete[] hit;
}

void doLinearSearchOnDisk(char *keyFileName, char *seekFileName) {
    // Reading Seek seekFile
    char *seekBuffer;
    int *s;
    streampos seekSize;
    int *hit;
    struct timeval start_tm;
    struct timeval end_tm;

    seekSize = readSeekFile(seekFileName, &seekBuffer);
    s = (int *) seekBuffer;

    // Initialize hit array
    initializeArray(&hit, seekSize / sizeof(int));

    // Reading Key seekFile
    char *keyBuffer = new char[sizeof(int) / sizeof(char)];
    int *k;
    streampos intSize = sizeof(int);

    gettimeofday(&start_tm, NULL);
    ifstream keyFile(keyFileName, ios::in | ios::binary | ios::ate);
    if (keyFile.is_open()) {
        for (int i = 0; i < seekSize / sizeof(int); i++) {
            keyFile.seekg(0, ios::beg);
            while (!keyFile.eof()) {
                keyFile.read(keyBuffer, intSize);
                k = (int *) keyBuffer;
                if (k[0] == s[i]) {
                    hit[i] = 1;
                    break;
                }
            }
            // Clearing the EOF flag.
            keyFile.clear();
        }
        keyFile.close();
    } else {
        cout << "Key file not found" << endl;
        exit(0);
    }
    gettimeofday(&end_tm, NULL);

    long sec = end_tm.tv_sec - start_tm.tv_sec;
    long usec = end_tm.tv_usec - start_tm.tv_usec;
    if (usec < 0) {
        usec = 1000000 + usec;
        sec = sec - 1;
    }
    printOutput(s, hit, seekSize / sizeof(int), sec, usec);

    delete[] s;
    delete[] k;
    delete[] hit;
}

void doBinarySearchOnDisk(char *keyFileName, char *seekFileName) {
    char *seekBuffer;
    int *s;
    streampos seekSize;
    int *hit;
    struct timeval start_tm;
    struct timeval end_tm;
    char *keyBuffer = new char[sizeof(int) / sizeof(char)];
    int *k;
    streampos keySize;
    streampos intSize = sizeof(int);

    // Reading Seek seekFile
    seekSize = readSeekFile(seekFileName, &seekBuffer);
    s = (int *) seekBuffer;

    initializeArray(&hit, seekSize / sizeof(int));

    // Reading Key seekFile
    gettimeofday(&start_tm, NULL);
    ifstream keyFile(keyFileName, ios::in | ios::binary | ios::ate);
    if (keyFile.is_open()) {
        keySize = keyFile.tellg();
        for (int i = 0; i < seekSize / sizeof(int); i++) {
            int low = 0;
            int high = keySize / sizeof(int);
            while (low <= high) {
                int mid = (low + high) / 2;
                keyFile.seekg(mid * sizeof(int), ios::beg);
                keyFile.read(keyBuffer, intSize);
                k = (int *) keyBuffer;
                if (k[0] == s[i]) {
                    hit[i] = 1;
                    break;
                } else if (k[0] < s[i]) {
                    low = mid + 1;
                } else if (k[0] > s[i]) {
                    high = mid - 1;
                }
            }
            // Clearing the EOF flag.
            keyFile.clear();
        }
        keyFile.close();
    } else {
        cout << "Key file not found" << endl;
        exit(0);
    }
    gettimeofday(&end_tm, NULL);

    long sec = end_tm.tv_sec - start_tm.tv_sec;
    long usec = end_tm.tv_usec - start_tm.tv_usec;
    if (usec < 0) {
        usec = 1000000 + usec;
        sec = sec - 1;
    }
    printOutput(s, hit, seekSize / sizeof(int), sec, usec);

    delete[] s;
    delete[] keyBuffer;
    delete[] hit;
}

int main(int argc, char *argv[]) {
    if (argc == 4) {
        char *searchMode = argv[1];
        char *keyFileName = argv[2];
        char *seekFileName = argv[3];
        if (!strcmp("--mem-lin", searchMode)) {
            doLinearSearchInMemory(keyFileName, seekFileName);
        } else if (!strcmp("--mem-bin", searchMode)) {
            doBinarySearchInMemory(keyFileName, seekFileName);
        } else if (!strcmp("--disk-lin", searchMode)) {
            doLinearSearchOnDisk(keyFileName, seekFileName);
        } else if (!strcmp("--disk-bin", searchMode)) {
            doBinarySearchOnDisk(keyFileName, seekFileName);
        }
    } else {
        cout << "Invalid parameter list passed to the program";
        exit(0);
    }
    return 0;
}
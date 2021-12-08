#include <stdio.h>
#include <stdlib.h>
#include "cachelab.h"
#include <getopt.h>
#include <math.h>
#include <strings.h>

typedef unsigned long long int address;
typedef int bool;

#define TRUE 1
#define FALSE 0

struct Parameter {
    int S, s, E, b, B;
};
struct cacheResult {
    int e, m, h;
};
struct Line {
    address tag;
    bool valid;
    int used;
    char* block;
};
struct Set {
    struct Line* lines;
};
struct Cache {
    struct Set* sets;
};

void printUsage();
struct Cache cacheInit(int lineNum, long long blockSize, long long setNum, struct Cache C);
int indexRet(int* used, int lineNum, struct Set tempSet, bool empty);
struct cacheResult runCache(struct Cache cache, struct cacheResult result, address addr, int lineNum, int temp, int b);

int main(int argc, char* argv[]) {
    struct cacheResult result, resultS, resultM, resultL, resultBuffer;
    result.e = 0;
    result.h = 0;
    result.m = 0;
    struct Cache cache;
    address addr;
    struct Parameter parameter;
    FILE *traceIn;
    char buffer, type;
    int verbosity = 0;
    int size = 0;
    int temp = 0;
    long long setNum = 0;
    long long blockSize = 0;

    while ((buffer = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch(buffer) {
            case 's': temp = atoi(optarg); break;
            case 'E': parameter.E = atoi(optarg); break;
            case 'b': parameter.b = atoi(optarg); break;
            case 't': traceIn = fopen(optarg, "r"); break;
            case 'v': verbosity = TRUE; break;
            default:
            case 'h': printUsage(); exit(1);
        }
    }
    if (temp == 0 || parameter.E == 0 || parameter.b == 0 || traceIn == 0) {
        printf("ERROR - Incorrect parameters entered\n");
        printUsage();
        exit(1);
    }

    setNum = pow(2, (float)temp);
    temp += parameter.b;
    cache = cacheInit(parameter.E, blockSize, setNum, cache);
    if (traceIn == 0) {
        printf("ERROR - Unable to load trace\n");
        exit(1);
    }
    if (traceIn != NULL) {
        while (fscanf(traceIn, " %c %llx,%d", &type, &addr, &size) == 3) {
            if (type != 'I') {
                if (verbosity == TRUE) {
                    resultBuffer = runCache(cache, resultBuffer, addr, parameter.E, temp, parameter.b);
                    switch(type) {
                        case 'M': resultM = resultBuffer; break;
                        case 'S': resultS = resultBuffer; break;
                        case 'L': resultL = resultBuffer; break;
                    }
                    result = resultBuffer;
                }
                else {
                    result = runCache(cache, result, addr, parameter.E, temp, parameter.b);
                }
            }
        }
    }

    if (verbosity == TRUE) {
        printf("Results for M: ");
        printSummary(resultM.h, resultM.m, resultM.e);
        printf("Results for S: ");
        printSummary(resultS.h, resultS.m, resultS.e);
        printf("Results for L: ");
        printSummary(resultL.h, resultL.m, resultL.e);
    }

    printSummary(result.h, result.m, result.e);
    fclose(traceIn);
    return 0;
} // main


void printUsage() {
    printf("Usage: ./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
} // printUsage


struct Cache cacheInit(int lineNum, long long blockSize, long long setNum, struct Cache C) {
    struct Set tempSet;
    struct Line tempLine;
    int i, j;
    C.sets = (struct Set*)malloc(sizeof(struct Set)*setNum);
    for (j = 0; j < setNum; j++) {
        tempSet.lines = (struct Line*)malloc(sizeof(struct Line)*lineNum);
        C.sets[j] = tempSet;
        for (i = 0; i < lineNum; i++) {
            tempLine.used = 0;
            tempLine.tag = 0;
            tempLine.valid = FALSE;
            tempSet.lines[i] = tempLine;
        }
    }
    return C;
} // cacheInit


int indexRet(int* used, int lineNum, struct Set tempSet, bool empty) {
    int i;
    int mostFreq = tempSet.lines[0].used;
    int leastFreq = mostFreq;
    int sizeBuffer, j = 0;
    for (i = 1; i < lineNum; i++) {
        sizeBuffer = tempSet.lines[i].used;
        if (mostFreq < sizeBuffer) {
            mostFreq = sizeBuffer;
        }
        if (leastFreq > sizeBuffer) {
            j = i;
            leastFreq = sizeBuffer;
        }
    }

    used[1] = mostFreq;
    used[0] = leastFreq;
    if (empty) {
        return j;
    }

    for (i = 0; i < lineNum; i++) {
        if (!tempSet.lines[i].valid) {
            return i;
        }
    }
    return 0;
} // indexRet


struct cacheResult runCache(struct Cache cache, struct cacheResult result, address addr, int lineNum, int temp, int b) {
    int i, j;
    int tagWidth = 64-temp;
    int missCheck = result.h;
    bool spaceCheck = TRUE;
    address tag = addr >> temp;
    int index = ((addr << tagWidth) >> (tagWidth + b));
    struct Set tempSet = cache.sets[index];

    for (i = 0; i < lineNum; i++) {
        struct Line tempLine = tempSet.lines[i];
        if (tempLine.valid) {
            if (tag == tempLine.tag) {
                tempLine.used += 1;
                tempSet.lines[i] = tempLine;
                result.h++;
            }
        }
        else if (!tempLine.valid && spaceCheck) {
            spaceCheck = FALSE;
        }
    }

    if (result.h == missCheck) {
        result.m++;
    }
    else {
        return result;
    }
    int* used = (int*)malloc(sizeof(int)*2);
    j = indexRet(used, lineNum, tempSet, spaceCheck);
    tempSet.lines[j].used = used[1] + 1;
    tempSet.lines[j].tag = tag;

    if (spaceCheck) {
        result.e++;
    }
    else {
        printf("Space Check\n");
        tempSet.lines[j].valid = TRUE;
    }
    return result;
} // cacheResult

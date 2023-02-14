#include "tema1.h"

#include <stdio.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <set>
#include <queue>
#include <map>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include "inparser.h"
#include "constants.h"

/**
 * @brief Extract the number of mapper and reducer threads from the arguments
 * 
 * @param argc num of arguments
 * @param argv the string list of arguments
 * @param numMappers return reference to number of mappers
 * @param numReducers return reference to number of reducers
 */
void get_args(int argc, char **argv, 
                int &numMappers, int &numReducers, char **file) {
    if (argc < 4) {
        printf("Not enough arguments: USAGE: ./tema1 <numar_mapperi> <numar_reduceri> <fisier_intrare>");
        exit(1);
    }
    numMappers = atoi(argv[1]);
    numReducers = atoi(argv[2]);
    *file = argv[3];
}

/**
 * @brief Read the file the describes the what other files to read and store
 * parser objects to that files in the filesToOpen vector
 * 
 * @param argumentFileName the name of the file extracted from the argument
 * @param filesToOpen the vector that will hold the parser objects
 */
std::vector<std::string> read_input_file(char *argumentFileName) {
    FILE *inputFile = fopen(argumentFileName, "r");
    char currentFileName[FILE_NAME_SIZE];
    int numFiles, i;

    std::vector<std::string> filesToOpen;

    // Go through every file in the list
    fscanf (inputFile, "%d", &numFiles);
    for (i = 0; i < numFiles; ++i) {
        fscanf (inputFile, "%s", currentFileName);
        filesToOpen.push_back(currentFileName);
    }

    fclose(inputFile);
    return filesToOpen;
}

void create_mapping_queue(std::vector<std::string> &files,
                            int numReducers,
                            std::unordered_set<uint> *completeSets,
                            std::queue<MapInformation *> &mappingQueue) {
    for (auto file : files) {
        MapInformation *info = new MapInformation;
        info->parser = new InParser(file.c_str());
        info->numReducers = numReducers;
        info->sets = completeSets;
        mappingQueue.push(info);
    }
}


/**
 * @brief Generate the power set table with all the perfect powers up to UINT_MAX
 * 
 * @param sets The hash table that will hold the perfect powers
 * @param numReducers  The number of reducers threads
 */
void perfect_power_set_generator(std::unordered_set<uint> *sets, int numReducers) {
    long long num, power, maxNumber = (1LL << 32) - 1;
    int ex;

    for (num = 2; num * num <= maxNumber; num++) {
        ex = 2;
        for(power = num * num; ex <= numReducers + 1 && power <= maxNumber; power*= num, ex++)
            sets[ex].insert(power);
    }
}

MapPhaseResult execute_map(MapInformation *info) {
    MapPhaseResult res;
    int i, n, x, ex;

    std::unordered_map<int, std::unordered_set<uint>> auxMap;

    InParser *parser = info->parser;
    int numReducers = info->numReducers;
    std::unordered_set<uint> *complete_sets = info->sets;

    *parser >> n;
    for (i = 0; i < n; ++i) {
        *parser >> x;
        for (ex = 2; ex <= numReducers + 1; ex++) {
            if (x == 1)
                auxMap[ex].insert(x);
            else if(complete_sets[ex].count(x))
                auxMap[ex].insert(x);
        }
    }
    // close parser and free memory
    parser->close_parser_and_free();
    delete parser;

    for (auto it = auxMap.begin(); it != auxMap.end(); ++it) {
        res.push_back(std::make_pair(it->first, it->second));
    }

    return res;
}

/**
 * @brief The function that will be executed by the threads
 * 
 * @param arg ThreadInformation struct that holds all the information needed
 * @return void* 
 */
void *func(void *arg) {
    ThreadInformation* information = (ThreadInformation *) arg;
    char outputFile[FILE_NAME_SIZE];

    if (information->type == 0) {
        mapping_generic(information->q, execute_map, information->mutex, information->map);
    }
    // Wait for the mapper threads to finish
    pthread_barrier_wait(information->barrier);

    if(information->type == 1) {
        int threadExponent = information->id - *(information->numMappers) + 2;
        int result = execute_reduce_task(information->map, threadExponent);
        // write to file
        sprintf(outputFile, "out%d.txt", threadExponent);
        FILE *out = fopen(outputFile, "w");
        fprintf(out, "%d", result);
        fclose(out);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int numMappers, numReducers;
    char *argumentFileName;
    // list of parsers to the input files
    std::queue<MapInformation *> mappingQueue;

    // get arguments and read input file
    get_args(argc, argv, numMappers, numReducers, &argumentFileName);

    // generate array of unordered_sets that will help when determing perfect power
    std::unordered_set<uint> complete_sets[numReducers + 2];
    perfect_power_set_generator(complete_sets, numReducers);

    auto files = read_input_file(argumentFileName);
    create_mapping_queue(files, numReducers, complete_sets, mappingQueue);

    auto *reduceInput = new std::map<int, MapKey>();

    //print_information_in_files(filesParsers);
    //solve(filesParsers, numMappers, numReducers);

    pthread_t threads[numMappers + numReducers];
    int i, returnCode;
    void *status;
    ThreadInformation tasks[numMappers + numReducers];

    // initialise barrier
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, numMappers + numReducers);\

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < numMappers + numReducers; ++i) {
        // initialise thread information
        tasks[i].id = i;
        if(i < numMappers)
            tasks[i].type = 0;
        else
            tasks[i].type = 1;

        tasks[i].barrier = &barrier;
        tasks[i].sets = complete_sets;
        tasks[i].q = &mappingQueue;
        tasks[i].numMappers = &numMappers;
        tasks[i].numReducers = &numReducers; 
        tasks[i].mutex = &mutex;
        tasks[i].map = reduceInput;

        returnCode = pthread_create(&threads[i], NULL, func, &tasks[i]);

        if (returnCode) {
            printf("Error creating thread #%d\n", i);
			exit(-1);
        }
    }

    // wait for all threads to finish
    for (i = 0; i < numMappers + numReducers; ++i) {
        returnCode = pthread_join(threads[i], &status);

        if (returnCode) {
            printf("Error joining thread #%d\n", i);
			exit(-1);
        }
    }

    // destroy mutex and barrier and free memory allocated on the heap
    delete reduceInput;
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);

    return 0;
}
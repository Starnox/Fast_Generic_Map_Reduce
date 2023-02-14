#pragma once
#include "mapreduce.h"
#include <string>


// This will be the KV from the map task
struct MapInformation {
    InParser *parser;
    int numReducers;
    std::unordered_set<uint> *sets;
};

typedef std::vector<std::pair<int, std::unordered_set<uint>>> MapPhaseResult;
typedef MapPhaseResult (*MapFunction)(MapInformation *info);

typedef struct ThreadInformation {
    u_int8_t type; // 0 for mapper / 1 for reducer
    int id;
    int *numMappers;
    int *numReducers;
    //InParser *parser;
    pthread_barrier_t *barrier;
    pthread_mutex_t *mutex;
    std::unordered_set<uint> *sets;
    std::queue<MapInformation *> *q;
    std::map<int, std::vector<std::unordered_set<uint>>> *map;

} ThreadInformation;


void get_args(int argc, char **argv, 
                int &numMappers, int &numReducers, char **file);
std::vector<std::string> read_input_file(char *argumentFileName);
void create_mapping_queue(std::vector<std::string> &files,
                            int numReducers,
                            std::unordered_set<uint> *completeSets,
                            std::queue<MapInformation *> &mappingQueue);
void perfect_power_set_generator(std::unordered_set<uint> *sets, int numReducers);
void *func(void *arg);

MapPhaseResult execute_map(MapInformation *info);
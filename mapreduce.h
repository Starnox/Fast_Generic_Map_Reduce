#pragma once
#include <stdio.h>
#include <vector>
#include <unordered_set>
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

typedef std::vector<std::unordered_set<uint>> MapKey;
typedef std::map<int, MapKey> MapResult;

void execute_map_task(
        InParser *parser,
        int numReducers,
        std::unordered_set<uint> complete_sets[],
        MapResult *result,
        pthread_mutex_t *mutex);

int execute_reduce_task(MapResult *result, int threadExponent);


/// @brief generic function that will take all (key, value) pairs from the queue, apply the map function on them,
/// group the results by key and return a common map
/// @tparam KV the key-value pair type
/// @tparam K2 the new key type
/// @tparam V2 the new value type
/// @tparam MappingFunction the type of the map function
/// @param queue the queue that holds the (key, value) pairs
/// @param mapFunction the map function
/// @param mutex the mutex that will be used to lock the queue and the result map
/// @return 
template <typename KV, typename K2, typename V2, typename MappingFunction>
void mapping_generic(std::queue<KV> *queue, 
                    MappingFunction mapFunction, 
                    pthread_mutex_t *mutex,
                    std::map<K2, std::vector<V2>> *result) {
    while (true) {
        pthread_mutex_lock(mutex);
        if (queue->empty()) {
            pthread_mutex_unlock(mutex);
            break;
        }
        KV pair = queue->front();
        queue->pop();
        pthread_mutex_unlock(mutex);
 
        // task result will be the form of a vector of pairs of K2 and V2
        std::vector<std::pair<K2,V2> > taskResult = mapFunction(pair);
        // Go though each pair and group them by K2 in the result vector
 
        for (std::pair<K2, V2> p : taskResult) {
            pthread_mutex_lock(mutex);
            (*result)[p.first].push_back(p.second);
            pthread_mutex_unlock(mutex);
        }
    }
}

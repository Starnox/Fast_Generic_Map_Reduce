#include "mapreduce.h"

// Write generic reduce function
template <typename K2, typename V2, typename KV3, typename ReducingFunction>
KV3 reduce_generic(std::pair<K2, std::vector<V2>> pair, ReducingFunction reduceFunction) {
    KV3 result; 
    for (V2 value : pair.second) {
        result = reduceFunction(pair.first, value, result);
    }
}

/**
 * @brief Executes the map task for a given parser and updates the common map
 * 
 * @param parser the parser object that will be used to read the file
 * @param numReducers the number of reducers
 * @param complete_sets the hash table that holds the perfect powers
 * @param result the common map that will be updated
 * @param mutex the mutex that will be used to lock the common map
 */
void execute_map_task(
        InParser *parser,
        int numReducers,
        std::unordered_set<uint> complete_sets[],
        MapResult *result,
        pthread_mutex_t *mutex)
    {
    std::vector<std::unordered_set<uint>> sets(numReducers + 2);
    int i, n, x, ex;

    *parser >> n;
    for (i = 0; i < n; ++i) {
        *parser >> x;
        for (ex = 2; ex <= numReducers + 1; ex++) {
            if (x == 1)
                sets[ex].insert(x);
            else if(complete_sets[ex].count(x))
                sets[ex].insert(x);
        }
    }
    // Close the files and free the memory
    parser->close_parser_and_free();
    delete parser;
    
    for (int ex = 2; ex <= numReducers + 1; ++ex) {
        pthread_mutex_lock(mutex);
        (*result)[ex].push_back(sets[ex]);
        pthread_mutex_unlock(mutex);
    }
} 

/**
 * @brief Executes the reduce task for the given exponent
 * 
 * @param result the common map that will be used
 * @param threadExponent the exponent that will be used to filter the map
 * @return int the number of unique perfect powers found
 */
int execute_reduce_task(MapResult *result, int threadExponent) {
    auto &vec_sets = (*result)[threadExponent];
    // aggregate all lists from an exponent
    std::unordered_set<uint> currentSet;
        for (auto &set : vec_sets) {
            for (auto &el : set) {
                currentSet.insert(el);
            }
        }

    return currentSet.size();
}


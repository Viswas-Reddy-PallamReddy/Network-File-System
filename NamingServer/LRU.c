#include "NamingServer.h"
// Convert error codes to strings for debugging
const char *cache_error_string(cache_error_t error)
{
    switch (error)
    {
    case CACHE_SUCCESS:
        return "Success";
    case CACHE_ERROR_NULL_PARAM:
        return "Null parameter";
    case CACHE_ERROR_INVALID_CAPACITY:
        return "Invalid capacity";
    case CACHE_ERROR_MEMORY:
        return "Memory allocation failed";
    case CACHE_ERROR_NOT_FOUND:
        return "Entry not found";
    case CACHE_ERROR_DUPLICATE:
        return "Duplicate entry";
    default:
        return "Unknown error";
    }
}

// Hash function for strings
static unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash;
}

// Create new HashMap
HashMap* create_hashmap(int capacity) {
    HashMap *map = (HashMap*)malloc(sizeof(HashMap));
    if (!map) {
        DEBUG_PRINT(DEBUG_ERROR, "Failed to allocate HashMap\n");
        return NULL;
    }
    
    map->capacity = capacity * 2; // Using 2x capacity for better distribution
    map->size = 0;
    map->buckets = (HashMapEntry**)calloc(map->capacity, sizeof(HashMapEntry*));
    
    if (!map->buckets) {
        DEBUG_PRINT(DEBUG_ERROR, "Failed to allocate HashMap buckets\n");
        free(map);
        return NULL;
    }
    
    // DEBUG_PRINT(DEBUG_INFO, "Created HashMap with capacity %d\n", map->capacity);
    return map;
}

// Put entry in HashMap
void hashmap_put(HashMap *map, const char *key, CacheNode *value) {
    if (!map || !key || !value) {
        DEBUG_PRINT(DEBUG_ERROR, "Null parameter in hashmap_put\n");
        return;
    }
    
    unsigned long hash = hash_string(key) % map->capacity;
    HashMapEntry *entry = map->buckets[hash];
    
    // Check if key already exists
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            DEBUG_PRINT(DEBUG_ALL, "Updating existing HashMap entry: %s\n", key);
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
    
    // Create new entry
    HashMapEntry *new_entry = (HashMapEntry*)malloc(sizeof(HashMapEntry));
    if (!new_entry) {
        DEBUG_PRINT(DEBUG_ERROR, "Failed to allocate HashMap entry\n");
        return;
    }
    
    new_entry->key = strdup(key);
    if (!new_entry->key) {
        DEBUG_PRINT(DEBUG_ERROR, "Failed to duplicate key in hashmap_put\n");
        free(new_entry);
        return;
    }
    
    new_entry->value = value;
    new_entry->next = map->buckets[hash];
    map->buckets[hash] = new_entry;
    map->size++;
    
    DEBUG_PRINT(DEBUG_ALL, "Added new HashMap entry: %s\n", key);
}

// Get entry from HashMap
CacheNode* hashmap_get(HashMap *map, const char *key) {
    if (!map || !key) {
        DEBUG_PRINT(DEBUG_ERROR, "Null parameter in hashmap_get\n");
        return NULL;
    }
    
    unsigned long hash = hash_string(key) % map->capacity;
    HashMapEntry *entry = map->buckets[hash];
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            DEBUG_PRINT(DEBUG_ALL, "Found HashMap entry: %s\n", key);
            return entry->value;
        }
        entry = entry->next;
    }
    
    DEBUG_PRINT(DEBUG_ALL, "HashMap entry not found: %s\n", key);
    return NULL;
}

// Remove entry from HashMap
void hashmap_remove(HashMap *map, const char *key) {
    if (!map || !key) {
        DEBUG_PRINT(DEBUG_ERROR, "Null parameter in hashmap_remove\n");
        return;
    }
    
    unsigned long hash = hash_string(key) % map->capacity;
    HashMapEntry *entry = map->buckets[hash];
    HashMapEntry *prev = NULL;
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                map->buckets[hash] = entry->next;
            }
            
            DEBUG_PRINT(DEBUG_ALL, "Removing HashMap entry: %s\n", key);
            free(entry->key);
            free(entry);
            map->size--;
            return;
        }
        prev = entry;
        entry = entry->next;
    }
    
    DEBUG_PRINT(DEBUG_INFO, "Entry not found for removal: %s\n", key);
}

// Free HashMap resources
void free_hashmap(HashMap *map) {
    if (!map) {
        DEBUG_PRINT(DEBUG_ERROR, "Null map in free_hashmap\n");
        return;
    }
    
    DEBUG_PRINT(DEBUG_INFO, "Freeing HashMap resources\n");
    
    for (int i = 0; i < map->capacity; i++) {
        HashMapEntry *entry = map->buckets[i];
        while (entry) {
            HashMapEntry *next = entry->next;
            DEBUG_PRINT(DEBUG_ALL, "Freeing HashMap entry: %s\n", entry->key);
            free(entry->key);
            free(entry);
            entry = next;
        }
    }
    
    free(map->buckets);
    free(map);
    DEBUG_PRINT(DEBUG_INFO, "HashMap freed successfully\n");
}

// Initialize cache with error handling
LRUCache* init_cache(int capacity, cache_error_t *error) {
    if (capacity <= 0) {
        DEBUG_PRINT(DEBUG_ERROR, "Invalid cache capacity: %d\n", capacity);
        if (error) *error = CACHE_ERROR_INVALID_CAPACITY;
        return NULL;
    }

    LRUCache *cache = (LRUCache*)malloc(sizeof(LRUCache));
    if (!cache) {
        DEBUG_PRINT(DEBUG_ERROR, "Failed to allocate cache: %s\n", strerror(errno));
        if (error) *error = CACHE_ERROR_MEMORY;
        return NULL;
    }

    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;
    cache->hits = 0;
    cache->misses = 0;
    cache->evictions = 0;
    
    cache->map = create_hashmap(capacity);
    if (!cache->map) {
        DEBUG_PRINT(DEBUG_ERROR, "Failed to create hashmap\n");
        free(cache);
        if (error) *error = CACHE_ERROR_MEMORY;
        return NULL;
    }

    // DEBUG_PRINT(DEBUG_INFO, "Cache initialized with capacity %d\n", capacity);
    if (error) *error = CACHE_SUCCESS;
    return cache;
}

// Create cache node with error handling
CacheNode* create_cache_node(const char *filepath, int ss_number, cache_error_t *error) {
    if (!filepath) {
        DEBUG_PRINT(DEBUG_ERROR, "Null filepath provided\n");
        if (error) *error = CACHE_ERROR_NULL_PARAM;
        return NULL;
    }

    CacheNode *node = (CacheNode*)malloc(sizeof(CacheNode));
    if (!node) {
        DEBUG_PRINT(DEBUG_ERROR, "Failed to allocate cache node: %s\n", strerror(errno));
        if (error) *error = CACHE_ERROR_MEMORY;
        return NULL;
    }

    node->filepath = strdup(filepath);
    if (!node->filepath) {
        DEBUG_PRINT(DEBUG_ERROR, "Failed to duplicate filepath: %s\n", strerror(errno));
        free(node);
        if (error) *error = CACHE_ERROR_MEMORY;
        return NULL;
    }

    node->storage_server_number = ss_number;
    node->prev = NULL;
    node->next = NULL;
    node->last_access = time(NULL);
    node->access_count = 1;

    DEBUG_PRINT(DEBUG_INFO, "Created cache node for filepath: %s\n", filepath);
    if (error) *error = CACHE_SUCCESS;
    return node;
}

// Move node to front with debug info
void move_to_front(LRUCache *cache, CacheNode *node) {
    if (!cache || !node) {
        DEBUG_PRINT(DEBUG_ERROR, "Null parameter in move_to_front\n");
        return;
    }

    if (node == cache->head) {
        DEBUG_PRINT(DEBUG_ALL, "Node already at front: %s\n", node->filepath);
        return;
    }

    DEBUG_PRINT(DEBUG_ALL, "Moving node to front: %s\n", node->filepath);

    // Remove node from current position
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    if (node == cache->tail) {
        cache->tail = node->prev;
    }

    // Move to front
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) {
        cache->head->prev = node;
    }
    cache->head = node;
    if (!cache->tail) {
        cache->tail = node;
    }

    node->last_access = time(NULL);
    node->access_count++;
}

// Remove LRU entry with debug info
void remove_lru(LRUCache *cache) {
    if (!cache || !cache->tail) {
        DEBUG_PRINT(DEBUG_ERROR, "Invalid cache or empty cache in remove_lru\n");
        return;
    }

    CacheNode *node = cache->tail;
    DEBUG_PRINT(DEBUG_INFO, "Removing LRU node: %s (access count: %u, last access: %ld)\n", 
                node->filepath, node->access_count, node->last_access);

    cache->tail = node->prev;
    if (cache->tail) {
        cache->tail->next = NULL;
    } else {
        cache->head = NULL;
    }

    hashmap_remove(cache->map, node->filepath);
    free(node->filepath);
    free(node);
    cache->size--;
    cache->evictions++;
}

// Add or update cache entry with debug info
cache_error_t cache_put(LRUCache *cache, const char *filepath, int ss_number) {
    if (!cache || !filepath) {
        DEBUG_PRINT(DEBUG_ERROR, "Null parameter in cache_put\n");
        return CACHE_ERROR_NULL_PARAM;
    }

    DEBUG_PRINT(DEBUG_INFO, "Putting entry in cache: %s -> %d\n", filepath, ss_number);

    CacheNode *node = hashmap_get(cache->map, filepath);
    if (node) {
        DEBUG_PRINT(DEBUG_INFO, "Updating existing entry: %s\n", filepath);
        node->storage_server_number = ss_number;
        move_to_front(cache, node);
        return CACHE_SUCCESS;
    }

    cache_error_t error;
    node = create_cache_node(filepath, ss_number, &error);
    if (!node) {
        return error;
    }

    if (cache->size >= cache->capacity) {
        DEBUG_PRINT(DEBUG_INFO, "Cache full, removing LRU entry\n");
        remove_lru(cache);
    }

    node->next = cache->head;
    if (cache->head) {
        cache->head->prev = node;
    }
    cache->head = node;
    if (!cache->tail) {
        cache->tail = node;
    }

    hashmap_put(cache->map, filepath, node);
    cache->size++;

    DEBUG_PRINT(DEBUG_INFO, "Added new entry to cache: %s (size: %d/%d)\n", 
                filepath, cache->size, cache->capacity);
    return CACHE_SUCCESS;
}

// Get entry from cache with debug info
int cache_get(LRUCache *cache, const char *filepath) {
    if (!cache || !filepath) {
        DEBUG_PRINT(DEBUG_ERROR, "Null parameter in cache_get\n");
        return -1;
    }

    CacheNode *node = hashmap_get(cache->map, filepath);
    if (!node) {
        cache->misses++;
        DEBUG_PRINT(DEBUG_INFO, "Cache miss: %s (total misses: %lu)\n", 
                    filepath, cache->misses);
        return -1;
    }

    cache->hits++;
    DEBUG_PRINT(DEBUG_INFO, "Cache hit: %s (total hits: %lu)\n", filepath, cache->hits);
    move_to_front(cache, node);
    return node->storage_server_number;
}

// Print cache statistics
void print_cache_stats(LRUCache *cache) {
    if (!cache) {
        DEBUG_PRINT(DEBUG_ERROR, "Null cache in print_cache_stats\n");
        return;
    }

    double hit_rate = (cache->hits + cache->misses) > 0 ? 
        (double)cache->hits / (cache->hits + cache->misses) * 100.0 : 0.0;

    DEBUG_PRINT(DEBUG_INFO, "\n=== Cache Statistics ===\n");
    DEBUG_PRINT(DEBUG_INFO, "Size: %d/%d\n", cache->size, cache->capacity);
    DEBUG_PRINT(DEBUG_INFO, "Hits: %lu\n", cache->hits);
    DEBUG_PRINT(DEBUG_INFO, "Misses: %lu\n", cache->misses);
    DEBUG_PRINT(DEBUG_INFO, "Hit Rate: %.2f%%\n", hit_rate);
    DEBUG_PRINT(DEBUG_INFO, "Evictions: %lu\n", cache->evictions);

    if (CURRENT_DEBUG_LEVEL >= DEBUG_ALL) {
        DEBUG_PRINT(DEBUG_ALL, "\nCache Contents:\n");
        CacheNode *node = cache->head;
        while (node) {
            DEBUG_PRINT(DEBUG_ALL, "  %s -> SS%d (accesses: %u, last: %s)", 
                       node->filepath, node->storage_server_number, 
                       node->access_count, ctime(&node->last_access));
            node = node->next;
        }
    }
}

// Free cache resources with debug info
void free_cache(LRUCache *cache) {
    if (!cache) {
        DEBUG_PRINT(DEBUG_ERROR, "Null cache in free_cache\n");
        return;
    }

    DEBUG_PRINT(DEBUG_INFO, "Freeing cache resources\n");
    print_cache_stats(cache);

    CacheNode *current = cache->head;
    while (current) {
        CacheNode *next = current->next;
        DEBUG_PRINT(DEBUG_ALL, "Freeing node: %s\n", current->filepath);
        free(current->filepath);
        free(current);
        current = next;
    }

    free_hashmap(cache->map);
    free(cache);
    DEBUG_PRINT(DEBUG_INFO, "Cache freed successfully\n");
}
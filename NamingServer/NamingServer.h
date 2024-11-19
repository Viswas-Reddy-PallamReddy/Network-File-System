#ifndef NAMING_SERVER_H
#define NAMING_SERVER_H

#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#define BUFFER_SIZE 5000
#define MAX_STORAGE_SERVERS 20
#define MAX_CLIENTS 200
#define NM_PORT 5354
#define MAX_FILES_PER_STORAGE_SERVER 100
#define MAX_ACCESSIBLE_PATHS 100
#define INITIAL_STORAGE_SERVERS 2///// 2 for backup SS and 2 for primary SS
#define MAX_FILE_NAME_SIZE 4096
#define MAX_PATH_LENGTH_1 4096
#define INITIAL_CASHE_SIZE 10


typedef struct
{
    char ip[INET_ADDRSTRLEN];
    int server_port;
    int client_port;
    int num_accessible_paths;
    char accessible_paths[MAX_ACCESSIBLE_PATHS][MAX_FILE_NAME_SIZE];
    int storage_server_number;
    bool Backup_SS;
    bool server_down;
} StorageServer;


#define CHUNK_SIZE 256
typedef struct packet {
    int seq_num;            // Sequence number of the packet
    int total_chunks;       // Total number of chunks
    char data[CHUNK_SIZE];  // Data chunk
} packet;

typedef struct TrieNode
{
    struct TrieNode *children[256];
    StorageServer *server;
    int isEndOfPath; // Added to mark end of valid paths
} TrieNode;

// Error logging enum and function
typedef enum
{
    TRIE_SUCCESS = 0,
    TRIE_NULL_POINTER,
    TRIE_INVALID_PATH,
    TRIE_PATH_NOT_FOUND,
    TRIE_PATH_ALREADY_EXISTS,
    TRIE_MEMORY_ERROR,
    TRIE_PATH_TOO_LONG
} TrieError;

const char *get_error_string(TrieError error);
void log_error(const char *function_name, TrieError error, const char *path);
TrieError validate_path(const char *path);
TrieNode *create_trie_node();
TrieError insert_path(TrieNode *root, const char *path, StorageServer *server);
TrieError delete_path(TrieNode *root, const char *path);
TrieError update_server(TrieNode *root, const char *path, StorageServer *new_server);
TrieError move_path(TrieNode *root, const char *old_path, const char *new_path, StorageServer *new_server);
StorageServer *find_storage_server(TrieNode *root, const char *path);

typedef struct CacheNode
{
    char *filepath;
    int storage_server_number;
    struct CacheNode *prev;
    struct CacheNode *next;
    time_t last_access;        // Track when this entry was last accessed
    unsigned int access_count; // Track how many times this entry was accessed
} CacheNode;

// HashMap entry structure
typedef struct HashMapEntry
{
    char *key;                 // filepath
    CacheNode *value;          // pointer to cache node
    struct HashMapEntry *next; // for collision handling
} HashMapEntry;

// HashMap structure
typedef struct
{
    HashMapEntry **buckets;
    int capacity;
    int size;
} HashMap;

typedef struct
{
    int capacity;
    int size;
    CacheNode *head;
    CacheNode *tail;
    HashMap *map;
    unsigned long hits;      // Cache hits counter
    unsigned long misses;    // Cache misses counter
    unsigned long evictions; // Number of entries evicted
} LRUCache;

// Debug levels
#define DEBUG_NONE 0
#define DEBUG_ERROR 1
#define DEBUG_INFO 2
#define DEBUG_ALL 3

// Set current debug level
#define CURRENT_DEBUG_LEVEL DEBUG_ALL

// Debug macro
#define DEBUG_PRINT(level, fmt, ...)                               \
    do                                                             \
    {                                                              \
        if (level <= CURRENT_DEBUG_LEVEL)                          \
        {                                                          \
            time_t now = time(NULL);                               \
            char timestamp[26];                                    \
            ctime_r(&now, timestamp);                              \
            timestamp[24] = '\0';                                  \
            fprintf(stderr, "[%s][%s:%d] " fmt,                    \
                    timestamp, __FILE__, __LINE__, ##__VA_ARGS__); \
        }                                                          \
    } while (0)

// Error codes
typedef enum
{
    CACHE_SUCCESS = 0,
    CACHE_ERROR_NULL_PARAM = -1,
    CACHE_ERROR_INVALID_CAPACITY = -2,
    CACHE_ERROR_MEMORY = -3,
    CACHE_ERROR_NOT_FOUND = -4,
    CACHE_ERROR_DUPLICATE = -5
} cache_error_t;

// Global cache instance


const char *cache_error_string(cache_error_t error);
LRUCache *init_cache(int capacity, cache_error_t *error);
CacheNode *create_cache_node(const char *filepath, int ss_number, cache_error_t *error);
void move_to_front(LRUCache *cache, CacheNode *node);
void remove_lru(LRUCache *cache);
cache_error_t cache_put(LRUCache *cache, const char *filepath, int ss_number);
int cache_get(LRUCache *cache, const char *filepath);
void print_cache_stats(LRUCache *cache);
void free_cache(LRUCache *cache);
void hashmap_remove(HashMap *map, const char *key);

#endif
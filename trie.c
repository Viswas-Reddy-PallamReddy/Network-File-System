#include "NamingServer.h"

TrieNode *create_trie_node()
{
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    if (node == NULL)
    {
        perror("Failed to allocate memory for trie node");
        exit(1);
    }
    memset(node->children, 0, sizeof(node->children));
    node->server = NULL;
    node->isEndOfPath = 0;
    return node;
}

// Insert a path into the trie
void insert_path(TrieNode *root, const char *path, StorageServer *server)
{
    if (!root || !path || !server)
    {
        return;
    }

    TrieNode *current = root;

    // Traverse/create the path in trie
    for (int i = 0; path[i] != '\0'; i++)
    {
        unsigned char index = (unsigned char)path[i];
        if (!current->children[index])
        {
            current->children[index] = create_trie_node();
        }
        current = current->children[index];
    }

    // Mark end of path and store server
    current->isEndOfPath = 1;
    current->server = server;
}

StorageServer *find_storage_server(TrieNode *root, const char *path)
{
    if (!root || !path)
    {
        return NULL;
    }

    TrieNode *current = root;

    // Debug output
    printf("Searching for path: %s\n", path);

    // Traverse the trie following the path
    for (int i = 0; path[i] != '\0'; i++)
    {
        unsigned char index = (unsigned char)path[i];
        printf("Checking character: %c (index: %d)\n", path[i], index);

        if (!current->children[index])
        {
            printf("No matching node found for character: %c\n", path[i]);
            return NULL;
        }
        current = current->children[index];
    }

    // Check if this is a valid end of path
    if (current->isEndOfPath)
    {
        printf("Found server for path %s\n", path);
        return current->server;
    }

    printf("Path exists but no server found (not a complete path)\n");
    return NULL;
}

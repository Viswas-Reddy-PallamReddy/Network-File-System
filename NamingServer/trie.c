#include "NamingServer.h"

// TrieNode *create_trie_node()
// {
//     TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
//     if (node == NULL)
//     {
//         perror("Failed to allocate memory for trie node");
//         exit(1);
//     }
//     memset(node->children, 0, sizeof(node->children));
//     node->server = NULL;
//     node->isEndOfPath = 0;
//     return node;
// }

// // Insert a path into the trie
// void insert_path(TrieNode *root, const char *path, StorageServer *server)
// {
//     if (!root || !path || !server)
//     {
//         return;
//     }

//     TrieNode *current = root;

//     // Traverse/create the path in trie
//     for (int i = 0; path[i] != '\0'; i++)
//     {
//         unsigned char index = (unsigned char)path[i];
//         if (!current->children[index])
//         {
//             current->children[index] = create_trie_node();
//         }
//         current = current->children[index];
//     }

//     // Mark end of path and store server
//     current->isEndOfPath = 1;
//     current->server = server;
// }

// StorageServer *find_storage_server(TrieNode *root, const char *path)
// {
//     if (!root || !path)
//     {
//         return NULL;
//     }

//     TrieNode *current = root;

//     // Debug output
//     printf("Searching for path: %s\n", path);

//     // Traverse the trie following the path
//     for (int i = 0; path[i] != '\0'; i++)
//     {
//         unsigned char index = (unsigned char)path[i];
//         printf("Checking character: %c (index: %d)\n", path[i], index);

//         if (!current->children[index])
//         {
//             printf("No matching node found for character: %c\n", path[i]);
//             return NULL;
//         }
//         current = current->children[index];
//     }

//     // Check if this is a valid end of path
//     if (current->isEndOfPath)
//     {
//         printf("Found server for path %s\n", path);
//         return current->server;
//     }

//     printf("Path exists but no server found (not a complete path)\n");
//     return NULL;
// }

const char *get_error_string(TrieError error)
{
    switch (error)
    {
    case TRIE_SUCCESS:
        return "Operation successful";
    case TRIE_NULL_POINTER:
        return "Null pointer provided";
    case TRIE_INVALID_PATH:
        return "Invalid path format";
    case TRIE_PATH_NOT_FOUND:
        return "Path not found";
    case TRIE_PATH_ALREADY_EXISTS:
        return "Path already exists";
    case TRIE_MEMORY_ERROR:
        return "Memory allocation failed";
    case TRIE_PATH_TOO_LONG:
        return "Path exceeds maximum length";
    default:
        return "Unknown error";
    }
}

void log_error(const char *function_name, TrieError error, const char *path)
{
    fprintf(stderr, "[ERROR] %s: %s - Path: %s\n",
            function_name,
            get_error_string(error),
            path ? path : "NULL");
}

// Path validation function
TrieError validate_path(const char *path)
{
    if (!path)
    {
        return TRIE_NULL_POINTER;
    }

    // Check path length
    size_t len = strlen(path);
    // printf("[DEBUG] Path length: %zu\n", len);

    // printf("[DEBUG] Path: %s\n", path);
    if (len == 0 || len > 4096)
    { // 4096 is a typical max path length
        return TRIE_PATH_TOO_LONG;
    }

    // // Path must start with '/'
    // if (path[0] != '/')
    // {
    //     return TRIE_INVALID_PATH;
    // }

    // Check for invalid characters and consecutive slashes
    for (size_t i = 1; i < len; i++)
    {
        // Check for control characters
        if (iscntrl(path[i]))
        {
            return TRIE_INVALID_PATH;
        }

        // Check for consecutive slashes
        if (path[i] == '/' && path[i - 1] == '/')
        {
            return TRIE_INVALID_PATH;
        }
    }

    return TRIE_SUCCESS;
}

// Modified create_trie_node with error handling
TrieNode *create_trie_node()
{
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    if (node == NULL)
    {
        log_error("create_trie_node", TRIE_MEMORY_ERROR, NULL);
        return NULL;
    }
    memset(node->children, 0, sizeof(node->children));
    node->server = NULL;
    node->isEndOfPath = 0;
    return node;
}

// Modified insert_path with validation and error handling
TrieError insert_path(TrieNode *root, const char *path, StorageServer *server)
{
    if (!root || !path || !server)
    {
        log_error("insert_path", TRIE_NULL_POINTER, path);
        return TRIE_NULL_POINTER;
    }

    // Validate path
    TrieError validation_result = validate_path(path);
    if (validation_result != TRIE_SUCCESS)
    {
        log_error("insert_path", validation_result, path);
        return validation_result;
    }

    // Check if path already exists
    if (find_storage_server(root, path) != NULL)
    {
        log_error("insert_path", TRIE_PATH_ALREADY_EXISTS, path);
        return TRIE_PATH_ALREADY_EXISTS;
    }

    TrieNode *current = root;
    for (int i = 0; path[i] != '\0'; i++)
    {
        unsigned char index = (unsigned char)path[i];
        if (!current->children[index])
        {
            current->children[index] = create_trie_node();
            if (!current->children[index])
            {
                log_error("insert_path", TRIE_MEMORY_ERROR, path);
                return TRIE_MEMORY_ERROR;
            }
        }
        current = current->children[index];
    }

    current->isEndOfPath = 1;
    current->server = server;
    return TRIE_SUCCESS;
}

// Modified delete_path with validation and error handling
// Helper function to check if a node has any children
int has_children(TrieNode *node)
{
    if (!node)
        return 0;

    // Check all possible child nodes
    for (int i = 0; i < 256; i++)
    {
        if (node->children[i] != NULL)
        {
            return 1; // Found at least one child
        }
    }
    return 0; // No children found
}

// Delete path function with helper
TrieError delete_path(TrieNode *root, const char *path)
{
    if (!root || !path)
    {
        log_error("delete_path", TRIE_NULL_POINTER, path);
        return TRIE_NULL_POINTER;
    }

    // Validate path
    TrieError validation_result = validate_path(path);
    if (validation_result != TRIE_SUCCESS)
    {
        log_error("delete_path", validation_result, path);
        return validation_result;
    }

    printf("[DEBUG] Path validated successfully: %s\n", path);

    TrieNode *stack[4096]; // Stack to store nodes for backtracking
    int depths[4096];      // Store the depth/position of each node
    int top = 0;           // Stack pointer

    TrieNode *current = root;
    int path_len = strlen(path);

    // Traverse to the target node while storing the path
    for (int i = 0; i < path_len; i++)
    {
        unsigned char index = (unsigned char)path[i];
        if (!current->children[index])
        {
            log_error("delete_path", TRIE_PATH_NOT_FOUND, path);
            return TRIE_PATH_NOT_FOUND;
        }
        stack[top] = current; // Store parent node
        depths[top] = i;      // Store the depth/character position
        top++;
        current = current->children[index];
    }

    // Check if this is a valid end of path
    if (!current->isEndOfPath)
    {
        log_error("delete_path", TRIE_PATH_NOT_FOUND, path);
        return TRIE_PATH_NOT_FOUND;
    }
    printf("[DEBUG] Found target node for deletion\n");

    // Clear the end node markers
    current->isEndOfPath = 0;
    current->server = NULL;

    // If the node has no children, we can delete it and potentially its parents
    if (!has_children(current))
    {
        top--; // Move back to last parent
        while (top >= 0)
        {
            unsigned char index = (unsigned char)path[depths[top]];
            TrieNode *parent = stack[top];

            // Free the child node
            free(parent->children[index]);
            parent->children[index] = NULL;

            // Stop if parent has other children, is root, or is end of another path
            if (has_children(parent) || top == 0 || parent->isEndOfPath)
            {
                break;
            }
            top--;
        }
    }
    printf("[DEBUG] Path successfully deleted\n");
    return TRIE_SUCCESS;
}
// Modified update_server with validation and error handling
TrieError update_server(TrieNode *root, const char *path, StorageServer *new_server)
{
    if (!root || !path || !new_server)
    {
        log_error("update_server", TRIE_NULL_POINTER, path);
        return TRIE_NULL_POINTER;
    }

    // Validate path
    TrieError validation_result = validate_path(path);
    if (validation_result != TRIE_SUCCESS)
    {
        log_error("update_server", validation_result, path);
        return validation_result;
    }

    TrieNode *current = root;
    for (int i = 0; path[i] != '\0'; i++)
    {
        unsigned char index = (unsigned char)path[i];
        if (!current->children[index])
        {
            log_error("update_server", TRIE_PATH_NOT_FOUND, path);
            return TRIE_PATH_NOT_FOUND;
        }
        current = current->children[index];
    }

    if (!current->isEndOfPath)
    {
        log_error("update_server", TRIE_PATH_NOT_FOUND, path);
        return TRIE_PATH_NOT_FOUND;
    }

    current->server = new_server;
    return TRIE_SUCCESS;
}

// Modified move_path with validation and error handling
TrieError move_path(TrieNode *root, const char *old_path, const char *new_path, StorageServer *new_server)
{
    if (!root || !old_path || !new_path)
    {
        log_error("move_path", TRIE_NULL_POINTER, old_path);
        return TRIE_NULL_POINTER;
    }

    // Validate both paths
    TrieError validation_result = validate_path(old_path);
    if (validation_result != TRIE_SUCCESS)
    {
        log_error("move_path", validation_result, old_path);
        return validation_result;
    }

    validation_result = validate_path(new_path);
    if (validation_result != TRIE_SUCCESS)
    {
        log_error("move_path", validation_result, new_path);
        return validation_result;
    }

    // Check if old path exists and get its server
    StorageServer *server = find_storage_server(root, old_path);
    if (!server)
    {
        log_error("move_path", TRIE_PATH_NOT_FOUND, old_path);
        return TRIE_PATH_NOT_FOUND;
    }

    // Check if new path already exists
    if (find_storage_server(root, new_path))
    {
        log_error("move_path", TRIE_PATH_ALREADY_EXISTS, new_path);
        return TRIE_PATH_ALREADY_EXISTS;
    }

    // Use existing server if new_server not provided
    if (!new_server)
    {
        new_server = server;
    }

    // Delete old path
    TrieError delete_result = delete_path(root, old_path);
    if (delete_result != TRIE_SUCCESS)
    {
        return delete_result;
    }

    // Insert new path
    return insert_path(root, new_path, new_server);
}

StorageServer *find_storage_server(TrieNode *root, const char *path)
{
    if (!root || !path)
    {
        log_error("find_storage_server", TRIE_NULL_POINTER, path);
        return NULL;
    }

    // Validate path
    TrieError validation_result = validate_path(path);
    if (validation_result != TRIE_SUCCESS)
    {
        log_error("find_storage_server", validation_result, path);
        return NULL;
    }

    TrieNode *current = root;

// Debug output if needed
#ifdef DEBUG
    printf("Searching for path: %s\n", path);
#endif

    // Traverse the trie following the path
    for (int i = 0; path[i] != '\0'; i++)
    {
        unsigned char index = (unsigned char)path[i];

#ifdef DEBUG
        printf("Checking character: %c (index: %d)\n", path[i], index);
#endif

        if (!current->children[index])
        {
#ifdef DEBUG
            printf("No matching node found for character: %c\n", path[i]);
#endif
            log_error("find_storage_server", TRIE_PATH_NOT_FOUND, path);
            return NULL;
        }
        current = current->children[index];
    }

    // Check if this is a valid end of path
    if (current->isEndOfPath && current->server)
    {
#ifdef DEBUG
        printf("Found server for path %s\n", path);
#endif
        return current->server;
    }

#ifdef DEBUG
    printf("Path exists but no server found (not a complete path)\n");
#endif
    log_error("find_storage_server", TRIE_PATH_NOT_FOUND, path);
    return NULL;
}
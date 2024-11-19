#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define ip_address "192.168.137.243"

void traverse_directory(const char *dirname, const char *base, int *path_count) {
    DIR *dir;
    struct dirent *ent;
    struct stat st;

    // Open the directory
    if ((dir = opendir(dirname)) != NULL) {
        while ((ent = readdir(dir)) != NULL && *path_count < 1024) {
            if (ent->d_name[0] == '.') {
                continue;  // Skip hidden files and directories (e.g., "." and "..")
            }

            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dirname, ent->d_name);

            // Get the file status
            if (stat(full_path, &st) == 0) {
                char relative_path[1024];

                // Calculate the relative path
                if (strcmp(base, ".") == 0) {
                    // If the base is ".", use only the file/folder name
                    snprintf(relative_path, sizeof(relative_path), "%s", ent->d_name);
                } else {
                    // Otherwise, construct the relative path by removing the base length from the full path
                    snprintf(relative_path, sizeof(relative_path), "./%s", full_path + strlen(base) + 1);
                }

                // Recursive call for subdirectories
                if (S_ISDIR(st.st_mode)) {
                    traverse_directory(full_path, base, path_count);
                }

                // Print the relative path
                printf("Relative Path: %s\n", relative_path);
                (*path_count)++;
            }
        }
        closedir(dir);
    } else {
        perror("Error reading directory");
    }
}

void list_all_accessible_files() {
    char base[1024];
    getcwd(base, sizeof(base));
    int path_count = 0;
    traverse_directory(base, base, &path_count);
}

int main() {
    list_all_accessible_files();
    return 0;
}
MSDIMPSDMF
imdfsmfpsm
skmfdsmfms
dkmfksmfsm
lkms;mfksdm
msflsnfokms
fnm;slmfklms
jfnsmfjlsdm
jlfnms;lmfs
fmslnfsd
imfds;jfsd
smf;mfdsl
jnfksmf
UDOSFHYIEUOUFW
FSNFOISFUS
SFODUSHFHDSIUF
SFOSJFISJS
FUOSJDFSYIFHIUS
ISFODSJHIFHDSO
SFSDOFHISHOFIS
SUFHDS9UHFS

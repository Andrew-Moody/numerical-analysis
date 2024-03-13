#include "filepath.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define PATH_BUFFER_SIZE 4096

// Returns a string containing the full filepath given a filepath relative to the repository
// prefix can used to add an optional path between the repository root and filename i.e. models/
char* get_full_filepath(const char* filename, const char* prefix)
{
    char* path = malloc(sizeof(char) * PATH_BUFFER_SIZE);

    int path_length = readlink("/proc/self/exe", path, PATH_BUFFER_SIZE - 1);

    if (path_length == -1)
    {
        printf("Error Importing File: readlink(/proc/self/exe) failed to get path to executable\n");
        free(path);
        return NULL;
    }

    // Readlink does not add null terminating character
    path[path_length] = '\0';

    printf("Found Executable Path: %s\n", path);

    const char* needle = "build";
    char* builddir = strstr(path, needle);

    if (builddir)
    {
        // Temporarily shorten path to show build directory 
        builddir[5] = '\0';
        printf("Found Build Directory: %s\n", path);
        builddir[5] = '/';

        // Check there is enough room to add the full filepath;
        // including the path to repo, prefix, filename, and terminating character
        int repopath_length = builddir - path;
        int filename_length = strlen(filename);

        int prefix_length = 0;
        if (prefix)
        {
            prefix_length = strlen(prefix);
        }

        if (repopath_length + prefix_length + filename_length + 1 > PATH_BUFFER_SIZE)
        {
            printf("Error Importing File: path to model file is too long > PATH_MAX: %i\n", PATH_BUFFER_SIZE);
            free(path);
            return NULL;
        }

        // The directory above the build directory is the repository root (may have a different name)
        // first append the prefix if provided
        if (prefix)
        {
            strcpy(builddir, prefix);
            builddir += prefix_length;

            printf("Found Subdirectory: %s\n", path);
        }

        // append filename (or relative path)
        strcpy(builddir, filename);

        printf("Found File Path: %s\n", path);
    }
    else
    {
        printf("Error Importing File: Build Directory Not Found In Executable Directory\n"
            "This means the build executable is not located under the build directory\n"
            "Executable Directory: %s\n", path);

        free(path);
        return NULL;
    }

    return path;
}
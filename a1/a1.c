#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

int succes=0;

int list_normal(char *path)
{
    DIR *dir = NULL;
    char fullPath[512];
    struct dirent *entry = NULL;

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("Could not open directory");
        return -1;
    }
    if(succes==0)
    {
        printf("SUCCESS\n");
        succes=1;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
        if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0)
           printf("%s\n", fullPath+2);
    }
    return 1;
    closedir(dir);
}

int list_recursiv(char *path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("Could not open directory");
        return -1;
    }
    if(succes==0)
    {
        printf("SUCCESS\n");
        succes=1;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if (lstat(fullPath, &statbuf) == 0)
            {
                printf("%s\n", fullPath+2);
                if (S_ISDIR(statbuf.st_mode))
                {
                    list_recursiv(fullPath);
                }
            }
        }
    }
    return 1;
    closedir(dir);
}

int main(int argc, char **argv)
{

    int list = 0, recursive = 0;
    char *path=NULL;

    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("44076\n");
        }
        else
        {

            for (int i = 1; i < argc; i++)
            {
                if (strcmp(argv[i], "list") == 0)
                    list = 1;

                if (strcmp(argv[i], "recursive") == 0)
                    recursive = 1;

                if (strcmp(strtok(argv[i], "="), "path") == 0)
                {
                    path=(char*)malloc(sizeof(char)*(3+strlen(argv[i]+5)));
                    //path="./";
                    strcpy(path,"./");
                    //printf("%s\n", path);
                    strcat(path,argv[i]+5);
                    //path = argv[i] + 5;
                    //printf("%s\n", path);
                }
            }
        }
    }

    if (list == 1 && recursive == 1)
    {
        if(list_recursiv(path)==-1)
        {
            printf("ERROR");
            perror("invalid directory path");
        }
    }
    else if (list == 1)
        if(list_normal(path)==-1)
        {
            printf("ERROR");
            perror("invalid directory path");
        }
    return 0;
}
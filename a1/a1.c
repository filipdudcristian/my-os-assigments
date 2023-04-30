#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

int succes = 0;

typedef struct Section_Header
{
    char sect_name[17];
    int sect_type;
    int sect_offset;
    int sect_size;
} Section_Header;

int list_normal(char *path, char *name_starts_with, int has_perm_execute)
{
    DIR *dir = NULL;
    char fullPath[512];
    struct dirent *entry = NULL;
    struct stat statbuf;

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("Could not open directory");
        return -1;
    }
    if (succes == 0)
    {
        printf("SUCCESS\n");
        succes = 1;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            if (has_perm_execute == 1)
            {
                if (lstat(fullPath, &statbuf) == 0)
                {
                    if (statbuf.st_mode & S_IXUSR)
                    {
                        if (name_starts_with != NULL)
                        {

                            if (strncmp(name_starts_with, entry->d_name, strlen(name_starts_with)) == 0)
                            {
                                printf("%s\n", fullPath + 2);
                            }
                        }
                        else
                            printf("%s\n", fullPath + 2);
                    }
                }
            }
            else
            {
                if (name_starts_with != NULL)
                {
                    if (strncmp(name_starts_with, entry->d_name, strlen(name_starts_with)) == 0)
                    {
                        printf("%s\n", fullPath + 2);
                    }
                }
                else
                    printf("%s\n", fullPath + 2);
            }
        }
    }
    closedir(dir);
    return 1;
}

int list_recursiv(char *path, char *name_starts_with, int has_perm_execute)
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
    if (succes == 0)
    {
        printf("SUCCESS\n");
        succes = 1;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if (lstat(fullPath, &statbuf) == 0)
            {
                if (has_perm_execute == 1)
                {
                    if (statbuf.st_mode & S_IXUSR)
                    {
                        if (name_starts_with != NULL)
                        {

                            if (strncmp(name_starts_with, entry->d_name, strlen(name_starts_with)) == 0)
                            {
                                printf("%s\n", fullPath + 2);
                            }
                        }
                        else
                            printf("%s\n", fullPath + 2);_
                    }
                }
                else
                {
                    if (name_starts_with != NULL)
                    {
                        if (strncmp(name_starts_with, entry->d_name, strlen(name_starts_with)) == 0)
                        {
                            printf("%s\n", fullPath + 2);
                        }
                    }
                    else
                        printf("%s\n", fullPath + 2);
                }

                if (S_ISDIR(statbuf.st_mode))
                {
                    list_recursiv(fullPath, name_starts_with, has_perm_execute);
                }
            }
        }
    }
    closedir(dir);
    return 1;
}

int parse_SF_file(char *path)
{
    off_t fd;
    fd = open(path, O_RDONLY);

    lseek(fd, -1, SEEK_END);
    char magic;
    int header_size = 0, version = 0, no_of_sections = 0;

    if (read(fd, &magic, 1) != 1 || magic != 'd')
    {
        printf("ERROR\nwrong magic\n");
        return -1;
    }

    lseek(fd, -3, SEEK_END);
    read(fd, &header_size, 2);

    lseek(fd, -header_size, SEEK_END);

    read(fd, &version, 2);
    if (version < 41 || version > 151)
    {
        printf("ERROR\nwrong version\n");
        return -1;
    }

    read(fd, &no_of_sections, 1);
    if (no_of_sections < 5 || no_of_sections > 19)
    {
        printf("ERROR\nwrong sect_nr\n");
        return -1;
    }

    Section_Header sections[no_of_sections];
    for (int i = 0; i < no_of_sections; i++)
    {
        strcpy(sections[i].sect_name, "");
        sections[i].sect_type = 0;
        sections[i].sect_offset = 0;
        sections[i].sect_size = 0;

        read(fd, &(sections[i].sect_name), 17);
        sections[i].sect_name[17] = '\0';

        read(fd, &(sections[i].sect_type), 2);
        if (sections[i].sect_type != 52 && sections[i].sect_type != 41 && sections[i].sect_type != 89)
        {
            printf("ERROR\nwrong sect_types\n");
            return -1;
        }
        read(fd, &(sections[i].sect_offset), 4);
        read(fd, &(sections[i].sect_size), 4);
    }

    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", no_of_sections);

    for (int i = 0; i < no_of_sections; i++)
    {
        printf("section%d: %s %d %d\n", i + 1, sections[i].sect_name, sections[i].sect_type, sections[i].sect_size);
    }
    close(fd);
    return 1;
}

int extract_from_SF(char *path, int section, int line)
{
   
    off_t fd = 0;
    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        printf("ERROR\ninvalid file path\n");
        return -1;
    }

    lseek(fd, -1, SEEK_END);
    char magic = 0;
    int header_size = 0, version = 0, no_of_sections = 0;

    if (read(fd, &magic, 1) != 1 || magic != 'd')
    {
        printf("ERROR\ninvalid file magic\n");
        close(fd);
        return -1;
    }

    lseek(fd, -3, SEEK_END);
    read(fd, &header_size, 2);

    lseek(fd, -header_size, SEEK_END);

    read(fd, &version, 2);
    if (version < 41 || version > 151)
    {
        printf("ERROR\ninvalid file version\n");
        close(fd); 
        return -1;
    }

    read(fd, &no_of_sections, 1);
    if (no_of_sections < 5 || no_of_sections > 19)
    {
        printf("ERROR\nwinvalid file no of sections\n");
        close(fd);
        return -1;
    }

    if (section > no_of_sections)
    {
        printf("ERROR\ninvalid section number");
        close(fd);
        return -1;
    }
    Section_Header sections[no_of_sections + 1];
    for (int i = 1; i <= no_of_sections; i++)
    {
        strcpy(sections[i].sect_name, "");
        sections[i].sect_type = 0;
        sections[i].sect_offset = 0;
        sections[i].sect_size = 0;

        read(fd, &(sections[i].sect_name), 17);
        sections[i].sect_name[17] = '\0';
        read(fd, &(sections[i].sect_type), 2);
        if (sections[i].sect_type != 52 && sections[i].sect_type != 41 && sections[i].sect_type != 89)
        {
            printf("ERROR\ninvalid file section type\n");
            close(fd);
            return -1;
        }
        read(fd, &(sections[i].sect_offset), 4);
        read(fd, &(sections[i].sect_size), 4);
    }

   char *section_content = NULL, *pos = NULL;

    int nrLines = 1;
    char c = 0;
    lseek(fd, sections[section].sect_offset, SEEK_SET);
    section_content = (char *)malloc(sizeof(char) * sections[section].sect_size);
    read(fd, section_content, sections[section].sect_size);
    pos = section_content;
    do
    {
        pos = strchr(pos, '\x0A');

        if (pos != NULL)
        {
            pos++;
            nrLines++;
        }

    } while (pos != NULL);
    if (line > nrLines)
    {
        printf("ERROR\ninvalid line\n");
        free(section_content);
        close(fd);
        return -1;
    }

    lseek(fd, sections[section].sect_offset, SEEK_SET);

    pos = section_content;
    do
    {

        if (nrLines == line)
        {

            printf("SUCCESS\n");
            c = 0;
     
            lseek(fd, pos - section_content, SEEK_CUR);
            while (c != '\x0A')
    
            {
                if (read(fd, &c, 1) != 1)
                {
                    perror("Eroare la citire");
                    free(section_content);
                    return -1;
                }
                printf("%c", c);
  
            }
        }

        pos = strchr(pos, '\x0A');

        if (pos != NULL)
        {
            pos++;
            nrLines--;
        }
        
        if(nrLines < line)
            break;
    } while (pos != NULL && nrLines > 0);

    free(section_content);

    close(fd);
    return -1;
}

int SF(char *path)
{
    off_t fd;
    fd = open(path, O_RDONLY);

    lseek(fd, -1, SEEK_END);
    char magic;
    int header_size = 0, version = 0, no_of_sections = 0;

    if (read(fd, &magic, 1) != 1 || magic != 'd')
    {
        return -1;
    }
    lseek(fd, -3, SEEK_END);
    read(fd, &header_size, 2);

    lseek(fd, -header_size, SEEK_END);

    read(fd, &version, 2);
    if (version < 41 || version > 151)
    {
        return -1;
    }

    read(fd, &no_of_sections, 1);
    if (no_of_sections < 5 || no_of_sections > 19)
    {
        return -1;
    }

    Section_Header sections[no_of_sections];
    for (int i = 0; i < no_of_sections; i++)
    {
        strcpy(sections[i].sect_name, "");
        sections[i].sect_type = 0;
        sections[i].sect_offset = 0;
        sections[i].sect_size = 0;

        read(fd, &(sections[i].sect_name), 17);
        sections[i].sect_name[17] = '\0';

        read(fd, &(sections[i].sect_type), 2);
        if (sections[i].sect_type != 52 && sections[i].sect_type != 41 && sections[i].sect_type != 89)
        {
            return -1;
        }
        read(fd, &(sections[i].sect_offset), 4);
        read(fd, &(sections[i].sect_size), 4);
    }

    int nrLines;

    char *section_content, *c;
    for (int section = 0; section < no_of_sections; section++)
    {
        nrLines = 1;
        lseek(fd, sections[section].sect_offset, SEEK_SET);
        section_content = (char *)malloc(sizeof(char) * sections[section].sect_size);
        read(fd, section_content, sections[section].sect_size);
        c = section_content;
        do
        {
            c = strchr(c, '\x0A');

            if (c != NULL)
            {
                c++;
                nrLines++;
            }
            if (nrLines > 16)
            {
                free(section_content);
                return 1;
            }

        } while (c != NULL);

        free(section_content);
    }
    close(fd);
    return -1;
}



int findall_SF(char *path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    dir = opendir(path);
    char fullPath[512];
    struct stat statbuf;

    if (dir == NULL)
    {
        perror("ERROR\ninvalid directory path\n");
        return -1;
    }
    if (succes == 0)
    {
        printf("SUCCESS\n");
        succes = 1;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);

            lstat(fullPath, &statbuf);
            if (!S_ISDIR(statbuf.st_mode))
                if (SF(fullPath) == 1)
                    printf("%s\n", fullPath + 2);
            if (S_ISDIR(statbuf.st_mode))
            {
                findall_SF(fullPath);
            }
        }
    }
    closedir(dir);
    return 1;
}

int main(int argc, char **argv)
{

    int list = 0, recursive = 0, has_perm_execute = 0, parse = 0, extract = 0, section = 0, line = 0, findall = 0;
    char *path = NULL, *name_starts_with = NULL;

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

                if (strcmp(argv[i], "parse") == 0)
                    parse = 1;

                char *temp_argv = (char *)malloc(sizeof(char) * strlen(argv[i]));
                strcpy(temp_argv, argv[i]);
                char *name_filtration = strtok(temp_argv, "=");

                if (strcmp(name_filtration, "path") == 0)
                {
                    path = (char *)malloc(sizeof(char) * (3 + strlen(argv[i] + 5)));

                    strcpy(path, "./");

                    strcat(path, argv[i] + 5);

                }
                if (strcmp(name_filtration, "name_starts_with") == 0)
                {
                    name_starts_with = (char *)malloc(sizeof(char) * (3 + strlen(argv[i] + 17))); 
                    strcpy(name_starts_with, argv[i] + 17);                                       
                }
                if (strcmp(argv[i], "has_perm_execute") == 0)
                {
                    has_perm_execute = 1;
                }

                if (strcmp(argv[i], "extract") == 0)
                {
                    extract = 1;
                }

                if (strcmp(name_filtration, "section") == 0)
                {
                    sscanf(argv[i], "section=%d", &section);
                }

                if (strcmp(name_filtration, "line") == 0)
                {
                    sscanf(argv[i], "line=%d", &line);
                }

                if (strcmp(argv[i], "findall") == 0)
                {
                    findall = 1;
                }

                free(temp_argv);
            }
        }
    }

    if (list == 1 && recursive == 1)
    {
        if (list_recursiv(path, name_starts_with, has_perm_execute) == -1)
        {
            printf("ERROR");
            perror("invalid directory path");
        }
    }
    else if (list == 1)
        if (list_normal(path, name_starts_with, has_perm_execute) == -1)
        {
            printf("ERROR");
            perror("invalid directory path");
        }

    if (parse == 1)
    {
        parse_SF_file(path);
    }

    if (extract == 1)
    {
        extract_from_SF(path, section, line);
    }
    if (findall == 1)
    {
        findall_SF(path);
    }

    free(path);
    free(name_starts_with);
    return 0;
}

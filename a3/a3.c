#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>

typedef struct Section_Header
{
    char sect_name[17];
    short *sect_type;
    int *sect_offset;
    int *sect_size;
} Section_Header;

volatile char *data = NULL;
unsigned int size = 0;
char file_name[50];
int fd;
int file_size;
char *file;

int main(int argc, char **argv)
{
    unlink("RESP_PIPE_44076");

    int fd_resp, fd_req;
    unsigned ver_num = 44076;
    char comanda[50];
    int shm_memorie_partajata = 0;

    if (mkfifo("RESP_PIPE_44076", 0600) != 0)
    {
        printf("ERROR\ncannot create the response pipe\n");
        return -1;
    }

    fd_req = open("REQ_PIPE_44076", O_RDONLY);
    fd_resp = open("RESP_PIPE_44076", O_WRONLY);

    if (fd_resp == -1)
    {
        printf("ERROR\ncannot open the request pipe\n");
    }

    char dollar_sign = '$';
    char hello[] = "HELLO";

    write(fd_resp, hello, 5);
    write(fd_resp, &dollar_sign, 1);
    printf("SUCCESS\n");
    // char string[200];
    while (1)
    {
        memset(comanda, 0, 50);

        char tempC = 0;
        int i = 0;
        while (tempC != '$')
        {
            read(fd_req, &tempC, 1);
            if (tempC == '$')
            {
                // printf("%s %c\n", comanda, tempC);
                break;
            }
            comanda[i] = tempC;
            i++;
        }
        tempC = 0;
        if (strcmp(comanda, "PING") == 0)
        {
            char ping[] = "PING";
            char pong[] = "PONG";
            write(fd_resp, &ping, 4);
            write(fd_resp, &dollar_sign, 1);

            write(fd_resp, &ver_num, 4);

            write(fd_resp, &pong, 4);
            write(fd_resp, &dollar_sign, 1);
        }

        if (strcmp(comanda, "EXIT") == 0)
        {
            close(fd_req);
            close(fd_resp);
            unlink("RESP_PIPE_44076");
            break;
        }

        if (strcmp(comanda, "CREATE_SHM") == 0)
        {
            read(fd_req, &size, 4);

            shm_memorie_partajata = shm_open("/fq6LQv1", O_CREAT | O_RDWR, 0664);

            char create_shm[] = "CREATE_SHM";

            if (shm_memorie_partajata >= 0)
            {

                ftruncate(shm_memorie_partajata, size);
                data = (volatile char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_memorie_partajata, 0);
                char succes[] = "SUCCESS";

                write(fd_resp, &create_shm, 10);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &succes, 7);
                write(fd_resp, &dollar_sign, 1);
            }
            else
            {
                char error[] = "ERROR";

                write(fd_resp, &create_shm, 10);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
            }
        }

        if (strcmp(comanda, "WRITE_TO_SHM") == 0)
        {
            unsigned int offset = 0, value = 0;
            read(fd_req, &offset, 4);
            read(fd_req, &value, 4);

            char write_to_shm[] = "WRITE_TO_SHM";
            if (offset >= 0 && offset <= 3267632) 
            {
                char *byte = (char *)&value;
                data = data + offset;

                for (int i = 0; i < 4; i++)
                {
                    data[i] = byte[i];
                }

                char succes[] = "SUCCESS";

                write(fd_resp, &write_to_shm, 12);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &succes, 7);
                write(fd_resp, &dollar_sign, 1);
            }
            else
            {
                char error[] = "ERROR";

                write(fd_resp, &write_to_shm, 12);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
            }
        }

        if (strcmp(comanda, "MAP_FILE") == 0)
        {
            int j = 0;
            while (tempC != '$')
            {
                read(fd_req, &tempC, 1);
                if (tempC == '$')
                {
                    break;
                }
                file_name[j] = tempC;
                j++;
            }
            file_name[j] = '\0';
            char map_file[] = "MAP_FILE";

            fd = open(file_name, O_RDONLY);
            file_size = lseek(fd, 0, SEEK_END);
            file = (char *)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

            if (file != (void *)-1)
            {
                char succes[] = "SUCCESS";

                write(fd_resp, &map_file, 8);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &succes, 7);
                write(fd_resp, &dollar_sign, 1);
            }
            else
            {
                char error[] = "ERROR";

                write(fd_resp, &map_file, 8);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
            }
        }

        if (strcmp(comanda, "READ_FROM_FILE_OFFSET") == 0)
        {
            unsigned int offset = 0, no_of_bytes = 0;
            read(fd_req, &offset, 4);
            read(fd_req, &no_of_bytes, 4);

            char read_file_from_offset[] = "READ_FROM_FILE_OFFSET";

            if (offset + no_of_bytes < file_size && file != (void *)-1)
            {
                file = file + offset;
                for (int i = 0; i < no_of_bytes; i++)
                {
                    data[i] = file[i];
                }

                char succes[] = "SUCCESS";

                write(fd_resp, &read_file_from_offset, 21);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &succes, 7);
                write(fd_resp, &dollar_sign, 1);
            }
            else
            {
                char error[] = "ERROR";

                write(fd_resp, &read_file_from_offset, 21);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
            }
        }

        if (strcmp(comanda, "READ_FROM_FILE_SECTION") == 0)
        {
            unsigned int section_no = 0, offset = 0, no_of_bytes = 0;
            read(fd_req, &section_no, 4);
            read(fd_req, &offset, 4);
            read(fd_req, &no_of_bytes, 4);

            char read_file_from_section[] = "READ_FROM_FILE_SECTION";

            if (file[file_size - 1] != 'd')
            {
                printf("MAGIC GRESIT\n");
                char error[] = "ERROR";

                write(fd_resp, &read_file_from_section, 22);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
                continue;
            }

            short *header_size = (short *)(&file[file_size - 3]);

            int header_start = file_size - *header_size;

            short *version = (short *)(&file[header_start]);

            if (*version < 41 || *version > 151)
            {
                char error[] = "ERROR";

                write(fd_resp, &read_file_from_section, 22);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
                continue;
            }

            char *no_of_sections = &file[header_start + 2];

            if (*no_of_sections < 5 || *no_of_sections > 19)
            {
                char error[] = "ERROR";

                write(fd_resp, &read_file_from_section, 22);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
                continue;
            }

            int section_start = header_start + 3 + (section_no - 1) * 27;

            Section_Header *section = (Section_Header *)malloc(sizeof(Section_Header));

            for (int i = 0; i < 17; i++)
            {
                section->sect_name[i] = file[section_start + i];
            }

            section->sect_type = (short *)(file + section_start + 17); 

            if (*section->sect_type != 52 && *section->sect_type != 41 && *section->sect_type != 89)
            {
                char error[] = "ERROR";

                write(fd_resp, &read_file_from_section, 22);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
                continue;
            }
            
            section->sect_offset = (int *)(file + section_start + 19);
            section->sect_size = (int *)(file + section_start + 23);

            if (offset + no_of_bytes > *section->sect_size)
            {

                char error[] = "ERROR";

                write(fd_resp, &read_file_from_section, 22);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
            }
            else
            {
                file = file + offset + *section->sect_offset;
                for (int i = 0; i < no_of_bytes; i++)
                {
                    data[i] = file[i];
                }

                file = file - (offset + *section->sect_offset);

                char succes[] = "SUCCESS";

                write(fd_resp, &read_file_from_section, 22);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &succes, 7);
                write(fd_resp, &dollar_sign, 1);
            }
            free(section);
        }

        if (strcmp(comanda, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {
            unsigned int logical_offset = 0, no_of_bytes = 0;
            read(fd_req, &logical_offset, 4);
            read(fd_req, &no_of_bytes, 4);

            char read_from_logical_space_offset[] = "READ_FROM_LOGICAL_SPACE_OFFSET";

            if (file[file_size - 1] != 'd')
            {
                char error[] = "ERROR";

                write(fd_resp, &read_from_logical_space_offset, 30);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
                continue;
            }

            short *header_size = (short *)(&file[file_size - 3]);

            int header_start = file_size - *header_size;

            short *version = (short *)(&file[header_start]);

            if (*version < 41 || *version > 151)
            {
                char error[] = "ERROR";

                write(fd_resp, &read_from_logical_space_offset, 30);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
                continue;
            }

            char *no_of_sections = &file[header_start + 2];

            if (*no_of_sections < 5 || *no_of_sections > 19)
            {
                char error[] = "ERROR";

                write(fd_resp, &read_from_logical_space_offset, 30);
                write(fd_resp, &dollar_sign, 1);

                write(fd_resp, &error, 5);
                write(fd_resp, &dollar_sign, 1);
                continue;
            }

            Section_Header sections[*no_of_sections + 1];

            int cursor = header_start + 3;
            for (int i = 1; i <= *no_of_sections; i++)
            {
                for (int i = 0; i < 17; i++)
                {
                    sections[i].sect_name[i] = file[cursor + i];
                }
                cursor = cursor + 17;
                sections[i].sect_type = (short *)(&file[cursor]);
                cursor = cursor + 2;

                if (*sections[i].sect_type != 52 && *sections[i].sect_type != 41 && *sections[i].sect_type != 89)
                {
                    char error[] = "ERROR";

                    write(fd_resp, &read_from_logical_space_offset, 30);
                    write(fd_resp, &dollar_sign, 1);

                    write(fd_resp, &error, 5);
                    write(fd_resp, &dollar_sign, 1);
                    continue;
                }

                sections[i].sect_offset = (int *)(&file[cursor]);
                cursor = cursor + 4;
                sections[i].sect_size = (int *)(&file[cursor]);
                cursor = cursor + 4;
            }

            int data_iterator = 0;
            int contor_bytes = 0;

            for (int i = 1; i <= *no_of_sections; i++)
            {
                cursor = *sections[i].sect_offset;

                for (int j = 0; j < *sections[i].sect_size; j++)
                {
                    if (data_iterator >= logical_offset && contor_bytes < no_of_bytes)
                    {
                        data[contor_bytes] = file[cursor];
                        contor_bytes++;
                        if (contor_bytes == no_of_bytes)
                        {
                            char succes[] = "SUCCESS";

                            write(fd_resp, &read_from_logical_space_offset, 30);
                            write(fd_resp, &dollar_sign, 1);

                            write(fd_resp, &succes, 7);
                            write(fd_resp, &dollar_sign, 1);
                        }
                    }
                    cursor++;
                    data_iterator++;
                }
                data_iterator = (data_iterator / 2048) * 2048 + 2048;
            }
        }
    }
    return 0;
}

/*
 *  Copies the contents of one file to another
 *  Compile using: cc copy.c -o copy
 *  Run using: ./copy source destination
 *  Also support: ./copy -h
 *  and support: ./copy --help
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define  CHUNK_SIZE 1024

int main(int argc, char* argv[])
{   
    char buffer[CHUNK_SIZE];
    int sourceFd, destinationFd;
    int numRead, numWritten, status;

    if (argc != 3 || 
        strcmp(argv[1], "-h") == 0 ||
        strcmp(argv[1], "--help") == 0
    )
    {
        printf("Usage: %s source destination\n", argv[0]);
        exit(1);
    }   

    sourceFd = open(argv[1], O_RDONLY);
    if (sourceFd == -1)
    {
        printf("%s: No such file or directory", argv[1]);
        exit(1);
    }

    destinationFd = open(
        argv[2], 
        O_CREAT | O_WRONLY | O_APPEND, 
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH  /* rw-r-xr-x*/
    );
    if (destinationFd == -1)
    {
        printf("Failed to open destination file: %s", argv[2]);
        exit(1);

    }

    numRead = read(sourceFd, buffer, CHUNK_SIZE);

    while (numRead > 0)
    {
        numWritten = write(destinationFd, buffer, numRead);
        if (numWritten < numRead)
        {
            printf("Partial write error on chunk.\n");
            exit(1);
        }
        
        numRead = read(sourceFd, buffer, CHUNK_SIZE);        
    }

    if (numRead == -1)
    {
        printf("Failed to read from the source file.\n");
        exit(1);
    }

    status = close(sourceFd);
    if (status == -1)
    {
        printf("Failed to close source file.\n");
        exit(1);
    }

    status = close(destinationFd);
    if (status == -1)
    {
        printf("Failed to close destination file.\n");
        exit(1);
    }
}
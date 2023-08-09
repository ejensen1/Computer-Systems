#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//

int main() {

    // read from stdin

    char buf[65536];
    int bytes = 0;
    int total = 0;

    //while ((bytes = read(STDIN_FILENO,buf,65536)) > 0) {
    //  total += bytes;
    //  if (total == 65536) {
    //    break;
    //  }
    //}

    //do {
    //  bytes = read(STDIN_FILENO,buf,65536);
    //} while (bytes >0);

    while ((bytes = read(STDIN_FILENO, buf + total, 65536 - total)) > 0) {
        total += bytes;
        if (total == 65536) {
            //printf("buf = %s\n", buf);
            break;
        }
    }
    //char command[20];
    char file_name[256];
    char file[65536];

    char dummy[65536];

    sscanf(buf, "%s %s %s", dummy, file_name, file);

    char *command = strtok(buf, " ");
    if ((strcmp(command, "get") == 0) && strcmp(file, "")) {
        fprintf(stderr, "Operation Failed\n");
        return 1;
    }

    //sscanf(buf, "%s %s\n", command, file_name);
    //while(sscanf(buf, "%s %s %s", command, file_name, file)==1){
    //  if((strcmp(command,"get") != 0) && (strcmp(command,"set") != 0)){
    //    fprintf(stderr, "Operation Failed\n");
    //    return 1;
    //  } else {
    //    break;
    //  }
    //}
    //printf("file name= %s\n", file_name);

    //printf("%d\n",fd);

    if (strcmp(command, "get") == 0) {
        int fd = open(file_name, O_RDWR | O_RDWR | O_SYNC);
        if (fd < 0) {
            fprintf(stderr, "Operation Failed\n");
            close(fd);
            return 1;
        }
        //printf("get\n");
        bytes = 0;
        total = 0;
        char buf2[65536];

        do {
            bytes = read(fd, buf2, 65536);
            write(STDOUT_FILENO, buf2, bytes);
        } while (bytes > 0);

        //while ((bytes = read(fd, buf2 + total, 65536 - total)) > 0) {
        //    //printf("here\n");
        //    total += bytes;
        //    write(STDOUT_FILENO,buf2, bytes);
        //    //printf("%s\n", buf2);
        //    if (total == 65536) {
        //        break;
        //    }
        //}
    } else if (strcmp(command, "set") == 0) {
        sscanf(buf, "%s %s\n%s", command, file_name, file);
        //        int fd_write = open(file_name, O_RDWR | O_APPEND);
        //       if (fd_write < 0) {
        //           fprintf(stderr, "Operation Failed\n");
        //           close(fd_write);
        //           return 1;
        //       }
        //char buf4[65536];
        int fd = open(file_name, O_WRONLY | O_TRUNC);
        if (fd < 0) {
            close(fd);
            fprintf(stderr, "Operation Failed\n");
            return 1;
        }
        //printf("fd = %d\n", fd);
        //printf("file = %s\n", file);
        //printf("strlen(file) = %lu\n",strlen(file));
        write(fd, file, strlen(file));
        //do {
        //  bytes = read(STDIN_FILENO, buf4, 65536);
        //  write(fd_write, buf4, bytes);
        //} while(bytes >0);
        //printf("fd_write = %d\n", fd_write);

        //char command[20];
        //char file_name[20];
        //char file[20];

        //char *buf3 = strstr(buf, "\n");
        //char *buf4 = NULL;
        //sscanf(buf3,"\n %s", buf4);
        //printf("buf3 = %s\n", buf3);
        //printf("buf4 = %s\n", buf4);
        write(STDOUT_FILENO, "OK\n", 3);
        close(fd);

        return 0;
    } else {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }

    return 0;

    // identify get or set

    //printf("buf = %s\n", buf);
    //char* command = strtok(buf, " ");
    //char* file_name = strtok(NULL, " ");
    printf("file_name len %lu", strlen(file_name));

    int fd_read = open(file_name, O_RDWR);
    printf("fd_read = %d\n", fd_read);

    // perform get or set

    if (strcmp(command, "get ")) {
        printf("get\n");
        bytes = 0;
        total = 0;
        printf("aa = %zd", read(fd_read, buf + total, 65536 - total));
        while ((bytes = read(fd_read, buf + total, 65536 - total)) > 0) {
            total += bytes;
            if (total == 65536) {
                printf("writing to stdout\n");
                printf("read buf = %s ", buf);
                break;
            }
        }
    } else {
        printf("not get\n");
    }
    return 0;

    //do {
    //    bytes_read = read(STDIN_FILENO, buf, 65536);
    //    if (bytes_read < 0) {
    //        fprintf(stderr, "Unable to read, %d\n", errno);
    //        return 1;
    //    } else if (bytes_read > 0) {
    //        int bytes_written = 0;
    //        do {
    //            int bytes = write(STDOUT_FILENO,
    //                              buf + bytes_written,
    //                              bytes_read - bytes_written);
    //            if (bytes <= 0) {
    //                fprintf(stderr, "Cannot write to STDOUT\\n");
    //            }
    //            bytes_written += bytes;
    //        } while(bytes_written < bytes_read);
    //    }
    //} while(bytes_read > 0);

    //return 0;

    //char *p;
    //scanf("%ms", &p);
    //printf("p = %s", p);
    //return 0;

    //size_t cap = 65536, len = 0;
    //char *buf = malloc(cap * sizeof(char));
    //char *c;
    //while (scanf("%s", c) > 0) {
    //    //buf[len] = c;
    //    if (++len == cap) {
    //        buf = realloc(buf, (cap  *= 2) * sizeof (char));
    //    }
    //}

    ////buf = realloc(buf, (len + 1) * sizeof (char));
    ////buf[len] = '\0';
    //printf("buf = %s\n", buf);
    //char* command = strtok(buf, " ");
    //char* read_file = strtok(NULL, " ");
    //printf("command = %s\n",command);
    //printf("read_file = %s\n", read_file);
    //return 0;

    //// Handle input from stdin
    ////FILE *infile = stdin;
    ////char *buffer;
    ////size_t bufsize = 32;
    ////char buf[65536];
    ////int fd = open(infile, "r");
    ////
    //while (scanf("%s", buf) > 0){

    //}
    ////scanf("%s", buf);
    //printf("\nbuf = %s\n", buf);

    //if (strcmp(command, "get")){
    //    printf("get\n");
    //}
    //return 0;

    ////while (scanf("%s", buf) != EOF){
    ////    printf("buf %s\n",buf);
    ////}

    ////return 0;

    ////while ((ret = read(fd, buf, sizeof(buf)-1)) >0) {
    ////    buf[ret] = 0x00;
    ////    printf("block read: \n<%s>\n", buf);
    ////}
    ////return 0;

    //    buffer = (char *)malloc(bufsize * sizeof(char));
    //    getline(&buffer,&bufsize,infile);
    //    printf("%s", buffer);
    //
    //    // Parse string
    //    char* command = strtok(buffer, " ");
    //    char* read_file = strtok(NULL, " ");
    //    char* write_file = strtok(NULL, "\n");
    //
    //    printf("command = %s\n",command);
    //    printf("read_file = %s\n", read_file);
    //    printf("write_file = %s\n", write_file);
    //
    //
    //
    //    // Free memory
    //    free(buffer);
    //
    //    return 0;
}

//loop
//
//if iteration == 1
// use str tok
// to  set command and read/write file
// else {
//     if set
//         write to file
//    else;
//
// }
//
// how do you terminate stdin?:wq
//
//
// I want to test my code
// lets start by reading the

// read wroks for small files
// write prints to stdout correclty but does not write correctly

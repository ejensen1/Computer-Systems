// standard stuff
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

// error handling
#include <err.h>
#include <errno.h>
#include <assert.h>

// network / regex
#include <sys/socket.h>
#include <netinet/in.h>
#include <regex.h>
#include <sys/stat.h>

// resources helper functions
#include "asgn2_helper_funcs.h"

#define BUFSIZE 4096

#define PARSE_REGEX "^[a-zA-Z0-9._]+$"

#define response200 "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n"
#define response201 "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n"
#define response400 "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n"
#define response403 "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n"
#define response404 "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n"
#define response500                                                                                \
    "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 22\r\n\r\nInternal Server Error\n"
#define response501 "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n"
#define response505                                                                                \
    "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion Not Supported\n"

struct Request {
    bool isGet;
    bool isPut;
    bool isImplementedMethod;
    bool versionIsInvalid;
    bool isInvalid;
    bool fileExists;
    bool is403;
    char uri[63];
    int infd;
    int outfd;
    char file_name[128];
    int cl;
};

// returns:
//  0 if get
//  1 if put
// -1 if invalid
//  2 if unimplemented method
int check_method(char *method) {
    if (strcmp(method, "GET") == 0) {
        return 0;
    } else if (strcmp(method, "PUT") == 0) {
        return 1;
        //}else if (strcmp(method, "HEAD") == 0){
        //    return 2;
        //}else if (strcmp(method, "POST") == 0){
        //    return 2;
        //}else if (strcmp(method, "PATCH") == 0){
        //    return 2;
        //}else if (strcmp(method, "TRACE") == 0){
        //    return 2;
        //}else if (strcmp(method, "OPTIONS") == 0){
        //    return 2;
        //}else if (strcmp(method, "DELETE") == 0){
        //    return 2;
    } else {
        //return -1;
        return 2;
    }
    return 0;
}
// returns:
// -1 if invalid
// 0 if valid
int check_uri(char *uri) {

    // check that first character is "/"
    if ((int) uri[0] != 47) {
        return -1;
    }

    // check that length is in between 2 and 64 (includsive)
    int len = strlen(uri);
    if (len < 2) {
        return -1;
    }
    if (len > 64) {
        return -1;
    }

    char s[len - 1];
    sscanf(uri, "/%s", s);
    // regex time
    regex_t re;
    //regmatch_t matches[3];
    //regmatch_t matches[3];
    int rc;
    rc = regcomp(&re, PARSE_REGEX, REG_EXTENDED);
    assert(!rc);
    rc = regexec(&re, s, 0, NULL, 0);
    if (rc == REG_NOMATCH) {
        //printf("bad uri\n");
        regfree(&re);
        return -1;
    } else {
        //printf("good uri\n");
    }

    regfree(&re);

    return 0;
}

//returns 0 if valid
// returns -1 if invalid
// retursn 1 if not supported
int check_version(char *version) {
    //printf("check_version\n");
    if (strcmp(version, "HTTP/1.1") == 0) {
        return 0;
    }
    regex_t re;
    //"^([A-Z{1,8}) +(/[a-zA-Z0-9._]{1,63}) +(HTTP/1\\.1)$"
    int rc = regcomp(&re, "HTTP/[0-9].[0-9]$", REG_EXTENDED);
    //int rc = regcomp(&re, "(HTTP/1.1)$",REG_EXTENDED);
    assert(!rc);
    rc = regexec(&re, version, 0, NULL, 0);
    //printf("rc = %d\n", rc);
    if (rc == REG_NOMATCH) {
        //printf("400\n");
        regfree(&re);
        return -1;
    } else {
        //printf("504\n");
        regfree(&re);
        return 1;
    }
    regfree(&re);
    return 0;
}

struct Request parse(char *header, int socket_fd) {
    char method[1024] = { 0 };
    char uri[1024] = { 0 };
    char version[1024] = { 0 };
    sscanf(header, "%s %s %s\r\n\r\n", method, uri, version);
    //printf("header = %s\n", header);
    struct Request r;
    r.isGet = 0;
    r.isPut = 0;
    r.isImplementedMethod = 1;
    r.versionIsInvalid = 0;
    r.isInvalid = 0;
    r.fileExists = 0;
    r.is403 = 0;

    int check_method_result = check_method(method);

    // CHECK METHOD

    if (check_method_result == 0) {
        r.isGet = 1;
        r.isPut = 0;
        r.isInvalid = 0;
        r.isImplementedMethod = 1;
    } else if (check_method_result == 1) {
        r.isGet = 0;
        r.isPut = 1;
        r.isInvalid = 0;
        r.isImplementedMethod = 1;
    } else if (check_method_result == 2) {
        r.isGet = 0;
        r.isPut = 0;
        r.isInvalid = 0;
        r.isImplementedMethod = 0;
        return r; // we can cut short because we cannot process this request
    } else { // check_method_result == -1
        r.isGet = 0;
        r.isPut = 0;
        r.isInvalid = 1;
        r.isImplementedMethod = 1; // this is strange, but it makes it send 501
        return r;
    }

    // CHECK URI

    int check_uri_result = check_uri(uri);
    if (check_uri_result == -1) {
        r.isInvalid = 1;
        return r;
    }

    char file_name[strlen(uri) - 1];
    sscanf(uri, "/%s", file_name);
    strcpy(r.file_name, file_name);

    // Check if file exists
    if (access(file_name, F_OK) == 0) {
        r.fileExists = 1;
    } else {
        r.fileExists = 0;
    }

    // return early if 404
    if (r.isGet == 1 && r.fileExists == 0) {
        return r;
    }

    // populate infile and outfile
    if (r.isGet == 1 && r.fileExists == 1) {
        r.infd = open(file_name, O_RDONLY);
        if (r.infd == EACCES) {
            r.is403 = 1;
            return r;
        }
        r.outfd = socket_fd;
    } else if (r.isPut == 1) {
        r.infd = socket_fd;
        r.outfd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT);
    }

    // CHECK VERSION

    int check_version_result = check_version(version);

    if (check_version_result == -1) {
        r.isInvalid = 1;
        r.versionIsInvalid = 0;
        return r;
    } else if (check_version_result == 1) {
        r.versionIsInvalid = 1;
        return r;
    }

    if (r.isPut == 1) {
        char *x = strstr(header, "Content-Length");
        if (x) {

            char y[4096] = { 0 };
            char z[4096] = { 0 };
            sscanf(x, "Content-Length %s\r\n%s", y, z);

            r.cl = atoi(z);
        } else {
            r.isInvalid = 1;
            return r;
        }
    }

    return r;
}

void process_bad_request(int status_code, int out) {

    if (status_code == 400) {
        write(out, response400, strlen(response400));
    } else if (status_code == 403) {
        write(out, response403, strlen(response403));
    } else if (status_code == 404) {
        write(out, response404, strlen(response404));
    } else if (status_code == 500) {
        write(out, response500, strlen(response500));
    } else if (status_code == 501) {
        write(out, response501, strlen(response501));
    } else if (status_code == 505) {
        write(out, response505, strlen(response505));
    }
    close(out);
    return;
}

void process_get(char *file_name, int in, int out) {
    write(out, "HTTP/1.1 200 OK\r\nContent-Length: ", 33);

    struct stat sb;
    if (stat(file_name, &sb) != 0) {
        fprintf(stderr, "Stat failed\n");
        return;
    }
    int sz = sb.st_size;

    char str[128] = { 0 };
    int len = sprintf(str, "%d", sz);
    write(out, str, len);
    write(out, "\r\n\r\n", 4);
    pass_bytes(in, out, sz);
    close(out);
    close(in);

    return;
}

void put_new(int sock, int out, int cl, bool small_put, char *AB) {
    if (small_put) {
        char *mb = strstr(AB, "\r\n\r\n");
        if (strcmp(mb, "\r\n\r\n") == 0) {
            //printf("AB does not include message body!\n");
            pass_bytes(sock, out, cl);
            write(sock, response201, strlen(response201));
        } else {
            //printf("AB includes the header and the message body\n");
            mb = strtok(mb, "\r\n\r\n");
            write(out, mb, strlen(mb)); // problematic
            write(sock, response201, strlen(response201));
        }
    }
    close(out);
    return;
}

void put_old(int sock, int out, int cl, bool small_put, char *AB) {
    if (small_put) {
        char *mb = strstr(AB, "\r\n\r\n");
        if (strcmp(mb, "\r\n\r\n") == 0) {
            //printf("AB does not include message body!\n");
            pass_bytes(sock, out, cl);
            //char buf[4096] = { 0 };
            //read(sock, buf, cl);
            //write(out, buf, cl);
            write(sock, response200, strlen(response200));
        } else {
            //printf("AB includes the header and the message body\n");
            mb = strtok(mb, "\r\n\r\n");
            write(out, mb, strlen(mb)); // problematic
            write(sock, response200, strlen(response200));
        }
    }
    close(out);
    return;
}

int main(int argc, char *argv[]) {

    // Check number of command line arguments
    if (argc != 2) {
        fprintf(stderr, "Incorrect number of command line arguments\n");
        return 1;
    }
    int port = atoi(argv[1]);

    // Validate port number
    if (port > 65535 || port < 0) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }

    // Open socket
    Listener_Socket sock;
    if (listener_init(&sock, port) < 0) {
        fprintf(stderr, "Error initializing port\n");
        return 1;
    }

    char buf[BUFSIZE + 1] = { 0 };
    while (1) {
        int listenfd = listener_accept(&sock);

        if (listenfd > 0) {
            int bytes_read = read_until(listenfd, buf, 4096, "\r\n\r\n");
            buf[bytes_read] = 0;
            if (bytes_read < 1) {
                fprintf(stderr, "Error reading from socket\n");
                close(listenfd);
                return 1;
            }

            // sort out separating header from body
            struct Request r;
            r.isGet = 0;
            r.isPut = 0;
            r.isImplementedMethod = 1;
            r.versionIsInvalid = 0;
            r.isInvalid = 0;
            r.fileExists = 0;
            r.is403 = 0;
            //struct Request {
            //    bool isGet;
            //    bool isPut;
            //    bool isImplementedMethod;
            //    bool versionIsInvalid;
            //    bool isInvalid;
            //    bool fileExists;
            //    bool is403;
            //    char uri[63];
            //    int infd;
            //    int outfd;
            //    char file_name[128];
            //    int cl;
            r = parse(buf, listenfd);

            int status_code = 0;
            if (r.isImplementedMethod == 0) {
                status_code = 501;
            } else if (r.versionIsInvalid == 1) {
                status_code = 505;
            } else if (r.is403 == 1) {
                status_code = 403;
            } else if (r.fileExists == 0 && r.isGet == 1) {
                status_code = 404;
            } else if (r.isInvalid == 1) {
                status_code = 400;
            } else if (r.fileExists == 1 && r.isGet == 1) {
                status_code = 200;
            } else if (r.fileExists == 0 && r.isPut == 1) {
                status_code = 201;
            } else if (r.fileExists == 1 && r.isPut == 1) {
                status_code = 200;
            }
            //process_bad_request(status_code, listenfd);

            bool small_put = 0;
            if (bytes_read < 4096) {
                small_put = 1;
            }

            //char *result = strstr(buf, "\r\n\r\n");
            //int position = result - buf;
            if (status_code > 201) {
                process_bad_request(status_code, listenfd);
            } else {
                if (r.isGet == 1) {
                    process_get(r.file_name, r.infd, r.outfd);
                } else if (r.isPut == 1 && r.fileExists == 0) {
                    put_new(listenfd, r.outfd, r.cl, small_put, buf);
                } else if (r.isPut == 1 && r.fileExists == 1) {
                    put_old(listenfd, r.outfd, r.cl, small_put, buf);
                }
            }

            r.isImplementedMethod = 1;
            r.versionIsInvalid = 0;
            r.is403 = 0;

            close(listenfd);
        }
    }

    return 0;
}

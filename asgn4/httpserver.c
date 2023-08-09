#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "response.h"
#include "request.h"
#include "queue.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <pthread.h>
#include <sys/file.h>

pthread_mutex_t mutex;

void handle_connection(int);

struct Log_Entry {
    char type[16];
    char uri[63];
    uint16_t status_code;
    char request_id[200];
};

int handle_get(conn_t *);
int handle_put(conn_t *);
void handle_unsupported(conn_t *);

void audit_log(struct Log_Entry l) {
    if (strcmp(l.request_id, "") != 0) {
        fprintf(stderr, "%s,/%s,%d,%s\n", l.type, l.uri, l.status_code, l.request_id);
    } else {
        fprintf(stderr, "%s,/%s,%d,0\n", l.type, l.uri, l.status_code);
    }
    return;
}

void *worker_code(void *q) {
    uintptr_t r;
    int connfd;
    while (1) {
        queue_pop(q, (void **) &r);
        //printf("popped %lu\n", r);
        connfd = (int) r;
        handle_connection(connfd);
        close(connfd);
        //printf("fd = %d\n", fd);
    }
    return q;
}

int main(int argc, char **argv) {
    pthread_mutex_init(&mutex, NULL);

    int num_workers = 4;

    int opt = 0;
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't': num_workers = atoi(optarg); break;
        }
    }

    if (argc < 2) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr = NULL;
    size_t port;
    if (argc == 2) {
        port = (size_t) strtoull(argv[1], &endptr, 10);
    } else {
        port = (size_t) strtoull(argv[3], &endptr, 10);
    }

    if (endptr && *endptr != '\0') {
        warnx("invalid port number: %s", argv[1]);
        return EXIT_FAILURE;
    }

    pthread_t threads[num_workers];
    queue_t *q = queue_new(5 * num_workers);
    for (int i = 0; i < num_workers; ++i) {
        pthread_create(&threads[i], NULL, worker_code, q);
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    while (1) {
        int connfd = listener_accept(&sock);
        queue_push(q, (void *) (uintptr_t) connfd);
        //handle_connection(connfd);
        //close(connfd);
    }
    queue_delete(&q);

    return EXIT_SUCCESS;
}

void handle_connection(int connfd) {

    conn_t *conn = conn_new(connfd);

    const Response_t *res = conn_parse(conn);

    struct Log_Entry l;
    int status_code = 0;
    if (res != NULL) {
        //printf("status_code: %d\n", response_get_code(res));
        //printf("message: %s\n", response_get_message(res));
        conn_send_response(conn, res);
    } else {

        //debug("%s", conn_str(conn));
        const Request_t *req = conn_get_request(conn);
        if (req == &REQUEST_GET) {
            status_code = handle_get(conn);
            strcpy(l.type, "GET");
        } else if (req == &REQUEST_PUT) {
            status_code = handle_put(conn);
            strcpy(l.type, "PUT");
        } else {
            handle_unsupported(conn);
            strcpy(l.type, "UNSUPPORTED");
        }
        strcpy(l.uri, conn_get_uri(conn));

        if (conn_get_header(conn, "Request-Id")) {
            strcpy(l.request_id, conn_get_header(conn, "Request-Id"));
        }
        l.status_code = status_code;

        audit_log(l);
    }

    conn_delete(&conn);
}

int handle_get(conn_t *conn) {
    int status_code;

    char *uri = conn_get_uri(conn);
    //debug("GET request not implemented. But, we want to get %s", uri);

    // What are the steps in here?

    // 1. Open the file.
    // If  open it returns < 0, then use the result appropriately
    //   a. Cannot access -- use RESPONSE_FORBIDDEN
    //   b. Cannot find the file -- use RESPONSE_NOT_FOUND
    //   c. other error? -- use RESPONSE_INTERNAL_SERVER_ERROR
    // (hint: check errno for these cases)!

    int fd = open(uri, O_RDONLY);
    if (fd < 0) {
        conn_send_response(conn, &RESPONSE_NOT_FOUND);

        status_code = 404;
        return 404;
    }
    flock(fd, LOCK_SH);

    // 2. Get the size of the file.
    // (hint: checkout the function fstat)!
    struct stat sb;
    stat(uri, &sb);
    int sz = sb.st_size;

    // Get the size of the file.

    // 3. Check if the file is a directory, because directories *will*
    // open, but are not valid.
    // (hint: checkout the macro "S_IFDIR", which you can use after you call fstat!)

    // 4. Send the file
    // (hint: checkout the conn_send_file function!)
    const Response_t *res = conn_send_file(conn, fd, sz);
    status_code = 200;
    if (status_code) {
    }
    if (res) {
        printf("res defined\n");
        conn_send_response(conn, res);
        response_get_code(res);
    } else {
    }
    flock(fd, LOCK_UN);
    return status_code;
}

void handle_unsupported(conn_t *conn) {
    debug("handling unsupported request");

    // send responses
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
}

int handle_put(conn_t *conn) {
    int status_code;

    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;

    //debug("handling put request for %s", uri);

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;
    //debug("%s existed? %d", uri, existed);
    pthread_mutex_lock(&mutex);
    if (existed) {
        status_code = 200;
    } else {
        status_code = 201;
    }

    // Open the file..
    //int fd = open(uri, O_CREAT | O_WRONLY, 0600);
    int fd = open(uri, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) {
        //debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }
    flock(fd, LOCK_EX);
    //ftruncate(fd,0);
    pthread_mutex_unlock(&mutex);

    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
        conn_send_response(conn, res);
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
        conn_send_response(conn, res);
    }
    //status_code = response_get_code(res);
    if (status_code) {
    }
    flock(fd, LOCK_UN);

    close(fd);
    return status_code;

out:
    conn_send_response(conn, res);
    return status_code;
}

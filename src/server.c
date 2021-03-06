/**
 * webserver.c -- A webserver written in C
 *
 * Test with curl (if you don't have it, install it):
 *
 *    curl -D - http://localhost:3490/
 *    curl -D - http://localhost:3490/d20
 *    curl -D - http://localhost:3490/date
 *
 * You can also test the above URLs in your browser! They should work!
 *
 * Posting Data:
 *
 *    curl -D - -X POST -H 'Content-Type: text/plain' -d 'Hello, sample data!' http://localhost:3490/save
 *
 * (Posting data is harder to test from a browser.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include "net.h"
#include "file.h"
#include "mime.h"
#include "cache.h"

#define PORT "3490"  // the port users will be connecting to

#define SERVER_FILES "./serverfiles"
#define SERVER_ROOT "./serverroot"

/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 *
 * Return the value from the send() function.
 */
int send_response(int fd, char *header, char *content_type, void *body, int content_length)
{
        // this function builds an HTTP response with all the given parameters: header, content_type, body the actual
        // content of the request, and content_length the total length of the header and body of the response in bytes
        // this is so the send() function below  recieves the respond legnth which it needs to send a message on a socket.
        const int max_response_size = 65536;
        char response[max_response_size];
        // response is an array of characers so do I need:
        // response_length = sizeof(respone)/sizeof(char)?
        // just initalize response length for now
        // date vars:
        time_t unstructdate;
        struct tm *date;
        time(&unstructdate);
        date = localtime(&unstructdate);
        //make body into char:
        char* body_char;
        body_char = (char*)body;



        // Build HTTP response and store it in response
        int response_length = 0;

        response_length = sprintf(response, "%s\n connection: close\n content_length: %d\n content_type: %s\n date: %s\n%s", header, content_length, content_type, asctime(date), body_char);

        ///////////////////
        // IMPLEMENT ME! //
        ///////////////////

        // Send it all!
        int rv = send(fd, response, response_length, 0);

        if (rv < 0) {
                perror("send");
        }

        return rv;
}


/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd)
{
        // Generate a random number between 1 and 20 inclusive
        // need length as well for send response
        int random, len;
        // array of characters for sprintf
        char body_num[15];
        random = 1 + rand()%21;
        len = sprintf(body_num, "%d\n", random);


        // Use send_response() to send it back as text/plain data

        ///////////////////
        // IMPLEMENT ME! //
        ///////////////////
        send_response(fd, "HTTP/1.1 200 OK", "text/plain", body_num, len);
}

/**
 * Send a 404 response
 */
void resp_404(int fd)
{
        char filepath[4096];
        struct file_data *filedata;

        char *mime_type;

        // Fetch the 404.html file
        snprintf(filepath, sizeof filepath, "%s/404.html", SERVER_FILES);
        filedata = file_load(filepath);


        if (filedata == NULL) {
                resp_404(fd);
                fprintf(stderr, "cannot find system 404 file\n");
                //exit(3);
                return;
        }

        // need mime type of path in cache
        mime_type = mime_type_get(filepath);

        send_response(fd, "HTTP/1.1 404 NOT FOUND", mime_type, filedata->data, filedata->size);

        file_free(filedata);
}

/**
 * Read and return a file from disk or cache
 */
void get_file(int fd, struct cache *cache, char *request_path)
{
        ///////////////////
        // IMPLEMENT ME! //
        ///////////////////
        char path[4096];
        // from file.c:
        struct file_data *filedata;
        // cache entry, check to see if pointer is in the hash table path:
        struct cache_entry *cache_check = cache_get(cache, path);
        //for content-type header
        char *mime_type;
        // print out filepath conditional on the endpint, using snprintf to limit size of buffer:
        sprintf(path, "%s%s", SERVER_ROOT, request_path);

        // if (strcmp(path, "/index.html") == 0) {
        //         snprintf(path, sizeof(path), "%s%s", SERVER_ROOT, "/index.html");
        // } else {
        //         snprintf(path, sizeof(path), "%s%s", SERVER_ROOT, request_path);
        // }
        //load the file
        //filedata = file_load(file);

        //check result of cache_get:
        if(cache_check == NULL){
          // if it's not there load the file from the disk
          filedata = file_load(path);
          //also from file.c
          // if there is no file data: 404
          if(filedata == NULL) {
                  resp_404(fd);
                  //printf("\n%what's going on\n");
                  return;
          }

          mime_type = mime_type_get(path);
          //store entry into cache with cache_put:
          cache_put(cache, path, mime_type, filedata->data, filedata->size);
          // serve it back, resposne filedata->data is the body, and filedata->size is size of the file in bytes.
          send_response(fd,"HTTP/1.1 200 OK",  mime_type, filedata->data, filedata->size);
          // which frees the struct and the filedata member
          file_free(filedata);
        } else{
          // if it's there serve it back:
          send_response(fd, "HTTP/1.1 200 OK ", cache_check->content_type, cache_check->content, cache_check->content_length);
          printf("this came from da cache\n" );index
        }

}

/**
 * Search for the end of the HTTP header
 *
 * "Newlines" in HTTP can be \r\n (carriage return followed by newline) or \n
 * (newline) or \r (carriage return).
 */
char *find_start_of_body(char *header)
{
        ///////////////////
        // IMPLEMENT ME! // (Stretch)
        ///////////////////
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache)
// this function is where the request is recieved.
{
        const int request_buffer_size = 65536; // 64K
        char request[request_buffer_size];

        // Read request
        int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

        if (bytes_recvd < 0) {
                perror("recv");
                return;
        }


        ///////////////////
        // IMPLEMENT ME! //
        ///////////////////

        // Read the three components of the first request line
        printf("request %s\n", request);
        char method[10], file[30], protocol[30];
        //On success, the function returns the number of variables filled.
        //In the case of an input failure before any data could be
        //successfully read,EOF is returned.
        sscanf(request, "%s %s %s", method, file, protocol);
        printf("method:%s\n file:%s", method, file );

        // If GET, handle the get endpoints
        if(strcmp(method, "GET") == 0) {
                //    Check if it's /d20 and handle that special case
                if(strcmp(file, "/d20") == 0) {
                        get_d20(fd);
                }  else {
                        //    Otherwise serve the requested file by calling get_file()
                        get_file(fd, cache, file);
                        //resp_404(fd);
                }
        }



        // (Stretch) If POST, handle the post request
}

/**
 * Main
 */
int main(void)
{
        int newfd; // listen on sock_fd, new connection on newfd
        struct sockaddr_storage their_addr; // connector's address information
        char s[INET6_ADDRSTRLEN];

        struct cache *cache = cache_create(10, 0);

        // Get a listening socket
        int listenfd = get_listener_socket(PORT);

        if (listenfd < 0) {
                fprintf(stderr, "webserver: fatal error getting listening socket\n");
                exit(1);
        }

        printf("webserver: waiting for connections on port %s...\n", PORT);

        // This is the main loop that accepts incoming connections and
        // forks a handler process to take care of it. The main parent
        // process then goes back to waiting for new connections.

        while(1) {
                socklen_t sin_size = sizeof their_addr;

                // Parent process will block on the accept() call until someone
                // makes a new connection:
                newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
                if (newfd == -1) {
                        perror("accept");
                        continue;
                }

                // Print out a message that we got the connection
                inet_ntop(their_addr.ss_family,
                          get_in_addr((struct sockaddr *)&their_addr),
                          s, sizeof s);
                printf("server: got connection from %s\n", s);

                // newfd is a new socket descriptor for the new connection.
                // listenfd is still listening for new connections.

                handle_http_request(newfd, cache);

                close(newfd);
        }

        // Unreachable code
        // socket has been closed can't send any responses.

        return 0;
}

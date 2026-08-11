#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "web/client.h"
#include "web/http.h"
#include "web/server.h"

#define CHECK(expr)                                                                  \
    do {                                                                             \
        if (!(expr)) {                                                               \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            return 1;                                                                \
        }                                                                            \
    } while (0)

static int test_request_parser(void)
{
    const char request[] = "POST /submit?a=1 HTTP/1.1\r\nHost: example\r\n"
                           "Content-Length: 3\r\n\r\nabc";
    web_request_t* parsed = NULL;
    CHECK(web_request_parse(request, sizeof(request) - 1, &parsed) == 0);
    CHECK(parsed != NULL);
    CHECK(parsed->method == WEB_METHOD_POST);
    CHECK(strcmp(parsed->path, "/submit") == 0);
    CHECK(strcmp(parsed->query, "a=1") == 0);
    CHECK(parsed->body_len == 3 && memcmp(parsed->body, "abc", 3) == 0);
    web_request_destroy(parsed);

    CHECK(web_request_parse("POST / HTTP/1.1\r\nContent-Length: 4\r\n\r\nabc",
                            sizeof("POST / HTTP/1.1\r\nContent-Length: 4\r\n\r\nabc") - 1,
                            &parsed) < 0);
    CHECK(web_request_parse("GET / HTTP/1.1\r\nX-Test: a\r\nX-Test: b\r\n\r\n",
                            sizeof("GET / HTTP/1.1\r\nX-Test: a\r\nX-Test: b\r\n\r\n") - 1,
                            &parsed) == 0);
    web_request_destroy(parsed);
    return 0;
}

static int test_response_header_validation(void)
{
    web_response_t* response = web_response_new(WEB_STATUS_OK);
    CHECK(response != NULL);
    CHECK(web_response_set_header(response, "X-Test", "safe") == 0);
    CHECK(web_response_set_header(response, "X-Test", "bad\r\nInjected: yes") < 0);
    web_response_destroy(response);
    return 0;
}

static web_response_t* hello_handler(const web_request_t* request, void* userdata)
{
    (void)request;
    (void)userdata;
    return web_response_text(WEB_STATUS_OK, "hello");
}

static int test_server_client_roundtrip(void)
{
    web_server_t* server = web_server_create("127.0.0.1", 18765);
    web_client_t* client;
    web_response_t* response;
    pid_t child;
    int status;
    CHECK(server != NULL);
    CHECK(web_server_set_keepalive(server, 0) == 0);
    CHECK(web_server_add_route(server, "GET", "/hello", hello_handler, NULL) == 0);
    child = fork();
    CHECK(child >= 0);
    if (child == 0) {
        alarm(10);
        (void)web_server_start(server);
        _exit(0);
    }
    usleep(300000);
    client = web_client_create(2, 2);
    CHECK(client != NULL);
    response = web_client_request(client, "GET", "http://127.0.0.1:18765/hello", NULL, NULL, 0);
    CHECK(response != NULL);
    CHECK(response->status == WEB_STATUS_OK);
    CHECK(response->body_len == 5 && memcmp(response->body, "hello", 5) == 0);
    web_response_destroy(response);
    web_client_destroy(client);
    kill(child, SIGTERM);
    CHECK(waitpid(child, &status, 0) == child);
    web_server_destroy(server);
    return 0;
}

static int test_client_error_paths(void)
{
    web_client_t* client = web_client_create(1, 1);
    web_response_t* response;
    CHECK(client != NULL);
    response = web_client_request(client, "GET", "https://example.com/", NULL, NULL, 0);
    CHECK(response == NULL);
    CHECK(strstr(web_client_error(client), "http://") != NULL);
    response = web_client_request(client, "GET", "http://127.0.0.1:1/", NULL, NULL, 0);
    CHECK(response == NULL);
    web_client_destroy(client);
    return 0;
}

int main(void)
{
    CHECK(test_request_parser() == 0);
    CHECK(test_response_header_validation() == 0);
    CHECK(test_server_client_roundtrip() == 0);
    CHECK(test_client_error_paths() == 0);
    puts("web tests passed");
    return 0;
}

#include "web/router.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WEB_ROUTER_MAX_PARAMS 16
#define WEB_ROUTER_MAX_METHOD 16

typedef struct web_route_entry {
    char* method;
    char* pattern;
    web_route_handler_fn handler;
    void* userdata;
    web_route_dtor_fn dtor;
    struct web_route_entry* next;
} web_route_entry_t;

typedef struct web_route_param {
    char* name;
    char* value;
} web_route_param_t;

struct web_router {
    web_route_entry_t* routes;
    int ignore_case;
    char error_buf[256];
};

struct web_route_match {
    const web_route_entry_t* route;
    web_route_param_t params[WEB_ROUTER_MAX_PARAMS];
    size_t count;
};

static int string_equal(const char* a, const char* b, int ignore_case)
{
    if (ignore_case) {
        while (*a && *b) {
            char ca = *a >= 'A' && *a <= 'Z' ? (char)(*a + ('a' - 'A')) : *a;
            char cb = *b >= 'A' && *b <= 'Z' ? (char)(*b + ('a' - 'A')) : *b;
            if (ca != cb)
                return 0;
            ++a;
            ++b;
        }
        return *a == *b;
    }
    return strcmp(a, b) == 0;
}

static char* duplicate_range(const char* begin, size_t length)
{
    char* result = (char*)malloc(length + 1);
    if (!result)
        return NULL;
    memcpy(result, begin, length);
    result[length] = '\0';
    return result;
}

static int next_segment(const char** cursor, char* buffer, size_t capacity)
{
    const char* begin;
    const char* end;
    size_t length;

    while (**cursor == '/')
        ++*cursor;
    if (**cursor == '\0')
        return 0;
    begin = *cursor;
    end = begin;
    while (*end && *end != '/')
        ++end;
    length = (size_t)(end - begin);
    if (length + 1 > capacity)
        return -1;
    memcpy(buffer, begin, length);
    buffer[length] = '\0';
    *cursor = end;
    return 1;
}

static void match_clear(web_route_match_t* match)
{
    size_t i;
    if (!match)
        return;
    for (i = 0; i < match->count; ++i) {
        free(match->params[i].name);
        free(match->params[i].value);
    }
    match->count = 0;
}

static int route_match(const web_route_entry_t* route,
                       const char* path,
                       int ignore_case,
                       web_route_match_t* match)
{
    const char* pattern_cursor = route->pattern;
    const char* path_cursor = path;
    char pattern_segment[1024];
    char path_segment[4096];
    int pattern_status;
    int path_status;

    for (;;) {
        pattern_status = next_segment(&pattern_cursor, pattern_segment, sizeof(pattern_segment));
        path_status = next_segment(&path_cursor, path_segment, sizeof(path_segment));
        if (pattern_status < 0 || path_status < 0)
            return 0;
        if (pattern_status == 0) {
            return path_status == 0;
        }
        if (pattern_segment[0] == '*') {
            if (match->count >= WEB_ROUTER_MAX_PARAMS)
                return 0;
            match->params[match->count].name = duplicate_range(
                pattern_segment + 1, strlen(pattern_segment + 1));
            match->params[match->count].value = duplicate_range(
                path_cursor - strlen(path_segment), strlen(path_segment));
            if (!match->params[match->count].name || !match->params[match->count].value)
                return 0;
            ++match->count;
            return 1;
        }
        if (path_status == 0)
            return 0;
        if (pattern_segment[0] == ':') {
            if (match->count >= WEB_ROUTER_MAX_PARAMS)
                return 0;
            match->params[match->count].name = duplicate_range(
                pattern_segment + 1, strlen(pattern_segment + 1));
            match->params[match->count].value = duplicate_range(
                path_segment, strlen(path_segment));
            if (!match->params[match->count].name || !match->params[match->count].value)
                return 0;
            ++match->count;
        } else if (!string_equal(pattern_segment, path_segment, ignore_case)) {
            return 0;
        }
    }
}

WEB_API web_router_t* web_router_create(void)
{
    return (web_router_t*)calloc(1, sizeof(web_router_t));
}

WEB_API void web_router_set_ignore_case(web_router_t* router, int ignore)
{
    if (router)
        router->ignore_case = ignore ? 1 : 0;
}

WEB_API int web_router_add(web_router_t* router,
                           const char* method,
                           const char* pattern,
                           web_route_handler_fn handler,
                           void* userdata,
                           web_route_dtor_fn dtor)
{
    web_route_entry_t* route;
    if (!router || !method || !pattern || !handler || pattern[0] != '/')
        return -1;
    route = (web_route_entry_t*)calloc(1, sizeof(*route));
    if (!route)
        return -1;
    route->method = strdup(method);
    route->pattern = strdup(pattern);
    route->handler = handler;
    route->userdata = userdata;
    route->dtor = dtor;
    if (!route->method || !route->pattern) {
        free(route->method);
        free(route->pattern);
        free(route);
        return -1;
    }
    route->next = router->routes;
    router->routes = route;
    return 0;
}

WEB_API int web_router_route(const web_router_t* router,
                             const char* method,
                             const char* path,
                             web_route_match_t** out_match)
{
    const web_route_entry_t* route;
    web_route_match_t* match;
    if (!router || !method || !path || !out_match)
        return -1;
    *out_match = NULL;
    for (route = router->routes; route; route = route->next) {
        if (!string_equal(route->method, method, router->ignore_case) &&
            strcmp(route->method, "*") != 0)
            continue;
        match = (web_route_match_t*)calloc(1, sizeof(*match));
        if (!match)
            return -1;
        match->route = route;
        if (route_match(route, path, router->ignore_case, match)) {
            *out_match = match;
            return 0;
        }
        match_clear(match);
        free(match);
    }
    return -1;
}

WEB_API const char* web_route_match_param(const web_route_match_t* match, const char* name)
{
    size_t i;
    if (!match || !name)
        return NULL;
    for (i = 0; i < match->count; ++i) {
        if (strcmp(match->params[i].name, name) == 0)
            return match->params[i].value;
    }
    return NULL;
}

WEB_API web_route_handler_fn web_route_match_handler(const web_route_match_t* match)
{
    return match && match->route ? match->route->handler : NULL;
}

WEB_API void* web_route_match_userdata(const web_route_match_t* match)
{
    return match && match->route ? match->route->userdata : NULL;
}

WEB_API void web_route_match_free(web_route_match_t* match)
{
    if (!match)
        return;
    match_clear(match);
    free(match);
}

WEB_API const char* web_router_error(const web_router_t* router)
{
    return router ? router->error_buf : NULL;
}

WEB_API void web_router_destroy(web_router_t* router)
{
    web_route_entry_t* route;
    if (!router)
        return;
    route = router->routes;
    while (route) {
        web_route_entry_t* next = route->next;
        if (route->dtor)
            route->dtor(route->userdata);
        free(route->method);
        free(route->pattern);
        free(route);
        route = next;
    }
    free(router);
}

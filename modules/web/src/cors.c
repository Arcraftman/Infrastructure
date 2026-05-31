#include "web/cors.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Default config
 * ========================================================================= */

WEB_API void
web_cors_config_default(web_cors_config_t *config)
{
    if (!config) return;
    config->origin           = strdup("*");
    config->methods          = strdup("GET, POST, PUT, DELETE, PATCH, OPTIONS");
    config->headers          = strdup("Content-Type, Authorization");
    config->expose_headers   = NULL;
    config->allow_credentials = 0;
    config->max_age          = 86400;
}

/* =========================================================================
 * Apply CORS headers
 * ========================================================================= */

WEB_API int
web_cors_apply(web_response_t *resp, const web_cors_config_t *config)
{
    if (!resp || !config) return -1;

    if (config->origin)
        web_response_set_header(resp,
            "Access-Control-Allow-Origin", config->origin);

    if (config->methods)
        web_response_set_header(resp,
            "Access-Control-Allow-Methods", config->methods);

    if (config->headers)
        web_response_set_header(resp,
            "Access-Control-Allow-Headers", config->headers);

    if (config->expose_headers)
        web_response_set_header(resp,
            "Access-Control-Expose-Headers", config->expose_headers);

    if (config->allow_credentials)
        web_response_set_header(resp,
            "Access-Control-Allow-Credentials", "true");

    if (config->max_age > 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%ld", config->max_age);
        web_response_set_header(resp,
            "Access-Control-Max-Age", buf);
    }

    return 0;
}

/* =========================================================================
 * Preflight handler
 * ========================================================================= */

WEB_API web_response_t *
web_cors_handle_preflight(const web_request_t *req,
                           const web_cors_config_t *config)
{
    if (!req || !config) return NULL;

    /* Only handle OPTIONS */
    if (strcmp(req->method, "OPTIONS") != 0)
        return NULL;

    /* Require Origin header for CORS preflight */
    const char *origin = web_request_get_header(req, "Origin");
    if (!origin)
        return NULL;

    web_response_t *resp = web_response_create(204);
    if (!resp) return NULL;

    /* Set Content-Length: 0 explicitly */
    web_response_set_header(resp, "Content-Length", "0");

    web_cors_apply(resp, config);

    /* Also reflect the specific origin if config origin is "*" with credentials */
    if (config->allow_credentials && config->origin &&
        strcmp(config->origin, "*") == 0) {
        web_response_set_header(resp,
            "Access-Control-Allow-Origin", origin);
        web_response_set_header(resp,
            "Vary", "Origin");
    }

    return resp;
}

/* =========================================================================
 * Destroy
 * ========================================================================= */

WEB_API void
web_cors_config_destroy(web_cors_config_t *config)
{
    if (!config) return;
    free(config->origin);
    free(config->methods);
    free(config->headers);
    free(config->expose_headers);
    /* Zero out to prevent double-free */
    config->origin         = NULL;
    config->methods        = NULL;
    config->headers        = NULL;
    config->expose_headers = NULL;
}

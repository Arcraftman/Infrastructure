#include "web/auth.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * RFC 7617 (Basic) and RFC 6750 (Bearer) authentication helpers.
 */

/* =========================================================================
 * Base64 decode (subset — decodes RFC 4648, returns allocated buffer)
 * ========================================================================= */

static unsigned char *
b64_decode(const char *in, size_t in_len, size_t *out_len)
{
    static const unsigned char DEC[256] = {
        ['A']=0, ['B']=1, ['C']=2, ['D']=3, ['E']=4, ['F']=5,
        ['G']=6, ['H']=7, ['I']=8, ['J']=9, ['K']=10, ['L']=11,
        ['M']=12, ['N']=13, ['O']=14, ['P']=15, ['Q']=16, ['R']=17,
        ['S']=18, ['T']=19, ['U']=20, ['V']=21, ['W']=22, ['X']=23,
        ['Y']=24, ['Z']=25,
        ['a']=26, ['b']=27, ['c']=28, ['d']=29, ['e']=30, ['f']=31,
        ['g']=32, ['h']=33, ['i']=34, ['j']=35, ['k']=36, ['l']=37,
        ['m']=38, ['n']=39, ['o']=40, ['p']=41, ['q']=42, ['r']=43,
        ['s']=44, ['t']=45, ['u']=46, ['v']=47, ['w']=48, ['x']=49,
        ['y']=50, ['z']=51,
        ['0']=52, ['1']=53, ['2']=54, ['3']=55, ['4']=56,
        ['5']=57, ['6']=58, ['7']=59, ['8']=60, ['9']=61,
        ['+']=62, ['/']=63
    };

    if (!in || in_len == 0) { *out_len = 0; return NULL; }

    size_t pad = 0;
    if (in_len > 0 && in[in_len - 1] == '=') pad++;
    if (in_len > 1 && in[in_len - 2] == '=') pad++;

    size_t raw_len = in_len / 4 * 3 - pad;
    unsigned char *out = (unsigned char *)malloc(raw_len + 1);
    if (!out) { *out_len = 0; return NULL; }

    size_t o = 0;
    for (size_t i = 0; i < in_len; i += 4) {
        unsigned a = i < in_len && in[i] != '=' ? DEC[(unsigned char)in[i]] : 0;
        unsigned b = i+1 < in_len && in[i+1] != '=' ? DEC[(unsigned char)in[i+1]] : 0;
        unsigned c = i+2 < in_len && in[i+2] != '=' ? DEC[(unsigned char)in[i+2]] : 0;
        unsigned d = i+3 < in_len && in[i+3] != '=' ? DEC[(unsigned char)in[i+3]] : 0;

        out[o++] = (unsigned char)((a << 2) | (b >> 4));
        if (i+2 < in_len && in[i+2] != '=') out[o++] = (unsigned char)((b << 4) | (c >> 2));
        if (i+3 < in_len && in[i+3] != '=') out[o++] = (unsigned char)((c << 6) | d);
    }
    out[o] = '\0';
    *out_len = o;
    return out;
}

static const char B64_TABLE[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char *
b64_encode(const unsigned char *in, size_t in_len)
{
    size_t out_len = 4 * ((in_len + 2) / 3) + 1;
    char *out = (char *)malloc(out_len);
    if (!out) return NULL;

    size_t i = 0, o = 0;
    while (i < in_len) {
        unsigned a = in[i];
        unsigned b = i + 1 < in_len ? in[i + 1] : 0;
        unsigned c = i + 2 < in_len ? in[i + 2] : 0;

        out[o++] = B64_TABLE[a >> 2];
        out[o++] = B64_TABLE[((a & 3) << 4) | (b >> 4)];
        out[o++] = i + 1 < in_len ? B64_TABLE[((b & 0x0F) << 2) | (c >> 6)] : '=';
        out[o++] = i + 2 < in_len ? B64_TABLE[c & 0x3F] : '=';
        i += 3;
    }
    out[o] = '\0';
    return out;
}

/* =========================================================================
 * Basic Auth
 * ========================================================================= */

WEB_API int
web_auth_basic_parse(const char *header, char **username, char **password)
{
    if (!header || !username || !password) return -1;
    *username = NULL;
    *password = NULL;

    /* Expect "Basic base64string" */
    while (*header == ' ') header++;
    if (strncasecmp(header, "Basic ", 6) != 0) return -1;
    header += 6;
    while (*header == ' ') header++;

    size_t b64len = strlen(header);
    /* Remove trailing whitespace */
    while (b64len > 0 && (header[b64len - 1] == ' ' || header[b64len - 1] == '\t' ||
           header[b64len - 1] == '\r' || header[b64len - 1] == '\n'))
        b64len--;

    /* Strip potentially quoted / extra data after Base64 */
    /* Valid Base64 ends with = or alphanumeric/+/.  Stop at anything else */
    size_t valid = 0;
    for (size_t i = 0; i < b64len; i++) {
        char c = header[i];
        if (isalnum((unsigned char)c) || c == '+' || c == '/' || c == '=')
            valid = i + 1;
        else
            break;
    }

    size_t dec_len = 0;
    unsigned char *dec = b64_decode(header, valid, &dec_len);
    if (!dec) return -1;

    /* Format: user:password */
    char *colon = (char *)memchr(dec, ':', dec_len);
    if (colon) {
        *colon = '\0';
        *username = strdup((const char *)dec);
        *password = strdup((const char *)(colon + 1));
    } else {
        *username = strdup((const char *)dec);
        *password = strdup("");
    }
    free(dec);

    if (!*username || !*password) {
        free(*username);
        free(*password);
        *username = NULL;
        *password = NULL;
        return -1;
    }
    return 0;
}

WEB_API char *
web_auth_basic_encode(const char *username, const char *password)
{
    if (!username || !password) return NULL;

    size_t ulen = strlen(username);
    size_t plen = strlen(password);
    unsigned char *raw = (unsigned char *)malloc(ulen + 1 + plen);
    if (!raw) return NULL;
    memcpy(raw, username, ulen);
    raw[ulen] = ':';
    memcpy(raw + ulen + 1, password, plen);

    char *b64 = b64_encode(raw, ulen + 1 + plen);
    free(raw);
    if (!b64) return NULL;

    size_t out_len = 6 + strlen(b64) + 1; /* "Basic " + b64 */
    char *out = (char *)malloc(out_len);
    if (!out) { free(b64); return NULL; }
    snprintf(out, out_len, "Basic %s", b64);
    free(b64);
    return out;
}

WEB_API web_response_t *
web_auth_basic_challenge(const char *realm)
{
    web_response_t *resp = web_response_create(401);
    if (!resp) return NULL;

    char hdr[256];
    snprintf(hdr, sizeof(hdr), "Basic realm=\"%s\"", realm ? realm : "");
    web_response_set_header(resp, "WWW-Authenticate", hdr);
    web_response_set_body_string(resp, "401 Unauthorized\n");
    return resp;
}

/* =========================================================================
 * Bearer Token
 * ========================================================================= */

WEB_API char *
web_auth_bearer_parse(const char *header)
{
    if (!header) return NULL;

    while (*header == ' ') header++;
    if (strncasecmp(header, "Bearer ", 7) != 0) return NULL;
    header += 7;
    while (*header == ' ') header++;

    size_t len = strlen(header);
    while (len > 0 && (header[len - 1] == ' ' || header[len - 1] == '\t' ||
           header[len - 1] == '\r' || header[len - 1] == '\n'))
        len--;

    return strndup(header, len);
}

WEB_API web_response_t *
web_auth_bearer_challenge(const char *scope)
{
    web_response_t *resp = web_response_create(401);
    if (!resp) return NULL;

    if (scope && *scope) {
        char hdr[256];
        snprintf(hdr, sizeof(hdr), "Bearer scope=\"%s\"", scope);
        web_response_set_header(resp, "WWW-Authenticate", hdr);
    } else {
        web_response_set_header(resp, "WWW-Authenticate", "Bearer");
    }
    web_response_set_body_string(resp, "401 Unauthorized\n");
    return resp;
}

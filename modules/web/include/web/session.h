#ifndef WEB_SESSION_H
#define WEB_SESSION_H

#include "web/def.h"
#include "web/http.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file session.h
 * @brief Simple in-memory session management.
 *
 * Sessions are identified by a unique session ID (UUID-like string).
 * Data is stored as key-value string pairs.
 * Expired sessions are periodically cleaned up.
 */

typedef struct web_session      web_session_t;
typedef struct web_session_store web_session_store_t;

/**
 * Create a new session store.
 * @param expiry_secs  Session idle expiry in seconds (0 = default 3600).
 * @return New session store, or NULL on error.
 */
WEB_API web_session_store_t *
web_session_store_create(long expiry_secs);

/**
 * Create or resume a session from a cookie value.
 * If `session_id` is not NULL and valid, resumes that session.
 * If `session_id` is NULL or invalid, creates a new session.
 * @param store      Session store.
 * @param session_id Existing session ID to resume (may be NULL).
 * @param[out] new_session_id  If a new session was created, receives the ID.
 *                             Caller must NOT free this (owned by session).
 * @return The session (still owned by the store), or NULL on error.
 */
WEB_API web_session_t *
web_session_get(web_session_store_t *store,
                const char *session_id,
                const char **new_session_id);

/**
 * Set a string value in a session.
 * @param store  Session store.
 * @param sess   Session.
 * @param key    Key (copied internally).
 * @param value  Value (copied internally). Pass NULL to delete the key.
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_session_set(web_session_store_t *store, web_session_t *sess,
                const char *key, const char *value);

/**
 * Get a string value from a session.
 * @return The value (owned by the session, valid until modified or session ends),
 *         or NULL if not found.
 */
WEB_API const char *
web_session_get_value(const web_session_t *sess, const char *key);

/**
 * Destroy a session (removes it from the store).
 */
WEB_API void
web_session_destroy(web_session_store_t *store, web_session_t *sess);

/**
 * Remove expired sessions from the store.
 */
WEB_API void
web_session_store_cleanup(web_session_store_t *store);

/**
 * Destroy the session store and all sessions.
 */
WEB_API void
web_session_store_destroy(web_session_store_t *store);

#ifdef __cplusplus
}
#endif

#endif /* WEB_SESSION_H */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "stk/core/hashmap.h"
#include "stk/core/str.h"
#include "stk/core/vector.h"
#include "stk/utils/status.h"

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return 1;                                                           \
        }                                                                       \
    } while (0)

static int test_vector(void)
{
    stk_vector vector = {0};
    int values[] = {10, 20, 30, 40};

    CHECK(stk_vector_init(NULL) == STK_EINVAL);
    CHECK(stk_vector_init(&vector) == STK_OK);
    CHECK(stk_vector_empty(&vector));
    CHECK(stk_vector_len(&vector) == 0);
    CHECK(stk_vector_pop(&vector) == STK_EMPTY);

    for (size_t i = 0; i < 4; ++i)
        CHECK(stk_vector_push(&vector, &values[i]) == STK_OK);

    CHECK(stk_vector_len(&vector) == 4);
    CHECK(stk_vector_cap(&vector) >= 4);
    CHECK(*(int*)stk_vector_get(&vector, 0) == 10);
    CHECK(*(int*)stk_vector_get(&vector, 3) == 40);
    CHECK(stk_vector_get(&vector, 4) == NULL);
    CHECK(stk_vector_set(&vector, 1, &values[3]) == STK_OK);
    CHECK(*(int*)stk_vector_get(&vector, 1) == 40);
    CHECK(stk_vector_set(&vector, 4, &values[0]) == STK_ERANGE);

    CHECK(stk_vector_insert(&vector, 1, &values[1]) == STK_OK);
    CHECK(stk_vector_len(&vector) == 5);
    CHECK(*(int*)stk_vector_get(&vector, 1) == 20);
    CHECK(*(int*)stk_vector_get(&vector, 2) == 40);
    CHECK(stk_vector_erase(&vector, 2) == STK_OK);
    CHECK(stk_vector_len(&vector) == 4);
    CHECK(*(int*)stk_vector_get(&vector, 2) == 30);
    CHECK(stk_vector_erase(&vector, 99) == STK_ERANGE);

    CHECK(stk_vector_reverse(&vector) == STK_OK);
    CHECK(*(int*)stk_vector_get(&vector, 0) == 40);
    CHECK(*(int*)stk_vector_get(&vector, 3) == 10);
    CHECK(stk_vector_resize(&vector, 6) == STK_OK);
    CHECK(stk_vector_len(&vector) == 6);
    CHECK(stk_vector_get(&vector, 4) == NULL);
    CHECK(stk_vector_clear(&vector) == STK_OK);
    CHECK(stk_vector_empty(&vector));
    CHECK(stk_vector_shrink(&vector) == STK_OK);
    CHECK(stk_vector_cap(&vector) == 0);
    CHECK(stk_vector_free(&vector) == STK_OK);
    return 0;
}

static int test_string(void)
{
    stk_str string = {0};

    CHECK(stk_str_init(NULL) == STK_EINVAL);
    CHECK(stk_str_init(&string) == STK_OK);
    CHECK(stk_str_empty(&string));
    CHECK(stk_str_len(&string) == 0);
    CHECK(strcmp(stk_str_cstr(&string), "") == 0);
    CHECK(stk_str_pop(&string) == STK_EMPTY);

    CHECK(stk_str_init_from(&string, "hello") == STK_OK);
    CHECK(stk_str_len(&string) == 5);
    CHECK(strcmp(stk_str_cstr(&string), "hello") == 0);
    CHECK(stk_str_push(&string, '!') == STK_OK);
    CHECK(strcmp(stk_str_cstr(&string), "hello!") == 0);
    CHECK(stk_str_append(&string, " world") == STK_OK);
    CHECK(strcmp(stk_str_cstr(&string), "hello! world") == 0);
    CHECK(stk_str_find_char(&string, 'w', 0) == 7);
    CHECK(stk_str_find(&string, "world", 0) == 7);
    CHECK(stk_str_cmp_cstr(&string, "hello! world") == 0);
    CHECK(stk_str_to_upper(&string) == STK_OK);
    CHECK(strcmp(stk_str_cstr(&string), "HELLO! WORLD") == 0);
    CHECK(stk_str_to_lower(&string) == STK_OK);
    CHECK(strcmp(stk_str_cstr(&string), "hello! world") == 0);

    CHECK(stk_str_clear(&string) == STK_OK);
    CHECK(stk_str_empty(&string));
    CHECK(stk_str_free(&string) == STK_OK);
    CHECK(stk_str_free(NULL) == STK_EINVAL);
    return 0;
}

static bool count_callback(void* key, void* value, void* user_data)
{
    (void)key;
    (void)value;
    size_t* count = user_data;
    (*count)++;
    return true;
}

static int test_hashmap(void)
{
    stk_hashmap map = {0};
    int value_a = 11;
    int value_b = 22;
    const char* key_a = "alpha";
    const char* key_b = "beta";
    size_t visited = 0;

    CHECK(stk_hashmap_init(NULL, 0, NULL, NULL) == STK_EINVAL);
    CHECK(stk_hashmap_init(&map, 2, NULL, NULL) == STK_OK);
    CHECK(stk_hashmap_capacity(&map) >= 8);
    CHECK(stk_hashmap_count(&map) == 0);
    CHECK(!stk_hashmap_has(&map, key_a));
    CHECK(stk_hashmap_set(&map, (void*)key_a, &value_a) == STK_OK);
    CHECK(stk_hashmap_set(&map, (void*)key_b, &value_b) == STK_OK);
    CHECK(stk_hashmap_count(&map) == 2);
    CHECK(stk_hashmap_has(&map, key_a));
    CHECK(*(int*)stk_hashmap_get(&map, key_a) == 11);

    value_a = 33;
    CHECK(stk_hashmap_set(&map, (void*)key_a, &value_a) == STK_OK);
    CHECK(stk_hashmap_count(&map) == 2);
    CHECK(*(int*)stk_hashmap_get(&map, key_a) == 33);
    CHECK(stk_hashmap_foreach(&map, count_callback, &visited) == STK_OK);
    CHECK(visited == 2);
    CHECK(stk_hashmap_remove(&map, key_a) == &value_a);
    CHECK(!stk_hashmap_has(&map, key_a));
    CHECK(stk_hashmap_remove(&map, key_a) == NULL);
    CHECK(stk_hashmap_clear(&map) == STK_OK);
    CHECK(stk_hashmap_count(&map) == 0);
    CHECK(stk_hashmap_free(&map) == STK_OK);
    CHECK(stk_hashmap_free(NULL) == STK_EINVAL);
    return 0;
}

int main(void)
{
    CHECK(test_vector() == 0);
    CHECK(test_string() == 0);
    CHECK(test_hashmap() == 0);
    puts("stk_core: PASS");
    return 0;
}

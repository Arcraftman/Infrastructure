if(NOT INFRA_BUILD_TESTS)
    return()
endif()

set(TEST_STK_HASHMAP ${CMAKE_CURRENT_SOURCE_DIR}/test_stk_hashmap.c)

infra_add_test(test_stk_hashmap ${TEST_STK_HASHMAP} infra::stk)

# P0 (allocator injection) + P1 (explicit iterator) regression tests.
set(TEST_STK_HASHMAP_P0P1 ${CMAKE_CURRENT_SOURCE_DIR}/test_hashmap_p0p1.c)
infra_add_test(test_stk_hashmap_p0p1 ${TEST_STK_HASHMAP_P0P1} infra::stk)

# set.c fixes: difference/intersection/symmetric semantics, iterator-based
# to_array / is_subset (no internal hashmap field access).
set(TEST_STK_SET_FIX ${CMAKE_CURRENT_SOURCE_DIR}/test_set_fix.c)
infra_add_test(test_stk_set_fix ${TEST_STK_SET_FIX} infra::stk)
set(TEST_REPRO ${CMAKE_CURRENT_SOURCE_DIR}/test_hashmap_iter_repro.c)
infra_add_test(test_hashmap_iter_repro ${TEST_REPRO} infra::stk)

set(TEST_ALLOC ${CMAKE_CURRENT_SOURCE_DIR}/test_p0p1_alloc.c)
infra_add_test(test_p0p1_alloc ${TEST_ALLOC} infra::stk)

set(TEST_ITERDEF ${CMAKE_CURRENT_SOURCE_DIR}/test_iter_default.c)
infra_add_test(test_iter_default ${TEST_ITERDEF} infra::stk)

# Pre-existing dual-mode hashmap regression tests.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_hashmap_mode.c)
    infra_add_test(test_hashmap_mode ${CMAKE_CURRENT_SOURCE_DIR}/test_hashmap_mode.c infra::stk)
endif()
# Core smoke test: vector + str + hashmap (both backends) through the
# current stk_hashmap_create() entry point.
set(TEST_CORE ${CMAKE_CURRENT_SOURCE_DIR}/test_core.c)
infra_add_test(test_core ${TEST_CORE} infra::stk)

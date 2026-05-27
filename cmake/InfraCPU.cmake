# InfraCPU.cmake - CPU 特性检测和分发

if(DEFINED INFRA_CPU_INCLUDED)
    return()
endif()
set(INFRA_CPU_INCLUDED TRUE)

# CPU 基线特性
set(INFRA_CPU_BASELINE "" CACHE STRING "CPU baseline instruction set")
set(INFRA_CPU_DISPATCH "" CACHE STRING "CPU instruction sets for dispatch")

# 检测 CPU 特性
macro(infra_cpu_detect)
    set(INFRA_CPU_FEATURES "")
    
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64|i[3456]86)")
        include(CheckCXXSourceCompiles)
        
        # 检测 SSE2
        check_cxx_source_compiles("
            #ifdef __SSE2__
            int main() { return 0; }
            #else
            #error no sse2
            #endif
        " INFRA_HAVE_SSE2)
        if(INFRA_HAVE_SSE2)
            list(APPEND INFRA_CPU_FEATURES "SSE2")
        endif()
        
        # 检测 SSE4_1
        check_cxx_source_compiles("
            #include <smmintrin.h>
            int main() {
                __m128i a = _mm_set1_epi32(0);
                __m128i b = _mm_ceil_ps(a);
                return 0;
            }
        " INFRA_HAVE_SSE4_1)
        if(INFRA_HAVE_SSE4_1)
            list(APPEND INFRA_CPU_FEATURES "SSE4_1")
        endif()
        
        # 检测 AVX2
        check_cxx_source_compiles("
            #include <immintrin.h>
            int main() {
                __m256i a = _mm256_set1_epi32(0);
                return 0;
            }
        " INFRA_HAVE_AVX2)
        if(INFRA_HAVE_AVX2)
            list(APPEND INFRA_CPU_FEATURES "AVX2")
        endif()
        
        # 检测 AVX512
        check_cxx_source_compiles("
            #include <immintrin.h>
            int main() {
                __m512i a = _mm512_set1_epi32(0);
                return 0;
            }
        " INFRA_HAVE_AVX512F)
        if(INFRA_HAVE_AVX512F)
            list(APPEND INFRA_CPU_FEATURES "AVX512F")
        endif()
        
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)")
        # 检测 NEON
        check_cxx_source_compiles("
            #ifdef __ARM_NEON
            int main() { return 0; }
            #else
            #error no neon
            #endif
        " INFRA_HAVE_NEON)
        if(INFRA_HAVE_NEON)
            list(APPEND INFRA_CPU_FEATURES "NEON")
        endif()
        
        # 检测 NEON_DOTPROD
        check_cxx_source_compiles("
            #ifdef __ARM_FEATURE_DOTPROD
            int main() { return 0; }
            #else
            #error no dotprod
            #endif
        " INFRA_HAVE_NEON_DOTPROD)
        if(INFRA_HAVE_NEON_DOTPROD)
            list(APPEND INFRA_CPU_FEATURES "NEON_DOTPROD")
        endif()
    endif()
    
    message(STATUS "Infra: CPU features detected: ${INFRA_CPU_FEATURES}")
endmacro()

# 添加分发源文件
macro(infra_add_dispatch SOURCE_FILE)
    get_filename_component(BASE_NAME ${SOURCE_FILE} NAME_WE)
    get_filename_component(SOURCE_DIR ${SOURCE_FILE} DIRECTORY)
    
    set(DISPATCH_FEATURES ${ARGN})
    set(SOURCE_FILES "")
    
    # 生成基础版本
    set(BASE_FILE ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.cpp)
    configure_file(${SOURCE_FILE} ${BASE_FILE} COPYONLY)
    list(APPEND SOURCE_FILES ${BASE_FILE})
    
    # 为每个特性生成优化版本
    foreach(FEATURE ${DISPATCH_FEATURES})
        if(INFRA_CPU_${FEATURE}_ENABLED)
            set(OPT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}_${FEATURE}.cpp)
            configure_file(${SOURCE_FILE} ${OPT_FILE} COPYONLY)
            
            # 设置编译选项
            if(FEATURE STREQUAL "AVX2")
                set_source_files_properties(${OPT_FILE} PROPERTIES
                    COMPILE_FLAGS "-mavx2 -mfma")
            elseif(FEATURE STREQUAL "AVX512F")
                set_source_files_properties(${OPT_FILE} PROPERTIES
                    COMPILE_FLAGS "-mavx512f -mavx512bw")
            elseif(FEATURE STREQUAL "SSE4_1")
                set_source_files_properties(${OPT_FILE} PROPERTIES
                    COMPILE_FLAGS "-msse4.1")
            elseif(FEATURE STREQUAL "NEON")
                set_source_files_properties(${OPT_FILE} PROPERTIES
                    COMPILE_FLAGS "-mfpu=neon")
            elseif(FEATURE STREQUAL "NEON_DOTPROD")
                set_source_files_properties(${OPT_FILE} PROPERTIES
                    COMPILE_FLAGS "-march=armv8.2-a+dotprod")
            endif()
            
            list(APPEND SOURCE_FILES ${OPT_FILE})
        endif()
    endforeach()
    
    set(${BASE_NAME}_SOURCES ${SOURCE_FILES} PARENT_SCOPE)
endmacro()

macro(infra_cpu_runtime_dispatch)
    set(INFRA_CPU_DISPATCH_CODE "
        #if defined(__x86_64__) || defined(_M_X64)
        #include <cpuid.h>
        
        static int infra_cpu_has_avx2(void) {
            unsigned int eax, ebx, ecx, edx;
            if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
                return (ebx & (1 << 5)) != 0;
            }
            return 0;
        }
        
        static int infra_cpu_has_avx512f(void) {
            unsigned int eax, ebx, ecx, edx;
            if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
                return (ebx & (1 << 16)) != 0;
            }
            return 0;
        }
        #endif
        
        #if defined(__ARM_NEON)
        #include <sys/auxv.h>
        #include <asm/hwcap.h>
        
        static int infra_cpu_has_neon_dotprod(void) {
            unsigned long hwcap = getauxval(AT_HWCAP);
            return (hwcap & HWCAP_ASIMDDP) != 0;
        }
        #endif
    ")
endmacro()
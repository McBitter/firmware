#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
#
# Copyright (C) 2014 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)
common_cflags := \
	-std=gnu99 \
	-D_REENTRANT \
	-fvisibility=hidden \
	-Wno-unused-parameter \

# ifeq ($(TARGET_BUILD_VARIANT),eng)
#   common_cflags += \
#  	  -DJEMALLOC_DEBUG
# endif

# These parameters change the way jemalloc works.
#   ANDROID_ALWAYS_PURGE
#     If defined, always purge immediately when a page is purgeable.
#   ANDROID_MAX_ARENAS=XX
#     The total number of arenas will be less than or equal to this number.
#     The number of arenas will be calculated as 2 * the number of cpus
#     but no larger than XX.
#   ANDROID_TCACHE_NSLOTS_SMALL_MAX=XX
#     The number of small slots held in the tcache. The higher this number
#     is, the higher amount of PSS consumed. If this number is set too low
#     then small allocations will take longer to complete.
#   ANDROID_TCACHE_NSLOTS_LARGE=XX
#     The number of large slots held in the tcache. The higher this number
#     is, the higher amount of PSS consumed. If this number is set too low
#     then large allocations will take longer to complete.
#   ANDROID_LG_TCACHE_MAXCLASS_DEFAULT=XX
#     1 << XX is the maximum sized allocation that will be in the tcache.
common_cflags += \
	-DANDROID_ALWAYS_PURGE \
	-DANDROID_MAX_ARENAS=2 \
	-DANDROID_TCACHE_NSLOTS_SMALL_MAX=8 \
	-DANDROID_TCACHE_NSLOTS_LARGE=16 \
	-DANDROID_LG_TCACHE_MAXCLASS_DEFAULT=16 \

common_c_includes := \
	$(LOCAL_PATH)/src \
	$(LOCAL_PATH)/include \

lib_src_files := \
	src/arena.c \
	src/atomic.c \
	src/base.c \
	src/bitmap.c \
	src/chunk.c \
	src/chunk_dss.c \
	src/chunk_mmap.c \
	src/ckh.c \
	src/ctl.c \
	src/extent.c \
	src/hash.c \
	src/huge.c \
	src/jemalloc.c \
	src/mb.c \
	src/mutex.c \
	src/prof.c \
	src/quarantine.c \
	src/rtree.c \
	src/stats.c \
	src/tcache.c \
	src/tsd.c \
	src/util.c \

#-----------------------------------------------------------------------
# jemalloc static library
#-----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libjemalloc
LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := \
	$(LOCAL_PATH)/Android.mk \

LOCAL_CFLAGS := \
	$(common_cflags) \

LOCAL_C_INCLUDES := \
	$(common_c_includes) \

LOCAL_SRC_FILES := \
	$(lib_src_files) \
	android/src/je_mallinfo.c

include $(BUILD_STATIC_LIBRARY)

#-----------------------------------------------------------------------
# jemalloc static jet library
#-----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libjemalloc_jet
LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := \
	$(LOCAL_PATH)/Android.mk \

LOCAL_CFLAGS := \
	$(common_cflags) \
	-DJEMALLOC_JET \

LOCAL_C_INCLUDES := \
	$(common_c_includes) \

LOCAL_SRC_FILES := \
	$(lib_src_files) \

include $(BUILD_STATIC_LIBRARY)

#-----------------------------------------------------------------------
# jemalloc unit test library
#-----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libjemalloc_unittest
LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := \
	$(LOCAL_PATH)/Android.mk \

LOCAL_CFLAGS := \
	$(common_cflags) \
	-DJEMALLOC_UNIT_TEST \

LOCAL_C_INCLUDES := \
	$(common_c_includes) \
	$(LOCAL_PATH)/test/src \
	$(LOCAL_PATH)/test/include \

LOCAL_SRC_FILES := \
	test/src/math.c \
	test/src/mtx.c \
	test/src/SFMT.c \
	test/src/test.c \
	test/src/thd.c \

LOCAL_WHOLE_STATIC_LIBRARIES := libjemalloc_jet

include $(BUILD_STATIC_LIBRARY)

#-----------------------------------------------------------------------
# jemalloc unit tests
#-----------------------------------------------------------------------
unit_tests := \
	test/unit/bitmap.c \
	test/unit/ckh.c \
	test/unit/hash.c \
	test/unit/junk.c \
	test/unit/mallctl.c \
	test/unit/math.c \
	test/unit/mq.c \
	test/unit/mtx.c \
	test/unit/prof_accum.c \
	test/unit/prof_gdump.c \
	test/unit/prof_idump.c \
	test/unit/ql.c \
	test/unit/qr.c \
	test/unit/quarantine.c \
	test/unit/rb.c \
	test/unit/rtree.c \
	test/unit/SFMT.c \
	test/unit/stats.c \
	test/unit/tsd.c \
	test/unit/util.c \
	test/unit/zero.c \

$(foreach test,$(unit_tests), \
  $(eval test_name := $(basename $(notdir $(test)))); \
  $(eval test_src := $(test)); \
  $(eval test_cflags := -DJEMALLOC_UNIT_TEST); \
  $(eval test_libs := libjemalloc_unittest); \
  $(eval test_path := jemalloc_unittests); \
  $(eval include $(LOCAL_PATH)/Android.test.mk) \
)

#-----------------------------------------------------------------------
# jemalloc integration test library
#-----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libjemalloc_integrationtest
LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := \
	$(LOCAL_PATH)/Android.mk \

LOCAL_CFLAGS := \
	$(common_cflags) \
	-DJEMALLOC_INTEGRATION_TEST \

LOCAL_C_INCLUDES := \
	$(common_c_includes) \
	$(LOCAL_PATH)/test/src \
	$(LOCAL_PATH)/test/include \

LOCAL_SRC_FILES := \
	test/src/math.c \
	test/src/mtx.c \
	test/src/SFMT.c \
	test/src/test.c \
	test/src/thd.c \

LOCAL_WHOLE_STATIC_LIBRARIES := libjemalloc

include $(BUILD_STATIC_LIBRARY)

#-----------------------------------------------------------------------
# jemalloc integration tests
#-----------------------------------------------------------------------
integration_tests := \
	test/integration/aligned_alloc.c \
	test/integration/allocated.c \
	test/integration/chunk.c \
	test/integration/mallocx.c \
	test/integration/MALLOCX_ARENA.c \
	test/integration/posix_memalign.c \
	test/integration/rallocx.c \
	test/integration/thread_arena.c \
	test/integration/thread_tcache_enabled.c \
	test/integration/xallocx.c \

$(foreach test,$(integration_tests), \
  $(eval test_name := $(basename $(notdir $(test)))); \
  $(eval test_src := $(test)); \
  $(eval test_cflags := -DJEMALLOC_INTEGRATION_TEST); \
  $(eval test_libs := libjemalloc_integrationtest); \
  $(eval test_path := jemalloc_integrationtests); \
  $(eval include $(LOCAL_PATH)/Android.test.mk) \
)

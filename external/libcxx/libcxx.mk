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

# To use libc++, "include external/libcxx/libcxx.mk" in your target.

LOCAL_C_INCLUDES := \
	external/libcxx/include \
	$(LOCAL_C_INCLUDES) \

LOCAL_CFLAGS += -D_USING_LIBCXX
LOCAL_CPPFLAGS += -nostdinc++
LOCAL_LDFLAGS += -nodefaultlibs
LOCAL_LDLIBS += -lm -lc
LOCAL_SHARED_LIBRARIES += libc++

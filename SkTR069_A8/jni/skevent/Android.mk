LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=select.c poll.c epoll.c signal.c \
	event.c evthread.c buffer.c \
	bufferevent.c bufferevent_sock.c bufferevent_filter.c \
	bufferevent_pair.c listener.c bufferevent_ratelim.c \
	evmap.c log.c evutil.c evutil_rand.c strlcpy.c \
	event_tagging.c http.c evdns.c evrpc.c

#LOCAL_SRC_FILES := $(call all-c-files-under,.)


LOCAL_MODULE:= libevent

LOCAL_EXPORT_LDLIBS := 
#LOCAL_LIBS := -levent
#LOCAL_STATIC_LIBRARIES := libevent
LOCAL_SHARED_LIBRARIES := libc liblog

LOCAL_CFLAGS := -O2 -g -W -Wall -DHAVE_CONFIG_H

LOCAL_C_INCLUDES :=  $(LOCAL_PATH)/compat \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/event2

LOCAL_MODULE_TAGS := eng

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)


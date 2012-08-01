
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := skypelib-jni
LOCAL_SRC_FILES := skypelib-jni.cpp \
                   packetdump.cpp \
                   random.cpp \
                   rootkeys.cpp \
                   thinclientprotocol.cpp \
                   bmcrypto/AttributeContainer.cpp \
                   bmcrypto/Crc.cpp \
                   bmcrypto/sha1.cpp \
                   bmcrypto/signature-padding.cpp \
                   bmcrypto/tc_crypto.cpp
LOCAL_CXXFLAGS := -Wno-write-strings -D__linux
LOCAL_LDLIBS   := -llog

include $(BUILD_SHARED_LIBRARY)

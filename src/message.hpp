#pragma once

#include <cstdint>
#include <stddef.h>
#include "ui/gaggia_ui.h"

// TODO: Cleanup this file

enum class MainMessage_e {
    NOOP,
    POWERSAVE_RESTART,
    SET_DEFAULTBREWTEMPERATURE,
    SET_DEFAULTSTEAMTEMPERATURE,
    LOAD_SCRIPT,
    AUTO_REMOVE_MESSAGE_DIALOG,
};

enum class UIMessage_e {
    NOOP,
    CHANGE_VIEW,
    SET_VISIBILITY,
    SET_TEXT,
    SET_TEXT_STATIC, // Use this if you are sure that the text you are placing fits in the existing buffer
};

template <typename P1, typename P2>
struct DataPair {
    P1 left;
    P2 right;
};

template <typename T, std::size_t desiredCapacity>
struct MainQueue_message {
    MainQueue_message(T t) : type(t) {
    }
    explicit MainQueue_message(T t, float value) : type(t),  floatValue(value) {
    }
    explicit MainQueue_message(T t, uint32_t value) : type(t),  intValue(value) {
    }
    explicit MainQueue_message(T t, bool value) : type(t),  boolValue(value) {
    }

    bool setData(const void* data, const size_t length) {
        assert(length <= desiredCapacity);
        memcpy(charValue, data, length);
        return true;
    }

    bool getData(void* data, const size_t length) {
        assert(length <= desiredCapacity);
        memcpy(data, charValue, length);
        return true;
    }

    bool copyChar(char const* data) {
        strncpy(charValue, data, desiredCapacity);
        return true;
    }

    T type;
    union {
        float floatValue;
        uint32_t intValue;
        bool boolValue;
        char charValue[desiredCapacity];
    };

};

template <typename T, std::size_t desiredCapacity>
struct UIQueue_message {
    UIQueue_message(T t) : type(t),  element(_LAST_ITEM_STUB) {
    }
    explicit UIQueue_message(T t, ui_element_types e, float value) : type(t),  element(e), floatValue(value) {
    }
    explicit UIQueue_message(T t, ui_element_types e, uint32_t value) : type(t),  element(e), intValue(value) {
    }
    explicit UIQueue_message(T t, ui_element_types e, bool value) : type(t),  element(e), boolValue(value) {
    }
    explicit UIQueue_message(T t, ui_element_types e, const char* value) : type(t),  element(e) {
        strncpy(charValue, value, desiredCapacity);
    }

    bool setData(const void* data, const size_t length) {
        assert(length <= desiredCapacity);
        memcpy(charValue, data, length);
        return true;
    }

    bool getData(void* data, const size_t length) {
        assert(length <= desiredCapacity);
        memcpy(data, charValue, length);
        return true;
    }

    bool copyChar(char const* data) {
        strncpy(charValue, data, desiredCapacity);
        return true;
    }

    T type;
    ui_element_types element;
    union {
        float floatValue;
        uint32_t intValue;
        bool boolValue;
        char charValue[desiredCapacity];
    };

};


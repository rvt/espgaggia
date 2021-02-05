#pragma once

#include <cstdint>
#include <functional>

enum OneShotStatus { START, END, NOP };

typedef std::function<const void (void)> CallbackFunction;
typedef std::function<const bool (void)> ModifiedFunction;

class OneShot final {
private:
    const uint32_t m_delayTimeMS;
    const CallbackFunction m_startCallback;
    const CallbackFunction m_endCallback;
    const ModifiedFunction m_modified;
    bool m_lastStatus;
    OneShotStatus m_oneShotStatus;
    uint32_t m_startTime;
public:
    OneShot(const uint32_t p_delayTimeMS,
             const CallbackFunction p_startCallback,
             const CallbackFunction p_endCallback,
             const ModifiedFunction p_modified);
    void handle();
    void direct();
    void hold();
};

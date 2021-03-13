#pragma once

#include <cstdint>
#include <functional>

enum OneShotStatus { WAIT_TRIGGER, STARTED, ENDED };

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
    uint32_t m_lastHandleTime;

    // Will trigger a start
    void triggerStart(uint32_t currentMillis);
    void triggerEnd();
public:
    OneShot(const uint32_t p_delayTimeMS,
            const CallbackFunction p_startCallback,
            const CallbackFunction p_endCallback,
            const ModifiedFunction p_modified);
    void handle(uint32_t currentMillis);

    /* Reset the oneshot, when triggered triggerStart is not called
       Reset is only effective when the state is ENDED */
    void reset();

    /* Start the oneshot, when triggered triggerStart is called immediatly
       Start is only effective when the state is ENDED */
    void start();

    /* Stop the oneshot, don't wait for a trigger and do not call triggerEnd */
    void stop();
    void stop(bool withEndTrigger);

    void trigger();

    /* Hold off the endcallback, but when will triggerEnd after p_delayTimeMS has elapsed after hold is released */
    void hold();
};

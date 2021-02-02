#pragma once

#include <button.hpp>


typedef bool (*button_event)();
class TestKnob : public Button {
private:
    bool m_current;
    const button_event m_event;
public:
    TestKnob(button_event event) :
        Button(false, 150),
        m_current(false),
        m_event(event) {
    }

    virtual bool raw() {
        return m_event();
    }

    void setCurrent(bool current) {
        m_current = current;
    };

};

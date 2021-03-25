#pragma once

#include <memory>
#include <array>

#include "boiler.hpp"
#include "temperaturesensor.hpp"
#include "heatelement.hpp"
#include "gaggiaio.hpp"

#include <Fuzzy.h>
#include <cstdlib>
#include <algorithm>

constexpr uint8_t UPDATES_PER_SECOND = 1; // numnber of fuzzy calculations per second
constexpr uint8_t TEMPERATUR_DIFFERENCE_OVER_SEC = 2;

struct GaggiaClassicControllerConfig {
    std::array<float, 4> boiler_lower  = std::array<float, 4> { {-5, -2, -2, 1} };
    std::array<float, 4> boiler_steady  = std::array<float, 4> { {-2, 0, 0, 2} };
    std::array<float, 4> boiler_higher = std::array<float, 4> { {1, 2, 2, 5} };

    std::array<float, 4> temp_error_low = std::array<float, 4> { {-2, 0, 0, 2} };
    std::array<float, 4> temp_error_medium  = std::array<float, 4> { {0, 5, 5, 10} };
    std::array<float, 4> temp_error_hight = std::array<float, 4> { {5, 20, 100, 100} };

    // Change per 1 seconds
    std::array<float, 4> temp_change_slow = std::array<float, 4> { {-0.25, 0, 0, -0.25} };
    std::array<float, 4> temp_change_medium = std::array<float, 4> { {0, 0.5, 0.5, 1} };
    std::array<float, 4> temp_change_fast = std::array<float, 4> { {0.5, 2, 10, 10} };
};

// TODO: need to be renamed to something like GaggiaBoilerController
class GaggiaClassicController : public Boiler {
private:
    GaggiaIO* m_gaggiaIO;
    Fuzzy* m_fuzzy;
    float m_setPoint;        // Setpoint
    GaggiaClassicControllerConfig m_config;
    long m_periodStartMillis;
    std::array < float, UPDATES_PER_SECOND* TEMPERATUR_DIFFERENCE_OVER_SEC + 1 > m_tempStore;
    bool m_brewMode = true;
public:
    GaggiaClassicController(GaggiaIO* p_gaggiaIO);
    virtual ~GaggiaClassicController();
    /**
     * Very important, call this once in 5 seconds
     */
    virtual void handle(const uint32_t millis);
    virtual void setPoint(float temperature);
    void brewMode(bool brewMode) {
        m_brewMode = brewMode;
    }
    bool brewMode() const {
        return m_brewMode;
    }
    virtual float setPoint() const;

    // Fuzzy inputs monitoring
    float tempChangeInput() const {
        return m_tempStore.front() - m_tempStore.back();
    }

    float lastErrorInput() const {
        return m_tempStore.front() - m_setPoint;
    }

    bool ruleFired(uint8_t i);
    void config(const GaggiaClassicControllerConfig& p_config);
    GaggiaClassicControllerConfig config() const;

    /**
     * Cretae a fuzzy set from a vector of floats
     * Warning don´t change to &data !!
     */
    template<std::size_t SIZE>
    static FuzzySet* fuzzyFromVector(std::array<float, SIZE>& data, bool flipped) {
        static_assert(SIZE == 2 || SIZE == 4, "Must be 2 or 4");

        if (SIZE == 2) {
            return new FuzzySet(-data[1], -data[0], data[0], data[1]);
        } else if (flipped) {
            return new FuzzySet(-data[3], -data[2], -data[1], -data[0]);
        } else {
            return new FuzzySet(data[0], data[1], data[2], data[3]);
        }
    }

    void init();
    virtual const char* name() const {
        return "fuzzy";
    }

private:
    FuzzyRule* joinSingle(int rule, FuzzySet* fi, FuzzySet* fo);
    FuzzyRule* joinSingleAND(int rule, FuzzySet* fi1, FuzzySet* fi2, FuzzySet* fo);

};

#include "gaggiaClassicController.hpp"

#include <memory>
#include <algorithm>

#ifdef UNIT_TEST
#include <iostream>
#include <iomanip>
#endif
#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>
#include <cmath>

constexpr uint8_t TEMP_ERROR_INPUT = 1;
constexpr uint8_t TEMP_CHANGE_INPUT = 2;
constexpr uint8_t BOILER_OUTPUT = 1;

GaggiaClassicController::GaggiaClassicController(GaggiaIO* p_gaggiaIO) :
    m_gaggiaIO(p_gaggiaIO),
    m_fuzzy(nullptr),
    m_setPoint(20.0f),
    m_periodStartMillis(0),
    m_brewMode(true) {
    m_tempStore.fill(20.0);
}

GaggiaClassicController::~GaggiaClassicController() {
    // m_heatElement->power(0.f);
}

void GaggiaClassicController::config(const GaggiaClassicControllerConfig& p_config) {
    m_config = p_config;
}

GaggiaClassicControllerConfig GaggiaClassicController::config() const {
    return m_config;
}

void GaggiaClassicController::init() {
    delete m_fuzzy;
    m_fuzzy = new Fuzzy();

    // Create input for Temperature errors
    FuzzyInput* tempErrorInput = new FuzzyInput(TEMP_ERROR_INPUT);
    m_fuzzy->addFuzzyInput(tempErrorInput);

    FuzzySet* tempErrorNegativeHigh = fuzzyFromVector(m_config.temp_error_high, true);
    tempErrorInput->addFuzzySet(tempErrorNegativeHigh);
    FuzzySet* tempErrorNegativeMedium = fuzzyFromVector(m_config.temp_errorNeg_medium, false);
    tempErrorInput->addFuzzySet(tempErrorNegativeMedium);
    FuzzySet* tempErrorLow = fuzzyFromVector(m_config.temp_error_low, false);
    tempErrorInput->addFuzzySet(tempErrorLow);
    FuzzySet* tempErrorPositiveMedium = fuzzyFromVector(m_config.temp_errorPos_medium, false);
    tempErrorInput->addFuzzySet(tempErrorPositiveMedium);
    FuzzySet* tempErrorPositiveHigh = fuzzyFromVector(m_config.temp_error_high, false);
    tempErrorInput->addFuzzySet(tempErrorPositiveHigh);

    FuzzySet* tempErrorNegativeLow = fuzzyFromVector(m_config.temp_errorNeg_low, false);
    tempErrorInput->addFuzzySet(tempErrorNegativeLow);

    // Input for temperature changes
    FuzzyInput* tempDrop = new FuzzyInput(TEMP_CHANGE_INPUT);
    m_fuzzy->addFuzzyInput(tempDrop);
    FuzzySet* tempDecreasesFast = fuzzyFromVector(m_config.temp_change_fast, true);
    tempDrop->addFuzzySet(tempDecreasesFast);
    FuzzySet* tempDecreasesMedium = fuzzyFromVector(m_config.temp_changeDec_medium, false);
    tempDrop->addFuzzySet(tempDecreasesMedium);
    FuzzySet* tempChangesSlow = fuzzyFromVector(m_config.temp_change_slow, false);
    tempDrop->addFuzzySet(tempChangesSlow);
    FuzzySet* tempIncreasedMedium = fuzzyFromVector(m_config.temp_changeInc_medium, false);
    tempDrop->addFuzzySet(tempIncreasedMedium);
    FuzzySet* tempIncreasesFast = fuzzyFromVector(m_config.temp_change_fast, false);
    tempDrop->addFuzzySet(tempIncreasesFast);

    // Create Output for Fan
    FuzzyOutput* fan = new FuzzyOutput(BOILER_OUTPUT);
    m_fuzzy->addFuzzyOutput(fan);

    FuzzySet* boilerLower = fuzzyFromVector(m_config.boiler_lower, false);
    fan->addFuzzySet(boilerLower);
    FuzzySet* boilerSteady = fuzzyFromVector(m_config.boiler_steady, false);
    fan->addFuzzySet(boilerSteady);
    FuzzySet* boilerHigher2 = fuzzyFromVector(m_config.boiler_higher2, false);
    fan->addFuzzySet(boilerHigher2);
    FuzzySet* boilerHigher = fuzzyFromVector(m_config.boiler_higher, false);
    fan->addFuzzySet(boilerHigher);
    FuzzySet* boilerHigh = fuzzyFromVector(m_config.boiler_high, false);
    fan->addFuzzySet(boilerHigh);

    uint8_t rule = 30;
    joinSingle(rule++, tempErrorNegativeHigh, boilerHigh);
    joinSingle(rule++, tempErrorPositiveHigh, boilerLower);


    joinSingle(rule++, tempErrorNegativeMedium, boilerHigher);
    joinSingle(rule++, tempErrorPositiveMedium, boilerLower);

    //    joinSingleAND(rule++, tempErrorNegativeLow, tempDecreasesMedium, boilerHigher);

}


void GaggiaClassicController::setPoint(float setTemp) {
    m_setPoint = setTemp;
}
float GaggiaClassicController::setPoint() const {
    return m_setPoint;
}

void GaggiaClassicController::handle(const uint32_t millis) {
    if (millis - m_periodStartMillis < (1000 / UPDATES_PER_SECOND)) {
        return;
    }

    m_periodStartMillis = millis;

    std::rotate(m_tempStore.begin(), m_tempStore.begin() + m_tempStore.size() - 1, m_tempStore.end());

    if (m_brewMode) {
        m_tempStore[0] = m_gaggiaIO->brewTemperature()->get();

        // Feed temp error
        m_fuzzy->setInput(TEMP_ERROR_INPUT, lastErrorInput());

        // Temperature change input
        m_fuzzy->setInput(TEMP_CHANGE_INPUT, tempChangeInput());

        /////////////////////////////////////////////

        // Run fuzzy rules
        m_fuzzy->fuzzify();

        //m_gaggiaIO->boilerIncrease(m_fuzzy->defuzzify(BOILER_OUTPUT));
        m_gaggiaIO->boilerSet(m_fuzzy->defuzzify(BOILER_OUTPUT));
    } else {
        m_tempStore[0] = m_gaggiaIO->steamTemperature()->get();

        if (lastErrorInput() < 0.0f) {
            m_gaggiaIO->boilerSet(100.0);
        } else {
            m_gaggiaIO->boilerSet(0.0);
        }
    }

}

bool GaggiaClassicController::ruleFired(uint8_t i) {
    return m_fuzzy->isFiredRule(i);
}

FuzzyRule* GaggiaClassicController::joinSingle(int rule, FuzzySet* fi, FuzzySet* fo) {
    FuzzyRuleAntecedent* ifCondition = new FuzzyRuleAntecedent();
    ifCondition->joinSingle(fi);
    FuzzyRuleConsequent* thenConsequence = new FuzzyRuleConsequent();
    thenConsequence->addOutput(fo);
    FuzzyRule* fr = new FuzzyRule(rule, ifCondition, thenConsequence);
    m_fuzzy->addFuzzyRule(fr);
    return fr;
}

FuzzyRule* GaggiaClassicController::joinSingleAND(int rule, FuzzySet* fi1, FuzzySet* fi2, FuzzySet* fo) {
    FuzzyRuleAntecedent* ifCondition = new FuzzyRuleAntecedent();
    ifCondition->joinWithAND(fi1, fi2);
    FuzzyRuleConsequent* thenConsequence = new FuzzyRuleConsequent();
    thenConsequence->addOutput(fo);
    FuzzyRule* fr = new FuzzyRule(rule, ifCondition, thenConsequence);
    m_fuzzy->addFuzzyRule(fr);
    return fr;
}

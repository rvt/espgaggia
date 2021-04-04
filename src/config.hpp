// How often we are updating the mqtt state in ms
#include <cstdint>
constexpr char MQTT_LASTWILL[]="lastwill";
constexpr char MQTT_STATUS[]="status";

constexpr uint8_t STEAM_BUTTON_PIN = CONFIG_STEAM_BUTTON_PIN;   //  IN
constexpr uint8_t BREW_BUTTON_PIN = CONFIG_BREW_BUTTON_PIN;     //  IN

constexpr uint8_t BOILER_PIN = CONFIG_SSR_BOILER_PIN;           //  OUT
constexpr uint8_t VALVE_PIN = CONFIG_SSR_VALVE_PIN;             //  OUT
constexpr uint8_t PUMP_PIN = CONFIG_SSR_PUMP_PIN;               //  OUT

// Pin 15 didn´t make the esp start up so we skipped it and took 2
constexpr uint8_t BREW_PIN_SPI_CS = CONFIG_BREW_PIN_SPI_CS;     // OUT
constexpr uint8_t STEAM_PIN_SPI_CS = CONFIG_STEAM_PIN_SPI_CS;   // OUT

constexpr uint8_t PERI_PIN_SPI_MISO = CONFIG_PERI_PIN_SPI_MISO; // IN
constexpr uint8_t PERI_PIN_SPI_CLK = CONFIG_PERI_PIN_SPI_CLK;   // OUT

constexpr char CONTROLLER_CONFIG_FILENAME[] = "/controllerCfg.conf";
constexpr char CONFIG_FILENAME[] = "/gaggiaCfg.conf";
constexpr char STARTUP_SCRIPT[] = "/_startup.txt";
constexpr char QUICKSTART_SCRIPT[] = "/_quickStart.txt";
constexpr char STOP_SCRIPT[] = "/_stop.txt";
constexpr char POWERDOWN_SCRIPT[] = "/_powerdown.txt";
constexpr char POWERSAVE_SCRIPT[] = "/_powersave.txt";


// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
constexpr float RREF_OVEN = 430.0;

// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
constexpr float RNOMINAL_OVEN = 100.0;

constexpr int32_t BREW_TEMP_MIN = 70;
constexpr int32_t BREW_TEMP_MAX = 120;

constexpr int32_t STEAM_TEMP_MIN = 120;
constexpr int32_t STEAM_TEMP_MAX = 150;

constexpr int16_t MAX_MENU_STR_LENGTH = 256;

#define WATER_PUMP_PIN 2
#define INFO_LED 13
#define ON "ON"
#define OFF "OFF"
#define SWAP "SWAP"
#define CELSIUS "CELSIUS"
#define FAHRENHEIT "FAHRENHEIT"

#define TRIGGER_BUTTON 7
#define DURATION 2000

#define SERVER_PORT 8080
#define DNS_HOST "cyverplant"

#define POW_DHTPIN 4
#define DHTPIN 3
#define DHTTYPE DHT22

#define POW_SOIL_MOISTURE_PIN 5
#define SOIL_MOISTURE_PIN A0

#define POW_PHOTORES_PIN 0
#define PHOTORES_PIN A1

#define POW_WATER_LVL_PIN 8
#define WATER_LVL_PIN A2

#define TEMP_MIN 15            // Temperatura mínima para permitir riego
#define TEMP_MAX 35            // Temperatura máxima para permitir riego
#define HUMIDITY_THRESHOLD 40  // Humedad ambiental baja
#define LIGHT_THRESHOLD 90     // Nivel de luz para evitar regar en pleno sol
#define DRY_THRESHOLD 60       // Porcentaje de humedad del suelo mínimo antes de regar

#define MEASURE_TIMES 5

#define TIME_TO_WATERING 60000
//#define TIME_TO_WATERING 10000
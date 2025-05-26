// Auto Dimming LED Strip
// Matt Grammes 2018

#include "LedStrip.h"
#include "Arduino.h"
#include "ConfigNVS.h"
#include <FastLED.h>

// #define DATA_PIN 22
#define DATA_PIN 4
#define MASTER_BRIGHTNESS 255
#define COLOR_ORDER RBG
#define MAX_LED_MODES 15 

// Note: numA thru numD values will add up to NUM_LEDS, the total
// number of pixels in the whole display, which is 32 in this exmaple.

#define NUM_LEDS 56

CRGB leds[NUM_LEDS]; // The array that actually gets displayed

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

static const char *MODULE_PREFIX = "LedStrip: ";
LedStrip::LedStrip(ConfigBase &ledNvValues) : _ledNvValues(ledNvValues)
{
    _isSetup = false;
    _isSleeping = false;
    _ledPin = -1;
    _sensorPin = -1;
}

void LedStrip::setup(ConfigBase *pConfig, const char *ledStripName)
{
    _name = ledStripName;
    // Save config and register callback on config changed
    if (_pHwConfig == NULL)
    {
        _pHwConfig = pConfig;
        _pHwConfig->registerChangeCallback(std::bind(&LedStrip::configChanged, this));
    }

    // Get LED config
    ConfigBase ledConfig(pConfig->getString(ledStripName, "").c_str());
    Log.trace("%ssetup name %s configStr %s\n", MODULE_PREFIX, _name.c_str(),
              ledConfig.getConfigCStrPtr());

    // LED Strip Negative PWM Pin
    String pinStr = ledConfig.getString("ledPin", "");
    int ledPin = -1;
    if (pinStr.length() != 0)
        ledPin = ConfigPinMap::getPinFromName(pinStr.c_str());
    Log.info("%sLED PIN VALUE IS: %d", MODULE_PREFIX, ledPin);

    FastLED.addLeds<WS2812B, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(MASTER_BRIGHTNESS);
    FastLED.clear();
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
    // // Ambient Light Sensor Pin
    // String sensorStr = ledConfig.getString("sensorPin", "");
    // int sensorPin = -1;
    // if (sensorStr.length() != 0)
    //     sensorPin = ConfigPinMap::getPinFromName(sensorStr.c_str());

    // Log.notice("%sLED pin %d Sensor pin %d\n", MODULE_PREFIX, ledPin, sensorPin);
    // // Sensor pin isn't necessary for operation.
    // if (ledPin == -1)
    //     return;

    // // Setup led pin
    // if (_isSetup && (ledPin != _ledPin))
    // {
    //     ledcDetachPin(_ledPin);
    // }
    // else
    // {
    //     _ledPin = ledPin;
    //     ledcSetup(LED_STRIP_LEDC_CHANNEL, LED_STRIP_PWM_FREQ, LED_STRIP_LEDC_RESOLUTION);
    //     ledcAttachPin(_ledPin, LED_STRIP_LEDC_CHANNEL);
    // }

    // // Setup the sensor
    // _sensorPin = sensorPin;
    // if (_sensorPin != -1) {
    //     pinMode(_sensorPin, INPUT);
    //     for (int i = 0; i < NUM_SENSOR_VALUES; i++) {
    //         sensorValues[i] = 0;
    //     }
    // }

    // // If there is no LED data stored, set to default
    // String ledStripConfigStr = _ledNvValues.getConfigString();
    // if (ledStripConfigStr.length() == 0 || ledStripConfigStr.equals("{}")) {
    //     Log.trace("%sNo LED Data Found in NV Storge, Defaulting\n", MODULE_PREFIX);
    //     // Default to LED On, Half Brightness
    //     _ledOn = true;
    //     _ledValue = 0xff;
    //     _autoDim = false;
    //     updateNv();
    // } else {
    //     _ledOn = _ledNvValues.getLong("ledOn", 0) == 1;
    //     _ledValue = _ledNvValues.getLong("ledValue", 0xFF);
    //     _autoDim = _ledNvValues.getLong("autoDim", 0) == 1;
    //     Log.trace("%sLED Setup from JSON: %s On: %d, Value: %d, Auto Dim: %d\n", MODULE_PREFIX,
    //                 ledStripConfigStr.c_str(), _ledOn, _ledValue, _autoDim);
    // }

    // _isSetup = true;
    // // Trigger initial write
    // ledConfigChanged = true;
    // Log.trace("%sLED Configured: On: %d, Value: %d, AutoDim: %d\n", MODULE_PREFIX, _ledOn, _ledValue, _autoDim);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, MASTER_BRIGHTNESS, currentBlending);
        colorIndex += 3;
    }
}

void LedStrip::updateLedFromConfig(const char *pLedJson)
{

    boolean changed = false;
    boolean ledOn = RdJson::getLong("ledOn", 0, pLedJson) == 1;
    if (ledOn != _ledOn)
    {
        _ledOn = ledOn;
        changed = true;
    }
    byte ledValue = RdJson::getLong("ledValue", 0, pLedJson);
    if (ledValue != _ledValue)
    {
        _ledValue = ledValue;
        changed = true;
    }
    boolean autoDim = RdJson::getLong("autoDim", 0, pLedJson) == 1;
    if (autoDim != _autoDim)
    {
        // TODO Never Enable Auto Dim
        //_autoDim = autoDim;
        _autoDim = false;
        changed = true;
    }

    if (changed)
        updateNv();
}

const char *LedStrip::getConfigStrPtr()
{
    return _ledNvValues.getConfigCStrPtr();
}

void LedStrip::updateLedValue()
{
    if (_realLedValue + 1 > MAX_LED_MODES){
        _realLedValue = 1;
    }
    else{
        _realLedValue++;
    }
}

void LedStrip::service()
{

    //--------------Animation A--------------
    //   static uint8_t color;
    //   EVERY_N_MILLISECONDS(180) {
    //     fadeToBlackBy( ledsA, numA, 230);
    //     static uint8_t i = 0;
    //     ledsA[i] = CHSV( color+random8(0,10), 160, 255 );
    //     i++;
    //     if (i == numA) { i = 0; }  //reset

    //     //copy ledsA data to leds
    //     for (uint8_t i=0; i<numA; i++) { leds[i] = ledsA[i]; }
    //   }

    //   EVERY_N_SECONDS(2) {
    //     color = random8();
    //   }
    const uint8_t delta = 255 / NUM_LEDS / 20;
    static uint8_t color;
    static uint8_t count;
        
    static uint8_t startIndex = 0;
 /* motion speed */
    switch (_realLedValue)
    {
    case 1:
        EVERY_N_MILLISECONDS(60) { count++; }
        fill_rainbow(leds, NUM_LEDS, count, -1 * delta);
        break;
    case 2:
        currentPalette = RainbowColors_p;
        currentBlending = LINEARBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break; //end EVERY_N gradMoveDelay
    case 3:
        currentPalette = RainbowStripeColors_p;
        currentBlending = NOBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break; //end EVERY_N gradMoveDelay
    case 4:
        EVERY_N_MILLISECONDS(400) {
            for (uint8_t i=0; i<3; i++) {  //lightup some random pixels
                uint8_t pick = random8(NUM_LEDS);
                static uint8_t hue;
                leds[pick] = CHSV( hue, random8(128,200), random8(200,255) );
                hue = hue + random8(4,8);
            }
        }

        EVERY_N_MILLISECONDS(200) {
            uint8_t pick = random8(NUM_LEDS);
            leds[pick] = CRGB::Black;
        }
        break;
    case 5:
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        break;
    case 6:
        fill_solid(leds, NUM_LEDS, CRGB::White);
        break;
    case 7:
        currentPalette = RainbowStripeColors_p;
        currentBlending = LINEARBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    case 8:
        SetupPurpleAndGreenPalette();
        currentBlending = LINEARBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    case 9:
        SetupTotallyRandomPalette();
        currentBlending = LINEARBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    case 10:
        SetupBlackAndWhiteStripedPalette();
        currentBlending = NOBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    case 11:
        SetupBlackAndWhiteStripedPalette();
        currentBlending = LINEARBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    case 12:
        currentPalette = CloudColors_p;
        currentBlending = LINEARBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    case 13:
        currentPalette = PartyColors_p;
        currentBlending = LINEARBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    case 14:
        currentPalette = myRedWhiteBluePalette_p;
        currentBlending = NOBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    case 15:
        currentPalette = myRedWhiteBluePalette_p;
        currentBlending = LINEARBLEND;
        startIndex = startIndex + 1;
        FillLEDsFromPaletteColors( startIndex);
        break;
    }
    //--------------Animation B--------------

    //   //--------------Animation C--------------
    //   for (uint8_t i=0; i<numC; i++) {
    //     uint8_t blue = (millis()/30)+(i*3);
    //     if (blue < 128) {
    //       ledsC[i] = CRGB(200, 0, 0);
    //     } else {
    //       ledsC[i] = CRGB(50, 0, blue);
    //     }
    //   }
    //   fadeToBlackBy( ledsC, numC, 220);
    //   uint8_t pos = beatsin8(20,0,numC-1);
    //   ledsC[pos] = CRGB::Green;

    //   //copy ledsC data to leds
    //   for (uint8_t i=0; i<numC; i++) { leds[i+numA+numB] = ledsC[i]; }

    //   //--------------Animation D--------------
    //   EVERY_N_MILLISECONDS(400) {
    //     for (uint8_t i=0; i<3; i++) {  //lightup some random pixels
    //       uint8_t pick = random8(numD);
    //       static uint8_t hue;
    //       ledsD[pick] = CHSV( hue, random8(128,200), random8(200,255) );
    //       hue = hue + random8(4,8);
    //     }

    //     //copy ledsD data to leds
    //     for (uint8_t i=0; i<numD; i++) { leds[i+numA+numB+numC] = ledsD[i]; }
    //   }

    //   EVERY_N_MILLISECONDS(200) {
    //     uint8_t pick = random8(numD);
    //     ledsD[pick] = CRGB::Black;

    //     //copy ledsD data to leds
    //     for (uint8_t i=0; i<numD; i++) { leds[i+numA+numB+numC] = ledsD[i]; }
    //   }

    // display all the updates on leds array

    FastLED.show();
    FastLED.delay(100);

    // // Check if active
    // if (!_isSetup)
    //     return;

    // // If the switch is off or sleeping, turn off the led
    // if (!_ledOn || _isSleeping)
    // {
    //     _ledValue = 0x0;
    // }
    // else
    // {
    //     // TODO Auto Dim isn't working as expected - this should never go enabled right now
    //     // Check if we need to read and evaluate the light sensor
    //     if (_autoDim)
    //     {
    //         if (_sensorPin != -1) {
    //             sensorValues[sensorReadingCount++ % NUM_SENSOR_VALUES] = analogRead(_sensorPin);
    //             uint16_t sensorAvg = LedStrip::getAverageSensorReading();
    //             // if (count % 100 == 0) {
    //             // Log.trace("Ambient Light Avg Value: %d, reading count %d\n", sensorAvg, sensorReadingCount % NUM_SENSOR_VALUES);

    //             // Convert ambient light (int) to led value (byte)
    //             int ledBrightnessInt = sensorAvg / 4;

    //             // This case shouldn't be hit
    //             if (ledBrightnessInt > 255) {
    //                 ledBrightnessInt = 255;
    //                 Log.error("%sAverage Sensor Value over max!\n", MODULE_PREFIX);
    //             }
    //             byte ledValue = ledBrightnessInt;
    //             if (_ledValue != ledValue) {
    //                 _ledValue = ledValue;
    //                 updateNv();
    //             }
    //         }
    //     }
    // }

    // if (ledConfigChanged    ledConfigChanged = false;
    //     Log.trace("Writing LED Value: 0x%x\n", _ledValue);
    //     ledcWrite(LED_STRIP_LEDC_CHANNEL, _ledValue);
    // }) {
    //
}

// This function fills the palette with totally random colors.
void LedStrip::SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void LedStrip::SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void LedStrip::SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

void LedStrip::configChanged()
{
    // Reset config
    Log.trace("%sconfigChanged\n", MODULE_PREFIX);
    setup(_pHwConfig, _name.c_str());
}

void LedStrip::updateNv()
{
    String jsonStr;
    jsonStr += "{";
    jsonStr += "\"ledOn\":";
    jsonStr += _ledOn ? "1" : "0";
    jsonStr += ",";
    jsonStr += "\"ledValue\":";
    jsonStr += _ledValue;
    jsonStr += ",";
    jsonStr += "\"autoDim\":";
    jsonStr += _autoDim ? "1" : "0";
    jsonStr += "}";
    _ledNvValues.setConfigData(jsonStr.c_str());
    _ledNvValues.writeConfig();
    Log.trace("%supdateNv() : wrote %s\n", MODULE_PREFIX, _ledNvValues.getConfigCStrPtr());
    ledConfigChanged = true;
}

// Get the average sensor reading
uint16_t LedStrip::getAverageSensorReading()
{
    uint16_t sum = 0;
    for (int i = 0; i < NUM_SENSOR_VALUES; i++)
    {
        sum += sensorValues[i];
    }
    return sum / NUM_SENSOR_VALUES;
}

// Set sleep mode
void LedStrip::setSleepMode(int sleep)
{
    _isSleeping = sleep;
}

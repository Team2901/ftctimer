
#ifndef _APP_H
#define _APP_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/semphr.h>
#include <led_strip.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_mac.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_spiffs.h>
#include <esp_random.h>
#include <esp_http_server.h>
#include <driver/gpio.h>
#include <esp_vfs.h>
#include <esp_vfs_fat.h>
#include <esp_netif.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_app_format.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>
#include <esp_ota_ops.h>
#include <esp_http_client.h>
#include <nvs_flash.h>
#include <esp_adc/adc_oneshot.h>
#include <wifi-captive-portal.h>
#include <esp_timer.h>
#include <driver/uart.h>

// #include <esp_vfs_spiffs.h>
#include <mdns.h>

#ifdef __cplusplus // Provide C++ Compatibility

extern "C"
{
#endif

  //   typedef enum
  //   {
  //     TREE_STATE_RUNNING,
  //     TREE_STATE_START_POWERDOWN,
  //     TREE_STATE_IN_POWERDOWN,
  //     TREE_STATE_POWERDOWN,
  //     TREE_STATE_START_AP,
  //     TREE_STATE_AP_RUNNING,
  //     TREE_STATE_AP_CONNECTED,
  //     TREE_STATE_CONNECTING_WIFI,
  //   } APP_STATES;

  // #define GPIO_INPUT_IO_0 CONFIG_GPIO_INPUT_0
  // #define GPIO_INPUT_IO_1 CONFIG_GPIO_INPUT_1
  // #define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))

  // #define ADC_CHANNEL ADC_CHANNEL_5 // GPIO16 (ADC 2 Channel 5)
  // #define ADC_UNIT ADC_UNIT_2       // ADC2
  // #define ADC_ATTEN ADC_ATTEN_DB_12 // Attenuation for 0 to 3.3V range

  // #define LED_COLUMNS 4
  // #define LED_ROWS 13
  // #define LED_MAX_TRANSITIONS 6

  //   // #define MAX_BRIGHT 255
  //   // #define LIMIT_RED 180
  //   // #define LIMIT_BLUE 160
  //   // #define LIMIT_GREEN 120
  //   // #define MAX_BRIGHT 128
  //   // #define LIMIT_RED 90
  //   // #define LIMIT_BLUE 80
  //   // #define LIMIT_GREEN 60

  // #define MAX_BRIGHT 128
  // #define LIMIT_RED 60   // 45
  // #define LIMIT_BLUE 53  // 38
  // #define LIMIT_GREEN 40 // 30

  // #define RED_GAMMA (3.0) // (5.0) // (2.8)
  // #define BLUE_GAMMA (3.0)
  // #define GREEN_GAMMA (3.0) // (2.8)

  // #define MAX_SEQUENCES 64
  // #define TOTAL_LEDS (12 + 12 + 12 + 12 + 1)

  // #define COLOR_BLACK 0
  // #define COLOR_WHITE 1
  // #define RANGE_COLORS 126
  // #define TICKS_PER_SECOND 24
  // #define REFRESH_PER_SECOND 350
  // #define CYCLE_TIME 30
  // #define TIMER_INTERVAL (pdMS_TO_TICKS(1000 / TICKS_PER_SECOND))
  // #define REFRESH_INTERVAL (pdMS_TO_TICKS(1000 / REFRESH_PER_SECOND))
  // #define DEFAULT_REFRESH_LIMIT 2
  // #define DEFAULT_REFRESH_ON 1

  // // GPIO assignment
  // #define LED_STRIP_AB 4
  // #define LED_STRIP_CD 5
  // #define LED_STRIP_EF 6
  // #define LED_STRIP_GH 7

  // #define MAP_COLOR(cbase, ccur) (((((cbase) & 0x7f) + (ccur)) % RANGE_COLORS) + 2)

  // #define PUSH_BUTTON_PORT GPIO_NUM_15
  // #define SEQ_RANDOM 0x80

  //   typedef struct _sequenceelem
  //   {
  //     unsigned char ucTime;
  //     unsigned char ucColor;
  //   } SequenceElem;

  //   typedef struct _sequenceTable
  //   {
  //     unsigned char ucCount;
  //     unsigned char ucLength[1];
  //   } SequenceTable;

  //   typedef struct _stageelem
  //   {
  //     unsigned char ucSequence;
  //     unsigned char ucColor;
  //     unsigned char ucOffsetTime;
  //   } StageElem;

  //   typedef struct _stage
  //   {
  //     StageElem aStageElem[TOTAL_LEDS];
  //   } Stage;

  //   typedef enum
  //   {
  //     IN_APP_Tasks = 0,
  //     IN_TREE_InitRGB,
  //     IN_TREE_Tick,
  //     IN_TREE_Refresh,
  //     IN_TREE_LoadPattern,
  //     IN_TREE_CalcRGB,
  //     IN_TREE_CalcRender,
  //     IN_TREE_Button,
  //     IN_TREE_NextTick,
  //   } RoutineId;

  //   typedef struct _rgb
  //   {
  //     unsigned char ucRed;
  //     unsigned char ucGreen;
  //     unsigned char ucBlue;
  //   } RGB;

  //   typedef struct _ledState
  //   {
  //     unsigned char ucSequence;  // Which sequence we are using
  //     unsigned char ucColor;     // Current offset color
  //     unsigned char ucTime;      // Current time offset from start of the sequence
  //     unsigned char ucStep;      // Which step (index) in the sequence we are at
  //     unsigned char ucStage;     // Which stage we are on
  //     unsigned char ucNextColor; // What color we are advancing to
  //   } LEDState;

  //   typedef struct _patternLookup
  //   {
  //     const char *name;
  //     const unsigned char *pPattern;
  //   } PatternLookup;

  // #define TICKS_PRESSED 2
  // #define TICKS_DOUBLETAP 10
  // #define TICKS_POWERDOWN (TICKS_PER_SECOND * 2)
  // #define TICKS_WIFICONFIG (TICKS_PER_SECOND * 5)
  // #define MAX_APP_FILELIST 500

  //   typedef struct
  //   {
  //     RoutineId eRoutineId;
  //     adc_oneshot_unit_handle_t adc1_handle;

  //     int nDebugRefreshCount;
  //     int nDebugTickCount;
  //     int nRefreshLimit;
  //     int nRefreshCount;
  //     int nOnCount;
  //     int nLightSensor;
  //     int nPressedCount;
  //     int nReleasedCount;
  //     int nPatternTime;
  //     bool initialized;
  //     bool cyclepattern;
  //     bool bLoadPattern;
  //     bool bLoadPatternList; // We need to load the pattern list from FFS or memory because it has been updated
  //     bool bNextStep;
  //     unsigned long maskval;
  //     int curColor;
  //     int curRow;
  //     int curColumn;
  //     int swingState;

  //     const char *pPatternList;
  //     int nPatternListSize;
  //     const char *pCurrentPattern;

  //     const unsigned char *pLoadedPattern;
  //     int nLoadedPatternSize;

  //     unsigned char aucRedMap[256];
  //     unsigned char aucBlueMap[256];
  //     unsigned char aucGreenMap[256];
  //     RGB aRGB[128];
  //     SequenceElem *apSequenceElem[MAX_SEQUENCES];
  //     Stage *pStage;
  //     LEDState aLEDState[TOTAL_LEDS];
  //     // Tree Colors
  //     //
  //     // 4 rows of 12/13 LEDs
  //     //
  //     // So
  //     unsigned char aaucRed[LED_ROWS][LED_COLUMNS];
  //     unsigned char aaucGreen[LED_ROWS][LED_COLUMNS];
  //     unsigned char aaucBlue[LED_ROWS][LED_COLUMNS];

  //     SemaphoreHandle_t hTimerSemaphore;

  //     led_strip_handle_t ahLEDStrips[LED_COLUMNS];
  //     // int bBlankIt;
  //     touch_button_handle_t button_handle;

  //     bool bTouchPressed;
  //     APP_STATES stateApp;

  //   } APP_DATA;

  typedef enum
  {
    OTA_QUERY,
    OTA_EXECUTE,
  } OtaCommand;

  typedef struct _otaRequest
  {
    OtaCommand eOtaCommand;
  } ota_request_t;

#define FILE_PATH_MAX (CONFIG_SPIFFS_OBJ_NAME_LEN + 1)
#define MAX_FILE_SIZE 2048 // 2K
#define MAX_FILE_SIZE_STR "2K"
#define SCRATCH_BUFSIZE 2048

  //   extern APP_DATA appData;

  /**
   * @brief Application states
   *
   */
  typedef enum
  {
    APP_STATE_SCROLLMSG,       // We are showing the codebusters logo
    APP_STATE_WAIT_START,      // Displaying the number of minutes left, waiting for the start button
    APP_STATE_TIMED_QUESTION,  // Initial 10 minute interval for the timed question
    APP_STATE_WAIT_25_MINUTES, // Timed question complete, waiting for half way point
    APP_STATE_WAIT_10_MINUTES, // Waiting for 10 minutes remaining
    APP_STATE_WAIT_2_MINUTES,  // Waiting for 2 minutes remaining
    APP_STATE_WAIT_10_SECONDS, // Waiting for the last 10 seconds
    APP_STATE_FINAL_10SECONDS, // Final 10 seconds (showing second timer) waiting for end
    APP_STATE_DONE,            // Test complete
    APP_STATE_CONFIG,          // COnfiguration mode (currently does nothing)
  } APP_STATES;

  /**
   * @brief Configuration for the LEDs on the display
   * Note that each segment consists of several LEDS in the strip and the period is
   * a single LED in the strip.
   */
#define DISPLAY_DIGITS 3
#define DIGIT_SEGMENTS 8
#define BASE_LEDS_PER_SEGMENT 6

// Numbers of the LED in the strip
#define LED_STRIP_TOTAL_LEDS (DISPLAY_DIGITS * (((DIGIT_SEGMENTS - 1) * BASE_LEDS_PER_SEGMENT) + 3))
/**
 * @brief Masks for the segments to display
 *
 */
#define SEG_A (0x01 << 0)
#define SEG_B (0x01 << 1)
#define SEG_C (0x01 << 2)
#define SEG_D (0x01 << 3)
#define SEG_E (0x01 << 4)
#define SEG_F (0x01 << 5)
#define SEG_G (0x01 << 6)
#define SEG_DOT (0x01 << 7)
#define SEG_ALL (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DOT)

#define TICKS_PER_SECOND 24
#define TIMER_INTERVAL (pdMS_TO_TICKS(1000 / TICKS_PER_SECOND))

// GPIO assignments
#define LED_STRIP_PORT 0
#define PUSH_BUTTON_PORT GPIO_NUM_1

#define UART_NUM UART_NUM_1
#define TXD_PIN GPIO_NUM_6
#define RXD_PIN GPIO_NUM_7
  /**
   * @brief
   *
   */
  typedef uint32_t rgb_t;

#define rgb(r, g, b) ((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))
#define RGB_GET_R(color) (((color) >> 16) & 0xff)
#define RGB_GET_G(color) (((color) >> 8) & 0xff)
#define RGB_GET_B(color) ((color) & 0xff)
/**
 * @brief LED Colors to display
 */
#define RGB_ORANGE rgb(255, 165, 0)
#define RGB_RED rgb(255, 0, 0)
#define RGB_GREEN rgb(0, 255, 0)
#define RGB_WHITE rgb(255, 255, 255)
#define RGB_YELLOW rgb(255, 255, 0)
#define RGB_BLACK rgb(0, 0, 0)
#define RGB_PURPLE rgb(255, 0, 255)
#define RGB_BLUE rgb(0, 0, 255)
/**
 * @brief Button press intervals
 */
#define TICKS_PRESSED 2
#define TICKS_DOUBLETAP 10
#define TICKS_REQUEST_RESET (TICKS_PER_SECOND * 2)
#define TICKS_REQUEST_CONFIG (TICKS_PER_SECOND * 5)

#define DFPLAYER_CMD_LENGTH 10
#define DFPLAYER_INIT_DELAY_MS 2000 // Initial power-on wait
#define DFPLAYER_RETRY_DELAY_MS 100 // Retry delay for first command
#define DFPLAYER_RETRY_COUNT 2      // How many times to retry first play command

#define MAX_SCROLL_MESSAGE 24 // Maximum length of the scroll message
/**
 * @brief Player tracks
 */
#define TRACK_WELCOME_TO_CODEBUSTERS 2
#define TRACK_NO_MORE_TIMED_BONUS 1
#define TRACK_25_MINUTES_REMAIN 3
#define TRACK_10_MINUTES_REMAIN 4
#define TRACK_2_MINUTES_REMAIN 5
#define TRACK_TIMES_UP 6
/**
 * @brief Time intervals
 */
#if 1
#define END_TIMED_SECONDS (10 * 60)
#define ANNOUNCE_25_MINUTES (25 * 60)
#define ANNOUNCE_10_MINUTES (40 * 60)
#define ANNOUNCE_2_MINUTES (48 * 60)
#define FINAL_SECONDS ((50 * 60) - 10)
#define EVENT_LENGTH (50 * 60)
#define SCALE_SPEED (1)
#else
#define END_TIMED_SECONDS (10)
#define ANNOUNCE_25_MINUTES (15)
#define ANNOUNCE_10_MINUTES (30)
#define ANNOUNCE_2_MINUTES (38)
#define FINAL_SECONDS (40)
#define EVENT_LENGTH (50)
#define SCALE_SPEED (60)
#endif

  typedef struct
  {
    int nPressedCount;                          // Ticks that the button is pressed
    int nReleasedCount;                         // Ticks that the button is released
    APP_STATES stateApp;                        // Application state
    bool bStartState;                           // Flag indicating that the state was just started
    int64_t tStartTime;                         // Time in microseconds that we started
    int64_t tNow;                               // Current time in microseconds
    double dLastSeconds;                        // Last time we updated display
    double dElapsedSeconds;                     // Total elapsed seconds since start
    uint32_t amDigits[DISPLAY_DIGITS];          // Digits to display
    SemaphoreHandle_t hTimerSemaphore;          // Semaphore to run a tick
    led_strip_handle_t ahLEDStrip;              // IO Handle for the LED Strip
    char scrollMessage[MAX_SCROLL_MESSAGE + 1]; // Scroll message buffer
    bool bScrollable;                           // Flag indicating if the scroll message is actually scrollable
    rgb_t scrollColor;                          // Color to use for the scroll message

    // OTA Shared Data
    char aucOtaNewVersion[32];
    char aucOtaImageDate[16];
    char aucOtaImageTime[16];
    int nOtaImageSize;
    int nOtaBytesRead;

  } APP_DATA;

  extern APP_DATA appData;

  extern uint32_t Get_Segment_Mask(int nVal);

  extern void init_leds(void);
  extern void turn_on_user_led(void);
  extern void turn_off_user_led(void);

  extern led_strip_handle_t configure_led(int gpio);
  extern rgb_t getRGB(void);
  extern void Process_Tick(TimerHandle_t xTimer);
  extern void dfplayer_send_command(uint8_t command, uint16_t param);
  extern void dfplayer_play_track(uint16_t track_num);
  extern void dfplayer_set_volume(uint8_t volume);
  extern void dfplayer_safe_init(uint8_t initial_volume, uint16_t test_track);
  extern void init_uart(void);
  extern void Update_Display(void);
  extern void HW_Initialize(void);
  extern void APP_Initialize(void);
  extern void ScrollMessage(void);
  extern void showSecondsCountdownTime(void);
  extern void showCountdownTime(void);
  extern void Switch_To_State(APP_STATES newState);
  extern bool HandleTimedState(double timeLimit, int track, APP_STATES nextState);
  extern void APP_Main(void);
  extern led_strip_handle_t configure_led(int gpio);
  void APP_Initialize(void);
  void APP_Main(void);
  char *getBuiltinPatternsJSON(void);
  void TREE_DisplayRGB(void);
  void TREE_InitRGB(bool bHalfBright);
  void TREE_Tick(TimerHandle_t xTimer);
  void TREE_Timer(TimerHandle_t xTimer);
  void TREE_Refresh(TimerHandle_t xTimer);
  void TREE_Button(void);
  void TREE_NextTick(void);
  void AP_Start(void);
  void Wifi_Init(void);
  void FileSystem_Init(void);
  void list_spiffs_files(void);
  bool makeSpiffsFileName(const char *filename, char *buffer, int len);
  bool ends_with(const char *str, const char *suffix);
  char *getFilesBySuffixJson(const char *pSuffix);
  int GetFileSize(const char *filename);
  int ReadFile(const char *filename, char *buffer, int len);
  void ota_update_task(void *pvParameter);
  void check_dns(void);
#endif /* _APP_H */

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

/*******************************************************************************
 End of File
 */
/**
 * @file app.c
 * @author John Toebes (john@toebes.com)
 * @brief
 * @version 0.1
 * @date 2025-05-13
 *
 * @copyright
 * Copyright (c) 2025 John A. Toebes
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "app.h"
static const char *TAG = "app";

APP_DATA appData;

// Segment mask for unsupported characters
#define SEG_UNKNOWN (SEG_A | SEG_B | SEG_E | SEG_G | SEG_DOT)

static const uint8_t segment_map[128] = {
    [' '] = 0,
    ['*'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG_DOT,
    ['-'] = SEG_G,
    ['.'] = SEG_DOT,
    ['_'] = SEG_D,
    ['0'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    ['1'] = SEG_B | SEG_C,
    ['2'] = SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
    ['3'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
    ['4'] = SEG_B | SEG_C | SEG_F | SEG_G,
    ['5'] = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
    ['6'] = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    ['7'] = SEG_A | SEG_B | SEG_C,
    ['8'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    ['9'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,
    ['A'] = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
    ['a'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
    ['B'] = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    ['b'] = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    ['C'] = SEG_A | SEG_D | SEG_E | SEG_F,
    ['c'] = SEG_D | SEG_E | SEG_G,
    ['D'] = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
    ['d'] = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
    ['E'] = SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
    ['e'] = SEG_A | SEG_B | SEG_D | SEG_E | SEG_F | SEG_G,
    ['F'] = SEG_A | SEG_E | SEG_F | SEG_G,
    ['f'] = SEG_A | SEG_E | SEG_F | SEG_G,
    ['G'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,
    ['g'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,
    ['H'] = SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
    ['h'] = SEG_C | SEG_E | SEG_F | SEG_G,
    ['I'] = SEG_E | SEG_F,
    ['i'] = SEG_E,
    ['J'] = SEG_B | SEG_C | SEG_D | SEG_E,
    ['j'] = SEG_B | SEG_C | SEG_D,
    ['K'] = SEG_E | SEG_F | SEG_G,
    ['k'] = SEG_E | SEG_F | SEG_G,
    ['L'] = SEG_D | SEG_E | SEG_F,
    ['l'] = SEG_E | SEG_F,
    ['M'] = SEG_A | SEG_C | SEG_E,
    ['m'] = SEG_A | SEG_C | SEG_E,
    ['N'] = SEG_C | SEG_E | SEG_G,
    ['n'] = SEG_C | SEG_E | SEG_G,
    ['O'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    ['o'] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    ['P'] = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,
    ['p'] = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,
    ['Q'] = SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,
    ['q'] = SEG_A | SEG_B | SEG_C | SEG_F | SEG_G,
    ['R'] = SEG_E | SEG_G,
    ['r'] = SEG_E | SEG_G,
    ['S'] = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
    ['s'] = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
    ['T'] = SEG_A | SEG_B | SEG_C,
    ['t'] = SEG_A | SEG_B | SEG_C,
    ['U'] = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    ['u'] = SEG_C | SEG_D | SEG_E,
    ['V'] = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    ['v'] = SEG_C | SEG_D | SEG_E,
    ['W'] = SEG_B | SEG_D | SEG_F,
    ['w'] = SEG_B | SEG_D | SEG_F,
    ['X'] = SEG_B | SEG_C | SEG_G,
    ['x'] = SEG_B | SEG_C | SEG_G,
    ['Y'] = SEG_B | SEG_C | SEG_F | SEG_G,
    ['y'] = SEG_B | SEG_C | SEG_F | SEG_G,
    ['Z'] = SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
    ['z'] = SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
    ['^'] = SEG_A,
    ['?'] = SEG_UNKNOWN};

/**
 * @brief Return which Segments correspond to a given letter
 *
 * @param nVal Letter/value to look up
 * @return uint32_t Mask of segments to turn on
 */
uint32_t Get_Segment_Mask(int ch)
{
    if (ch == ' ')
    {
        return 0;
    }
    if (ch >= 0 && ch < 128)
    {
        return segment_map[(unsigned char)ch] ? segment_map[(unsigned char)ch] : SEG_UNKNOWN;
    }
    return SEG_UNKNOWN;
}
/**
 * @brief Determine the color to display in
 *
 * @return rgb_t RGB value corresponding to the state of the application
 */
rgb_t getRGB(void)
{
    switch (appData.stateApp)
    {
    case APP_STATE_SCROLLMSG:
        return appData.scrollColor;
        // return RGB_WHITE;

    case APP_STATE_WAIT_START:
    case APP_STATE_DONE:
        return RGB_RED;

    case APP_STATE_TIMED_QUESTION:
        return RGB_GREEN;

    case APP_STATE_WAIT_25_MINUTES:
    case APP_STATE_WAIT_10_MINUTES:
    case APP_STATE_WAIT_2_MINUTES:
    case APP_STATE_WAIT_10_SECONDS:
        return RGB_WHITE;

    case APP_STATE_FINAL_10SECONDS:
        return RGB_YELLOW;

    case APP_STATE_CONFIG:
        return RGB_PURPLE;
    }
    return RGB_BLACK;
}
/**
 * @brief Handles the timer callback for the timer to check button presses.
 *
 * @param xTimer Timer handle for the callback
 */
void Process_Tick(TimerHandle_t xTimer)
{
    if (gpio_get_level(PUSH_BUTTON_PORT) == 0)
    {
        appData.nPressedCount++;
        if (appData.nReleasedCount > 0 &&
            appData.nReleasedCount < TICKS_DOUBLETAP)
        {
            Switch_To_State(APP_STATE_DONE);
        }
        else if (appData.nPressedCount == TICKS_PRESSED)
        {
            if (appData.stateApp == APP_STATE_WAIT_START)
            {
                ESP_LOGI(TAG, "Starting Event");
                Switch_To_State(APP_STATE_TIMED_QUESTION);
            }
            else if (appData.stateApp == APP_STATE_DONE)
            {
                ESP_LOGI(TAG, "Going to Codebusters");
                Switch_To_State(APP_STATE_SCROLLMSG);
            }
        }
        else if (appData.nPressedCount == TICKS_REQUEST_RESET &&
                 appData.stateApp != APP_STATE_WAIT_START)
        {
            ESP_LOGI(TAG, "Reset to Wait State");
            Switch_To_State(APP_STATE_WAIT_START);
        }
        else if (appData.nPressedCount == TICKS_REQUEST_CONFIG &&
                 appData.stateApp == APP_STATE_SCROLLMSG)
        {
            ESP_LOGI(TAG, "Going to Config State");
            Switch_To_State(APP_STATE_CONFIG);
        }
        appData.nReleasedCount = 0;
    }
    else
    {
        appData.nPressedCount = 0;
        appData.nReleasedCount++;
    }
    xSemaphoreGiveFromISR(appData.hTimerSemaphore, NULL);
}

/**
 * @brief Compute checksum (2's complement of sum of bytes 1 to 6)
 *
 * @param cmd Command bytes to checksum
 * @return uint16_t Place to put checksum value
 */
static uint16_t dfplayer_checksum(uint8_t *cmd)
{
    uint16_t sum = 0;
    for (int i = 1; i < 7; i++)
    {
        sum += cmd[i];
    }
    return 0xFFFF - sum + 1;
}

/**
 * @brief Send DFPlayer command with checksum
 *
 * @param command Command to send to the player
 * @param param Parameter for the command
 */
void dfplayer_send_command(uint8_t command, uint16_t param)
{
    uint8_t cmd[DFPLAYER_CMD_LENGTH] = {0};

    cmd[0] = 0x7E;                // Start byte
    cmd[1] = 0xFF;                // Version
    cmd[2] = 0x06;                // Length
    cmd[3] = command;             // Command
    cmd[4] = 0x00;                // No feedback
    cmd[5] = (param >> 8) & 0xFF; // High byte of param
    cmd[6] = param & 0xFF;        // Low byte of param

    uint16_t checksum = dfplayer_checksum(cmd);
    cmd[7] = (checksum >> 8) & 0xFF; // Checksum high byte
    cmd[8] = checksum & 0xFF;        // Checksum low byte
    cmd[9] = 0xEF;                   // End byte

    uart_write_bytes(UART_NUM_1, (const char *)cmd, sizeof(cmd));

    ESP_LOG_BUFFER_HEX("DFPlayer CMD", cmd, sizeof(cmd));
}
/**
 * @brief Play a specific track number
 *
 * @param track_num Which track number to play
 */
void dfplayer_play_track(uint16_t track_num)
{
    dfplayer_send_command(0x03, track_num);
}
/**
 * @brief Set Volume
 *
 * @param volume Volume (0-30)
 */
void dfplayer_set_volume(uint8_t volume)
{
    dfplayer_send_command(0x06, volume);
}
void dfplayer_safe_init(uint8_t initial_volume, uint16_t test_track)
{
    ESP_LOGI("DFPlayer", "Initializing DFPlayer (waiting %d ms for power stabilization)...", DFPLAYER_INIT_DELAY_MS);
    vTaskDelay(pdMS_TO_TICKS(DFPLAYER_INIT_DELAY_MS));

    // Set initial volume
    dfplayer_set_volume(initial_volume);
    vTaskDelay(pdMS_TO_TICKS(100)); // Small delay after volume set

    // Try playing the test track with retry logic
    for (int i = 0; i < DFPLAYER_RETRY_COUNT; i++)
    {
        ESP_LOGI("DFPlayer", "Attempting to play track %d (attempt %d)...", test_track, i + 1);
        dfplayer_play_track(test_track);
        vTaskDelay(pdMS_TO_TICKS(DFPLAYER_RETRY_DELAY_MS));
    }

    ESP_LOGI("DFPlayer", "DFPlayer Safe Init Complete.");
}
/**
 * @brief Initialize UART
 *
 */
void init_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_driver_install(UART_NUM, 1024, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Wait for DFPlayer to respond
    vTaskDelay(pdMS_TO_TICKS(300));
}
/**
 * @brief Display the current value on the timer
 * appData.amDigits are displayed using the current RGB Color for the state
 */
void Update_Display(void)
{

    led_strip_handle_t led_strip = appData.ahLEDStrip;
    rgb_t RGBOn = getRGB();
    int nLed = 0;
    for (int nDigit = 0; nDigit < DISPLAY_DIGITS; nDigit++)
    {
        int nMask = SEG_A;
        int mThisDigit = appData.amDigits[nDigit];
        for (int nSegment = 0; nSegment < DIGIT_SEGMENTS; nSegment++)
        {
            rgb_t color = RGB_BLACK;
            if (mThisDigit & nMask)
            {
                color = RGBOn;
            }
            int nSegmentLeds = BASE_LEDS_PER_SEGMENT;
            if (nSegment == (DIGIT_SEGMENTS - 1))
            {
                if (nDigit == 0)
                {
                    nSegmentLeds = 2;
                }
                else if (nDigit == 1)
                {
                    nSegmentLeds = 1;
                }
                else
                {
                    nSegmentLeds = 0; // No segment for the last digit
                }
            }
            // ESP_LOGI(TAG, "Digit:%d Segment:%d Display:%08x Mask: %08x Color(%d,%d,%d)", nDigit, nSegment, mThisDigit, nMask, (int)RGB_GET_R(color), (int)RGB_GET_G(color), (int)RGB_GET_B(color));
            while (nSegmentLeds-- > 0)
            {
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, nLed++, RGB_GET_R(color), RGB_GET_G(color), RGB_GET_B(color)));
            }
            nMask <<= 1;
        }
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}
/**
 * @brief Initialize all the hardware
 *
 */
void HW_Initialize(void)
{
    ESP_LOGI(TAG, "Requesting strip with %d LEDS", LED_STRIP_TOTAL_LEDS);
    appData.ahLEDStrip = configure_led(LED_STRIP_PORT);

    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL << PUSH_BUTTON_PORT;
    // disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
    init_uart();
    init_leds();
    // dfplayer_safe_init(30, TRACK_WELCOME_TO_CODEBUSTERS);
}
const char *esp_reset_reason_to_name(esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_POWERON:
        return "Power-On";
    case ESP_RST_EXT:
        return "External Reset";
    case ESP_RST_SW:
        return "Software Reset";
    case ESP_RST_PANIC:
        return "Exception/Panic Reset";
    case ESP_RST_INT_WDT:
        return "Interrupt Watchdog";
    case ESP_RST_TASK_WDT:
        return "Task Watchdog";
    case ESP_RST_WDT:
        return "Other Watchdog";
    case ESP_RST_DEEPSLEEP:
        return "Wake from Deep Sleep";
    case ESP_RST_BROWNOUT:
        return "Brownout";
    case ESP_RST_SDIO:
        return "SDIO Reset";
    default:
        return "Unknown";
    }
}
/**
 * @brief Initialize all the application data and start the timer
 *
 */
void APP_Initialize(void)
{

    esp_reset_reason_t reason = esp_reset_reason();
    ESP_LOGI(TAG, "***Reset reason: %d (%s)", reason, esp_reset_reason_to_name(reason));

    TimerHandle_t hTickTimer;
    appData.nPressedCount = 0;
    appData.nReleasedCount = 0;

    appData.hTimerSemaphore = xSemaphoreCreateBinary();
    appData.bStartState = true;
    appData.tStartTime = 0;
    appData.dLastSeconds = 0;
    appData.scrollColor = RGB_WHITE; // Default color for the scroll message
    strncpy(appData.scrollMessage, "1:2.3", MAX_SCROLL_MESSAGE);
    appData.bScrollable = false; // Set to true to allow scrolling
    // strncpy(appData.scrollMessage, "HELLO  ", MAX_SCROLL_MESSAGE);
    appData.scrollMessage[MAX_SCROLL_MESSAGE] = '\0'; // Null-terminate the string
    // appData.bScrollable = true;                       // Set to true to allow scrolling

    for (int nDigit = 0; nDigit < DISPLAY_DIGITS; nDigit++)
    {
        appData.amDigits[nDigit] = SEG_ALL;
    }

    ESP_LOGI(TAG, "Timer Interval is %ld", TIMER_INTERVAL);
    hTickTimer = xTimerCreate("Tick", TIMER_INTERVAL, pdTRUE, (void *)2, &Process_Tick);
    // Check if the timer was created successfully
    if (hTickTimer == NULL)
    {
        ESP_LOGI(TAG, "Failed to create timer");
    }
    else
    {
        // Start the timers
        if (xTimerStart(hTickTimer, 0) != pdPASS)
        {
            ESP_LOGI(TAG, "Failed to start timer\n");
        }
    }
    Switch_To_State(APP_STATE_SCROLLMSG);
}
/**
 * @brief Scroll the Codebusters text at the rate of 2/second
 *
 */
void ScrollMessage(void)
{
    // int nChars = strlen(appData.scrollMessage);

    if (appData.bStartState)
    {
        appData.tStartTime = esp_timer_get_time();
        appData.dLastSeconds = -1;
        appData.bStartState = false;
        dfplayer_play_track(TRACK_WELCOME_TO_CODEBUSTERS);
    }
    double elapsed_ticks = 2 * appData.dElapsedSeconds;
    int tick_spot = (int)(round(elapsed_ticks));

    int slot = (tick_spot - 1 + strlen(appData.scrollMessage)) % strlen(appData.scrollMessage);
    if (slot != appData.dLastSeconds)
    {
        appData.dLastSeconds = slot;
        if (appData.bScrollable)
        {
            appData.dLastSeconds = slot;
            int nextSlot = (slot + 1) % strlen(appData.scrollMessage);
            int thirdSlot = (slot + 2) % strlen(appData.scrollMessage);
            appData.amDigits[0] = Get_Segment_Mask(appData.scrollMessage[slot]);
            appData.amDigits[1] = Get_Segment_Mask(appData.scrollMessage[nextSlot]);
            appData.amDigits[2] = Get_Segment_Mask(appData.scrollMessage[thirdSlot]);
            // ESP_LOGI(TAG, "Display: %ld %ld %ld\n", appData.amDigits[0], appData.amDigits[1], appData.amDigits[2]);
        }
        else
        {
            // Not scrollable, we just need to display the characters
            int pos = 0;
            appData.amDigits[0] = Get_Segment_Mask(appData.scrollMessage[pos++]);
            // A Colon is special and we make it part of the first digit
            if (appData.scrollMessage[pos] == ':')
            {
                pos++;
                if (tick_spot & 1)
                {
                    appData.amDigits[0] |= SEG_DOT; // Show the colon (blinking once a second)
                }
            }
            appData.amDigits[1] = Get_Segment_Mask(appData.scrollMessage[pos++]);
            // Likewise a period after the second digit also is part of it
            if (appData.scrollMessage[pos] == '.')
            {
                pos++;
                appData.amDigits[1] |= SEG_DOT; // Show the colon (blinking once a second)
            }
            appData.amDigits[2] = Get_Segment_Mask(appData.scrollMessage[pos]);
        }
        Update_Display();
    }
}
/**
 * @brief Show the countdown to zero in 10th of a second
 *
 */
void showSecondsCountdownTime(void)
{
    double dElapsedSecondsTenths = roundf((appData.tNow - appData.tStartTime) / 100000.0);

    int nTenthsRemain = ceil(((float)EVENT_LENGTH * 10) - dElapsedSecondsTenths);
    if (nTenthsRemain != appData.dLastSeconds)
    {
        appData.dLastSeconds = nTenthsRemain;

        int nSeconds = trunc(nTenthsRemain / 10);
        int nTenths = nTenthsRemain % 10;

        appData.amDigits[0] = Get_Segment_Mask(nSeconds) | SEG_DOT;
        appData.amDigits[1] = Get_Segment_Mask(nTenths);
        Update_Display();
    }
}
/**
 * @brief Show the remaining time in minutes
 *
 */
void showCountdownTime(void)
{
    if (floor(appData.dElapsedSeconds) != floor(appData.dLastSeconds))
    {
        int nSecondsRemain;
        int nMinutesRemain;
        int nTenDigit;
        int nOneDigit;
        appData.dLastSeconds = appData.dElapsedSeconds;
        nSecondsRemain = ceil(EVENT_LENGTH - appData.dElapsedSeconds);
        nMinutesRemain = (((nSecondsRemain * SCALE_SPEED) + 59) / 60);

        nTenDigit = (nMinutesRemain % 100) / 10;
        nOneDigit = nMinutesRemain % 10;
        if (nTenDigit == 0)
        {
            nTenDigit = ' ';
        }
        appData.amDigits[0] = Get_Segment_Mask(nTenDigit);
        appData.amDigits[1] = Get_Segment_Mask(nOneDigit);
        ESP_LOGI(TAG, "Remain: %02d:%02d Time: %.2f", nMinutesRemain, nSecondsRemain % 60, appData.dElapsedSeconds);
        Update_Display();
    }
}
/**
 * @brief Switch the state that the application is in
 *
 * @param newState New state to switch to
 */
void Switch_To_State(APP_STATES newState)
{
    appData.bStartState = true;
    appData.stateApp = newState;
}
/**
 * @brief Process a timed state transition
 *
 * @param timeLimit How long to stay in this state
 * @param track Track to play if the state transitions
 * @param nextState New state to transition
 * @return true State transitioned
 * @return false State did not transition
 */
bool HandleTimedState(double timeLimit, int track, APP_STATES nextState)
{
    if (appData.dElapsedSeconds >= timeLimit)
    {
        if (track != -1)
        {
            dfplayer_play_track(track);
        }
        Switch_To_State(nextState);
        return true;
    }
    showCountdownTime();
    return false;
}
/**
 * @brief Main application loop
 *
 */
void APP_Main(void)
{
    HW_Initialize();
    APP_Initialize();
    Wifi_Init();
    FileSystem_Init();
    ESP_LOGI(TAG, "Initialized");

    appData.tStartTime = esp_timer_get_time(); // Record start time in microseconds
    appData.dLastSeconds = 0;

    for (;;)
    {

        // Wait for the timer to tell us to run another step.  Note that we will
        // timeout after double the expected time just to keep us running.
        xSemaphoreTake(appData.hTimerSemaphore, 2 * TIMER_INTERVAL);
        appData.tNow = esp_timer_get_time();
        // Compute the elapsed time to the nearest 10th of a second.
        appData.dElapsedSeconds = roundf((float)(appData.tNow - appData.tStartTime) / 100000.0) / 10.0;

        switch (appData.stateApp)
        {
        case APP_STATE_SCROLLMSG:
            ScrollMessage();
            break;
        case APP_STATE_WAIT_START:
            appData.amDigits[0] = Get_Segment_Mask(5);
            appData.amDigits[1] = Get_Segment_Mask(0);
            Update_Display();
            break;
        case APP_STATE_TIMED_QUESTION:

            if (appData.bStartState)
            {
                appData.tStartTime = appData.tNow;
                appData.dElapsedSeconds = 0;
                appData.bStartState = false;
            }
            HandleTimedState(END_TIMED_SECONDS, TRACK_NO_MORE_TIMED_BONUS, APP_STATE_WAIT_25_MINUTES);
            break;
        case APP_STATE_WAIT_25_MINUTES:
            HandleTimedState(ANNOUNCE_25_MINUTES, TRACK_25_MINUTES_REMAIN, APP_STATE_WAIT_10_MINUTES);
            break;
        case APP_STATE_WAIT_10_MINUTES:
            HandleTimedState(ANNOUNCE_10_MINUTES, TRACK_10_MINUTES_REMAIN, APP_STATE_WAIT_2_MINUTES);
            break;
        case APP_STATE_WAIT_2_MINUTES:
            HandleTimedState(ANNOUNCE_2_MINUTES, TRACK_2_MINUTES_REMAIN, APP_STATE_WAIT_10_SECONDS);
            break;
        case APP_STATE_WAIT_10_SECONDS:
            HandleTimedState(FINAL_SECONDS, -1, APP_STATE_FINAL_10SECONDS);
            break;

        case APP_STATE_FINAL_10SECONDS:
            if (appData.dElapsedSeconds >= EVENT_LENGTH)
            {
                dfplayer_play_track(TRACK_TIMES_UP);
                Switch_To_State(APP_STATE_DONE);
            }
            showSecondsCountdownTime();
            break;
        case APP_STATE_DONE:
            if (appData.bStartState)
            {
                appData.tStartTime = appData.tNow;
                appData.dElapsedSeconds = 0;
                appData.bStartState = false;

                appData.amDigits[0] = Get_Segment_Mask(0);
                appData.amDigits[1] = Get_Segment_Mask(0);
                Update_Display();
            }
            break;
        case APP_STATE_CONFIG:
            break;
        }
    }
}

// // *****************************************************************************
// // *****************************************************************************
// // Section: Included Files
// // *****************************************************************************
// // *****************************************************************************

// #include "app.h"
// static const char *TAG = "app";

// // *****************************************************************************
// // *****************************************************************************
// // Section: Global Data Definitions
// // *****************************************************************************
// // *****************************************************************************

// // *****************************************************************************
// /* Application Data

//   Summary:
//     Holds application data

//   Description:
//     This structure holds the application's data.

//   Remarks:
//     This structure should be initialized by the APP_Initialize function.

//     Application strings and buffers are be defined outside this structure.
//  */

// APP_DATA appData;

// const unsigned char offsetInc[] = {
//     1,
//     7,
//     21,
//     42,
// };

// char *getBuiltinPatternsJSON(void)
// {
//     int nSize = 0;
//     char *pResult = NULL;
//     char *pCurrent = NULL;

//     // We didn't load the pattern (either it didn't exist or was too big to load)
//     // So look through our mappings of pattern names to preloaded patterns to see
//     // if we get a match.
//     int num_entries = sizeof(pPatternMap) / sizeof(pPatternMap[0]);
//     for (int i = 0; i < num_entries; ++i)
//     {
//         nSize += strlen(pPatternMap[i].name) + 3; // Account for the quotes and the comma
//     }
//     // If there was nothing there, don't bother to allocate any memory.  However, this
//     // should never happen as there should be at least one builtin entry
//     if (nSize == 0)
//     {
//         return NULL;
//     }
//     pCurrent = pResult = malloc(nSize + 3); // Account for the [] and trailing null
//     if (pCurrent == NULL)
//     {
//         return NULL;
//     }
//     for (int i = 0; i < num_entries; ++i)
//     {
//         *pCurrent++ = i ? ',' : '[';
//         *pCurrent++ = '"';
//         strcpy(pCurrent, pPatternMap[i].name);
//         pCurrent += strlen(pPatternMap[i].name);
//         *pCurrent++ = '"';
//     }
//     strcpy(pCurrent, "]");
//     return pResult;
// }

// /**
//  * @brief Advance to the next pattern, checking to see if it was the last in the list
//  *
//  * @return true There are more patterns in the list
//  * @return false We advanced past the end of the list and restarted
//  */
// bool advanceCurrentPattern(void)
// {
//     // Are we already at the end of the list?
//     if (*appData.pCurrentPattern)
//     {
//         // No so advance past the current string
//         appData.pCurrentPattern += strlen(appData.pCurrentPattern) + 1;
//     }
//     // See if we had advanced to the end.
//     if (*appData.pCurrentPattern != 0)
//     {
//         // No, there are more to go so let them know
//         return true;
//     }
//     // Past the end of the list, so reset to the start
//     appData.pCurrentPattern = appData.pPatternList;
//     return false;
// }

// /******************************************************************************
//  * Function:
//  *   void TREE_Tick(uintptr_t context, uint32_t currTick)
//  *
//  *  Summary:
//  *   Handles the timer callback for the timer to advance the sequence.
//  *
//  * Description:
//  *   This function is called 12 times a second and triggers when the next step
//  *   in the LED pattern sequence is to occur.
//  *
//  */
// void TREE_Tick(TimerHandle_t xTimer)
// {
//     appData.nDebugTickCount++;
//     /*esp_err_t ret = */ adc_oneshot_read(appData.adc1_handle, ADC_CHANNEL, &appData.nLightSensor);

//     if (gpio_get_level(PUSH_BUTTON_PORT) == 0 || appData.bTouchPressed)
//     {
//         appData.nPressedCount++;
//         if (appData.nReleasedCount > 0 &&
//             appData.nReleasedCount < TICKS_DOUBLETAP)
//         {
//             if (appData.stateApp == TREE_STATE_AP_RUNNING ||
//                 appData.stateApp == TREE_STATE_AP_CONNECTED)
//             {
//                 esp_restart();
//             }
//             appData.stateApp = TREE_STATE_RUNNING;
//             // TODO: See if there is a better way to do this.
//             // It feels odd to set the current pattern to nothing, but
//             // it means when it advances the pattern it goes to the first one in the list
//             appData.pCurrentPattern = "\0"; // appData.pPatternList;
//             // appData.nCurrentPattern = 0;
//             appData.bLoadPattern = true;
//             appData.cyclepattern = true;
//         }
//         else if (appData.nPressedCount == TICKS_PRESSED)
//         {
//             if (appData.stateApp == TREE_STATE_AP_RUNNING ||
//                 appData.stateApp == TREE_STATE_AP_CONNECTED)
//             {
//                 esp_restart();
//             }
//             appData.stateApp = TREE_STATE_RUNNING;
//             appData.bLoadPattern = true;
//             appData.cyclepattern = false;
//             if (!advanceCurrentPattern())
//             {
//                 appData.cyclepattern = true;
//             }
//             appData.bLoadPattern = true;
//         }
//         else if (appData.nPressedCount == TICKS_POWERDOWN)
//         {
//             appData.stateApp = TREE_STATE_START_POWERDOWN;
//         }
//         else if (appData.nPressedCount == TICKS_WIFICONFIG)
//         {
//             appData.stateApp = TREE_STATE_START_AP;
//         }
//         appData.nReleasedCount = 0;
//     }
//     else
//     {
//         appData.nPressedCount = 0;
//         appData.nReleasedCount++;
//     }
//     if (appData.bLoadPattern)
//     {
//         appData.nPatternTime = 0;
//     }
//     else if (appData.cyclepattern)
//     {
//         appData.nPatternTime++;
//         if (appData.nPatternTime >= (TICKS_PER_SECOND * CYCLE_TIME))
//         {
//             advanceCurrentPattern();
//             appData.bLoadPattern = true;
//             appData.nPatternTime = 0;
//         }
//         else
//         {
//             appData.bNextStep = true;
//         }
//     }
//     else
//     {
//         appData.bNextStep = true;
//     }
// }
// void TREE_Timer(TimerHandle_t xTimer)
// {
//     xSemaphoreGiveFromISR(appData.hTimerSemaphore, NULL);
// }

// static void button_handler(touch_button_handle_t out_handle, touch_button_message_t *out_message, void *arg)
// {
//     APP_DATA *pAppData = (APP_DATA *)arg;
//     (void)out_handle; // Unused
//     if (out_message->event == TOUCH_BUTTON_EVT_ON_PRESS)
//     {
//         ESP_LOGI(TAG, "Button[%d] Press", (int)arg);
//         pAppData->bTouchPressed = true;
//     }
//     else if (out_message->event == TOUCH_BUTTON_EVT_ON_RELEASE)
//     {
//         ESP_LOGI(TAG, "Button[%d] Release", (int)arg);
//         pAppData->bTouchPressed = false;
//     }
//     else if (out_message->event == TOUCH_BUTTON_EVT_ON_LONGPRESS)
//     {
//         ESP_LOGI(TAG, "Button[%d] LongPress", (int)arg);
//     }
// }

// void HW_Initialize(void)
// {
//     const int anGPIOSlots[LED_COLUMNS] = {LED_STRIP_AB, LED_STRIP_CD, LED_STRIP_EF, LED_STRIP_GH};

//     // Initialize all the LED columns
//     for (int nCol = 0; nCol < LED_COLUMNS; nCol++)
//     {
//         appData.ahLEDStrips[nCol] = configure_led(anGPIOSlots[nCol]);
//     }

//     // Set up the ADC for the light sensor
//     adc_oneshot_unit_init_cfg_t unit_config = {
//         .unit_id = ADC_UNIT, // Select ADC1
//     };
//     ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &appData.adc1_handle));

//     // Configure ADC channel
//     adc_oneshot_chan_cfg_t channel_config = {
//         .bitwidth = ADC_BITWIDTH_DEFAULT, // Default 12-bit resolution
//         .atten = ADC_ATTEN,               // Attenuation for voltage range
//     };
//     ESP_ERROR_CHECK(adc_oneshot_config_channel(appData.adc1_handle, ADC_CHANNEL, &channel_config));

//     // zero-initialize the config structure.
//     gpio_config_t io_conf = {};
//     // disable interrupt
//     io_conf.intr_type = GPIO_INTR_DISABLE;
//     // set as output mode
//     io_conf.mode = GPIO_MODE_INPUT;
//     // bit mask of the pins that you want to set,e.g.GPIO18/19
//     io_conf.pin_bit_mask = 1ULL << PUSH_BUTTON_PORT;
//     // disable pull-down mode
//     io_conf.pull_down_en = 0;
//     // disable pull-up mode
//     io_conf.pull_up_en = 1;
//     // configure GPIO with the given settings
//     gpio_config(&io_conf);

//     //

//     /* Initialize Touch Element library */
//     touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
//     ESP_ERROR_CHECK(touch_element_install(&global_config));
//     ESP_LOGI(TAG, "Touch element library installed");

//     touch_button_global_config_t button_global_config = TOUCH_BUTTON_GLOBAL_DEFAULT_CONFIG();
//     ESP_ERROR_CHECK(touch_button_install(&button_global_config));
//     ESP_LOGI(TAG, "Touch button installed");
//     touch_button_config_t button_config = {
//         .channel_num = TOUCH_PAD_NUM2,
//         .channel_sens = 0.1F};
//     /* Create Touch buttons */
//     ESP_ERROR_CHECK(touch_button_create(&button_config, &appData.button_handle));
//     /* Subscribe touch button events (On Press, On Release, On LongPress) */
//     ESP_ERROR_CHECK(touch_button_subscribe_event(appData.button_handle,
//                                                  TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE,
//                                                  (void *)&appData));

//     ESP_ERROR_CHECK(touch_button_set_dispatch_method(appData.button_handle, TOUCH_ELEM_DISP_CALLBACK));
//     /* Register a handler function to handle event messages */
//     ESP_ERROR_CHECK(touch_button_set_callback(appData.button_handle, button_handler));

//     /* Set LongPress event trigger threshold time */
//     ESP_ERROR_CHECK(touch_button_set_longpress(appData.button_handle, 2000));
//     ESP_LOGI(TAG, "Touch buttons created");

//     touch_element_start();
//     ESP_LOGI(TAG, "Touch element library start");
// }

// void APP_Initialize(void)
// {
//     // TimerHandle_t hTickTimer;
//     TimerHandle_t hRefreshTimer;

//     appData.nRefreshLimit = DEFAULT_REFRESH_LIMIT;
//     appData.nRefreshCount = 0;
//     appData.nOnCount = DEFAULT_REFRESH_ON;

//     appData.eRoutineId = IN_APP_Tasks;
//     appData.initialized = false;
//     appData.cyclepattern = true;
//     appData.nPressedCount = 0;
//     appData.nReleasedCount = 0;
//     appData.pPatternList = NULL;
//     appData.nPatternListSize = 0;
//     appData.pCurrentPattern = NULL;
//     appData.bLoadPatternList = false;
//     appData.bNextStep = false;
//     appData.bLoadPattern = false;
//     Load_PatternList();
//     appData.pCurrentPattern = "\0";
//     appData.pLoadedPattern = NULL;
//     appData.nLoadedPatternSize = 0;

//     // Initialize the OTA state
//     strcpy(appData.aucOtaNewVersion, "Not Set");
//     strcpy(appData.aucOtaImageDate, "Not Set");
//     strcpy(appData.aucOtaImageTime, "Not Set");
//     appData.nOtaImageSize = 0;
//     appData.nOtaBytesRead = 0;

//     SetupGamma(appData.aucRedMap, LIMIT_RED, RED_GAMMA);
//     SetupGamma(appData.aucGreenMap, LIMIT_GREEN, GREEN_GAMMA);
//     SetupGamma(appData.aucBlueMap, LIMIT_BLUE, BLUE_GAMMA);
//     TREE_LoadNamedPattern("Startup", 0);
//     TREE_CalcRGB();
//     appData.nPatternTime = (TICKS_PER_SECOND * (CYCLE_TIME - 3));

//     appData.maskval = 0;
//     appData.curColor = 0;
//     appData.curRow = 0;
//     appData.curColumn = 0;
//     appData.swingState = 0;
//     appData.hTimerSemaphore = xSemaphoreCreateBinary();
//     appData.initialized = true;

//     HW_Initialize();

//     hRefreshTimer = xTimerCreate("Refresh", REFRESH_INTERVAL, pdTRUE, NULL, &TREE_Timer);
//     // Check if the timers were created successfully
//     if (hRefreshTimer == NULL)
//     {
//         ESP_LOGI(TAG, "Failed to create Refresh Timer");
//     }
//     else
//     {
//         // Start the timers
//         if (xTimerStart(hRefreshTimer, 0) != pdPASS)
//         {
//             ESP_LOGI(TAG, "Failed to start Refresh Timer\n");
//         }
//     }

//     // hRefreshTimer = xTimerCreate("Refresh", REFRESH_INTERVAL, pdTRUE, NULL, &TREE_Refresh);
//     // // Check if the timers were created successfully
//     // if (hRefreshTimer == NULL)
//     // {
//     //     ESP_LOGI(TAG, "Failed to create Refresh Timer");
//     // }
//     // else
//     // {
//     //     // Start the timers
//     //     if (xTimerStart(hRefreshTimer, 0) != pdPASS)
//     //     {
//     //         ESP_LOGI(TAG, "Failed to start Refresh Timer\n");
//     //     }
//     // }

//     // ESP_LOGI(TAG, "Timer Interval=%ld Refresh Interval=%ld TICK_RATE_HZ=%d", TIMER_INTERVAL, REFRESH_INTERVAL, configTICK_RATE_HZ);

//     // hTickTimer = xTimerCreate("Tick", TIMER_INTERVAL, pdTRUE, NULL, &TREE_Tick);
//     // // Check if the timers were created successfully
//     // if (hTickTimer == NULL)
//     // {
//     //     ESP_LOGI(TAG, "Failed to create Main Timer");
//     // }
//     // else
//     // {
//     //     // Start the timers
//     //     if (xTimerStart(hTickTimer, 0) != pdPASS)
//     //     {
//     //         ESP_LOGI(TAG, "Failed to start Main Timer\n");
//     //     }
//     // }

//     appData.stateApp = TREE_STATE_RUNNING;
// }
// /**
//  * @brief Load the pattern list from flash or use the default
//  * if no list can be found in flash
//  *
//  */
// void Load_PatternList(void)
// {
//     const char *patternFilename = "patterns.txt";
//     int fileSize;

//     // If we had previously loaded a pattern, free the memory from it
//     if (appData.nPatternListSize)
//     {
//         free((void *)appData.pPatternList);
//         appData.pPatternList = NULL;
//         appData.nPatternListSize = 0;
//     }
//     appData.nPatternListSize = 0;
//     appData.pPatternList = default_Patterns;

//     // See how big the file would be (if it is there at all)
//     fileSize = GetFileSize(patternFilename);
//     if (fileSize > 0 && fileSize < MAX_APP_FILELIST)
//     {
//         char *pPatternList;
//         int readSize;
//         fileSize += 2; // Account for double null we will put at the end
//         pPatternList = malloc(fileSize);
//         readSize = ReadFile(patternFilename, pPatternList, fileSize);
//         if (readSize > 0)
//         {
//             appData.pPatternList = pPatternList;
//             appData.nPatternListSize = fileSize;
//             // Now we want to go through and change all carriage return/linefeeds to blank.
//             // Unfortunately it also means that we may have to move the patterns up
//             // in the allocated memory (when there is more than one cr/lf)
//             char *src = pPatternList;
//             char *dest = pPatternList;
//             bool wasBlank = true;
//             int remain = fileSize - 1;
//             while (remain-- > 0)
//             {
//                 char c = *src++;
//                 if (isalnum(c))
//                 {
//                     *dest++ = c;
//                     wasBlank = false;
//                 }
//                 else if (!wasBlank)
//                 {
//                     *dest++ = 0;
//                     wasBlank = true;
//                 }
//             }
//             *dest++ = 0;
//             *dest++ = 0;
//         }
//         else
//         {
//             // Something went wrong so we free the memory and keep the default
//             free(pPatternList);
//         }
//     }
//     // Remember that we are starting at the beginning of the list
//     appData.pCurrentPattern = appData.pPatternList;
// }

// void TREE_Refresh(TimerHandle_t xTimer)
// {
//     appData.nDebugRefreshCount++;
//     if (appData.nRefreshCount < appData.nOnCount)
//     {
//         TREE_DisplayRGB();
//     }
//     else
//     {
//         TREE_BlankRGB();
//     }
//     appData.nRefreshCount--;
//     if (appData.nRefreshCount < 0)
//     {
//         appData.nRefreshCount = appData.nRefreshLimit;
//     }
// }

// void APP_Main(void)
// {
//     APP_Initialize();
//     Wifi_Init();
//     FileSystem_Init();
//     ESP_LOGI(TAG, "Initialized");
//     appData.nDebugRefreshCount = 0;
//     appData.nDebugTickCount = 0;
//     double dNextTreeTick = xTaskGetTickCount();
//     double dNextDebugTick = xTaskGetTickCount();

//     for (;;)
//     {
//         if (appData.bLoadPatternList)
//         {
//             Load_PatternList();
//             // Show_PatternList();
//             appData.bLoadPatternList = false;
//         }
//         if (appData.bLoadPattern)
//         {
//             appData.bLoadPattern = false;
//             TREE_LoadNamedPattern(appData.pCurrentPattern, 0);
//         }
//         if (appData.bNextStep)
//         {
//             appData.bNextStep = false;
//             if (appData.stateApp != TREE_STATE_POWERDOWN)
//             {
//                 TREE_NextTick();
//             }
//         }
//         // Wait for the timer to tell us to run another step.  Note that we will
//         // timeout after double the expected time just to keep us running.
//         xSemaphoreTake(appData.hTimerSemaphore, 2 * REFRESH_INTERVAL);
//         // See if it is time to run the
//         double dCurrentTickCount = xTaskGetTickCount();
//         if (dCurrentTickCount > dNextTreeTick)
//         {
//             TREE_Tick(NULL);
//             dNextTreeTick += ((double)configTICK_RATE_HZ / (double)TICKS_PER_SECOND);
//         }
//         TREE_Refresh(NULL);
//         if (dCurrentTickCount > dNextDebugTick)
//         {

//             ESP_LOGI(TAG, ">>Light: %d Refreshes=%d Ticks=%d", appData.nLightSensor, appData.nDebugRefreshCount, appData.nDebugTickCount);
//             appData.nDebugRefreshCount = 0;
//             appData.nDebugTickCount = 0;
//             dNextDebugTick += (double)configTICK_RATE_HZ;
//         }
//         switch (appData.stateApp)
//         {
//         case TREE_STATE_RUNNING:
//             break;
//         case TREE_STATE_START_POWERDOWN:
//             TREE_LoadNamedPattern("PowerDown", 0);
//             appData.cyclepattern = true;
//             appData.stateApp = TREE_STATE_IN_POWERDOWN;
//             break;
//         case TREE_STATE_POWERDOWN:
//             // TODO: Figure out when the powerdown pattern is complete and go into low power mode looking only for the button.
//             TREE_BlankRGB();
//             break;
//         case TREE_STATE_START_AP:
//             TREE_LoadNamedPattern("APMode", 0);
//             appData.cyclepattern = false;
//             AP_Start();
//             appData.stateApp = TREE_STATE_AP_RUNNING;
//             break;
//         case TREE_STATE_AP_RUNNING:
//             break;
//         case TREE_STATE_AP_CONNECTED:
//             break;
//         case TREE_STATE_CONNECTING_WIFI:
//             break;
//         }
//     }
// }

// /*******************************************************************************
//  End of File
//  */

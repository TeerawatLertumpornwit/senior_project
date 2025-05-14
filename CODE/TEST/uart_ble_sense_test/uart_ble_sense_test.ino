#include <Arduino_LSM9DS1.h>
#include "project_final_3_inferencing.h"  // Changed to 16 samples model

// Defines and constants
#define SAMPLES_PER_FRAME 16       // Changed to 16 samples
#define SAMPLE_INTERVAL_MS 125     // Changed to 125ms (0.125s)
#define EI_CLASSIFIER_USE_HW_FFT 0

// Normalization ranges
#define ACCEL_MAX 4.0f
#define ACCEL_MIN -4.0f
#define GYRO_MAX 2000.0f
#define GYRO_MIN -2000.0f

// Global variables
float timeSeriesBuffer[SAMPLES_PER_FRAME][6];
int currentSample = 0;
static bool debug_nn = false;
unsigned long lastSendTime = 0;

// Normalize function with constrain
float normalize(float value, float min, float max) {
    float normalized = 2.0f * (value - min) / (max - min) - 1.0f;
    return constrain(normalized, -1.0f, 1.0f);
}

void setup() {
    // Initialize Serial1 for wireless communication
    Serial1.begin(115200);
    
    // Optional: Initialize Serial for debugging during development
    // But don't make the program wait for it
    Serial.begin(115200);
    
    // Give Serial1 time to initialize
    delay(1000);
    
    // Initialize IMU
    if (!IMU.begin()) {
        // If IMU fails, attempt to send error message and flash onboard LED
        Serial1.println("Failed to initialize IMU!");
        
        // If your board has an onboard LED, you can use this for error indication
        pinMode(LED_BUILTIN, OUTPUT);
        while (1) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(300);
            digitalWrite(LED_BUILTIN, LOW);
            delay(300);
        }
    }
    
    // Send initialization message via Serial1
    Serial1.println("IMU initialized successfully");
    Serial1.println("Collecting data every 125ms, 16 samples per prediction");
}

void loop() {
    float ax, ay, az, gx, gy, gz;
    unsigned long startTime = millis();
    
    // Read IMU values
    if (IMU.readAcceleration(ax, ay, az) && IMU.readGyroscope(gx, gy, gz)) {
        
        // Normalize values
        float n_ax = normalize(ax, ACCEL_MIN, ACCEL_MAX);
        float n_ay = normalize(ay, ACCEL_MIN, ACCEL_MAX);
        float n_az = normalize(az, ACCEL_MIN, ACCEL_MAX);
        float n_gx = normalize(gx, GYRO_MIN, GYRO_MAX);
        float n_gy = normalize(gy, GYRO_MIN, GYRO_MAX);
        float n_gz = normalize(gz, GYRO_MIN, GYRO_MAX);
        
        // Store in buffer
        timeSeriesBuffer[currentSample][0] = n_ax;
        timeSeriesBuffer[currentSample][1] = n_ay;
        timeSeriesBuffer[currentSample][2] = n_az;
        timeSeriesBuffer[currentSample][3] = n_gx;
        timeSeriesBuffer[currentSample][4] = n_gy;
        timeSeriesBuffer[currentSample][5] = n_gz;
        
        currentSample++;
        
        if (currentSample >= SAMPLES_PER_FRAME) {
            
            // Process data when we have 16 samples
            float dataBuffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
            
            // Fill the data buffer for Edge Impulse
            for (int i = 0; i < SAMPLES_PER_FRAME; i++) {
                for (int j = 0; j < 6; j++) {
                    dataBuffer[i * 6 + j] = timeSeriesBuffer[i][j];
                }
            }
            
            // Create signal from buffer
            signal_t signal;
            int err = numpy::signal_from_buffer(dataBuffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
            if (err != 0) {
                Serial1.println("Error: Failed to create signal from buffer");
                currentSample = 0;
                return;
            }
            
            // Run classifier
            ei_impulse_result_t result = { 0 };
            err = run_classifier(&signal, &result, debug_nn);
            if (err != EI_IMPULSE_OK) {
                Serial1.print("Error: Failed to run classifier (");
                Serial1.print(err);
                Serial1.println(")");
                currentSample = 0;
                return;
            }
            
            // Process results
            float move_score = 0, risk_score = 0, idle_score = 0;
            String predicted_class = "unknown";
            float max_score = 0;
            
            
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                
                // เก็บค่าแต่ละคลาส
                if (strcmp(result.classification[ix].label, "idle") == 0) {
                    idle_score = result.classification[ix].value;
                    if (idle_score > max_score) {
                        max_score = idle_score;
                        predicted_class = "idle";
                    }
                } else if (strcmp(result.classification[ix].label, "move") == 0) {
                    move_score = result.classification[ix].value;
                    if (move_score > max_score) {
                        max_score = move_score;
                        predicted_class = "move";
                    }
                } else if (strcmp(result.classification[ix].label, "risk") == 0) {
                    risk_score = result.classification[ix].value;
                    if (risk_score > max_score) {
                        max_score = risk_score;
                        predicted_class = "risk";
                    }
                }
            }
            
            // Send class scores and predicted class via Serial1
            char resultBuffer[80];
            snprintf(resultBuffer, sizeof(resultBuffer), "%.5f,%.5f,%.5f,%s\n", 
                    idle_score, move_score, risk_score, predicted_class.c_str());
            Serial.print(resultBuffer);
            Serial1.write(resultBuffer, strlen(resultBuffer));
            Serial1.flush();  // Wait for data to be sent
            
            // Reset for the next batch of samples
            currentSample = 0;
            lastSendTime = millis();
        }
    }
    
    // Ensure 125ms (0.125s) interval before reading new data
    while (millis() - startTime < SAMPLE_INTERVAL_MS) {
        delay(1);
    }
}
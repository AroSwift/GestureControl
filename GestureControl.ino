// Adafruit color TFT display libraries
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

// Gesture Sensor
#include "Adafruit_APDS9960.h"
Adafruit_APDS9960 apds;

// Color LCD output pins
const int cs_pin = 10;
const int reset_pin = 9;
const int dc_pin = 8;

// Initialize the colorful LCD screen display
Adafruit_ST7735 tft = Adafruit_ST7735(cs_pin,  dc_pin, reset_pin);

// Button pin
const int button_pin = 6;

// Drawing variables
const int increment_amount = 10;
const int max_rows = 50;
uint8_t lines[max_rows][1]; // Save gesture and color for lines
uint8_t possible_colors[] = { ST7735_BLACK, ST7735_BLUE, ST7735_RED, ST7735_GREEN }; 
int current_row_index = 0;
int current_x = 64;
int current_y = 64;
boolean redraw = false;
boolean done_drawing = false;

void setup() {
  Serial.begin(9600);

  // Ensure the gesture sensor is active
  if(!apds.begin()){
    Serial.println("Failed to initialize device!");
  } else {
    Serial.println("Device initialized!");
  }

  // Enable gesture and proximity sensing
  apds.enableProximity(true);
  apds.enableGesture(true);

  // Initialize the 1.44" tft ST7735S chip with a white background
  tft.initR(INITR_144GREENTAB);
  tft.fillScreen(ST7735_WHITE);

  // Draw black pixel so the user knows were they start
  tft.drawPixel(64, 64, ST7735_BLACK);

  // Get a random seed from analog pin A0
  randomSeed(analogRead(0));

  // Set the pin mode for the button
  pinMode(button_pin, INPUT_PULLUP);
}

void loop() {
  // Read a gesture
  uint8_t gesture = apds.readGesture();
  // Then randomly pick a color
  int color_index = random(0, 3);
  uint8_t color = possible_colors[color_index];
  
  // Continually map the gesture to a draw
  map_gesture(gesture, color);
  // redraw if a redraw has been initiated
  handle_redraw();
}

// Read the lines from the user and map it to a draw command
void map_gesture(uint8_t gesture, uint8_t color) {
  // When the user is done drawing
  if(done_drawing) {
    // And a gesture has been done
    if(gesture != 0) {

      // Reset the current index and coordinates
      current_row_index = 0;
      current_x = 64;
      current_y = 64;
      
      // Clear the screen
      tft.fillScreen(ST7735_WHITE);

      // Indicate that we would like to start drawing again
      done_drawing = false;

      // Draw black pixel so the user knows were they start
      tft.drawPixel(64, 64, ST7735_BLACK);
    }
  } else { // The program should be drawing
    // When the user swipes down
    if(gesture == APDS9960_DOWN) {
      // Draw a line with the same x, but new y coordinate
      int new_y = current_y - increment_amount;
      draw_line(current_x, new_y, APDS9960_DOWN, color);
    // When the user swipes up
    } else if(gesture == APDS9960_UP) {
      // Draw a line with the same x, but new y coordinate
      int new_y = current_y + increment_amount;
      draw_line(current_x, new_y, APDS9960_UP, color);
    // When the user swipes left
    } else if(gesture == APDS9960_LEFT) {
      // Draw a line with the same y, but new x coordinate
      int new_x = current_x + increment_amount;
      draw_line(new_x, current_y, APDS9960_LEFT, color);
    // When the user swipes right
    } else if(gesture == APDS9960_RIGHT) {
      // Draw a line with the same y, but new x coordinate
      int new_x = current_x - increment_amount;
      draw_line(new_x, current_y, APDS9960_RIGHT, color);
    }
  }
}

// Redraw the lines the user has drawn
void handle_redraw() {
  // When the button is pressed
  if(digitalRead(button_pin) == LOW) {
    // Clear the screen
    tft.fillScreen(ST7735_WHITE);
    // Reset the coordinates
    current_x = 64;
    current_y = 64;
    // Indicate that we are redrawing now
    redraw = true;

    // Iterate over the coordinate values
    for(int i = 0; i < current_row_index; i++) {
      // Get the saved gesture and color
      map_gesture(lines[i][0], lines[i][1]);
      // Wait a bit so we can see the lines drawn
      delay(200);
    }

    // We are done redrawing
    redraw = false;
    done_drawing = true;
  }
}

// Draw the line where the user wants it and remeber the gesture if this is not a redraw
void draw_line(int new_x, int new_y, uint8_t gesture, uint8_t color) {
  // When the new coordinates are within the bounds of the screen
  if (new_x <= 128 && new_x >= 0 && new_y <= 128 && new_y >= 0) {
    // Draw the line
    tft.drawLine(current_x, current_y, new_x, new_y, color);

    // When we can still add to the saved gesture list and ensure we are not redrawing
    if(current_row_index < max_rows - 1 && !redraw) {
      // Save the gesture and color for later      
      lines[current_row_index][0] = gesture;
      lines[current_row_index][1] = color;
      current_row_index++;
    }
    
    // Update the coordinates
    current_x = new_x;
    current_y = new_y;
  }
}



/* Copyright 2024 Saivi Madan */
// Saivi Madan
// samadan@cs.washington.edu


// Inclusions from the Makeability Ardunio Library
#include <Shape.hpp>
#include <ParallaxJoystick.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED _display width, in pixels
#define SCREEN_HEIGHT 64 // OLED _display height, in pixels

// Define game mode buttons
#define EASY_BUTTON_PIN 7 
#define MEDIUM_BUTTON_PIN 12
#define HARD_BUTTON_PIN 8

// define vibrator and piezo pins
#define VIBRATOR_OUTPUT 6
#define OUTPUT_PIEZO_PIN 5


#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// JoyStick X and Y directions
const int JOYSTICK_UPDOWN_PIN = A1;
const int JOYSTICK_LEFTRIGHT_PIN = A2;

// RGB LED Output Pins
const int RGB_RED_PIN = 9;
const int RGB_GREEN_PIN  = 11;
const int RGB_BLUE_PIN  = 10;

const int MAX_ANALOG_VAL = 1023; 

// Text to print to OLED
const char STR_LOADSCREEN_CREATOR[] = "Saivi Madan";
const char STR_LOADSCREEN_APP_NAME_LINE1[] = "Cross That";
const char STR_LOADSCREEN_APP_NAME_LINE2[] = "Road!";
const char STR_PRESS_BUTTON_TO_PLAY_LINE1[] = "Press button to play";
const char STR_PRESS_BUTTON_TO_PLAY_LINE2[] = "Easy, Medium, Hard";
const char STR_GAME_LOST[] = "Game Over!";
const char STR_GAME_WON[] = "You Won!";


const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

ParallaxJoystick _joystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);
Ball _ball(SCREEN_WIDTH / 2, SCREEN_HEIGHT, 4);

// For tracking fps
float _fps = 0;
unsigned long _frameCount = 0;
unsigned long _fpsStartTimeStamp = 0;

// Status bar
const boolean _drawStatusBar = true; 

const int LOAD_SCREEN_SHOW_MS = 2000;

// Controls game start
boolean gameStarted = false;

// Car class
class Car : public Rectangle {
  
  public:
    Car(int x, int y, int width, int height) : Rectangle(x, y, width, height)
    {
    }
};

// Min and Max dimensions for cars
const int MIN_CAR_WIDTH = 8;
const int MAX_CAR_WIDTH = 15; 
const int MIN_CAR_X_SPACING_DISTANCE = 15; 
const int MAX_CAR_X_SPACING_DISTANCE = 55; 

int _carSpeed = 1;
const int NUM_CARS = 5;

Car _row1[NUM_CARS] = { Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0)
                            };

Car _row2[NUM_CARS] = { Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0)
                               };


Car _row3[NUM_CARS] = { Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0),
                              Car(0, 0, 0, 0)
                               };

int victoryPattern[][2] = {
  {1400, 100},  // Frequency 1400Hz, Duration 200ms
  {1600, 100},  // Frequency 1600Hz, Duration 200ms
  {1400, 100},  // Frequency 1400Hz, Duration 200ms
  {1600, 100},  // Frequency 1600Hz, Duration 200ms
  {1400, 100},  // Frequency 1400Hz, Duration 200ms
  {1600, 100},  // Frequency 1600Hz, Duration 200ms
};


enum GameState {
  NEW_GAME,
  PLAYING,
  GAME_WON,
  GAME_LOST,
};

GameState _gameState = NEW_GAME;

void setup() {
  Serial.begin(9600);

  pinMode(JOYSTICK_LEFTRIGHT_PIN, INPUT);
  pinMode(EASY_BUTTON_PIN, OUTPUT); // easy mode button
  pinMode(MEDIUM_BUTTON_PIN, OUTPUT); // medium mode button
  pinMode(HARD_BUTTON_PIN, OUTPUT); // hard mode button
  pinMode(VIBRATOR_OUTPUT, OUTPUT);
  pinMode(OUTPUT_PIEZO_PIN, OUTPUT);
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);

  initializeOledAndShowStartupScreen();

  initializeGameEntities();

  _ball.setSpeed(0, 0);
  _fpsStartTimeStamp = millis();

}

// Selects game speed and color based on button selection
// Runs game by moving ball and calling game play
// determines if game state changes
void loop() {

  if(digitalRead(EASY_BUTTON_PIN) == HIGH && _gameState == NEW_GAME) {
    _gameState == PLAYING;
    _carSpeed = 1;
    _ball.setSpeed(0, 0);
    gameStarted = true;
    analogWrite(RGB_RED_PIN, 0);
    analogWrite(RGB_GREEN_PIN, 50);
    analogWrite(RGB_BLUE_PIN, 0); 
  } else if (digitalRead(MEDIUM_BUTTON_PIN) == HIGH && _gameState == NEW_GAME) {
    _gameState == PLAYING;
    _carSpeed = 2;
    _ball.setSpeed(0, 0);
    gameStarted = true;
    analogWrite(RGB_RED_PIN, 0);
    analogWrite(RGB_GREEN_PIN, 0);
    analogWrite(RGB_BLUE_PIN, 50); 
  } else if (digitalRead(HARD_BUTTON_PIN) == HIGH && _gameState == NEW_GAME) {
    _gameState == PLAYING;
     _carSpeed = 3;
    _ball.setSpeed(2, 2);
    gameStarted = true;
    analogWrite(RGB_RED_PIN, 50);
    analogWrite(RGB_GREEN_PIN, 0);
    analogWrite(RGB_BLUE_PIN, 0); 
  }


  if (gameStarted && (_gameState == PLAYING || _gameState == NEW_GAME)) {

    _display.clearDisplay();
    
    moveBallLoop();
    gamePlayLoop();

    _display.display();
  } else if(_gameState == GAME_WON) {
    gameWon();
  } else if(_gameState == GAME_LOST) {
    gameLost();
  }
}

// Startup screen for game
void initializeOledAndShowStartupScreen(){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  
  _display.clearDisplay();

  
  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);

  int yText = 10;
  int h;
  h = printMessage(yText, STR_LOADSCREEN_CREATOR);
  _display.setTextSize(2);
  yText = yText + h + 1;
  h = printMessage(yText, STR_LOADSCREEN_APP_NAME_LINE1);
  yText = yText + h + 1;
  h = printMessage(yText, STR_LOADSCREEN_APP_NAME_LINE2);
  _display.display();
  delay(LOAD_SCREEN_SHOW_MS);

  _display.clearDisplay();
  _display.setTextSize(1);
  yText = 20;
  h = printMessage(yText, STR_PRESS_BUTTON_TO_PLAY_LINE1);
  yText = yText + h + 1;
  h = printMessage(yText, STR_PRESS_BUTTON_TO_PLAY_LINE2);
  _display.display();

}

// helper method to print to display
uint16_t printMessage(int yText, const char* MESSAGE) {
  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds(MESSAGE, 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() / 2 - w / 2, yText);
  _display.print(MESSAGE);
  return h;
}

// initializes cars on screen
void initializeGameEntities() {
  int lastCarX = _display.width() / 2;
  int lastCar2X = _display.width() / 2;
  int lastCar3X = _display.width() / 2;
  for (int i = 0; i < NUM_CARS; i++) {

    
    initializeGameEntitiesHelper(lastCarX + random(MIN_CAR_X_SPACING_DISTANCE, MAX_CAR_X_SPACING_DISTANCE),
                                random(MIN_CAR_WIDTH, MAX_CAR_WIDTH), 5, _row1, i, 45);
    initializeGameEntitiesHelper(lastCar2X + random(MIN_CAR_X_SPACING_DISTANCE, MAX_CAR_X_SPACING_DISTANCE),
                                random(MIN_CAR_WIDTH, MAX_CAR_WIDTH), 5, _row2, i, 25);
    initializeGameEntitiesHelper(lastCar3X + random(MIN_CAR_X_SPACING_DISTANCE, MAX_CAR_X_SPACING_DISTANCE),
                                random(MIN_CAR_WIDTH, MAX_CAR_WIDTH), 5, _row3, i, 5);


    lastCarX = _row1[i].getRight();
    lastCar2X = _row2[i].getRight();
    lastCar3X = _row3[i].getRight();
  }
}

// helper method for initializing game entities.
void initializeGameEntitiesHelper(int carX, int carWidth, int carHeight, Car* row, int i, int location) {

    row[i].setLocation(carX, location);
    row[i].setDimensions(carWidth, carHeight);
    row[i].setDrawFill(false);

}


// ensures that joystick can control
void moveBallLoop() {
  if(_drawStatusBar){
      drawStatusBar();
    }
    
    calcFrameRate();
    
    // Read new data
    _joystick.read();
    if(_gameState == NEW_GAME) {
      _gameState = PLAYING;
      _ball.setLocation(SCREEN_WIDTH / 2, SCREEN_HEIGHT);
    }

    // Update ball based on joystick location
    int upDownVal = _joystick.getUpDownVal();
    int leftRightVal = _joystick.getLeftRightVal();

    // Translate joystick movement into amount of pixels to move
    int yMovementPixels = map(upDownVal, 0, _joystick.getMaxAnalogValue() + 1, -1, 2);
    int xMovementPixels = map(leftRightVal, 0, _joystick.getMaxAnalogValue() + 1, -1, 2);

    // Set the new ball location based on joystick position
    //Serial.println((String)"upDownVal:" + upDownVal + " leftRightVal:" + leftRightVal + " xMovementPixels:" + xMovementPixels + " yMovementPixels:" + yMovementPixels);
    _ball.setLocation(_ball.getX() + xMovementPixels, _ball.getY() - yMovementPixels);
    _ball.forceInside(0, 0, _display.width(), _display.height());

    _ball.draw(_display);
    if(_ball.getY() == 0) {
      _gameState = GAME_WON;
      gameStarted = false;
    }
}

void gamePlayLoop() {
  // xMaxRight tracks the furthest right pixel of the furthest right pipe
  // which we will use to reposition pipes that go off the left part of screen
  int xMaxRight = 0;
  int x2MaxRight = 0;
  int x3MaxRight = 0;

  // Iterate through pipes and check for collisions and scoring
  for (int i = 0; i < NUM_CARS; i++) {

    _row1[i].setX(_row1[i].getX() - _carSpeed);
    _row2[i].setX(_row2[i].getX() - _carSpeed);
    _row3[i].setX(_row3[i].getX() - _carSpeed);
    _row1[i].draw(_display);
    _row2[i].draw(_display);
    _row3[i].draw(_display);
    

    // xMaxRight is used to track future placements of pipes once
    // they go off the left part of the screen
    if (xMaxRight < _row1[i].getRight()) {
      xMaxRight = _row1[i].getRight();
    }
    if (x2MaxRight < _row2[i].getRight()) {
      x2MaxRight = _row2[i].getRight();
    }
    if (x3MaxRight < _row3[i].getRight()) {
      x3MaxRight = _row3[i].getRight();
    }

    //Check for collisions and end of game
    if (_row1[i].overlaps(_ball) || _row2[i].overlaps(_ball) || _row3[i].overlaps(_ball) ) {
      _gameState = GAME_LOST;
      gameStarted = false;
    }
    
  }

  // Check for pipes that have gone off the screen to the left
  // and reset them to off the screen on the right
  xMaxRight = max(xMaxRight, _display.width());
  x2MaxRight = max(x2MaxRight, _display.width());
  x3MaxRight = max(x3MaxRight, _display.width());
  for (int i = 0; i < NUM_CARS; i++) {
    if (_row1[i].getRight() < 0) {
        initializeGameEntitiesHelper(xMaxRight + random(MIN_CAR_X_SPACING_DISTANCE, MAX_CAR_X_SPACING_DISTANCE),
                                random(MIN_CAR_WIDTH, MAX_CAR_WIDTH), 5, _row1, i, 45);

      xMaxRight = _row1[i].getRight();
    } 
    if (_row2[i].getRight() < 0) {

      initializeGameEntitiesHelper(x2MaxRight + random(MIN_CAR_X_SPACING_DISTANCE, MAX_CAR_X_SPACING_DISTANCE),
                                random(MIN_CAR_WIDTH, MAX_CAR_WIDTH), 5, _row2, i, 25);


      x2MaxRight = _row2[i].getRight();
    } 
    if (_row3[i].getRight() < 0) {

      initializeGameEntitiesHelper(x3MaxRight + random(MIN_CAR_X_SPACING_DISTANCE, MAX_CAR_X_SPACING_DISTANCE),
                                random(MIN_CAR_WIDTH, MAX_CAR_WIDTH), 5, _row3, i, 5);

      x3MaxRight = _row3[i].getRight();
    } 


  }

}


void gameWon() {
  _display.clearDisplay();
  
  int yText = 10;
  _display.setTextSize(2);

  int h = printMessage(yText, STR_GAME_WON);
  
  _display.setTextSize(1);
  yText = yText + h + 1;
  h = printMessage(yText, STR_PRESS_BUTTON_TO_PLAY_LINE1);
  

  yText = yText + h + 1;
  h = printMessage(yText, STR_PRESS_BUTTON_TO_PLAY_LINE2);
  _display.display();
  _gameState = NEW_GAME;
  playVictoryBeat();
  analogWrite(VIBRATOR_OUTPUT, 0);
  initializeGameEntities();

}

void playVictoryBeat() {
  int numNotes = sizeof(victoryPattern) / sizeof(victoryPattern[0]);
  for (int i = 0; i < numNotes; i++) {
    int frequency = victoryPattern[i][0];
    int duration = victoryPattern[i][1];
    tone(OUTPUT_PIEZO_PIN, frequency); // Play the tone
    analogWrite(RGB_RED_PIN, 0);
    analogWrite(RGB_GREEN_PIN, 50);
    analogWrite(RGB_BLUE_PIN, 0); 
    delay(duration); // Wait for the duration
    analogWrite(RGB_GREEN_PIN, 0);
    noTone(OUTPUT_PIEZO_PIN); // Stop the tone
    delay(50); // Short delay between notes for a smoother transition
    analogWrite(VIBRATOR_OUTPUT , 153 );
  }
}


void gameLost() {
  _display.clearDisplay();
  
  int yText = 10;
  
  _display.setTextSize(2);
  int h = printMessage(yText, STR_GAME_LOST);
  

  _display.setTextSize(1);
  yText = yText + h + 1;
  h = printMessage(yText, STR_PRESS_BUTTON_TO_PLAY_LINE1);

  yText = yText + h + 1;
  h = printMessage(yText, STR_PRESS_BUTTON_TO_PLAY_LINE2);
  _display.display();
  _gameState = NEW_GAME;

  analogWrite(VIBRATOR_OUTPUT, 153 );
  tone(OUTPUT_PIEZO_PIN, 200); // Play the tone
  delay(200);
  noTone(OUTPUT_PIEZO_PIN); // Stop the tone
  analogWrite(VIBRATOR_OUTPUT, 0);
  int duration = victoryPattern[0][1];
  for(int i = 0; i < 4; i++) {
    if(i % 2 == 0) {
      analogWrite(RGB_RED_PIN, 50);
      analogWrite(RGB_GREEN_PIN, 0);
      analogWrite(RGB_BLUE_PIN, 0); 
    } else {
      analogWrite(RGB_RED_PIN, 0);
    }
    delay(duration);
  }

  initializeGameEntities();

}



/**
 * Call this every frame to calculate frame rate
 */
void calcFrameRate() {
  unsigned long elapsedTime = millis() - _fpsStartTimeStamp;
  _frameCount++;
  if (elapsedTime > 1000) {
    _fps = _frameCount / (elapsedTime / 1000.0);
    _fpsStartTimeStamp = millis();
    _frameCount = 0;
  }
}

/**
 * Draws the status bar at top of screen with fps
 */
void drawStatusBar() {

  // Draw frame count
  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds("XX.XX fps", 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() - w, 0);
  _display.print(_fps);
  _display.print(" fps");
}
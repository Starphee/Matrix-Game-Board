#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

// --- Hardware Configuration ---
#define TCAADDR 0x70  // TCA9548A I2C Multiplexer address
const uint8_t MATRIX_ADDRESSES[3] = {0x71, 0x72, 0x74}; // Addresses on each TCA channel
const int MUX_S0 = 15, MUX_S1 = 14, MUX_S2 = 0, MUX_S3 = 8, MUX_SIG = 7; // Button MUX pins
const int PIEZO_PIN = A2; // Piezo pin

// --- Game Constants ---
const int BOARD_SIZE = 9;
const int EMPTY = 0;
const int PLAYER_X = 1;
const int PLAYER_O = 2;
const int DRAW = 3;

// --- Sound Frequencies & Durations (Hz, ms) ---
// Startup
const int STARTUP_FREQ1 = 523;  // C5
const int STARTUP_FREQ2 = 659;  // E5
const int STARTUP_FREQ3 = 784;  // G5
const int STARTUP_FREQ4 = 1046; // C6
const int STARTUP_DUR1 = 100;
const int STARTUP_DUR2 = 150;
// Game Start (per game)
const int GAME_START_FREQ1 = 1046; // C6
const int GAME_START_FREQ2 = 1318; // E6
const int GAME_START_DUR = 80;
// UI/Move Sounds
const int BUTTON_PRESS_FREQ = 2000;
const int BUTTON_PRESS_DUR = 20;
const int INVALID_MOVE_FREQ = 200;
const int INVALID_MOVE_DUR = 100;
const int PLAYER_X_MOVE_FREQ = 880; // A5
const int PLAYER_X_MOVE_DUR = 50;
const int PLAYER_O_MOVE_FREQ = 784; // G5
const int PLAYER_O_MOVE_DUR = 50;
// Win Sound (Fanfare)
const int WIN_NOTE_C5 = 523;
const int WIN_NOTE_G5 = 784;
const int WIN_NOTE_C6 = 1046;
const int WIN_NOTE_E6 = 1318;
const int WIN_NOTE_G6 = 1568;
const int WIN_DUR_SHORT = 120;
const int WIN_DUR_LONG = 250;
const int WIN_PAUSE = 30;
// Draw Sound
const int DRAW_FREQ = 440;        // A4
const int DRAW_DURATION = 200;

// --- Timings ---
const unsigned long RESET_DELAY = 3000; // 3 seconds delay before reset
const int ANIMATION_DELAY = 60;         // Delay between animation frames (ms)
const int WIN_FLASH_DELAY = 200;       // Delay for flashing winning line
const int SNAKE_PIXEL_DELAY = 10;      // Delay for startup snake (ms per pixel)
const int PULSE_DELAY = 150;          // Delay for win animation pulsing
const int WIPE_DELAY = 40;            // Delay for draw animation wipe (ms per row)


// --- Global Variables ---
Adafruit_8x8matrix matrices[BOARD_SIZE];
bool matrixInitialized[BOARD_SIZE] = {false};
bool prevButtonStates[BOARD_SIZE] = {false};
int board[BOARD_SIZE];
int currentPlayer;
bool gameOver;
int winner;
int movesMade;
int winningLine[3] = {-1,-1,-1};
unsigned long gameOverTimestamp;

// --- Function Prototypes ---
void tcaselect(uint8_t channel);
void initMatrices();
void resetGame();
void drawBoard();
void clearCell(uint8_t matrixIndex);
void drawX(uint8_t matrixIndex, bool animate);
void drawO(uint8_t matrixIndex, bool animate);
void updateCellDisplay(int index, bool animate);
void checkButtons();
bool readButton(int channel);
void makeMove(int index);
bool checkForWin();
bool checkForDraw();
void playStartupJingle();      // New
void playStartupAnimation();   // New
void playButtonPressSound();
void playInvalidMoveSound();
void playGameStartSound();
void playPlayerXMoveSound();
void playPlayerOMoveSound();
void playWinSound();           // Modified
void playDrawSound();
void animateWin();             // Modified
void highlightWinningLine(bool turnOn, int brightness); // Added brightness param
void animateDraw();            // Modified
void printBoardState();
void printPinInfo();


// =========================================================================
// SETUP FUNCTION
// =========================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n--- Tic Tac Toe Game Initializing ---");

  // --- Hardware Setup ---
  Serial.println("Setting up hardware pins...");
  pinMode(MUX_S0, OUTPUT); pinMode(MUX_S1, OUTPUT); pinMode(MUX_S2, OUTPUT); pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_SIG, INPUT_PULLUP);
  pinMode(PIEZO_PIN, OUTPUT); digitalWrite(PIEZO_PIN, LOW);
  printPinInfo();

  Serial.println("Initializing I2C and LED Matrices...");
  Wire.begin(); Wire.setClock(400000);
  initMatrices();

  // --- One-Time Startup Sequence ---
  playStartupJingle();
  playStartupAnimation();

  // --- Initialize First Game ---
  Serial.println("Setting up First Game...");
  resetGame(); // Initialize game variables, clear board, play game start sound

  Serial.println("\n--- Game Ready! Player X's Turn ---");
  printBoardState();
}

// =========================================================================
// MAIN LOOP FUNCTION
// =========================================================================
void loop() {
  if (!gameOver) {
    checkButtons(); // Poll buttons if game is active
  } else {
    // Check if reset delay has passed
    if (millis() - gameOverTimestamp >= RESET_DELAY) {
      Serial.println("\n--- Resetting Game ---");
      resetGame(); // Reset state, clear board, play game start sound
      Serial.println("--- Game Ready! Player X's Turn ---");
      printBoardState();
    }
  }
  delay(10); // Small delay to prevent excessive polling
}

// =========================================================================
// GAME LOGIC FUNCTIONS
// =========================================================================

void resetGame() {
  Serial.println("Resetting game variables and board.");
  // Reset game state variables
  for (int i = 0; i < BOARD_SIZE; i++) {
    board[i] = EMPTY;
    prevButtonStates[i] = false; // Reset button states for next game
  }
  currentPlayer = PLAYER_X; // X always starts
  gameOver = false;
  winner = EMPTY;
  movesMade = 0;
  winningLine[0] = winningLine[1] = winningLine[2] = -1; // Clear winning line info
  gameOverTimestamp = 0; // Reset timer

  drawBoard(); // Clear all matrix displays visually
  playGameStartSound(); // Sound for the start of *this* game session

  Serial.println("Board cleared. Player X starts.");
}

void makeMove(int index) {
  // Check if the move is valid
  if (board[index] == EMPTY && !gameOver) {
    // Play move sound first for responsiveness
    if (currentPlayer == PLAYER_X) {
      playPlayerXMoveSound();
    } else {
      playPlayerOMoveSound();
    }

    Serial.print("Player "); Serial.print(currentPlayer == PLAYER_X ? "X" : "O");
    Serial.print(" places mark at cell "); Serial.println(index + 1);

    // Update game state
    board[index] = currentPlayer;
    movesMade++;

    // Update display
    updateCellDisplay(index, true); // Draw X or O with animation
    printBoardState(); // Log board state

    // Check for game end conditions
    if (checkForWin()) {
        gameOver = true;
        winner = currentPlayer;
        gameOverTimestamp = millis(); // Start reset timer
        Serial.print("!!! Player "); Serial.print(currentPlayer == PLAYER_X ? "X" : "O"); Serial.println(" WINS !!!");
        animateWin(); // Play win animation (includes sound)
    } else if (checkForDraw()) {
        gameOver = true;
        winner = DRAW;
        gameOverTimestamp = millis(); // Start reset timer
        Serial.println("!!! GAME DRAW !!!");
        animateDraw(); // Play draw animation (includes sound)
    } else {
        // If game continues, switch player
        currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
        Serial.print("Turn: Player "); Serial.println(currentPlayer == PLAYER_X ? "X" : "O");
    }
  } else { // Handle invalid move conditions
      if (board[index] != EMPTY) {
          Serial.print("Cell "); Serial.print(index+1); Serial.println(" is already occupied. Invalid move.");
      } else { // Game must be over
           Serial.println("Game is over. Cannot make move.");
      }
      playInvalidMoveSound(); // Play error sound for any invalid attempt
  }
}

bool checkForWin() {
  // Define all 8 winning combinations
  const int winPatterns[8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // Rows
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // Columns
    {0, 4, 8}, {2, 4, 6}             // Diagonals
  };

  // Check each pattern
  for (int i = 0; i < 8; i++) {
    int a = winPatterns[i][0];
    int b = winPatterns[i][1];
    int c = winPatterns[i][2];
    // Check if the cells are non-empty and match the current player
    if (board[a] != EMPTY && board[a] == board[b] && board[a] == board[c]) {
      // Store the winning line indices
      winningLine[0] = a;
      winningLine[1] = b;
      winningLine[2] = c;
      Serial.print("Win detected: Cells ");
      Serial.print(a+1); Serial.print("-"); Serial.print(b+1); Serial.print("-"); Serial.println(c+1);
      return true; // Win found
    }
  }
  return false; // No win found
}

bool checkForDraw() {
  // A draw occurs if all 9 moves have been made and no winner was declared *in the current check cycle*
  if (movesMade == BOARD_SIZE && winner == EMPTY) {
      Serial.println("Draw condition met (9 moves, no winner).");
      return true;
  }
  return false;
}

// =========================================================================
// BUTTON HANDLING FUNCTIONS
// =========================================================================

void checkButtons() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    bool currentState = !readButton(i); // Read button state (inverted due to PULLUP)

    // Check for a state change (press or release)
    if (currentState != prevButtonStates[i]) {
      delay(20); // Basic debounce delay
      currentState = !readButton(i); // Read again after delay

      // Confirm state change after debounce
      if (currentState != prevButtonStates[i]) {
          prevButtonStates[i] = currentState; // Store the new stable state

          // Act only on button press (transition from HIGH to LOW)
          if (currentState) {
              playButtonPressSound(); // Play click sound for ANY confirmed press
              Serial.print("Button "); Serial.print(i+1); Serial.println(" pressed.");
              makeMove(i); // Attempt to make a move using the button index
          }
          // Optional: Could add logic here for button release if needed
      }
    }
  }
}

// Reads the state of a specific button channel via the MUX
bool readButton(int channel) {
  // Set MUX address lines based on the channel number
  digitalWrite(MUX_S0, channel & 0x01);        // LSB
  digitalWrite(MUX_S1, (channel >> 1) & 0x01);
  digitalWrite(MUX_S2, (channel >> 2) & 0x01);
  digitalWrite(MUX_S3, (channel >> 3) & 0x01); // MSB
  delayMicroseconds(5); // Allow MUX lines to settle
  // Read the signal pin (HIGH if not pressed, LOW if pressed due to PULLUP)
  return digitalRead(MUX_SIG);
}


// =========================================================================
// DISPLAY FUNCTIONS (LED Matrices)
// =========================================================================

// Selects the correct I2C bus on the TCA9548A
void tcaselect(uint8_t channel) {
  if (channel > 7) return; // Invalid channel
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << channel); // Send byte to select channel
  byte error = Wire.endTransmission();
  if (error != 0) {
    Serial.print("TCA select channel "); Serial.print(channel); Serial.print(" failed! Error code: "); Serial.println(error);
  }
  delayMicroseconds(100); // Short delay after switching channels might help stability
}

// Initialize all matrices
void initMatrices() {
  int initializedCount = 0;
  Serial.println("Initializing matrices...");
  for (uint8_t i = 0; i < BOARD_SIZE; i++) {
    uint8_t tcaChannel = i / 3; // Determine TCA channel (0, 1, or 2)
    uint8_t matrixAddr = MATRIX_ADDRESSES[i % 3]; // Determine address on that channel

    tcaselect(tcaChannel); // Select the correct I2C bus

    Serial.print("  Matrix "); Serial.print(i + 1);
    Serial.print(" (TCA Chan:"); Serial.print(tcaChannel);
    Serial.print(", Addr:0x"); Serial.print(matrixAddr, HEX); Serial.print(")... ");

    // Attempt to initialize the matrix
    if (matrices[i].begin(matrixAddr)) {
      matrices[i].setBrightness(7); // Set default brightness (0-15)
      matrices[i].setTextWrap(false); // Important for text rendering if used
      matrices[i].clear(); // Clear display buffer
      matrices[i].writeDisplay(); // Write buffer to matrix
      matrixInitialized[i] = true; // Mark as initialized
      Serial.println("OK");
      initializedCount++;
    } else {
      Serial.println("FAILED"); // Initialization failed
      matrixInitialized[i] = false;
    }
    delay(10); // Small delay between initializations
  }
  Serial.print("Matrix initialization complete. "); Serial.print(initializedCount); Serial.println("/9 matrices found.");
  if(initializedCount < BOARD_SIZE){
     Serial.println("!!! WARNING: Not all matrices initialized. Game may not display correctly. Check wiring and addresses.");
  }
}

// Draws the entire board based on the 'board' array (no animation)
void drawBoard() {
  // Serial.println("Drawing full board state to matrices."); // Less verbose
  for (int i = 0; i < BOARD_SIZE; i++) {
    updateCellDisplay(i, false); // Draw each cell without animation
  }
}

// Clears a single matrix display
void clearCell(uint8_t matrixIndex) {
   if (!matrixInitialized[matrixIndex]) return; // Skip if not initialized
   tcaselect(matrixIndex / 3); // Select correct I2C channel
   matrices[matrixIndex].clear();
   matrices[matrixIndex].writeDisplay();
}

// Draws an 'X' on the specified matrix with expanding animation
void drawX(uint8_t matrixIndex, bool animate) {
    if (!matrixInitialized[matrixIndex]) return; // Skip if not initialized
    tcaselect(matrixIndex / 3);
    Adafruit_8x8matrix &m = matrices[matrixIndex]; // Use a reference for cleaner code
    m.clear(); // Clear previous content

    if (animate) {
        // Expanding X from center, step-by-step
        // Step 1: Center pixels
        m.drawPixel(3, 3, LED_ON); m.drawPixel(4, 4, LED_ON);
        m.drawPixel(3, 4, LED_ON); m.drawPixel(4, 3, LED_ON);
        m.writeDisplay();
        delay(ANIMATION_DELAY);

        // Step 2: Expand one step
        m.drawPixel(2, 2, LED_ON); m.drawPixel(5, 5, LED_ON);
        m.drawPixel(2, 5, LED_ON); m.drawPixel(5, 2, LED_ON);
        m.writeDisplay();
        delay(ANIMATION_DELAY);

        // Step 3: Expand second step
        m.drawPixel(1, 1, LED_ON); m.drawPixel(6, 6, LED_ON);
        m.drawPixel(1, 6, LED_ON); m.drawPixel(6, 1, LED_ON);
        m.writeDisplay();
        delay(ANIMATION_DELAY);

        // Step 4: Corners (final)
        m.drawPixel(0, 0, LED_ON); m.drawPixel(7, 7, LED_ON);
        m.drawPixel(0, 7, LED_ON); m.drawPixel(7, 0, LED_ON);
        m.writeDisplay();
        // No delay after final step, move sound plays next

    } else {
        // Draw immediately without animation
        m.drawLine(0, 0, 7, 7, LED_ON);
        m.drawLine(0, 7, 7, 0, LED_ON);
        m.writeDisplay();
    }
}


// Draws an 'O' on the specified matrix with expanding animation
void drawO(uint8_t matrixIndex, bool animate) {
  if (!matrixInitialized[matrixIndex]) return; // Skip if not initialized
  tcaselect(matrixIndex / 3);
  Adafruit_8x8matrix &m = matrices[matrixIndex];
  m.clear(); // Clear previous content

  if (animate) {
    // Simple expanding circle animation
    m.drawCircle(3, 3, 1, LED_ON); // Small inner circle (center is between 3,3 and 4,4)
    m.writeDisplay();
    delay(ANIMATION_DELAY);
    m.drawCircle(3, 3, 2, LED_ON); // Middle circle
    m.writeDisplay();
    delay(ANIMATION_DELAY);
    m.drawCircle(3, 3, 3, LED_ON); // Outer circle (radius 3 fits well on 8x8)
    m.writeDisplay();
    // No delay after final step
  } else {
     // Draw full circle immediately
     m.drawCircle(3, 3, 3, LED_ON);
     m.writeDisplay();
  }
}

// Updates a single cell display based on the board state
void updateCellDisplay(int index, bool animate) {
  if (index < 0 || index >= BOARD_SIZE || !matrixInitialized[index]) return; // Validate index and initialization

  // Choose drawing function based on board state
  switch (board[index]) {
    case PLAYER_X:
      drawX(index, animate);
      break;
    case PLAYER_O:
      drawO(index, animate);
      break;
    case EMPTY:
    default:
      clearCell(index); // Clear the cell if empty
      break;
  }
}

// Animation for a win - includes sound now
void animateWin() {
    Serial.println("Playing Win Animation & Sound...");
    playWinSound(); // Play the fanfare first

    // Flash the winning line 5 times
    Serial.println("Flashing winning line...");
    for (int i = 0; i < 5; i++) {
        highlightWinningLine(true, 15);  // Highlight bright (max brightness)
        delay(WIN_FLASH_DELAY);
        highlightWinningLine(false, 0); // Turn off (brightness 0, effectively off)
        delay(WIN_FLASH_DELAY);
    }

    // Pulse the winning line 3 times
    Serial.println("Pulsing winning line...");
    for (int pulse = 0; pulse < 3; pulse++) {
         highlightWinningLine(true, 5);  // Set to Dim brightness
         delay(PULSE_DELAY);
         highlightWinningLine(true, 15); // Set to Bright brightness
         delay(PULSE_DELAY);
    }
    // Leave the winning line highlighted brightly
    highlightWinningLine(true, 15);
    Serial.println("Win animation complete.");
}

// Turns the LEDs of the winning line on or off, with specific brightness
void highlightWinningLine(bool turnOn, int brightness) {
    if (winningLine[0] == -1) return; // No winning line stored

    for(int i=0; i<3; i++){
        int cellIndex = winningLine[i];
        // Check if the cell index is valid and the matrix is initialized
        if(cellIndex >= 0 && cellIndex < BOARD_SIZE && matrixInitialized[cellIndex]){
            tcaselect(cellIndex / 3); // Select correct I2C channel
            matrices[cellIndex].setBrightness(brightness); // Set desired brightness

            if(turnOn) {
                 // Re-draw the X or O (without animation) to ensure it's visible at the new brightness
                 if(board[cellIndex] == PLAYER_X) drawX(cellIndex, false);
                 else if(board[cellIndex] == PLAYER_O) drawO(cellIndex, false);
                 // drawX/O calls writeDisplay implicitly
            } else {
                // Turn off for flashing effect
                matrices[cellIndex].clear();
                matrices[cellIndex].writeDisplay();
            }
        }
    }
    // Note: Brightness setting persists on the matrix until changed again
}

// Animation for a draw - includes sound now
void animateDraw() {
    Serial.println("Playing Draw Animation & Sound...");
    playDrawSound(); // Play draw sound first

    // Simple animation: Fill all screens briefly
    Serial.println("Filling screens for draw...");
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (matrixInitialized[i]) {
            tcaselect(i / 3);
            matrices[i].fillScreen(LED_ON);
            matrices[i].writeDisplay();
        }
    }
    delay(300); // Show filled screens for a moment

    // Draw a checkerboard pattern as a draw indicator
    Serial.println("Displaying checkerboard pattern...");
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (matrixInitialized[i]) {
            tcaselect(i / 3);
            matrices[i].clear();
            for(int y=0; y<8; y++) {
                for(int x=0; x<8; x++) {
                    // Alternate pattern based on matrix index for visual interest
                    if ((x+y) % 2 == (i % 2)) {
                       matrices[i].drawPixel(x, y, LED_ON);
                    }
                }
            }
            matrices[i].writeDisplay();
        }
    }
    delay(1000); // Show checkerboard pattern for a second

    // Wipe clear effect (downwards)
    Serial.println("Wiping draw animation...");
    for (int y = 0; y < 8; y++) { // Iterate through rows top to bottom
        for (int i = 0; i < BOARD_SIZE; i++) { // Modify all matrices for this row
             if (matrixInitialized[i]) {
                tcaselect(i / 3);
                // Draw a horizontal line of OFF pixels to clear the row
                matrices[i].drawFastHLine(0, y, 8, LED_OFF);
             }
        }
         // Update displays after modifying the row on all matrices simultaneously
         for (int i = 0; i < BOARD_SIZE; i++) {
             if (matrixInitialized[i]) {
                tcaselect(i / 3);
                matrices[i].writeDisplay();
             }
         }
        delay(WIPE_DELAY); // Delay between wiping rows
    }
    // All screens should be clear now, ready for reset delay
    Serial.println("Draw animation complete.");
}


// =========================================================================
// SOUND FUNCTIONS
// =========================================================================

void playStartupJingle() {
  Serial.println("Playing Startup Jingle...");
  tone(PIEZO_PIN, STARTUP_FREQ1, STARTUP_DUR1); delay(STARTUP_DUR1 + 20);
  tone(PIEZO_PIN, STARTUP_FREQ2, STARTUP_DUR1); delay(STARTUP_DUR1 + 20);
  tone(PIEZO_PIN, STARTUP_FREQ3, STARTUP_DUR1); delay(STARTUP_DUR1 + 20);
  tone(PIEZO_PIN, STARTUP_FREQ4, STARTUP_DUR2); delay(STARTUP_DUR2); // Wait for last note
  Serial.println("Startup Jingle Finished.");
}

// Animation that runs a pixel across all matrices sequentially
void playStartupAnimation() {
    Serial.println("Playing Startup Animation (Pixel Snake)...");
    int lastX = -1, lastY = -1; // Track previous pixel to erase on the *same* matrix

    for (int i = 0; i < BOARD_SIZE; i++) { // Iterate through matrices 0 to 8
        if (!matrixInitialized[i]) continue; // Skip uninitialized matrices

        tcaselect(i / 3); // Select the correct I2C channel
        Adafruit_8x8matrix &m = matrices[i]; // Get reference to current matrix
        // m.clear(); // Ensure matrix is clear before starting (optional, wipe at end is better)

        // Snake pattern: simple row-by-row sweep on this matrix
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                // Erase previous pixel *on this matrix*
                if (lastX != -1) { // Check if there *was* a previous pixel on this matrix
                    m.drawPixel(lastX, lastY, LED_OFF);
                }
                // Draw current pixel
                m.drawPixel(x, y, LED_ON);
                m.writeDisplay(); // Update the display IMMEDIATELY

                // Store current pixel position for erasing next time
                lastX = x;
                lastY = y;

                delay(SNAKE_PIXEL_DELAY); // Control speed of the snake
            }
        }
         // Clear the last pixel of this matrix after finishing its sweep
        if (lastX != -1) {
             m.drawPixel(lastX, lastY, LED_OFF);
             m.writeDisplay();
        }
        lastX = -1; lastY = -1; // Reset tracker for the next matrix
        // delay(50); // Optional: Small pause between matrices
    }
     Serial.println("Startup Animation Finished.");
     // Ensure all matrices are visually cleared *after* animation completes
     // This handles cases where some matrices might not have been visited by the snake if init failed
     for(int i=0; i<BOARD_SIZE; i++) clearCell(i);
}


void playButtonPressSound() {
  // Short, high-pitched click
  tone(PIEZO_PIN, BUTTON_PRESS_FREQ, BUTTON_PRESS_DUR);
  // No delay needed, it's very short
}

void playInvalidMoveSound() {
  // Low-pitched, short buzz/thud
  Serial.println("Playing Invalid Move Sound.");
  tone(PIEZO_PIN, INVALID_MOVE_FREQ, INVALID_MOVE_DUR);
  // No delay needed after tone() with duration
}

void playGameStartSound() {
  // Sound played at the beginning of each new game (after reset)
  Serial.println("Playing Game Start Sound.");
  // Simple two-tone "ready?" sound
  tone(PIEZO_PIN, GAME_START_FREQ1, GAME_START_DUR);
  delay(GAME_START_DUR + 10); // Short pause between notes
  tone(PIEZO_PIN, GAME_START_FREQ2, GAME_START_DUR * 1.5); // Slightly longer second tone
}

void playPlayerXMoveSound() {
  // Specific sound for Player X placing a mark
  // Serial.println("Playing Player X Move Sound."); // Optional: Debug log
  tone(PIEZO_PIN, PLAYER_X_MOVE_FREQ, PLAYER_X_MOVE_DUR);
}

void playPlayerOMoveSound() {
  // Specific sound for Player O placing a mark
  // Serial.println("Playing Player O Move Sound."); // Optional: Debug log
  tone(PIEZO_PIN, PLAYER_O_MOVE_FREQ, PLAYER_O_MOVE_DUR);
}

// Modified Win Sound - Fanfare
void playWinSound() {
  Serial.println("Playing Win Sound (Fanfare).");
  // Play a sequence of notes for a more melodic win sound
  tone(PIEZO_PIN, WIN_NOTE_C6, WIN_DUR_SHORT); delay(WIN_DUR_SHORT + WIN_PAUSE);
  tone(PIEZO_PIN, WIN_NOTE_C6, WIN_DUR_SHORT); delay(WIN_DUR_SHORT + WIN_PAUSE); // Repeat C6
  tone(PIEZO_PIN, WIN_NOTE_G6, WIN_DUR_LONG); delay(WIN_DUR_LONG + WIN_PAUSE); // Long G6

  tone(PIEZO_PIN, WIN_NOTE_E6, WIN_DUR_SHORT); delay(WIN_DUR_SHORT + WIN_PAUSE);
  tone(PIEZO_PIN, WIN_NOTE_C6, WIN_DUR_SHORT); delay(WIN_DUR_SHORT + WIN_PAUSE);
  tone(PIEZO_PIN, WIN_NOTE_G5, WIN_DUR_LONG); delay(WIN_DUR_LONG + WIN_PAUSE); // Long G5

  tone(PIEZO_PIN, WIN_NOTE_C5, WIN_DUR_LONG * 1.5); delay(WIN_DUR_LONG * 1.5); // Final very long low C5
}

void playDrawSound() {
  // Sound for a draw game
  Serial.println("Playing Draw Sound.");
  // Descending tone sequence
  tone(PIEZO_PIN, DRAW_FREQ + 100, DRAW_DURATION); // Higher pitch
  delay(DRAW_DURATION + 20);
  tone(PIEZO_PIN, DRAW_FREQ, DRAW_DURATION * 1.5); // Lower pitch, longer duration
}

// =========================================================================
// DEBUGGING FUNCTIONS
// =========================================================================

// Prints the current board state to the Serial Monitor
void printBoardState() {
  Serial.println("Current Board State:");
  Serial.println("-------------");
  for (int row = 0; row < 3; row++) {
    Serial.print("| ");
    for (int col = 0; col < 3; col++) {
      int index = row * 3 + col;
      char symbol = ' ';
      if (board[index] == PLAYER_X) symbol = 'X';
      else if (board[index] == PLAYER_O) symbol = 'O';
      else symbol = '-'; // Use '-' for empty cells for clarity
      Serial.print(symbol); Serial.print(" | ");
    }
    Serial.println(); // Newline at the end of the row
    Serial.println("-------------"); // Separator line
  }
   Serial.print("Moves Made: "); Serial.println(movesMade); // Show move count
}

// Prints the pin assignments to the Serial Monitor
void printPinInfo() {
    Serial.println("--- Pin Configuration ---");
    Serial.print("I2C MUX (TCA9548A) Address: 0x"); Serial.println(TCAADDR, HEX);
    Serial.println("LED Matrix Addresses per channel: 0x71, 0x72, 0x74");
    Serial.print("Button MUX S0: GPIO "); Serial.println(MUX_S0);
    Serial.print("Button MUX S1: GPIO "); Serial.println(MUX_S1);
    Serial.print("Button MUX S2: GPIO "); Serial.println(MUX_S2);
    Serial.print("Button MUX S3: GPIO "); Serial.println(MUX_S3);
    Serial.print("Button MUX SIG (Input): GPIO "); Serial.println(MUX_SIG);
    Serial.print("Piezo Speaker Pin (Output): "); Serial.println(PIEZO_PIN);
    Serial.println("-------------------------");
}
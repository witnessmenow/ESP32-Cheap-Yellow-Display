// ============================================================================
// Monte-Carlo Pi Visualizer with Persistent State
// Designed for the ESP32 Cheap Yellow Display (CYD)
// ============================================================================
//
// The program repeatedly throws random "darts" at a square and determines
// whether they land inside a quarter circle. The ratio:
//
//    hits / attempts
//
// approaches π/4. Therefore:
//
//    π ≈ 4 * hits / attempts
//
// The simulation continues indefinitely and stores progress in flash so that
// it resumes after a reboot.
// ============================================================================


// ----------------------------------------------------------------------------
// Libraries
// ----------------------------------------------------------------------------

// TFT display driver library
#include <TFT_eSPI.h>

// SPI library required by the display
#include <SPI.h>

// Preferences library provides access to ESP32 NVS flash storage
#include <Preferences.h>


// ----------------------------------------------------------------------------
// Global objects
// ----------------------------------------------------------------------------

// Display driver object
TFT_eSPI tft = TFT_eSPI();

// Preferences object used for nonvolatile storage
Preferences prefs;


// ----------------------------------------------------------------------------
// Display layout configuration
// ----------------------------------------------------------------------------

// Size of the dartboard square (pixels)
#define BOARD_SIZE 200

// Position of the dartboard on the screen
#define BOARD_X 20
#define BOARD_Y 10

// After this many darts are drawn the board is cleared visually
// (the counters are NOT reset)
#define MAX_DARTS_ON_SCREEN 250000


// ----------------------------------------------------------------------------
// Simulation counters
// ----------------------------------------------------------------------------

// Total darts thrown since the program first started
uint64_t attempts = 0;

// Total darts that landed inside the circle
uint64_t hits = 0;

// Number of darts currently drawn on the screen
uint32_t dartsOnScreen = 0;


// ----------------------------------------------------------------------------
// Color definitions (RGB565)
//
// If your CYD panel displays incorrect colors you can adjust these values.
// ----------------------------------------------------------------------------

#define HIT_COLOR  0x07E0   // green
#define MISS_COLOR 0xF800   // red


// ----------------------------------------------------------------------------
// Flash save interval
//
// Saving too frequently would wear out flash memory, so we save every
// 30 seconds.
// ----------------------------------------------------------------------------

#define SAVE_INTERVAL 30000


// ============================================================================
// Utility Functions
// ============================================================================


// ----------------------------------------------------------------------------
// formatNumber()
// Converts a large integer to a string with commas for readability.
//
// Example:
//     1234567 -> "1,234,567"
// ----------------------------------------------------------------------------
String formatNumber(uint64_t num)
{
  String s = String(num);
  int len = s.length();

  // Insert commas every 3 digits from the right
  for(int i = len - 3; i > 0; i -= 3)
    s = s.substring(0, i) + "," + s.substring(i);

  return s;
}


// ----------------------------------------------------------------------------
// saveProgress()
// Writes the current simulation counters to flash storage.
// ----------------------------------------------------------------------------
void saveProgress()
{
  prefs.putULong64("attempts", attempts);
  prefs.putULong64("hits", hits);
}


// ----------------------------------------------------------------------------
// loadProgress()
// Restores counters from flash storage when the program starts.
// ----------------------------------------------------------------------------
void loadProgress()
{
  attempts = prefs.getULong64("attempts", 0);
  hits     = prefs.getULong64("hits", 0);
}


// ============================================================================
// Display Drawing Functions
// ============================================================================


// ----------------------------------------------------------------------------
// drawBoard()
// Draws the dartboard square and the quarter circle outline.
// ----------------------------------------------------------------------------
void drawBoard()
{
  // Clear the board area
  tft.fillRect(BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE, TFT_BLACK);

  // Draw square boundary
  tft.drawRect(BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE, TFT_WHITE);

  // Draw quarter circle boundary
  tft.drawCircle(BOARD_X, BOARD_Y + BOARD_SIZE, BOARD_SIZE, TFT_DARKGREY);
}


// ----------------------------------------------------------------------------
// drawLabels()
// Draws static text labels on the screen.
// ----------------------------------------------------------------------------
void drawLabels()
{
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(20,230);
  tft.print("N:");

  tft.setCursor(20,260);
  tft.print("Pi:");
}


// ----------------------------------------------------------------------------
// drawStats()
// Displays the current simulation statistics.
// ----------------------------------------------------------------------------
void drawStats()
{
  if(attempts == 0) return;

  // Calculate π estimate
  double pi = 4.0 * (double)hits / (double)attempts;

  // Clear previous values
  tft.fillRect(70,230,160,20,TFT_BLACK);

  // Print number of attempts with commas
  tft.setCursor(70,230);
  tft.print(formatNumber(attempts));

  // Clear previous π value
  tft.fillRect(70,260,160,20,TFT_BLACK);

  // Print π approximation
  tft.setCursor(70,260);
  tft.print(pi,6);
}


// ============================================================================
// Monte Carlo Simulation
// ============================================================================


// ----------------------------------------------------------------------------
// throwBatch()
// Throws a batch of random darts.
//
// Using batches improves performance because drawing and calculations are
// done in groups rather than one dart per loop iteration.
// ----------------------------------------------------------------------------
void throwBatch()
{
  const int batch = 800;

  for(int i=0;i<batch;i++)
  {
    // Generate random coordinates in the range [0,1]
    float x = (esp_random() & 0xFFFF) / 65535.0f;
    float y = (esp_random() & 0xFFFF) / 65535.0f;

    // Convert to screen coordinates
    int px = BOARD_X + x * BOARD_SIZE;
    int py = BOARD_Y + BOARD_SIZE - (y * BOARD_SIZE);

    attempts++;
    dartsOnScreen++;

    // Check if dart landed inside the quarter circle
    if(x*x + y*y <= 1.0f)
    {
      hits++;

      // Draw hit pixel
      tft.drawPixel(px, py, HIT_COLOR);
    }
    else
    {
      // Draw miss pixel
      tft.drawPixel(px, py, MISS_COLOR);
    }

    // If the board is too full, clear it visually
    if(dartsOnScreen >= MAX_DARTS_ON_SCREEN)
    {
      drawBoard();
      dartsOnScreen = 0;
    }
  }
}


// ============================================================================
// Arduino Setup
// ============================================================================

void setup()
{
  // Initialize display
  tft.init();
  tft.setRotation(0);   // portrait orientation

  // Clear entire screen
  tft.fillScreen(TFT_BLACK);

  // Open NVS namespace
  prefs.begin("pi_sim", false);

  // Load previous simulation counters
  loadProgress();

  // Draw initial screen layout
  drawBoard();
  drawLabels();
  drawStats();
}


// ============================================================================
// Main Program Loop
// ============================================================================

void loop()
{
  // Perform Monte Carlo simulation
  throwBatch();

  static uint32_t lastUpdate = 0;
  static uint32_t lastSave   = 0;

  // Update displayed statistics 4 times per second
  if(millis() - lastUpdate > 250)
  {
    drawStats();
    lastUpdate = millis();
  }

  // Save progress periodically
  if(millis() - lastSave > SAVE_INTERVAL)
  {
    saveProgress();
    lastSave = millis();
  }
}
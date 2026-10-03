
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SparkFun_AS7265X.h>

// ============================================================
// NUTRISCAN AI
// ESP32 DEVKIT V1
// AS7265X + PCA9685 + 2 SERVOS
//
// SCAN:
//
// CLOCKWISE:
// 0 -> 34 -> 68 -> 102 -> 136 -> 170
//
// ANTICLOCKWISE:
// 136 -> 102 -> 68 -> 34 -> 0
//
// 170 DEGREE IS NOT SCANNED AGAIN.
//
// Vertical at every rotation:
//
// 30 -> 55 -> 90 -> 125 -> 150
//
// Then vertical returns to HOME = 30.
//
// TOTAL SCAN PASSES:
// 6 clockwise + 5 anticlockwise = 11
//
// TOTAL MEASUREMENTS:
// 11 x 5 = 55
//
// NOTE:
// The 5 return positions are intentionally measured again.
// ============================================================


// ============================================================
// I2C CONFIGURATION
// ============================================================

// PCA9685
// SDA = GPIO 21
// SCL = GPIO 22
// Address = 0x40

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);


// AS7265X
// SDA = GPIO 18
// SCL = GPIO 19

AS7265X sensor;


// ============================================================
// PCA9685 SERVO CHANNELS
// ============================================================

// Mango rotation servo
#define SERVO_ROTATE 14

// Vertical sensor arm servo
#define SERVO_VERTICAL 3


// ============================================================
// SERVO PWM
// ============================================================

#define SERVO_MIN 150
#define SERVO_MAX 600


// ============================================================
// MANGO ROTATION ANGLES
// ============================================================
//
// CLOCKWISE:
//
// 0
// 34
// 68
// 102
// 136
// 170
//
// ANTICLOCKWISE:
//
// 136
// 102
// 68
// 34
// 0
//
// ============================================================

int rotateAngles[6] =
{
  0,
  34,
  68,
  102,
  136,
  170
};

#define ROTATION_POINTS 6


// ============================================================
// VERTICAL SENSOR POSITIONS
// ============================================================

int vertAngles[5] =
{
  30,
  55,
  90,
  125,
  150
};

#define VERTICAL_POINTS 5


// ============================================================
// SENSOR READING CONFIGURATION
// ============================================================

// Take 3 measurements and average them
#define READINGS 3

// Delay between sensor measurements
#define SENSOR_DELAY 300


// ============================================================
// SERVO DELAYS
// ============================================================

#define ROTATION_DELAY 800
#define VERTICAL_DELAY 500


// ============================================================
// CORRECT AS7265X WAVELENGTH ORDER
// ============================================================
//
// IMPORTANT:
//
// The AS7265X library channel names are NOT simply
// A B C D E F G H I J K L R S T U V W
// when arranged by wavelength.
//
// Correct spectral order:
//
// A = 410 nm
// B = 435 nm
// C = 460 nm
// D = 485 nm
// E = 510 nm
// F = 535 nm
// G = 560 nm
// H = 585 nm
// R = 610 nm
// I = 645 nm
// S = 680 nm
// J = 705 nm
// T = 730 nm
// U = 760 nm
// V = 810 nm
// W = 860 nm
// K = 900 nm
// L = 940 nm
//
// ============================================================

const int wavelengths[18] =
{
  410,
  435,
  460,
  485,
  510,
  535,
  560,
  585,
  610,
  645,
  680,
  705,
  730,
  760,
  810,
  860,
  900,
  940
};


// ============================================================
// ANGLE -> PWM
// ============================================================

int angleToPulse(int angle)
{
  angle = constrain(angle, 0, 180);

  return map(
    angle,
    0,
    180,
    SERVO_MIN,
    SERVO_MAX
  );
}


// ============================================================
// MOVE MANGO ROTATION SERVO
// ============================================================

void moveRotate(int angle)
{
  angle = constrain(angle, 0, 180);

  int pulse = angleToPulse(angle);

  Serial.print("MANGO ROTATION -> ");
  Serial.print(angle);
  Serial.println(" degrees");

  pca.setPWM(
    SERVO_ROTATE,
    0,
    pulse
  );

  delay(ROTATION_DELAY);
}


// ============================================================
// MOVE VERTICAL SERVO
// ============================================================

void moveVertical(int angle)
{
  angle = constrain(angle, 0, 180);

  int pulse = angleToPulse(angle);

  Serial.print("VERTICAL -> ");
  Serial.print(angle);
  Serial.println(" degrees");

  pca.setPWM(
    SERVO_VERTICAL,
    0,
    pulse
  );

  delay(VERTICAL_DELAY);
}


// ============================================================
// READ ONE AS7265X SPECTRUM
// ============================================================
//
// Returns values in CORRECT WAVELENGTH ORDER:
//
// 410
// 435
// 460
// 485
// 510
// 535
// 560
// 585
// 610
// 645
// 680
// 705
// 730
// 760
// 810
// 860
// 900
// 940
//
// ============================================================

void readAndSend(
  int rotPos,
  int vertPos
)
{
  float vals[18] =
  {
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
  };


  // ==========================================================
  // TAKE 3 READINGS
  // ==========================================================

  for (int i = 0; i < READINGS; i++)
  {
    sensor.takeMeasurementsWithBulb();


    // --------------------------------------------------------
    // 410 nm - A
    // --------------------------------------------------------

    vals[0] += sensor.getCalibratedA();


    // --------------------------------------------------------
    // 435 nm - B
    // --------------------------------------------------------

    vals[1] += sensor.getCalibratedB();


    // --------------------------------------------------------
    // 460 nm - C
    // --------------------------------------------------------

    vals[2] += sensor.getCalibratedC();


    // --------------------------------------------------------
    // 485 nm - D
    // --------------------------------------------------------

    vals[3] += sensor.getCalibratedD();


    // --------------------------------------------------------
    // 510 nm - E
    // --------------------------------------------------------

    vals[4] += sensor.getCalibratedE();


    // --------------------------------------------------------
    // 535 nm - F
    // --------------------------------------------------------

    vals[5] += sensor.getCalibratedF();


    // --------------------------------------------------------
    // 560 nm - G
    // --------------------------------------------------------

    vals[6] += sensor.getCalibratedG();


    // --------------------------------------------------------
    // 585 nm - H
    // --------------------------------------------------------

    vals[7] += sensor.getCalibratedH();


    // --------------------------------------------------------
    // 610 nm - R
    // --------------------------------------------------------

    vals[8] += sensor.getCalibratedR();


    // --------------------------------------------------------
    // 645 nm - I
    // --------------------------------------------------------

    vals[9] += sensor.getCalibratedI();


    // --------------------------------------------------------
    // 680 nm - S
    // --------------------------------------------------------

    vals[10] += sensor.getCalibratedS();


    // --------------------------------------------------------
    // 705 nm - J
    // --------------------------------------------------------

    vals[11] += sensor.getCalibratedJ();


    // --------------------------------------------------------
    // 730 nm - T
    // --------------------------------------------------------

    vals[12] += sensor.getCalibratedT();


    // --------------------------------------------------------
    // 760 nm - U
    // --------------------------------------------------------

    vals[13] += sensor.getCalibratedU();


    // --------------------------------------------------------
    // 810 nm - V
    // --------------------------------------------------------

    vals[14] += sensor.getCalibratedV();


    // --------------------------------------------------------
    // 860 nm - W
    // --------------------------------------------------------

    vals[15] += sensor.getCalibratedW();


    // --------------------------------------------------------
    // 900 nm - K
    // --------------------------------------------------------

    vals[16] += sensor.getCalibratedK();


    // --------------------------------------------------------
    // 940 nm - L
    // --------------------------------------------------------

    vals[17] += sensor.getCalibratedL();


    delay(SENSOR_DELAY);
  }


  // ==========================================================
  // SEND DATA
  // ==========================================================
  //
  // IMPORTANT:
  //
  // Keep this format because your Python collector expects:
  //
  // R:index,V:index,value1,value2,...,value18
  //
  // The 18 values are now in correct wavelength order.
  //
  // ==========================================================

  Serial.print("R:");
  Serial.print(rotPos);

  Serial.print(",V:");
  Serial.print(vertPos);

  Serial.print(",");


  for (int j = 0; j < 18; j++)
  {
    float average =
      vals[j] / READINGS;

    Serial.print(
      average,
      4
    );

    if (j < 17)
    {
      Serial.print(",");
    }
  }

  Serial.println();
}


// ============================================================
// PRINT WAVELENGTH HEADER
// ============================================================

void printWavelengthHeader()
{
  Serial.println();
  Serial.println("SPECTRAL ORDER:");
  Serial.println(
    "410,435,460,485,510,535,560,585,"
    "610,645,680,705,730,760,810,860,900,940"
  );

  Serial.println();

  Serial.println(
    "CHANNEL ORDER:"
  );

  Serial.println(
    "A,B,C,D,E,F,G,H,R,I,S,J,T,U,V,W,K,L"
  );

  Serial.println();
}


// ============================================================
// TEST SENSOR
// ============================================================

void testRead()
{
  Serial.println();
  Serial.println("========================================");
  Serial.println("AS7265X SENSOR TEST");
  Serial.println("========================================");

  Serial.println();

  sensor.takeMeasurementsWithBulb();


  // 410 nm
  Serial.print("410nm = ");
  Serial.println(sensor.getCalibratedA(), 4);


  // 435 nm
  Serial.print("435nm = ");
  Serial.println(sensor.getCalibratedB(), 4);


  // 460 nm
  Serial.print("460nm = ");
  Serial.println(sensor.getCalibratedC(), 4);


  // 485 nm
  Serial.print("485nm = ");
  Serial.println(sensor.getCalibratedD(), 4);


  // 510 nm
  Serial.print("510nm = ");
  Serial.println(sensor.getCalibratedE(), 4);


  // 535 nm
  Serial.print("535nm = ");
  Serial.println(sensor.getCalibratedF(), 4);


  // 560 nm
  Serial.print("560nm = ");
  Serial.println(sensor.getCalibratedG(), 4);


  // 585 nm
  Serial.print("585nm = ");
  Serial.println(sensor.getCalibratedH(), 4);


  // 610 nm
  Serial.print("610nm = ");
  Serial.println(sensor.getCalibratedR(), 4);


  // 645 nm
  Serial.print("645nm = ");
  Serial.println(sensor.getCalibratedI(), 4);


  // 680 nm
  Serial.print("680nm = ");
  Serial.println(sensor.getCalibratedS(), 4);


  // 705 nm
  Serial.print("705nm = ");
  Serial.println(sensor.getCalibratedJ(), 4);


  // 730 nm
  Serial.print("730nm = ");
  Serial.println(sensor.getCalibratedT(), 4);


  // 760 nm
  Serial.print("760nm = ");
  Serial.println(sensor.getCalibratedU(), 4);


  // 810 nm
  Serial.print("810nm = ");
  Serial.println(sensor.getCalibratedV(), 4);


  // 860 nm
  Serial.print("860nm = ");
  Serial.println(sensor.getCalibratedW(), 4);


  // 900 nm
  Serial.print("900nm = ");
  Serial.println(sensor.getCalibratedK(), 4);


  // 940 nm
  Serial.print("940nm = ");
  Serial.println(sensor.getCalibratedL(), 4);


  Serial.println();
  Serial.println("TEST COMPLETE");
}


// ============================================================
// SCAN 5 VERTICAL POINTS
// ============================================================

void scanVerticalPoints(
  int rotationIndex,
  int rotationAngle
)
{
  Serial.println();
  Serial.println("----------------------------------------");

  Serial.print(
    "SCANNING MANGO AT ROTATION = "
  );

  Serial.print(rotationAngle);

  Serial.println(" degrees");

  Serial.println("----------------------------------------");


  // ==========================================================
  // VERTICAL:
  //
  // 30
  // 55
  // 90
  // 125
  // 150
  //
  // ==========================================================

  for (int v = 0; v < VERTICAL_POINTS; v++)
  {
    int verticalAngle =
      vertAngles[v];


    moveVertical(
      verticalAngle
    );


    Serial.print("CAPTURE: Rotation ");
    Serial.print(rotationAngle);

    Serial.print("°, Vertical ");
    Serial.print(verticalAngle);

    Serial.println("°");


    readAndSend(
      rotationIndex,
      v
    );
  }


  // ==========================================================
  // RETURN VERTICAL ARM TO HOME
  // ==========================================================

  Serial.println();
  Serial.println(
    "5 VERTICAL POINTS COMPLETE"
  );

  Serial.println(
    "RETURNING VERTICAL ARM TO HOME = 30°"
  );

  moveVertical(30);

  Serial.println(
    "VERTICAL ARM HOME"
  );
}


// ============================================================
// FULL MANGO SCAN
// ============================================================

void fullScan()
{
  Serial.println();
  Serial.println("================================================");
  Serial.println("             FULL MANGO SCAN START");
  Serial.println("================================================");

  Serial.println();

  Serial.println("SCAN PATTERN:");

  Serial.println(
    "CLOCKWISE   : 0 -> 34 -> 68 -> 102 -> 136 -> 170"
  );

  Serial.println(
    "ANTICLOCKWISE: 136 -> 102 -> 68 -> 34 -> 0"
  );

  Serial.println();

  Serial.println(
    "170° WILL NOT BE SCANNED AGAIN."
  );

  Serial.println(
    "ROTATION SCAN PASSES = 11"
  );

  Serial.println(
    "UNIQUE PHYSICAL ANGLES = 6"
  );

  Serial.println(
    "VERTICAL POINTS PER PASS = 5"
  );

  Serial.println(
    "TOTAL SENSOR MEASUREMENTS = 55"
  );

  printWavelengthHeader();


  // ==========================================================
  // PHASE 1
  //
  // CLOCKWISE
  //
  // 0
  // 34
  // 68
  // 102
  // 136
  // 170
  //
  // 6 x 5 = 30 measurements
  // ==========================================================

  Serial.println();
  Serial.println("================================================");
  Serial.println("PHASE 1 - CLOCKWISE");
  Serial.println("================================================");


  for (int r = 0; r < ROTATION_POINTS; r++)
  {
    int rotationAngle =
      rotateAngles[r];


    moveRotate(
      rotationAngle
    );


    scanVerticalPoints(
      r,
      rotationAngle
    );
  }


  // ==========================================================
  // PHASE 2
  //
  // ANTICLOCKWISE
  //
  // IMPORTANT:
  //
  // DO NOT START FROM 170.
  //
  // Start directly from 136.
  //
  // 136
  // 102
  // 68
  // 34
  // 0
  //
  // 5 x 5 = 25 measurements
  // ==========================================================

  Serial.println();
  Serial.println("================================================");
  Serial.println("PHASE 1 COMPLETE");
  Serial.println("MANGO IS AT 170°");
  Serial.println("170° WILL NOT BE SCANNED AGAIN");
  Serial.println("STARTING DIRECT ANTICLOCKWISE MOVEMENT");
  Serial.println("================================================");


  for (
    int r = ROTATION_POINTS - 2;
    r >= 0;
    r--
  )
  {
    int rotationAngle =
      rotateAngles[r];


    // Directly move from the previous
    // position to the next anticlockwise position.

    moveRotate(
      rotationAngle
    );


    scanVerticalPoints(
      r,
      rotationAngle
    );
  }


  // ==========================================================
  // FINAL HOME
  // ==========================================================

  Serial.println();
  Serial.println("Returning entire system to HOME...");

  moveRotate(0);

  moveVertical(30);


  // ==========================================================
  // COMPLETE
  // ==========================================================

  Serial.println();
  Serial.println("================================================");
  Serial.println("             FULL MANGO SCAN COMPLETE");
  Serial.println("================================================");

  Serial.println();

  Serial.println(
    "CLOCKWISE MEASUREMENTS   = 30"
  );

  Serial.println(
    "ANTICLOCKWISE MEASUREMENTS = 25"
  );

  Serial.println(
    "TOTAL MEASUREMENTS       = 55"
  );

  Serial.println();

  Serial.println(
    "FINAL MANGO POSITION = 0°"
  );

  Serial.println(
    "FINAL VERTICAL POSITION = 30°"
  );

  Serial.println();

  Serial.println("SCAN_END");

  Serial.println(
    "================================================"
  );
}


// ============================================================
// HOME
// ============================================================

void goHome()
{
  Serial.println();
  Serial.println("Moving system to HOME...");

  moveRotate(0);

  moveVertical(30);

  Serial.println("HOME COMPLETE");
}


// ============================================================
// STATUS
// ============================================================

void status()
{
  Serial.println();
  Serial.println("========================================");
  Serial.println("SYSTEM STATUS");
  Serial.println("========================================");

  Serial.println(
    "ESP32        : ESP32 DevKit V1"
  );

  Serial.println(
    "PCA9685      : 0x40"
  );

  Serial.println(
    "Rotation CH  : 14"
  );

  Serial.println(
    "Vertical CH  : 3"
  );

  Serial.println(
    "PCA9685 SDA  : GPIO21"
  );

  Serial.println(
    "PCA9685 SCL  : GPIO22"
  );

  Serial.println(
    "AS7265X SDA  : GPIO18"
  );

  Serial.println(
    "AS7265X SCL  : GPIO19"
  );

  Serial.println();

  Serial.println(
    "Rotation CW:"
  );

  Serial.println(
    "0 -> 34 -> 68 -> 102 -> 136 -> 170"
  );

  Serial.println();

  Serial.println(
    "Rotation CCW:"
  );

  Serial.println(
    "136 -> 102 -> 68 -> 34 -> 0"
  );

  Serial.println();

  Serial.println(
    "Vertical:"
  );

  Serial.println(
    "30 -> 55 -> 90 -> 125 -> 150"
  );

  Serial.println();

  Serial.println(
    "Spectral channels:"
  );

  Serial.println(
    "410,435,460,485,510,535,560,585,"
    "610,645,680,705,730,760,810,860,900,940"
  );

  Serial.println();
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  delay(1500);


  // ==========================================================
  // START MESSAGE
  // ==========================================================

  Serial.println();
  Serial.println("================================================");
  Serial.println("              NUTRISCAN AI");
  Serial.println("        MANGO SPECTRAL SCANNER");
  Serial.println("             ESP32 DEVKIT V1");
  Serial.println("================================================");


  // ==========================================================
  // PCA9685
  // ==========================================================

  Serial.println();
  Serial.println("Starting PCA9685...");

  Wire.begin(
    21,
    22
  );

  delay(100);


  if (!pca.begin())
  {
    Serial.println(
      "ERROR:PCA9685_NOT_FOUND"
    );

    Serial.println(
      "Check PCA9685 SDA -> GPIO21"
    );

    Serial.println(
      "Check PCA9685 SCL -> GPIO22"
    );

    while (1)
    {
      delay(1000);
    }
  }


  Serial.println(
    "PCA9685 FOUND"
  );


  // 50 Hz for servos
  pca.setPWMFreq(50);

  delay(500);


  // ==========================================================
  // AS7265X
  // ==========================================================

  Serial.println();
  Serial.println("Starting AS7265X...");


  Wire1.begin(
    18,
    19
  );

  delay(100);


  if (
    sensor.begin(Wire1) == false
  )
  {
    Serial.println(
      "ERROR:SENSOR_NOT_FOUND"
    );

    Serial.println(
      "Check AS7265X SDA -> GPIO18"
    );

    Serial.println(
      "Check AS7265X SCL -> GPIO19"
    );

    Serial.println(
      "Check AS7265X VCC -> 3.3V"
    );

    Serial.println(
      "Check AS7265X GND -> GND"
    );


    while (1)
    {
      delay(1000);
    }
  }


  Serial.println(
    "AS7265X FOUND"
  );


  // ==========================================================
  // SENSOR CONFIGURATION
  // ==========================================================

  sensor.setMeasurementMode(
    AS7265X_MEASUREMENT_MODE_6CHAN_CONTINUOUS
  );


  sensor.setGain(
    AS7265X_GAIN_16X
  );


  sensor.setIntegrationCycles(
    49
  );


  sensor.disableIndicator();


  // ==========================================================
  // PRINT SPECTRAL ORDER
  // ==========================================================

  printWavelengthHeader();


  // ==========================================================
  // MOVE TO HOME
  // ==========================================================

  Serial.println(
    "Moving servos to HOME..."
  );

  moveRotate(0);

  moveVertical(30);

  delay(500);


  // ==========================================================
  // READY
  // ==========================================================

  Serial.println();
  Serial.println("================================================");
  Serial.println("                    READY");
  Serial.println("================================================");

  Serial.println();

  Serial.println(
    "Commands:"
  );

  Serial.println(
    "SCAN"
  );

  Serial.println(
    "HOME"
  );

  Serial.println(
    "TEST"
  );

  Serial.println(
    "STATUS"
  );

  Serial.println();

  Serial.println(
    "IMPORTANT: Keep the scanner closed during dataset collection."
  );

  Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
  if (Serial.available())
  {
    String cmd =
      Serial.readStringUntil('\n');


    cmd.trim();

    cmd.toUpperCase();


    // ========================================================
    // SCAN
    // ========================================================

    if (cmd == "SCAN")
    {
      fullScan();
    }


    // ========================================================
    // HOME
    // ========================================================

    else if (cmd == "HOME")
    {
      goHome();
    }


    // ========================================================
    // TEST
    // ========================================================

    else if (cmd == "TEST")
    {
      testRead();
    }


    // ========================================================
    // STATUS
    // ========================================================

    else if (cmd == "STATUS")
    {
      status();
    }


    // ========================================================
    // UNKNOWN
    // ========================================================

    else
    {
      Serial.print(
        "UNKNOWN COMMAND: "
      );

      Serial.println(cmd);
    }
  }
}

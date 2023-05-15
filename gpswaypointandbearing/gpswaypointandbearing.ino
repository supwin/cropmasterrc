#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883L.h>
#include <SD.h>
#include <AFMotor.h>

// Pin assignments
#define MOTOR_PIN_1 1
#define MOTOR_PIN_2 2
#define DIR_PIN_1 3
#define DIR_PIN_2 4
#define SD_CS_PIN 10

// Struct to represent a waypoint
struct Waypoint {
  double latitude;
  double longitude;
};

struct CurrentPoint {
  double latitude;
  double longitude;
  double current_heading;
};

// Global variables
AF_DCMotor* motor1;
AF_DCMotor* motor2;
Waypoint waypoints[10];
int num_waypoints;
// double current_latitude;
// double current_longitude;
// double current_heading;

// Initialize compass
Adafruit_HMC5883L compass = Adafruit_HMC5883L();

// Function to convert degrees to radians
double toRadians(double degrees) {
  return degrees * 3.14159 / 180.0;
}

// Function to compute distance between two GPS coordinates using the haversine formula
double getDistance(double lat1, double lon1, double lat2, double lon2) {
  double dLat = toRadians(lat2 - lat1);
  double dLon = toRadians(lon2 - lon1);
  double a = pow(sin(dLat / 2), 2) + cos(toRadians(lat1)) * cos(toRadians(lat2)) * pow(sin(dLon / 2), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return 6371000 * c;  // Return distance in meters
}

// Function to compute bearing between two GPS coordinates
double getBearing(double lat1, double lon1, double lat2, double lon2) {
  double y = sin(toRadians(lon2 - lon1)) * cos(toRadians(lat2));
  double x = cos(toRadians(lat1)) * sin(toRadians(lat2)) - sin(toRadians(lat1)) * cos(toRadians(lat2)) * cos(toRadians(lon2 - lon1));
  double bearing = atan2(y, x);
  return bearing >= 0 ? bearing : 2 * 3.14159 + bearing;  // Return bearing in radians between 0 and 2*pi
}

// Function to turn the car in the right direction before going to the destination
void turnToDestination(double bearing) {
  // double heading_error = bearing - current_heading;
  // if (heading_error > 3.14159) {
  //   heading_error -= 2 * 3.14159;
  // } else if (heading_error < -3.14159) {
  //   heading_error += 2 * 3.14159;
  // }
  // if (heading_error > 0) {
  //   motor1->setSpeed(100);
  //   motor2->setSpeed(0);
  // } else {
  //   motor1->setSpeed(0);
  //   motor2->setSpeed(100);
  // }
  while (abs(heading_error) > 0.1) {
    compass.getEvent(&event);
    current_heading = atan2(event.magnetic.y, event.magnetic.x);
    heading_error = bearing - current_heading;
    if (heading_error > 3.14159) {
      heading_error -= 2 * 3.14159;
    } else if (heading_error < -3.14159) {
      heading_error += 2 * 3.14159;
    }
    if (heading_error > 0) {
      motor1->run(FORWARD);
      motor2->run(BACKWARD);
    } else {
      motor1->run(BACKWARD);
      motor2->run(FORWARD);
    }
  }
  motor1->setSpeed(0);
  motor2->setSpeed(0);
}

// Function to go to a given waypoint
void goToWaypoint(Waypoint waypoint) {
  double distance = getDistance(current_latitude, current_longitude, waypoint.latitude, waypoint.longitude);
  double bearing = getBearing(current_latitude, current_longitude, waypoint.latitude, waypoint.longitude);
  turnToDestination(bearing);
  
  while (distance > 5) {
    UpdateCurrentGPS();
    distance = getDistance(current.latitude, current.longitude, waypoint.latitude, waypoint.longitude);
    bearing = getBearing(current.latitude, current.longitude, waypoint.latitude, waypoint.longitude);
    turnToDestination(bearing);
    motor1->setSpeed(100);
    motor2->setSpeed(100);
  }
  motor1->setSpeed(0);
  motor2->setSpeed(0);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  compass.begin();
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  File file = SD.open("waypoints.txt");
  num_waypoints = 0;
  while (file.available() && num_waypoints < 10) {
    String line = file.readStringUntil('\n');
    int comma_pos = line.indexOf(',');
    waypoints[num_waypoints].latitude = line.substring(0, comma_pos).toDouble();
    waypoints[num_waypoints].longitude = line.substring(comma_pos + 1).toDouble();
    num_waypoints++;
  }
  file.close();
  motor1 = new AF_DCMotor(MOTOR_PIN_1);
  motor2 = new AF_DCMotor(MOTOR_PIN_2);
}

void UpdateCurrentGPS(){
    while (gpsSerial.available() > 0) {  // หลังจากโครงเสร็จต้องเพิ่มให้มี err ด้วย
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        current.latitude = gps.location.lat();
        current.longitude = gps.location.lng();
      }
    }
  }
}


void loop() {
  // compass.getEvent(&event);
  // current_heading = atan2(event.magnetic.y, event.magnetic.x);

  UpdateCurrentGPS();  

  for (int i = 0; i < num_waypoints; i++) {
    goToWaypoint(waypoints[i]);
  }
}
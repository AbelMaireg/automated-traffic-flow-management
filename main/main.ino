// CONSTANTS
const int HIGH_TRAFFIC = 16;
const int MEDIUM_TRAFFIC = 8;
const int LOW_TRAFFIC = 4;
const int NO_TRAFFIC = 1;
const int YELLOW_LIMIT = 2;

// sensors { sensor_pin, last_read_value, car_count }
int sensors[4][3] = {
  { 2, 1, 0 },
  { 3, 1, 0 },
  { 4, 1, 0 },
  { 5, 1, 0 }
};

// lights { green_pin, yellow_pin, red_pin }
int lights[4][3] = {
  { 8, 7, 6 },
  { 11, 10, 9 },
  { A0, 13, 12 },
  { A3, A2, A1 }
};

// rolling queue of lanes
int queue[4] = { 0, 1, 2, 3 };

// current traffic level of the current lane
int traffic_level = NO_TRAFFIC;

// last set timestamp
unsigned long int green_timestamp = 1;
unsigned long int yellow_timestamp = 0;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) {
    pinMode(sensors[i][0], INPUT);
    pinMode(lights[i][0], OUTPUT);
    pinMode(lights[i][1], OUTPUT);
    pinMode(lights[i][2], OUTPUT);
    digitalWrite(lights[i][2], HIGH);
  }

  digitalWrite(lights[queue[0]][0], HIGH);
  digitalWrite(lights[queue[0]][2], LOW);
}


void loop() {
  update_car_count();

  if (yellow_timestamp > green_timestamp) {
    if (millis() - yellow_timestamp >= YELLOW_LIMIT * 1000) {
      set_lane_red(queue[0]);
      queue_shift();
      set_lane_green(queue[0]);
      debug_on_last_yellow();
    }
  } else {
    if (millis() - green_timestamp >= traffic_level * 1000) {
      set_lane_yellow(queue[0]);
      debug_on_last_green();
    }
  }
}

// increments the count of cars if the sensor detects a change
// * on open space(a.k.a no car) the read value is 1
// * when object is detected the read is 0
// * for the car count to increase the there must be a change from 1 to 0,
//   between the last_read value and the read_value(a.k.a current value)
void update_car_count() {
  for (int i = 0; i < 4; i++) {
    int read_value = digitalRead(sensors[i][0]);

    if (read_value == sensors[i][1]) continue;

    if (sensors[i][1] == 1) sensors[i][2]++;

    sensors[i][1] = read_value;

    Serial.print("LANE: ");
    Serial.print(i);
    Serial.print(" -- CAR COUNT: ");
    Serial.println(sensors[i][2]);
  }
}

// moves the current lane to the end of the queue
// * [a, b, c, d] -> [b, c, d, a]
void queue_shift() {
  int _current = queue[0];
  for (int i = 0; i < 3; i++) {
    queue[i] = queue[i + 1];
  }
  queue[3] = _current;
}

// sets a given lane green light on
// * turns the previously set red light off
// * sets the green_timestamp
// * determines the traffic level of the given lane
void set_lane_green(int lane) {
  digitalWrite(lights[lane][0], HIGH);
  digitalWrite(lights[lane][2], LOW);

  green_timestamp = millis();

  if (sensors[lane][2] >= HIGH_TRAFFIC) {
    traffic_level = HIGH_TRAFFIC;
  } else if (sensors[lane][2] >= MEDIUM_TRAFFIC) {
    traffic_level = MEDIUM_TRAFFIC;
  } else if (sensors[lane][2] >= LOW_TRAFFIC) {
    traffic_level = LOW_TRAFFIC;
  } else {
    traffic_level = NO_TRAFFIC;
  }
}

// sets a given lane red light on
// * turns of the yellow light of the lane
// * sets the car count of the lane to 0
void set_lane_red(int lane) {
  digitalWrite(lights[lane][1], LOW);
  digitalWrite(lights[lane][2], HIGH);
  sensors[lane][2] = 0;
}

// set a given lane yellow light on
// * turns of the green light of the lane
// * sets the yellow_timestamp
void set_lane_yellow(int lane) {
  digitalWrite(lights[lane][0], LOW);
  digitalWrite(lights[lane][1], HIGH);
  yellow_timestamp = millis();
}

// code after this for debugging purpose only
// it won't add any functionality

void print_queue_status() {
  Serial.print("queue: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(" -- ");
    Serial.print(queue[i]);
  }
  Serial.println();
}

void print_count_status() {
  Serial.print("count: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(" -- ");
    Serial.print(sensors[i][2]);
  }
  Serial.println();
}

// debugging after yellow time limit is ended
// * print the current queue_status
// * print count_status(although the current lane's car count will be set to 0)
// * print the timestamp when the yellow light is switched off and the green light of the second lane is set.
void debug_on_last_yellow() {
  print_queue_status();
  print_count_status();

  Serial.print("GREEN_timestamp: ");
  Serial.println(green_timestamp);
}

// dubugging after green light time limit is ended
// * print the queue status
// * print count_status
// * print the timestamp when the green light is switched off and the yellow light is set
void debug_on_last_green() {
  print_queue_status();
  print_count_status();

  Serial.print("YELLOW_timestamp: ");
  Serial.println(yellow_timestamp);
}
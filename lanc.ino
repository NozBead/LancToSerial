#define LANC_IN_PIN 2
#define LANC_OUT_PIN 3

#define LANC_BEAT_TIME 9
#define LANC_BIT_TIME 104

unsigned char frame[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void handle_byte(unsigned char *byte, int read) {
  if (read) {
    *byte = 0;
  }

  unsigned char mask = 0x1;
  for (int i = 0 ; i < 8 ; i++) {
    if (read) {
      if (digitalRead(LANC_IN_PIN) == LOW) {
        (*byte) += mask;
      }
    }
    else {
        digitalWrite(LANC_OUT_PIN, *byte & mask ? LOW : HIGH);
    }
    mask <<= 1;
    delayMicroseconds(LANC_BIT_TIME-4);
  }
}

void setup() {
  pinMode(LANC_IN_PIN, INPUT);
  pinMode(LANC_OUT_PIN, OUTPUT);
  digitalWrite(LANC_OUT_PIN, HIGH);
  Serial.begin(9600);
}

void wait_low() {
  while (digitalRead(LANC_IN_PIN) == HIGH);
}

void sync_frame() {
  int wait_next_frame = 1;
  while (wait_next_frame) {
    unsigned long last = millis();
    wait_low();
    unsigned long current = millis();
    wait_next_frame = current - last < LANC_BEAT_TIME;
  }
}

void loop() {
  if (Serial.readBytes(frame, 2) != 2) {
    frame[0] = 0;
    frame[1] = 0;
  }
  
  sync_frame();
  for (int n = 0 ; n < 4 ; n++) {
    for (int i = 0 ; i < 8 ; i++) {
      delayMicroseconds(LANC_BIT_TIME);
      if (i >= 2) {
        handle_byte(frame+i, 1);
      }
      else {
        handle_byte(frame+i, 0);
        digitalWrite(LANC_OUT_PIN, HIGH);
      } 
    
      wait_low();
    }
  }
  Serial.write(frame, 8);
  Serial.flush();
}

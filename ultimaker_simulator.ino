// ultimaker_similator.ino
// Written July 2014 by Ralph Bolton (coofercat on github.com)

// This sketch attempts to simulate an Ultimaker 3D printer
// (Marlin firmware). It doesn't take over the entire Arduino like
// the Marlin firmware, so there are some differences (most obviously
// we don't have the same serial/command buffer that a real printer
// has. In our defense, this sketch runs on just about any Arduino,
// not just the Mega that's found in the real printer.
//
// This simulator adds an additional command (P1) to the repertoire
// which causes the next line of numbered/checksummed gcode to be
// interpreted as an error. That means it's possible to simulate
// serial data transfer problems.

#define BUFFER_SIZE 100

char read_buffer[100];
unsigned long gcode_last_n;
static char *strchr_pointer;
unsigned long gcode_N, Stopped_gcode_LastN = 0;
unsigned char in_error = 0;

char *cmd;

char inject_error = 0;

char out[100];

// Start up - we normally run at 250K, or maybe 115200 if
// required.
void setup() {
  Serial.begin(250000);
  //Serial.begin(115200);
  //Serial.begin(9600);
  Serial.write("echo 3D Printer Simulator starting up...\n");
  gcode_last_n = 0;
  gcode_N = 0;
}

// The main event
void loop() {
  unsigned long x;
  int i = 0;
  // While there's nothing available, we sleep for a moment.
  // This is a bit different to a real printer, but means that
  // we're not trying to read when the internal buffer isn't full
  // yet.
  while(!Serial.available()) {
    delay(1);
  }
  while (1) {
    int c = -1;
    while(c == -1) {
      c = Serial.read();
    }
    if(c == '\r') {
      continue;
    }
    if(c == '\n') {
      read_buffer[i] = '\0';
      break;
    }
    if(i < BUFFER_SIZE) {
      read_buffer[i++] = (char)c;
    }
  }
  
  cmd = read_buffer;
  gcode_N = 0;

  if(strchr(read_buffer, 'N') != NULL)
  {
    strchr_pointer = strchr(read_buffer, 'N');
    cmd = strchr(strchr_pointer, ' ');
    cmd++;
    gcode_N = (strtol(&read_buffer[strchr_pointer - read_buffer + 1], NULL, 10));
    if(gcode_N != gcode_last_n + 1 && (strstr_P(read_buffer, PSTR("M110")) == NULL) ) {
      sprintf(out, "Error:Line Number is not Last Line Number+1, Last Line: %d\n", gcode_last_n);
      Serial.write(out);
      sprintf(out, "Resend %d\n", gcode_last_n + 1);
      Serial.write(out);
      return;
    }

    if(strchr(read_buffer, '*') != NULL)
    {
       byte checksum = 0;
       byte count = 0;
       while(read_buffer[count] != '*') checksum = checksum^read_buffer[count++];
       strchr_pointer = strchr(read_buffer, '*');
       
       if(inject_error == 1) {
         checksum++;
         inject_error = 0;
       }

       if( (int)(strtod(&read_buffer[strchr_pointer - read_buffer + 1], NULL)) != checksum) {
         sprintf(out, "Error:checksum mismatch, Last Line: %d\n", gcode_last_n);
         Serial.write(out);
         sprintf(out, "Resend %d\n", gcode_last_n + 1);
         Serial.write(out);
         return;
       }
       //if no errors, continue parsing
    }
    gcode_last_n = gcode_N;
  }
      
  // to be here, we've got a line of buffer in
  if(strcmp(cmd, "M105") == 0) {
    //Serial.write("ok T: 150 /200 B: 70 /70\n");
    Serial.write("ok T:150.1/200.0 B:64.4/70.0 T0:150.1/200.0 @:0 B@:0\n");
    return;
  } else if(strncmp(cmd,"M106", 4) == 0) {
    // turn on the 'fan'
    digitalWrite(13, HIGH);
  } else if(strcmp(cmd, "M107") == 0) {
    // Turn the 'fan' off
    digitalWrite(13, LOW);
  } else if(strcmp(cmd, "M110") == 0) {
    // Reset our "N" counter
    gcode_last_n = 0;
    gcode_N = 0;
  } else if(strcmp(cmd, "P1") == 0) {
    // A bit non-standard, but this allows us to deliberately cause a
    // serial flow error. It causes the next received Gcode command
    // that has a checksum to be incorrectly received (even if it was
    // transmitted correctly). 
    inject_error = 1;
  } else if(strcmp(cmd, "RESET") == 0) {
    
    gcode_last_n = 0;
    gcode_N = 0;
    digitalWrite(13, LOW);
  } else {
    Serial.write("ok\n");
    return;
  }
  Serial.write("ok\n");
}

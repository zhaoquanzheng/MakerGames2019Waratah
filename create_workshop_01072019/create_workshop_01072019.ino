void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600); 
  //Serial is the monitor which receives sensory information from the arduino through the 'serial' USB cable
  //9600 is the board rate (bits/second)

  
}

void loop() {
  // put your main code here, to run repeatedly:

  int light_pin = analogRead(A0);
  //A0 is a pin on the Arduino board, most top left
  //We use analog read because A0 stands for Analog, whereas D0 would stand for Digital

  Serial.println(light_pin);
}

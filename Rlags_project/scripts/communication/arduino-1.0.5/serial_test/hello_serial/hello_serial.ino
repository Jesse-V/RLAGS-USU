int val = 0;
char response[20] = "Hello!:";

void setup(){
  Serial.begin(9600);
}

void loop(){
  
  if(Serial.available()){
    int dataCounter = 0;

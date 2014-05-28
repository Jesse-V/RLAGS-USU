int val = 0;   
char response[20] = "Hello!:";

void setup() {
  Serial.begin(9600);
}

void loop () {

  if (Serial.available() ) {
    int dataCounter = 0;
    while((val = Serial.read()) != ':') {
      if(val!= -1) {
        response[dataCounter] = val;
        ++dataCounter;
      }
    }
 //   processData();
    response[dataCounter] = ':';
    response[dataCounter+1] = '\0';
    Serial.print(response);
  }
}



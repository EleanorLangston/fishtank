int sal;            //salinity
int heat;           //heat
int lcl;            //lower control limit for salinity
int hlcl;           //lower control limit for heat
int ucl;            //upper control limit for salinity
int hucl;           //upper control limit for heat
int setpoint;       //set salinity as analog
int heatpoint;      //set heat as analog
double spfloat;     //set salinity as decimal
double heatfloat;   //set heat as decimal
double salconstant; //set salinity in %weight
int tlast;          //time
int t;              //time
int deadtime;       //deadtime constant
int hlast;          //time
int h;              //time
int heattime;       //deadtime constant
int stdVar;         //standard variation
int htdVar;         //standard variation for heat
double gain;  
double mass;        //mass of water in fishtank
double flowrate;    //flowrate of valves

void setup() {
  mass = 65.9;         
  spfloat = 0.0009;
  heatfloat = 25.0; 
  salconstant = spfloat*100;
  deadtime= 5000;
  heattime=5000;
  tlast = 0;
  hlast = 0;
  stdVar = 10; //already multplied by 3 and truncated
  htdVar = 6;  //already multiplied by 3 and truncated
  gain = 0.8;  //can change this
  flowrate = 0.006783;
  pinMode(1, OUTPUT); //LCD Screen 
  pinMode(3, OUTPUT); //conductivity sensor
  pinMode(4, OUTPUT); //salty pin
  pinMode(5, OUTPUT); //di pin
  pinMode(6, OUTPUT); //heat pin
 
  Serial.begin(9600); //set up LCD screen
  setpoint = 1274.1*pow(spfloat, 0.1325);  //convert spfloat to setpoint
  heatpoint = (8.175*heatfloat) + 277.926; //convert heatfloat to heatpoint
  ucl = setpoint + (stdVar);   //define upper control limit for salinity
  hucl = heatpoint + (htdVar); //define upper control limit for heat
  lcl = setpoint - (stdVar);   //define lower control limit for salinity
  hlcl = setpoint - (stdVar);  //define lower control limit for heat   
  Serial.write(12);           //display constants on LCD
  Serial.write(132);          // move cursor to row 3, position 14
  Serial.write("LCL    SP   UCL"); 
  Serial.write(148);
  Serial.print("S: ");
  Serial.print(saltest(lcl)*100);
  Serial.write(157);
  Serial.print(spfloat*100);
  Serial.write(163);
  Serial.print(saltest(ucl)*100);
  Serial.write(168);
  Serial.print("T:  ");
  Serial.print(((int)(heattest(hlcl)*100))/100.0);
  Serial.write(178);
  Serial.print(heatfloat);
  Serial.write(184);
  Serial.print(((int)(heattest(hucl)*100))/100.0);
  Serial.write(188);
  Serial.print("S=");
  Serial.write(196);
  Serial.print("T=");
  Serial.write(203);
  Serial.print("H=");
}

void loop() {
   digitalWrite(3,HIGH);        // apply 5V to the conductivity sensor
   delay(100);                  // hold the voltage at pin 3 for 0.1s
   sal = analogRead(0);         // read voltage on + side of 10kohm resistor for salinity
   //sal = 510;                 // test salinity value
   digitalWrite(3,LOW);         // turn off power to the conductivity sensor
   heat = analogRead(1);        // read voltage on + side of 10kohm resistor for heat
   Serial.write(190);           //display current salinity
   Serial.print(saltest(sal)*100);
   Serial.write(198);
   Serial.print(heattest(heat));
   Serial.write(205);           //reset heater status
   Serial.write("OFF");
   //Serial.write(155);
   //Serial.print(spfloat*100);
   t=millis()-tlast;
   h=millis()-hlast;
    
   if (sal < lcl && t> deadtime) { //not enough salt
      lclfx(saltest(sal));
   }
   if (sal > ucl && t>deadtime) {  //too much salt
      uclfx(saltest(sal));
   }
   if (heat < lcl && h>heattime) { //not enough heat
      heatfx(heattest(heat));       
   }
     
  
}

double saltest(int output) {    //convert from output to salinity
  double salinity = 3.6686E-24*pow(output,7.5472);
  return salinity;
}
double heattest(int output) {   //convert from output to temperature
  double temperature = (output - 277.926) / 8.175;
  return temperature;
}
void lclfx(double subsal) {     //DI OFF Salty on
  double targetsal = (spfloat - subsal) * gain;
  targetsal += subsal;
  double targetmass = mass * ((1-subsal) - (1-targetsal));
  double oflow = 1 - (.85 * subsal + .0015);
  targetmass = targetmass / ((oflow)*(.01-subsal));
  solenoid(targetmass, 4);
}

void uclfx(double supersal) { //DI on salty off
  double targetsal = (supersal - spfloat) * gain;
  targetsal += supersal;
  double targetmass = mass * ((1-supersal) - (1-targetsal));
  double oflow = 1 - (.85 * supersal);
  targetmass = targetmass / ((oflow)*supersal);
  solenoid(targetmass, 5);
}

void solenoid(double tmass, int pin) {     //open valves
  double doubletim = (tmass/flowrate);
  int tim = (int) doubletim;
  digitalWrite(pin, HIGH);
  delay(tim);
  digitalWrite(pin, LOW);
  tlast=millis();
}


void heatfx(double subheat) { //Heater on
  double doubletim = (34.43275*subheat); // mass of water (65.9g) * specific heat of water (4.18J/g°C) / voltage * current (8 VA)
  int tim = (int) doubletim;
  Serial.write(205);
  Serial.print("ON");
  digitalWrite(6, HIGH);
  delay(tim);
  digitalWrite(6, HIGH);
  hlast=millis();
}


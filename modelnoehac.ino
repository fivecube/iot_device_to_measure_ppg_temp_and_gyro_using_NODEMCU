#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
const uint8_t MPU6050SlaveAddress = 0x68;
// Network Parameters
const char* ssid = "King_of_hilltop";
const char* password = "Sword_breaker23";

// Select SDA and SCL pins for I2C communication
const uint8_t scl = D6;
const uint8_t sda = D8;

// sensitivity scale factor respective to full scale setting provided in datasheet
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;
int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;

// ThingSpeak information
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 580620;
char* readAPIKey = "V0CTFL65I6UDNHCF";
char* writeAPIKey = "QG258O9UAT8UT97U";
const unsigned long postingInterval = ((120L * 1000L)/25);
unsigned int dataFieldOne = 1;                            
unsigned int dataFieldTwo = 2;                       
unsigned int dataFieldThree = 3;                     
unsigned int dataFieldFour = 4;                                            
unsigned int dataFieldFive = 5;                            
unsigned int dataFieldSix = 6;                             
unsigned int dataFieldSeven = 7;
unsigned int dataFieldEight = 8;    
                          
unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0; 
WiFiClient client;                                  

void setup() {

Serial.begin(115200);
Serial.println("Start");
WiFi.begin(ssid,password);
Serial.println();
Serial.print("Connecting");
while( WiFi.status() != WL_CONNECTED ){
      delay(500);
      Serial.print(".");        
      }
ThingSpeak.begin( client );
  Wire.begin(sda, scl);
  MPU6050_Init();
}

void loop() {
    double Ax, Ay, Az, T, Gx, Gy, Gz;
    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
    float bpm2;
    // Only update if posting time is exceeded
    if (millis() - lastUpdateTime >=  postingInterval) {
        lastUpdateTime = millis();
        float bpm = analogRead(A0);
        bpm2=(((bpm/1024.0)*3300)/10-100);
          Ax = (double)AccelX/AccelScaleFactor;
          Ay = (double)AccelY/AccelScaleFactor;
          Az = (double)AccelZ/AccelScaleFactor;
          T =  (double)Temperature/340+36.53; //temperature formula
          Gx = (double)GyroX/GyroScaleFactor;
          Gy = (double)GyroY/GyroScaleFactor;
          Gz = (double)GyroZ/GyroScaleFactor;
  
          Serial.print("Ax: "); Serial.print(Ax);
          Serial.print(" Ay: "); Serial.print(Ay);
          Serial.print(" Az: "); Serial.print(Az);
          Serial.print(" T: "); Serial.print(T);
          Serial.print(" Gx: "); Serial.print(Gx);
          Serial.print(" Gy: "); Serial.print(Gy);
          Serial.print(" Gz: "); Serial.println(Gz);
        write2TSData( channelID,dataFieldOne,bpm,dataFieldTwo,T,dataFieldThree,Ax,dataFieldFour,Ay,dataFieldFive,Az,dataFieldSix,Gx,dataFieldSeven,Gy,dataFieldEight,Gz );  // Write the temperature in F, C, and time since starting.
    }
}
  float readTSData( long TSChannel,unsigned int TSField ){   
  float data =  ThingSpeak.readFloatField( TSChannel, TSField, readAPIKey );
  Serial.println( " Data read from ThingSpeak: " + String( data, 9 ) );
  return data;
}

// Use this function if you want to write a single field
int writeTSData( long TSChannel, unsigned int TSField, float data ){
  int  writeSuccess = ThingSpeak.writeField( TSChannel, TSField, data, writeAPIKey ); // Write the data to the channel
  if ( writeSuccess ){
    
    Serial.println( String(data) + " written to Thingspeak." );
    }
    
    return writeSuccess;
}

//use this function if you want multiple fields simultaneously
int write2TSData( long TSChannel, unsigned int TSField1, float field1Data, unsigned int TSField2, long field2Data, unsigned int TSField3, long field3Data,unsigned int TSField4, long field4Data, unsigned int TSField5, long field5Data, unsigned int TSField6, long field6Data, unsigned int TSField7, long field7Data, unsigned int TSField8, long field8Data ){

  ThingSpeak.setField( TSField1, field1Data );
  ThingSpeak.setField( TSField2, field2Data );
  ThingSpeak.setField( TSField3, field3Data );
  ThingSpeak.setField( TSField4, field4Data );
  ThingSpeak.setField( TSField5, field5Data );
  ThingSpeak.setField( TSField6, field6Data );
  ThingSpeak.setField( TSField7, field7Data );
  ThingSpeak.setField( TSField8, field8Data );
   
  int writeSuccess = ThingSpeak.writeFields( TSChannel, writeAPIKey );
  return writeSuccess;
}
void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}
void MPU6050_Init(){
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}



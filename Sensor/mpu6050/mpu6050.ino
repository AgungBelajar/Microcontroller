#include <Wire.h>
  
//Declaring some global variables
int gyro_x, gyro_y, gyro_z;
long acc_x, acc_y, acc_z, acc_total_vector;
int temperature;
long gyro_x_cal, gyro_y_cal, gyro_z_cal;
long loop_timer;
float angle_pitch, angle_roll;
int angle_pitch_buffer, angle_roll_buffer;
boolean set_gyro_angles;
float angle_roll_acc, angle_pitch_acc;
float angle_pitch_output, angle_roll_output;
int i = 1;
int N = 10000;
float offset_angle_pitch_acc, offset_angle_roll_acc, sum_angle_pitch_acc, sum_angle_roll_acc;
float hitung = 0;
float Ts = 0.004; //0.01;  // 0.01 ms (100 Hz), 0.004 ms (250 Hz)
  
void setup() {
  Wire.begin();                                                        //Start I2C as master
  Serial.begin(57600);                                                 //Use only for debugging
  pinMode(13, OUTPUT);                                                 //Set output 13 (LED) as output
    
  setup_mpu_6050_registers();                                          //Setup the registers of the MPU-6050 (500dfs / +/-8g) and start the gyro
  
  digitalWrite(13, HIGH);                                              //Set digital output 13 high to indicate startup
  Serial.println("  MPU-6050 IMU");
  Serial.println("     V1.0");
  delay(1500);                                                         //Delay 1.5 second to display the text
  Serial.println("Calibrating gyro");
    
  for (int cal_int = 0; cal_int < 2000 ; cal_int ++){                  //Run this code 2000 times
    if(cal_int % 125 == 0)Serial.print(".");                              //Print a dot on the LCD every 125 readings
    read_mpu_6050_data();                                              //Read the raw acc and gyro data from the MPU-6050
    gyro_x_cal += gyro_x;                                              //Add the gyro x-axis offset to the gyro_x_cal variable
    gyro_y_cal += gyro_y;                                              //Add the gyro y-axis offset to the gyro_y_cal variable
    gyro_z_cal += gyro_z;                                              //Add the gyro z-axis offset to the gyro_z_cal variable
    delay(3);                                                          //Delay 3us to simulate the 250Hz program loop
  }
  gyro_x_cal /= 2000;                                                  //Divide the gyro_x_cal variable by 2000 to get the avarage offset
  gyro_y_cal /= 2000;                                                  //Divide the gyro_y_cal variable by 2000 to get the avarage offset
  gyro_z_cal /= 2000;                                                  //Divide the gyro_z_cal variable by 2000 to get the avarage offset
  
  Serial.print("Pitch:");                                                 //Print text to screen
  Serial.print("Roll :");                                                 //Print text to screen
    
  digitalWrite(13, LOW);                                               //All done, turn the LED off
    
  loop_timer = micros();                                               //Reset the loop timer
}
  
void loop(){
  //NOTE PENTING!!: Tempatkan MPU6050 di tempat datar ketika proses kalibrasi berlangsung
  
  read_mpu_6050_data();                                                //Read the raw acc and gyro data from the MPU-6050
  
  gyro_x -= gyro_x_cal;                                                //Subtract the offset calibration value from the raw gyro_x value
  gyro_y -= gyro_y_cal;                                                //Subtract the offset calibration value from the raw gyro_y value
  gyro_z -= gyro_z_cal;                                                //Subtract the offset calibration value from the raw gyro_z value
    
  //Gyro angle calculations
  //0.0000611 = 1 / (250Hz / 65.5)
  angle_pitch += gyro_x * (1/(1/Ts)/65.5);                                   //Calculate the traveled pitch angle and add this to the angle_pitch variable
  angle_roll += gyro_y * (1/(1/Ts)/65.5);                                    //Calculate the traveled roll angle and add this to the angle_roll variable
    
  //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
  angle_pitch += angle_roll * sin(gyro_z * 1/(1/Ts)/65.5*3.142/180);               //If the IMU has yawed transfer the roll angle to the pitch angel
  angle_roll -= angle_pitch * sin(gyro_z * 1/(1/Ts)/65.5*3.142/180);               //If the IMU has yawed transfer the pitch angle to the roll angel
    
  //Accelerometer angle calculations
  acc_total_vector = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));  //Calculate the total accelerometer vector
  //57.296 = 1 / (3.142 / 180) The Arduino asin function is in radians
  angle_pitch_acc = asin((float)acc_y/acc_total_vector)* 57.296;       //Calculate the pitch angle
  angle_roll_acc = asin((float)acc_x/acc_total_vector)* -57.296;       //Calculate the roll angle
    
  while(micros() - loop_timer < Ts*1000000);                                 //Wait until the loop_timer reaches 4000us (250Hz) before starting the next loop
  loop_timer = micros();                                               //Reset the loop timer
  
  Serial.print("pitch : ");
  Serial.print(angle_pitch_acc);
  Serial.print("\t");
  Serial.print("roll : ");
  Serial.println(angle_roll_acc);
    
  sum_angle_pitch_acc = sum_angle_pitch_acc + angle_pitch_acc;
  sum_angle_roll_acc = sum_angle_roll_acc + angle_roll_acc;
  
  if(i>=N)
  {
    offset_angle_pitch_acc = sum_angle_pitch_acc/N;
    offset_angle_roll_acc = sum_angle_roll_acc/N;
      
    Serial.print("Banyak data = "); Serial.println(i);
    Serial.print("offset_angle_pitch_acc = ");
    Serial.println(offset_angle_pitch_acc);
    Serial.print("offset_angle_roll_acc = ");
    Serial.print(offset_angle_roll_acc);
    while(1){};
  }
  i++;
}
  
  
void read_mpu_6050_data(){                                             //Subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x3B);                                                    //Send the requested starting register
  Wire.endTransmission();                                              //End the transmission
  Wire.requestFrom(0x68,14);                                           //Request 14 bytes from the MPU-6050
  while(Wire.available() < 14);                                        //Wait until all the bytes are received
  acc_x = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_x variable
  acc_y = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_y variable
  acc_z = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_z variable
  temperature = Wire.read()<<8|Wire.read();                            //Add the low and high byte to the temperature variable
  gyro_x = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_x variable
  gyro_y = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_y variable
  gyro_z = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_z variable
  
}
  
void setup_mpu_6050_registers(){
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6B);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1C);                                                    //Send the requested starting register
  Wire.write(0x10);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1B);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
}

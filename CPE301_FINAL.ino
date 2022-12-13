//Ashley Yakish
#define RDA 0x80
#define TBE 0x20  

#include "DHT.h"
#include <LiquidCrystal.h>
#include "RTClib.h"
#include <Stepper.h>

#define STEPS 32

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
 
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

volatile unsigned char* PORT_c = (unsigned char*) 0x28;
volatile unsigned char* DDR_c  = (unsigned char*) 0x27; 
volatile unsigned char* PIN_c  = (unsigned char*) 0x26;

volatile unsigned char* PORT_c4 = (unsigned char*) 0x28;
volatile unsigned char* DDR_c4  = (unsigned char*) 0x27; 
volatile unsigned char* PIN_c4  = (unsigned char*) 0x26;

volatile unsigned char* PORT_c6 = (unsigned char*) 0x28;
volatile unsigned char* DDR_c6  = (unsigned char*) 0x27; 
volatile unsigned char* PIN_c6  = (unsigned char*) 0x26;

volatile unsigned char* PORT_a = (unsigned char*) 0x22;
volatile unsigned char* DDR_a  = (unsigned char*) 0x21; 
volatile unsigned char* PIN_a  = (unsigned char*) 0x20;

volatile unsigned char* PORT_h = (unsigned char*) 0x102;
volatile unsigned char* DDR_h  = (unsigned char*) 0x101; 
volatile unsigned char* PIN_h  = (unsigned char*) 0x100;

volatile unsigned char* PORT_l = (unsigned char*) 0x10B;
volatile unsigned char* DDR_l  = (unsigned char*) 0x10A; 
volatile unsigned char* PIN_l  = (unsigned char*) 0x109;

volatile unsigned char* PORT_l4 = (unsigned char*) 0x10B;
volatile unsigned char* DDR_l4  = (unsigned char*) 0x10A; 
volatile unsigned char* PIN_l4  = (unsigned char*) 0x109;

volatile unsigned char* PORT_l5 = (unsigned char*) 0x10B;
volatile unsigned char* DDR_l5  = (unsigned char*) 0x10A; 
volatile unsigned char* PIN_l5  = (unsigned char*) 0x109;

volatile unsigned char* PORT_g = (unsigned char*) 0x34;
volatile unsigned char* DDR_g  = (unsigned char*) 0x33; 
volatile unsigned char* PIN_g  = (unsigned char*) 0x32;

volatile unsigned char* PORT_p = (unsigned char*) 0x34;
volatile unsigned char* DDR_p  = (unsigned char*) 0x33; 
volatile unsigned char* PIN_p  = (unsigned char*) 0x32;

unsigned long previousMillis = 0;
const long interval = 60000;

int i = 0;
int j =0;

LiquidCrystal lcd(12,11,5,4,3,2);
Stepper stepper (STEPS, 7, 9, 8, 10);

#define DHTPIN 13
#define DHTTYPE DHT11

RTC_DS1307 rtc; 

int Pval = 0;

int potVal = 0;


DHT dht (DHTPIN,DHTTYPE);
void setup() 
{
  // setup the UART
  U0init(9600);

  //INPUT = PA2
  *DDR_a &= 0xFB; 

  //Resistor on PA2
  *PORT_a |= 0x04; 

  //OUTPUT (YELLOW)
  *DDR_c |= 0x01; // 0b 0000 0001
  //OUTPUT (GRN)
  *DDR_h |= 0x08; // 0b 0000 1000
  //OUTPUT PL3 (RED)
  *DDR_l |= 0x08;
  //OUTPUT PL4 (BLUE)
  *DDR_l4 |= 0x16;
  //OUTPUT PC4(FAN)
  *DDR_c4 |= 0x16; // 0b 0010 0000
  //OUTPUT PC6(FAN)
  *DDR_c6 |= 0x64; // 0b 0010 0000

  // setup the ADC
  adc_init();
  // set up the dht
  dht.begin();
  // set up the LCD (number of columns and rows):
  lcd.begin(16,2);


  // basic lcd set up
  lcd.print("Temp: Humidity:");
  rtc.begin();

  if (! rtc.begin()) {
  Serial.println("Couldn't find RTC");
  while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  stepper.setSpeed(200);

}
void loop() 
{

  if(*PIN_a & 0x04) //this if statement says that when it is on (green light on), it will run the system
  {
    i = 0;
    for (j; j < 1; j++){
      Serial.println("ENABLED AT:");
      rtc_measurements();
      Serial.println("\n");
    }

    //YELLOW LOW
    *PORT_c &= 0xFE;
    //GRN HIGH
    *PORT_h |= 0x08; 
    //RED LOW
    *PORT_l &= 0xF7;
    //BLUE LOW
    *PORT_l4 &= 0xEF;
    //FAN LOW
    *PORT_c6 &= 0xDF;
    *PORT_c4 &= 0xEF;

    // get the reading from the ADC
    unsigned int adc_reading = adc_read(0);
    // print it to the serial port
    //print_int(adc_reading);

    lcd_screen();  

    potVal = map(analogRead(A1),0,1024,0,500);

    if (potVal>Pval){
      stepper.step(5);
    }
    if (potVal<Pval){
    stepper.step(-5);
    }

    Pval = potVal;

    //if temperature is below/above threshold
    float t = dht.readTemperature();

    while (adc_reading > 100 && t > 22 || t < 18) {
      // FAN HIGH
      *PORT_c6 |= 0x64;
      *PORT_c4 &= 0xEF;
      //BLUE HIGH
      *PORT_l4 |= 0x16;
      //GRN LOW
      *PORT_h &= 0xF7;
      //RED LOW 
      *PORT_l &= 0xF7;
      adc_reading = adc_read(0);

      // vent position control (stepper motor)
      potVal = map(analogRead(A1),0,1024,0,500);
        if (potVal>Pval){
          stepper.step(15);
        }
        if (potVal<Pval){
          stepper.step(-15);
        }
        Pval = potVal;
      
        Serial.println("Vent position changed:");
        rtc_measurements();
        Serial.println("\n");
    }

    //if water level is too low
    while (adc_reading < 100){
      //RED HIGH
      *PORT_l |= 0x08; // 0b 1111 0111
      //GRN LOW
      *PORT_h &= 0xF7;
      //BLUE LOW
      *PORT_l4 &= 0xEF;
      //FAN LOW
      //*PORT_c6 &= 0xDF;
      //*PORT_c4 &= 0xEF;
      lcd_error();
      adc_reading = adc_read(0);
      //print_int(adc_reading);
    }

    lcd_screen(); 
  }
  else //if it is disabled, nothing runs and the yellow LED is on
  {
    j = 0;
    //YELLOW HIGH
    *PORT_c |= 0x01;
    //GRN LOW
    *PORT_h &= 0xF7; // 0b 1111 0111
    //RED LOW
    *PORT_l &= 0xF7;
    //BLUE LOW
    *PORT_l4 &= 0xEF;

    lcd_dis();

    for (i; i < 1; i++){
      Serial.println("DISABLED AT:");
      rtc_measurements();
      Serial.println("\n");
    }
  }
  delay(10);


}
void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000;  // set bit 7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11011111;  // clear bit 5 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &=  0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &=  0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &=  0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |=  0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &=  0b11011111;// clear bit 5 to 0 for right adjust result
  *my_ADMUX  &=  0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &=  0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}
unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}
void print_int(unsigned int out_num)
{
  // clear a flag (for printing 0's in the middle of numbers)
  unsigned char print_flag = 0;
  // if its greater than 1000
  if(out_num >= 1000)
  {
    // get the 1000's digit, add to '0' 
    U0putchar(out_num / 1000 + '0');
    // set the print flag
    print_flag = 1;
    // mod the out num by 1000
    out_num = out_num % 1000;
  }
  // if its greater than 100 or we've already printed the 1000's
  if(out_num >= 100 || print_flag)
  {
    // get the 100's digit, add to '0'
    U0putchar(out_num / 100 + '0');
    // set the print flag
    print_flag = 1;
    // mod the output num by 100
    out_num = out_num % 100;
  } 
  // if its greater than 10, or we've already printed the 10's
  if(out_num >= 10 || print_flag)
  {
    U0putchar(out_num / 10 + '0');
    print_flag = 1;
    out_num = out_num % 10;
  } 
  // always print the last digit (in case it's 0)
  U0putchar(out_num + '0');
  // print a newline
  U0putchar('\n');
}
void U0init(int U0baud)
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}
unsigned char U0getchar()
{
  return *myUDR0;
}
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}

unsigned char lcd_screen()
{
  //DHT and screen
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
  unsigned long currentMillis = millis();

 //refreshing every minute
  if (currentMillis - previousMillis >= interval || millis() <= 60000) {
    lcd.setCursor(0,1);
    lcd.print(t);
    lcd.setCursor(6,1);
    lcd.print(h);
    previousMillis = currentMillis;
  }

}

unsigned char rtc_measurements()
{
  DateTime now = rtc.now();
  Serial.print("Date & Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
}

unsigned char lcd_error()
{
  lcd.setCursor(0,1);
  lcd.print("ERROR");
  lcd.setCursor(6,1);
  lcd.print("ERROR");
}

unsigned char lcd_dis()
{
  lcd.setCursor(0,1);
  lcd.print("DISAB");
  lcd.setCursor(6,1);
  lcd.print("DISAB");
}
// Made for Arduino UNO
// Created by Raul Stelescu

// Dependencies
#include <LiquidCrystal.h>

// Pin layout
const int pin_amp_battery = A0; // Battery current PIN
const int pin_voltage_battery = A1; // Battery voltage PIN
const int pin_voltage_panel = A2; // Panel voltage PIN
const int pin_float = 8; // Pin for float charging
const int pin_boost = 9; // Pin for boost charging
const int pin_charging_module = 10; // Pin for charging module

// Variable declaration
double mVperAmp = 6.25;
double battery_voltage = 0; // vout
double panel_voltage = 0; // Panel voltage
double amp_to_mV = 0; // VoltageA0
double low_barrier = 44.0; // The low barrier for charging the battery
double high_barrier = 57.6; // The high barrier for charging the battery
  
int raw_battery_amp = 0;
int raw_battery_voltage = 0;
int raw_pannel_voltage = 0;
int ACSoffset = 2500;
int battery_amps = 0;

boolean fullyDischarged = false;
boolean hadFirstCycle = false;
String current_charging_mode = "";

// Initialize lcd
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2); // 16x2 lcd
  pinMode(pin_float, OUTPUT);
  pinMode(pin_boost, OUTPUT);
  pinMode(pin_charging_module, OUTPUT);
}

void loop() {
  raw_battery_amp = analogRead(pin_amp_battery);
  raw_battery_voltage = analogRead(pin_voltage_battery);
  raw_pannel_voltage = analogRead(pin_voltage_panel);

  amp_to_mV = (raw_battery_amp / 1023.0) * 5000; // transform to mv
  battery_amps = (int)floor((amp_to_mV - ACSoffset) / mVperAmp);

  battery_voltage = (raw_battery_voltage * 5.0) / 1023.0 / 0.05;
  panel_voltage = (raw_pannel_voltage * 5.0) / 1023.0 / 0.05;

  // This indicates whether the battery should charge or not
  if (panel_voltage > battery_voltage) {
    digitalWrite(pin_charging_module, HIGH);
  } else {
    digitalWrite(pin_charging_module, LOW);
  }
  
  // This goes to high_barrier in boost and then switches to float
  // Until it reaches back to low_barrier  
  if (battery_voltage < high_barrier) {
    if (fullyDischarged == false && battery_voltage <= low_barrier && hadFirstCycle == true) {
      fullyDischarged = true;
      digitalWrite(pin_boost, HIGH);
      digitalWrite(pin_float, LOW);
      current_charging_mode = "boost";
    } else if (fullyDischarged == false && battery_voltage > low_barrier && battery_voltage <= high_barrier && hadFirstCycle == true) {
      digitalWrite(pin_boost, LOW);
      digitalWrite(pin_float, HIGH);
      current_charging_mode = "float";
    } else if (fullyDischarged == false && hadFirstCycle == false && battery_voltage < high_barrier) {
      digitalWrite(pin_boost, HIGH);
      digitalWrite(pin_float, LOW);
      current_charging_mode = "boost";
    }
  } else if (battery_voltage >= high_barrier) {
    digitalWrite(pin_boost, LOW);
    digitalWrite(pin_float, HIGH);
    current_charging_mode = "float";
    fullyDischarged = false;
    hadFirstCycle = true;
  }

  // Build the output on the lcd
  int nr_of_digits_amps = 7 + String(battery_amps).length();
  int nr_of_digits_voltage = 0;
  String output_voltage = String(battery_voltage);
  output_voltage.remove(output_voltage.length() - 1);
  nr_of_digits_voltage = 9 + output_voltage.length();

  //Clear lcd
  lcd.clear();

  // Write V to lcd
  lcd.setCursor(0, 0);
  lcd.print("Tensiune:");
  lcd.setCursor(9, 0);
  lcd.print(output_voltage);
  lcd.setCursor(nr_of_digits_voltage, 0);
  lcd.print("V");

  // Write Amps to lcd
  lcd.setCursor(0, 1);
  lcd.print("Curent:");
  lcd.setCursor(7, 1);
  lcd.print(battery_amps);
  lcd.setCursor(nr_of_digits_amps, 1);
  lcd.print("A");

  String tesiunePOutput = "Tensiune panou: " + String(panel_voltage);
  String tensiuneBOutput = "Tenisune baterie: " + String(battery_voltage);

  Serial.println(tesiunePOutput);
  Serial.println(tensiuneBOutput);

  if (current_charging_mode == "boost") {
    lcd.setCursor(15, 1);
    lcd.print("B");
  } else if (current_charging_mode == "float") {
    lcd.setCursor(15, 1);
    lcd.print("F");
  } else {
    lcd.setCursor(15, 1);
    lcd.print("");
  }

  delay(100);
}

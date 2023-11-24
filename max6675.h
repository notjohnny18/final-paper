#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>

#define pinoSO 8
#define pinoCS 9
#define pinoCLK 10
#define saidaRele 11

#define buttonSettingD 2
#define buttonSettingE 4

float maxTemp = 30;
float temp = 0;

int displayInt = 0x27;
int displaySize = 16;
int displayLine = 2;

int stateButtonD = 0;
int stateButtonE = 0;

enum state {
  OFF, ON
};

enum display {
  ERROR, SETUP, HOME, CONTROL, CONFIRM, TESTING, SHUTDOWN
};

state checkState;
display checkDisplay;

MAX6675 termo(pinoCLK, pinoCS, pinoSO);
LiquidCrystal_I2C lcd(displayInt, displaySize, displayLine);

void setup() {

  lcd.init();

  pinMode(saidaRele, OUTPUT);
  digitalWrite(saidaRele, HIGH);

  checkState = ON;
  checkDisplay = SETUP;

  displayStart();

  maxTemp = termo.readCelsius();
  temp = termo.readCelsius();

}

void loop() {

  if (checkDisplay == SHUTDOWN && checkState == OFF || checkDisplay == ERROR) {
    checkDisplay = HOME;
    checkState = ON;
    maxTemp = 30;
    temp = 0;

  } else {

    loadDisplays();
    controlButton();
    checkTemp();
  }

}

void checkTemp(){

  if(termo.readCelsius() >= maxTemp && maxTemp >= 0 && checkDisplay == TESTING && digitalRead(saidaRele) == LOW){
    checkDisplay = SHUTDOWN;
    digitalWrite(saidaRele, HIGH);
  }
}

void loadDisplays() {

  switch (checkDisplay) {
    case HOME:
      displayHome();
      break;

    case CONTROL:
      displayControl();
      break;

    case CONFIRM:
      displayConfirm();
      break;

    case TESTING:
    displayTesting();
    break;

    case SHUTDOWN:
      displayShutdown();
      checkState = OFF;
      if(digitalRead(saidaRele) == LOW){
        digitalWrite(saidaRele, HIGH);
      }
      break;

    default:
      checkDisplay = SHUTDOWN;
      checkState = OFF;
      break;
  }
  delay(500);
}

void displayStart() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando.");
  lcd.setCursor(0, 1);
  lcd.print("Vedatest");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando..");
  lcd.setCursor(0, 1);
  lcd.print("Vedatest");
  delay(1000);
  lcd.clear();
  lcd.print("Iniciando...");
  lcd.setCursor(0, 1);
  lcd.print("Vedatest");
  delay(3500);
  checkDisplay = HOME;
}

void displayHome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Clique button(E).");
  lcd.setCursor(0, 1);
  lcd.print("Vedatest");
}

void displayControl() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Atual: ");
  lcd.print("C");
  lcd.print(termo.readCelsius(), 0);
  lcd.setCursor(0, 1);
  lcd.print("Set: ");
  lcd.print("C");
  lcd.print(maxTemp, 1);
}

void displayConfirm() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirme: ");
  lcd.print(maxTemp, 0);
  lcd.setCursor(0, 1);
  lcd.print("Sim (E) e Nao (D).");
}
void displayTesting() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Atual: ");
  lcd.print(termo.readCelsius(), 0);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Estimasse: ");
  lcd.print(maxTemp, 1);
  lcd.print("C");
}

void displayShutdown() {
  digitalWrite(saidaRele, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Concluimos.");
  lcd.setCursor(0, 1);
  lcd.print("Vedatest - Home");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Concluimos..");
  lcd.setCursor(0, 1);
  lcd.print("Vedatest - Home");
  delay(1000);
  lcd.clear();
  lcd.print("Concluimos...");
  lcd.setCursor(0, 1);
  lcd.print("Tudo corretamente!");
  delay(3500);
}

void controlButton() {
  switch (checkDisplay) {
    case HOME:
      if (digitalRead(buttonSettingE) == HIGH) {
        checkDisplay = CONTROL;
        digitalWrite(buttonSettingE, LOW);
      }
      break;

    case CONTROL:
      if (digitalRead(buttonSettingD) == HIGH && digitalRead(buttonSettingE) == LOW) {
        maxTemp = maxTemp + 1;
      }
      else if(digitalRead(buttonSettingE) == HIGH && digitalRead(buttonSettingD) == LOW){
        checkDisplay = CONFIRM;
        digitalWrite(buttonSettingE, LOW);
      }
      delay(1);
      break;

    case CONFIRM:

      if (digitalRead(buttonSettingE) == HIGH && digitalRead(buttonSettingD) == LOW) {
        checkDisplay = TESTING;
        digitalWrite(buttonSettingE, LOW);
        digitalWrite(saidaRele, LOW);

      }else if(maxTemp <= 30 || maxTemp >= 1000 || maxTemp <= termo.readCelsius()){
      
       checkDisplay = ERROR;
       digitalWrite(buttonSettingE, LOW);
       lcd.clear();
       lcd.setCursor(0, 0);
       lcd.print("Ocorreu um erro!");
       lcd.setCursor(0, 1);
       lcd.print("Valor incorreto.");
       delay(2000);
       checkDisplay = CONTROL;
       
      } else if (digitalRead(buttonSettingD) == HIGH && digitalRead(buttonSettingE) == LOW) {
        checkDisplay = CONTROL;
        digitalWrite(buttonSettingD, HIGH);

      } else if (digitalRead(buttonSettingD) == HIGH && digitalRead(buttonSettingE) == HIGH) {

        checkDisplay = ERROR;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Aperte apenas 1.");
        delay(1500);
        checkDisplay = CONTROL;
        digitalWrite(buttonSettingD, LOW);
        digitalWrite(buttonSettingE, LOW);

      }
      break;

    case TESTING:
      if (digitalRead(buttonSettingD) == HIGH) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Cancelando...");
        lcd.setCursor(0, 1);
        lcd.print("Vedatest - Teste");
        delay(1500);
        digitalWrite(saidaRele, HIGH);
        checkDisplay = SHUTDOWN;
      }
      break;

    case SHUTDOWN:
      displayShutdown();
      checkState = OFF;
      break;

    default:
      checkDisplay = SHUTDOWN;
      checkState = OFF;
      break;
  }
}

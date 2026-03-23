#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>
#include <avr/eeprom.h>


Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

byte customChar[8] = {
	0b00100,
	0b01110,
	0b10101,
	0b00100,
	0b00100,
	0b00100,
	0b00000,
	0b00000
};

byte customChar2[8] = {
	0b00000,
	0b00000,
	0b00100,
	0b00100,
	0b00100,
	0b10101,
	0b01110,
	0b00100
};

unsigned long previousMillis = 0; 
const long interval = 2000;
bool R = true;
int currentAccountdisplayed = 0;
char jobtitle = 0;
bool Program = false;

struct PayrollAccount {
    String ID;         // employee ID
    int Grade;         // Job grade 
    String JobTitle;   // Job title 
    float Salary;      // Salary in £
    String Pension;      // Pension status 
}; // structures the Payroll accounts




const int max_accounts = 10; // This is the maximum amount of accounts
PayrollAccount accounts[max_accounts];
int accountCount = 0;


void setup() {
  Serial.begin(9600);  
  lcd.begin(16, 2);     
  lcd.setBacklight(3); 
  lcd.createChar(0, customChar);  // custom characters 
  lcd.createChar(1, customChar2);
}

void loop() {

  

      if (R) {
        delay(1000);
        Serial.print("R"); //R running repeatdly
        delay(1000);
        lcd.clear();

        
      }
    MainProgram();
  buttonInput();
}


void MainProgram() {
  if (Serial.available()) {  // Reads the serial monitor for an input
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (!Program && input == "BEGIN") {
      Program = true;
      Serial.println("BASIC");
      lcd.clear();
      lcd.setBacklight(7);
      lcd.print("DEBUG: START"); // make sure that the LCD is working fine, i had issues with it not running the program at all
      delay(2000);
      lcd.clear();
      lcd.print("NO ACCOUNT");
      lcd.setCursor(0, 1);
      lcd.print("AVAILABLE");

      
      
      
     
      
      R = false;

      } else if (Program) {
      // commands
      if (input.startsWith("ADD")) {   
        addPayrollAccount(input);

      } else if (input.startsWith("SAL")) {
        updateSalary(input);  

      } else if (input.startsWith("PST")) {
        updatePensionStatus(input);

      } else if (input.startsWith("GRD")) {
        updateJobGrade(input);

      } else if (input.startsWith("CJT")) {
        updateJobTitle(input);

      } else if (input.startsWith("DEL")) {
        deletePayrollAccount(input);

      } else {
        Serial.println("ERROR: Request not verified");   // Ensuring inputs start like this or request is not verified
      }
    }
  }  
}

void addPayrollAccount (String input) {
  if (accountCount >= max_accounts) {
    Serial.println("ERROR: Maximum account limit reached");
    return;
  } // code for how the payroll presentation
  int fdash = input.indexOf('-');
  int sdash = input.indexOf('-', fdash + 1);
  int tdash = input.indexOf('-', sdash + 1);

  if (fdash == -1 || sdash == -1 || tdash == -1) { // sorting
    Serial.println("ERROR: Invalid ADD");
    return;
  }
  String ID = input.substring(fdash + 1, sdash);
  String GradeString = input.substring(sdash + 1, tdash);
  String JobTitle = input.substring(tdash + 1);

  if (ID.length() !=7 || JobTitle.length() <3 || JobTitle.length() > 17) {
    Serial.println("ERROR: Invalid ID or title length");
    return;
  }
  
  int Grade = GradeString.toInt();
  if (Grade < 1 || Grade > 9) {
    Serial.println("ERROR: INVALID GRADE");
    return;
  }

  // ADD TO PAYROLL ACCOUNTS
  accounts[accountCount].ID = ID;
  accounts[accountCount].Grade = Grade;
  accounts[accountCount].JobTitle = JobTitle;
  accounts[accountCount].Salary = 0.0;
  accounts[accountCount].Pension = "";
  accountCount++;
 

  
  Serial.println("DONE :)");
  showPayrollAccount(currentAccountdisplayed);

    Serial.println("Accounts added so far:");
    for (int x = 0; x < accountCount; x++) {    //debugging to make sure accounts are added.
    Serial.print("ID: ");
    Serial.println(accounts[x].ID);
  }
}

void updateSalary (String input) {
  int fdash = input.indexOf('-');
  int sdash = input.indexOf('-', fdash + 1);
  

  if (fdash == -1 || sdash == -1) {
    Serial.println("ERROR: Invalid format of update salary");
    return;
  }

  String ID = input.substring(fdash + 1, sdash);
  String SALstr = input.substring(sdash + 1);
 
  float newSalary = SALstr.toFloat(); // turning the salary string to a float

  if (newSalary <= 0 || SALstr.length() == 0) {
    Serial.println("ERROR: Invalid ID or amount inputted");
    return;
  }
  bool foundID = false;
  for (int x = 0; x < accountCount; x++) { // making sure the ID inputted is in the system
    if (accounts[x].ID == ID) {
        foundID = true;
        
        accounts[x].Salary = newSalary;

        Serial.println("DONE :)");
        return;
      }
    }
    if (!foundID) {
    Serial.println("ERROR: ID could not be found");
  } 
} 

  



void updatePensionStatus(String input) {
  int fdash = input.indexOf('-');
  int sdash = input.indexOf('-', fdash + 1);

  if (fdash == -1 || sdash == -1) {
    Serial.println("ERROR: invalid PST format");
    return;
  }
  String ID = input.substring(fdash + 1, sdash);
  String newPension = input.substring(sdash + 1);

  if (newPension != "PEN" && newPension != "NPEN") {
    Serial.println("ERROR: INVALID PEN VALUE");
    return;
  }

  bool foundID = false;
  for (int y = 0; y < accountCount; y++) {
    if (accounts[y].ID == ID) {
      foundID = true;
    
      if (accounts[y].Salary == 0.0) {
        Serial.println("ERROR: pension status cannot be altered for an account with £0.00");
      }

      accounts[y].Pension = newPension;
      Serial.println("DONE :)");
      return;
    }
  }
}

void updateJobGrade(String input) {
  int fdash = input.indexOf('-');
  int sdash = input.indexOf('-', fdash + 1);

  if (fdash == -1 || sdash == -1) {
    Serial.println("ERROR: GRD invalid  format");
    return;
  }
  String ID = input.substring(fdash + 1, sdash);
  String newJBgrade = input.substring(sdash + 1);

  int newGrade = newJBgrade.toInt();
  if (newGrade < 1 || newGrade > 9 || newJBgrade.length() == 0) {
    Serial.println("ERROR: Invalid GRD value");
    return;
  }

  bool foundID = false;
  for (int x = 0; x < accountCount; x++) {
    if (accounts[x].ID == ID) {
      foundID = true;

      accounts[x].Grade = newGrade;
      Serial.println("DONE :)");
      return;
    }
  }
  if (!foundID) {
    Serial.println("ERROR: ID not found");
    return;
  }

}

void updateJobTitle(String input) {
  int fdash = input.indexOf('-');
  int sdash = input.indexOf('-', fdash + 1);

  if (fdash == -1 || sdash == -1) {
    Serial.println("ERROR: CJT invalid  format");
    return;
  }
  String ID = input.substring(fdash + 1, sdash);
  String newJobtitle = input.substring(sdash + 1);

  if (newJobtitle.length() < 3 || newJobtitle.length() > 17) {
    Serial.println("ERROR: Invalid title length");
    return;
  }

  bool foundID = false;
  for (int x = 0; x < accountCount; x++) {
    if (accounts[x].ID == ID) {
      foundID = true;

      accounts[x].JobTitle = newJobtitle;

      Serial.println("DONE :)");
      return;
    }
  }

  if (!foundID) {
    Serial.println("ERROR: ID not found");
  }  
}

void deletePayrollAccount(String input) {
  int fdash = input.indexOf('-');

  if (fdash == -1) {
    Serial.println("ERROR: invalid DEL format");
    return;
  }

  String ID = input.substring(fdash + 1);

  bool foundID = false;
  for (int x = 0; x < accountCount; x++) {
    if (accounts[x].ID == ID) {
      foundID = true;

      for (int z = x; z < accountCount - 1; z++) {
        accounts[z] = accounts[z + 1];
      }

      accountCount--;

      Serial.println("DONE :)");
      return;
    }
  }

  if (foundID) {
    Serial.println("ERROR: ID not found");
  }
}

void showPayrollAccount(int index) {
  if (index < 0 || index >= accountCount) {
    Serial.println("ERROR: No accounts available");
    return;
  }

  PayrollAccount account = accounts[index];

  // Top row
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(8);
  lcd.print(account.Grade);
  lcd.print(" ");
  lcd.print(account.Pension);
  lcd.print(" ");
  lcd.print(account.Salary);
  // Bottom row
  lcd.setCursor(0, 1);
  lcd.write(1);
  lcd.print(account.ID);
  lcd.print(" ");
  lcd.print(account.JobTitle);
    
}


void buttonInput() {
  uint8_t buttons = lcd.readButtons();

  if (buttons & BUTTON_DOWN) {
    delay(500);
    currentAccountdisplayed++;
    if (currentAccountdisplayed >= accountCount) {
      currentAccountdisplayed = 0;
    }
    showPayrollAccount(currentAccountdisplayed);
  }

  if (buttons & BUTTON_UP) {
    delay(500);
    currentAccountdisplayed--;
    if (currentAccountdisplayed < 0) {
      currentAccountdisplayed = accountCount - 1;
    }
    showPayrollAccount(currentAccountdisplayed);
  }

  if (buttons & BUTTON_SELECT) { // showing my Name and Student ID
    
    lcd.clear();
    lcd.setBacklight(5);
    lcd.setCursor(0, 0);
    lcd.print("F428898");
    lcd.setCursor(0, 1);
    lcd.print("Wesley Da Silva");
    delay(2000);
    showPayrollAccount(currentAccountdisplayed);
    lcd.setBacklight(7);
    return;

  }

}






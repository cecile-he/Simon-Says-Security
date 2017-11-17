// declare constants for button and light pins
const int greenButton = 7;
const int greenLED = 13;

const int redButton = 6;
const int redLED = 12; 

const int blueButton = 5;
const int blueLED = 11;

const int yellowButton = 4;
const int yellowLED = 10;

// declare constant for interrupt pin
const int interPin = 2; // interrupt is attached to pin 2

// declare state variables (changes)
int greenState = 0;
int redState = 0;
int blueState = 0;
int yellowState = 0;

// declare variable to keep track of round 
int roundNumber = 0; 

// declare variable to keep track of react points; you must have 4 react points to win the game
int reactPoints = 0; 

// declare variable to keep track of unlock points; you must have 2 unlock points and 4 react points to unlock the vault 
int unlockPoints = 0; 

// declare variable that stores the value of the button was hit (1 to 4)
// volatile because we will be changing it inside the interrupt
volatile int hitButton = 0;

// declare variable that stores the rounds that will be skipped (1, 2, 3, or 4)
int skipOne = 0; // first round that is skipped
int skipTwo = 0; // second round that is skipped

// declare variable to store interrupt button state (changes)
volatile int buttonState = 0; // variable that triggers the interrupt

//variables for timing:
unsigned long seconds = 0; // time in seconds (incremeted in timerA ISR)
unsigned long startTime = 0;
unsigned long endTime = 0;
unsigned long duration = 0;

// the "setup" code that will only run once
void setup() { 
  // initialize push button pins as inputs:
  pinMode(greenButton, INPUT);
  pinMode(redButton, INPUT);
  pinMode(blueButton, INPUT);
  pinMode(yellowButton, INPUT);
  
  pinMode(interPin, INPUT); // interrupt pin is an input too

  // initialize LED pins as outputs: 
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);

  // attach an interrupt to the ISR vector
  attachInterrupt(digitalPinToInterrupt(interPin), buttonPushed, RISING); // handler 'buttonPushed' is attached to pin 2, triggered with state change

  // set up serial monitor
  Serial.begin(9600);
  Serial.println("Welcome to the Serial Monitor.");

  // random seed will ensure that each run of the sketch results in new random numbers
  randomSeed(analogRead(0));

 cli(); //disable interrupts
    TCCR1A = 0;   //set timer clock registers to 0
    TCCR1B = 0;  
    
    TCNT1  = 0;   //initialize counter value to 0
    
    //set compare register to count for 1 seconds
    //15625 is the number of ticks that occur during 1 second if prescaled to 1024
    //must be less than 65536 ticks (max value for a 16 bit)
    //FORMULA: 16'000'000(Hz) / 1024(prescaler) -1 = 15624 --> we divided this by 1000 to get milliseconds instead of seconds.
    OCR1A = 15624; //(OCR = output compare register)
    //it is divided by 1000 to get seconds instead of milliseconds
    
    TCCR1B |= (1 << WGM12); //turn on CTC (clear timer on compare) (i.e. it will reset again after every cycle to avoid overflow)
    
    //Set prescaler to 1024 - CS(12, 11, 10) bits are (1, 0, 1)

    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    
    TIMSK1 |= (1 << OCIE1A); //enable timer compare interrupt. This triggers the interrupt after the comparison is true.

    //the output compare (OCR1A) is cleared on the compare match between TCNT1 and OCR1A
    
 sei(); //re-enable interrupts

}
//--------------------------END OF SETUP------------------------------------------------------------------------------------------------


void loop() {
  
  if (roundNumber == 0) { // in round "0", the vaultkeeper will select the 2 rounds for which "simon doesn't say so" (must be skipped)
    Serial.println("Hi there. Please select the first round that the unlocker must skip.");
    
    delay(4000);
    skipOne = hitButton;
    if (hitButton == 0){
      Serial.println("You took too long! The first round will be randomly selected. ");
      skipOne = random(1, 5);
    }
    Serial.print("Your first secret round to skip is: "); 
    Serial.println(skipOne); 
    hitButton = 0; // set back to zero

    Serial.println(" "); // to leave a space 
    Serial.println("Now select the second round that the unlocker must skip.");

    delay(4000);

    skipTwo = hitButton;
    //if took too long, code is random.
    
    if (hitButton == 0){
      Serial.println("You took too long! The second round will be randomly selected. ");
      skipTwo = random(1, 5);
    }
 
    Serial.print("Your second secret round to skip is: ");
    Serial.println(skipTwo);

    roundNumber++; // round 0 goes to round 1 
    hitButton = 0;
  }

//----------------------------BEGIN GAME ------------------------------------------------------------------------------------

  else if (roundNumber == 1) { // IN ROUND ONE
    Serial.println("");
    Serial.println("Welcome to round 1. Try to match the light!");

    int copyMe = random(1, 5); // generating random int from 1-4, representing corresponding LED
    //Serial.println(copyMe);
    delay(2500);

    ledFlash(copyMe); // call function to flash that LED
    //delay(5000);
    
startTime = seconds; //takes the seconds value from the start of the round
delay(5000);

//while (hitButton == 0){

//}

if (hitButton == 0){
  endTime = startTime; //if no button was pressed, set endTime equal to startTime to make duration = 0.
}

Serial.println("");
Serial.print("startTime: ");
Serial.println(startTime);
Serial.print("endTime: ");
Serial.println(endTime);
duration = endTime - startTime;
Serial.print("duration: ");
Serial.println(duration);
Serial.println("");
     
    bool correct = compareInput(copyMe, hitButton); // compare user Input
    if (correct == 1) {
      reactPoints++;
    }
    else if (correct != 1) {
      reactPoints = reactPoints;
    }
    Serial.print("You now have this many react points: ");
    Serial.println(reactPoints);
    Serial.print("Round 1 took you ");
    Serial.print(duration);
    Serial.println(" seconds. ");

    if ((roundNumber == skipOne) || (roundNumber == skipTwo)) { // check if this round was supposed to be skipped
      if (hitButton == 0) { // if the round was skipped (no button pushed), increase unlockPoints
        if (skipOne == skipTwo) { // if this round was selected as both skipOne and skipTwo, unlockPoints increases by 2
          unlockPoints = unlockPoints + 2;
        }
        else {
          unlockPoints++;
        } 
      }
      else { // if a button was pushed and round was played, no points added
        unlockPoints = unlockPoints; // no points added
      }
    }

    if ((roundNumber != skipOne) && (roundNumber != skipTwo) && hitButton == 0) { // so player can't skip all levels to bypass security
      unlockPoints = unlockPoints - 1;
    }

    Serial.print("You now have this many unlock points: ");
    Serial.println(unlockPoints);
    
    delay(5000);
    roundNumber++; // round = 2 now 
    hitButton = 0;
    startTime = 0;
    endTime = 0;
    duration = 0;
  }
//-------------------------------------------------------------------
  else if (roundNumber == 2) { // IN ROUND TWO
    Serial.println("");
    Serial.println("Welcome to round 2. Try to match the light!");
    
    int copyMe = random(1, 5); 
    //Serial.println(copyMe);
    delay(2500);
    
    ledFlash(copyMe); 
    //delay(5000);

startTime = seconds;
delay(5000);

if (hitButton == 0){
  endTime = startTime;
}

Serial.println("");
Serial.print("startTime: ");
Serial.println(startTime);
Serial.print("endTime: ");
Serial.println(endTime);
duration = endTime - startTime;
Serial.print("duration: ");
Serial.println(duration);
Serial.println("");
    
    bool correct = compareInput(copyMe, hitButton); // compare user Input
    if (correct == 1) {
      reactPoints++;
    }
    else if (correct != 1) {
      reactPoints = reactPoints;
    }
    Serial.print("You now have this many react points: ");
    Serial.println(reactPoints);
    Serial.print("Round 2 took you ");
    Serial.print(duration);
    Serial.println(" seconds. ");

    if ((roundNumber == skipOne) || (roundNumber == skipTwo)) { // check if this round was supposed to be skipped
      if (hitButton == 0) { // if the round was skipped (no button pushed), increase unlockPoints
        if (skipOne == skipTwo) { // if this round was selected as both skipOne and skipTwo, unlockPoints increases by 2
          unlockPoints = unlockPoints + 2;
        }
        else {
          unlockPoints++;
        } 
      }
      else { // if a button was pushed and round was played, no points added
        unlockPoints = unlockPoints; // no points added
      }
    }

    if ((roundNumber != skipOne) && (roundNumber != skipTwo) && hitButton == 0) { // so player can't skip all levels to bypass security
      unlockPoints = unlockPoints - 1;
    }

    Serial.print("You now have this many unlock points: ");
    Serial.println(unlockPoints);
    
    delay(5000);
    roundNumber++; // round = 3 now
    hitButton = 0;
    startTime = 0;
    endTime = 0;
    duration = 0;

  }
//------------------------------------------------
  else if (roundNumber == 3) { // IN ROUND THREE
    Serial.println("");
    Serial.println("Welcome to round 3. Try to match the light!");

    int copyMe = random(1, 5); 
    //Serial.println(copyMe);
    delay(2500);
    
    ledFlash(copyMe); 
    //delay(5000);

startTime = seconds;
delay(5000);

if (hitButton == 0){
  endTime = startTime;
}

Serial.println("");
Serial.print("startTime: ");
Serial.println(startTime);
Serial.print("endTime: ");
Serial.println(endTime);
duration = endTime - startTime;
Serial.print("duration: ");
Serial.println(duration);
Serial.println("");
    
    bool correct = compareInput(copyMe, hitButton); // compare user Input
    if (correct == 1) {
      reactPoints++;
    }
    else if (correct != 1) {
      reactPoints = reactPoints;
    }
    Serial.print("You now have this many react points: ");
    Serial.println(reactPoints);
    Serial.print("Round 3 took you ");
    Serial.print(duration);
    Serial.println(" seconds. ");

    if ((roundNumber == skipOne) || (roundNumber == skipTwo)) { // check if this round was supposed to be skipped
      if (hitButton == 0) { // if the round was skipped (no button pushed), increase unlockPoints
        if (skipOne == skipTwo) { // if this round was selected as both skipOne and skipTwo, unlockPoints increases by 2
          unlockPoints = unlockPoints + 2;
        }
        else {
          unlockPoints++;
        } 
      }
      else { // if a button was pushed and round was played, no points added
        unlockPoints = unlockPoints; // no points added
      }
    }

    if ((roundNumber != skipOne) && (roundNumber != skipTwo) && hitButton == 0) { // so player can't skip all levels to bypass security
      unlockPoints = unlockPoints - 1;
    }

    Serial.print("You now have this many unlock points: ");
    Serial.println(unlockPoints);
    
    delay(5000);
    roundNumber++; // round = 4 now
    hitButton = 0;
    startTime = 0;
    endTime = 0;
    duration = 0;

  }
//-------------------------------------------------------
  else if (roundNumber == 4) { // IN ROUND FOUR
    Serial.println("");
    Serial.println("Welcome to round 4 - the last round! Try to match the light!");
    
    int copyMe = random(1, 5); 
    //Serial.println(copyMe);
    delay(2500);

    
    ledFlash(copyMe);
    //delay(5000); 

startTime = seconds;
delay(5000);

if (hitButton ==0){
  endTime = startTime;
}

Serial.println("");
Serial.print("startTime: ");
Serial.println(startTime);
Serial.print("endTime: ");
Serial.println(endTime);
duration = endTime - startTime;
Serial.print("duration: ");
Serial.println(duration);
Serial.println("");
    
    bool correct = compareInput(copyMe, hitButton); // compare user Input
    if (correct == 1) {
      reactPoints++;
    }
    else if (correct != 1) {
      reactPoints = reactPoints;
    }
    Serial.print("You now have this many react points: ");
    Serial.println(reactPoints);
    Serial.print("Round 4 took you ");
    Serial.print(duration);
    Serial.println(" seconds. ");

    if ((roundNumber == skipOne) || (roundNumber == skipTwo)) { // check if this round was supposed to be skipped
      if (hitButton == 0) { // if the round was skipped (no button pushed), increase unlockPoints
        if (skipOne == skipTwo) { // if this round was selected as both skipOne and skipTwo, unlockPoints increases by 2
          unlockPoints = unlockPoints + 2;
        }
        else {
          unlockPoints++;
        } 
      }
      else { // if a button was pushed and round was played, no points added
        unlockPoints = unlockPoints; // no points added
      }
    }

    if ((roundNumber != skipOne) && (roundNumber != skipTwo) && hitButton == 0) { // so player can't skip all levels to bypass security
      unlockPoints = unlockPoints - 1;
    }

    Serial.print("You now have this many unlock points: ");
    Serial.println(unlockPoints);
    
    delay(5000);
    finishGame();
    roundNumber++; // round = 5 now, which doesn't exist...
    hitButton = 0;
    startTime = 0;
    endTime = 0;
    duration = 0;

  }
  else if (roundNumber > 4) {
    while (1 == 1) {
      // do nothing =) user should exit game or restart on their own
    }
  }
}


//-----------------------INTERRUPT HANDLER-------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
  seconds++; //add 1 second to total time counting
}

//-----------------------FLASHING LED LIGHT------------------------------------------------

void ledFlash(int led) { // deciding which LED to light up based on which button was pushed

  if (led == 1){
    digitalWrite(greenLED, HIGH);
    delay(1000);
    digitalWrite(greenLED, LOW);
  }

  else if (led == 2){
    digitalWrite(redLED, HIGH);
    delay(1000);
    digitalWrite(redLED, LOW);
  }

  else if (led == 3){
    digitalWrite(blueLED, HIGH);
    delay(1000);
    digitalWrite(blueLED, LOW);
  }

  else if (led == 4){
    digitalWrite(yellowLED, HIGH);
    delay(1000);
    digitalWrite(yellowLED, LOW);
  }
}

bool compareInput(int led, int hit) { // bool function checks if user input matched led light; returns true/false
  int correct = 0;
  if (hit == led) {
    Serial.println("You got 'em!");
    correct = 1;
  }
  else {
    Serial.println("Try again, sweetie.");
    correct = 0;
  }
  return correct;
}

void finishGame() {
  Serial.print("The game is finished. You have won ");
  Serial.print(reactPoints);
  Serial.println(" points out of 4.");

  duration = seconds;
  Serial.print("You were playing the game for ");
  Serial.print(duration);
  Serial.println(" seconds. ");

  if (reactPoints == 4) {
    Serial.println("You have won by matching the light in all 4 rounds!");
    if (unlockPoints == 2) { // technically this loop will NEVER run as you cannot answera ll 4 rounds and have unlock = 2
      Serial.println("You have also unlocked bypassed the security for skipping the secret round(s). Sneaky!");
    }
    else if (unlockPoints != 2) {
      Serial.println("Unfortunately, you have not bypassed the security as you did not skip the secret round(s). Impostor!");
    }
    
  }
  else {
    Serial.println("You did not match the light in all 4 rounds. ");
    if (unlockPoints == 2) { 
      Serial.println("However, you did skip the correct secret round(s), and you have bypassed security! Sneaky!"); 
    }
    else if (unlockPoints != 2) {
      Serial.println("You also failed to skip only the correct secret round(s), so you have no bypassed the security. Impostor!");
    }
  }
  delay(5000);
}

void buttonPushed() {
  buttonState = digitalRead(buttonState);
  // Serial.println("Interrupted fired! Button pressed was number: "); // test if interrupt works 
endTime = seconds;
  // decide which button was pressed 
  if (digitalRead(greenButton) == HIGH) {
    hitButton = 1;
    digitalWrite(greenLED, HIGH);
    delay(1000);
    digitalWrite(greenLED, LOW);
    Serial.println(hitButton);
    
  }

  else if (digitalRead(redButton) == HIGH) {
    hitButton = 2;
    digitalWrite(redLED, HIGH);
    delay(1000);
    digitalWrite(redLED, LOW);
    Serial.println(hitButton);
  } 

  else if (digitalRead(blueButton) == HIGH) {
    hitButton = 3;
    digitalWrite(blueLED, HIGH);
    delay(1000);
    digitalWrite(blueLED, LOW);
    Serial.println(hitButton);
  } 

  else if (digitalRead(yellowButton) == HIGH) {
    hitButton = 4;
    digitalWrite(yellowLED, HIGH);
    delay(1000);
    digitalWrite(yellowLED, LOW);
    Serial.println(hitButton);
  } 

  buttonState = !buttonState;
}


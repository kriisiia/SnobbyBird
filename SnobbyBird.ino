#include <WaveHC.h>
#include <WaveUtil.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;   // This object represent the WAV file 
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time


#define playcomplete(x) ROM_playcomplete(PSTR(x))         // save RAM by using program memory strings

#define servo 7
#define redled 9
#define eyeleds 18
#define mouthleds 17
#define midmouthleds 16
#define outermouthleds 19

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println(F("Wave test!"));

  pinMode(2, OUTPUT); 
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(redled, OUTPUT);
  pinMode(servo, OUTPUT);
  pinMode(eyeleds, OUTPUT);
  pinMode(outermouthleds, OUTPUT);
  pinMode(midmouthleds, OUTPUT);
  pinMode(mouthleds, OUTPUT);

  randomSeed(analogRead(0));


  if (!card.init()) {
    Serial.println(F("Card init. failed!")); 
    return;
  }
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);

  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {   // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                           // we found one, lets bail
  }
  if (part == 5) {                     // if we ended up not finding one  :(
    Serial.println(F("No valid FAT partition!"));  // Something went wrong, lets print out why
  }

  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  Serial.print(F(", type is FAT"));
  Serial.println(vol.fatType(), DEC);     // FAT16 or FAT32?

  // Try to open the root directory
  if (!root.openRoot(vol)) {
    Serial.println(F("Can't open root dir!"));      // Something went wrong,
  }

  // Whew! We got past the tough parts.
  Serial.println(F("Files found (* = fragmented):"));

  // Print out all of the files in all the directories.
  root.ls(LS_R | LS_FLAG_FRAGMENTED);
}


void pulseServo(uint8_t servopin, uint16_t p) {

  digitalWrite(servopin, HIGH);
  delayMicroseconds(600);
  while (p--) {
    delayMicroseconds(4);
  }
  digitalWrite(servopin, LOW);
  delay(18);
}

uint8_t pumpkinstate = 0;

void loop() { 
  int distsensor, i;
  long time;
  /*
   for (i=0; i<50; i++) {
   pulseServo(servo,0);
   }
   for (i=0; i<50; i++) {
   pulseServo(servo,400);
   }
   return;
   */
  distsensor = 0;
  for (i=0; i<8; i++) {
    distsensor += analogRead(0);
    delay(50);
  }
  distsensor /= 8;

  Serial.print(F("Sensor = ")); 
  Serial.println(distsensor);


  if (distsensor < 299 ){
    playcomplete("004.WAV"); 
  }

  else if ((distsensor > 300) && (distsensor < 400)) {
    playcomplete("003.WAV");   


  }
  if (distsensor <= 500) {
    digitalWrite(eyeleds, HIGH); 
  } 
  if (distsensor > 500) {
    digitalWrite(eyeleds, LOW);  
    pumpkinstate = 1;
    // nobody there. one out of 200 times play one of the scary sounds (once every few minutes)
    i = random(200);
    //Serial.println(i);
    if (i == 0) {
      i = random(3);
      if (i == 0) {
        playcomplete("001.WAV");
      } 
      else if (i == 1) {
        playcomplete("002.WAV");
      } 
      else {
        playcomplete("003.WAV");   
      }
    }
  } 
  else if ((distsensor > 300) && (distsensor < 400)) {
    if (pumpkinstate <= 1) {    // play "hello children"
      playcomplete("004.WAV"); 
    } 
    else {
      i = random(60);            // more often
      //Serial.println(i);
      if (i == 0) {
        i = random(3);
        if (i == 0) {
          playcomplete("005.WAV");
        } 
        else if (i == 1) {
          playcomplete("006.WAV");
        } 
        else {
          playcomplete("007.WAV");   
        }
      } 
    }
    pumpkinstate = 2; 
  } 
  else if ((distsensor > 100) && (distsensor < 200)) {
    if (pumpkinstate <= 2) {    // play "hello children"
      playcomplete("008.WAV"); 
    } 
    else {
      i = random(50);            // more often
      //Serial.println(i);
      if (i == 0) {
        i = random(3);
        if (i == 0) {
          playcomplete("009.WAV");
        } 
        else if (i == 1) {
          playcomplete("010.WAV");
        } 
        else {
          playcomplete("011.WAV");   
        }
      }
    }
    pumpkinstate = 3;
  } 
  else if (distsensor < 50) {
    if (pumpkinstate <= 3) {    // play "hello children"
      playcomplete("012.WAV");    
    } 
    else {
      i = random(30);            // more often
      //Serial.println(i);
      if (i == 0) {
        i = random(2);
        if (i == 0) {
          playcomplete("013.WAV");
        } 
        else if (i == 1) {
          playcomplete("014.WAV");
        } 
      }

    }
    pumpkinstate = 4;
  }
}



void ROM_playcomplete(const char *romname) {
  char name[13], i;
  uint8_t volume;
  int v2;

  for (i=0; i<13; i++) {
    name[i] = pgm_read_byte(&romname[i]);
  }
  name[12] = 0;
  Serial.println(name);
  playfile(name);
  while (wave.isplaying) {
    volume = 0;
    for (i=0; i<8; i++) {
      v2 = analogRead(1) - 512;
      if (v2 < 0) 
        v2 *= -1;
      if (v2 > volume)
        volume = v2;
      delay(5);
    }
    if (volume > 200) {
      digitalWrite(outermouthleds, HIGH);
    } 
    else {
      digitalWrite(outermouthleds, LOW);
    }
    if (volume > 150) {
      digitalWrite(midmouthleds, HIGH);
    } 
    else {
      digitalWrite(midmouthleds, LOW);
    } 
    if (volume > 100) {
      digitalWrite(mouthleds, HIGH);
    } 
    else {
      digitalWrite(mouthleds, LOW);
    } 
    //Serial.print(F("vol = ")); Serial.println(volume, DEC);
  }
  file.close();
}

void playfile(char *name) {
  // look in the root directory and open the file
  if (!file.open(root, name)) {
    putstring("Couldn't open file "); 
    Serial.print(name); 
    return;
  }
  if (!wave.create(file)) {
    Serial.println(F(" Not a valid WAV")); 
    return;
  }
  // ok time to play!
  wave.play();
}




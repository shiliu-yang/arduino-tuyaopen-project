#include <Arduino.h>
#include "hardwareConfig.h"
#include "appDisplay.h"
#include "songs.h"

void playSong(int note[],int duration[],int legth,float timeCoefficient)
{
  for (int thisNote = 0; thisNote < legth; thisNote++) {
    int noteDuration = 1000 / duration[thisNote];
    if (!clockIsBell()) {
      break;
    }
    tone(BUZZER_PIN, note[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * timeCoefficient;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}

void playSongs(int clockBellNum)
{
  switch(clockBellNum){
    case 0:
      playSong(SuperMario_note,SuperMario_duration,sizeof(SuperMario_note)/sizeof(int),2.1);
      break;
    case 1:
      playSong(AlwaysWithMe_note,AlwaysWithMe_duration,sizeof(AlwaysWithMe_note)/sizeof(int),3);
      break;
    case 2:
      playSong(DreamWedding_note,DreamWedding_duration,sizeof(DreamWedding_note)/sizeof(int),2.7);
      break;
    case 3:
      playSong(CastleInTheSky_note,CastleInTheSky_duration,sizeof(CastleInTheSky_note)/sizeof(int),2.8);
      break;
    case 4:
      playSong(Canon_note,Canon_duration,sizeof(Canon_note)/sizeof(int),1.8);
      break;
    default:
      break;
  }
}

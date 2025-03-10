#ifndef __AUDIO_GROUP_H__
#define __AUDIO_GROUP_H__

#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <MD_AD9833.h>

class AudioGroup
{
public:
  AudioGroup(hd44780_I2Cexp *lcd, MD_AD9833 *generator, byte id, long frequency, byte step, int phase, byte state){
    _lcd = lcd;
    _generator = generator;
    _id = id;
    _frequency = frequency;
    _step = step;
    _phase = phase;
    _state = state;
  }

  void step_frequency(int steps){
    int increment = step_to_frequency();
    _frequency += increment * steps;
    if(_frequency < 0)
      _frequency = 0;
    while(_frequency > MAX_FREQUENCY)
      _frequency -= MAX_FREQUENCY;
  }

  void step_phase(int steps){
    _phase += steps;
    while(_phase < 0)
      _phase += MAX_PHASE;
    while(_phase > MAX_PHASE)
      _phase -= MAX_PHASE;
  }

  void step_step(int steps){
    _step += steps;
    if(_step < 0)
      _step = 0;
    while(_step > MAX_STEP)
      _step -= MAX_STEP;
  }

  void toggle_state(AudioGroup **groups, int num_groups=3){
    switch(_state){
      case STATE_MUTED:
        _state = STATE_NORMAL;
        break;
      case STATE_NORMAL:
        _state = STATE_SOLO;
        for(int i = 0; i < num_groups; i++)
          if(_id != groups[i]->_id){
            groups[i]->_state = AudioGroup::STATE_MUTED;
            groups[i]->update_generator();
          }
        break;
      case STATE_SOLO:
        _state = STATE_MUTED;
        break;
    }
  }

  void update_generator(){
    switch(_state){
      case STATE_NORMAL:
      case STATE_SOLO:
        _generator->setFrequency((MD_AD9833::channel_t)0, _frequency / 10.0);
        _generator->setPhase((MD_AD9833::channel_t)0, _phase);
        break;
      case STATE_MUTED:
        _generator->setFrequency((MD_AD9833::channel_t)0, SILENT_FREQ);
        break;
    }
  }

  void decimalize(long value, char *buffer){
    long main = value / 10L;
    int dec = value % 10L;
    sprintf(buffer, "%ld.%d", main, dec);
  }

  void show_right_aligned(byte col, byte row, const char *buffer, byte max_width){
    byte width = strlen(buffer);
    if(width <= max_width){
      byte diff = max_width - width;
      for(byte i = 0; i < diff; i++){
        _lcd->setCursor(col++, row);
        _lcd->write(' ');
      }
    }
    _lcd->setCursor(col, row);
    _lcd->write(buffer);
  }

  void show_centered(byte col, byte row, const char *buffer, byte max_width){
    byte width = strlen(buffer);
    if(width <= max_width){
      col += (max_width - width) / 2;
    }
    _lcd->setCursor(col, row);
    _lcd->write(buffer);
  }

  // long represents a value in 1/10ths
  void show_fixed_point_long(long value, byte col, byte row, char *buffer, byte max_width){
    decimalize(value, buffer);
    show_right_aligned(col, row, buffer, max_width);
  }

  void show_frequency(byte col, byte row, char *buffer, byte max_width){
    show_fixed_point_long(_frequency, col, row, buffer, max_width);
  }

  // returns a step number from 0 to 4 into a step frequency in 1/10th Hz multiples 1 10 100 1000 10000
  long step_to_frequency(){
    switch(_step){
      case 0:
        return 1L;
      case 1:
        return 10L;
      case 2:
        return 100L;
      case 3:
        return 1000L;
      case 4:
        return 10000L;
    }
	return 10L;
}

  void show_step(byte col, byte row, char *buffer, byte max_width){
    long frequency = step_to_frequency();
    show_fixed_point_long(frequency, col, row, buffer, max_width);
  }

  void show_phase(byte col, byte row, char *buffer, byte max_width){
    show_fixed_point_long(_phase, col, row, buffer, max_width-1);
    _lcd->write(223);
  }

  void show_state(byte col, byte row, byte max_width){
    switch(_state){
      case STATE_NORMAL:
        show_centered(col, row, "Norm", max_width);
        break;
      case STATE_MUTED:
        show_centered(col, row, "Mute", max_width);
        break;
      case STATE_SOLO:
        show_centered(col, row, "Solo", max_width);
        break;
    }
  }

  // 0=top, 1=middle, 2=bottom
  void show_sep(){
    _lcd->setCursor(6, 0);
    _lcd->write(1);
    _lcd->setCursor(13, 0);
    _lcd->write(1);
    _lcd->setCursor(6, 1);
    _lcd->write(2);
    _lcd->setCursor(13, 1);
    _lcd->write(2);
    _lcd->setCursor(6, 2);
    _lcd->write(3);
    _lcd->setCursor(13, 2);
    _lcd->write(3);
    _lcd->setCursor(6, 4);
    _lcd->write(3);
    _lcd->setCursor(13, 4);
    _lcd->write(3);
  }

  void show(bool last_group=false){
    char buffer[10];
    byte col = (_id - 1) * GROUP_WIDTH;
    byte max_width = GROUP_WIDTH-1;

    show_frequency(col, 0, buffer, max_width);
    show_step(col, 1, buffer, max_width);
    show_phase(col, 2, buffer, max_width);
    show_state(col, 3, max_width);
  }

  static const long MAX_FREQUENCY = 125000000L;
  static const int MAX_STEP = 4;
  static const int MAX_PHASE = 3600;
  static const int SILENT_FREQ = 0;

  static const int GROUP_WIDTH = 7;

  static const int STATE_NORMAL = 0;
  static const int STATE_MUTED = 1;
  static const int STATE_SOLO = 2;

private:
  hd44780_I2Cexp *_lcd;
  MD_AD9833 *_generator;
  byte _id;
  long _frequency; // in 1/10 Hz
  byte _step;      // in 1/10 Hz
  int _phase;      // in 1/10 degree
  byte _state;

};

#endif

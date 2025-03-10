#include <Arduino.h>
#include "led_handler.h"

LEDHandler::LEDHandler(int num_leds, const int *led_pins, const int *intensity, int show_time, int blank_time){
	_led_pins = led_pins;
	_num_leds = num_leds;
	_intensity = intensity;
	_show_time = show_time;
	_blank_time = blank_time;

	_style = 0;
	_frame = 0;
	_next_frame = 0;
	_num_frames = 0;
	_num_states = 0;
	_blanking = false;
	_active = 0; 		 // virtual current active led or other state
	_enabled = NULL;	 // array of enabled leds for animation matching LED count
}

void LEDHandler::begin(unsigned long time, int style, int show_time, int blank_time, bool *enabled){
	_show_time = show_time ? show_time : DEFAULT_SHOW_TIME;
	_blank_time = blank_time ? blank_time : DEFAULT_BLANK_TIME;

	// if enabled specified, disable it if none of the LEDs are enabled
	bool detect = false;
	for(int i = 0; i < _num_leds; i++){
		if(enabled[i]){
			detect = true;
			break;
		}
	}
	_enabled = detect ? enabled : NULL;

	_style = style;

	// with one LED the random style cannot be used
	if(_num_leds < 2)
		_style &= ~STYLE_RANDOM;

	// with an odd number of LEDs the mirror style cannot be used
	if(_num_leds % 2)
		_style &= ~STYLE_MIRROR;

	_frame = -1;
	_next_frame = time;

	_num_frames = _num_leds;
	if(_style & STYLE_BLANKING)
		_num_frames *= 2;
	if(_style & STYLE_MIRROR)
		_num_frames /= 2;

	_num_states = _num_leds;
	if(_style & STYLE_MIRROR)
		_num_states /= 2;

	_active = -1;
	_blanking = false;
}

void LEDHandler::deactivate_led(int virtual_pin, bool mirror){
	digitalWrite(_led_pins[virtual_pin], LOW);
	if(mirror)
		deactivate_led(virtual_pin + (_num_leds / 2));
}

void LEDHandler::activate_led(int virtual_pin, bool mirror){

	if(_intensity[virtual_pin] == 0)
		digitalWrite(_led_pins[virtual_pin], HIGH);
	else
		analogWrite(_led_pins[virtual_pin], _intensity[virtual_pin]);
	if(mirror)
		activate_led(virtual_pin + (_num_leds / 2));
}

void LEDHandler::step(unsigned long time){
	if(time >= _next_frame){
	if(_active != -1)
		deactivate_led(_active, _style & STYLE_MIRROR);

	_frame++;
	if(_frame >= _num_frames)
		_frame = 0;

	bool blanking_period = (_style & STYLE_BLANKING) && (_frame % 2);
	if(!blanking_period){
		if(_style & STYLE_RANDOM){
			int r; // skip prevention logic here if nothing enabled
			while( (r = random(_num_states)) == _active )
				;
			_active = r;
		} else {
			_active++;
			if(_active >= _num_states)
				_active = 0;
			while(_enabled && !_enabled[_active]){
				_active++;
				if(_active >= _num_states)
					_active = 0;
			}
		}
		if(_enabled == NULL || _enabled[_active])
			activate_led(_active, _style & STYLE_MIRROR);
	}

	_next_frame = time + (blanking_period ? _blank_time : _show_time);
	}
}

// states points to a bool array with the [0] position 'any' ignored
void LEDHandler::activate_leds(const volatile bool * states, bool mirror){
	int effective_pins = mirror ? (_num_leds / 2) : _num_leds;
	for(int virtual_pin = 0; virtual_pin < effective_pins; virtual_pin++){
		if(states[1 + virtual_pin])
			activate_led(virtual_pin, mirror);
		else
			deactivate_led(virtual_pin, mirror);
	}
}

// turn all on or off
void LEDHandler::activate_all(bool state, bool mirror){

	int effective_pins = mirror ? (_num_leds / 2) : _num_leds;



	for(int virtual_pin = 0; virtual_pin < effective_pins; virtual_pin++){

		if(state){


			activate_led(virtual_pin, mirror);
		} else {

			deactivate_led(virtual_pin, mirror);
		}
	}


}

void LEDHandler::deactivate_leds(bool mirror){
	int effective_pins = mirror ? (_num_leds / 2) : _num_leds;
	for(int virtual_pin = 0; virtual_pin < effective_pins; virtual_pin++)
		deactivate_led(virtual_pin, mirror);
}

void LEDHandler::flash_leds(bool mirror, long time){
	long effective_time = time ? time : DEFAULT_FLASH_TIME;
	int effective_pins = mirror ? (_num_leds / 2) : _num_leds;

	for(int virtual_pin = 0; virtual_pin < effective_pins; virtual_pin++){
		digitalWrite(_led_pins[virtual_pin], HIGH);
		if(mirror)
			digitalWrite(_led_pins[virtual_pin] + (_num_leds / 2), HIGH);
	}

	delay(effective_time);

	for(int virtual_pin = 0; virtual_pin < effective_pins; virtual_pin++){
		digitalWrite(_led_pins[virtual_pin], LOW);
		if(mirror)
			digitalWrite(_led_pins[virtual_pin] + (_num_leds / 2), LOW);
	}
}

// flash the LEDs at full intensity, non-blocking
void LEDHandler::begin_flash(bool mirror, int on_time){
	_flash_mirror = mirror;
	_flash_pins = mirror ? (_num_leds / 2) : _num_leds;

	for(int virtual_pin = 0; virtual_pin < _flash_pins; virtual_pin++){
		digitalWrite(_led_pins[virtual_pin], HIGH);
		if(_flash_mirror)
			digitalWrite(_led_pins[virtual_pin] + (_num_leds / 2), HIGH);
	}

	_flash_on_time = on_time != 0 ? on_time : DEFAULT_FLASH_TIME;
	_flash_state = true;
	_next_flash_change = millis() + _flash_on_time;
}

// returns true to keep going
bool LEDHandler::step_flash(unsigned long time){
	if(!_flash_state)
		// already done flashing
		return false;

	if(time < _next_flash_change)
		// not time yet to stop flash
		return true;

	// currently on, turn off
	for(int virtual_pin = 0; virtual_pin < _flash_pins; virtual_pin++){
		digitalWrite(_led_pins[virtual_pin], LOW);
		if(_flash_mirror)
			digitalWrite(_led_pins[virtual_pin] + (_num_leds / 2), LOW);
	}
	_flash_state = false;
	return false;
}

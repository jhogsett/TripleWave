#include <Wire.h>
#include <hd44780.h>											 // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <MD_AD9833.h>
#include "led_handler.h"
#include "generator_handler.h"

#define DEFAULT_SILENT_FREQ 10000000L

GeneratorHandler::GeneratorHandler(hd44780_I2Cexp *lcd, MD_AD9833 *generator, LEDHandler *handler, byte id, long frequency, byte step, int phase, MD_AD9833::mode_t mode, byte state){
	_lcd = lcd;
	_generator = generator;
	_handler = handler;
	_id = id;
	_frequency = frequency;
	_step = step;
	_phase = phase;
	_mode = mode;
	_state = state;
	_silent_freq = DEFAULT_SILENT_FREQ;

	_generator->begin();
	_generator->setMode(_mode);
	_generator->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void GeneratorHandler::silence(){
	_generator->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void GeneratorHandler::step_frequency(int steps){
	int increment = step_to_frequency();
	_frequency += increment * steps;
	if(_frequency < 0)
		_frequency = 0;
	while(_frequency > MAX_FREQUENCY)
		_frequency -= MAX_FREQUENCY;

}

void GeneratorHandler::step_phase(int steps){
	_phase += steps;
	while(_phase < 0)
		_phase += MAX_PHASE;
	while(_phase > MAX_PHASE)
		_phase -= MAX_PHASE;
}

void GeneratorHandler::step_step(int steps){
	_step += steps;
	if(_step < 0)
		_step = 0;
	while(_step > MAX_STEP)
		_step -= MAX_STEP;
}

void GeneratorHandler::switch_to_normal(byte old_state, GeneratorHandler **handlers, int num_handlers){
	_state = STATE_NORMAL;
	if(old_state == STATE_SYNC){
		for(int i = 0; i < num_handlers; i++){
			if(_id != handlers[i]->_id){
				handlers[i]->_state = GeneratorHandler::STATE_NORMAL;
				handlers[i]->update_generator();
				handlers[i]->show();
			}
		}
	}
}

void GeneratorHandler::switch_to_muted(byte old_state, GeneratorHandler **handlers, int num_handlers){
	_state = STATE_MUTED;
	if(old_state == STATE_SYNC){
		for(int i = 0; i < num_handlers; i++){
			if(_id != handlers[i]->_id){
				handlers[i]->_state = GeneratorHandler::STATE_MUTED;
				handlers[i]->update_generator();
				handlers[i]->show();
			}
		}
	}
}

void GeneratorHandler::switch_to_solo(byte old_state, GeneratorHandler **handlers, int num_handlers){
	_state = STATE_SOLO;
	for(int i = 0; i < num_handlers; i++){
		if(_id != handlers[i]->_id){
			handlers[i]->_state = GeneratorHandler::STATE_MUTED;
			handlers[i]->update_generator();
			handlers[i]->show();
		}
	}
}

void GeneratorHandler::switch_to_sync(byte old_state, GeneratorHandler **handlers, int num_handlers){
	_state = STATE_SYNC;
	for(int i = 0; i < num_handlers; i++){
		if(_id != handlers[i]->_id){
			handlers[i]->_state = GeneratorHandler::STATE_SYNC;
			handlers[i]->update_generator();
			handlers[i]->show();
		}
	}
}

void GeneratorHandler::toggle_state(GeneratorHandler **handlers, int num_handlers){
	byte old_state = _state;
	switch(_state){
		case STATE_MUTED:
			switch_to_normal(old_state, handlers, num_handlers);
			break;
		case STATE_NORMAL:
			switch_to_solo(old_state, handlers, num_handlers);
			break;
		case STATE_SOLO:
			switch_to_sync(old_state, handlers, num_handlers);
			break;
		case STATE_SYNC:
			switch_to_muted(old_state, handlers, num_handlers);
			break;
	}
}

void GeneratorHandler::update_generator(){
	switch(_state){
		case STATE_NORMAL:
		case STATE_SYNC:
		case STATE_SOLO:
			if(_frequency != _last_set_freq){
				_generator->setFrequency((MD_AD9833::channel_t)0, _frequency / 10.0);
				_last_set_freq = _frequency;
			}
			if(_phase != _last_set_phase){
				_generator->setPhase((MD_AD9833::channel_t)0, _phase);
				_last_set_phase = _phase;
			}
		break;
		case STATE_MUTED:
			_generator->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
			_last_set_freq = _silent_freq;
			break;
	}
}

void GeneratorHandler::decimalize(long value, char *buffer){
	long main = value / 10L;
	int dec = value % 10L;
	sprintf(buffer, "%ld.%d", main, dec);
}

void GeneratorHandler::show_right_aligned(byte col, byte row, const char *buffer, byte max_width){
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

void GeneratorHandler::show_centered(byte col, byte row, const char *buffer, byte max_width){
	byte width = strlen(buffer);
	if(width <= max_width){
		col += (max_width - width) / 2;
	}
	_lcd->setCursor(col, row);
	_lcd->write(buffer);
}

// long represents a value in 1/10ths
void GeneratorHandler::show_fixed_point_long(long value, byte col, byte row, char *buffer, byte max_width){
	decimalize(value, buffer);
	show_right_aligned(col, row, buffer, max_width);
}

void GeneratorHandler::show_frequency(byte col, byte row, char *buffer, byte max_width){
	show_fixed_point_long(_frequency, col, row, buffer, max_width);
}

// returns a step number from 0 to 4 into a step frequency in 1/10th Hz multiples 1 10 100 1000 10000
long GeneratorHandler::step_to_frequency(){
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

void GeneratorHandler::show_step(byte col, byte row, char *buffer, byte max_width){
	long frequency = step_to_frequency();
	show_fixed_point_long(frequency, col, row, buffer, max_width);
}

void GeneratorHandler::show_phase(byte col, byte row, char *buffer, byte max_width){
	show_fixed_point_long(_phase, col, row, buffer, max_width-1);
	_lcd->write(223);
}

void GeneratorHandler::show_state(byte col, byte row, byte max_width){
	switch(_state){
		case STATE_NORMAL:
			show_centered(col, row, "Norm", max_width);
			break;
		case STATE_MUTED:
			show_centered(col, row, "Mute", max_width);
			break;
		case STATE_SYNC:
			show_centered(col, row, "Sync", max_width);
			break;
		case STATE_SOLO:
			show_centered(col, row, "Solo", max_width);
			break;
	}
}

// 0=top, 1=middle, 2=bottom
void GeneratorHandler::show_sep(){
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

void GeneratorHandler::show_led_per_state(){
	switch(_state){
		case STATE_NORMAL:
			_handler->activate_led(_id);
			break;
		case STATE_MUTED:
			_handler->deactivate_led(_id);
		break;
		case STATE_SYNC:
			_handler->activate_led(_id);
			break;
		case STATE_SOLO:
			_handler->activate_led(_id);
			break;
	}
}

void GeneratorHandler::show(bool last_handler){
	char buffer[10];
	byte col = (_id) * HANDLER_WIDTH;
	byte max_width = HANDLER_WIDTH-1;

	show_frequency(col, 0, buffer, max_width);
	show_step(col, 1, buffer, max_width);
	show_phase(col, 2, buffer, max_width);
	show_state(col, 3, max_width);
	show_led_per_state();
}

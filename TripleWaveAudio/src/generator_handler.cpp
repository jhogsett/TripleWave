#include <Wire.h>
#include <hd44780.h>											 // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <MD_AD9833.h>
#include "generator_handler.h"

GeneratorHandler::GeneratorHandler(hd44780_I2Cexp *lcd, MD_AD9833 *generator, byte id, long frequency, byte step, int phase, byte state){
	_lcd = lcd;
	_generator = generator;
	_id = id;
	_frequency = frequency;
	_step = step;
	_phase = phase;
	_state = state;
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

void GeneratorHandler::toggle_state(GeneratorHandler **handlers, int num_handlers){
switch(_state){
	case STATE_MUTED:
	_state = STATE_NORMAL;
	break;
	case STATE_NORMAL:
	_state = STATE_SOLO;
	for(int i = 0; i < num_handlers; i++)
		if(_id != handlers[i]->_id){
		handlers[i]->_state = GeneratorHandler::STATE_MUTED;
		handlers[i]->update_generator();
		}
	break;
	case STATE_SOLO:
	_state = STATE_MUTED;
	break;
}
}

void GeneratorHandler::update_generator(){
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

void GeneratorHandler::show(bool last_handler){
	char buffer[10];
	byte col = (_id - 1) * HANDLER_WIDTH;
	byte max_width = HANDLER_WIDTH-1;

	show_frequency(col, 0, buffer, max_width);
	show_step(col, 1, buffer, max_width);
	show_phase(col, 2, buffer, max_width);
	show_state(col, 3, max_width);
}

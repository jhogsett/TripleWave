#ifndef __GENERATOR_HANDLER_H__
#define __GENERATOR_HANDLER_H__

// #include <Wire.h>
// #include <hd44780.h>											 // main hd44780 header
// #include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
// #include <MD_AD9833.h>

class GeneratorHandler
{
public:
	GeneratorHandler(hd44780_I2Cexp *lcd, MD_AD9833 *generator, byte id, long frequency, byte step, int phase, byte state);
	void step_frequency(int steps);
	void step_phase(int steps);
	void step_step(int steps);
	void toggle_state(GeneratorHandler **handlers, int num_handlers=3);
	void update_generator();
	void decimalize(long value, char *buffer);
	void show_right_aligned(byte col, byte row, const char *buffer, byte max_width);
	void show_centered(byte col, byte row, const char *buffer, byte max_width);
	void show_fixed_point_long(long value, byte col, byte row, char *buffer, byte max_width);
	void show_frequency(byte col, byte row, char *buffer, byte max_width);
	long step_to_frequency();
	void show_step(byte col, byte row, char *buffer, byte max_width);
	void show_phase(byte col, byte row, char *buffer, byte max_width);
	void show_state(byte col, byte row, byte max_width);
	void show_sep();
	void show(bool last_handler=false);

	static const long MAX_FREQUENCY = 125000000L;
	static const int MAX_STEP = 4;
	static const int MAX_PHASE = 3600;
	static const int SILENT_FREQ = 0;
	static const int HANDLER_WIDTH = 7;
	static const int STATE_NORMAL = 0;
	static const int STATE_MUTED = 1;
	static const int STATE_SOLO = 2;

private:
	hd44780_I2Cexp *_lcd;
	MD_AD9833 *_generator;
	byte _id;
	long _frequency; // in 1/10 Hz
	byte _step;			// in 1/10 Hz
	int _phase;			// in 1/10 degree
	byte _state;
};

#endif

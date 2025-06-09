
///////////////////////////////////////////////////////////////////////////////


#include "avr_io_bitfields.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

//////////////////

// Motors pins.
#define PG 2 // INT2
#define PWM 3 // OC2B
#define DIR 4
#define MAYBE_BREAK 5

#define SW_EN 9
#define N_PB_EN 10
#define N_PB_SPEED_P 11 // Red.
#define N_PB_SPEED_N 12 // Blue.

#define LED_DEBUG LED_BUILTIN

//////////////////

#define EFF_STEP 10
#define EFF_START 10

//////////////////

typedef i32 pulses_t;
static volatile pulses_t pos = 0;

enum dir_t {
	CW = +1,
	CCW = -1
};
static volatile dir_t dir = CW;


static void set_abs_eff(u8 percents) {
	tc2.r.ocrb = percents;
}
static void set_dir(dir_t d) {
	dir = d;
	digitalWrite(DIR, dir == CW);
}

static void pos_pulse() {
	pos++;
}

static void set_eff(i8 eff) {
	if(eff > 0){
		set_dir(CW);
		set_abs_eff(eff);
	}else{
		set_dir(CCW);
		set_abs_eff(-eff);
	}
}


///////////////////////////////////////////////////////////////////////////////

// 10kHz
ISR(TIMER2_COMPA_vect) {
}

static i8 eff = EFF_START;

static float C_rpm;

u8 read_id() {
	pinMode(A5, OUTPUT);
	digitalWrite(A5, 0);
	for(i8 pin = A4; pin >= A0; pin--){
		pinMode(pin, INPUT_PULLUP);
	}
	u8 id = 0;
	for(i8 pin = A4; pin >= A0; pin--){
		id <<= 1;
		id |= !digitalRead(pin);
	}
	return id;
}

void setup() {
	Serial.begin(115200);
	
	pinMode(LED_DEBUG, OUTPUT);
	digitalWrite(LED_DEBUG, 0);

	
	pinMode(SW_EN, INPUT_PULLUP);
	pinMode(N_PB_EN, INPUT_PULLUP);
	pinMode(N_PB_SPEED_P, INPUT_PULLUP);
	pinMode(N_PB_SPEED_N, INPUT_PULLUP);
	
	// Connect A2 to A5 to have smaller motor selected.
	u8 id = read_id();
	if(id < 3){
		// Larger BLDC.
		pinMode(PG, INPUT_PULLUP);
		C_rpm = float(60)*1000/6;
	}else{
		// Smaller BLDC.
		pinMode(PG, INPUT);
		C_rpm = float(60)*1000/9;
	}
	// ISR on pin PG pin.
	attachInterrupt(digitalPinToInterrupt(PG), pos_pulse, RISING);
	
	pinMode(DIR, OUTPUT);
	set_dir(CW);

	pinMode(PWM, OUTPUT);
	digitalWrite(PWM, 1); // Off.
	// Cleanup Arduino stuff.
	tc2.r.tccra = 0;
	tc2.r.tccrb = 0;
	irq.timsk[2] = 0;
	tc2.f.comb = 0b11; // Inverted output on OC2B.
	tc2.r.ocrb = 0; // Off.
	tc2.r.ocra = 100; // Before setting mode.
	// Phase Correct PWM [0,OCRA] aka mode 5.
	tc2.f.wgm0 = 1;
	tc2.f.wgm1 = 0;
	tc2.f.wgm2 = 1;
	// For 10kHz PWM.
	tc2.f.cs = 0b010; // CLK/8
	// Enable IRQ for Watchdog.
	irq.timsk2.ociea = 1;
	
	set_eff(eff);
}

typedef unsigned long millis_t;

void loop() {
	// Control speed+dir with PBs.
	// Step eff in range -100 <-> 0 <-> +100
	static bool prev_p;
	static bool prev_n;
	bool curr_p = !digitalRead(N_PB_SPEED_P);
	bool curr_n = !digitalRead(N_PB_SPEED_N);
	bool re_p = curr_p && !prev_p;
	bool re_n = curr_n && !prev_n;
	
	bool en = digitalRead(SW_EN) || !digitalRead(N_PB_EN);
	
	digitalWrite(LED_DEBUG, en);
	
	if(re_p){
		eff = min(eff+EFF_STEP, 100);
	}else if(re_n){
		eff = max(eff-EFF_STEP, -100);
	}
	if(en){
		set_eff(eff);
	}else{
		set_eff(0);
	}
	
	
	prev_p = curr_p;
	prev_n = curr_n;
	
	delay(10); // Debouncing.
	
	static millis_t prev_t;
	static pulses_t prev_pos;
	pulses_t curr_pos = pos;
	millis_t curr_t = millis();
	
	
	pulses_t dp = curr_pos - prev_pos;
	millis_t dt = curr_t - prev_t;
	
	int rpm = C_rpm*dp/dt;
	Serial.print(pos);
	Serial.print(" cnt\t");
	Serial.print(rpm);
	Serial.print(" rpm");
	Serial.println();
	
	
	prev_pos = curr_pos;
	prev_t = curr_t;
}

#include "SerialHandler.h"

//externs
extern PeachyStepper g_Stepper;
extern PeachyPrintState g_PrintState;
extern uint8_t g_Serial_starved;
extern uint16_t g_Serial_starved_count;
extern double g_layer_float;

//globals
uint8_t g_dripper_state = OFF;
uint8_t g_layer_state = OFF;

void serialDrip(uint8_t state){
	g_dripper_state=state;
}

void serialLayer(uint8_t state){
	g_layer_state=state;
}

void serialPrintDone(){
	//dunno what to do here? return to zero?
	findUpperLimit();
	g_Stepper.stop();
	g_PrintState.stop();
}

void nextLayer(){
	g_layer_float-=STEPS_PER_LAYER;
	g_PrintState.updateHeightSteps(PRINT_STATE_PRINTING,(int32_t)g_layer_float); //once it's ready also trigger end of current layer
	g_PrintState.externalTrigger(PRINT_STATE_PRINTING);
}

void sendHello(){
	Serial.write("OK\n");
}

void handleChar(){
	uint8_t serial_data;

	if (Serial.available()){
		serial_data=Serial.read();
		switch(serial_data){
			case '7':
				//DRIP ON
				//serialDrip(ON);
				break;
			case '8':
				//DRIP OFF
				//serialDrip(OFF);
				break;
			case 'S':
				//LAYER STARTED
				serialLayer(ON);
				break;
			case 'E':
				//LAYER ENDED
				serialLayer(OFF);
				nextLayer();
				//break;
			case 'H':
				//NEXT LAYER
				//nextLayer();
				break;
			case 'D':
				//it gave me the D .... -_-
				sendHello();
				break;
			case 'Z':
				serialPrintDone();
				//PRINT ENDED
				break;
		}
	}
	else
		g_Serial_starved_count=0;
}

// This gets called if there are SERIAL_NUMBYTES_TRIGGER characters in the buffer
// ... or it would if the arduino documentation was accurate
void serialEvent(){

	do{
		handleChar();

		//Check for roll over before subtracting from the starve counter
		if (g_Serial_starved_count > SERIAL_BYTE_TICKS)
			g_Serial_starved_count -= SERIAL_BYTE_TICKS;
		else{
			g_Serial_starved_count=0;
			g_Serial_starved=0; //Exit override case if timer interrupt doesn't clear this first
		}
	}
	while(g_Serial_starved); // Serial_starved controlled by the timer interrupt
}

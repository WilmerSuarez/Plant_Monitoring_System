/*********************************************
  File Name            : "FSM.h" 
  Title                : FSM Header File
  Date                 : 04/09/2018  
  Version              : 1.0 
  Target MCU           : ATMEGA128A
  Author               : Wilmer Suarez 
  DESCRIPTION 
  This header file includes the external 
  function declerations used for the FSM.
  The file also declares the states, keys,
  and present_state.
*********************************************/

// ---------- FSM States ---------- //
typedef enum{idle, changeTime, changeAlarm0, dispCO2} state ;

// ---------- Keys on the keypad ---------- //
typedef enum {zero, one, two, three, four, five, six, seven, eight, nine, setTime, setAlarm0, back, tempChange, del, co2, eol} key ;

// Functions to implement the task(s) associated with a state transition.
// All functions must have the same signature (parameters and return type)
extern void idle_fn(key keyVal);
extern void fsm(state ps, key keyval);       // FSM
extern void changeTime_fn(key keyVal);       // Change the time in the DS1306
extern void changeAlarm0_fn(key keyVal);     // Change the Alarm 0 in the DS1306
extern void back_fn(key keyVal);             // Backspace
extern void dispCO2_fn(key keyVal);          // Displays the measuremnt of CO2
extern void display();                       // Helper function for dispCO2_fn
extern void int2Hex(unsigned int result);    // Helper function for dispCO2_fn
extern void error_fn(key keyVal);            // Error Message

// --- Present state variable declereation --- //
extern state present_state;
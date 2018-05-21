/****************************************************************
 File Name            : "fsm_ui.c" 
 Title                : Table Driven FSM
 Date                 : 04/09/2018  
 Version              : 1.0 
 Target MCU           : ATMEGA128A
 Author               : Wilmer Suarez 
 DESCRIPTION 
 This file contains the definition of the fsm function and the 
 transistion arrays that are traversed whenever their is a 
 keypad press.
****************************************************************/    

// ----- Include Files ----- //
#include "header.h"     // Includes the ATmega128 Definitions, Macros, and Instinsic functions
#include <iom128.h> 
#include "FSM.h"

// Declare type task_fn_ptr as a pointer to a task function
typedef void (* task_fn_ptr) (key keyVal);

// A structure transition represents one row of a state transition table
// it has a field for the input key value, the next state, and a pointer
// to the task function.
typedef struct {
  key keyval;
  state next_state;
  task_fn_ptr tf_ptr;
} transition;

// The state transition table consists of an array of arrays of structures.
// Each array of structures corresponds to a particular present state value.
// Each structure in such an array corresponds to a transition from the
// state for a given input value and the task function associated with the
// transition. Accordingly, each structure in an array has fields
// corresponding to an input value, the next state for this input value,
// and a pointer to the function task for this input value.
// The last transition structure in each array has a keyval field value
// of eol. This is a default value meaning any key value that has not
// been explcitly listed in a previous transition structure in the array.

const transition idle_transitions [] = {           // subtable for idle state
//  KEY INPUT   NEXT_STATE    FUNCTION
    {setTime,   changeTime,   changeTime_fn},
    {setAlarm0, changeAlarm0, changeAlarm0_fn},
    {co2,       dispCO2,      dispCO2_fn},
    {eol,       idle,         error_fn}
};
    
const transition changeTime_transitions [] = {     // subtable for setTime state
//  KEY INPUT   NEXT_STATE    FUNCTION
    {zero,      changeTime,  changeTime_fn},
    {one,       changeTime,  changeTime_fn},
    {two,       changeTime,  changeTime_fn},
    {three,     changeTime,  changeTime_fn},
    {four,      changeTime,  changeTime_fn},
    {five,      changeTime,  changeTime_fn},
    {six,       changeTime,  changeTime_fn},
    {seven,     changeTime,  changeTime_fn},
    {eight,     changeTime,  changeTime_fn},
    {nine,      changeTime,  changeTime_fn},
    {back,      idle,        idle_fn},
    {del,       changeTime,  back_fn},
    {eol,       changeTime,  error_fn}
};
    
const transition changeAlarm0_transitions [] = {   // subtable for set_temp state
//  KEY INPUT   NEXT_STATE     FUNCTION
    {zero,      changeAlarm0,  changeAlarm0_fn},
    {one,       changeAlarm0,  changeAlarm0_fn},
    {two,       changeAlarm0,  changeAlarm0_fn},
    {three,     changeAlarm0,  changeAlarm0_fn},
    {four,      changeAlarm0,  changeAlarm0_fn},
    {five,      changeAlarm0,  changeAlarm0_fn},
    {six,       changeAlarm0,  changeAlarm0_fn},
    {seven,     changeAlarm0,  changeAlarm0_fn},
    {eight,     changeAlarm0,  changeAlarm0_fn},
    {nine,      changeAlarm0,  changeAlarm0_fn}, 
    {back,      idle,          idle_fn},
    {del,       changeAlarm0,  back_fn},
    {eol,       changeAlarm0,  error_fn}
};   

const transition dispCO2_transitions [] = {        // subtable for dispCO2 state
//  KEY INPUT   NEXT_STATE     FUNCTION
    {back,      idle,          idle_fn},
    {eol,       dispCO2,       error_fn}
}; 
    
// The outer array is an array of pointers to an array of transition
// structures for each present state.
const transition * ps_transitions_ptr[4] = {
  idle_transitions,    
  changeTime_transitions,
  changeAlarm0_transitions, 
  dispCO2_transitions
};


/***********************************************************
 Function             : void fsm (state ps, key keyval)
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function searches an array of 'transistion' structures
 of the present state passed in for the keyvalue the user
 entered. 
 If the keyvalue is found, the associated function is 
 called. If it is not found or if an invalid key (eol) was
 pressed (respective of the present state) the assocaited
 error function is called. In both cases the present state
 is updated with the next state in the 'transition' struct.
***********************************************************/
void fsm (state ps, key keyval) {
  // Search for the transition struct array for the key value.
  int i = 0;
  for (i = 0; (ps_transitions_ptr[ps][i].keyval != keyval)
       && (ps_transitions_ptr[ps][i].keyval != eol); i++);

  // i now has the value of the index of the transition structure
  // corresponding to the current intput key value.
  
  // Make the present state equal to the next state value of the current
  // transition structure.
  present_state = ps_transitions_ptr[ps][i].next_state;
  
  // Call the task function pointed to by the task function pointer
  // of the current transition structure.
  ps_transitions_ptr[ps][i].tf_ptr(keyval);
}

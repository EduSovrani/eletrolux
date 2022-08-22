/**
* @file <question2.ino>
* @brief <Arduino Source File>
*
* <Question 2 - Eletrolux - Knowledge Assessment>
*
* @author <Eduardo Falchetti Sovrani>
*
*/

// ---------------------------------------
// DEFINES
// ---------------------------------------

// PINS INPUTS & OUTPUTS --------------------- 
#define BUTTON_PIN (2) 
#define LED_DEBUG  (13)  // For DEBUG

// BUTTON CONFIGURATION ---------------
#define DEBOUNCE         (100)    // (ms) - Prevent flickering when pressing or releasing the button
#define PROTECTION_TIME  (10000)  // (ms)

// STATE MACHINE - STATES ---------------
#define SM_STATE_OFF     (0)
#define SM_STATE_ON      (1)
#define SM_STATE_PROTECT (2)
#define SM_STATE_ERROR   (4)
#define SM_STATE_NUMBER  (5)   /* Total number of state */

// STATE MACHINE - EVENTS ----------------
#define SM_EVENT_START           (0)
#define SM_EVENT_BUTTON_PRESSED  (1)
#define SM_EVENT_BUTTON_RELEASED (2)
#define SM_EVENT_BUTTON_WAIT     (3)
#define SM_EVENT_ERROR           (4)
#define SM_EVENT_NUMBER          (5) /* Total number of events */

// STATE MACHINE - ERROS ----------------
#define SM_ERROR_NONE                     (0)  /* Flag no Error */
#define SM_ERROR_STATE_INDEX_OVERFLOW     (1)  /* Flag error at state index */
#define SM_ERROR_EVENT_INDEX_OVERFLOW     (2)  /* Flag error at event index */
#define SM_ERROR_EXECUTION                (4)  /* Flag error at state execution */

// ---------------------------------------
// STRUCTS
// ---------------------------------------

/* STATE MACHINE DATA STRUCT */
typedef struct{
   int sm_state_current; /* Current state */
   int sm_state_future;  /* Next state */
   int sm_flag_error;    /* Flag error */
   int sm_event_current; /* Current event */
} sm_data_t;

typedef struct{
   sm_data_t SM;               /* State Machine Data */
   int BUTTON_WAIT_TIME;       /* Button Wait Time */
   long BUTTON_WAIT_COUNTER;   /* Button Wait Time Counter */
   int BUTTON_WAIT_FLAG;       /* Flag to indicate when reset BUTTON_WAIT_COUNTER*/
} global_data_t;

// ---------------------------------------
// FUNCTION PROTOTYPE
// ---------------------------------------

/***********************************************************************************************************************
* Function  : (action) State machine action handler function
* Arguments : _sm_data - pointer to the state machine data structure
*             _params - pointer to the parameters
* Return    : (0 - Success) / (other - Error)
***********************************************************************************************************************/
typedef int (*sm_action)(sm_data_t * _sm_data, void * _params);

/* SM  Operation Functions */
void sm_button_init(sm_data_t * _sm_data);
int sm_button_event(sm_data_t * _sm_data, void * _object, int _event);
int sm_button_get_state(sm_data_t * _sm_data);

/* SM Transition Actions */
int button_no_action(sm_data_t * _sm_data, void * _params);
int button_wait_protect_action(sm_data_t * _sm_data, void * _params);

/* SM Call Events Functions */
void event_button_pressed(void);
void event_button_released(void);
void event_button_wait(void);
void event_button_init(void);
void event_button_off(void);

/* Button Functions */
void button_analysis(void);

// ---------------------------------------
// VARIABLES DECLARATION
// ---------------------------------------

// --------- STATE MACHINE VARIABLES ---------

/* SM STATE TRANSITION MAP */
static int sm_transition_map[SM_EVENT_NUMBER][SM_STATE_NUMBER] = {
/* STATE */
/* 0:SM_STATE_OFF,   1:SM_STATE_ON,    2:SM_STATE_PROTECT,  3:SM_STATE_ERROR */
/* EVENT */
/* 0:SM_EVENT_START */
   { SM_STATE_OFF,   SM_STATE_ON,      SM_STATE_OFF,      SM_STATE_OFF},
/* 1:SM_EVENT_BUTTON_PRESSED */
   { SM_STATE_ON,    SM_STATE_ON,      SM_STATE_PROTECT,  SM_STATE_ERROR},
/* 2:SM_EVENT_BUTTON_RELEASED */
   { SM_STATE_OFF,   SM_STATE_PROTECT, SM_STATE_PROTECT,  SM_STATE_ERROR},
/* 3:SM_EVENT_BUTTON_WAIT */  
   { SM_STATE_OFF,   SM_STATE_ON,      SM_STATE_PROTECT,  SM_STATE_ERROR},
/* 4:SM_EVENT_ERROR */  
   { SM_STATE_ERROR, SM_STATE_ERROR,   SM_STATE_ERROR,    SM_STATE_ERROR},
};

/* SM STATE TRANSITION ACTION */
static sm_action sm_transition_action[SM_EVENT_NUMBER][SM_STATE_NUMBER] = {
/* STATE */
/* 0:SM_STATE_OFF,     1:SM_STATE_ON,    2:SM_STATE_PROTECT,  3:SM_STATE_ERROR */
/* EVENT */
/* 0:SM_EVENT_START */
   { button_no_action, button_no_action, button_no_action,    button_no_action},
/* 1:STATEMACHINE_EVENT_RUN   */
   { button_no_action, button_no_action, button_no_action,    button_no_action},
/* 2:SM_EVENT_BUTTON_RELEASED */
   { button_no_action, button_no_action, button_no_action,    button_no_action},
/* 3:SM_EVENT_BUTTON_WAIT */  
   { button_no_action, button_no_action, button_wait_protect_action, button_no_action},
/* 4:SM_EVENT_ERROR */  
   { button_no_action, button_no_action, button_no_action,    button_no_action},
};

/* GLOBAL STATE MACHINE DATA */
global_data_t BUTTON;

// --------- BUTTON DEBOUNCE VARIABLES ---------
int buttonState = 0;
int buttonStateOld = 0;
long buttonTimer = 0;

// --------------------------------------- 
// SYSTEM INITIALIZATION CONFIGURATION
// ---------------------------------------
void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_DEBUG, OUTPUT);

  //Open the Sate Machine Button
  event_button_init();
  BUTTON.BUTTON_WAIT_TIME = PROTECTION_TIME;
  BUTTON.BUTTON_WAIT_COUNTER = 0;
  BUTTON.BUTTON_WAIT_FLAG = 0;

  //Serial.begin(9600);   // DEBUG
  //Serial.println("INIT");
}

// --------------------------------------- 
// SYSTEM LOOP
// ---------------------------------------

/***********************************************************************************************************************
* Function  : (loop) - Perform the state machine analysis and take an action
* Arguments : none
* Return    : none
***********************************************************************************************************************/
void loop() {
  switch(sm_button_get_state(&BUTTON.SM)){
    case SM_STATE_OFF:
      button_analysis();
      //Serial.println("OFF"); //DEBUG
    break;
    case SM_STATE_ON:
      button_analysis();
      //Serial.println("ON"); //DEBUG
    break;
    case SM_STATE_PROTECT:
      if(BUTTON.BUTTON_WAIT_FLAG == 1){
        if((millis() - BUTTON.BUTTON_WAIT_COUNTER) > BUTTON.BUTTON_WAIT_TIME){
          event_button_off();
          BUTTON.BUTTON_WAIT_FLAG = 0;
          //Serial.println("PROTECT1"); // DEBUG
        }
      }
      else{
        event_button_wait();
        //Serial.println("PROTECT2"); // DEBUG
      }
    break;
    case SM_STATE_ERROR:
      sm_button_init(&BUTTON.SM);
      //Serial.println("ERROR"); // DEBUG
    break;
  }
}

// -------------------------------------
// BUTTON ANALYSIS
// -------------------------------------

/***********************************************************************************************************************
* Function  : (button_analysis) - Perform the button state LOW and HIGH analysis with a debounce code and call a state
*                                 machine transition
* Arguments : none
* Return    : none
***********************************************************************************************************************/
void button_analysis(void)
{
  // read the state of the pushbutton value:
  buttonState = !digitalRead(BUTTON_PIN);
  
  if( (buttonState == LOW)&&(buttonStateOld == HIGH) ){
    if( (millis() - buttonTimer) > DEBOUNCE){
      buttonStateOld = LOW;
      buttonTimer = millis();
      event_button_released();
    }
  }
  else if( (buttonState == HIGH)&&(buttonStateOld == LOW) ){
    if( (millis() - buttonTimer) > DEBOUNCE){
      buttonStateOld = HIGH;
      buttonTimer = millis();
      event_button_pressed();
    }
  }
  else{
    buttonTimer = millis();
  }
}

// -------------------------------------
// STATE MACHINE CALL THE EVENTS
// -------------------------------------

/***********************************************************************************************************************
* Function  : (event_button_init) - Perform de state machine initialization
* Arguments : void
* Return    : void
***********************************************************************************************************************/
void event_button_init(void)
{
  sm_button_init(&BUTTON.SM);
}

/***********************************************************************************************************************
* Function  : (event_button_init) - Call the SM event to SM state OFF.
* Arguments : void
* Return    : void
***********************************************************************************************************************/
void event_button_off(void)
{
  sm_button_event(&BUTTON.SM, 0, SM_EVENT_START);
}

/***********************************************************************************************************************
* Function  : (event_button_init) - Call the SM event on button protect state to start a timer counter
* Arguments : void
* Return    : void
***********************************************************************************************************************/
void event_button_wait(void)
{
  static int protection_time = PROTECTION_TIME;
  sm_button_event(&BUTTON.SM, &protection_time, SM_EVENT_BUTTON_WAIT);
}

/***********************************************************************************************************************
* Function  : (event_button_init) - Call the SM event on button press
* Arguments : void
* Return    : void
***********************************************************************************************************************/
void event_button_pressed(void)
{
  sm_button_event(&BUTTON.SM, 0, SM_EVENT_BUTTON_PRESSED);
  digitalWrite(LED_DEBUG, HIGH);
}

/***********************************************************************************************************************
* Function  : (event_button_init) - Call the SM event on button released
* Arguments : void
* Return    : void
***********************************************************************************************************************/
void event_button_released(void)
{
  sm_button_event(&BUTTON.SM, 0, SM_EVENT_BUTTON_RELEASED);
  digitalWrite(LED_DEBUG, LOW);
}

// -------------------------------------
// STATE MACHINE TRANSITION ACTIONS
// -------------------------------------

/***********************************************************************************************************************
* Function  : (button_no_action) - Take no action
* Arguments : _sm_data - Pointer to the state machine data structure
*             _params - Pointer to a generic parameter structure to be passed to the action handler
* Return    : 0
***********************************************************************************************************************/
int button_no_action(sm_data_t * _sm_data, void * _params){
  return (0);
}

/***********************************************************************************************************************
* Function  : (button_wait_protect_action) - Set an action to be executed in 10 Seconds
* Arguments : _sm_data - Pointer to the state machine data structure
*             _params - Pointer to a generic parameter structure to be passed to the action handler
* Return    : 0
***********************************************************************************************************************/
int button_wait_protect_action(sm_data_t * _sm_data, void * _params){
  BUTTON.BUTTON_WAIT_TIME = *(int*)_params;
  BUTTON.BUTTON_WAIT_COUNTER = millis();
  BUTTON.BUTTON_WAIT_FLAG = 1;
  return (0);
}

// -------------------------------------
// STATE MACHINE OPERATION FUNCTIONS
// -------------------------------------

/***********************************************************************************************************************
* Function  : (sm_button_event) - Execute the state transition according "sm_transition_map" and the action 
                                  according "sm_transition_action".
* Arguments : _sm_data - Pointer to the state machine data structure
*             _object  - Pointer to a generic parameter structure to be passed to the action handler
*             _event   - Event index to be executed
*                        SM_EVENT_START           - Start Button SM and Reset SM from Erros
*                        SM_EVENT_BUTTON_PRESSED  - Button Pressed
*                        SM_EVENT_BUTTON_RELEASED - Button Released
*                        SM_EVENT_BUTTON_WAIT     - Button Wait to Off State
*                        SM_EVENT_ERROR           - Error Execution and stop SM
* Return    : Error flags of the state machine
*                        BIT0: Event index is out of bound
*                        BIT1: State index is out of bound
*                        BIT2: Execution error (value other than 0 returned from transition action)
***********************************************************************************************************************/
int sm_button_event(sm_data_t * _sm_data, void * _object, int _event)
{
  sm_action func_action;
  int       action_ret;
  
  /* Check the index of the state transition table*/
  if (SM_EVENT_NUMBER <= _event)
  {
      /* Event index is out of bound */
      _event                  =  SM_EVENT_ERROR;
      _sm_data->sm_flag_error |= SM_ERROR_EVENT_INDEX_OVERFLOW;
  }
  if (SM_STATE_NUMBER <= _sm_data->sm_state_current)
  {
    /* State index is out of bound */
    _event                     =  SM_EVENT_ERROR;
    _sm_data->sm_state_current =  SM_STATE_OFF;
    _sm_data->sm_flag_error    |= SM_ERROR_STATE_INDEX_OVERFLOW;
  }
  
  _sm_data->sm_event_current = _event;
  _sm_data->sm_state_future   = sm_transition_map[_event][_sm_data->sm_state_current];

  /* Get action function from action table and execute action */
  func_action = sm_transition_action[_event][_sm_data->sm_state_current];
  action_ret  = func_action(_sm_data, _object);

  /* If return value is not zero, set the Action Exception flag */
  if (action_ret != 0U)
  {
    _sm_data->sm_flag_error |= SM_ERROR_EXECUTION;
    //Serial.println("SM_ERROR"); //DEBUG 
  }
  _sm_data->sm_state_current = _sm_data->sm_state_future;

  return (_sm_data->sm_flag_error);
}

/***********************************************************************************************************************
* Function  : (sm_button_init) - Initializes state machine for button
* Arguments : _sm_data - Pointer to the state machine data structure
* Return    : none
***********************************************************************************************************************/
void sm_button_init(sm_data_t * _sm_data){
  _sm_data->sm_state_current    = SM_STATE_OFF;
  _sm_data->sm_state_future     = SM_STATE_OFF;
  _sm_data->sm_flag_error       = SM_ERROR_NONE;
  _sm_data->sm_event_current    = 0;
}

/***********************************************************************************************************************
* Function  : (sm_button_get_state) - Get state of state machine
* Arguments : _sm_data - Pointer to the state machine data structure
* Return    : The state machine state
***********************************************************************************************************************/
int sm_button_get_state(sm_data_t * _sm_data){
  return (_sm_data->sm_state_current);
}

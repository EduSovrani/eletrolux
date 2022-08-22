/**
* @file <question2.ino>
* @brief <Arduino Source File>
*
* <Question 4 - Eletrolux - Knowledge Assessment>
*
* @author <Eduardo Falchetti Sovrani>
*/

// ---------------------------------------
// INCLUDES
// ---------------------------------------
#include <avr/io.h>      // Contains all the I/O Register Macros
#include <util/delay.h>  // Generates a Blocking Delay
#include <avr/interrupt.h> // Contains all interrupt vectors

// ---------------------------------------
// DEFINES
// ---------------------------------------

// AVR ARDUINO UART CONFIGURATION ----------------------------------
#define F_CPU 16000000UL // Defining the CPU Frequency

#define BAUDRATE 9600 // Desired Baud Rate
#define BAUD_PRESCALER (((F_CPU / (BAUDRATE * 16UL))) - 1)

#define ASYNCHRONOUS (0<<UMSEL00) // USART Mode Selection

#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED // USART Parity Bit Selection

#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT      // USART Stop Bit Selection

#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT  // USART Data Bit Selection

#define RX_COMPLETE_INTERRUPT         (1<<RXCIE0)
#define DATA_REGISTER_EMPTY_INTERRUPT (1<<UDRIE0)

// PINS INPUTS --------------------- 
#define LED_DEBUG  (13)  // For DEBUG

// PLAYLOAD DATA SIZE ----------------------------------
#define MSG_DATA_SIZE sizeof(msg_data_t)

// PLAYLOAD DATA TYPES ----------------------------------
#define MSG_TYPE_VOLTAGE     (0xA)
#define MSG_TYPE_TEMPERATURE (0xB)
#define MSG_TYPE_SPEED       (0xC)

// ---------------------------------------
// STRUCTS
// ---------------------------------------
typedef struct         // Message playload struct with fixed size 
{
   uint8_t TYPE;       // define the message type 
   long DATA;          // Store the transmited data
} msg_data_t;

// ---------------------------------------
// VARIABLES DEFINITION
// ---------------------------------------

// Arduino Interruption Config
volatile uint8_t USART_ReceiveBuffer;             // Global UART Buffer

// Data Struct
volatile uint8_t RECEIVED_BUFFER[MSG_DATA_SIZE];  // Buffer to store the UART received bytes
volatile msg_data_t RECEIVED_DATA;                // Struct to receive data
uint16_t RECEIVED_BUFFER_FILL = 0;                // The RECEIVED_BUFFER fill

// --------------------------------------- 
// SYSTEM INITIALIZATION CONFIGURATION
// ---------------------------------------
void setup() {
  // DEBUG CODE;
  pinMode(LED_DEBUG, OUTPUT);

  // UART INITIALIZATION
  UART_init();
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
  // Playload Analysis
  playload_uart_analisys();
}

// ---------------------------------------
// PLAYLOAD ANALYSIS
// ---------------------------------------

/***********************************************************************************************************************
* Function  : (playload_uart_reception) - Store the received data in the buffer until MSG_DATA_SIZE
*                                       - That function must be placed in UART interruption
* Arguments : _uart_buffer - The Reception Byte From UART
* Return    : none
***********************************************************************************************************************/
void playload_uart_reception(uint8_t _uart_buffer){
  if(RECEIVED_BUFFER_FILL < MSG_DATA_SIZE){
    RECEIVED_BUFFER[RECEIVED_BUFFER_FILL] = _uart_buffer;
    RECEIVED_BUFFER_FILL++;
  }
}

/***********************************************************************************************************************
* Function  : (playload_uart_analisys) - Analyse if the RECEIVED_BUFFER received all bytes at RECEIVED_BUFFER_FILL.
*                                      - If RECEIVED_BUFFER_FILL == MSG_DATA_SIZE analyse the type of the received data 
*                                      and execute the users functions;
*                                      - If RECEIVED_BUFFER_FILL > 1 and RECEIVED_BUFFER_FILL < MSG_DATA_SIZE durring
*                                      1 second, the RECEIVED_BUFFER if cleaned. It's a protection. 
* Arguments : none
* Return    : none
***********************************************************************************************************************/
void playload_uart_analisys(void){
  static long TIMER_LIMIT = 0;
  static bool TIMER_LIMIT_FLAG = 0;  
  
  bool flag_msg_received = false;
  
  if(RECEIVED_BUFFER_FILL >= MSG_DATA_SIZE){                    // Receive and clean data Buffer
     memcpy(&RECEIVED_DATA, &RECEIVED_BUFFER, MSG_DATA_SIZE);
     memset(&RECEIVED_BUFFER, 0, MSG_DATA_SIZE);
     RECEIVED_BUFFER_FILL = 0;
     flag_msg_received = true;
     TIMER_LIMIT_FLAG = false;
  }
  else if(RECEIVED_BUFFER_FILL > 1){                            // Clean data Buffer if TIMER_LIMIT is reached when RECEIVED_BUFFER_FILL > 1 and RECEIVED_BUFFER_FILL
     if(TIMER_LIMIT_FLAG == false){
        TIMER_LIMIT_FLAG = true;
        TIMER_LIMIT = millis();
     }
     if((millis() - TIMER_LIMIT) > 1000)
     {
        memset(&RECEIVED_BUFFER, 0, MSG_DATA_SIZE);
        RECEIVED_BUFFER_FILL = 0;
        TIMER_LIMIT_FLAG = false;
     }
  }

  if(flag_msg_received == true){
    switch(RECEIVED_DATA.TYPE){                                 // Analyse the buffer type data and execute user action
      case MSG_TYPE_VOLTAGE:
      {
        float voltage = RECEIVED_DATA.DATA;
        UART_sendPolling('F');  // DEBUG FUNCTION
        UART_sendPolling('L');
        UART_sendPolling('O');
        UART_sendPolling('A');
        UART_sendPolling('T');
        //UART_sendPollingSize(&RECEIVED_BUFFER[2], 4);
        // USER PUT HERE A FUNCTION TO TAKE AN ACTION IN THE CODE
      }
      break;

      case MSG_TYPE_TEMPERATURE:
      {
        int temperature = RECEIVED_DATA.DATA;
        UART_sendPolling('T');  // DEBUG FUNCTION
        UART_sendPolling('E');
        UART_sendPolling('M');
        UART_sendPolling('P');
        //UART_sendPollingSize(&RECEIVED_BUFFER[2], 4);
        // USER PUT HERE A FUNCTION TO TAKE AN ACTION IN THE CODE        
      }
      break;
      
      case MSG_TYPE_SPEED:
      {
        uint16_t SPPED = RECEIVED_DATA.DATA;   
        UART_sendPolling('S');  // DEBUG FUNCTION
        UART_sendPolling('P');
        UART_sendPolling('E');
        UART_sendPolling('E');
        UART_sendPolling('P');
        //UART_sendPollingSize(&RECEIVED_BUFFER[2], 4);
        // USER PUT HERE A FUNCTION TO TAKE AN ACTION IN THE CODE 
      }
      break;

      default:
        UART_sendPolling('E');  // DEBUG FUNCTION
        UART_sendPolling('R');
        UART_sendPolling('R');
        UART_sendPolling('O');
        UART_sendPolling('R');
        // USER PUT HERE A FUNCTION TO TAKE AN ACTION IN THE CODE 
      break;
    }
    UART_sendPolling(13); // DEBUG FUNCTION
  }
  
}

// ---------------------------------------
// UART ARDUINO CONFIGURATION
// ---------------------------------------

/***********************************************************************************************************************
* Function  : (UART_sendPolling) - Send data by UART in pooling
* Arguments : _dataByte - byte to be send
* Return    : none
***********************************************************************************************************************/
void UART_sendPolling(uint8_t _dataByte){
  while (( UCSR0A & (1<<UDRE0)) == 0) {}; // Do nothing until UDR is ready
  UDR0 = _dataByte;
}

/***********************************************************************************************************************
* Function  : (UART_sendPolling) - Send data by UART in pooling
* Arguments : _dataByte - Pointer to data to be send
*             _size - size of the data
* Return    : none
***********************************************************************************************************************/
void UART_sendPollingSize(uint8_t *_buf, uint16_t _size)
{
  for (int i = 0; i <= _size; i++) {
    UDR0 = *_buf;  //put character into send buffer
    _buf++;
    while (!(UCSR0A & (1<<UDRE0))); //wait for buffer to be empty
  }
}

/***********************************************************************************************************************
* Function  : (UART_init) - UART AVR initialization
* Arguments : none
* Return    : none
***********************************************************************************************************************/
void UART_init(){
  // Set Baud Rate
  UBRR0H = BAUD_PRESCALER >> 8;
  UBRR0L = BAUD_PRESCALER;
  
  // Set Frame Format
  UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
  
  // Enable Receiver and Transmitter
  UCSR0B = (1<<RXEN0) | (1<<TXEN0);
  
  //Enable Global Interrupts
  sei();

  UCSR0B |= RX_COMPLETE_INTERRUPT;
}

/***********************************************************************************************************************
* Function  : (USART_RX_vect) - UART RX Reception Interruption
* Arguments : none
* Return    : none
***********************************************************************************************************************/
ISR(USART_RX_vect){
  USART_ReceiveBuffer = UDR0;
  playload_uart_reception(USART_ReceiveBuffer);

  //static bool estado = false;  // DEBUG CODE
  //if (estado == false){  
  //  digitalWrite(LED_DEBUG, HIGH);
  //  estado = true;
  //}
  //else if (estado == true){
  //  digitalWrite(LED_DEBUG, LOW);
  //  estado = false;
  //}
}

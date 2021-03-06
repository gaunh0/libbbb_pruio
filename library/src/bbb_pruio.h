/*
 * Lib BBB Pruio
 * Copyright (C) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

/**
 * A structure for easy reading of incoming messages.
 */
typedef struct bbb_pruio_message{
   int is_gpio; // 1 if gpio, 0 if adc
   int value;
   int adc_channel;
   int gpio_number;
} bbb_pruio_message;

typedef enum{  
   BBB_PRUIO_OUTPUT_MODE = 0,
   BBB_PRUIO_INPUT_MODE = 1
} bbb_pruio_gpio_mode;

/**
 * Initializes PRU, GPIO and ADC hardware and starts sampling ADC channels.
 */
int bbb_pruio_start();

/**
 * Stops PRU and ADC hardware, no more samples are aquired.
 */
int bbb_pruio_stop();


/**
 * Returns a gpio number from a string with the pin name "P9_11" for example
 */
int bbb_pruio_get_gpio_number(char* pin_name);  

/**
 * Configures how a GPIO channel is used
 */
int bbb_pruio_init_gpio_pin(int gpio_number, bbb_pruio_gpio_mode mode);  

/**
 * Sets the value of an output pin (0 or 1)
 */
void bbb_pruio_set_pin_value(int gpio_number, int value);

/**
 * Starts reading from an ADC pin
 */
int bbb_pruio_init_adc_pin(int channel_number); 

/**
 * Returns 1 if there is data available from the PRU
 */
inline int bbb_pruio_messages_are_available();

/**
 * Puts the next message available from the PRU in the message address.
 */
inline void bbb_pruio_read_message(bbb_pruio_message *message);

/**
 * Returns a structure for easy interpretation of the message.
 */
inline void bbb_pruio_parse_message(unsigned int *message, bbb_pruio_message* parsed_message);


///////////////////////////////////////////////////////////////////////////////
// !!!
// Not safe to use anything below this line from client code.
// We've put it here just so the compiler can inline it.
///////////////////////////////////////////////////////////////////////////////


// Communication with PRU is done througn a ring buffer in the
// PRU shared memory area.
// shared_ram[0] to shared_ram[1023] is the buffer data.
// shared_ram[1024] is the start (read) pointer.
// shared_ram[1025] is the end (write) pointer.
//
// Messages are 32 bit unsigned ints.
// 
// Read these:
// * http://en.wikipedia.org/wiki/Circular_buffer#Mirroring
// * https://groups.google.com/forum/#!category-topic/beagleboard/F9JI8_vQ-mE

volatile unsigned int *bbb_pruio_shared_ram = NULL;
unsigned int bbb_pruio_buffer_size;
volatile unsigned int *bbb_pruio_buffer_start;
volatile unsigned int *bbb_pruio_buffer_end;

inline __attribute__ ((always_inline)) int bbb_pruio_messages_are_available(){
   return (*bbb_pruio_buffer_start != *bbb_pruio_buffer_end);
}

inline __attribute__ ((always_inline)) void bbb_pruio_read_message(bbb_pruio_message* message){
   unsigned int raw_message = bbb_pruio_shared_ram[*bbb_pruio_buffer_start & (bbb_pruio_buffer_size-1)];

   message->is_gpio = (raw_message&(1<<31))==0;
   if(message->is_gpio){
      message->value = (raw_message&(1<<8))==0;
      message->gpio_number = raw_message & 0xFF;
   }
   else{
      message->value = (raw_message >> 4) & 0xFFF; 
      message->adc_channel = raw_message & 0xF;
   }

   // Don't write buffer start before reading message (mem barrier)
   // http://stackoverflow.com/questions/982129/what-does-sync-synchronize-do
   // https://en.wikipedia.org/wiki/Memory_ordering#Compiler_memory_barrier
   __sync_synchronize();

   // Increment buffer start, wrap around 2*size
   *bbb_pruio_buffer_start = (*bbb_pruio_buffer_start+1) & (2*bbb_pruio_buffer_size - 1);
}

inline __attribute__ ((always_inline)) void bbb_pruio_parse_message(unsigned int *message, bbb_pruio_message* parsed_message){

}

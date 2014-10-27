////////////////////////////////////////////////////////////////////////////////
// 
//  Lib BBB Pruio
//  Copyright (C) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
//
//  This program is free software: you can redistribute it and/or modify it 
//  under the terms of the GNU General Public License as published by the Free 
//  Software Foundation, either version 3 of the License, or (at your option) 
//  any later version.
//  
//  This program is distributed in the hope that it will be useful, but WITHOUT 
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
//  more details.  
//  
//  You should have received a copy of the GNU General Public License along 
//  with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#include "m_pd.h"
#include <bbb_pruio.h>

/* #include <sys/time.h> */
/* #include <time.h> */

/* #include <string.h> */
/* #include <pthread.h> */
/* #include <unistd.h> */
/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include "libbbb_gpio.h" */

////////////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct input {
   t_object x_obj;
   t_outlet *outlet_left;

   unsigned long time;
   unsigned long previous_time;
   unsigned long average_time;
   unsigned int counter;

   t_clock *clock;

} t_input;

// A pointer to the class object.
t_class *bbb_input_class;


static void clock_tick(t_input *x){
   clock_delay(x->clock, 10.0); // milliseconds

   // Read messages from PRU.
   unsigned int message;
   while(bbb_pruio_messages_are_available()){
      bbb_pruio_read_message(&message);
      outlet_float(x->outlet_left, message);
   }
}

///////////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void *bbb_input_new(void) {
   t_input *x = (t_input *)pd_new(bbb_input_class);
   x->outlet_left = outlet_new(&x->x_obj, &s_anything);
   x->clock = clock_new(x, (t_method)clock_tick);

   // Use a pd clock to tick every 0.666 millisecond
   clock_delay(x->clock, 10);

   bbb_pruio_start();

   return (void *)x;
}

static void bbb_input_free(t_input *x) { 
   clock_free(x->clock);
   bbb_pruio_stop();
}

///////////////////////////////////////////////////////////////////////////////
// Class definition
// 

void bbb_input_setup(void) {
   bbb_input_class = class_new(gensym("bbb_input"), (t_newmethod)bbb_input_new, (t_method)bbb_input_free, sizeof(t_input), CLASS_DEFAULT, (t_atomtype)0);
}

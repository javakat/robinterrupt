
/*****************************************************************************\
* Laboratory Exercises COMP 3510                                              *
* Author: Saad Biaz                                                           *
* Date  : March 5, 2013                                                   *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/


#include "common.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/



/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/
#define MAX_NUMBER_EVENTS 256




/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/
typedef struct EventQueue {
  Identifier head;
  Identifier tail;
  Event contents[MAX_NUMBER_EVENTS];
} EventQueue;

EventQueue * enqueue(EventQueue *eq, Event e) {
  if(eq->tail == -1) {
    eq->tail = eq->head;
    eq->contents[eq->head] = e;
  }
  else if(eq->tail + 1 == eq->head
  ||(eq->tail + 1 >= MAX_NUMBER_EVENTS && eq->head == 0)) {
    // we're full, so print that and drop the event
    printf("Event queue is full. (Head: %d, tail: %d)\n", eq->head, eq->tail);
  }
  else {
    int newTail = eq->tail + 1;
    if(newTail >= MAX_NUMBER_EVENTS) newTail = 0;
    eq->contents[newTail] = e;
    eq->tail = newTail;
  }
  return eq;
}

int isEmpty(EventQueue *eq) {
  if(eq->tail == -1) return TRUE;
  return FALSE;
}

EventQueue * dequeue(EventQueue *eq) {
  if(isEmpty(eq) == FALSE) {

    eq->contents[eq->head].EventID = -1;

    if(eq->head == eq->tail) {
      // Now it's empty, so mess it all up
      eq->tail = -1;
    }
    else if (eq->head + 1 == MAX_NUMBER_EVENTS) {
      eq->head = 0;
    }
    else {
      eq->head++;
    }
  }
  else {
    printf("Attempted to dequeue from an empty queue.");
  }
  return eq;
}

Event * peek(EventQueue *eq) {
  // If they're different (so the queue has contents),
  // or they're the same, but nonnegative (so the queue has one element),
  if(isEmpty(eq) == FALSE) {
    // return the front element.
    return &eq->contents[eq->head];
  }
  // Otherwise, return a bogus event.
  return NULL;
}

/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/
EventQueue queue = (EventQueue){ .head = 0, .tail = -1 };
int events = 0;
float responseTime = 0;
float turnaroundTime = 0;

/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void Control(void);
void InterruptRoutineHandlerDevice(void);
void BookKeeping();


/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment for embedded systems. The parent *
*           process is the "control" process while children process will gene-*
*           generate events on devices                                        *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {

   if (Initialization(argc,argv)){
     Control();
   }
} /* end of main function */

/***********************************************************************\
 * Input : none                                                          *
 * Output: None                                                          *
 * Function: Monitor Devices and process events (written by students)    *
 \***********************************************************************/
void Control(void){

  while (1) {
    if(isEmpty(&queue) == FALSE) {
      Event ev = *peek(&queue);
      if(&ev == NULL) continue;
      queue = *dequeue(&queue);
      Server(&ev);
      turnaroundTime += Now() - ev.When;
      if(Show) DisplayEvent('C', &ev);
    }
  }

}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run whenever an event occurs on a device    *
*           The id of the device is encoded in the variable flag        *
\***********************************************************************/
void InterruptRoutineHandlerDevice(void){
  if (Show) printf("An event occured at %f  Flags = %d \n", Now(), Flags);
  // Put Here the most urgent steps that cannot wait
  // Determine device id
  Identifier interruptingDevice = 0;
  int x;
  for(x = 0; x < 32; x++) {
    if(Flags >= exp2(x)) {
      interruptingDevice = x;
    }
    else break;
  }
  if(Show) DisplayEvent('I', &BufferLastEvent[interruptingDevice]);
  queue = *enqueue(&queue, BufferLastEvent[interruptingDevice]);
  Flags -= exp2(interruptingDevice);
  responseTime += Now() - queue.contents[queue.tail].When;
  events++;
}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This must print out the number of Events buffered not yet   *
*           not yet processed (Server() function not yet called)        *
\***********************************************************************/
void BookKeeping(void){
  // For EACH device, print out the following metrics :
  // 1) the percentage of missed events,
  // 2) the average response time, and
  // 3) the average turnaround time.
  // Print the overall averages of the three metrics 1-3 above
  int totalEvents = 0;
  int x;
  for(x = 0; x < MAX_NUMBER_DEVICES; x++) {
    // If it's not a valid event, skip it
    if(BufferLastEvent[x].EventID <= 0) continue;
    // Accounting for conversion from index to size
    totalEvents += BufferLastEvent[x].EventID + 1;
  }

  printf("Percentage of missed events: %3.3f\n", (float)(totalEvents - events) / totalEvents * 100);
  printf("Average response time: %10.10f\n", responseTime/events);
  printf("Average turn-around time: %10.10f\n", turnaroundTime/events);
}

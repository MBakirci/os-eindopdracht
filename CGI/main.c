#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "semaphore.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 1024

typedef struct {
  bool D_UP;              //!< D-Pad up
  bool D_DOWN;            //!< D-Pad down
  bool D_LEFT;            //!< D-Pad left
  bool D_RIGHT;           //!< D-pad right
  bool START;             //!< Start button
  bool BACK;              //!< Back button
  bool LS_PRESS;          //!< Left stick press
  bool RS_PRESS;          //!< Right stick press
  bool LB;                //!< Button LB
  bool RB;                //!< Button RB
  bool LOGO;              //!< Xbox LOGO button
  bool SPARE;             //!< Unused
  bool A;                 //!< Button A
  bool B;                 //!< Button B
  bool X;                 //!< Button X
  bool Y;                 //!< Button Y
  uint8_t Left_trigger;   //!< Left trigger. Produces a value from 0 to 255
  uint8_t Right_trigger;  //!< Right trigger. Produces a value from 0 to 255
  int16_t Left_stick_X;   //!< Left joystick x-value. Produces a value from
                          //!-32768 to 32767
  int16_t Left_stick_Y;   //!< Left joystick y-value. Produces a value from
                          //!-32768 to 32767
  int16_t Right_stick_X;  //!< Right joystick x-value. Produces a value from
                          //!-32768 to 32767
  int16_t Right_stick_Y;  //!< Right joystick y-value. Produces a value from
                          //!-32768 to 32767
} Button;

int main(void) {
  void *vaddr;
  int shm_fd = 0;

  Button *button;

  if ((shm_fd = shm_open("my_shm", O_CREAT | O_RDWR, 0666)) == -1) {
    perror("cannot open");
    return -1;
  }

  /* set the shared memory size to SHM_SIZE */
  if (ftruncate(shm_fd, SIZE) != 0) {
    perror("cannot set size");
    return -1;
  }

  /* Map shared memory in address space. MAP_SHARED flag tells that this is a
   shared mapping */
  if ((vaddr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0)) ==
      MAP_FAILED) {
    perror("cannot mmap");
    return -1;
  }

  if (mlock(vaddr, SIZE) != 0) {
    perror("cannot mlock");
    return -1;
  }

  char *data;
  char command[256];
  printf("%s%c%c\n", "Content-Type:text/html;charset=iso-8859-1", 13, 10);
  printf("<TITLE>! -- XBOX CONTROLLER -- !</TITLE>\n");
  printf("<H3>! -- XBOX CONTROLLER -- !</H3>\n");
  data = getenv("QUERY_STRING");
  if (data == NULL) {
    printf("<P>Error! Error in passing data from form to script.");
  } else {
    sscanf(data, "command=%s", command);
    printf("<P>The command is %s", command);
    button = (Button *)vaddr;
    if (button->LOGO) {
      printf("LOGO button ingedrukt");
    }
  }

  return 0;
}
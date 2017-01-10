#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>  // for abs()
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "syslog.h"
#include <mqueue.h>
#include <signal.h>
#include <time.h>
#include "string.h"
#include <sys/types.h>

#define QUEUE_NAME "/my_queue"
#define PRIORITY 1
#define SIZE 1024

bool rumbled = false;
bool ledjeson = false;
bool quit = false;

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
} Buttons;

typedef struct {
  char command[255];
} Command;

int inputs(libusb_device_handle *handle, Buttons *buttons) {
  uint8_t input[20];
  int result, transferred;

  result = libusb_interrupt_transfer(handle, 0x81, input, sizeof input,
                                     &transferred, 0);

  // That particular value is calculated as eg. 0.2.0
  // in hex, or 2 * 16-1 (2/16) multiplied by
  // 1024, which gives (2 * 1024 / 16) or 128 in decimals

  // D-Pad values
  // 0x02.0	 1	 D-Pad up	 D-Pad up
  // 0x02.1	 1	 D-Pad down	 D-Pad down
  // 0x02.2	 1	 D-Pad left	 D-Pad left
  // 0x02.3	 1	 D-pad right D-Pad right
  buttons->D_UP = input[2] & 0x01;
  buttons->D_DOWN = input[2] & 0x02;
  buttons->D_LEFT = input[2] & 0x04;
  buttons->D_RIGHT = input[2] & 0x08;
  //
  // 0x02.4	 1	 Start button	 Button 8
  // 0x02.5	 1	 Back button	 Button 7
  // 0x02.6	 1	 Left stick press	 Button 9
  // 0x02.7	 1	 Right stick press	 Button 10
  buttons->START = input[2] & 0x10;
  buttons->BACK = input[2] & 0x20;
  buttons->LS_PRESS = input[2] & 0x40;
  buttons->RS_PRESS = input[2] & 0x80;
  // 0x03.0	 1	 Button LB	 Button 5
  // 0x03.1	 1	 Button RB	 Button 6
  // 0x03.2	 1	 Xbox logo button
  // 0x03.3	 1	 Unused
  buttons->LB = input[3] & 0x01;
  buttons->RB = input[3] & 0x02;
  buttons->LOGO = input[3] & 0x04;
  buttons->SPARE = input[3] & 0x08;
  // Char buttons
  // 0x03.4	 1	 Button A	 Button 1
  // 0x03.5	 1	 Button B	 Button 2
  // 0x03.6	 1	 Button X	 Button 3
  // 0x03.7	 1	 Button Y	 Button 4
  buttons->A = input[3] & 0x10;
  buttons->B = input[3] & 0x20;
  buttons->X = input[3] & 0x40;
  buttons->Y = input[3] & 0x80;
  // Analog triggers/buttons
  // 0x04.0	 8	 Left trigger	 Z-axis down
  // 0x05.0	 8	 Right trigger	 Z-axis up
  // 0x06.0	 16	 Left stick X-axis	 X-axis
  // 0x08.0	 16	 Left stick Y-axis	 Y-axis
  // 0x0a.0	 16	 Right stick X-axis	 X-turn
  // 0x0c.0	 16	 Right stick Y-axis	 Y-turn
  // 0x0e.0	 48	 Unused
  buttons->Left_trigger = input[0x04];
  buttons->Right_trigger = input[0x05];
  buttons->Left_stick_X = input[0x07] << 8 | input[0x06];
  buttons->Left_stick_Y = input[0x09] << 8 | input[0x08];
  buttons->Right_stick_X = input[0x0b] << 8 | input[0x0a];
  buttons->Right_stick_Y = input[0x0d] << 8 | input[0x0c];

  return result;
}

void rumble(libusb_device_handle *handle) {
  int written = 0;

  if (rumbled) {
    printf("trillleeeen");
    unsigned char rumble[8] = {0x00, 0x08, 0x00, 0x55, 0x55, 0x00, 0x00, 0x00};
    libusb_interrupt_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), rumble, 8,
                              &written, 0);
    rumbled = false;
  } else {
    unsigned char rumble[8] = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    libusb_interrupt_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), rumble, 8,
                              &written, 0);
    rumbled = true;
  }
}

void leds(libusb_device_handle *handle) {
  int written;

  if (ledjeson) {
    printf("ledjes draaaien");
    unsigned char data[3];
    data[0] = 0x01;
    data[1] = 0x03;
    data[2] = 0x0A;
    libusb_interrupt_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), data, 3,
                              &written, 0);
    ledjeson = false;
  } else {
    unsigned char data[3];
    data[0] = 0x01;
    data[1] = 0x03;
    data[2] = 0x00;
    libusb_interrupt_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), data, 3,
                              &written, 0);
    ledjeson = true;
  }
}
void sigHandler();

int main(int argc, char *argv[]) {
  printf("start\n");
  Command my_command;
  FILE *fp = NULL;
  libusb_device_handle *handle;
  pid_t pid, sid;
  //   Buttons buttons;
  pid = fork();
  // We got a good pid, Close the Parent Process
  if (pid < 0) {
    exit(1);
  }

  if (pid > 0) {
    printf("process_id of child process %d \n", pid);
    exit(0);
    // exit(EXIT_FAILURE);
  }

  umask(0);
  // Create a new Signature Id for our child
  sid = setsid();
  if (sid < 0) {
    exit(1);
  }
  // Close stdin. stdout and stderr
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  fp = fopen("Log.txt", "w+");

  libusb_init(NULL);
  libusb_set_debug(NULL, 3);
  handle = libusb_open_device_with_vid_pid(NULL, 0x045e, 0x028e);
  libusb_kernel_driver_active(handle, 0);
  libusb_detach_kernel_driver(handle, 0);
  libusb_claim_interface(handle, 0);
  if (handle == NULL) {
    fprintf(fp, "Failed to connect the device\n");
    fflush(fp);
    return (1);
  } else {
    fprintf(fp, "device connected \n");
    fflush(fp);
  }
  fprintf(fp,
          "\nDruk op een knop of beweeg met een stick.(LOGO is modus "
          "veranderen)\n");
  fflush(fp);
  fprintf(fp, "voor ds\n");
  fflush(fp);
  mqd_t ds;
  if ((ds = mq_open(QUEUE_NAME, O_RDWR, 0600, NULL)) == (mqd_t)-1) {
    perror("Creating queue error");
    return -1;
  }
  fprintf(fp, "voor signal\n");
  fflush(fp);

  signal(SIGINT, sigHandler);
//   char command[255];
  fprintf(fp, "voor while loop\n");
  fflush(fp);

  while (!quit) {
    // inputs(handle, &buttons);
    struct timespec ts = {time(0) + 5, 0};
    if (mq_timedreceive(ds,(char *) &my_command, SIZE, 0, &ts) == -1) {
      perror("cannot receive");
      return -1;
    }
    if (strcmp(my_command.command, "rumble\n") == 0) {
      fprintf(fp, my_command.command);
      fflush(fp);
      rumble(handle);
    }
    if (strcmp(my_command.command, "ledOn\n") == 0) {
      fprintf(fp, my_command.command);
      fflush(fp);
      leds(handle);
    }
    // command[0] = 0;
  }
  /* Close queue... */
  if (mq_close(ds) == -1) perror("Closing queue error");
  fclose(fp);
  return (0);
}

void sigHandler() { quit = true; }

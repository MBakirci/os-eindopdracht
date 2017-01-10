#include <stdio.h>
#include <stdlib.h>
int main(void) {
  char *data;
  char command[256];
  printf("%s%c%c\n", "Content-Type:text/html;charset=iso-8859-1", 13, 10);
  printf("<TITLE>! -- XBOX CONTROLLER -- !</TITLE>\n");
  printf("<H3>! -- XBOX CONTROLLER -- !</H3>\n");
  data = getenv("QUERY_STRING");
  if (data == NULL) {
    printf("<P>Error! Error in passing data from form to script.");
  } else {
    sscanf(data,"command=%s",command);
    printf("<P>The command is %s", command);
  }
  return 0;
}
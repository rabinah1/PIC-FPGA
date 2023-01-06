int serialOpen(char *port, int baud_rate);
void serialPuts(int fd, char *command);
int serialDataAvail(int fd);
char serialGetchar(int fd);
void serialClose(int fd);

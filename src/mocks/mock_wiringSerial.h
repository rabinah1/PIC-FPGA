int serialOpen(char *port, int baud_rate);
void serialPuts(int fd, char *command);
int serialDataAvail(int fd);
int serialGetchar(int fd);
void serialClose(int fd);

void *result_thread(void *arguments);
void *mem_dump_thread(void *arguments);
void *clk_thread(void *arguments);
void *timer_ext_clk_thread(void *arguments);
bool process_command(char *command, void *arguments, FILE *result_file, bool write_to_file, char *serial_port);
void print_help(void);

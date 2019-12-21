#define BCM2835_PERI_BASE        0x3F000000			//peripheral base address
#define GPIO_BASE		(BCM2835_PERI_BASE + 0x200000) 	// GPIO controller base address

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define BLOCK_SIZE (4*20)	//only using gpio registers region
#define CLK_PIN 5
#define CLK_FREQ_HZ 2
#define RESET_PIN 6
#define LITERAL_PIN 13
#define OPCODE_PIN 19
#define ALU_ENABLE_PIN 26
#define LITERAL_ENABLE_PIN 21
#define OPCODE_ENABLE_PIN 20
#define WREG_ENABLE_PIN 12
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define GET_GPIO(g) (*(gpio+13)&(1<<g))

int clk_enable = 0;
int clk_exit = 0;

struct reset_struct
{
  volatile unsigned *gpio;
  int reset_enable;
};

struct literal_struct
{
  volatile unsigned *gpio;
  char bit;
};

void init_pins(void *vargp)
{
  volatile unsigned *gpio;
  gpio = (volatile unsigned *)vargp;

  int arr[] = {CLK_PIN, RESET_PIN, LITERAL_PIN, OPCODE_PIN, ALU_ENABLE_PIN, LITERAL_ENABLE_PIN, OPCODE_ENABLE_PIN, WREG_ENABLE_PIN};
  size_t len = sizeof(arr)/sizeof(arr[0]);
  int idx = 0;
  while (idx < len)
    {
      INP_GPIO(arr[idx]);
      OUT_GPIO(arr[idx]);
      idx = idx + 1;
    }
}

volatile unsigned* init_gpio_map(void)
{
  // open file for mapping
  int mem_fd;
  void *gpio_map;
  volatile unsigned *gpio;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
    {
      printf("can't open /dev/mem \n");
      exit(-1);
    }
  
  gpio_map = mmap(
		  NULL,             	// Any adddress in our space will do
		  BLOCK_SIZE,      	// Map length
		  PROT_READ|PROT_WRITE,	// Enable reading & writting to mapped memory
		  MAP_SHARED,       	// Shared with other processes
		  mem_fd,          	// File to map
		  GPIO_BASE        	// Offset to GPIO peripheral
		  );
  close(mem_fd);
  
  if (gpio_map == MAP_FAILED)
    {
      printf("mmap error %d\n", (int)gpio_map);
      exit(-1);
    }
  gpio = (volatile unsigned *)gpio_map;
  return gpio;
}

void *read_literal(void *vargp)
{
  volatile unsigned *gpio;
  char bit;
  struct literal_struct *my_literal = (struct literal_struct *)vargp;
  gpio = my_literal->gpio;
  bit = my_literal->bit;

  if (bit == '0')
    {
      GPIO_CLR = 1<<LITERAL_PIN;
    }
  else if (bit == '1')
    {
      GPIO_SET = 1<<LITERAL_PIN;
    }
  return NULL;
}

void *read_opcode(void *vargp)
{
  volatile unsigned *gpio;
  char bit;
  struct literal_struct *my_literal = (struct literal_struct *)vargp;
  gpio = my_literal->gpio;
  bit = my_literal->bit;

  if (bit == '0')
    {
      GPIO_CLR = 1<<OPCODE_PIN;
    }
  else if (bit == '1')
    {
      GPIO_SET = 1<<OPCODE_PIN;
    }
  return NULL;
}

void *clk_thread(void *vargp)
{
  volatile unsigned *gpio;
  gpio = (volatile unsigned *)vargp;
  
  double clk_period = 1000000/CLK_FREQ_HZ;
  while(1)
    {
      if (clk_exit == 1)
	break;
      if (clk_enable == 1)
	{
	  GPIO_SET = 1<<CLK_PIN;
	  usleep(clk_period/2);
	  GPIO_CLR = 1<<CLK_PIN;
	  usleep(clk_period/2);
	}
    }
  return NULL;
}

void *reset_thread(void *vargp)
{
  volatile unsigned *gpio;
  int reset_enable;
  struct reset_struct *my_struct = (struct reset_struct *)vargp;
  reset_enable = my_struct->reset_enable;
  gpio = my_struct->gpio;

  if (reset_enable == 1)
    {
      GPIO_SET = 1<<RESET_PIN;
    }
  if (reset_enable == 0)
    {
      GPIO_CLR = 1<<RESET_PIN;
    }
  return NULL;
}

void *enable_alu(void *vargp)
{
  volatile unsigned *gpio;
  int alu_enable;
  struct reset_struct *my_struct = (struct reset_struct *)vargp;
  alu_enable = my_struct->reset_enable;
  gpio = my_struct->gpio;

  if (alu_enable == 1)
    {
      GPIO_SET = 1<<ALU_ENABLE_PIN;
    }
  if (alu_enable == 0)
    {
      GPIO_CLR = 1<<ALU_ENABLE_PIN;
    }
  return NULL;
}

void *enable_literal(void *vargp)
{
  volatile unsigned *gpio;
  int literal_enable;
  struct reset_struct *my_struct = (struct reset_struct *)vargp;
  literal_enable = my_struct->reset_enable;
  gpio = my_struct->gpio;

  if (literal_enable == 1)
    {
      GPIO_SET = 1<<LITERAL_ENABLE_PIN;
    }
  if (literal_enable == 0)
    {
      GPIO_CLR = 1<<LITERAL_ENABLE_PIN;
    }
  return NULL;
}

void *enable_opcode(void *vargp)
{
  volatile unsigned *gpio;
  int opcode_enable;
  struct reset_struct *my_struct = (struct reset_struct *)vargp;
  opcode_enable = my_struct->reset_enable;
  gpio = my_struct->gpio;

  if (opcode_enable == 1)
    {
      GPIO_SET = 1<<OPCODE_ENABLE_PIN;
    }
  if (opcode_enable == 0)
    {
      GPIO_CLR = 1<<OPCODE_ENABLE_PIN;
    }
  return NULL;
}

void *enable_wreg(void *vargp)
{
  volatile unsigned *gpio;
  int wreg_enable;
  struct reset_struct *my_struct = (struct reset_struct *)vargp;
  wreg_enable = my_struct->reset_enable;
  gpio = my_struct->gpio;

  if (wreg_enable == 1)
    {
      GPIO_SET = 1<<WREG_ENABLE_PIN;
    }
  if (wreg_enable == 0)
    {
      GPIO_CLR = 1<<WREG_ENABLE_PIN;
    }
  return NULL;
}

int main(void)
{
  char binary_data[199];
  pthread_t clk_thread_id;
  pthread_t literal_thread_id;
  pthread_t reset_thread_id;
  pthread_t opcode_thread_id;
  pthread_t alu_enable_thread_id;
  pthread_t literal_enable_thread_id;
  pthread_t opcode_enable_thread_id;
  pthread_t wreg_enable_thread_id;
  int operation;
  int rising_check = 0;
  volatile unsigned *gpio = init_gpio_map();
  struct reset_struct *res;
  struct literal_struct *literal;
  res = malloc(sizeof(struct reset_struct));
  literal = malloc(sizeof(struct literal_struct));
  res->gpio = gpio;
  res->reset_enable = 0;
  literal->gpio = gpio;
  literal->bit = '0';
  init_pins((void *)gpio);
  pthread_create(&clk_thread_id, NULL, clk_thread, (void *)gpio);
  while(1)
    {
      printf("What do you want to do?\n");
      printf("1) Enable clock\n");
      printf("2) Disable clock\n");
      printf("3) Enable ALU\n");
      printf("4) Disable ALU\n");
      printf("5) Enable literal\n");
      printf("6) Disable literal\n");
      printf("7) Enable opcode\n");
      printf("8) Disable opcode\n");
      printf("9) Enable W-register\n");
      printf("10) Disable W-register\n");
      printf("11) Enable reset\n");
      printf("12) Disable reset\n");
      printf("13) Send literal data\n");
      printf("14) Send opcode\n");
      printf("15) Exit program\n");
      scanf("%d", &operation);
      
      if (operation == 1)
	clk_enable = 1;
      else if (operation == 2)
	clk_enable = 0;
      else if (operation == 3)
	{
	  res->reset_enable = 1;
	  pthread_create(&alu_enable_thread_id, NULL, enable_alu, (void *)res);
	  pthread_join(alu_enable_thread_id, NULL);
	}
      else if (operation == 4)
	{
	  res->reset_enable = 0;
	  pthread_create(&alu_enable_thread_id, NULL, enable_alu, (void *)res);
	  pthread_join(alu_enable_thread_id, NULL);
	}
      else if (operation == 5)
	{
	  res->reset_enable = 1;
	  pthread_create(&literal_enable_thread_id, NULL, enable_literal, (void *)res);
	  pthread_join(literal_enable_thread_id, NULL);
	}
      else if (operation == 6)
	{
	  res->reset_enable = 0;
	  pthread_create(&literal_enable_thread_id, NULL, enable_literal, (void *)res);
	  pthread_join(literal_enable_thread_id, NULL);
	}
      else if (operation == 7)
	{
	  res->reset_enable = 1;
	  pthread_create(&opcode_enable_thread_id, NULL, enable_opcode, (void *)res);
	  pthread_join(opcode_enable_thread_id, NULL);
	}
      else if (operation == 8)
	{
	  res->reset_enable = 0;
	  pthread_create(&opcode_enable_thread_id, NULL, enable_opcode, (void *)res);
	  pthread_join(opcode_enable_thread_id, NULL);
	}
      else if (operation == 9)
	{
	  res->reset_enable = 1;
	  pthread_create(&wreg_enable_thread_id, NULL, enable_wreg, (void *)res);
	  pthread_join(wreg_enable_thread_id, NULL);
	}
      else if (operation == 10)
	{
	  res->reset_enable = 0;
	  pthread_create(&wreg_enable_thread_id, NULL, enable_wreg, (void *)res);
	  pthread_join(wreg_enable_thread_id, NULL);
	}
      else if (operation == 11)
	{
	  res->reset_enable = 1;
	  pthread_create(&reset_thread_id, NULL, reset_thread, (void *)res);
	  pthread_join(reset_thread_id, NULL);
	}
      else if (operation == 12)
	{
	  res->reset_enable = 0;
	  pthread_create(&reset_thread_id, NULL, reset_thread, (void *)res);
	  pthread_join(reset_thread_id, NULL);
	}
      else if (operation == 13)
	{
	  if (clk_enable == 0)
	    {
	      printf("ERROR: clock is disabled\n");
	      continue;
	    }
	  if (res->reset_enable == 1)
	    {
	      printf("ERROR: reset is enabled\n");
	      continue;
	    }
	  printf("Enter data to send: ");
	  scanf("%s", binary_data);
	  int length = strlen(binary_data);
	  int idx = 0;
	  while(idx < length)
	    {
	      if (GET_GPIO(CLK_PIN) && rising_check == 0)
		{
		  rising_check = 1;
		  literal->bit = binary_data[idx];
		  pthread_create(&literal_thread_id, NULL, read_literal, (void *)literal);
		  pthread_join(literal_thread_id, NULL);
		  idx++;
		}
	      else if (rising_check == 1 && !(GET_GPIO(CLK_PIN)))
		rising_check = 0;
	    }
	}
      else if (operation == 14)
	{
	  if (clk_enable == 0)
	    {
	      printf("ERROR: clock is disabled\n");
	      continue;
	    }
	  if (res->reset_enable == 1)
	    {
	      printf("ERROR: reset is enabled\n");
	      continue;
	    }
	  printf("Enter data to send: ");
	  scanf("%s", binary_data);
	  int length = strlen(binary_data);
	  int idx = 0;
	  while(idx < length)
	    {
	      if (GET_GPIO(CLK_PIN) && rising_check == 0)
		{
		  rising_check = 1;
		  literal->bit = binary_data[idx];
		  pthread_create(&opcode_thread_id, NULL, read_opcode, (void *)literal);
		  pthread_join(opcode_thread_id, NULL);
		  idx++;
		}
	      else if (rising_check == 1 && !(GET_GPIO(CLK_PIN)))
		rising_check = 0;
	    }
	}
      else if (operation == 15)
	{
	  res->reset_enable = 0;
	  pthread_create(&reset_thread_id, NULL, reset_thread, (void *)res);
	  pthread_join(reset_thread_id, NULL);
	  clk_exit = 1;
	  free(res);
	  free(literal);
	  break;
	}
    }
  pthread_join(clk_thread_id, NULL);
  exit(0);
}

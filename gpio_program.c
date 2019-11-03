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
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))

int  mem_fd;
void *gpio_map;
volatile unsigned int *gpio;
int clk_enable = 0;
int clk_exit = 0;

void *myThread2(void *vargp)
{
	// open file for mapping
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

        close(mem_fd);		//No need to keep file open after mmap

        if (gpio_map == MAP_FAILED)
	{
                printf("mmap error %d\n", (int)gpio_map);//errno also set!
                exit(-1);
        }
        
        gpio = (volatile unsigned *)gpio_map;		// Always use volatile pointer!

	char *bit = (char *)vargp;
	INP_GPIO(4);
	OUT_GPIO(4);
	int i = 0;

	if (*bit == '0')
	  {
	    (*(gpio+10)) |= (1<<4);
	  }
	else if (*bit == '1')
	  {
	    (*(gpio+7)) |= (1<<4);
	  }
        return NULL;
}

void *myThread1(void *vargp)
{
	// open file for mapping
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

        close(mem_fd);		//No need to keep file open after mmap

        if (gpio_map == MAP_FAILED)
	{
                printf("mmap error %d\n", (int)gpio_map);//errno also set!
                exit(-1);
        }
        
        gpio = (volatile unsigned *)gpio_map;		// Always use volatile pointer!

	INP_GPIO(17);
	OUT_GPIO(17);
	int i = 0;
        while(1)
        {
	  if (clk_exit == 1)
	    break;
	  if (clk_enable == 1)
	    {
	      (*(gpio+7)) |= (1<<17);			// Turn pin high
	      usleep(100000);				// sleep process for 1 second
	      (*(gpio+10)) |= (1<<17);		// Turn pin low
	      usleep(100000);
	    }
        }
        return NULL;
}


int main(void)
{
  char binary_data[199];
  pthread_t thread_id_1;
  pthread_t thread_id_2;
  int operation;
  pthread_create(&thread_id_1, NULL, myThread1, NULL);
  while(1)
    {
      printf("What do you want to do?\n");
      printf("1) Enable clock\n");
      printf("2) Disable clock\n");
      printf("3) Send data\n");
      printf("4) Exit program\n");
      scanf("%d", &operation);

      if (operation == 1)
	clk_enable = 1;
      else if (operation == 2)
	clk_enable = 0;
      else if (operation == 3)
	{
	  printf("Enter data to send: ");
	  scanf("%s", binary_data);
	  int length = strlen(binary_data);
	  int idx = 0;
	  while(idx < length)
	    {
	      char bit = binary_data[idx];
	      pthread_create(&thread_id_2, NULL, myThread2, (void *)&bit);
	      pthread_join(thread_id_2, NULL);
	      idx++;
	      sleep(1);
	    }
	}
      else if (operation == 4)
	{
	  clk_exit = 1;
	  break;
	}
    }
  char bit = '0';
  pthread_create(&thread_id_2, NULL, myThread2, (void *)&bit);
  pthread_join(thread_id_1, NULL);
  pthread_join(thread_id_2, NULL);
  exit(0);
}

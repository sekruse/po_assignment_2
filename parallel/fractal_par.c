#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

// Modification: Added for measuring times.
#include "../measure.h"

// draws a mandelbrot fractal
// compile with gcc fractal.c -lm -std=c99 -o fractal

#define NUM_THREADS 2
#define NUM_WORK_ITEMS 100

typedef struct
{
    int start_row, end_row;
} work_item;

typedef struct
{
    float width, height;
    unsigned int *pixmap;
    int* counter;
    pthread_mutex_t* counter_lock;
    work_item* work_items;
} mandelbrot_args;


int pal[256] = {
    0xb2000a,0xb20009,0xb2000a,0xb1000a,0xb1000b,0xaf000d,0xaf000e,0xae000f,0xad0011,0xac0012,0xab0013,0xaa0015,
    0xa90016,0xa80018,0xa70019,0xa6001b,0xa4001d,0xa3001e,0xa2001f,0xa00022,0x9f0023,0x9e0025,0x9c0028,0x9b0029,
    0x9a002b,0x99002d,0x97002f,0x960031,0x940033,0x930035,0x910037,0x8f0039,0x8e003c,0x8d003d,0x8b0040,0x8a0042,
    0x870044,0x860047,0x840048,0x82004b,0x81004e,0x7f0050,0x7d0053,0x7b0055,0x7a0057,0x780059,0x76005c,0x74005e,
    0x730061,0x710063,0x6f0066,0x6e0068,0x6b006a,0x6a006d,0x680070,0x660072,0x640075,0x630077,0x60007a,0x5e007d,
    0x5c007f,0x5b0081,0x590084,0x570086,0x550089,0x54008c,0x52008f,0x500091,0x4e0093,0x4c0096,0x4a0099,0x49009b,
    0x46009e,0x4500a1,0x4300a2,0x4100a5,0x4000a8,0x3e00aa,0x3b00ac,0x3a00af,0x3900b1,0x3600b4,0x3500b6,0x3300b9,
    0x3100bb,0x2f00bd,0x2d00c0,0x2c00c2,0x2a00c4,0x2900c7,0x2700c9,0x2500cb,0x2400cd,0x2200d0,0x2100d2,0x2000d4,
    0x1e00d5,0x1c00d8,0x1b00da,0x1a00dc,0x1800dd,0x1700e0,0x1600e1,0x1400e4,0x1200e5,0x1200e7,0x1100e8,0xf00ea,
    0xd00ec,0xc00ee,0xc00ef,0xa00f1,0x900f2,0x900f4,0x700f6,0x600f6,0x600f8,0x400fa,0x300fb,0x200fb,0x100fd,
    0x100fe,0xff,0x1ff,0x2ff,0x3ff,0x5ff,0x6ff,0x8ff,0x9ff,0xaff,0xcff,0xdff,
    0xfff,0x10ff,0x12ff,0x13ff,0x15ff,0x17ff,0x19ff,0x1aff,0x1cff,0x1fff,0x20ff,0x22ff,
    0x24ff,0x26ff,0x29ff,0x2aff,0x2cff,0x2eff,0x31ff,0x34ff,0x35ff,0x38ff,0x3aff,0x3cff,
    0x3eff,0x41ff,0x43ff,0x46ff,0x49ff,0x4bff,0x4dff,0x50ff,0x52ff,0x54ff,0x57ff,0x59ff,
    0x5cff,0x5eff,0x61ff,0x64ff,0x66ff,0x69ff,0x6cff,0x6eff,0x71ff,0x74ff,0x76ff,0x79ff,
    0x7bff,0x7eff,0x81ff,0x83ff,0x86ff,0x89ff,0x8bff,0x8eff,0x91ff,0x93ff,0x96ff,0x98ff,
    0x9bff,0x9dff,0xa0ff,0xa2ff,0xa5ff,0xa7ff,0xa9ff,0xadff,0xafff,0xb1ff,0xb4ff,0xb6ff,
    0xb9ff,0xbcff,0xbdff,0xbfff,0xc2ff,0xc5ff,0xc7ff,0xc9ff,0xcbff,0xceff,0xcfff,
    0xd1ff,0xd3ff,0xd6ff,0xd7ff,0xd9ff,0xdcff,0xdeff,0xe0ff,0xe2ff,0xe4ff,0xe5ff,
    0xe7ff,0xe9ff,0xebff,0xecff,0xedff,0xefff,0xf1ff,0xf2ff,0xf3ff,0xf5ff,
    0xf6ff,0xf7ff,0xf8ff,0xfaff,0xfbff,0xfcff,0xfcff,0xfcff,0xfcff,0xfcff,
    0xfcff,0xfcff,0xfcff,0xfcff,0xfcff,0xfcff,0xfcff,0xfcff,0xfcff};

void mandelbrot_thread(void *args)
{
    mandelbrot_args *margs = args;
    float height = margs->height;
    float width = margs->width;
    unsigned int *pixmap = margs->pixmap;
	int i, j, start_row, end_row, work_item_index;
    int num_items = 0;
	float xmin = -1.6f;
	float xmax = 1.6f;
	float ymin = -1.6f;
	float ymax = 1.6f;
    clock_t a, b;

    a = 0;
    while (1)
    {
        // Try to get a work item.
        b = clock();
        pthread_mutex_lock(margs->counter_lock);
        a += clock() - b;
        work_item_index = --(*(margs->counter));
        pthread_mutex_unlock(margs->counter_lock);
        if (work_item_index < 0) break;
        start_row = margs->work_items[work_item_index].start_row;
        end_row = margs->work_items[work_item_index].end_row;
        num_items++;

        // Process the work item.
	    for (i = start_row; i < end_row; i++) {
		    for (j = 0; j < width; j++) {
			    float b = xmin + j * (xmax - xmin) / width;
			    float a = ymin + i * (ymax - ymin) / height;
			    float sx = 0.0f;
			    float sy = 0.0f;
			    int ii = 0;
			    while (sx + sy <= 64.0f) {
				    float xn = sx * sx - sy * sy + b;
				    float yn = 2 * sx * sy + a;
				    sx = xn;
				    sy = yn;
				    ii++;
				    if (ii == 1500)	{
					    break;
				    }
			    }
			    if (ii == 1500)	{
				    pixmap[j+i*(int)width] = 0;
			    }
			    else {
				    int c = (int)((ii / 32.0f) * 256.0f);
				    pixmap[j + i *(int)width] = pal[c%256];
			    }
		    }
        }
	}

    printf("Thread processed %d work items.\n", num_items);
    printf("Waited %.3f secs, %ld ticks\n", ((float)a) / CLOCKS_PER_SEC, a);
    pthread_exit(0);
}

void* mandelbrot(float height, float width, unsigned int* pixmap)
{
    // Initialize arguments.
    int d_row = ((int)height + NUM_WORK_ITEMS - 1) / NUM_WORK_ITEMS;
    int start_row, end_row;

    int* counter = (int*) malloc(sizeof(int));
    work_item* work_items = (work_item*) malloc(sizeof(work_item) * NUM_WORK_ITEMS);
    pthread_mutex_t* counter_lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    mandelbrot_args* thread_args = (mandelbrot_args*) malloc(sizeof(mandelbrot_args));
    
    pthread_t threads[NUM_THREADS];
    pthread_attr_t thread_attr;
    int i;

    // Prepare the work items.
    pthread_mutex_init(counter_lock, NULL);
    end_row = 0;
    *counter = 0;
    while (end_row < height)
    {
        start_row = end_row;
        end_row += d_row;
        if (end_row > height) end_row = height;
        work_items[*counter].start_row = start_row;
        work_items[*counter].end_row = end_row;
        (*counter)++;        
    }

    // Prepare the thread arguments.
    thread_args->height = height;
    thread_args->width = width;
    thread_args->pixmap = pixmap;
    thread_args->counter = counter;
    thread_args->counter_lock = counter_lock;
    thread_args->work_items = work_items;

    // Start the threads.
    pthread_attr_init(&thread_attr);
    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], &thread_attr, mandelbrot_thread, (void*) thread_args);
    }

    // Wait for the threads to finish.
    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
        
}

void writetga(unsigned int *pixmap, unsigned int width, unsigned int height, char *name)
{
	FILE *f;
	int i,j;
	char buffer[50];
	f = fopen(name,"wb");
	fwrite("\x00\x00\x02",sizeof(char),3,f);
	fwrite("\x00\x00\x00\x00\x00",sizeof(char),5,f);
	fwrite("\x00\x00",sizeof(char),2,f);
	fwrite("\x00\x00",sizeof(char),2,f);
	sprintf(buffer,"%c%c",(width & 0x00ff)%0xff,(width & 0xff00)%0xff);
	fwrite(buffer,sizeof(char),2,f);
	sprintf(buffer,"%c%c",(height & 0x00ff)%0xff,(height & 0xff00)%0xff);
	fwrite(buffer,sizeof(char),2,f);
	fwrite("\x18\x00",sizeof(char),2,f);
	for (i = height; i >= 0; i--) {
		for (j = 0; j < width; j++) {
			sprintf(buffer, "%c%c%c",
				(pixmap[j+(i)*width]>>16)&0x000000ff,
				(pixmap[j+i*width]>>8)&0x000000ff,
				(pixmap[j+i*width])&0x000000ff);
			fwrite(buffer,sizeof(char),3,f);
		}
	}
	fclose(f);
}


int main(int a, char *args[])
{
	int i, j;
	printf("fractal");
	unsigned int* pixmap = malloc(1024*1024*sizeof(int));

    start_timer();
	mandelbrot(1024.0f, 1024.0f, pixmap);
    stop_timer("Calculated mandelbrot.");

    start_timer();
	writetga(pixmap, 1024, 1024, "fracout.tga");
    stop_timer("Written file.");

	free(pixmap);
	return 0;
}

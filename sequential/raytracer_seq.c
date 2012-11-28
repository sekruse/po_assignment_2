#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "measure.h"

// very simple raytracer with reflections, sphere and plane
// compile with gcc raytracer.c -std=c99 -lm -o raytracer

typedef struct{
	char *type;
	float radius;
	float reflection;
	float diffuse;
	float *normal;
	float *color;
} primitive;

// define objects to raytrace

int primitivenumber = 11;
primitive p[11] = {
    {"plane",4.4f,0.0f,1.0f,(float[3]){0.0f,1.0f,0.0f},(float[3]){0.3f,0.3f,0.3f}},
    {"sphere",2.5f,0.7f,1.0f,(float[3]){0.0f,-0.0f,3.0f},(float[3]){0.2f,0.8f,1.0f}},
    {"sphere",2.5f,0.7f,1.0f,(float[3]){4.0f,-0.0f,3.0f},(float[3]){0.2f,0.8f,1.0f}},
    {"sphere",2.5f,0.7f,1.0f,(float[3]){-4.0f,-0.0f,3.0f},(float[3]){0.2f,0.8f,1.0f}},

    {"sphere",2.5f,0.7f,1.0f,(float[3]){0.0f,-4.0f,3.0f},(float[3]){0.7f,0.7f,0.7f}},
    {"sphere",2.5f,0.7f,1.0f,(float[3]){4.0f,-4.0f,3.0f},(float[3]){0.7f,0.7f,0.7f}},
    {"sphere",2.5f,0.7f,1.0f,(float[3]){-4.0f,-4.0f,3.0f},(float[3]){0.7f,0.7f,0.7f}},

    {"sphere",2.5f,0.7f,1.0f,(float[3]){0.0f,4.0f,3.0f},(float[3]){0.2f,0.5f,0.8f}},
    {"sphere",2.5f,0.7f,1.0f,(float[3]){4.0f,4.0f,3.0f},(float[3]){0.2f,0.5f,0.8f}},
    {"sphere",2.5f,0.7f,1.0f,(float[3]){-4.0f,4.0f,3.0f},(float[3]){0.2f,0.5f,0.8f}},

    {"light",0.1f,1.0f,1.0f,(float[3]){0.0f,3.0f,-5.0f},(float[3]){0.7f,0.7f,0.7f}},
};

float viewplanex0 = -5.0f;
float viewplanex1 = 5.0f;
float viewplaney0 = 5.0f;
float viewplaney1 = -5.0f;
float deltax, deltay;
float stepx, stepy;

void normalize(float *v)
{
	float l = 1.0f / sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	v[0]*=l;
	v[1]*=l;
	v[2]*=l;
}

float dotproduct(float *v, float *w)
{
	return v[0]*w[0] + v[1]*w[1] + v[2]*w[2];
}

int sphereintersect(float *raydir, float *viewer, float *cdist, int i)
{
	float *v = (float[3]){viewer[0]-p[i].normal[0], viewer[1]-p[i].normal[1], viewer[2]-p[i].normal[2]};
	float b = -dotproduct(v,raydir);
	float det = (b*b) - dotproduct(v,v) + p[i].radius;
	int ret = 0;
	if (det > 0) {
		det = sqrt(det);

		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0) {
			if (i1 < 0) {
				if (i2 < *cdist) {
					*cdist = i2;
					ret = -1;
				}
			}
			else {
				if (i1 < *cdist) {
					*cdist = i1;
					ret = 1;
				}
			}
		}
	}
	return ret;
}

int planeintersect(float *raydir, float *viewer, float *cdist, int i)
{
	float d = dotproduct(p[i].normal,raydir);
	if (d != 0) {
		float dist = -(dotproduct(p[i].normal, viewer) + p[i].radius)/d;
		if (dist > 0) {
			if (dist < *cdist) {
				*cdist = dist;
				return 1;
			}
		}
	}
	return 0;
}

void trace(float *raydir, float *viewer, float *color,int depth, float maxdist)
{
	int i;
	int res = 0;
	int prim = -1;
	int hittest = 0;

	if (depth >= 10) { return; }

	for (i = 0; i < primitivenumber; i++) {
		if (strcmp(p[i].type,"plane") == 0) {
			res = planeintersect(raydir, viewer, &maxdist, i);
			if (res != 0) {
			    prim = i;
			}
		}
		else if ((strcmp(p[i].type,"sphere") == 0) || (strcmp(p[i].type,"light") == 0)) {
			res = sphereintersect(raydir, viewer, &maxdist, i);
			if (res != 0) {
			    prim = i;
		 	}
		}
	}

	if (prim == -1) { return; }
	if (prim != -1) {
		if (strcmp(p[prim].type,"light") == 0) {
			color[0] = color[1] = color[2] = 1.0f;
		}
		else {
			float *pi = (float[3]){viewer[0] + raydir[0] * maxdist,
					       viewer[1] + raydir[1] * maxdist,
					       viewer[2] + raydir[2] * maxdist};

			for (i = 0; i < primitivenumber; i++) {
				if (strcmp(p[i].type,"light") == 0) {
					float *l = (float[3]){p[i].normal[0] - pi[0], 
							      p[i].normal[1] - pi[1], 
							      p[i].normal[2] - pi[2]};
					normalize(l);
					float *n = p[prim].normal;
					if (strcmp(p[prim].type,"sphere") == 0) {
						n = (float[3]){(pi[0] - p[prim].normal[0]) * p[prim].radius, 
							       (pi[1] - p[prim].normal[1]) * p[prim].radius,
							       (pi[2] - p[prim].normal[2]) * p[prim].radius};
					}
					if (p[prim].diffuse > 0.0f) {
						float dot = dotproduct(l,n);
						if (dot > 0.0f) {
							float diff = dot * p[prim].diffuse;
							color[0]+= diff*p[prim].color[0]*p[i].color[0];
							color[1]+= diff*p[prim].color[1]*p[i].color[1];
							color[2]+= diff*p[prim].color[2]*p[i].color[2];
						}
					}

				}
			}
			float ref = p[prim].reflection;
			if (ref > 0.0f) {
				float *n = p[prim].normal;
				if (strcmp(p[prim].type,"sphere") == 0) {
					n = (float[3]){(pi[0] - p[prim].normal[0]) * p[prim].radius, 
						       (pi[1] - p[prim].normal[1]) * p[prim].radius,
						       (pi[2] - p[prim].normal[2]) * p[prim].radius};
				}
				float *r = (float[3]){raydir[0] - 2.0f * dotproduct(raydir,n) * n[0],
						      raydir[1] - 2.0f * dotproduct(raydir,n) * n[1],
						      raydir[2] - 2.0f * dotproduct(raydir,n) * n[2]};
				normalize(r);
				float *rcol = (float[3]){0.0f, 0.0f, 0.0f};
				float *tt = (float[3]){pi[0] + r[0] * 0.001f, pi[1] + r[1] * 0.001f, pi[2] + r[2] * 0.001f};
				normalize(tt);
				trace(tt, r, rcol,depth+1,100000.0f);
				color[0]+= ref*rcol[0]*p[prim].color[0];
				color[1]+= ref*rcol[1]*p[prim].color[1];
				color[2]+= ref*rcol[2]*p[prim].color[2];
			}
		}
	}
}

void render(float width, float height, int *pixmap)
{
	int i,j;
	stepy = viewplaney0;
	deltax = (viewplanex1 - viewplanex0) / width;
	deltay = (viewplaney1 - viewplaney0) / height;
	stepy += deltay;

	float *viewer = (float[3]){0.0f, 0.0f, -7.0f};
	for (i = 0 ; i < height; i++) {
		stepx = viewplanex0;
		for (j = 0; j < width; j++) {
			float *color = (float[3]){0.0f, 0.0f, 0.0f};
			float *raydir = (float[3]){stepx - viewer[0], stepy - viewer[1], 0 - viewer[2]};
			float maxdist = 100000.0f;
			normalize(raydir);
			trace(raydir, viewer, color,  0,maxdist);
			int r = (int)(roundf(color[0]*255.0f));
			if (r > 255) { r = 255; }
			int g = (int)(roundf(color[1]*255.0f));
			if (g > 255) { g = 255; }
			int b = (int)(roundf(color[2]*255.0f));
			if (b > 255) { b = 255; }
			pixmap[j+i*(int)width] = (r << 16) | (g << 8) | (b);
			stepx += deltax;
 		}
	stepy += deltay;
	}
}

// writes an uncompressed 24bits tga file

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
	printf("raytracer");
	unsigned int* pixmap = malloc(1024*1024*sizeof(int));

    start_timer();
	render(1024.0f, 1024.0f, pixmap);
    stop_timer("Rendering finished.");

    start_timer();
	writetga(pixmap, 1024, 1024, "rayout.tga");
    stop_timer("File written.");

	free(pixmap);
	return 0;
}

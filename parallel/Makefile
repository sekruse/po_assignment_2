# Create all three programs. 
all: raytracer qsort fractal

# Raytracer
raytracer: raytracer_par.c ../measure.c
	gcc -std=c99 -o $@ $^ -lm 

# Quicksort
qsort: qsort_par.c ../measure.c
	gcc -std=c99 -o $@ $^

# Fractal
fractal: fractal_par.c ../measure.c
	gcc -std=c99 -o $@ $^ -lpthread

clean:
	rm -f *~
	rm -f *.o
	rm -f raytracer qsort fractal
	rm -f *.tga

# Create all three programs. 
all: raytracer qsort fractal

# Raytracer
raytracer: raytracer_seq.c ../measure.c
	gcc -std=c99 -o $@ $^ -lm 

# Quicksort
qsort: qsort_seq.c ../measure.c
	gcc -std=c99 -o $@ $^

# Fractal
fractal: fractal_seq.c ../measure.c
	gcc -std=c99 -o $@ $^

clean:
	rm -f *~
	rm -f *.o
	rm -f raytracer qsort fractal
	rm -f *.tga

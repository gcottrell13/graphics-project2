all:
	gcc -o project2 project2.c -lm 

test1:
	./project2 1000 1000 good01.json output.ppm
test2:
	./project2 5 5 good01.json output.ppm
all:
	../../build/bin/sycc quick_sort.c -O2 -o qsort
run: all
	./a.out < test.in
test:all
	./linux-amd64 ./a.out	
clean:
	rm *.ll *.o a.out

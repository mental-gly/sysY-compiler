all:
	../../build/bin/sycc gemm.c -O2 -o gemm
run: all
	./a.out < test.in
test:all
	./linux-amd64 ./a.out	
clean:
	rm *.o *.ll a.out

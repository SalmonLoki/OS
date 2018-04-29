all: rezult
path: rezult
	./rezult

#create libfs.a
./static/%.o: ./static/%.c
	gcc -c -o $@ $<
libfs.a: ./static/f1.o ./static/f2.o
	ar rc $@ ./static/f1.o ./static/f2.o
	ranlib $@

#create libfsdyn.so
./dynamic/%.o: ./dynamic/%.c
	gcc -fPIC -c -o $@ $<
libfsdyn.so: ./dynamic/f3.o
	gcc -shared -o $@ $<

#create libfsdyn2.so
./dynamic/%.o: ./dynamic2/%.c
	gcc -fPIC -c -o $@ $<
libfsdyn2.so: ./dynamic2/f4.o
	gcc -shared -o $@ $<

rezult: main.o libfs.a libfsdyn.so libfsdyn2.so
	gcc -Wl,-rpath=.:./rezult -ldl $< libfs.a libfsdyn.so -o $@


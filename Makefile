hw5: hw5.c
	gcc -o hw5 hw5.c

check: hw5
	echo "Testing hw5"
	./hw5 -a < hw5.in
	./hw5 -d < hw5.in
clean:
	rm -f hw5
	rm -f a.out
dist: clean
	dir=`basename $$PWD`; cd ..; tar czvf $$dir.tar.gz ./$$dir
	dir=`basename $$PWD`; ls -l ../$$dir.tar.gz

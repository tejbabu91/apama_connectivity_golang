.PHONY: clean

libadder.a: adder.go libadder.h
	go build -buildmode=c-archive -o $@

clean:
	rm -f libadder.*

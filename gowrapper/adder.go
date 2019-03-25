package main

import "fmt"

// #include "../test.h"
import "C"

//export GoAdder2
func GoAdder2(x, y *C.TestS) int {
	fmt.Printf("Go says: adding %v and %v\n", x, y)
	return int(x.a + y.a)
}

//export GoAdder
func GoAdder(x, y int) int {
	fmt.Printf("Go says: adding %v and %v\n", x, y)
	return x + y
}

func main() {} // Required but ignored

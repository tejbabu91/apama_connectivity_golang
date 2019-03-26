package main

import "fmt"

// #cgo LDFLAGS: -Wl,--unresolved-symbols=ignore-all
// #include "c_go_interface.h"
import "C"

import "unsafe"

type gorun interface {
	Start()
	Stop()
	DeliverMessageTowardsTransport()
}

type handler struct {
	obj unsafe.Pointer
}

func (*handler) Start() {

}

func (*handler) Stop() {

}

func (*handler) DeliverMessageTowardsTransport() {

}

var gobj *handler = nil

//export go_init
func go_init(obj unsafe.Pointer) {
	fmt.Printf("Init called: %v", obj)
	gobj = &handler{obj}
}

//export Start
func Start() {
	fmt.Printf("Start called: %v\n", gobj)
}

//export Stop
func Stop() {
	fmt.Printf("Stop called\n")
}

//export DeliverMessageTowardsTransport
func DeliverMessageTowardsTransport(buf unsafe.Pointer, bufLen C.int) {
	gobuf := C.GoBytes(buf, bufLen)
	fmt.Printf("deliver: %v\n", gobuf)
	for _, c := range gobuf {
		fmt.Printf("%c", c)
	}
	fmt.Println()
	C.c_callback(gobj.obj)
}

func main() {} // Required but ignored

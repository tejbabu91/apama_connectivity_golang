package main

// #cgo LDFLAGS: -Wl,--unresolved-symbols=ignore-all
// #include "c_go_interface.h"
import "C"

import (
	"encoding/json"
	"fmt"
	"unsafe"
)

type Message struct {
	payload  interface{}
	metadata map[string]interface{}
}

func serializeMsg(msg *Message) []byte {
	var data = map[string]interface{}{
		"data":     msg.payload,
		"metadata": msg.metadata,
	}

	val, err := json.Marshal(data)

	if err != nil {
		fmt.Println(err)
		return []byte{}
	}
	return val
}

func deserializeMsg(v []byte) *Message {
	var data map[string]interface{}

	err := json.Unmarshal(v, &data)
	if err != nil {
		return nil
	}

	msg := Message{
		payload:  data["data"],
		metadata: data["metadata"].(map[string]interface{}),
	}

	return &msg
}

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
	C.c_callback(gobj.obj, unsafe.Pointer(&gobuf[0]), C.int(len(gobuf)))
}

func main() {} // Required but ignored

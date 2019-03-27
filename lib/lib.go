package lib

// #include "c_go_interface.h"
import "C"

import (
	"encoding/json"
	"fmt"
	"unsafe"
)

// The Message struct used for message passing
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
		payload: data["data"],
	}

	//mdata, ok := data["metadata"].(map[string]interface{}),

	return &msg
}

// The Transport interface which user needs to implement
type Transport interface {
	// To get the RawCppTransport from base transport
	RawCppTransport() unsafe.Pointer
	// InitBase is called from framework to do initialization work on BaseTransport
	InitBase(unsafe.Pointer)
	// Start is called when transport is to be started
	Start()
	// Shutdown is called to shutdown the transport
	Shutdown()
	// HostReady is called to notify transport that host is ready to receive messages
	HostReady()
	// DeliverMessageTowardsTransport is called to deliver message to transport
	DeliverMessageTowardsTransport(msg *Message)
	// DeliverMessageTowardsHost is called to deliver message to host
	DeliverMessageTowardsHost(msg *Message)
}

type TransportCreateFuncType func() Transport

var TransportCreateFunc TransportCreateFuncType

var TransportObject Transport

// The BaseTransport which user need to embed when creating a new transport.
// User should create empty value when creating its transport instance.
// Fields of this transport will be set by the framework.
type BaseTransport struct {
	// Pointer to underlying C++ transport object. Will be set by framework.
	cppTransport unsafe.Pointer
}

// RawCppTransport returns underlying C++ transport object.
func (t *BaseTransport) RawCppTransport() unsafe.Pointer {
	return t.cppTransport
}

// InitBase must be called by user from create_transport function after creating
func (t *BaseTransport) InitBase(ptr unsafe.Pointer) {
	t.cppTransport = ptr
	fmt.Println("BaseTransport initialised")
}

// DeliverMessageTowardsHost must be called by user to send message to host
func (t *BaseTransport) DeliverMessageTowardsHost(msg *Message) {
	fmt.Println("DeliverMessageTowardsHost called")
	gobuf := serializeMsg(msg)
	fmt.Printf("DeliverMessageTowardsHost deliver: %v\n", gobuf)
	C.c_callback(t.RawCppTransport(), unsafe.Pointer(&gobuf[0]), C.int(len(gobuf)))
}

//export go_transport_create
func go_transport_create(obj unsafe.Pointer) {
	fmt.Printf("go_transport_create called: %v\n", obj)
	if TransportObject != nil {
		fmt.Println("Go Transport seems to be already created")
	}
	// call user function to create go transport instance
	TransportObject = TransportCreateFunc()
	TransportObject.InitBase(obj)
}

//export go_transport_start
func go_transport_start() {
	fmt.Println("go_transport_start called")
	TransportObject.Start()
}

//export go_transport_shutdown
func go_transport_shutdown() {
	fmt.Println("go_transport_shutdown called")
	TransportObject.Shutdown()
}

//export go_transport_hostready
func go_transport_hostready() {
	fmt.Println("go_transport_hostready called")
	TransportObject.HostReady()
}

//export go_transport_deliverMessageTowardsTransport
func go_transport_deliverMessageTowardsTransport(buf unsafe.Pointer, bufLen C.int) {
	fmt.Println("go_transport_deliverMessageTowardsTransport called")
	gobuf := C.GoBytes(buf, bufLen)
	fmt.Printf("go_transport_deliverMessageTowardsTransport deliver: %v\n", gobuf)
	msg := deserializeMsg(gobuf)
	TransportObject.DeliverMessageTowardsTransport(msg)
}

// export go_transport_init
func go_transport_init() {

}
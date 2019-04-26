package lib

// #include "c_go_interface.h"
import "C"

import (
	"encoding/json"
	"fmt"
	"sync"
	"unsafe"
)

// The Message struct used for message passing
type Message struct {
	Payload  interface{}
	Metadata map[string]interface{}
}

func serializeMsg(msg *Message) []byte {
	var data = map[string]interface{}{
		"data":     msg.Payload,
		"metadata": msg.Metadata,
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
		Payload:  data["data"],
		Metadata: data["metadata"].(map[string]interface{}),
	}

	return &msg
}

// The Transport interface which user needs to implement
type Transport interface {
	// To get the RawCppTransport from base transport
	RawCppTransport() unsafe.Pointer
	// InitBase is called from framework to do initialization work on BaseTransport
	InitBase(unsafe.Pointer, map[string]interface{})
	GetConfig() map[string]interface{}
	// ======== User functions ==============
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

type TransportCreateFuncType func(config map[string]interface{}) Transport

var TransportCreateFunc TransportCreateFuncType

type ptrToObjMapper struct {
	mtx             sync.RWMutex
	transportObject map[unsafe.Pointer]Transport
}

var mapper = ptrToObjMapper{sync.RWMutex{}, map[unsafe.Pointer]Transport{}}

func (m *ptrToObjMapper) setMapping(p unsafe.Pointer, t Transport) {
	m.mtx.Lock()
	defer m.mtx.Unlock()
	m.transportObject[p] = t
}

func (m *ptrToObjMapper) getMapping(p unsafe.Pointer) Transport {
	m.mtx.RLock()
	defer m.mtx.RUnlock()
	return m.transportObject[p]
}

// The BaseTransport which user need to embed when creating a new transport.
// User should create empty value when creating its transport instance.
// Fields of this transport will be set by the framework.
type BaseTransport struct {
	// Pointer to underlying C++ transport object. Will be set by framework.
	cppTransport unsafe.Pointer
	config       map[string]interface{}
}

// RawCppTransport returns underlying C++ transport object.
func (t *BaseTransport) RawCppTransport() unsafe.Pointer {
	return t.cppTransport
}

func (t *BaseTransport) GetConfig() map[string]interface{} {
	return t.config
}

// InitBase must be called by user from create_transport function after creating
func (t *BaseTransport) InitBase(ptr unsafe.Pointer, config map[string]interface{}) {
	t.cppTransport = ptr
	t.config = config
}

// DeliverMessageTowardsHost must be called by user to send message to host
func (t *BaseTransport) DeliverMessageTowardsHost(msg *Message) {
	gobuf := serializeMsg(msg)
	C.c_callback(t.RawCppTransport(), unsafe.Pointer(&gobuf[0]), C.int(len(gobuf)))
}

//export go_transport_create
func go_transport_create(obj unsafe.Pointer, buf unsafe.Pointer, bufLen C.int) {
	gobuf := C.GoBytes(buf, bufLen)
	var config map[string]interface{}
	err := json.Unmarshal(gobuf, &config)
	fmt.Printf("go_transport_create called with config: %s\n", config)
	if err != nil {
		fmt.Printf("go_transport_create failed to populate config: %s\n", config)
	}
	TransportObject := mapper.getMapping(obj)
	if mapper.getMapping(obj) != nil {
		fmt.Println("Go Transport seems to be already created")
	}
	// call user function to create go transport instance
	TransportObject = TransportCreateFunc(config)
	TransportObject.InitBase(obj, config)
	mapper.setMapping(obj, TransportObject)
}

//export go_transport_start
func go_transport_start(obj unsafe.Pointer) {
	TransportObject := mapper.getMapping(obj)
	TransportObject.Start()
}

//export go_transport_shutdown
func go_transport_shutdown(obj unsafe.Pointer) {
	TransportObject := mapper.getMapping(obj)
	TransportObject.Shutdown()
}

//export go_transport_hostready
func go_transport_hostready(obj unsafe.Pointer) {
	TransportObject := mapper.getMapping(obj)
	TransportObject.HostReady()
}

//export go_transport_deliverMessageTowardsTransport
func go_transport_deliverMessageTowardsTransport(obj unsafe.Pointer, buf unsafe.Pointer, bufLen C.int) {
	// fmt.Println("go_transport_deliverMessageTowardsTransport called")
	gobuf := C.GoBytes(buf, bufLen)
	msg := deserializeMsg(gobuf)
	TransportObject := mapper.getMapping(obj)
	TransportObject.DeliverMessageTowardsTransport(msg)
}

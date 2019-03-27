package user

import (
	"fmt"

	"../lib"
)

// ============================================ User Code ===================================================

// MyTransport implements custom transport
type MyTransport struct {
	lib.BaseTransport
}

func (*MyTransport) Start() {
	fmt.Println("MyTransport Start")
}

func (*MyTransport) Shutdown() {
	fmt.Println("MyTransport Shutdown")
}

func (*MyTransport) HostReady() {
	fmt.Println("MyTransport hostready")
}

func (t *MyTransport) DeliverMessageTowardsTransport(msg *lib.Message) {
	fmt.Printf("MyTransport DeliverMessageTowardsTransport: %s", msg)
	t.DeliverMessageTowardsHost(msg)
}

func MyTransportCreate() lib.Transport {
	return &MyTransport{lib.BaseTransport{}}
}

// func init() {
// 	fmt.Println("User Init called")
// 	// User need to set the TransportCreateFunc function
// 	lib.TransportCreateFunc = MyTransportCreate
// }

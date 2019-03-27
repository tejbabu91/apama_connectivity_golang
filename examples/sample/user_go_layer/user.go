package user

import (
	"fmt"

	lib "github.com/tejbabu91/apama_connectivity_golang/lib"
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
	fmt.Printf("MyTransport DeliverMessageTowardsTransport: %v, %v\n", msg, t.GetConfig())
	t.DeliverMessageTowardsHost(msg)
}

func MyTransportCreate(config map[string]interface{}) lib.Transport {
	fmt.Printf("MyTransport created with config: %v\n", config)
	return &MyTransport{lib.BaseTransport{}}
}

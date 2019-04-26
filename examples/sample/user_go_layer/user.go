package user

import (
	"fmt"

	lib "github.com/tejbabu91/apama_connectivity_golang/lib"
)

// ============================================ User Code ===================================================

// EchoTransport implements custom transport
type EchoTransport struct {
	lib.BaseTransport
}

func (*EchoTransport) Start() {
	fmt.Println("EchoTransport Start")
}

func (*EchoTransport) Shutdown() {
	fmt.Println("EchoTransport Shutdown")
}

func (*EchoTransport) HostReady() {
	fmt.Println("EchoTransport hostready")
}

func (t *EchoTransport) DeliverMessageTowardsTransport(msg *lib.Message) {
	fmt.Printf("EchoTransport DeliverMessageTowardsTransport: %v, %v\n", msg, t.GetConfig())
	t.DeliverMessageTowardsHost(msg)
}

func EchoTransportCreate(config map[string]interface{}) lib.Transport {
	fmt.Printf("EchoTransport created with config: %v\n", config)
	return &EchoTransport{lib.BaseTransport{}}
}

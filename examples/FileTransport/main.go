package main

import (
	ft "./transport"
	lib "github.com/tejbabu91/apama_connectivity_golang/lib"
)

// This function must be present to register user code init func to lib.
// the user code inti func will be called by apama on plugin load.
func init() {
	lib.TransportCreateFunc = ft.FileTransportCreate
}

func main() {} // Required but ignored

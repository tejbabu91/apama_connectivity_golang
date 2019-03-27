package main

import (
	"./lib"
	user "./user_go_layer"
)

func init() {
	lib.TransportCreateFunc = user.MyTransportCreate
}

// TODO: We need some mechanism for user code to update the global variable
//TransportCreateFuncType = &MyTransportCreate

func main() {} // Required but ignored

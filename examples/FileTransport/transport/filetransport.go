package transport

import (
	"bufio"
	"fmt"
	"os"

	lib "github.com/tejbabu91/apama_connectivity_golang/lib"
)

// ============================================ User Code ===================================================

// FileTransport implements custom transport
type FileTransport struct {
	lib.BaseTransport
	fileNameToRead  string
	fileNameToWrite string
	fileToRead      *os.File
	fileToWrite     *os.File
	writer          *bufio.Writer
	reader          *bufio.Reader
}

func (t *FileTransport) Start() {
	fmt.Println("FileTransport Start")
	if t.fileNameToWrite != "" {
		f, err := os.Create(t.fileNameToWrite)
		if err != nil {
			fmt.Printf("ERROR !!! Unable to open or create file for writing: %s, %s\n", err, t.fileToWrite)
		} else {
			t.fileToWrite = f
			t.writer = bufio.NewWriter(f)
			fmt.Printf("File to write is: %s\n", f.Name())
		}
	}

	if t.fileNameToRead != "" {
		f, err := os.Open(t.fileNameToRead)
		if err != nil {
			fmt.Printf("ERROR !!! Unable to open file for reading: %s, %s\n", err, t.fileToRead)
		} else {
			t.fileToRead = f
			t.reader = bufio.NewReader(f)
		}
	}
}

func (t *FileTransport) Shutdown() {
	fmt.Println("FileTransport Shutdown")
	if t.fileToRead != nil {
		t.fileToRead.Close()
	}
	if t.fileToWrite != nil {
		t.fileToWrite.Close()
	}
}

func (t *FileTransport) HostReady() {
	fmt.Println("FileTransport hostready, reading the file")
	if t.reader != nil {
		go func() {
			for {
				line, _, err := t.reader.ReadLine()
				if err != nil {
					break
				}
				msg := lib.Message{string(line), make(map[string]interface{})}
				t.DeliverMessageTowardsHost(&msg)
			}
		}()
	}
}

func (t *FileTransport) DeliverMessageTowardsTransport(msg *lib.Message) {
	fmt.Printf("FileTransport DeliverMessageTowardsTransport: %s\n", msg)
	if t.writer != nil {

		line, ok := msg.Payload.(string)
		if !ok {
			line = fmt.Sprintf("%v", line)
		}
		fmt.Printf("Going to write: %s\n", line)
		n, err := t.writer.Write([]byte(line + "\n"))
		if err != nil {
			fmt.Printf("Failed to write line: %s\n", err)
		} else {
			fmt.Printf("Written: %s\n", n)
		}
		_ = t.writer.Flush()
	} else {
		fmt.Println("Not writing, writer null")
	}
}

func FileTransportCreate(config map[string]interface{}) lib.Transport {
	fmt.Printf("FileTransport created with config: %v\n", config)

	var fileToRead string
	var fileToWrite string
	fileToRead, _ = config["readFile"].(string)
	fileToWrite, _ = config["writeFile"].(string)

	t := &FileTransport{fileNameToRead: fileToRead, fileNameToWrite: fileToWrite}
	return t
}

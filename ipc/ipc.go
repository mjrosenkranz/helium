package ipc

import (
	"log"
	"net"
	"os"
)

// SOCKPATH is the path to the ipc socket
const (
	SOCKPATH = "/tmp/helium.sock"
	MSGLEN   = 48
)

func SendMsg(msg string) {
	conn, err := net.Dial("unix", SOCKPATH)
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()

	_, err = conn.Write([]byte(msg))
	if err != nil {
		log.Fatal(err)
	}

	reply := make([]byte, MSGLEN)
	_, err = conn.Read(reply)
	if err != nil {
		log.Fatal(err)
	}

	log.Println(string(reply))

}

func RecieveMsg() {
	os.Remove(SOCKPATH)

	l, err := net.Listen("unix", SOCKPATH)
	if err != nil {
		log.Fatalf("could not recieve control events because %s\n", err)
		return
	}
	defer l.Close()

	for {
		c, err := l.Accept()
		if err != nil {
			log.Printf("Error accepting IPC event conn: %s", err)
			continue
		}
		log.Println("New connection")
		go func(conn net.Conn) {
			msg := make([]byte, MSGLEN)
			_, err = conn.Read(msg)
			if err != nil {
				log.Fatal(err)
			}
			log.Println(string(msg))

			_, err = c.Write([]byte("bleep"))
			if err != nil {
				log.Printf("Could not write back to socket because %s\n", err)
			}
		}(c)
	}
}

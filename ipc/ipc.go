package ipc

import (
	"fmt"
	"log"
	"net"
	"os"
	"strings"
)

// SOCKPATH is the path to the ipc socket
const (
	SOCKPATH = "/tmp/helium.sock"
	MSGLEN   = 80
	EOM      = "EOM"
)

// Msg is wrapper for a ipc message to sent to event handlers
type Msg struct {
	Conn net.Conn
	Str  string
}

// NewIpcMsg returns a new IpcMsg from the given connection and string
func NewIpcMsg(c net.Conn, s string) Msg {
	return Msg{c, s + EOM}
}

// CtrlMsg sends a message from the ipc client to the windowmanager
func CtrlMsg(m string) {
	conn, err := net.Dial("unix", SOCKPATH)
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()

	msg := Msg{conn, m}
	Send(msg)

	reply := ""

	for {
		tmp := make([]byte, MSGLEN)
		_, err = conn.Read(tmp)
		if err != nil {
			log.Fatal(err)
		}

		part := string(tmp)

		if strings.Contains(part, EOM) {
			ss := strings.Split(part, EOM)
			reply += ss[0]
			break
		}

		reply += part
	}

	fmt.Println(reply)
}

// RecieveMsg sends messages read in a gorutine to the main wm process
func RecieveMsg(msgch chan Msg) {
	os.Remove(SOCKPATH)

	// open socket
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
		msg := make([]byte, MSGLEN)
		_, err = c.Read(msg)
		if err != nil {
			log.Fatal(err)
		}
		msgch <- Msg{c, strings.Trim(string(msg), "\x00")}
	}
}

func Send(msg Msg) {
	// b := []byte(msg.Msg)
	// b = append(b, make([]byte, MSGLEN-len(b))...)
	_, err := msg.Conn.Write([]byte(msg.Str))
	if err != nil {
		log.Printf("Could not write back to socket because %s\n", err)
	}
}

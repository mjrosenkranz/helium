package ipc

import (
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

// IpcMsg is wrapper for a ipc message to sent to event handlers
type IpcMsg struct {
	Conn net.Conn
	Msg  string
}

// CtrlMsg sends a message from the ipc client to the windowmanager
func CtrlMsg(m string) {
	conn, err := net.Dial("unix", SOCKPATH)
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()

	msg := IpcMsg{conn, m}
	Send(msg)

	for {
		tmp := make([]byte, MSGLEN)
		_, err = conn.Read(tmp)
		if err != nil {
			log.Fatal(err)
		}

		reply := string(tmp)

		if strings.Contains(reply, EOM) {
			ss := strings.Split(reply, EOM)
			log.Println(ss[0])
			return
		}
		log.Println(reply)
	}

}

// RecieveMsg sends messages read in a gorutine to the main wm process
func RecieveMsg(msgch chan IpcMsg) {
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
		msgch <- IpcMsg{c, string(msg)}
	}
}

func Send(msg IpcMsg) {
	// b := []byte(msg.Msg)
	// b = append(b, make([]byte, MSGLEN-len(b))...)
	_, err := msg.Conn.Write([]byte(msg.Msg))
	if err != nil {
		log.Printf("Could not write back to socket because %s\n", err)
	}
}

package ipc

import (
	"strconv"
	"fmt"
	"io"
	"net"
	"os"
	"strings"

	"github.com/xen0ne/helium/logger"
)

// SOCKPATH is the path to the ipc socket
const (
	SOCKPATH = "/tmp/%s.helium.sock"
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
	return Msg{c, s}
}

// CtrlMsg sends a message from the ipc client to the windowmanager
func CtrlMsg(m string) {
	conn, err := net.Dial("unix", getSockPath())
	if err != nil {
		logger.Log.Fatal(err)
	}
	defer conn.Close()

	msg := Msg{conn, m}
	Send(msg)

	reply := ""

	for {
		tmp := make([]byte, MSGLEN)
		_, err = conn.Read(tmp)
		if err != nil {
			if err == io.EOF {
				break
			} else {
				logger.Log.Fatal(err)
			}
		}
		part := string(tmp)
		reply += part

		if strings.Contains(part, "failed") {
			logger.Log.Fatal(
				strings.Trim(
					strings.Split(part, "failed")[1], "\n"))
		}
	}

	fmt.Print(reply)
	os.Exit(0)
}

// RecieveMsg sends messages read in a gorutine to the main wm process
func RecieveMsg(msgch chan Msg) {
	// set env
	err := os.Setenv("HELIUMPID", strconv.Itoa(os.Getpid()));
	if  err != nil {
		logger.Log.Fatal(err)
	}
	fmt.Println(os.Getpid())
	// delete an old socket if that exists
	os.Remove(getSockPath())

	// open socket
	l, err := net.Listen("unix", getSockPath())
	if err != nil {
		logger.Log.Fatalf("could not recieve control events because %s\n", err)
		return
	}
	defer l.Close()

	for {
		c, err := l.Accept()
		if err != nil {
			logger.Log.Printf("Error accepting IPC event conn: %s", err)
			continue
		}
		logger.Log.Println("New connection")
		msg := make([]byte, MSGLEN)
		_, err = c.Read(msg)
		if err != nil {
			logger.Log.Fatal(err)
		}
		msgch <- Msg{c, strings.Trim(string(msg), "\x00")}
	}
}

func Send(msg Msg) {
	// b := []byte(msg.Msg)
	// b = append(b, make([]byte, MSGLEN-len(b))...)
	_, err := msg.Conn.Write([]byte(msg.Str))
	if err != nil {
		logger.Log.Printf("Could not write back to socket because %s\n", err)
	}
}

func getSockPath() string {
	return "/tmp/helium." + os.Getenv("HELIUMPID") + ".sock"
}

package wm

import (
	"errors"
	"fmt"
	"log"
	"strings"

	"github.com/xen0ne/helium/ipc"
)

// HandleIpcMsg takes an ipcMsg and does whatever
// requred before returning a new response
func HandleIpcMsg(m ipc.Msg) ipc.Msg {
	// parse the message
	err := parseMsg(m.Str)
	if err != nil {
		log.Println(err)
	}
	// if cannot parse then send error msg back

	// if can parse then do the action or send it off
	// respond with sucess or not
	return ipc.NewIpcMsg(m.Conn, "success")
}

func parseMsg(m string) error {
	args := strings.Split(m, " ")
	switch len(args) {
	case 1:
		cmd := args[0]
		if "close" == cmd {
			f := GetFocused()
			if f != nil {
				fmt.Println(f)
				f.Close()
			}
		} else {
			return errors.New(fmt.Sprintf("Command %s not found", cmd))
		}
	case 2:
		fmt.Println("two")
	case 3:
		fmt.Println("three")
	default:
		return errors.New("Too many options")
	}
	return nil
}

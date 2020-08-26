package wm

import (
	"errors"
	"fmt"
	"strings"

	"github.com/xen0ne/helium/ipc"
)

// HandleIpcMsg takes an ipcMsg and does whatever
// requred before returning a new response
func HandleIpcMsg(m ipc.Msg) ipc.Msg {
	// parse the message
	err := parseMsg(m.Str)
	// if cannot parse then send error msg back
	if err != nil {
		return ipc.NewIpcMsg(m.Conn, err.Error())
	}

	// if can parse then do the action or send it off
	// respond with success or not
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
			return fmt.Errorf("Command %s not found", cmd)
		}
	case 2:
		switch args[0] {
		case "focus":
			return handleFocusMsg(args[1])
		default:
			return nil
		}
	case 3:
		fmt.Println("three")
	default:
		return errors.New("Too many options")
	}
	return nil
}

func handleFocusMsg(s string) error {
	switch s {
	case "next":
		wm.FocusNext()
	case "prev":
		wm.FocusPrev()

	default:
		return fmt.Errorf("%s is not a valid focus command")
	}
}

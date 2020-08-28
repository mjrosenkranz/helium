package wm

import (
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/xen0ne/helium/ipc"
)

// HandleIpcMsg takes an ipcMsg and does whatever
// requred before returning a new response
func HandleIpcMsg(m ipc.Msg) ipc.Msg {
	// parse the message
	msg, err := parseMsg(m.Str)
	// if cannot parse then send error msg back
	if err != nil {
		return ipc.NewIpcMsg(m.Conn, err.Error())
	}

	// if can parse then do the action or send it off
	// respond with success or not
	return ipc.NewIpcMsg(m.Conn, msg)
}

func parseMsg(m string) (string, error) {
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
			return "", fmt.Errorf("Command %s not found", cmd)
		}
	case 2:
		switch args[0] {
		case "focus":
			return handleFocusMsg(args[1])
		case "toggle":
			t, err := strconv.ParseInt(args[1], 0, 0)
			if err != nil {
				return err.Error(), err
			}
			if !IsValidTag(int(t)) {
				return "", fmt.Errorf("%d is not a valid tag", t)
			}
			ToggleTag(int(t))
			return "", nil
		case "tag":
			t, err := strconv.ParseInt(args[1], 0, 0)
			if err != nil {
				return err.Error(), err
			}
			if !IsValidTag(int(t)) {
				return "", fmt.Errorf("%d is not a valid tag", t)
			}
			if GetFocused() != nil {
				GetFocused().SetTag(int(t))
			}
		default:
			return "", fmt.Errorf("%s is not a command", args[0])
		}
	case 3:
		fmt.Println("three")
	default:
		return "", errors.New("Too many options")
	}
	return "success", nil
}

func handleFocusMsg(s string) (string, error) {
	switch s {
	case "next":
		FocusNext()
		return "", nil
	case "prev":
		FocusPrev()
		return "", nil
	default:
		return "", fmt.Errorf("%s is not a valid focus command", s)
	}
}

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
func HandleIpcMsg(m ipc.Msg) {
	// parse the message
	err := parseMsg(m)
	// if cannot parse then send error msg back
	if err != nil {
		m.Str = "failed\n" + err.Error()
		ipc.Send(m)
		m.Conn.Close()
		return
	}

	m.Conn.Close()
}

func parseMsg(m ipc.Msg) error {
	args := strings.Split(m.Str, " ")
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
		case "toggle":
			t, err := strconv.ParseInt(args[1], 0, 0)
			if err != nil {
				return err
			}

			if !IsValidTag(int(t)) {
				return fmt.Errorf("%d is not a valid tag", t)
			} else {
				if int(t) == 0 {
					return fmt.Errorf("%d is not a valid tag", t)
				}
			}
			ToggleTag(int(t))
			return nil
		case "tag":
			t, err := strconv.ParseInt(args[1], 0, 0)
			if err != nil {
				return err
			}
			if !IsValidTag(int(t)) {
				return fmt.Errorf("%d is not a valid tag", t)
			}
			if GetFocused() != nil {
				GetFocused().SetTag(int(t))
			}
		case "print":
			ret := ""
			switch args[1] {
			case "tags":
				for i, t := range Tags {
					ret += fmt.Sprintf("tag %d\n"+
						"mapped: %v\n"+
						"frames:%+v\n", i, t.IsMapped, t.Frames())
				}
				ipc.Send(ipc.Msg{m.Conn, ret})
				return nil
			case "queue":
				for _, f := range FocusQ {
					ret += fmt.Sprintf("%+v\n", f)
				}
				ipc.Send(ipc.Msg{m.Conn, ret})
				return nil
			default:
				return fmt.Errorf("cannot print %s", args[0])
			}
		default:
			return fmt.Errorf("%s is not a command", args[0])
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
		FocusNext()
		return nil
	case "prev":
		FocusPrev()
		return nil
	default:
		return fmt.Errorf("%s is not a valid focus command", s)
	}
}

package wm

import (
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/xen0ne/helium/config"
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
			return handleToggle(args[1])
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
			return handlePrint(m, args)
		default:
			return fmt.Errorf("%s is not a command", args[0])
		}
	case 3:
		if args[0] == "config" {
			return handleConfig(m, args[1], args[2])
		}
		return fmt.Errorf("Cannot parse %s", args[1])
	default:
		return errors.New("Too many options")
	}
	return nil
}

func handleConfig(m ipc.Msg, s, v string) error {
	switch s {
	case "bar_focused_color":
		err := config.ValidateAndModifyColor(&config.Bar.Focused, v)
		if err != nil {
			return err
		}
		UpdateFocused()
	case "bar_unfocused_color":
		err := config.ValidateAndModifyColor(&config.Bar.Unfocused, v)
		if err != nil {
			return err
		}
		UpdateUnfocused()
	case "bar_text_focused_color":
		err := config.ValidateAndModifyColor(&config.Bar.TextFocused, v)
		if err != nil {
			return err
		}
		UpdateFocused()
	case "bar_text_unfocused_color":
		err := config.ValidateAndModifyColor(&config.Bar.TextUnfocused, v)
		if err != nil {
			return err
		}
		UpdateUnfocused()
	case "bar_font":
		err := config.ChangeFont(v)
		if err != nil {
			return err
		}
		UpdateFocused()
		UpdateUnfocused()
	case "bar_font_size":
		err := config.ChangeFontSize(v)
		if err != nil {
			return err
		}
		UpdateFocused()
		UpdateUnfocused()
	default:
		return fmt.Errorf("%s is not a valid config variable", s)
	}

	return nil
}

func handlePrint(m ipc.Msg, args []string) error {
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
}

func handleToggle(s string) error {
	t, err := strconv.ParseInt(s, 0, 0)
	if err != nil {
		return err
	}

	if !IsValidTag(int(t)) {
		return fmt.Errorf("%d is not a valid tag", t)
	}

	if int(t) == 0 {
		return fmt.Errorf("%d is not a valid tag", t)
	}

	ToggleTag(int(t))
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

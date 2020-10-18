package wm

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/BurntSushi/xgbutil/xrect"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/consts"
	"github.com/xen0ne/helium/grid"
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
	switch args[0] {
	case "snap":
		f := GetFocused()
		if f != nil {
			f.SnapToGrid(grid.New(xrect.New(0, 0, 800, 600), 3, 4, 10))
		}
	case "close":
		f := GetFocused()
		if f != nil {
			fmt.Println(f)
			f.Close()
		}
	case "focus":
		if len(args) < 2 {
			return fmt.Errorf("Not enough arguments to %s", args[0])
		}
		return handleFocusMsg(args[1])
	case "toggle":
		if len(args) < 2 {
			return fmt.Errorf("Not enough arguments to %s", args[0])
		}
		return handleToggle(args[1])
	case "tag":
		if len(args) < 2 {
			return fmt.Errorf("Not enough arguments to %s", args[0])
		}
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
		if len(args) < 2 {
			return fmt.Errorf("Not enough arguments to %s, requires 2", args[0])
		}
		return handlePrint(m, args)
	case "config":
		if len(args) < 3 {
			return fmt.Errorf("Not enough arguments to %s, requires at least 2", args[0])
		}
		// TODO make just an array
		return handleConfig(m, args[1], args[2])
	case "resize":
		if len(args) < 3 {
			return fmt.Errorf("Not enough arguments to %s, requires 2", args[0])
		}

		nx, err := strconv.ParseInt(args[1], 0, 0)
		if err != nil {
			return err
		}

		ny, err := strconv.ParseInt(args[2], 0, 0)
		if err != nil {
			return err
		}

		f := GetFocused()
		if f != nil {
			f.Resize(int(nx), int(ny))
		}
	case "resize_rel":
		if len(args) < 3 {
			return fmt.Errorf("Not enough arguments to %s, requires 2", args[0])
		}

		n, err := strconv.ParseInt(args[1], 0, 0)
		if err != nil {
			return err
		}

		dir, err := consts.StringToDir(args[2])
		if err != nil {
			return err
		}

		f := GetFocused()
		if f != nil {
			f.ResizeRel(int(n), dir)
		}
	default:
		return fmt.Errorf("Command %s not found", args[0])
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
	case "bar_height":
		err := config.ValidateAndModifyInt(&config.Bar.Height, v)
		if err != nil {
			return err
		}
		UpdateFocused()
		UpdateUnfocused()
	case "bar_text_offset":
		err := config.ValidateAndModifyInt(&config.Bar.TextOffset, v)
		if err != nil {
			return err
		}
		UpdateFocused()
		UpdateUnfocused()
	case "bar_text_center":
		err := config.ValidateAndModifyBool(&config.Bar.CenterText, v)
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

package frame

import (
	"fmt"
	"log"

	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/wm"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/ewmh"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/BurntSushi/xgbutil/xprop"
	"github.com/BurntSushi/xgbutil/xwindow"
)

// A Frame is a window which holds a client and its decorations
type Frame struct {
	*xwindow.Window
	client       *xwindow.Window
	bar          *Bar
	state        State
	x, y, px, py int
	tag          int
}

// New creates a new client from a map event
func New(c *xwindow.Window) *Frame {
	wm.X.Grab()
	defer wm.X.Ungrab()

	// If this is an override redirect, skip...
	attrs, err := xproto.GetWindowAttributes(wm.X.Conn(), c.Id).Reply()
	if err != nil {
		log.Printf("Could not get window attributes for '%x': %s",
			c.Id, err)
	} else {
		if attrs.OverrideRedirect {
			log.Printf("Not managing override redirect window %x", c.Id)
			c.Map()
			return nil
		}
	}

	// make sure this is the kind of window we want to manage
	wtype, err := ewmh.WmWindowTypeGet(wm.X, c.Id)
	if err != nil {
		log.Println(err)
	}
	for _, t := range wtype {
		if t == "_NET_WM_WINDOW_TYPE_TOOLBAR" ||
			t == "_NET_WM_WINDOW_TYPE_DOCK" ||
			t == "_NET_WM_WINDOW_TYPE_DESKTOP" {
			log.Printf("Not managing window of type: %s\n", t)
			c.Map()
			return nil
		}
	}

	f := Frame{}
	f.client = c

	// need the geometry
	g, err := f.client.Geometry()
	if err != nil {
		log.Printf("Cannot get geometry for %x\n", f.client.Id)
	}

	f.Window, err = xwindow.Generate(wm.X)
	if err != nil {
		log.Printf("Could not create new window for %x\n", f.Id)
	}
	f.Create(wm.X.RootWin(),
		g.X(), g.Y(),
		g.Width(), g.Height()+config.Bar.Height,
		xproto.CwBackPixel|xproto.CwEventMask,
		config.Bar.Focused, xproto.EventMaskButtonPress|
			xproto.EventMaskButtonRelease|
			xproto.EventMaskButtonMotion|
			xproto.EventMaskSubstructureNotify)

	f.x = g.X()
	f.y = g.Y()

	// set the tag to initial
	f.tag = 0

	f.Map()

	err = xproto.ReparentWindowChecked(wm.X.Conn(),
		f.client.Id, f.Id, 0, int16(config.Bar.Height)).Check()
	if err != nil {
		log.Println("Could not reparent window")
	}

	f.addClientEvents()

	return &f
}

// Map maps all the components of a Frame
func (f *Frame) Map() {
	f.Window.Map()
	f.client.Map()
	if f.bar != nil {
		f.bar.Map()
	}
}

// Returns the id of a frame
func (f *Frame) FrameId() xproto.Window {
	return f.Id
}

// String returns a string representation of a Frame
func (f *Frame) String() string {
	return fmt.Sprintf("frame client: %x", f.client.Id)
}

// Focus alerts X of the Frame we want to focus and provides input focus
func (f *Frame) Focus() {
	err := ewmh.ActiveWindowSet(wm.X, f.Id)
	if err != nil {
		log.Printf("Cannot set active window to %s\n", f.String())
	}
	f.client.FocusParent(xproto.TimeCurrentTime)
	f.Stack(xproto.StackModeAbove)
	f.state = focused

	f.UpdateBar()

	// if there is a previously focued, tell them to unfocus
	if len(wm.FoucusQ) > 0 && wm.FoucusQ[0].FrameId() != f.FrameId() {
		wm.FoucusQ[0].Unfocus()
	}

	// remove from focus queue
	wm.FoucusQ = wm.RemoveFrame(f, wm.FoucusQ)
	// add to focus queue
	wm.FoucusQ = wm.AddFrame(f, wm.FoucusQ)
}

func (f *Frame) Unfocus() {
	f.state = unfocused
	f.UpdateBar()
}

// Contains tells us if the given frame has a window of the given id
func (f *Frame) Contains(id xproto.Window) bool {
	return f.Id == id || f.client.Id == id || f.bar.Id == id
}

// Id returns the id of the frame's parent
// func (f *Frame) Id() xproto.Window {
// 	return f.parent.Id
// }

// Close gracefully kills the client of the current frame
func (f *Frame) Close() {

	wm_protocols, err := xprop.Atm(wm.X, "WM_PROTOCOLS")
	if err != nil {
		log.Println(err)
		return
	}

	wm_del_win, err := xprop.Atm(wm.X, "WM_DELETE_WINDOW")
	if err != nil {
		log.Println(err)
		return
	}

	cm, err := xevent.NewClientMessage(32, f.client.Id, wm_protocols,
		int(wm_del_win))
	if err != nil {
		log.Println(err)
		return
	}

	err = xproto.SendEventChecked(wm.X.Conn(), false, f.client.Id, 0,
		string(cm.Bytes())).Check()
	if err != nil {
		log.Printf("Could not send WM_DELETE_WINDOW "+
			"ClientMessage because: %s", err)
	}
}

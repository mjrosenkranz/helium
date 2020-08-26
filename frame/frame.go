package frame

import (
	"fmt"
	"log"

	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/wm"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/ewmh"
	"github.com/BurntSushi/xgbutil/icccm"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/BurntSushi/xgbutil/xprop"
	"github.com/BurntSushi/xgbutil/xwindow"
)

// A Frame is a window which holds a client and its decorations
type Frame struct {
	*xwindow.Window
	client     *xwindow.Window
	bar        *Bar
	state      State
	x, y, w, h int
	px, py     int
	resizedir  Direction
	tag        int
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
			c.Stack(xproto.StackModeAbove)
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
	// check if window is a submenu
	t, err := icccm.WmTransientForGet(wm.X, c.Id)
	if err != nil {
		log.Printf("Could not get transient because: %s\n", err)
	} else {
		log.Printf("%+v\n", t)
	}

	f := Frame{}
	f.client = c

	// need the geometry
	g, err := f.client.Geometry()
	if err != nil {
		log.Printf("Cannot get geometry for %x, bc: %s\n", f.client.Id, err)
	}

	x := g.X()
	y := g.Y()
	w := g.Width()
	h := g.Height()

	if x < 0 {
		x = 0
	}
	if y < 0 {
		y = 0
	}

	// get the normal hints
	nh, err := icccm.WmNormalHintsGet(wm.X, c.Id)
	if err != nil {
		log.Printf("Could not get normal hints because %s\n", err)
	} else {
		if nh.Flags&icccm.SizeHintPMinSize > 0 {
			bw := int(nh.MinWidth)
			bh := int(nh.MinHeight)
			if bw > w {
				w = bw
			}
			if bh > h {
				h = bh
			}
		}
	}

	f.client.Resize(w, h)

	f.x = x
	f.y = y
	f.w = w
	f.h = h + config.Bar.Height

	f.Window, err = xwindow.Generate(wm.X)
	if err != nil {
		log.Printf("Could not create new window for %x\n", f.Id)
	}
	f.Create(wm.X.RootWin(),
		f.x, f.y,
		f.w, f.h,
		xproto.CwBackPixel|xproto.CwEventMask,
		config.Bar.Focused, xproto.EventMaskButtonPress|
			xproto.EventMaskButtonRelease|
			xproto.EventMaskButtonMotion|
			xproto.EventMaskSubstructureNotify)

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

// FrameId returns the id of a frame
func (f *Frame) FrameId() xproto.Window {
	return f.Id
}

// String returns a string representation of a Frame
func (f *Frame) String() string {
	return fmt.Sprintf("frame client: %x", f.client.Id)
}

// Focus alerts X of the Frame we want to focus and provides input focus
func (f *Frame) Focus() {
	err := ewmh.ActiveWindowSet(wm.X, f.client.Id)
	if err != nil {
		log.Printf("Cannot set active window to %s\n", f.String())
	}
	f.Stack(xproto.StackModeAbove)
	f.state = focusedState

	f.client.Focus()

	f.UpdateBar()

	// if there is a previously focued, tell them to unfocus
	if wm.GetFocused() != nil && !wm.IsFocused(f) {
		wm.GetFocused().Unfocus()
	}

	// remove from focus queue
	wm.FocusQ = wm.RemoveFrame(f, wm.FocusQ)
	// add to focus queue
	wm.FocusQ = wm.AddFrame(f, wm.FocusQ)
}

// Unfocus updates the state and the bar of the Frame
func (f *Frame) Unfocus() {
	f.state = unfocusedState
	f.UpdateBar()
}

// Contains tells us if the given frame has a window of the given id
func (f *Frame) Contains(id xproto.Window) bool {
	return f.Id == id || f.client.Id == id || f.bar.Id == id
}

// Close gracefully kills the client of the current frame
func (f *Frame) Close() {

	fmt.Println("Closing")

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

// Resize resizes the frame to the given width and height
func (f *Frame) Resize(w, h int) {
	f.w = w
	f.h = h
	f.Window.Resize(w, h)
	f.bar.Resize(w, config.Bar.Height)
	f.client.Resize(w, h-config.Bar.Height)
}

// ResizeRel resizes the frame to the given width and height
func (f *Frame) ResizeRel(dw, dh int, dir Direction) {

	switch f.resizedir {
	case northDir:
		f.y += dh
		f.h -= dh
	case southDir:
		f.h += dh
	case eastDir:
		f.w += dw
	case westDir:
		f.x += dw
		f.w -= dw
	default:
		f.Resize(f.w+dw, f.h+dw)
		return
	}

	f.MoveResize(f.x, f.y, f.w, f.h)
	f.bar.Resize(f.w, config.Bar.Height)
	f.client.Resize(f.w, f.h-config.Bar.Height)
}

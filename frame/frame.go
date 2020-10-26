package frame

import (
	"fmt"

	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/consts"
	"github.com/xen0ne/helium/logger"
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
	state      consts.State
	x, y, w, h int
	px, py     int
	resizedir  consts.Direction
	tag        int
}

func shouldManage(c *xwindow.Window) bool {
	// If this is an override redirect, skip...
	attrs, err := xproto.GetWindowAttributes(wm.X.Conn(), c.Id).Reply()
	if err != nil {
		logger.Log.Printf("Could not get window attributes for '%x': %s",
			c.Id, err)
	} else {
		if attrs.OverrideRedirect {
			logger.Log.Printf("Not managing override redirect window %x", c.Id)
			c.Map()
			c.Stack(xproto.StackModeAbove)
			return false
		}
	}

	// make sure this is the kind of window we want to manage
	wtype, err := ewmh.WmWindowTypeGet(wm.X, c.Id)
	if err != nil {
		logger.Log.Println(err)
	}
	for _, t := range wtype {
		if t == "_NET_WM_WINDOW_TYPE_TOOLBAR" ||
			t == "_NET_WM_WINDOW_TYPE_DOCK" ||
			t == "_NET_WM_WINDOW_TYPE_DESKTOP" {
			logger.Log.Printf("Not managing window of type: %s\n", t)
			c.Map()
			return false
		}
	}
	// check if window is a submenu
	t, err := icccm.WmTransientForGet(wm.X, c.Id)
	if err != nil {
		logger.Log.Printf("Could not get transient because: %s\n", err)
	} else {
		logger.Log.Printf("%+v\n", t)
		return false
	}

	return true
}

func setupGeom(c *xwindow.Window) (int, int, int, int) {

	// need the geometry
	g, err := c.Geometry()
	if err != nil {
		panic(fmt.Sprintf("Cannot get geometry for %x, bc: %s\n", c.Id, err))
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
		logger.Log.Printf("Could not get normal hints because %s\n", err)
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

	return x, y, w, h + config.Bar.Height
}

// New creates a new client from a map event
func New(c *xwindow.Window) *Frame {
	wm.X.Grab()
	defer wm.X.Ungrab()

	if !shouldManage(c) {
		return nil
	}

	f := Frame{}
	f.client = c

	f.x, f.y, f.w, f.h = setupGeom(c)

	// create the frame window!
	var err error
	f.Window, err = xwindow.Generate(wm.X)
	if err != nil {
		logger.Log.Printf("Could not create new window for %x\n", f.Id)
	}
	f.Create(wm.X.RootWin(),
		f.x, f.y,
		f.w, f.h,
		xproto.CwBackPixel|xproto.CwEventMask,
		config.Bar.Focused, xproto.EventMaskButtonPress|
			xproto.EventMaskButtonRelease|
			xproto.EventMaskButtonMotion|
			xproto.EventMaskSubstructureNotify)

	// initialize tag to 0
	f.tag = 0

	err = xproto.ReparentWindowChecked(wm.X.Conn(),
		f.client.Id, f.Id, 0, int16(config.Bar.Height)).Check()
	if err != nil {
		logger.Log.Println("Could not reparent window")
	}

	f.addClientEvents()

	f.AddBar()

	return &f
}

// Tag returns the tag of the frame
func (f *Frame) Tag() int {

	return f.tag
}

// SetTag sets the frame's tag
func (f *Frame) SetTag(t int) {
	f.tag = t
	if wm.Tags[t].IsMapped {
		f.UpdateBar()
	} else {
		f.Unmap()
	}
}

// Map maps all the components of a Frame
func (f *Frame) Map() {
	f.Window.Map()
	f.client.Map()
	if f.bar != nil {
		f.bar.Map()
	}
	f.state = consts.UnfocusedState
}

// Unmap unmaps the Frame and removes it from the focus queue
func (f *Frame) Unmap() {
	f.Unfocus()
	f.Window.Unmap()
	if wm.IsFocused(f) {
		wm.FocusPrev()
	}
	f.state = consts.UnmappedState
}

// FrameId returns the id of a frame
func (f *Frame) FrameId() xproto.Window {
	return f.Id
}

// String returns a string representation of a Frame
func (f *Frame) String() string {
	return fmt.Sprintf("frame id: %x, state: %d", f.Window.Id, f.state)
}

// Focus alerts X of the Frame we want to focus and provides input focus
func (f *Frame) Focus() {
	// if f.state == consts.FocusedState {
	// 	fmt.Println("already focused")
	// 	return
	// }

	// if there is a currently focued, tell them to unfocus
	if wm.GetFocused() != nil {
		wm.GetFocused().Unfocus()
	}

	// remove from focus queue
	wm.FocusQ = wm.RemoveFrame(f, wm.FocusQ)
	// add to focus queue
	wm.FocusQ = wm.AddFrame(f, wm.FocusQ)

	fmt.Printf("focusing %s\n", f.String())

	err := ewmh.ActiveWindowSet(wm.X, f.client.Id)
	if err != nil {
		logger.Log.Printf("Cannot set active window to %s\n", f.String())
	}
	f.Stack(xproto.StackModeAbove)
	f.state = consts.FocusedState
	f.client.Focus()

	f.UpdateBar()

}

// Unfocus updates the state and the bar of the Frame
func (f *Frame) Unfocus() {
	if f.state == consts.UnmappedState {
		return
	}
	f.state = consts.UnfocusedState
	f.UpdateBar()
}

// Contains tells us if the given frame has a window of the given id
func (f *Frame) Contains(id xproto.Window) bool {
	return f.Id == id || f.client.Id == id ||
		(f.bar.exists && f.bar.Id == id)
}

// Close gracefully kills the client of the current frame
func (f *Frame) Close() {

	fmt.Println("Closing")

	wm_protocols, err := xprop.Atm(wm.X, "WM_PROTOCOLS")
	if err != nil {
		logger.Log.Println(err)
		return
	}

	wm_del_win, err := xprop.Atm(wm.X, "WM_DELETE_WINDOW")
	if err != nil {
		logger.Log.Println(err)
		return
	}

	cm, err := xevent.NewClientMessage(32, f.client.Id, wm_protocols,
		int(wm_del_win))
	if err != nil {
		logger.Log.Println(err)
		return
	}

	err = xproto.SendEventChecked(wm.X.Conn(), false, f.client.Id, 0,
		string(cm.Bytes())).Check()
	if err != nil {
		logger.Log.Printf("Could not send WM_DELETE_WINDOW "+
			"ClientMessage because: %s", err)
	}
}

// State returns the state of the frame
func (f *Frame) State() consts.State {
	return f.state
}

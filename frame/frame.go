package frame

import (
	"fmt"
	"log"

	"github.com/BurntSushi/xgbutil"
	"github.com/xen0ne/helium/config"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/ewmh"
	"github.com/BurntSushi/xgbutil/xwindow"
)

var (
	X *xgbutil.XUtil
)

// A Frame is a window which holds a client and its decorations
type Frame struct {
	parent, client *xwindow.Window
	bar            *Bar
}

// Setup sets up the Frame struct
func Setup(x *xgbutil.XUtil) {
	X = x
}

// New creates a new client from a map event
func New(c *xwindow.Window) *Frame {
	X.Grab()
	defer X.Ungrab()

	// If this is an override redirect, skip...
	attrs, err := xproto.GetWindowAttributes(X.Conn(), c.Id).Reply()
	if err != nil {
		log.Printf("Could not get window attributes for '%x': %s",
			c.Id, err)
	} else {
		if attrs.OverrideRedirect {
			log.Printf("Not managing override redirect window %x", c.Id)
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

	f.parent, err = xwindow.Generate(X)
	if err != nil {
		log.Printf("Could not create new window for %x\n", f.parent.Id)
	}
	f.parent.Create(X.RootWin(),
		g.X(), g.Y(),
		g.Width(), g.Height()+config.Bar.Height,
		xproto.CwBackPixel, 0xffffff)

	f.Map()

	err = xproto.ReparentWindowChecked(X.Conn(),
		f.client.Id, f.parent.Id, 0, int16(config.Bar.Height)).Check()
	if err != nil {
		log.Println("Could not reparent window")
	}

	return &f
}

// Map maps all the components of a Frame
func (f *Frame) Map() {
	f.parent.Map()
	f.client.Map()
	if f.bar != nil {
		f.bar.win.Map()
	}
}

// String returns a string representation of a Frame
func (f *Frame) String() string {
	return fmt.Sprintf("%x", f.client.Id)
}

// Focus alerts X of the Frame we want to focus and provides input focus
func (f *Frame) Focus() {
	err := ewmh.ActiveWindowSet(X, f.parent.Id)
	if err != nil {
		log.Printf("Cannot set active window to %s\n", f.String())
	}
	f.client.Focus()
}

package client

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/frame"
	"github.com/xen0ne/helium/wm"
)

//
type Client struct {
	cwin  *xwindow.Window
	frame *frame.Frame
}

// New creates a new client from a map event
func New(id xproto.Window) *Client {
	wm.X.Grab()
	defer wm.X.Ungrab()

	// If this is an override redirect, skip...
	attrs, err := xproto.GetWindowAttributes(wm.X.Conn(), id).Reply()
	if err != nil {
		log.Printf("Could not get window attributes for '%x': %s",
			id, err)
	} else {
		if attrs.OverrideRedirect {
			log.Printf("Not managing override redirect window %x", id)
			return nil
		}
	}
	win := xwindow.New(wm.X, id)
	_, err = win.Geometry()
	if err != nil {
		log.Printf("Cannot get geometry for %x\n", id)
	}

	return &Client{win, nil}
}

// Map maps all the components of a Client
func (c *Client) Map() {
	c.cwin.Map()
}

func (c *Client) AddFrame() {
	f := frame.New(c.cwin)
	c.frame = f
}

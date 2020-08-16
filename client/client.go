package client

import (
	"fmt"
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/ewmh"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/wm"
)

//
type Client struct {
	win    *xwindow.Window
	parent *xwindow.Window
	bar    *xwindow.Window
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

	// need the geometry
	_, err = win.Geometry()
	if err != nil {
		log.Printf("Cannot get geometry for %x\n", id)
	}

	win.Map()

	return &Client{win, nil, nil}
}

// Map maps all the components of a Client
func (c *Client) Map() {
	c.parent.Map()
}

func (c *Client) String() string {
	return fmt.Sprintf("%x", c.win.Id)
}

func (c *Client) Focus() {
	err := ewmh.ActiveWindowSet(wm.X, c.parent.Id)
	if err != nil {
		log.Printf("Cannot set active window to %s\n", c.String())
	}
	c.win.Focus()
}

package client

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/wm"
)

//
type Client struct {
	cwin *xwindow.Window
}

// New creates a new client from a map event
func New(id xproto.Window) *Client {
	wm.X.Grab()
	defer wm.X.Ungrab()

	// If this is an override redirect, skip...
	attrs, err := xproto.GetWindowAttributes(wm.X.Conn(), id).Reply()
	if err != nil {
		log.Printf("Could not get window attributes for '%d': %s",
			id, err)
	} else {
		if attrs.OverrideRedirect {
			log.Printf("Not managing override redirect window %d", id)
			return nil
		}
	}

	return &Client{xwindow.New(wm.X, id)}
}

// Map maps all the components of a Client
func (c *Client) Map() {
	c.cwin.Map()
}

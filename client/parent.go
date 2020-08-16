package client

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/wm"
)

// Reparent creates a parent window for the given Client
// if the Client already has a parent we return early
func (c *Client) Reparent() {
	if c.parent != nil {
		log.Printf("Client %s already has a parent\n", c.String())
		return
	}

	g := c.win.Geom
	// create parent window
	p, err := xwindow.Generate(wm.X)
	if err != nil {
		log.Fatalf("Could not create new id %s", err)
	}
	p.Create(wm.X.RootWin(),
		g.X(), g.Y(),
		g.Width(), g.Height()+config.Bar.Height,
		xproto.CwBackPixel, 0xffffff)

	// reparent client
	err = xproto.ReparentWindowChecked(wm.X.Conn(),
		c.win.Id, p.Id, 0, int16(config.Bar.Height)).Check()
	if err != nil {
		log.Println("Could not reparent window")
	}

	c.parent = p

	p.Map()
}

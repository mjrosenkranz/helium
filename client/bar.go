package client

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/wm"
)

func (c *Client) AddBar() {

	// we can't add a bar if there is no parent
	if c.parent == nil {
		log.Printf("Client %s does not have a parent\n", c.String())
		return
	}

	if c.bar != nil {
		log.Printf("Client %s already has a bar \n", c.String())
		return
	}

	g := c.win.Geom

	b, err := xwindow.Generate(wm.X)
	if err != nil {
		log.Fatalf("Could not create new id %s", err)
	}
	b.Create(wm.X.RootWin(),
		0, 0,
		g.Width(), config.Bar.Height,
		xproto.CwBackPixel, config.Bar.Focused)

	// reparent bar
	err = xproto.ReparentWindowChecked(wm.X.Conn(), b.Id, c.parent.Id, 0, 0).Check()
	if err != nil {
		log.Println("Could not reparent bar")
	}

	c.bar = b

	b.Map()
}

// ChangeBarColor changes the color of the given client's bar
func (c *Client) ChangeBarColor(n uint32) {
	c.bar.Change(xproto.CwBackPixel, n)
	c.bar.ClearAll()
}

package frame

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/wm"
)

type Frame struct {
	parent, client, bar *xwindow.Window
}

// New creates a new frame
func New(c *xwindow.Window) *Frame {

	g := c.Geom

	// create parent window
	p, err := xwindow.Generate(wm.X)
	if err != nil {
		log.Fatalf("Could not create new id %s", err)
	}
	p.Create(wm.X.RootWin(),
		g.X(), g.Y(),
		g.Width(), g.Height()+config.Bar.Height,
		xproto.CwBackPixel, 0xffffff)

	// create bar window
	b, err := xwindow.Generate(wm.X)
	if err != nil {
		log.Fatalf("Could not create new id %s", err)
	}
	b.Create(wm.X.RootWin(),
		0, 0,
		g.Width(), config.Bar.Height,
		xproto.CwBackPixel, config.Bar.Focused)

	// reparent bar
	err = xproto.ReparentWindowChecked(wm.X.Conn(), b.Id, p.Id, 0, 0).Check()
	if err != nil {
		log.Println("Could not reparent window")
		return nil
	}
	// reparent client
	err = xproto.ReparentWindowChecked(wm.X.Conn(),
		c.Id, p.Id, 0, int16(config.Bar.Height)).Check()
	if err != nil {
		log.Println("Could not reparent window")
		return nil
	}

	p.Map()
	b.Map()

	return &Frame{p, nil, c}
}

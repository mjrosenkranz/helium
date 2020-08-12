package frame

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/wm"
)

type Frame struct {
	parent, client, bar *xwindow.Window
}

// New creates a new frame
func New(c *xwindow.Window) *Frame {

	p, err := xwindow.Generate(wm.X)
	if err != nil {
		log.Fatalf("Could not create new id %s", err)
	}

	g := c.Geom

	p.Create(wm.X.RootWin(), g.X(), g.Y(), g.Width(), g.Height()+20, xproto.CwBackPixel, 0xffffff)

	err = xproto.ReparentWindowChecked(wm.X.Conn(), c.Id, p.Id, 0, 20).Check()
	if err != nil {
		log.Println("Could not reparent window")
		return nil
	}

	// p.Create(wm.X.RootWin(), 0, 0, 100, +100, xproto.CwBackPixel, 0xffffff)
	p.Map()

	return &Frame{p, nil, c}
}

package wm

import (
	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/BurntSushi/xgbutil/xinerama"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/logger"
)

var (
	// X is our *XUtil
	X *xgbutil.XUtil
	// Root is an xwindow windo corresponding to the root window
	Root *xwindow.Window

	Heads xinerama.Heads
)

// Setup sets up event listening on the root window and assigns callbacks
func Setup(xu *xgbutil.XUtil) {
	X = xu
	Root = xwindow.New(X, X.RootWin())
	// setup events on root window
	mask := xproto.EventMaskPropertyChange |
		xproto.EventMaskFocusChange |
		xproto.EventMaskButtonPress |
		xproto.EventMaskButtonRelease |
		xproto.EventMaskStructureNotify |
		xproto.EventMaskSubstructureNotify |
		xproto.EventMaskSubstructureRedirect

	err := Root.Listen(mask)
	if err != nil {
		logger.Log.Fatalf("Could not listen to Root window events: %s\n", err)
	}

	Tags = []Tag{}
	for i := 0; i < config.Tags.Number; i++ {
		Tags = append(Tags, Tag{i, config.Tags.Names[i], true})
	}

	// setup heads
	
	// get info about the screen
	// var heads xinerama.Heads
	if X.ExtInitialized("XINERAMA") {
		Heads, err = xinerama.PhysicalHeads(X)
		if err != nil {
			logger.Log.Fatal(err)
		}
	} else {
		logger.Log.Println("awe man oh fuck")
	}


	// for i, head := range heads {
		
	// }
}

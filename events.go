package main

import (
	"log"

	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/frame"
	"github.com/xen0ne/helium/wm"
)

// AddHandlers adds the event handlers used by the wm
func AddHandlers() {
	xevent.MapRequestFun(handleMapReq).Connect(wm.X, wm.Root.Id)
	xevent.DestroyNotifyFun(handleDestroy).Connect(wm.X, wm.Root.Id)
}

func handleMapReq(X *xgbutil.XUtil, ev xevent.MapRequestEvent) {
	log.Printf("Map request for %x\n", ev.MapRequestEvent.Window)
	// check if the window is managed
	if wm.ById(ev.MapRequestEvent.Window) != nil {
		log.Printf("%x is already managed\n", ev.MapRequestEvent.Window)
	} else {
		win := xwindow.New(wm.X, ev.MapRequestEvent.Window)
		// create new window
		f := frame.New(win)
		f.AddBar()
		f.Focus()
		wm.AddFrame(f)
	}
}

func handleDestroy(X *xgbutil.XUtil, ev xevent.DestroyNotifyEvent) {
	log.Printf("destroy on %x", ev.DestroyNotifyEvent.Window)
}

package main

import (
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/frame"
	"github.com/xen0ne/helium/logger"
	"github.com/xen0ne/helium/wm"
)

// AddHandlers adds the event handlers used by the wm
func AddHandlers() {
	xevent.MapRequestFun(handleMapReq).Connect(wm.X, wm.Root.Id)
	xevent.DestroyNotifyFun(handleDestroy).Connect(wm.X, wm.Root.Id)
}

func handleMapReq(X *xgbutil.XUtil, ev xevent.MapRequestEvent) {
	logger.Log.Printf("Map request for %x\n", ev.MapRequestEvent.Window)
	// check if the window is managed
	if wm.ById(ev.MapRequestEvent.Window) != nil {
		logger.Log.Printf("%x is already managed\n", ev.MapRequestEvent.Window)
	} else {
		win := xwindow.New(wm.X, ev.MapRequestEvent.Window)
		// create new window
		f := frame.New(win)
		if f == nil {
			return
		}
		wm.ManagedFrames = wm.AddFrame(f, wm.ManagedFrames)
		f.AddBar()
		f.Focus()
	}
}

func handleDestroy(X *xgbutil.XUtil, ev xevent.DestroyNotifyEvent) {
	logger.Log.Printf("destroy on %x", ev.DestroyNotifyEvent.Window)
}

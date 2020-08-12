package main

import (
	"log"

	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/client"
	"github.com/xen0ne/helium/wm"
)

// AddHandlers adds the event handlers used by the wm
func AddHandlers() {
	xevent.MapRequestFun(
		func(X *xgbutil.XUtil, ev xevent.MapRequestEvent) {
			log.Printf("Map request for %x\n", ev.MapRequestEvent.Window)
			// check if the window is managed
			if client.ById(ev.MapRequestEvent.Window) != nil {
				log.Printf("%x is already managed\n", ev.MapRequestEvent.Window)
			} else {
				// create new window
				c := client.New(ev.MapRequestEvent.Window)
				c.AddFrame()
				c.Map()
			}
		}).Connect(wm.X, wm.Root.Id)

}

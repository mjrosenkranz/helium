package frame

import (
	"fmt"
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/mousebind"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/wm"
)

func (f *Frame) addClientEvents() {
	err := f.client.Listen(xproto.EventMaskPropertyChange |
		xproto.EventMaskStructureNotify)

	if err != nil {
		log.Println(err)
	}
	// tell us if the client is killed
	f.cdestroyNotify().Connect(X, f.client.Id)

	// add focous on click
	err = mousebind.ButtonPressFun(
		func(X *xgbutil.XUtil, ev xevent.ButtonPressEvent) {
			xproto.AllowEvents(X.Conn(), xproto.AllowReplayPointer, 0)
			f.Focus()
		}).Connect(X, f.client.Id, "1", false, true)
	if err != nil {
		fmt.Println(err)
	}

	mousebind.Drag(X, f.client.Id, f.client.Id, "Mod1-1", true,
		f.moveDragBegin, f.moveDragStep, f.moveDragEnd)
}

func (f *Frame) cdestroyNotify() xevent.DestroyNotifyFun {
	fn := func(X *xgbutil.XUtil, ev xevent.DestroyNotifyEvent) {
		log.Printf("destroy notify for %x\n", ev.Window)

		// get the last destroy notify
		X.Sync()
		xevent.Read(X, false)

		for i, ee := range xevent.Peek(X) {
			if dn, ok := ee.Event.(xproto.DestroyNotifyEvent); ok {
				if dn.Window == ev.Window {
					log.Printf("another destroy for %x\n", ev.Window)
					xevent.DequeueAt(X, i)
				}
			}
		}

		flast := wm.FoucusQ[0].FrameId() == f.FrameId()
		wm.FoucusQ = wm.RemoveFrame(f, wm.FoucusQ)

		if len(wm.FoucusQ) > 0 && flast {
			wm.FoucusQ[0].Focus()
		}

		f.bar.Destroy()
		f.Destroy()
		wm.ManagedFrames = wm.RemoveFrame(f, wm.ManagedFrames)
	}
	return xevent.DestroyNotifyFun(fn)
}

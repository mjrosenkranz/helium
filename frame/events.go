package frame

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/mousebind"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/wm"
)

func (f *Frame) manageEvents() {
	err := f.client.Listen(xproto.EventMaskPropertyChange |
		xproto.EventMaskStructureNotify)

	if err != nil {
		log.Println(err)
	}
	f.cdestroyNotify().Connect(X, f.client.Id)
	mousebind.ButtonPressFun(
		func(X *xgbutil.XUtil, ev xevent.ButtonPressEvent) {
			xproto.AllowEvents(X.Conn(), xproto.AllowReplayPointer, 0)
			f.Focus()
			mousebind.Detach(X, f.client.Id)
		}).Connect(X, f.client.Id,
		"1", false, true)
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

		flast := wm.FoucusQ[0].Id() == f.Id()
		wm.FoucusQ = wm.RemoveFrame(f, wm.FoucusQ)

		if len(wm.FoucusQ) > 0 && flast {
			wm.FoucusQ[0].Focus()
		}

		f.bar.win.Destroy()
		f.parent.Destroy()
		wm.ManagedFrames = wm.RemoveFrame(f, wm.ManagedFrames)
	}
	return xevent.DestroyNotifyFun(fn)
}

func (f *Frame) handleBarPress() xevent.ButtonPressFun {
	fn := func(X *xgbutil.XUtil, ev xevent.ButtonPressEvent) {
		if ev.Detail == 1 {
			f.px = int(ev.RootX)
			f.py = int(ev.RootY)
			f.parent.Stack(xproto.StackModeAbove)
			f.Focus()
			f.state = clicked
		}
	}

	return xevent.ButtonPressFun(fn)
}

func (f *Frame) handleBarMotion() xevent.MotionNotifyFun {
	fn := func(X *xgbutil.XUtil, ev xevent.MotionNotifyEvent) {
		if f.state == clicked {
			dx := int(ev.RootX) - f.px
			dy := int(ev.RootY) - f.py

			f.x += dx
			f.y += dy
			f.parent.Move(f.x, f.y)

			f.px = int(ev.RootX)
			f.py = int(ev.RootY)
		}
	}
	return xevent.MotionNotifyFun(fn)
}

func (f *Frame) handleBarRelease() xevent.ButtonReleaseFun {
	fn := func(X *xgbutil.XUtil, ev xevent.ButtonReleaseEvent) {
		switch ev.Detail {
		case 1, 3:
			f.state = focused
			f.Focus()
		case 2:
			f.client.Kill()
		}
	}

	return xevent.ButtonReleaseFun(fn)
}

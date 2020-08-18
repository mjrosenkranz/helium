package frame

import (
	"fmt"
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/wm"
)

func (f *Frame) manageEvents() {
	err := f.client.Listen(xproto.EventMaskPropertyChange |
		xproto.EventMaskStructureNotify | xproto.EventMaskButtonPress)

	if err != nil {
		log.Println(err)
	}
	f.cdestroyNotify().Connect(X, f.client.Id)
	f.cButtonPress().Connect(X, f.client.Id)
}

func (f *Frame) cButtonPress() xevent.ButtonPressFun {
	fn := func(X *xgbutil.XUtil, ev xevent.ButtonPressEvent) {
		fmt.Println("clicky")
	}
	return xevent.ButtonPressFun(fn)
}

func (f *Frame) cdestroyNotify() xevent.DestroyNotifyFun {
	fn := func(X *xgbutil.XUtil, ev xevent.DestroyNotifyEvent) {
		log.Printf("destroy notify for %x\n", ev.Window)
		f.bar.win.Destroy()
		f.parent.Destroy()
		wm.RemoveFrame(f)
	}
	return xevent.DestroyNotifyFun(fn)
}

func (f *Frame) handleBarPress() xevent.ButtonPressFun {
	fn := func(X *xgbutil.XUtil, ev xevent.ButtonPressEvent) {
		if ev.Detail == 1 {
			f.state = clicked
			f.px = int(ev.RootX)
			f.py = int(ev.RootY)
			f.parent.Stack(xproto.StackModeAbove)
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

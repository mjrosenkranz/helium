package frame

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/mousebind"
	"github.com/BurntSushi/xgbutil/xcursor"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/wm"
)

func (f *Frame) addFrameEvents() {
	// add drag movement
	mousebind.Drag(wm.X, f.bar.Id, f.bar.Id,
		"1", false,
		f.moveDragBegin, f.moveDragStep, f.moveDragEnd)

	// add window killing
	err := mousebind.ButtonReleaseFun(
		func(xu *xgbutil.XUtil, event xevent.ButtonReleaseEvent) {
			f.Close()
		}).Connect(wm.X, f.bar.Id, "2", false, false)
	if err != nil {
		log.Println(err)
	}
}

func (f *Frame) moveDragBegin(xu *xgbutil.XUtil, rootX, rootY, eventX, eventY int) (bool, xproto.Cursor) {
	f.px = rootX
	f.py = rootY
	f.Stack(xproto.StackModeAbove)
	f.Focus()
	f.state = dragging

	cur, err := xcursor.CreateCursor(wm.X, xcursor.Gumby)
	if err != nil {
		log.Println(err)
		return false, 0
	}

	return true, cur
}

func (f *Frame) moveDragStep(xu *xgbutil.XUtil, rootX, rootY, eventX, eventY int) {
	if f.state == dragging {
		dx := rootX - f.px
		dy := rootY - f.py

		f.x += dx
		f.y += dy
		f.Move(f.x, f.y)

		f.px = rootX
		f.py = rootY
	}
}

func (f *Frame) moveDragEnd(xu *xgbutil.XUtil, rootX, rootY, eventX, eventY int) {
	f.state = focused
	f.Focus()
}

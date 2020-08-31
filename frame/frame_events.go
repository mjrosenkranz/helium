package frame

import (
	"fmt"
	"log"
	"math"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/mousebind"
	"github.com/BurntSushi/xgbutil/xcursor"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/consts"
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
	f.state = consts.MovingState

	cur, err := xcursor.CreateCursor(wm.X, xcursor.Gumby)
	if err != nil {
		log.Println(err)
		return false, 0
	}

	return true, cur
}

func (f *Frame) moveDragStep(xu *xgbutil.XUtil, rootX, rootY, eventX, eventY int) {
	if f.state == consts.MovingState {
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
	f.state = consts.FocusedState
	f.Focus()
}

func (f *Frame) resizeDragBegin(xu *xgbutil.XUtil, rootX, rootY, eventX, eventY int) (bool, xproto.Cursor) {
	f.px = rootX
	f.py = rootY
	f.Stack(xproto.StackModeAbove)
	f.Focus()
	f.state = consts.ResizingState

	// set the drag direction by which edge is closest
	f.resizedir = f.dirFromPoint(eventX, eventY, f.w, f.h)

	fmt.Printf("direction %d\n", f.resizedir)

	cur, err := xcursor.CreateCursor(wm.X, xcursor.Gumby)
	if err != nil {
		log.Println(err)
		return false, 0
	}

	return true, cur
}

func (f *Frame) dirFromPoint(x, y, w, h int) consts.Direction {
	m := map[consts.Direction]int{
		consts.NorthDir: y,
		consts.SouthDir: h - y,
		consts.EastDir:  w - x,
		consts.WestDir:  x,
	}
	min := math.Inf(1)
	ret := consts.NoDir

	for k, v := range m {
		if float64(v) < min {
			min = float64(v)
			ret = k
		}
	}

	return ret
}

func (f *Frame) resizeDragStep(xu *xgbutil.XUtil, rootX, rootY, eventX, eventY int) {
	if f.state == consts.ResizingState {
		dx := rootX - f.px
		dy := rootY - f.py

		f.ResizeRel(dx, dy, f.resizedir)

		f.px = rootX
		f.py = rootY

		f.UpdateBar()
	}
}

func (f *Frame) resizeDragEnd(xu *xgbutil.XUtil, rootX, rootY, eventX, eventY int) {
	f.state = consts.FocusedState
	f.resizedir = consts.NoDir
	f.Focus()
	f.UpdateBar()
}

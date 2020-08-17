package wm

import (
	"github.com/BurntSushi/xgb/xproto"
	"github.com/xen0ne/helium/frame"
)

// ById returns a *Frame if the id matches that of a window or it's associated
func ById(id xproto.Window) *frame.Frame {
	for _, f := range Frames {
		if f.Contains(id) {
			return f
		}
	}
	return nil
}

// AddFrame adds the given frame to our managed frames
func AddFrame(f *frame.Frame) {
	Frames = append(Frames, f)
}

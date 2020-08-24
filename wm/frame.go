package wm

import "github.com/BurntSushi/xgb/xproto"

// Frame is an interface for a frame because I'm not quite sure what to do
type Frame interface {
	Contains(xproto.Window) bool
	FrameId() xproto.Window
	Focus()
	Unfocus()
	Close()
}

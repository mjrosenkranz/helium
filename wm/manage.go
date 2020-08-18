package wm

import (
	"fmt"

	"github.com/BurntSushi/xgb/xproto"
)

var (
	// ManagedFrames is a slice of all managed frames
	ManagedFrames []Frame

	// FocusQ is the order of focused frames
	FoucusQ []Frame
)

// ById returns a *Frame if the id matches that of a window or it's associated
func ById(id xproto.Window) Frame {
	for _, f := range ManagedFrames {
		if f.Contains(id) {
			return f
		}
	}
	return nil
}

func printFrames() {
	fmt.Printf("%+v", ManagedFrames)
}

// AddFrame adds the given frame to the slice of Frames s
func AddFrame(f Frame, s []Frame) []Frame {
	return append([]Frame{f}, s...)
}

// RemoveFrame removes a given frame from the wm list of managed frames
func RemoveFrame(f Frame, s []Frame) []Frame {
	for i, f2 := range s {
		if f2.FrameId() == f.FrameId() {
			s = append(s[:i], s[i+1:]...)
			break
		}
	}
	return s
}

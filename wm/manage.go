package wm

import (
	"fmt"

	"github.com/BurntSushi/xgb/xproto"
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

// AddFrame adds the given frame to our managed frames
func AddFrame(f Frame) {
	ManagedFrames = append(ManagedFrames, f)
}

// RemoveFrame removes a given frame from the wm list of managed frames
func RemoveFrame(f Frame) {
	for i, f2 := range ManagedFrames {
		if f2.Id() == f.Id() {
			ManagedFrames = append(ManagedFrames[:i], ManagedFrames[i+1:]...)
			break
		}
	}

}

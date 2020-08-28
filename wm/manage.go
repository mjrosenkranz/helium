package wm

import (
	"fmt"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/xen0ne/helium/consts"
)

var (
	// ManagedFrames is a slice of all managed frames
	ManagedFrames []Frame

	// FocusQ is the order of focused frames
	FocusQ []Frame

	IsShowing = false

	Tags []bool
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

// AddFrame adds the given frame to the slice of Frames s
func AddFrame(f Frame, s []Frame) []Frame {
	return append([]Frame{f}, s...)
}

// RemoveFrame removes a given frame from the wm list of managed frames
func RemoveFrame(f Frame, s []Frame) []Frame {
	ret := s
	for i, f2 := range s {
		if f2.FrameId() == f.FrameId() {
			ret = append(s[:i], s[i+1:]...)
			break
		}
	}
	// fmt.Printf("Just removed frame: %+v\n", ret)
	return ret
}

// GetFocused returns the currently focused frame or nil if there are none
func GetFocused() Frame {
	if len(FocusQ) == 0 {
		return nil
	}
	return FocusQ[0]
}

// IsFocused returns if the given frame is at the front of the FocusQ
func IsFocused(f Frame) bool {
	if len(FocusQ) == 0 {
		return false
	}

	return FocusQ[0].FrameId() == f.FrameId()
}

// FocusNext focuses the next window in the focus queue
func FocusNext() {
	if len(FocusQ) < 2 {
		return
	}

	prev := FocusQ
	FocusQ = append([]Frame{FocusQ[len(FocusQ)-1]},
		FocusQ[:len(FocusQ)-1]...)
	fmt.Printf("from %+v to: %+v\n", prev, FocusQ)

	GetFocused().Focus()
}

// FocusPrev focuses the next window in the focus queue
func FocusPrev() {
	if len(FocusQ) < 2 {
		return
	}

	// count how many times we have attempted to focus the previous
	for i := 0; i < len(FocusQ); i++ {
		if GetFocused().State() != consts.UnmappedState {
			break
		}
		FocusQ = append(FocusQ[1:], FocusQ[0])
	}

	GetFocused().Focus()
}

// ToggleTag toggles the visibility of the given tag
func ToggleTag(t int) {
	for _, f := range ManagedFrames {
		if f.Tag() == t {
			if Tags[t] {
				f.Unmap()
			} else {
				if NoneToFocus() {
					f.Map()
					f.Focus()
					fmt.Println("none to focus so reverting")
				} else {
					f.Map()
				}
			}
		}
	}

	Tags[t] = !Tags[t]
}

// IsValidTag returns if t is a valid tag
func IsValidTag(t int) bool {
	fmt.Printf("numtags: %d", len(Tags))
	return t > 0 && t < len(Tags)
}

// NoneToFocus returns wether or not there are any windows that are focusable
func NoneToFocus() bool {
	numFocusable := 0

	for _, f := range ManagedFrames {
		if f.State() != consts.UnmappedState {
			numFocusable++
		}
	}

	return numFocusable == 0
}

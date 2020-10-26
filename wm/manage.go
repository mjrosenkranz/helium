package wm

import (
	"fmt"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/consts"
)

var (
	// ManagedFrames is a slice of all managed frames
	ManagedFrames []Frame

	// FocusQ is the order of focused frames
	FocusQ []Frame

	Tags []Tag
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

	GetFocused().Unfocus()

	for i := 0; i < len(FocusQ); i++ {
		FocusQ = append([]Frame{FocusQ[len(FocusQ)-1]},
			FocusQ[:len(FocusQ)-1]...)

		if GetFocused().State() != consts.UnmappedState {
			fmt.Printf("found window %s with sate %d\n", GetFocused(), GetFocused().State())
			break
		}
	}

	GetFocused().Focus()
}

// FocusPrev focuses the next window in the focus queue
func FocusPrev() {
	if len(FocusQ) < 2 {
		return
	}

	prev := GetFocused()

	// count how many times we have attempted to focus the previous
	for i := 0; i < len(FocusQ); i++ {
		FocusQ = append(FocusQ[1:], FocusQ[0])
		if GetFocused().State() != consts.UnmappedState {
			fmt.Printf("found window %s with sate %d\n", GetFocused(), GetFocused().State())
			break
		} else {
			if i == len(FocusQ)-1 {
				fmt.Println("No more to focus, maintaining same position")
				return
			}
		}
	}
	prev.Unfocus()

	GetFocused().Focus()
}

func PrintFrames() {
	for _, f := range ManagedFrames {
		fmt.Printf("frame %s state: %d\n", f, f.State())
	}
}

// ToggleTag toggles the visibility of the given tag
func ToggleTag(t int) {
	for _, f := range ManagedFrames {
		if f.Tag() == t {
			// Unmap
			if Tags[t].IsMapped {
				f.Unmap()
			} else {
				if NoneToFocus() {
					f.Map()
					f.Focus()
					fmt.Println("none to focus so reverting")
				} else {
					f.Map()
					f.Focus()
				}
			}
		}
	}

	Tags[t].IsMapped = !Tags[t].IsMapped
}

// IsValidTag returns if t is a valid tag
func IsValidTag(t int) bool {
	fmt.Printf("numtags: %d", config.Tags.Number)
	return t >= 0 && t < config.Tags.Number
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

// UpdateFocued updates the focused frame
func UpdateFocused() {
	foc := GetFocused()
	if foc != nil {
		foc.UpdateBar()
	}
}

// UpdateUnfocused updates all unfocused frames
func UpdateUnfocused() {
	for _, f := range FocusQ {
		if f.State() == consts.UnfocusedState {
			f.UpdateBar()
		}
	}
}

// UpdateTag updates all frames in tag
func UpdateTag(t int) {
	for _, f := range FocusQ {
		if f.Tag() == t {
			f.UpdateBar()
		}
	}
}

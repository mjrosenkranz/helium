package consts

import "fmt"

// State is a possible window state
type State int

// A Direction is a cardinal direction
type Direction int

const (
	NoState State = iota
	FocusedState
	UnfocusedState
	MovingState
	ResizingState
	UnmappedState
)

const (
	NoDir Direction = iota
	NorthDir
	EastDir
	SouthDir
	WestDir
)

// StringToDir returns a Direction based on the string given
func StringToDir(s string) (Direction, error) {
	switch s {
	case "north":
		return NorthDir, nil
	case "south":
		return SouthDir, nil
	case "east":
		return EastDir, nil
	case "west":
		return WestDir, nil
	default:
		return NoDir, fmt.Errorf("%s is not a valid direction", s)
	}
}

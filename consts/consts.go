package consts

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

package frame

type State int
type Direction int

const (
	noneState State = iota
	focusedState
	unfocusedState
	movingState
	resizingState
)

const (
	noDir Direction = iota
	northDir
	eastDir
	southDir
	westDir
)

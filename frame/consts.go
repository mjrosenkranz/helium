package frame

type State int
type Direction int

const (
	noneState State = iota
	focusedState
	unfocusedState
	movingState
	resizingState
	unmappedState
)

const (
	noDir Direction = iota
	northDir
	eastDir
	southDir
	westDir
)

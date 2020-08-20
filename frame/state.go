package frame

type State int

const (
	none State = iota
	focused
	unfocused
	dragging
)

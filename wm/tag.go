package wm

// Tag is a group of windows that can all be mapped or unmapped together
type Tag struct {
	num    int
	Mapped bool
}

// Empty states if there are any windows assigned to the current tag
func (t *Tag) Empty() bool {
	return false
}

package wm

// Tag is a group of windows that can all be mapped or unmapped together
type Tag struct {
	num      int
	name     string
	IsMapped bool
}

// Empty states if there are any windows assigned to the current tag
func (t *Tag) Empty() bool {
	return len(t.Frames()) > 0
}

// Frames returns all the frames assigned to this tag
func (t *Tag) Frames() []Frame {
	frames := []Frame{}
	for _, f := range ManagedFrames {
		if f.Tag() == t.num {
			frames = append(frames, f)
		}
	}

	return frames
}

// ChangeName changes the name of the tag
func (t *Tag) ChangeName(n string) {
	t.name = n
}

// Name returns the name of the tag
func (t *Tag) Name() string {
	return t.name
}

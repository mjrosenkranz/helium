package config

var (
	Bar struct {
		Focused, Unfocued uint32
		Height            int
	}
)

func Setup() {
	// set defaults
	Bar.Focused = 0xc1c1c1
	Bar.Unfocued = 0x3f3f3f
	Bar.Height = 20
}

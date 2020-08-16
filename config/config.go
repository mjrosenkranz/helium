package config

var (
	// Bar contans all config items related to window title decorations
	Bar struct {
		Focused, UnFocused uint32
		Height             int
	}
)

// Defaults sets all the config variables to their default values
func Defaults() {
	// set defaults
	Bar.Focused = 0xc1c1c1
	Bar.UnFocused = 0x3f3f3f
	Bar.Height = 20
}

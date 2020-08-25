package config

var (
	// Bar contans all config items related to window title decorations
	Bar struct {
		// Hex Colors of bar background
		Focused, Unfocused uint32
		// Hex Colors of bar text
		TextFocused, TextUnfocused uint32
		// Height of the bar in pixels
		Height int
		// Should the title text be centered?
		CenterText bool
		// If the title text is not centered, the text offset from the ends
		TextOffset int
		// Path of the font to be used for the title bar
		FontPath string
		// Name of the bar font, it will replace the font path when I can figure that stuff out
		Font string
	}

	// Border contains all config items related to window borders
	Border struct {
		// Hex Colors of border
		Focused, Unfocused uint32
		// Width of the bar in pixels
		Width int
	}
)

// Defaults sets all the config variables to their default values
func Defaults() {
	// set defaults
	Bar.Focused = 0x007d9c
	Bar.Unfocused = 0xe0ebf5
	Bar.TextFocused = 0x000000
	Bar.TextUnfocused = 0x000000
	Bar.Height = 20
	Bar.CenterText = true
	Bar.TextOffset = 5
	Bar.FontPath = "./extra/DejaVuSans.ttf"
	Bar.Font = "DejaVuSans"

	Border.Focused = 0xffffdd
	Border.Unfocused = 0xddffff
	Border.Width = 0
}

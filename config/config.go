package config

import (
	"io/ioutil"
	"log"

	"github.com/BurntSushi/freetype-go/freetype"
	"github.com/BurntSushi/freetype-go/freetype/truetype"
)

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
		FontName string
		FontSize float64

		Font *truetype.Font
	}

	// Border contains all config items related to window borders
	Border struct {
		// Hex Colors of border
		Focused, Unfocused uint32
		// Width of the bar in pixels
		Width int
	}

	Tags struct {
		// Number of tags
		Number int
		// Names of each tag
		Names []string
	}
)

// Defaults sets all the config variables to their default values
func Defaults() {
	// set defaults
	Bar.Focused = 0xe0ebf5
	Bar.Unfocused = 0xffffdd
	Bar.TextFocused = 0x000000
	Bar.TextUnfocused = 0x000000
	Bar.Height = 20
	Bar.CenterText = true
	Bar.TextOffset = 5
	Bar.FontPath = "/home/xenone/code/helium/extra/DejaVuSans.ttf"
	Bar.FontName = "DejaVuSans"
	Bar.FontSize = 12.0
	Bar.Font = openFont()

	Border.Focused = 0x007d9c
	Border.Unfocused = 0xddffff
	Border.Width = 0

	Tags.Number = 7
	Tags.Names = []string{"nil", "one", "two", "three", "four", "five", "six"}
}

func openFont() *truetype.Font {
	// open ttf
	bs, err := ioutil.ReadFile(Bar.FontPath)
	if err != nil {
		log.Fatalln(err)
	}
	font, err := freetype.ParseFont(bs)
	if err != nil {
		log.Fatalln(err)
	}

	return font
}

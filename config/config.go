package config

import (
	"fmt"
	"io/ioutil"
	"strconv"
	"strings"
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
		// FontSize is the size of the bar font
		FontSize float64
		// Font is a pointer to a truetype.Font
		Font *truetype.Font
	}

	// Border contains all config items related to window borders
	Border struct {
		// Hex Colors of border
		Focused, Unfocused uint32
		// Width of the bar in pixels
		Width int
	}

	// Tags contains all config items for a Tag
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
	Bar.FontSize = 12.0
	var err error
	Bar.Font, err = openFont("/home/xenone/docs/code/helium/extra/DejaVuSans.ttf")
	if err != nil {
		log.Fatal(err)
	}

	Border.Focused = 0x007d9c
	Border.Unfocused = 0xddffff
	Border.Width = 0

	Tags.Number = 7
	Tags.Names = []string{"nil", "one", "two", "three", "four", "five", "six"}
}

// ValidateAndModifyColor validates a color and then sets the value if valid
func ValidateAndModifyColor(i *uint32, s string) error {
	s = strings.TrimLeft(s, "#")
	n, err := strconv.ParseInt(s, 16, 32)
	if err != nil {
		return err
	}
	*i = uint32(n)

	return nil
}

// ValidateAndModifyInt validates an int and then sets the value if valid
func ValidateAndModifyInt(i *int, s string) error {
	n, err := strconv.ParseInt(s, 0, 0)
	if err != nil {
		return err
	}
	*i = int(n)

	return nil
}

// ValidateAndModifyBool validates an int and then sets the value if valid
func ValidateAndModifyBool(b *bool, s string) error {
	n, err := strconv.ParseBool(s)
	if err != nil {
		return err
	}
	*b = n

	return nil
}

// ValidateAndModifyFloat validates an float and then sets the value if valid
func ValidateAndModifyFloat(f *float64, s string) error {
	n, err := strconv.ParseFloat(s, 64)
	if err != nil {
		return err
	}
	*f = float64(n)

	return nil
}

// ChangeFont changes the config font and path
func ChangeFont(path string) error {
	ft, err := openFont(path)
	if err != nil {
		return err
	}
	Bar.Font = ft
	return nil
}

// ChangeFontSize attempts to change the font size
func ChangeFontSize(s string) error {

	n, err := strconv.ParseFloat(s, 64)

	// check extents
	ctx := freetype.NewContext()
	ctx.SetDPI(72)
	ctx.SetFont(Bar.Font)
	ctx.SetFontSize(n)

	_, ehf, err := ctx.MeasureString("blah")
	eh := int(ehf / 256)
	if err != nil {
		return err
	}

	if eh > Bar.Height {
		return fmt.Errorf("%.1f is too large a font for the bar", n)
	}

	Bar.FontSize = n

	return nil
}

func openFont(path string) (*truetype.Font, error) {
	bs, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}

	font, err := freetype.ParseFont(bs)
	if err != nil {
		return nil, err
	}

	return font, nil
}

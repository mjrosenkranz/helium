package frame

import (
	"image"
	"image/color"
	"io/ioutil"
	"log"

	"github.com/BurntSushi/freetype-go/freetype"
	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/ewmh"
	"github.com/BurntSushi/xgbutil/xgraphics"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/config"
)

type Bar struct {
	win *xwindow.Window
}

func (f *Frame) AddBar() {

	// we can't add a bar if there is no parent
	if f.parent == nil {
		log.Printf("Frame %s does not have a parent\n", f.String())
		return
	}

	if f.bar != nil {
		log.Printf("Frame %s already has a bar \n", f.String())
		return
	}

	b := Bar{}

	g := f.client.Geom

	var err error
	b.win, err = xwindow.Generate(X)
	if err != nil {
		log.Fatalf("Could not create new id %s", err)
	}
	b.win.Create(X.RootWin(),
		0, 0,
		g.Width(), config.Bar.Height,
		xproto.CwBackPixel, config.Bar.Focused)

	// reparent bar
	err = xproto.ReparentWindowChecked(X.Conn(), b.win.Id, f.parent.Id, 0, 0).Check()
	if err != nil {
		log.Println("Could not reparent bar")
	}

	f.bar = &b

	b.win.Map()

	title, err := ewmh.WmNameGet(X, f.client.Id)
	if err != nil {
		log.Println(err)
		title = "bruh"
	}

	b.Draw(title, config.Bar.Focused, config.Bar.UnFocused)
}

// Draw draws the given text to the bar with a background
// color of bg and text color of fg
func (b *Bar) Draw(title string, bg, fg uint32) {
	g, err := b.win.Geometry()
	if err != nil {
		log.Printf("Cannot get geometry for %x\n", b.win.Id)
	}

	addtext(b.win, title, bg, fg, g.Width(), g.Height())
}

// HELPER FUNCTIONS

func addtext(bar *xwindow.Window, text string, bg, fg uint32, w, h int) {
	// Create an image using the over estimated extents.
	img := xgraphics.New(bar.X, image.Rect(0, 0, w, h))
	xgraphics.BlendBgColor(img, IntToColor(bg))
	// open ttf
	bs, err := ioutil.ReadFile(config.Bar.FontPath)
	if err != nil {
		log.Fatalln(err)
	}
	font, err := freetype.ParseFont(bs)
	if err != nil {
		log.Fatalln(err)
	}

	fontSize := 12.0

	ctx := freetype.NewContext()
	ctx.SetDPI(72)
	ctx.SetFont(font)
	ctx.SetFontSize(fontSize)

	runes := []rune(text)

	var ew, eh int

	for {
		ewf, ehf, err := ctx.MeasureString(string(runes))
		if err != nil {
			log.Println("Cold not get extents")
		}
		ew = int(ewf / 256)
		eh = int(ehf / 256)

		if eh > h {
			log.Println("height of font is too big for bar")
			return
		}

		if ew < (w - (2 * config.Bar.TextOffset)) {
			break
		}
		runes = runes[:len(runes)-1]
	}

	x := config.Bar.TextOffset
	if config.Bar.CenterText {
		x = (w - ew) / 2
	}

	_, _, err = img.Text(x, 0, IntToColor(fg), fontSize, font, string(runes))
	if err != nil {
		log.Printf("Could not draw font to bar because: %v", err)
		return
	}

	// Now draw the image to the window and destroy it.
	img.XSurfaceSet(bar.Id)
	// subimg := img.SubImage(image.Rect(0, 0, ew, eh))
	img.XDraw()
	img.XPaint(bar.Id)
	img.Destroy()
}

func IntToColor(i uint32) color.Color {
	r := i >> 16 & 0xff
	g := i >> 8 & 0xff
	b := i >> 0 & 0xff

	return color.RGBA{uint8(r), uint8(g), uint8(b), 255}
}

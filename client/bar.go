package client

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
	"github.com/xen0ne/helium/wm"
)

func (c *Client) AddBar() {

	// we can't add a bar if there is no parent
	if c.parent == nil {
		log.Printf("Client %s does not have a parent\n", c.String())
		return
	}

	if c.bar != nil {
		log.Printf("Client %s already has a bar \n", c.String())
		return
	}

	g := c.win.Geom

	b, err := xwindow.Generate(wm.X)
	if err != nil {
		log.Fatalf("Could not create new id %s", err)
	}
	b.Create(wm.X.RootWin(),
		0, 0,
		g.Width(), config.Bar.Height,
		xproto.CwBackPixel, config.Bar.Focused)

	// reparent bar
	err = xproto.ReparentWindowChecked(wm.X.Conn(), b.Id, c.parent.Id, 0, 0).Check()
	if err != nil {
		log.Println("Could not reparent bar")
	}

	c.bar = b

	b.Map()

	title, err := ewmh.WmNameGet(wm.X, c.win.Id)
	if err != nil {
		log.Println(err)
		title = "bruh"
	}

	addtext(c.bar, title, config.Bar.Focused, config.Bar.UnFocused,
		g.Width(), g.Height())
}

// ChangeBarColor changes the color of the given client's bar
func (c *Client) ChangeBarColor(n uint32) {
	c.bar.Change(xproto.CwBackPixel, n)
	c.bar.ClearAll()
}

// HELPER FUNCTIONS

func addtext(bar *xwindow.Window, text string, bg, fg uint32, w, h int) {
	// Create an image using the over estimated extents.
	img := xgraphics.New(bar.X, image.Rect(0, 0, w, h))
	xgraphics.BlendBgColor(img, IntToColor(bg))
	// open ttf
	bs, err := ioutil.ReadFile("/usr/share/fonts/TTF/DejaVuSans.ttf")
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

		if ew < (w - 10) {
			break
		}
		runes = runes[:len(runes)-1]
	}

	_, _, err = img.Text(5, 0, IntToColor(fg), fontSize, font, string(runes))
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

package frame

import (
	"errors"
	"fmt"
	"image"
	"image/color"

	"github.com/BurntSushi/freetype-go/freetype"
	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/ewmh"
	"github.com/BurntSushi/xgbutil/xgraphics"
	"github.com/BurntSushi/xgbutil/xwindow"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/consts"
	"github.com/xen0ne/helium/logger"
	"github.com/xen0ne/helium/wm"
)

// Bar is the title bar of the frame
type Bar struct {
	*xwindow.Window
	exists bool
}

// AddBar adds a title bar to the given frame
func (f *Frame) AddBar() {

	if f.bar != nil {
		logger.Log.Printf("Frame %s already has a bar \n", f.String())
		return
	}

	b := Bar{}
	g := f.client.Geom

	var err error
	b.Window, err = xwindow.Generate(wm.X)
	if err != nil {
		logger.Log.Fatalf("Could not create new id %s", err)
	}

	b.exists = config.Bar.Height > 0
	var h int
	if b.exists {
		h = config.Bar.Height
	} else {
		h = 1
	}
	b.Create(wm.X.RootWin(),
		0, 0,
		g.Width(), h,
		xproto.CwBackPixel|xproto.CwEventMask,
		config.Bar.Focused, xproto.EventMaskButtonPress|
			xproto.EventMaskButtonRelease|
			xproto.EventMaskButtonMotion)

	// reparent bar
	err = xproto.ReparentWindowChecked(wm.X.Conn(), b.Id, f.Id, 0, 0).Check()
	if err != nil {
		logger.Log.Println("Could not reparent bar")
	}

	f.bar = &b

	if f.bar.exists {
		b.Map()
		f.addFrameEvents()
		f.UpdateBar()
	}

}

// UpdateBar updates the title of the frames bar
func (f *Frame) UpdateBar() {
	if !f.bar.exists {
		logger.Log.Println("no bar")
		return
	}

	g, err := f.bar.Geometry()
	if err != nil {
		return
	}

	if g.Height() != config.Bar.Height {
		if config.Bar.Height == 0 {
			f.bar.Window.Unmap()
			fmt.Println("unmapping")
		} else {
			f.bar.Window.Map()
		}

		f.bar.MoveResize(0, 0, f.client.Geom.Width(), config.Bar.Height)
		f.client.MoveResize(0, config.Bar.Height,
			f.client.Geom.Width(), f.h-config.Bar.Height)
	}

	title, err := ewmh.WmNameGet(wm.X, f.client.Id)
	if err != nil {
		logger.Log.Println(err)
		title = ""
	}

	bg := config.Bar.Focused
	fg := config.Bar.TextFocused

	if f.state == consts.UnfocusedState {
		bg = config.Bar.Unfocused
		fg = config.Bar.TextUnfocused
	}

	f.bar.Draw(fmt.Sprintf("%s: %s", wm.Tags[f.tag].Name(), title), bg, fg)
}

// Draw draws the given text to the bar with a background
// color of bg and text color of fg
func (b *Bar) Draw(title string, bg, fg uint32) {
	if !b.exists {
		return
	}
	g, err := b.Geometry()
	if err != nil {
		logger.Log.Printf("Cannot get geometry for bar %x, bc: %s\n", b.Id, err)
		return
	}

	addtext(b.Window, title, bg, fg, g.Width(), g.Height())
}

// HELPER FUNCTIONS

func addtext(bar *xwindow.Window, text string, bg, fg uint32, w, h int) {
	// Create an image using the over estimated extents.
	img := xgraphics.New(wm.X, image.Rect(0, 0, w, h))
	xgraphics.BlendBgColor(img, intToColor(bg))

	x, y, t, err := trimText(w, h, text)
	if err != nil {
		logger.Log.Println(err)
		return
	}

	_, _, err = img.Text(x, y, intToColor(fg),
		config.Bar.FontSize, config.Bar.Font, t)

	if err != nil {
		logger.Log.Printf("Could not draw font to bar because: %v", err)
		return
	}
	// Now draw the image to the window and destroy it.
	img.XSurfaceSet(bar.Id)
	// subimg := img.SubImage(image.Rect(0, 0, ew, eh))
	img.XDraw()
	img.XPaint(bar.Id)
	img.Destroy()
}

func trimText(w, h int, text string) (int, int, string, error) {
	runes := []rune(text)

	ctx := freetype.NewContext()
	ctx.SetDPI(72)
	ctx.SetFont(config.Bar.Font)
	ctx.SetFontSize(config.Bar.FontSize)

	var ew, eh int

	for {
		ewf, ehf, err := ctx.MeasureString(string(runes))
		if err != nil {
			logger.Log.Println("Cold not get extents")
		}
		ew = int(ewf / 256)
		eh = int(ehf / 256)

		if eh > h {
			return -1, -1, "", errors.New("height of font is too big for bar")
		}

		if ew < (w - (2 * config.Bar.TextOffset)) {
			break
		}
		if len(runes) > 0 {
			runes = runes[:len(runes)-1]
		}
	}

	x := config.Bar.TextOffset
	if config.Bar.CenterText {
		x = (w - ew) / 2
	}

	if eh > h {
		return 0, 0, "", fmt.Errorf("Font is too tall")
	}

	y := (h - eh) / 2

	return x, y, string(runes), nil
}

func intToColor(i uint32) color.Color {
	r := i >> 16 & 0xff
	g := i >> 8 & 0xff
	b := i >> 0 & 0xff

	return color.RGBA{uint8(r), uint8(g), uint8(b), 255}
}

package frame

import (
	"fmt"

	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/consts"
	"github.com/xen0ne/helium/grid"
)

// resizeAll resizes the frame, client, and bar windows
func (f *Frame) resizeAll() {
	f.MoveResize(f.x, f.y, f.w, f.h)
	if f.bar.exists {
		f.bar.MoveResize(0, 0, f.w, config.Bar.Height)
	}
	f.client.MoveResize(0, config.Bar.Height, f.w, f.h-config.Bar.Height)
}

// Resize resizes the frame to the given width and height
func (f *Frame) Resize(w, h int) {
	f.w = w
	f.h = h
	f.resizeAll()
}

// ResizeRel resizes the frame to the given width and height
func (f *Frame) ResizeRel(amt int, dir consts.Direction) {
	switch dir {
	case consts.NorthDir:
		f.y -= amt
		f.h += amt
	case consts.SouthDir:
		f.h += amt
	case consts.EastDir:
		f.w += amt
	case consts.WestDir:
		f.x -= amt
		f.w += amt
	default:
		fmt.Println("asdfja;ldskj")
		return
	}

	f.resizeAll()
}

// SnapSizeToGrid moves and resizes the current window to the grid
func (f *Frame) SnapToGrid(g *grid.Grid) {
	// find nearest corner for each
	x, y := g.SnapCorner(f.x, f.y, consts.NWCorner)
	farx, fary := g.SnapCorner(f.x+f.w, f.y+f.h, consts.SECorner)

	f.x = x
	f.y = y

	f.w = farx - x
	f.h = fary - y

	f.resizeAll()
}

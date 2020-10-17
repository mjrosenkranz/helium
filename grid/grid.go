package grid

import (
	"github.com/BurntSushi/xgbutil/xrect"
	"github.com/xen0ne/helium/consts"
)

type Grid struct {
	rect            xrect.Rect
	rows, cols, gap int
}

// New creates a new grid with a bounding Rect and r rows and c columns
func New(rect xrect.Rect, r, c, g int) *Grid {
	return &Grid{rect, r, c, g}
}

// GetBounds returns the north, south, east, and west bounds of the grid
func (g *Grid) GetBounds(width, height int) (n, s, e, w int) {
	n = g.rect.Y()
	s = height - g.rect.Height() - n
	w = g.rect.X()
	e = width - g.rect.Width() - w

	return
}

//
/*
func (g *Grid) SnapCorner(c consts.Corner, x, y int) (nx, ny, int){
	switch c {
	case consts.NECorner:
	case consts.NWCorner:
	case consts.SWCorner:
	case consts.SECorner:
	}
}
*/

// GetCellCorner returns the x and y coordinates of the given cell
func (g *Grid) GetCellCorner(_x, _y int, c consts.Corner) (x, y int) {
	return 0, 0
}

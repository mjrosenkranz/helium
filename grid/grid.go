package grid

import (
	"math"

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

// SnapCorner snaps a given corner to the closest cell corner of the same kind
func (g *Grid) SnapCorner(x, y int, c consts.Corner) (nx, ny int) {
	d := math.MaxInt32
	nx = math.MaxInt32
	ny = math.MaxInt32

	for i := 0; i < g.cols; i++ {
		for j := 0; j < g.rows; j++ {
			tx, ty := g.CellCorner(i, j, c)

			if dist(x, y, tx, ty) < d {
				d = dist(x, y, tx, ty)
				nx = tx
				ny = ty
			}

		}
	}

	return
}

func dist(x1, y1, x2, y2 int) int {
	return int(math.Sqrt(
		math.Pow(float64(x1-x2), 2) +
			math.Pow(float64(y1-y2), 2)))
}

// CellCorner returns the x and y pixel coordinates of the given cell
// relative to the top left corner of the Grid
func (g *Grid) CellCorner(_x, _y int, c consts.Corner) (x, y int) {
	cw := g.CellWidth()
	ch := g.CellHeight()

	// calculate north west corner
	x = _x * (cw + g.gap)
	y = _y * (ch + g.gap)
	switch c {
	case consts.NWCorner:
		return
	case consts.NECorner:
		return x + cw, y
	case consts.SECorner:
		return x + cw, y + ch
	case consts.SWCorner:
		return x, y + ch
	}

	return
}

func (g *Grid) CellWidth() int {
	return (g.rect.Width() - ((g.cols - 1) * g.gap)) / g.cols
}

func (g *Grid) CellHeight() int {
	return (g.rect.Height() - ((g.rows - 1) * g.gap)) / g.rows
}

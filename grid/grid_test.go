package grid

import (
	"testing"

	"github.com/BurntSushi/xgbutil/xrect"
	"github.com/xen0ne/helium/consts"
)

const rows = 3
const cols = 4

var g *Grid = New(
										xrect.New(0, 0, 1920, 1080), rows, cols, 0) // "normal"
var g_offset *Grid = New(xrect.New(10, 15, 1910, 1065), rows, cols, 0)          // offset of 10, 15 from top left
var g_offset_all *Grid = New(xrect.New(10, 15, 1900, 1050), rows, cols, 0)      // offset of 10, 15 from top left
var g_gap *Grid = New(xrect.New(0, 0, 1920, 1080), rows, cols, 10)              // gap of 10
var g_offset_gap *Grid = New(xrect.New(10, 15, 1910, 1065), rows, cols, 10)     // gap of 10 offset of 10, 15
var g_offset_all_gap *Grid = New(xrect.New(10, 15, 1900, 1050), rows, cols, 10) // gap of 10 offset of 10, 15

func TestBounds(t *testing.T) {
	testBounds := func(g *Grid, en, es, ee, ew int) {
		n, s, e, w := g.GetBounds(1920, 1080)
		if n != en || s != es || e != ee || w != ew {
			t.Errorf("expected offset of %d, %d, %d, %d got %d, %d, %d, %d",
				en, es, ee, ew, n, s, e, w)
		}
	}
	testBounds(g, 0, 0, 0, 0)
	testBounds(g_offset, 15, 0, 0, 10)
	testBounds(g_offset_all, 15, 15, 10, 10)
	testBounds(g_gap, 0, 0, 0, 0)
	testBounds(g_offset_gap, 15, 0, 0, 10)
	testBounds(g_offset_all_gap, 15, 15, 10, 10)
}

func TestCellWidthAndHeight(t *testing.T) {
	testWH := func(g *Grid, ew, eh int) {
		cw := g.CellWidth()
		ch := g.CellHeight()
		if cw != ew || ch != eh {
			t.Errorf("expected w and h of %d %d but got %d %d", ew, eh, cw, ch)
		}
	}

	testWH(g, 480, 360)
	testWH(g_offset, 477, 355)
	testWH(g_offset_all, 475, 350)
	testWH(g_gap, 472, 353)
	testWH(g_offset_gap, 470, 348)
	testWH(g_offset_all_gap, 467, 343)
}

func TestCellCorner(t *testing.T) {
	testCorners := func(g *Grid, _x, _y,
		nwx, nwy, nex, ney, sex, sey, swx, swy int) {

		x, y := g.CellCorner(_x, _y, consts.NWCorner)
		if x != nwx || y != nwy {
			t.Errorf("NW expected x:%d y:%d not x:%d y:%d", nwx, nwy, x, y)
		}
		x, y = g.CellCorner(_x, _y, consts.NECorner)
		if x != nex || y != ney {
			t.Errorf("NE expected x:%d y:%d not x:%d y:%d", nex, ney, x, y)
		}

		x, y = g.CellCorner(_x, _y, consts.SWCorner)
		if x != swx || y != swy {
			t.Errorf("SW expected x:%d y:%d not x:%d y:%d", swx, swy, x, y)
		}
		x, y = g.CellCorner(_x, _y, consts.SECorner)
		if x != sex || y != sey {
			t.Errorf("SE expected x:%d y:%d not x:%d y:%d", sex, sey, x, y)
		}
	}

	testCorners(g, 0, 0,
		0, 0,
		480, 0,
		480, 360,
		0, 360)

	testCorners(g, 3, 1,
		1440, 360,
		1920, 360,
		1920, 720,
		1440, 720)

	testCorners(g_gap, 1, 0,
		482, 0,
		954, 0,
		954, 353,
		482, 353)
}

func TestSnapCorner(t *testing.T) {
	x, y := g_gap.SnapCorner(485, 10, consts.NWCorner)
	ex := 482
	ey := 0
	if x != ex || y != ey {
		t.Errorf("SE expected x:%d y:%d not x:%d y:%d", ex, ey, x, y)
	}
}

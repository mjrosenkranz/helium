package grid

import (
	"testing"

	"github.com/BurntSushi/xgbutil/xrect"
)

const rows = 3
const cols = 4

var g *Grid = New(xrect.New(0, 0, 1920, 1080), rows, cols, 0)               // "normal"
var g_offset *Grid = New(xrect.New(10, 15, 1910, 1065), rows, cols, 0)      // offset of 10, 15 from top left
var g_offset_all *Grid = New(xrect.New(10, 15, 1900, 1050), rows, cols, 0)  // offset of 10, 15 from top left
var g_gap *Grid = New(xrect.New(0, 0, 1920, 1080), rows, cols, 10)          // gap of 10
var g_offset_gap *Grid = New(xrect.New(10, 15, 1910, 1065), rows, cols, 10) // gap of 10 offset of 10, 15

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
}

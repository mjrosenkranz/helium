package grid

type Grid struct {
	cells [][]Cell
}

// GetCellAtPoint returns a pointer to the cell that contains the
// given x y coordinate is
func (*Grid) GetCellAtPoint(x, y int) *Cell {
	return NewCell(0, 0, 0, 0)
}

// New creates a new grid with top, bottom, left, right offets, r rows and c columns
func New(ot, ob, ol, or, r, c int) *Grid {
	cells := make([][]Cell, c)

	for x := range cells {
		cells[x] = make([]Cell, r)

		for y := range cells[x] {
			append(cells[x][y], NewCell(x, y, 100, 100))
		}
	}

	return &Grid{cells}
}

package grid

type Cell struct {
	x, y, w, h int
}

func NewCell(x, y, w, h int) *Cell {
	return &Cell{x, y, w, h}
}

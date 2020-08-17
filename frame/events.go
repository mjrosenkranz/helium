package frame

import (
	"log"

	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xevent"
)

func (f *Frame) manageEvents() {
	f.client.Listen(xproto.EventMaskPropertyChange |
		xproto.EventMaskStructureNotify)
	f.cDestroyNotify().Connect(X, f.client.Id)
}

func (f *Frame) cDestroyNotify() xevent.DestroyNotifyFun {
	fn := func(X *xgbutil.XUtil, ev xevent.DestroyNotifyEvent) {
		log.Println("unmanaging")
		f.bar.win.Destroy()
		f.parent.Destroy()
	}
	return xevent.DestroyNotifyFun(fn)
}

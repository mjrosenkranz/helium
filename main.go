package main

import (
	"fmt"
	"log"

	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/wm"
)

func main() {
	X, err := xgbutil.NewConn()
	if err != nil {
		log.Fatal(err)
	}
	defer X.Conn().Close()

	wm.Setup(X)

	xevent.MapRequestFun(
		func(X *xgbutil.XUtil, ev xevent.MapRequestEvent) {
			fmt.Println(ev.Window)
		}).Connect(X, wm.Root.Id)

	// start event loop
	pingBefore, pingAfter, pingQuit := xevent.MainPing(X)
	for {
		select {
		case <-pingBefore:
			// Wait for the event to finish processing.
			<-pingAfter
		// case otherVal := <-otherChan:
		// fmt.Printf("Processing other event: %d\n", otherVal)
		case <-pingQuit:
			fmt.Printf("xevent loop has quit")
			return
		}
	}
}

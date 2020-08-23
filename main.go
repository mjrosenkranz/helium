package main

import (
	"fmt"
	"log"
	"os"
	"strings"

	"github.com/BurntSushi/xgbutil/mousebind"

	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/ipc"
	"github.com/xen0ne/helium/wm"
)

func main() {
	if len(os.Args) > 1 {
		msg := strings.Join(os.Args[1:], " ")
		ipc.SendMsg(msg)
		return
	}

	X, err := xgbutil.NewConn()
	if err != nil {
		log.Fatal(err)
	}
	defer X.Conn().Close()

	mousebind.Initialize(X)
	wm.Setup(X)
	config.Defaults()
	AddHandlers()

	go ipc.RecieveMsg()

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

package main

import (
	"fmt"
	"os"
	"strings"

	"github.com/BurntSushi/xgbutil/mousebind"

	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/xevent"
	"github.com/xen0ne/helium/config"
	"github.com/xen0ne/helium/ipc"
	"github.com/xen0ne/helium/logger"
	"github.com/xen0ne/helium/wm"
)

func main() {
	logger.SetupLogger()

	if len(os.Args) > 1 {
		m := strings.Join(os.Args[1:], " ")
		ipc.CtrlMsg(m)
		return
	}

	X, err := xgbutil.NewConn()
	if err != nil {
		logger.Log.Fatal(err)
	}
	defer X.Conn().Close()

	mousebind.Initialize(X)
	config.Defaults()
	wm.Setup(X)
	AddHandlers()

	msgch := make(chan ipc.Msg)

	go ipc.RecieveMsg(msgch)

	// start event loop
	pingBefore, pingAfter, pingQuit := xevent.MainPing(X)
	for {
		select {
		case <-pingBefore:
			// Wait for the event to finish processing.
			<-pingAfter
		case msg := <-msgch:
			fmt.Printf("Recievd msg: %s\n", msg.Str)
			wm.HandleIpcMsg(msg)
		case <-pingQuit:
			fmt.Printf("xevent loop has quit")
			return
		}
	}
}

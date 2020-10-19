package main

import (
	"fmt"
	"os"
	"os/exec"
	"strings"

	"github.com/BurntSushi/xgbutil"
	"github.com/BurntSushi/xgbutil/mousebind"
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

	// now that initialization is done run the startup script
	cmd := &exec.Cmd{
		Path:   os.ExpandEnv("$HOME/docs/code/helium/extra/autostart.sh"),
		Stdout: os.Stderr,
		Stderr: os.Stderr,
	}

	err = cmd.Run()

	if err != nil {
		fmt.Print(err)
	}

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

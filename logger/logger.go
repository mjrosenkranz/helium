package logger

import (
	"log"
	"os"
)

var Log *log.Logger

// SetupLogger sets up our logger
func SetupLogger() {
	Log = log.New(os.Stderr, "helium: ", 0)
}

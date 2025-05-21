package main

import (
	"fmt"
	"net/http"
	"sync"
	"sync/atomic"
	"time"
)

const (
	url         = "http://localhost:81"
	numWorkers  = 20
	requestsPer = 5000
)

func main() {
	var successCount int64
	var failCount int64
	var wg sync.WaitGroup

	client := &http.Client{
		Timeout: 3 * time.Second, // strenger Timeout
	}

	fmt.Println("Starte Anfragen...")

	for i := 0; i < numWorkers; i++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			for j := 0; j < requestsPer; j++ {
				resp, err := client.Get(url)
				if err != nil {
					atomic.AddInt64(&failCount, 1)
					continue
				}
				resp.Body.Close()
				if resp.StatusCode >= 200 && resp.StatusCode < 300 {
					atomic.AddInt64(&successCount, 1)
				} else {
					atomic.AddInt64(&failCount, 1)
				}
			}
			fmt.Printf("Worker %d fertig\n", id+1)
		}(i)
	}

	wg.Wait()

	fmt.Println("Alle Anfragen abgeschlossen.")
	fmt.Printf("Erfolgreich: %d\n", successCount)
	fmt.Printf("Fehlgeschlagen: %d\n", failCount)
}
